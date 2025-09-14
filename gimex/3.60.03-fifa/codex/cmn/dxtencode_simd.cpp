// (c) 1997-2007 Electronic Arts Inc.

#ifndef __S3WRITE
#define __S3WRITE   1

// remove TIMING if not timing

#if defined(_MSC_VER)
#define TIMING 1
#pragma warning(push,1)
#endif

#if defined(_MSC_VER) && !defined(_XBOX)
#include "emmintrin.h"
#endif

#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#pragma warning(disable: 4514)
#endif

#include "codex.h"
#include "dxtcodex.h"
#include "string.h"

#define S3TCPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

/******************************************************/
/** Timer for debug                                  **/
/******************************************************/

#ifdef TIMING
#include "windows.h"
#include "spit.h"

#ifdef _MSC_VER
static double i64tod(__int64 it)
{
    return (double) it;
    //t = ((double)(unsigned int)(t1.HighPart)*4294967296.0) + ((double)(unsigned int)(t1.LowPart));
}
#endif

double SPIT_gettime(void)
{
    double t=0;

#ifdef _MSC_VER

    t = i64tod(GetTickCount());

#elif defined(__unix__) || defined(__BEOS__) || defined(__APPLE__)
    {
        struct timeval tv;
        struct timezone tz; // not used
        gettimeofday(&tv,&tz);
        t = (double)tv.tv_sec*1000.0+(double)tv.tv_usec/1000.0;
    }
#endif
    return(t);
}
#endif

/**
 * CompressBlock
 * -------------
 * Dxt compressed the given data block, srcBlock, and places the compressed data block into destBlock.
 *
 * parameters: srcBlock    - the block data to compress
 *             destBlock   - the location in which to store the compressed block data
 *             imgWidth    - the width of the full image in pixels (used to calculate scanline offsets)
 *             blockWidth  - some blocks may be less than width 4
 *             blockHeight - come blocks may be less than height 4
 *
 * input block format (512 bits, 64 bytes)
 *     16 pixels, each 32 bits in length (1 byte for each of a, r, g, b)
 *
 * output block format (64 bits, 8 bytes):
 *     16 bit - colour 1
 *     16 bit - colour 2
 *     32 bit - colour indices (16 indices that are 2 bits each)
 *
 * Written by: Pavel Margolin
 * Initial date: January 9, 2002
 */


static inline unsigned int c5to8(unsigned int rgb565)
{
    return 0xff000000
        | (((((rgb565>>11)&31)*255+16)/31)<<16)
        | (((((rgb565>>5)&63)*255+32)/63)<<8)
        | (((((rgb565   )&31)*255+16)/31)   );
}

static inline unsigned int c8to5(unsigned int rgb888)
{
    return ((((rgb888>>16)&255)>>3)<<11)
         | ((((rgb888>>8)&255)>>2)<<5)
         | ((((rgb888   )&255)>>3)   );
}

unsigned int Alpha8to4(unsigned int a8)
{
    return ((a8*15+128)/255);
}

// 0 -> 1
// 1 -> 7
// 2 -> 6
// 3 -> 5
// 4 -> 4
// 5 -> 3
// 6 -> 2
// 7 -> 0
unsigned int bitmapper[8]={1,7,6,5,4,3,2,0};
unsigned int Alpha8to3(unsigned int a8, unsigned int mina, unsigned int maxa)
{
    if (maxa==mina)
        return 0;

    return bitmapper[(((a8-mina)*7+(maxa-mina+1)/2)/(maxa-mina))];
}

static inline unsigned int interpolate_channel(unsigned int *p, unsigned int shiftval, int i1,int i2,int d)
{
    unsigned int c0 = ((p[0]>>shiftval)&255);
    unsigned int c1 = ((p[1]>>shiftval)&255);
    return ((c0*i1 + c1*i2)/d) << shiftval;
}

static inline unsigned int interpolate(unsigned int *p,int i1,int i2,int d)
{
    return 0xff000000
         | interpolate_channel(p,16,i1,i2,d)
         | interpolate_channel(p,8,i1,i2,d)
         | interpolate_channel(p,0,i1,i2,d);

}

#if defined(_MSC_VER) && !defined(_XBOX)

