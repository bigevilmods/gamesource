// (c) 2007 Electronic Arts Inc.

// version 1.4b

#ifndef EADXT_CPP
#define EADXT_CPP 1

#define FAST 1

#if defined(_MSC_VER)
#pragma warning(push,2)
#endif

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// enabling this will allow dxt3/5 to use opaque black in 3 colour mode.  rumour has it that ati dont support this
//define DXTUSEBLACK 1

// enable this for hashing to speed
#define ENABLE_TRYLIST 1

#include "eadxt.h"

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

#if defined(__PPCBROADWAY__) || defined(_R5900)
#define myuintptr_t unsigned int
#else
#define myuintptr_t uintptr_t
#endif

#define EADXT_offsetof(s,f) (int)((myuintptr_t)&(((s *)0)->f))
// shift is endian sensitive
#if (defined(_MSC_VER) && !defined(_M_PPC)) || defined(__i386__) || defined(__R5900)
#define EADXT_ALPHASHIFT (EADXT_offsetof(ARGB,a)*8)
#define EADXT_REDSHIFT   (EADXT_offsetof(ARGB,r)*8)
#define EADXT_GREENSHIFT (EADXT_offsetof(ARGB,g)*8)
#define EADXT_BLUESHIFT  (EADXT_offsetof(ARGB,b)*8)
#else
#define EADXT_ALPHASHIFT (24-(EADXT_offsetof(ARGB,a)*8))
#define EADXT_REDSHIFT   (24-(EADXT_offsetof(ARGB,r)*8))
#define EADXT_GREENSHIFT (24-(EADXT_offsetof(ARGB,g)*8))
#define EADXT_BLUESHIFT  (24-(EADXT_offsetof(ARGB,b)*8))
#endif

#if FAST
// rounding is not fully accurate

static inline unsigned int channel5to8(unsigned int c5)
{
    return (c5<<3)|(c5>>2);
}

static inline unsigned int channel6to8(unsigned int c6)
{
    return (c6<<2)|(c6>>4);
}

#else

static inline unsigned int channel5to8(unsigned int c5)
{
    return (c5*255+16)/31;
}

static inline unsigned int channel6to8(unsigned int c6)
{
    return (c6*255+32)/63;
}

#endif


static inline unsigned int c5to8(unsigned int rgb565)
{
    return                        (0xff<<EADXT_ALPHASHIFT)
        | (channel5to8((rgb565>>11)&31)<<EADXT_REDSHIFT)
        | (channel6to8((rgb565>>5 )&63)<<EADXT_GREENSHIFT)
        | (channel5to8((rgb565    )&31)<<EADXT_BLUESHIFT);
}
static inline unsigned int c8to5(unsigned int rgb888)
{
    return ((((rgb888>>EADXT_REDSHIFT)&255)>>3)<<11)
         | ((((rgb888>>EADXT_GREENSHIFT)&255)>>2)<<5)
         | ((((rgb888>>EADXT_BLUESHIFT)&255)>>3)   );
}

static inline unsigned int Alpha8to4(unsigned int a8)
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
static unsigned int bitmapper[8]={1,7,6,5,4,3,2,0};
static inline unsigned int Alpha8to3(unsigned int a8, unsigned int mina, unsigned int maxa)
{
    if (maxa==mina)
        return 0;

    return bitmapper[(((a8-mina)*7+(maxa-mina+1)/2)/(maxa-mina))];
}

static inline unsigned int diff_channelex(unsigned int c0, unsigned int c1)
{
    int d = (int)c0-(int)c1;
    return (unsigned int)(d*d);
}

static inline unsigned int diff_channel(unsigned int p0, unsigned int p1, unsigned int shiftval)
{
    unsigned int c0 = ((p0>>shiftval)&255);
    unsigned int c1 = ((p1>>shiftval)&255);
    return diff_channelex(c0,c1);
}



/* put intel memory */

static __inline void dxt_puti(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
    }
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[3] = static_cast<unsigned char>(data>>24);
    }
    else if (bytes==3)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>16);
    }
}

/* put motorolla memory */

static __inline void dxt_putm(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
    }
#if defined(__APPLE__) || (defined(__MWERKS__) && defined(__PPCGEKKO__))
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned short *>(dst))[0] = static_cast<unsigned short>(data);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned int *>(dst))[0] = static_cast<unsigned int>(data);
    }
#else
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>24);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[3] = static_cast<unsigned char>(data);
    }
#endif
    else if (bytes==3)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data);
    }
}

static inline unsigned int diffpixels2(unsigned int p0, unsigned int p1)
{
    return diff_channel(p0,p1,EADXT_REDSHIFT   )
         + diff_channel(p0,p1,EADXT_GREENSHIFT )
         + diff_channel(p0,p1,EADXT_BLUESHIFT  );
}

static inline unsigned int diffpixelstransparent(unsigned int p0, unsigned int p1)
{
    return diff_channel(p0,p1,EADXT_ALPHASHIFT   )
         + diff_channel(p0,p1,EADXT_REDSHIFT   )
         + diff_channel(p0,p1,EADXT_GREENSHIFT )
         + diff_channel(p0,p1,EADXT_BLUESHIFT  );
}

static inline unsigned int mappixel(unsigned int pixel, unsigned int *bits2,unsigned int *p)
{
    unsigned int bestdiff=0x7fffffff;
    unsigned int besti=0;

    for (unsigned int i=0; i<4; ++i)
    {
        unsigned int diff = diffpixels2(pixel,p[i]);
        if (diff<bestdiff)
        {
            bestdiff = diff;
            besti=i;
        }
    }
    *bits2 = besti;
    return bestdiff;
}

// mapper for dxt1 where alpha needs to be respected

static inline unsigned int mappixeltransparent(unsigned int pixel, unsigned int *bits2,unsigned int *p)
{
    unsigned int bestdiff=0x7fffffff;
    unsigned int besti=0;

    for (unsigned int i=0; i<4; ++i)
    {
        unsigned int diff = diffpixelstransparent(pixel,p[i]);
        if (diff<bestdiff)
        {
            bestdiff = diff;
            besti=i;
        }
    }
    *bits2 = besti;
    return bestdiff;
}