#if 1
// This function accepts 2 vectors of colors.  Each vector contains 4 argb color values.
// The L2 difference of the color values are computed and returned as 4 unsigned shorts in the first
// 4 elements of the result vector.  Any values greater than 65535 are clamped.
__m128i L2Difference( const __m128i &colA, const __m128i &colB )
{
    __m128i maxValues = _mm_max_epu8(colA, colB);
    __m128i minValues = _mm_min_epu8(colA, colB);
    __m128i differences = _mm_sub_epi8(maxValues, minValues);
    __m128i zero = _mm_xor_si128(colA, colA);
    __m128i lowDiffs  = _mm_unpacklo_epi8( zero, differences );
    __m128i highDiffs = _mm_unpackhi_epi8( zero, differences );
    __m128i lowProd  = _mm_mulhi_epu16(lowDiffs, lowDiffs);
    __m128i highProd = _mm_mulhi_epu16(highDiffs, highDiffs);

    // Now all the products are computed so all that remains is summing up values
    __m128i aarrggbb02 = _mm_unpacklo_epi16( lowProd, highProd );
    __m128i aarrggbb13 = _mm_unpackhi_epi16( lowProd, highProd );
    __m128i aaaarrrr0123 = _mm_unpacklo_epi16( aarrggbb02, aarrggbb13 );
    __m128i ggggbbbb0123 = _mm_unpackhi_epi16( aarrggbb02, aarrggbb13 );
    __m128i partialSum = _mm_adds_epu16( aaaarrrr0123, ggggbbbb0123 );
    __m128i partialSum2 = _mm_srli_si128( partialSum, 8 );
    __m128i sum = _mm_adds_epu16(partialSum, partialSum2);
    return sum;
}
#endif

#if 0
// This function accepts 2 vectors of colors.  Each vector contains 4 argb color values.
// The L2 difference of the color values are computed and returned as 4 unsigned shorts in the first
// 4 elements of the result vector.  Any values greater than 65535 are clamped.
static inline __m128i L2Difference( const __m128i &colA, const __m128i &colB )
{
    __m128i maxValues = _mm_max_epu8(colA, colB);
    __m128i minValues = _mm_min_epu8(colA, colB);
    __m128i differences = _mm_sub_epi8(maxValues, minValues);
    __m128i zero = _mm_xor_si128(colA, colA);
    __m128i lowDiffs  = _mm_unpacklo_epi8( zero, differences );
    __m128i lowProd  = _mm_mulhi_epu16(lowDiffs, lowDiffs);

    // Now all the products are computed so all that remains is summing up values
    __m128i aarrggbb02 = _mm_unpacklo_epi16( lowProd, lowProd );
    __m128i aaaarrrr0123 = _mm_unpacklo_epi16( aarrggbb02, aarrggbb02 );
    __m128i ggggbbbb0123 = _mm_unpackhi_epi16( aarrggbb02, aarrggbb02 );
    __m128i partialSum = _mm_adds_epu16( aaaarrrr0123, ggggbbbb0123 );
    __m128i partialSum2 = _mm_srli_si128( partialSum, 8 );
    __m128i sum = _mm_adds_epu16(partialSum, partialSum2);
    return sum;
}
#endif

static inline unsigned int diffpixels(unsigned int p0, unsigned int p1)
{
    __m128i colA;
    __m128i colB;
    colA.m128i_u32[0] = p0;
    colB.m128i_u32[0] = p1;

    __m128i diff = L2Difference(colA,colB);
    return diff.m128i_u16[0];
}


static inline unsigned int diff_channel(unsigned int p0, unsigned int p1, unsigned int shiftval)
{
    int c0 = ((p0>>shiftval)&255);
    int c1 = ((p1>>shiftval)&255);

    int d = c0-c1;
    return d*d;
}

static inline unsigned int trymappixel(unsigned int pixel, unsigned int *p)
{

    __m128i colA;
    colA.m128i_u32[0] = pixel;
    colA.m128i_u32[1] = pixel;
    colA.m128i_u32[2] = pixel;
    colA.m128i_u32[3] = pixel;

    __m128i diff = L2Difference(colA,*(__m128i*)p);

    unsigned int d1 = min( diff.m128i_u16[0],  diff.m128i_u16[1]);
    unsigned int d2 = min( diff.m128i_u16[2],  diff.m128i_u16[3]);
    return min(d1,d2);
}


#else

static inline unsigned int diff_channel(unsigned int p0, unsigned int p1, unsigned int shiftval)
{
    int c0 = ((p0>>shiftval)&255);
    int c1 = ((p1>>shiftval)&255);

    int d = c0-c1;
    return d*d;
}

static inline unsigned int diffpixels(unsigned int p0, unsigned int p1)
{
    return diff_channel(p0,p1,24)
         + diff_channel(p0,p1,16)
         + diff_channel(p0,p1,8)
         + diff_channel(p0,p1,0);
}
static inline unsigned int trymappixel(unsigned int pixel, unsigned int *p)
{
    unsigned int bestdiff = diffpixels(pixel,p[0]);

    for (unsigned int i=1; i<4; ++i)
    {
        unsigned int diff = diffpixels(pixel,p[i]);
        if (diff<bestdiff)
        {
            bestdiff = diff;
        }
    }
    return bestdiff;
}

#endif


static inline unsigned int mappixel(unsigned int pixel, unsigned int *bits2,unsigned int *p)
{
    unsigned int bestdiff=0x7fffffff;
    unsigned int besti=0;

    for (unsigned int i=0; i<4; ++i)
    {
        unsigned int diff = diffpixels(pixel,p[i]);
        if (diff<bestdiff)
        {
            bestdiff = diff;
            besti=i;
        }
    }
    *bits2 = besti;
    return bestdiff;
}





static inline unsigned int rmappixel(unsigned int pixel, unsigned int *p)
{
    unsigned int bestdiff=0x7fffffff;

    for (unsigned int i=0; i<4; ++i)
    {
        unsigned int diff = diff_channel(pixel,p[i],0);
        if (diff<bestdiff)
        {
            bestdiff = diff;
        }
    }
    return bestdiff;
}


static inline unsigned int rbmappixel(unsigned int pixel, unsigned int *p)
{
    unsigned int bestdiff=0x7fffffff;

    for (unsigned int i=0; i<4; ++i)
    {
        unsigned int diff = diff_channel(pixel,p[i],0)
                          + diff_channel(pixel,p[i],16);
        if (diff<bestdiff)
        {
            bestdiff = diff;
        }
    }
    return bestdiff;
}

// TRY list to keep track of colour combos we've already tried.

#if 1
#define TRYMAX 256
int trylen=0;
unsigned int trylist[TRYMAX];
static inline void TRY_clear(void)
{
    trylen = 0;
}

static inline void TRY_add(unsigned int c0, unsigned int c1)
{
    unsigned int tc = (c0<<16)|c1;
    if (trylen<TRYMAX)
    {
        trylist[trylen]=tc;
        ++trylen;
    }
}

static inline bool TRY_check(unsigned int c0, unsigned int c1)
{
    unsigned int tc = (c0<<16)|c1;
    for (int i=0; i<trylen; ++i)
    {
        if (trylist[i]==tc)
            return true;
    }
    return false;
}
#else
#define TRY_clear()
#define TRY_add(c0,c1)
#define TRY_check(c0,c1) false
#endif



#ifdef TIMING
int trymapblock_iterations=0;
#endif

static unsigned int trymapblock(void* srcBlock, unsigned int c0, unsigned int c1, unsigned int bestdiff)
{
#if 1
    if (TRY_check(c0,c1))
        return 0x7fffffff;
    TRY_add(c0,c1);
#endif

#ifdef TIMING
    ++trymapblock_iterations;
#endif

    unsigned int p[4];
    p[0] = c5to8(c0);
    p[1] = c5to8(c1);
    if (c0>c1)
    {
        p[2] = interpolate(p,2,1,3);
        p[3] = interpolate(p,1,2,3);
    }
    else
    {
        p[2] = interpolate(p,1,1,2);
        p[3] = 0;
    }
    unsigned int diff=0;

    unsigned int *sourcerow = ((unsigned int *) srcBlock);

    for (int y=0; y<4; ++y)
    {
        for (int x=0; x<4; ++x)
        {
            diff += trymappixel(*sourcerow,p);
            if (diff>bestdiff) return diff;

            ++sourcerow;
        }
    }


    return diff;
}


static unsigned int mapblock(void* srcBlock, void* destBlock, unsigned int c0, unsigned int c1)
{
    unsigned int p[4];
    p[0] = c5to8(c0);
    p[1] = c5to8(c1);
    if (c0>c1)
    {
        p[2] = interpolate(p,2,1,3);
        p[3] = interpolate(p,1,2,3);
    }
    else
    {
        p[2] = interpolate(p,1,1,2);
        p[3] = 0;
    }
    ((unsigned short *)destBlock)[0] = c0;
    ((unsigned short *)destBlock)[1] = c1;
    unsigned int diff=0;

    unsigned int *sourcerow = ((unsigned int *) srcBlock);

    for (int y=0; y<4; ++y)
    {
        unsigned int bits=0;
        for (int x=0; x<4; ++x)
        {
            unsigned int bits2=0;
            diff += mappixel(*sourcerow,&bits2,p);
            bits |= bits2<<(x*2);

            ++sourcerow;
        }
        ((unsigned char *)destBlock+4)[y] = bits;
    }


    return diff;
}