static inline unsigned int trymappixel(unsigned int pixel, unsigned int *p)
{
    unsigned int bestdiff = diffpixels2(pixel,p[0]);

    for (unsigned int i=1; i<4; ++i)
    {
        unsigned int diff = diffpixels2(pixel,p[i]);
        if (diff<bestdiff)
        {
            bestdiff = diff;
        }
    }
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

static inline unsigned int channelmappixel(unsigned int pixel, unsigned int *p)
{
    unsigned int bestdiff=0x7fffffff;

    for (unsigned int i=0; i<4; ++i)
    {
        unsigned int diff = diff_channelex(pixel,p[i]);
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

// TRYLIST list to keep track of colour combos we've already tried.

#ifdef ENABLE_TRYLIST

#define TRYLISTMAX 20
typedef struct TRYLIST
{
    int trylistlen;
    int trycount;
    unsigned int trylist[TRYLISTMAX];
} TRYLIST;

static inline void TRYLIST_clear(TRYLIST *trylist)
{
    trylist->trylistlen = 0;
    trylist->trycount = 0;
}

static inline void TRYLIST_add(TRYLIST *trylist, unsigned int c0, unsigned int c1)
{
    unsigned int tc = (c0<<16)|c1;
    if (trylist->trylistlen<TRYLISTMAX)
    {
        trylist->trylist[trylist->trylistlen]=tc;
        ++trylist->trylistlen;
    }
    ++trylist->trycount;
}

static inline bool TRYLIST_check(TRYLIST *trylist, unsigned int c0, unsigned int c1)
{
    unsigned int tc = (c0<<16)|c1;
    for (int i=0; i<trylist->trylistlen; ++i)
    {
        if (trylist->trylist[i]==tc)
            return true;
    }
    return false;
}
#else
#define TRYLIST_clear(t)
#define TRYLIST_add(t,c0,c1)
#define TRYLIST_check(t,c0,c1) false
#define TRYLIST void
#endif

#define FIXTHIRD 65536u
#define ONETHIRD 21845u
#define TWOTHIRD 43691u
#if 0
#define FIXTHIRD 3
#define ONETHIRD 1
#define TWOTHIRD 2
#endif


static inline unsigned int interpolate_channel(unsigned int *p, unsigned int shiftval, int i1,int i2,int d)
{
    unsigned int c0 = ((p[0]>>shiftval)&255);
    unsigned int c1 = ((p[1]>>shiftval)&255);
    return ((c0*i1 + c1*i2)/d) << shiftval;
}

template <bool Value>
static inline unsigned int interpolate(unsigned int *p,int i1,int i2,int d)
{
    return (0xff<<EADXT_ALPHASHIFT)
         | interpolate_channel(p,EADXT_REDSHIFT,i1,i2,d)
         | interpolate_channel(p,EADXT_GREENSHIFT,i1,i2,d)
         | interpolate_channel(p,EADXT_BLUESHIFT,i1,i2,d);

}


static inline unsigned int interpolatechannel(unsigned int *p, int i1,int i2,int d)
{
    unsigned int c0 = p[0];
    unsigned int c1 = p[1];
    return ((c0*i1 + c1*i2)/d);
}

// map one channel
static unsigned int trymapchannel5(unsigned char *srcBlock, unsigned int c0, unsigned int c1)
{
    unsigned int p[4];
    p[0] = channel5to8(c0);
    p[1] = channel5to8(c1);
    p[2] = interpolatechannel(p,TWOTHIRD,ONETHIRD,FIXTHIRD);
    p[3] = interpolatechannel(p,ONETHIRD,TWOTHIRD,FIXTHIRD);

    unsigned int diff=0;
    for (int y=0; y<16; ++y)
    {
        diff += channelmappixel((unsigned int)*srcBlock,p);
        srcBlock += sizeof(ARGB);
    }
    return diff;
}

// map one channel
static unsigned int trymapchannel6(unsigned char *srcBlock, unsigned int c0, unsigned int c1)
{
    unsigned int p[4];
    p[0] = channel6to8(c0);
    p[1] = channel6to8(c1);
    p[2] = interpolatechannel(p,TWOTHIRD,ONETHIRD,FIXTHIRD);
    p[3] = interpolatechannel(p,ONETHIRD,TWOTHIRD,FIXTHIRD);

    unsigned int diff=0;
    for (int y=0; y<16; ++y)
    {
        diff += channelmappixel((unsigned int)*srcBlock,p);
        srcBlock += sizeof(ARGB);
    }
    return diff;
}


static unsigned int trymapblock(TRYLIST *trylist, ARGB *srcBlock, unsigned int c0, unsigned int c1, unsigned int bestdiff)
{
#ifdef ENABLE_TRYLIST
    if (TRYLIST_check(trylist,c0,c1))
        return 0x7fffffff;
    TRYLIST_add(trylist,c0,c1);
#endif

    unsigned int p[4];
    p[0] = c5to8(c0);
    p[1] = c5to8(c1);
    if (c0>c1)
    {
        p[2] = interpolate<true>(p,TWOTHIRD,ONETHIRD,FIXTHIRD);
        p[3] = interpolate<true>(p,ONETHIRD,TWOTHIRD,FIXTHIRD);
    }
    else
    {
        p[2] = interpolate<true>(p,1,1,2);
#ifdef DXTUSEBLACK
        p[3] = 0xff000000;
#else
        p[3] = 0;
#endif
    }
    unsigned int diff=0;

    unsigned int *sourcerow = ((unsigned int *) srcBlock);

    for (int i=0; i<16; ++i)
    {
        diff += trymappixel(*sourcerow,p);
        if (diff>bestdiff) return diff;
        ++sourcerow;
    }

    return diff;
}


static unsigned int mapblock(void* srcBlock, void* destBlock, unsigned int c0, unsigned int c1, unsigned int , int mode)
{
    unsigned int p[4];
    p[0] = c5to8(c0);
    p[1] = c5to8(c1);
    if (c0>c1)
    {
        p[2] = interpolate<true>(p,TWOTHIRD,ONETHIRD,FIXTHIRD);
        p[3] = interpolate<true>(p,ONETHIRD,TWOTHIRD,FIXTHIRD);
    }
    else
    {
        p[2] = interpolate<true>(p,1,1,2);
#ifdef DXTUSEBLACK
        p[3] = 0xff000000;
#else
        p[3] = 0;
#endif
    }
    unsigned short *ss = (unsigned short *)destBlock;
    if (mode&EADXT_MODE_ENDIAN_WII)
    {
        dxt_putm(  &ss[0],  c0, 2 );
        dxt_putm(  &ss[1],  c1, 2 );
    }
    else
    {
        dxt_puti(  &ss[0],  c0, 2 );
        dxt_puti(  &ss[1],  c1, 2 );
    }
    unsigned int diff=0;

    unsigned int *sourcerow = ((unsigned int *) srcBlock);

    if (mode&EADXT_MODE_ENDIAN_WII)
    {
        if (mode&(EADXT_MODE_ALPHA_INTERPOLATED|EADXT_MODE_ALPHA_EXPLICIT)) //dxt3/5
        {
            for (int y=0; y<4; ++y)
            {
                unsigned int bits=0;
                diff += mappixel(sourcerow[3],&bits,p);

                unsigned int bits2=0;
                diff += mappixel(sourcerow[2],&bits2,p);
                bits |= (bits2<<2);

                diff += mappixel(sourcerow[1],&bits2,p);
                bits |= (bits2<<4);

                diff += mappixel(sourcerow[0],&bits2,p);
                bits |= (bits2<<6);

                sourcerow += 4;
                ((unsigned char *)destBlock+4)[y] = (unsigned char)bits;
            }
        }
        else //dxt1
        {
            for (int y=0; y<4; ++y)
            {
                unsigned int bits=0;
                diff += mappixeltransparent(sourcerow[3],&bits,p);

                unsigned int bits2=0;
                diff += mappixeltransparent(sourcerow[2],&bits2,p);
                bits |= (bits2<<2);

                diff += mappixeltransparent(sourcerow[1],&bits2,p);
                bits |= (bits2<<4);

                diff += mappixeltransparent(sourcerow[0],&bits2,p);
                bits |= (bits2<<6);

                sourcerow += 4;
                ((unsigned char *)destBlock+4)[y] = (unsigned char)bits;
            }
        }
    }
    else
    {
        if (mode&(EADXT_MODE_ALPHA_INTERPOLATED|EADXT_MODE_ALPHA_EXPLICIT)) //dxt3/5
        {
            for (int y=0; y<4; ++y)
            {
                unsigned int bits=0;
                diff += mappixel(sourcerow[0],&bits,p);

                unsigned int bits2=0;
                diff += mappixel(sourcerow[1],&bits2,p);
                bits |= (bits2<<2);

                diff += mappixel(sourcerow[2],&bits2,p);
                bits |= (bits2<<4);

                diff += mappixel(sourcerow[3],&bits2,p);
                bits |= (bits2<<6);

                sourcerow += 4;
                ((unsigned char *)destBlock+4)[y] = (unsigned char)bits;
            }
        }
        else //dxt1
        {
            for (int y=0; y<4; ++y)
            {
                unsigned int bits=0;
                diff += mappixeltransparent(sourcerow[0],&bits,p);

                unsigned int bits2=0;
                diff += mappixeltransparent(sourcerow[1],&bits2,p);
                bits |= (bits2<<2);

                diff += mappixeltransparent(sourcerow[2],&bits2,p);
                bits |= (bits2<<4);

                diff += mappixeltransparent(sourcerow[3],&bits2,p);
                bits |= (bits2<<6);

                sourcerow += 4;
                ((unsigned char *)destBlock+4)[y] = (unsigned char)bits;
            }
        }
    }

    return diff;
}


static unsigned int findendpoints( ARGB *srcBlock, unsigned int *bestc0ptr, unsigned int *bestc1ptr, unsigned int *colourcountptr, int /*mode*/, int quality )
{
    /******************************************************/
    /** Convert Pixels                                   **/
    /******************************************************/

    unsigned int c565table[16];
    unsigned int c565count=0;

    for (int i=0; i<16; ++i)
    {
        unsigned int p = ((unsigned int *)srcBlock)[i];

        unsigned int c565 = c8to5(p);
        // insert into 565 table sorted by value
        unsigned int j=0;
        while (j<c565count)
        {
            if (c565==c565table[j])
                break;
            if (c565<c565table[j])
            {
                 unsigned int t = c565table[j];
                 c565table[j] = c565;
                 c565 = t;
            }
            ++j;
        }
        if (j>=c565count)
             c565table[c565count++] = c565;
    }

    /******************************************************/
    /** Seed with min/max                                **/
    /******************************************************/

    unsigned int bestc0 = c565table[c565count-1];
    unsigned int bestc1 = c565table[0];
    unsigned int bestdiff = 0x7fffffff;

    if (c565count==1 && (bestc0==0 || bestc0==0xffff || quality<=20))
    {
        goto breakout;
    }


    if (quality>=1)
    {
    #ifdef ENABLE_TRYLIST
        TRYLIST trylist;
        TRYLIST_clear(&trylist);
    #else
        int trylist=0;
    #endif

        unsigned int goodenoughdiff = 4;
        if (quality>=40) goodenoughdiff = 3;
        if (quality>=60) goodenoughdiff = 1;
        if (quality>=70) goodenoughdiff = 0;

        /******************************************************/
        /** Search all combinations of colours in 4x4        **/
        /******************************************************/

        // seed with all colour combinations in the 4x4.  This is a reasonally good solution
        for (unsigned int i0=0; i0<c565count; ++i0)
        {
            unsigned int c1 = c565table[i0];

            for (int i1=((int)c565count-1); i1>(int)i0; --i1)
            {
                unsigned int c0 = c565table[i1];

                unsigned int diff = trymapblock(&trylist,srcBlock, c0, c1, bestdiff);
                if (diff<bestdiff)
                {
                    bestdiff = diff;
                    bestc0 = c0;
                    bestc1 = c1;
                    if (bestdiff<=goodenoughdiff) goto breakout;
                }
                if (quality<20 && (trylist.trycount >= (quality+1))) goto breakout;
            }
        }

        /******************************************************/
        /** Refine search                                    **/
        /******************************************************/

        if (quality>=20 && bestdiff)
        {
            /******************************************************/
            /** Find bounding box                                **/
            /******************************************************/

            ARGB minrgb={255,255,255,255};
            ARGB maxrgb={0,0,0,0};

            for (int i=0; i<16; ++i)
            {
                minrgb.a = gmin(minrgb.a,srcBlock[i].a);
                minrgb.r = gmin(minrgb.r,srcBlock[i].r);
                minrgb.g = gmin(minrgb.g,srcBlock[i].g);
                minrgb.b = gmin(minrgb.b,srcBlock[i].b);
                maxrgb.a = gmax(maxrgb.a,srcBlock[i].a);
                maxrgb.r = gmax(maxrgb.r,srcBlock[i].r);
                maxrgb.g = gmax(maxrgb.g,srcBlock[i].g);
                maxrgb.b = gmax(maxrgb.b,srcBlock[i].b);
            }


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

                fullcube_minrgb.a = (unsigned char)gmax(0,minrgb.a-(maxrgb.a-minrgb.a));
                fullcube_minrgb.r = (unsigned char)gmax(0,minrgb.r-(maxrgb.r-minrgb.r));
                fullcube_minrgb.g = (unsigned char)gmax(0,minrgb.g-(maxrgb.g-minrgb.g));
                fullcube_minrgb.b = (unsigned char)gmax(0,minrgb.b-(maxrgb.b-minrgb.b));

                fullcube_maxrgb.a = (unsigned char)gmin(255,maxrgb.a+(maxrgb.a-minrgb.a));
                fullcube_maxrgb.r = (unsigned char)gmin(255,maxrgb.r+(maxrgb.r-minrgb.r));
                fullcube_maxrgb.g = (unsigned char)gmin(255,maxrgb.g+(maxrgb.g-minrgb.g));
                fullcube_maxrgb.b = (unsigned char)gmin(255,maxrgb.b+(maxrgb.b-minrgb.b));

                // extend to allow interpolated values to be found
                fullcube_minc5.r = (unsigned char)gmax(0,(fullcube_minrgb.r>>3)-1);
                fullcube_minc5.g = (unsigned char)gmax(0,(fullcube_minrgb.g>>2)-1);
                fullcube_minc5.b = (unsigned char)gmax(0,(fullcube_minrgb.b>>3)-1);
                fullcube_maxc5.r = (unsigned char)gmin(31,(fullcube_maxrgb.r>>3)+2);
                fullcube_maxc5.g = (unsigned char)gmin(63,(fullcube_maxrgb.g>>2)+2);
                fullcube_maxc5.b = (unsigned char)gmin(31,(fullcube_maxrgb.b>>3)+2);

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
                search0_minc5.r = (unsigned char)gmax(0,gmax((int)fullcube_minc5.r,(int)((bestc0>>11)&31)-distdown));
                search0_minc5.g = (unsigned char)gmax(0,gmax((int)fullcube_minc5.g,(int)((bestc0>>5)&63)-distdown));
                search0_minc5.b = (unsigned char)gmax(0,gmax((int)fullcube_minc5.b,(int)((bestc0)&31)-distdown));
                search0_maxc5.r = (unsigned char)gmin(31,gmin((int)fullcube_maxc5.r,(int)((bestc0>>11)&31)+distup));
                search0_maxc5.g = (unsigned char)gmin(63,gmin((int)fullcube_maxc5.g,(int)((bestc0>>5)&63)+distup));
                search0_maxc5.b = (unsigned char)gmin(31,gmin((int)fullcube_maxc5.b,(int)((bestc0)&31)+distup));

                search1_minc5.r = (unsigned char)gmax(0,gmax((int)fullcube_minc5.r,(int)((bestc1>>11)&31)-distdown));
                search1_minc5.g = (unsigned char)gmax(0,gmax((int)fullcube_minc5.g,(int)((bestc1>>5)&63)-distdown));
                search1_minc5.b = (unsigned char)gmax(0,gmax((int)fullcube_minc5.b,(int)((bestc1)&31)-distdown));
                search1_maxc5.r = (unsigned char)gmin(31,gmin((int)fullcube_maxc5.r,(int)((bestc1>>11)&31)+distup));
                search1_maxc5.g = (unsigned char)gmin(63,gmin((int)fullcube_maxc5.g,(int)((bestc1>>5)&63)+distup));
                search1_maxc5.b = (unsigned char)gmin(31,gmin((int)fullcube_maxc5.b,(int)((bestc1)&31)+distup));
            }

    /******************************************************/
    /** Refine Method A: one end at a time               **/
    /******************************************************/

            if (quality>=20)
            {
                /* adjust c0 */
                for (unsigned int r0=search0_minc5.r; r0<=search0_maxc5.r; r0+=1)
                {
                    for (unsigned int b0=search0_minc5.b; b0<=search0_maxc5.b; b0+=1)
                    {
                        for (unsigned int g0=search0_minc5.g; g0<=search0_maxc5.g; g0+=1)
                        {
                            unsigned int c0=(r0<<11)|(g0<<5)|b0;

                            if (c0>bestc1) // only consider 1/3 2/3 and unique values.
                            {
                                unsigned int diff = trymapblock(&trylist,srcBlock, c0, bestc1, bestdiff);

                                if (diff<bestdiff)
                                {
                                    bestdiff = diff;
                                    bestc0 = c0;
                                    if (bestdiff<=goodenoughdiff) goto breakout;
                                }
                            }
                        }
                    }
                }
            }


            if (quality>=25)
            {
                /* adjust c1 */
                for (unsigned int r1=search1_minc5.r; r1<=search1_maxc5.r; r1+=1)
                {
                    for (unsigned int b1=search1_minc5.b; b1<=search1_maxc5.b; b1+=1)
                    {
                        for (unsigned int g1=search1_minc5.g; g1<=search1_maxc5.g; g1+=1)
                        {
                            unsigned int c1=(r1<<11)|(g1<<5)|b1;

                            if (bestc0>c1) // only consider 1/3 2/3 and unique values.
                            {
                                unsigned int diff = trymapblock(&trylist,srcBlock, bestc0, c1, bestdiff);

                                if (diff<bestdiff)
                                {
                                    bestdiff = diff;
                                    bestc1 = c1;
                                    if (bestdiff<=goodenoughdiff) goto breakout;
                                }
                            }
                        }
                    }
                }
            }

    /******************************************************/
    /** Refine Method B: both ends all combos            **/
    /******************************************************/

            #define THRESHHOLD 16

            if (quality>=30)
            {
                for (int g1=search1_minc5.g; g1<=search1_maxc5.g; g1+=1)
                {
                    for (int g0=search0_maxc5.g; g0>=search0_minc5.g; g0-=1)
                    {
                        unsigned int gdiff = trymapchannel6(&srcBlock[0].g, (unsigned int) g0, (unsigned int) g1);
                        if (gdiff<bestdiff)
                        {
                            for (int r1=search1_minc5.r; r1<=search1_maxc5.r; r1+=1)
                            {
                                for (int r0=search1_maxc5.r; r0>=search1_minc5.r; r0-=1)
                                {
                                    if (r0>r1)
                                    {
                                        unsigned int rdiff = trymapchannel5(&srcBlock[0].r, (unsigned int) r0, (unsigned int) r1);
                                        unsigned int cdiff = gdiff+rdiff;
                                        if (cdiff<bestdiff)
                                        {
                                            for (int b1=search1_minc5.b; b1<=search1_maxc5.b; b1+=1)
                                            {
                                                for (int b0=search0_maxc5.b; b0>=search0_minc5.b; b0-=1)
                                                {
//                                                    unsigned int bdiff = trymapchannel5(&srcBlock[0].b, b0, b1);
//                                                    cdiff = gdiff+rdiff+bdiff;
//                                                    if (cdiff<bestdiff)
                                                    {
                                                        unsigned int c0=(unsigned int)((r0<<11)|(g0<<5)|b0);
                                                        unsigned int c1=(unsigned int)((r1<<11)|(g1<<5)|b1);
                                                        if (c0>c1)
                                                        {
                                                            unsigned int  diff = trymapblock(&trylist,srcBlock, c0, c1, bestdiff);

															if (diff<bestdiff)
		                                                    {
																bestdiff = diff;
																bestc0 = c0;
																bestc1 = c1;
																if (bestdiff<=goodenoughdiff) goto breakout;
															}
	                                                        else
	                                                            if ((diff-bestdiff)>THRESHHOLD) b0-=1;
														}
                                                    }
                                                }
                                            }
                                        }
//                                        else
//                                            if ((cdiff-bestdiff)>THRESHHOLD*THRESHHOLD*THRESHHOLD) r0-=1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (quality==100) goto breakout;

        }
    }

#if 0



            #define THRESHHOLD 16

            if (quality>=30)
            {
                for (int g1=search1_minc5.g; g1<=search1_maxc5.g; g1+=1)
                {
                    for (int g0=search0_maxc5.g; g0>=search0_minc5.g; g0-=1)
                    {
                        unsigned int diff = trymapchannel6(&srcBlock[0].g, g0, g1);
//                        if (diff<bestdiff)
                        {
                            for (int r1=search1_minc5.r; r1<=search1_maxc5.r; r1+=1)
                            {
                                for (int r0=search1_maxc5.r; r0>=search1_minc5.r; r0-=1)
                                {
//                                    if (r0>r1)
                                    {
                                        diff += trymapchannel5(&srcBlock[0].r, r0, r1);
//                                        if (diff<bestdiff)
                                        {
                                            for (int b1=search1_minc5.b; b1<=search1_maxc5.b; b1+=1)
                                            {
                                                for (int b0=search0_maxc5.b; b0>=search0_minc5.b; b0-=1)
                                                {
                                                    diff += trymapchannel5(&srcBlock[0].b, b0, b1);

//                                                    if (diff<bestdiff)
                                                    {
                                                        unsigned int c0=(r0<<11)|(g0<<5)|b0;
                                                        unsigned int c1=(r1<<11)|(g1<<5)|b1;
                                                        if (c0>c1)
                                                        {
                                                            unsigned int diff = trymapblock(&trylist,srcBlock, c0, c1, bestdiff);
															if (diff<bestdiff)
		                                                    {
																bestdiff = diff;
																bestc0 = c0;
																bestc1 = c1;
//																if (bestdiff<=goodenoughdiff) goto breakout;
															}
//	                                                        else
//	                                                            if ((diff-bestdiff)>THRESHHOLD) b0-=1;
														}
                                                    }
                                                }
                                            }
                                        }
//                                        else
//                                            if ((diff-bestdiff)>THRESHHOLD*THRESHHOLD*THRESHHOLD) r0-=1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (quality==100) goto breakout;
        }
    }
#endif


breakout:

#if 0
    // swap if we need transparent
    if (((mode & EADXT_MODE_FORMAT_MASK) == 0) &&
		(minrgb.a<128) &&
		(bestc0>bestc1)) // dxt1 and has transparent
    {
        int t=bestc0;
        bestc0 = bestc1;
        bestc1 = t;
    }
#endif

    *bestc0ptr = bestc0;
    *bestc1ptr = bestc1;
    *colourcountptr = c565count;
    return bestdiff;
}

static void ExpliciteAlphaBlock( unsigned char *s, void* destBlock)
{
    unsigned char *d=(unsigned char *) destBlock;
    for (int i=0; i<16; i+=2)
    {
        *d = (unsigned char)(Alpha8to4(s[0]) | (Alpha8to4(s[4])<<4));
        s += (sizeof(ARGB)*2);
        d += 1;
    }
}

static void InterpolatedAlphaBlock( unsigned char *srcBlock,void* destBlock )
{
    unsigned short *d=(unsigned short *) destBlock;
    unsigned char *s = srcBlock;
    unsigned int mina=255, maxa=0;

    for (int i=0; i<16; ++i)
    {
        unsigned int a=s[0];
        mina = gmin(mina,a);
        maxa = gmax(maxa,a);
        s += sizeof(ARGB);
    }

    s = srcBlock;
    unsigned int r[4];
    for (int y=0; y<4; ++y)
    {
        r[y] = Alpha8to3(s[0],mina,maxa)
            | (Alpha8to3(s[4],mina,maxa)<<3)
            | (Alpha8to3(s[8],mina,maxa)<<6)
            | (Alpha8to3(s[12],mina,maxa)<<9);

        s += (sizeof(ARGB)*4);
    }

    ((unsigned char *)d)[0] = (unsigned char)(maxa);
    ((unsigned char *)d)[1] = (unsigned char)(mina);
    ++d;
    dxt_puti(  &d[0],  r[0]     | ((r[1]&15)<<12) , 2);  // 12 bits r0 and 4 bits r1
    dxt_puti(  &d[1], (r[1]>>4) | ((r[2]&255)<<8) , 2);  // 8 bits r1 and 8 bits r2
    dxt_puti(  &d[2], (r[2]>>8) | ((r[3])<<4)     , 2);  // 4 bits r2 and 12 bits r3
}


/**
 * eadxt
 * ------------
 * Dxt compresses the given data (pixel image), dataIn, and places the compressed data into dataOut.
 *
 * parameters: dataIn  - the data to compress
 *             dataOut - the location in which to store the compressed data
 *             numblocks - the number of 4x4 blocks in the image
 *
 * input block format (512 bits, 64 bytes)
 *     16 pixels, each 32 bits in length (1 byte for each of a, r, g, b)
 *
 * output block format (64 bits, 8 bytes):
 *     16 bit - colour 1
 *     16 bit - colour 2
 *     32 bit - colour indices (16 indices that are 2 bits each)
 *
 * Written by: Frank Barchard
 * Initial date: January, 2007
 */

// dxt needs all shorts swapped first

static void SwapOne(unsigned char *framebuf)
{
    unsigned int i=*(unsigned int *)framebuf;
    *(unsigned int *)framebuf = ((i&0xff00ff00)>>8) | ((i&0x00ff00ff)<<8);
}

static void ShortSwap(unsigned char *framebuf)
{
    SwapOne(framebuf);
    SwapOne(framebuf+4);
}

unsigned int eadxt(void *dataOut, const void *dataIn, int rowbytes, int numblocks, int mode, int quality)
{
    unsigned char* curOutput = (unsigned char *)dataOut;  // pointer to the current 64 bit (8 byte) output block
    unsigned char* curInput =  (unsigned char *)dataIn;   // pointer to the current 4x4 input block
    ARGB newSrc[16];

    unsigned int error=0;
    for ( int i = 0; i < numblocks; ++i )
    {
        // Copy Source
        {
            ARGB *s = (ARGB *)curInput;
            int j=0;
            for (int y=0; y<4; ++y)
            {
                for (int x=0; x<4; ++x)
                    *(unsigned int *)&(newSrc[j++]) = ((unsigned int *)s)[x];

                s = (ARGB *)((char *)s+rowbytes);
            }
        }

        if (mode & EADXT_MODE_ALPHA_EXPLICIT) // dxt3 4 bit alpha
        {
            ExpliciteAlphaBlock( &newSrc[0].a, curOutput );
            if (mode & EADXT_MODE_ENDIAN_XENON)
                ShortSwap(curOutput);
            curOutput += 8;
        }
        else if (mode & EADXT_MODE_ALPHA_INTERPOLATED)// dxt5 alpha (8 shades)
        {
            InterpolatedAlphaBlock( &newSrc[0].a, curOutput );
            if (mode & EADXT_MODE_ENDIAN_XENON)
                ShortSwap(curOutput);
            curOutput += 8;
        }

        if (mode & EADXT_MODE_ALPHA_INTERPOLATED_FROM_RED)// dxt5 alpha from red channel.  Use this plus alpha_interpolated for DXN
        {
            InterpolatedAlphaBlock( &newSrc[0].r, curOutput );
            if (mode & EADXT_MODE_ENDIAN_XENON)
                ShortSwap(curOutput);
            curOutput += 8;
        }

        if (~mode & EADXT_MODE_RGB_NONE)
        {
            unsigned int bestc0, bestc1, colourcount;
            error += findendpoints( newSrc, &bestc0, &bestc1, &colourcount, mode, quality );

            mapblock((unsigned int *)newSrc, curOutput, bestc0, bestc1, colourcount, mode);
            if (mode & EADXT_MODE_ENDIAN_XENON)
                ShortSwap(curOutput);

            curOutput += 8;
        }
        curInput += 4*4; // advance 4 pixels
    }
    return error;
}

// this function provided as an example of eadxt api.
// do a full image (or rectangle of one).  This is provided more for ease/example.
unsigned int eadxtimage(void *dataOut, int rowoutbytes, const void *dataIn, int width, int height, int rowbytes, int mode, int quality)
{
	unsigned int error = 0;

    for (int y=0; y<height/4; ++y)
    {
        char *rowout = (char *) dataOut + (rowoutbytes * y);
        char *rowin = (char *) dataIn + (rowbytes * y * 4);
        error += eadxt(rowout, rowin, rowbytes, width/4, mode, quality);
    }

	return error;
}

#if defined(_OPENMP)
#include "omp.h"
// this function provided as an example of eadxt api.
// do a full image (or rectangle of one).  This is provided more for ease/example.
void eadxtimageOpenMP(void *dataOut, int rowoutbytes, const void *dataIn, int width, int height, int rowbytes, int mode, int quality)
{
    #pragma omp parallel for
    for (int y=0; y<height/4; ++y)
    {
        char *rowout = (char *) dataOut + (rowoutbytes * y);
        char *rowin = (char *) dataIn + (rowbytes * y * 4);
        eadxt(rowout, rowin, rowbytes, width/4, mode, quality);
    }

	return;
}
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