// just red mapping
static int rtrymapblock(ARGB* srcBlock, unsigned int c0, unsigned int c1)
{
    unsigned int p[4];
    p[0] = c5to8(c0);
    p[1] = c5to8(c1);
    if (c0>c1)
    {
        p[2] = interpolate(p,2,1,3);
        p[3] = interpolate(p,1,2,3);
    }
    else
    {
        p[2] = interpolate(p,1,1,2);
        p[3] = 0;
    }
    unsigned int diff=0;
    for (int y=0; y<16; ++y)
    {
        diff += rmappixel(srcBlock[y].r,p);
    }
    return diff;
}

// red and blue mapping

static int rbtrymapblock(ARGB* srcBlock, unsigned int c0, unsigned int c1)
{
    unsigned int p[4];
    p[0] = c5to8(c0);
    p[1] = c5to8(c1);
    if (c0>c1)
    {
        p[2] = interpolate(p,2,1,3);
        p[3] = interpolate(p,1,2,3);
    }
    else
    {
        p[2] = interpolate(p,1,1,2);
        p[3] = 0;
    }
    unsigned int diff=0;
    for (int y=0; y<16; ++y)
    {
        diff += rbmappixel(((unsigned int *)srcBlock)[y],p);
    }
    return diff;
}


static void ExpliciteAlphaBlock( void* srcBlock, void* destBlock, unsigned int imgWidth)
{
    unsigned short *d=(unsigned short *) destBlock;
    ARGB *s = (ARGB *)srcBlock;
    for (int y=0; y<4; ++y)
    {
        *d = Alpha8to4(s[0].a)
          | (Alpha8to4(s[1].a)<<4)
          | (Alpha8to4(s[2].a)<<8)
          | (Alpha8to4(s[3].a)<<12);
        s += imgWidth;
        d += 1;
    }
}

static void InterpolatedAlphaBlock( void* srcBlock, void* destBlock, unsigned int imgWidth )
{
    unsigned short *d=(unsigned short *) destBlock;
    ARGB *s = (ARGB *)srcBlock;
    unsigned int mina=255, maxa=0;
    for (int y=0; y<4; ++y)
    {
        for (int x=0; x<4; ++x)
        {
            mina = gmin(mina,s[x+y*4].a);
            maxa = gmax(maxa,s[x+y*4].a);
        }
    }

    unsigned int r[4];
    for (int y=0; y<4; ++y)
    {
        r[y] = Alpha8to3(s[0].a,mina,maxa)
            | (Alpha8to3(s[1].a,mina,maxa)<<3)
            | (Alpha8to3(s[2].a,mina,maxa)<<6)
            | (Alpha8to3(s[3].a,mina,maxa)<<9);
        s += imgWidth;
    }

    ((unsigned char *)d)[0] = maxa;
    ((unsigned char *)d)[1] = mina;
    ++d;
    d[0] = r[0] | ((r[1]&15)<<12);      // 12 bits r0 and 4 bits r1
    d[1] = (r[1]>>4) | ((r[2]&255)<<8); // 8 bits r1 and 8 bits r2
    d[2] = (r[2]>>8) | ((r[3])<<4);     // 4 bits r2 and 12 bits r3
}


static unsigned int CompressBlock( void* srcBlock, unsigned int *bestc0ptr, unsigned int *bestc1ptr, int quality )
{
    /******************************************************/
    /** Find bounding box                                **/
    /******************************************************/

    ARGB newSrc[16];
    ARGB minrgb={255,255,255,255};
    ARGB maxrgb={0,0,0,0};
    int i=0;
    for (int y=0; y<4; ++y)
    {
        for (int x=0; x<4; ++x)
        {
            ARGB rgb = ((ARGB *)srcBlock)[x+y*4];
            newSrc[i++] = rgb;
            minrgb.a = gmin(minrgb.a,rgb.a);
            minrgb.r = gmin(minrgb.r,rgb.r);
            minrgb.g = gmin(minrgb.g,rgb.g);
            minrgb.b = gmin(minrgb.b,rgb.b);
            maxrgb.a = gmax(maxrgb.a,rgb.a);
            maxrgb.r = gmax(maxrgb.r,rgb.r);
            maxrgb.g = gmax(maxrgb.g,rgb.g);
            maxrgb.b = gmax(maxrgb.b,rgb.b);
        }
    }
//  maxrgb.r = gmin(255,maxrgb.r+7);
//  maxrgb.g = gmin(255,maxrgb.g+3);
//  maxrgb.b = gmin(255,maxrgb.b+7);

    TRY_clear();

    int maxiterations=1;
#if 0
    if (quality<20)
        maxiterations=quality/5;
    if (quality>=70)
        maxiterations=2;
#endif

    /******************************************************/
    /** Seed with min/max of bounding box                **/
    /******************************************************/

    unsigned int bestc0 = c8to5(*(unsigned int *)&minrgb);
    unsigned int bestc1 = c8to5(*(unsigned int *)&maxrgb);

    unsigned int bestdiff = trymapblock(srcBlock, bestc0, bestc1, 0x7fffffff);

    /******************************************************/
    /** Search all combinations of colours in 4x4        **/
    /******************************************************/

    if (quality>0)
    {
        // seed with all colour combinations in the 4x4.  This is a reasonally good solution
        for (int i0=0; i0<16; ++i0)
        {
            unsigned int c0 = c8to5(((unsigned int *) &newSrc)[i0]);

            for (int i1=0; i1<16; ++i1)
            {
                unsigned int c1 = c8to5(((unsigned int *) &newSrc)[i1]);
//              if (!TRY_check(c0,c1))
                    if (c0!=c1) // pruning, since these wont be optimal solution.  3x speed improvement
                    {
                        unsigned int diff = trymapblock(srcBlock, c0, c1, bestdiff);
                        if (diff<bestdiff)
                        {
                            bestdiff = diff;
                            bestc0 = c0;
                            bestc1 = c1;
                            if (!bestdiff) goto breakout;
                        }
                    }
            }
        }
    }

    /******************************************************/
    /** Refine search                                    **/
    /******************************************************/

    for (int interations=0; interations < maxiterations; ++interations)
    {
        if (quality>10)
        {
            ARGB search0_minc5;
            ARGB search0_maxc5;
            ARGB search1_minc5;
            ARGB search1_maxc5;

            ARGB fullcube_minc5;
            ARGB fullcube_maxc5;

            // full cube for Q100 and pruning local search
            {
                ARGB fullcube_minrgb;
                ARGB fullcube_maxrgb;

                fullcube_minrgb.a = gmax(0,minrgb.a-(maxrgb.a-minrgb.a));
                fullcube_minrgb.r = gmax(0,minrgb.r-(maxrgb.r-minrgb.r));
                fullcube_minrgb.g = gmax(0,minrgb.g-(maxrgb.g-minrgb.g));
                fullcube_minrgb.b = gmax(0,minrgb.b-(maxrgb.b-minrgb.b));

                fullcube_maxrgb.a = gmin(255,maxrgb.a+(maxrgb.a-minrgb.a));
                fullcube_maxrgb.r = gmin(255,maxrgb.r+(maxrgb.r-minrgb.r));
                fullcube_maxrgb.g = gmin(255,maxrgb.g+(maxrgb.g-minrgb.g));
                fullcube_maxrgb.b = gmin(255,maxrgb.b+(maxrgb.b-minrgb.b));

                // extend to allow interpolated values to be found
                fullcube_minc5.r = gmax(0,(fullcube_minrgb.r>>3)-1);
                fullcube_minc5.g = gmax(0,(fullcube_minrgb.g>>2)-1);
                fullcube_minc5.b = gmax(0,(fullcube_minrgb.b>>3)-1);
                fullcube_maxc5.r = gmin(31,(fullcube_maxrgb.r>>3)+2);
                fullcube_maxc5.g = gmin(63,(fullcube_maxrgb.g>>2)+2);
                fullcube_maxc5.b = gmin(31,(fullcube_maxrgb.b>>3)+2);

            }

            if (quality>=100) /* Q100 search entire bounding box plus border */
            {
                search1_minc5 = search0_minc5 = fullcube_minc5;
                search1_maxc5 = search0_maxc5 = fullcube_maxc5;
            }
            else /* search area of end points */
            {
                int distdown=quality/50;
                int distup=quality/40+1;

                // extend to allow interpolated values to be found
                search0_minc5.r = gmax(0,gmax(fullcube_minc5.r,((bestc0>>11)&31)-distdown));
                search0_minc5.g = gmax(0,gmax(fullcube_minc5.g,((bestc0>>5)&63)-distdown));
                search0_minc5.b = gmax(0,gmax(fullcube_minc5.b,((bestc0)&31)-distdown));
                search0_maxc5.r = gmin(31,gmin(fullcube_maxc5.r,((bestc0>>11)&31)+distup));
                search0_maxc5.g = gmin(63,gmin(fullcube_maxc5.g,((bestc0>>5)&63)+distup));
                search0_maxc5.b = gmin(31,gmin(fullcube_maxc5.b,((bestc0)&31)+distup));

                search1_minc5.r = gmax(0,gmax(fullcube_minc5.r,((bestc1>>11)&31)-distdown));
                search1_minc5.g = gmax(0,gmax(fullcube_minc5.g,((bestc1>>5)&63)-distdown));
                search1_minc5.b = gmax(0,gmax(fullcube_minc5.b,((bestc1)&31)-distdown));
                search1_maxc5.r = gmin(31,gmin(fullcube_maxc5.r,((bestc1>>11)&31)+distup));
                search1_maxc5.g = gmin(63,gmin(fullcube_maxc5.g,((bestc1>>5)&63)+distup));
                search1_maxc5.b = gmin(31,gmin(fullcube_maxc5.b,((bestc1)&31)+distup));
            }


    /******************************************************/
    /** Refine Method A: one end at a time               **/
    /******************************************************/

            if (quality<30)
            {
                /* adjust c1 */

                unsigned int r0 = (bestc0>>11)&31;
                unsigned int g0 = (bestc0>>5)&63;
                unsigned int b0 = (bestc0)&31;

                {
                    for (unsigned int r1=search1_minc5.r; r1<=search1_maxc5.r; r1+=1)
                    {
                        for (unsigned int b1=search1_minc5.b; b1<=search1_maxc5.b; b1+=1)
                        {
                            for (unsigned int g1=search1_minc5.g; g1<=search1_maxc5.g; g1+=1)
                            {
                                unsigned int c1=(r1<<11)|(g1<<5)|b1;

                                unsigned int diff = trymapblock(srcBlock, bestc0, c1, bestdiff);

                                if (diff<bestdiff)
                                {
                                    bestdiff = diff;
                                    bestc1 = c1;
                                    if (!bestdiff) goto breakout;
//                                  goto donec0;
                                }
                            }
                        }
                    }
                }
//              donec0: ;

                /* adjust c0 */

                unsigned int r1 = (bestc1>>11)&31;
                unsigned int g1 = (bestc1>>5)&63;
                unsigned int b1 = (bestc1)&31;
                {
                    for (unsigned int r0=search0_minc5.r; r0<=search0_maxc5.r; r0+=1)
                    {
                        for (unsigned int b0=search0_minc5.b; b0<=search0_maxc5.b; b0+=1)
                        {
                            for (unsigned int g0=search0_minc5.g; g0<=search0_maxc5.g; g0+=1)
                            {
                                unsigned int c0=(r0<<11)|(g0<<5)|b0;

                                unsigned int diff = trymapblock(srcBlock, c0, bestc1, bestdiff);

                                if (diff<bestdiff)
                                {
                                    bestdiff = diff;
                                    bestc0 = c0;
                                    if (!bestdiff) goto breakout;
//                                  goto donec1;
                                }
                            }
                        }
                    }
                }
//              donec1: ;
            }

    /******************************************************/
    /** Refine Method B: both ends all combos            **/
    /******************************************************/

            #define THRESHHOLD 16

            else
            {
                for (unsigned int r0=search0_minc5.r; r0<=search0_maxc5.r; r0+=1)
                {
                    for (unsigned int r1=search1_minc5.r; r1<=search1_maxc5.r; r1+=1)
                    {
                        unsigned int rdiff = rtrymapblock(newSrc, r0, r1);
                        if (rdiff<bestdiff)
                        {
                            for (unsigned int b0=search0_minc5.b; b0<=search0_maxc5.b; b0+=1)
                            {
                                unsigned int rb0=(r0<<11)|b0;
                                for (unsigned int b1=search1_minc5.b; b1<=search1_maxc5.b; b1+=1)
                                {
                                    unsigned int rb1=(r1<<11)|b1;

                                    unsigned int rbdiff = rbtrymapblock(newSrc, rb0, rb1);
                                    if (rbdiff<bestdiff)
                                    {
                                        for (unsigned int g0=search0_minc5.g; g0<=search0_maxc5.g; g0+=1)
                                        {
                                            unsigned int c0=(r0<<11)|(g0<<5)|b0;

                                            for (unsigned int g1=search1_minc5.g; g1<=search1_maxc5.g; g1+=1)
                                            {
                                                unsigned int c1=(r1<<11)|(g1<<5)|b1;

                    //                          if (prune(c0, c1, search0_minrgb, search0_maxrgb))
                                                {
//                                                  if (!TRY_check(c0,c1))
                                                    {
                                                        unsigned int diff = trymapblock(srcBlock, c0, c1, bestdiff);

                                                        if (diff<bestdiff)
                                                        {
                                                            bestdiff = diff;
                                                            bestc0 = c0;
                                                            bestc1 = c1;
                                                            if (!bestdiff) goto breakout;
                                                        }
                                                        else
                                                            if ((diff-bestdiff)>THRESHHOLD) g1+=1;
                                                    }

                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if ((rbdiff-bestdiff)>THRESHHOLD*THRESHHOLD*THRESHHOLD) b1+=1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (quality==100) goto breakout;
        }
//      if (quality==100 || quality<20) goto breakout;
    }

breakout:
    *bestc0ptr = bestc0;
    *bestc1ptr = bestc1;
    return bestdiff;
}



/**
 * CompressData
 * ------------
 * Dxt compresses the given data (pixel image), dataIn, and places the compressed data into dataOut.
 *
 * parameters: dataIn  - the data to compress
 *             dataOut - the location in which to store the compressed data
 *             width   - the width of the image in pixels.
 *             height  - the height of the image in pixels.
 *
 * input block format (512 bits, 64 bytes)
 *     16 pixels, each 32 bits in length (1 byte for each of a, r, g, b)
 *
 * output block format (64 bits, 8 bytes):
 *     16 bit - colour 1
 *     16 bit - colour 2
 *     32 bit - colour indices (16 indices that are 2 bits each)
 *
 * Written by: Pavel Margolin
 * Initial date: January 8, 2002
 */
void CompressData( void* dataIn, void* dataOut, unsigned int width, unsigned int height, unsigned int mode, int quality )
{
    char* curOutput        = (char*)dataOut; // pointer to the current 64 bit (8 byte) output block
    unsigned int curInput = 0;              // offset to the current 16 pixel (64 byte) input block

#ifdef TIMING
    double t=SPIT_gettime();
#endif

    // We would like to call CompressBlock for each 4x4 pixel data block in dataIn.

    // If the image size is not a multiple of 4 in either height or width, then we will
    // have to consider some blocks that are smaller than 4x4.
    for( unsigned int j = 0; j < height; j += 4 )
    {
        unsigned int blockHeight; // may be less than 4
        // blockHeight = gmin( 4, height - j );
        if ( 4 < height - j )
        {
            blockHeight = 4;
        }
        else
        {
            blockHeight = height - j;
        }

        for( unsigned int i = 0; i < width; i += 4 )
        {
            int blockWidth; // may be less than 4
            // blockwidth = gmin( 4, width - i );
            if ( 4 < width - i )
            {
                blockWidth = 4;
            }
            else
            {
                blockWidth = width - i;
            }

            // units in pixels
            curInput = j*width + i; // pointer to the current input block

            if (mode & DXT_MODE_ALPHA_EXPLICIT) // dxt3 4 bit alpha
            {
                ExpliciteAlphaBlock( (unsigned int *)dataIn + curInput, curOutput, width );
                curOutput += 8;
            }
            else if (mode & DXT_ENCODE_ALPHA_INTERPOLATED)// dxt5 3 bit alpha
            {
                InterpolatedAlphaBlock( (unsigned int *)dataIn + curInput, curOutput, width );
                curOutput += 8;
            }

            unsigned int bestc0, bestc1;
            CompressBlock( (unsigned int *)dataIn + curInput, &bestc0, &bestc1, quality );

            mapblock((unsigned int *)dataIn + curInput, curOutput, bestc0, bestc1);

            curOutput += 8;
        }
    }
#ifdef TIMING
    t=SPIT_gettime()-t;
    int blocks = width/4 * height/4;
    printf("DXT time %5.0f ms, %d maps, %.2f maps/block, q %d\n",t,trymapblock_iterations,(float)trymapblock_iterations/blocks,quality);
#endif

}


/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL DXT_encode(void *compressed, const void *uncompressed, int numbytes, int *options)
{
    int width= 0;
    int height= 0;
    int rowbytes= 0;
    int encodetype=DXT_ENCODE_RGB_FULL;
    int quality = 50;
    int mode= 0;
    int rgb555=0;
    int BigEndian = 0;
    int bigSize   = 0;
//    float weight[] = {0.3f,0.4f,0.3f};
    DXTHEADER *header = (DXTHEADER*)compressed;
    BIGDXTHEADER *Bigheader = (BIGDXTHEADER*)compressed;          // just in case my image size is greater
                                                                  // than 16MB (3 bytes DXTHEADER SIZE FIELD)
    if(numbytes > 16000000)      // bigger than 16BM then have to use alternate header.
        bigSize = 1;

    if (options)
    {
        encodetype = options[0];
        quality    = options[1]; // 0 fast, 50 medium, 100 slow
    }

/*  0 for no alpha,                                                 */
/*  1 for explicit alpha,                                           */
/*  2 for interpolated alpha.                                       */
/*  4 for bgr555 little endian                                      */
/*  8 for gsh bigendian                                             */
    mode = 0;
    int BytesPerBlock=8;
    if (encodetype&DXT_ENCODE_ALPHA_EXPLICIT)
    {
        mode = DXT_MODE_ALPHA_EXPLICIT; // 1 = DXT3
        BytesPerBlock = 16;
    }
    else if (encodetype&DXT_ENCODE_ALPHA_INTERPOLATED)
    {
        mode = DXT_ENCODE_ALPHA_INTERPOLATED;
        BytesPerBlock = 16;
    }
    else
        mode = DXT_MODE_RGB;
    if (encodetype & DXT_ENCODE_RGB_555)
    {
        rgb555 = 1;
        mode |= DXT_MODE_RGB555;
    }
    if(encodetype & DXT_DXT1B_ENCODEOPTION /*DXT_DXT1B_CODE*/)
    {
        BigEndian = 1;                      // set the bigendian flag.
        mode = DXT_MODE_RGB;               // only DXT1 mode
    }
    width = 4;
    rowbytes = 16;
    height = numbytes/rowbytes;
    if(encodetype & DXT_ENCODE_ALPHA_EXPLICIT)
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, DXT_DXT3_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT3_CODE);
    }
    else if(encodetype & DXT_ENCODE_ALPHA_INTERPOLATED)
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, DXT_DXT5_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT5_CODE);

    }
    else if(encodetype & DXT_DXT1B_ENCODEOPTION)
    {
        if(!bigSize)
        {
            S3TCPUTFIELD(header->code, DXT_DXT1B_CODE);          // we have set the BigEndian flag so mark as Bigendian GSH style DXT.
            encodetype = encodetype&~DXT_DXT1B_ENCODEOPTION;     // just bookeeping so get rid of here so it doesn't interfere
        }                                                         // with compression to DXT1.
        else
        {
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT1B_CODE);    // we have set the BigEndian flag so mark as Bigendian GSH style DXT.
            encodetype = encodetype&~DXT_DXT1B_ENCODEOPTION;     // just bookeeping so get rid of here so it doesn't interfere
        }                                                         // with compression to DXT1.
    }
    else
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, DXT_DXT1_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT1_CODE);
    }
    //S3TCPUTFIELD(header->code, (encodetype & DXT_ENCODE_ALPHA_EXPLICIT) ? DXT_DXT3_CODE : DXT_DXT1_CODE);
    if(!bigSize)
    {
        S3TCPUTFIELD(header->size, numbytes);
        S3TCPUTFIELD(header->mode, mode);
    }
    else
    {
        S3TCPUTFIELD(Bigheader->size, numbytes);
        S3TCPUTFIELD(Bigheader->mode, mode);
    }

    CompressData( (unsigned char*)uncompressed, (unsigned char*)compressed + sizeof(DXTHEADER), width, height, mode, quality );

    return ((width + 3) / 4) * ((height + 3) / 4) * BytesPerBlock + sizeof(DXTHEADER);
}

#undef S3TCPUTFIELD

#endif



