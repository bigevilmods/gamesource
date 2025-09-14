// (c) 2000-2007 Electronic Arts Inc.


/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  Mipmont - Morten Mipmap Montage                 */
/*                                                                  */
/*             by Morten Pedersen and Ian Davids, EAC               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 000509 FB based on fhalfsize and Morten's algorithm      */
/*    1.01 000511 FB page size based on bpp                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* mipmont()                                                        */
/*                                                                  */
/* Mipmont breaks up an image into ~64x64 pages and lays them out   */
/* vertically.  It them mipmaps and tacks those on the end.  The    */
/* small mipmaps fit into a final 64x32, layed out horizontally     */
/*                                                                  */
/* The page size is as follows                                      */
/*  32 bit   64 x  32                                               */
/*  16 bit   64 x  64                                               */
/*   8 bit  128 x  64                                               */
/*   4 bit  128 x 128                                               */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fmipmont.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define ALIGNTO(x,a) (((x)+((a)-1))&~((a)-1))
#define ISPOWER2(v) (!((v)&((v)-1)))

// Page sizes for the standard bit depths
#define PAGEWIDTH32    64
#define PAGEHEIGHT32   32
#define PAGEWIDTH16    64
#define PAGEHEIGHT16   64
#define PAGEWIDTH8    128
#define PAGEHEIGHT8    64
#define PAGEWIDTH4    128
#define PAGEHEIGHT4   128

#define BLOCKWIDTH32    8
#define BLOCKHEIGHT32   8
#define BLOCKWIDTH16   16
#define BLOCKHEIGHT16   8
#define  BLOCKWIDTH8   16
#define  BLOCKHEIGHT8  16
#define  BLOCKWIDTH4   32
#define  BLOCKHEIGHT4  16

#define NUMCOLS4        4
#define NUMCOLS8        8
#define NUMCOLS16       4
#define NUMCOLS32       8

int blocks4[] = { 0, 2, 8,10,
                  1, 3, 9,11,
                  4, 6,12,14,
                  5, 7,13,15,
                 16,18,24,26,
                 17,19,25,27,
                 20,21,28,30,
                 22,23,29,31};

int blocks8[] = { 0, 1, 4, 5,16,17,20,21,
                  2, 3, 6, 7,18,19,22,23,
                  8, 9,12,13,24,25,28,29,
                 10,11,14,15,26,27,30,31};

int blocks16[] = { 0, 2, 8,10,
                   1, 3, 9,11,
                   4, 6,12,14,
                   5, 7,13,15,
                  16,18,24,26,
                  17,19,25,27,
                  20,22,28,30,
                  21,23,29,31};

int blocks32[] = { 0, 1, 4, 5,16,17,20,21,
                   2, 3, 6, 7,18,19,22,23,
                   8, 9,12,13,24,25,28,29,
                  10,11,14,15,26,27,30,31};

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/


#define SQR(x) (x)*(x)

static unsigned char MIPMONT_getcolour(ARGB *rgbval, int numcolours, ARGB *colourtbl)
{
    int i, BestDiff=0x7fffffff, best=0;
    int finda = (int) rgbval->a;
    int findr = (int) rgbval->r*finda>>8;
    int findg = (int) rgbval->g*finda>>8;
    int findb = (int) rgbval->b*finda>>8;

    for (i = 0; (i < numcolours) && (BestDiff != 0); ++i)
    {
        int a = (int)colourtbl[i].a;
        int r = (int)colourtbl[i].r*a>>8;
        int g = (int)colourtbl[i].g*a>>8;
        int b = (int)colourtbl[i].b*a>>8;
        int Diff = (finda-a)*4;
        if (Diff>=0) Diff *= 4;
        Diff = SQR(Diff)+SQR(findr-r)+SQR(findg-g)+SQR(findb-b);
        if (Diff < BestDiff)
        {
            BestDiff = Diff;
            best = i;
        }
    }

    return((unsigned char)best);
}

#undef SQR

#define BLEND2(dest,source,source2) *(unsigned char *)(dest) = (unsigned char) ((int)(*(unsigned char *)(source) + (int)*(unsigned char *)(source2))>>1)

static void MIPMONT_blend2(char *dest, const char *source,const char *source2,int bpp, int pixeladvance,int numpixels,int numcolours,ARGB *colourtbl)
{
    int i;

    for (i=0; i<numpixels; ++i)
    {
        if (bpp>=4)
        {
            int a=((ARGB *)source)->a;
            int a2=((ARGB *)source2)->a;
            if (a<=a2 && a<=64)
            {
                dest[0] = source2[0];
                dest[1] = source2[1];
                dest[2] = source2[2];
                dest[3] = source2[3];
            }
            else if (a2<=a && a2<=64)
            {
                dest[0] = source[0];
                dest[1] = source[1];
                dest[2] = source[2];
                dest[3] = source[3];
            }
            else
            {
                BLEND2(dest+0,source+0,source2+0);
                BLEND2(dest+1,source+1,source2+1);
                BLEND2(dest+2,source+2,source2+2);
                BLEND2(dest+3,source+3,source2+3);
            }
            dest += 4;
        }
        else
        {
            if (colourtbl[*(unsigned char *)source].a<=16)
            {
                dest[0] = source2[0];
            }
            else if (colourtbl[*(unsigned char *)source2].a<=16)
            {
                dest[0] = source[0];
            }
            else
            {
                char bcolour[4];

                BLEND2(bcolour+0,(char *)&colourtbl[*(unsigned char *)source]+0,(char *)&colourtbl[*(unsigned char *)source2]+0);
                BLEND2(bcolour+1,(char *)&colourtbl[*(unsigned char *)source]+1,(char *)&colourtbl[*(unsigned char *)source2]+1);
                BLEND2(bcolour+2,(char *)&colourtbl[*(unsigned char *)source]+2,(char *)&colourtbl[*(unsigned char *)source2]+2);
                BLEND2(bcolour+3,(char *)&colourtbl[*(unsigned char *)source]+3,(char *)&colourtbl[*(unsigned char *)source2]+3);
                *dest = MIPMONT_getcolour((ARGB *)bcolour, numcolours, colourtbl);
            }
            ++dest;
        }
        source += pixeladvance;
        source2 += pixeladvance;
    }
}

#undef BLEND2

void MIPMONT_halfsize(char *dest,int rowbytes,const char *source,int sourcerowbytes,int width,int height,int bpp, GINFO *ginfo)
{
    int i;
    const int vstep=2;

    for (i=0; i<height; ++i)
    {
        char temp[32000];
        MIPMONT_blend2(temp,source,source+sourcerowbytes,bpp,bpp,width*2,ginfo->numcolours,ginfo->colourtbl);
        MIPMONT_blend2(dest,temp,temp+bpp,bpp,bpp*2,width,ginfo->numcolours,ginfo->colourtbl);

        source += sourcerowbytes*vstep;
        dest   += rowbytes;
    }

}

int GetPreviousOffset(GINFO * ginfo)
{
    // if this is the first mipmap I have entered
    if(ginfo->numhotspots == 1)
        return 0;
    return ginfo->hotspottbl[ginfo->numhotspots-1][0];
}


void MIPMONT_setoffset(int x,int y,GINFO * ginfo,int pagewidth,int pageheight,int blockwidth,int blockheight,int rowbytes,int isgst)
{
    int nx          = x%pagewidth;
    int ny          = y%pageheight;
    int pagestart   = ((int)(y/pageheight)*pageheight*rowbytes)+((int)(x/pagewidth))*pagewidth*ginfo->originalbpp/8;
    nx              = nx/blockwidth;
    ny              = ny/blockheight;
    int index       = 0;
    int blockoffset = 0;
    int bcols       = 0;
    int prev        = GetPreviousOffset(ginfo);

    if(isgst)
    {
        index       = (ny*NUMCOLS8+nx);
        blockoffset = blocks8[index];
        bcols       = NUMCOLS8;
        rowbytes    = (((int)(ginfo->width/pagewidth))+(ginfo->width%pagewidth?1:0))*pagewidth;
        pagestart   = ((int)(y/pageheight)*pageheight*rowbytes)+((int)(x/pagewidth))*pagewidth;
    }
    else if(ginfo->originalbpp <= 4)
    {
        index       = (ny*NUMCOLS4+nx);
        blockoffset = blocks4[index];
        bcols       = NUMCOLS4;
    }
    else if(ginfo->originalbpp <= 8)
    {
        index       = (ny*NUMCOLS8+nx);
        blockoffset = blocks8[index];
        bcols       = NUMCOLS8;
    }
    else if(ginfo->originalbpp <= 16)
    {
        index       = (ny*NUMCOLS16+nx);
        blockoffset = blocks16[index];
        bcols       = NUMCOLS16;
    }
    else if(ginfo->originalbpp <= 32)
    {
        index       = (ny*NUMCOLS32+nx);
        blockoffset = blocks32[index];
        bcols       = NUMCOLS32;
    }
        //printf("nx: %d  ny: %d\n",nx,ny);
        //printf("index: %d\n",index);
        //printf("blockoffset: %d\n",blockoffset);
        //printf("bcols: %d\n",bcols);
    ginfo->hotspottbl[ginfo->numhotspots][0] = pagestart + (blockoffset/bcols)*blockheight*pagewidth + (blockoffset%bcols)*blockwidth*blockheight;
    ginfo->hotspottbl[ginfo->numhotspots][1] = ginfo->hotspottbl[ginfo->numhotspots][0] - prev;
    ginfo->numhotspots++;
}


void MIPMONT_chop(char *dest,int rowbytes,const char *source,int sourcerowbytes,int width,int height,int bpp,int pageheight,int pagewidth,int pagesx)
{
    const char *s = source;
    char       *d = dest;
    int pagex = 0;
    int pagey = 0;

    for (int y = 0; y < height; y += pageheight)
    {
        int thispageheight= gmin(pageheight,height-y);
        for (int x = 0; x < width; x += pagewidth)
        {
            int thispagewidth = gmin(pagewidth,width-x);
            int pagebytes     = thispagewidth*bpp/8;

            s = source+y*sourcerowbytes+x*bpp/8;
            d = dest+pagey*pageheight*rowbytes+pagex*pagewidth*bpp/8;
            for (int y1 = 0; y1 < thispageheight; ++y1)
            {
               memcpy(d,s,pagebytes);
               d    += rowbytes;
               s    += sourcerowbytes;
            }
            pagex++;
            if(pagex >= pagesx)
            {
                pagex = 0;
                pagey++;
            }
        }
    }
}

char * MIPMONT_nextposition(int & x,int & y,int & width,int & height,char * dest,int rowbytes,int bpp,int pagewidth,int pageheight,int blockwidth,int blockheight,int &level,int pagesx)
{
    if(level == 0 && width > (pagewidth>>1))
    {
        y += ALIGNTO(height,blockheight);
    }
    else if(width <= (pagewidth>>1) && height >= pageheight) // handle the degenerate case where we have a tall image that spans many pages
    {
        x += ALIGNTO(width,blockwidth);
    }
    else if(width > pagewidth || height > pageheight)
    {
        int usedx = (int)ceil((double)width/pagewidth);
        int usedy = (int)ceil((double)height/pageheight);
        usedx     = usedy*usedx;
        usedy     = usedx/pagesx;
        y += usedy*pageheight;
        usedx     = usedx%pagesx;
        x += usedx*pagewidth;
    }
    else if(width > (blockwidth>>1))// if we are above the minimum decent size (8x8 or 4x4)
    {
        x += ALIGNTO(width,blockwidth);
    }
    else
    {
        x += width;
    }

    level++;
    width  >>= 1;
    height >>= 1;
    return(dest+y*rowbytes+x*bpp/8);
}


void GetWidths(int bpp,int &pagewidth,int & pageheight,int & blockwidth,int & blockheight,int isgst)
{
    if(isgst)
    {
        pagewidth   = PAGEWIDTH8;
        pageheight  = PAGEHEIGHT8;
        blockwidth  = BLOCKWIDTH8;
        blockheight = BLOCKHEIGHT8;
    }
    else if (bpp<=4)
    {
        pagewidth   = PAGEWIDTH4;
        pageheight  = PAGEHEIGHT4;
        blockwidth  = BLOCKWIDTH4;
        blockheight = BLOCKHEIGHT4;
    }
    else if (bpp<=8)
    {
        pagewidth   = PAGEWIDTH8;
        pageheight  = PAGEHEIGHT8;
        blockwidth  = BLOCKWIDTH8;
        blockheight = BLOCKHEIGHT8;
    }
    else if (bpp<=16)
    {
        pagewidth   = PAGEWIDTH16;
        pageheight  = PAGEHEIGHT16;
        blockwidth  = BLOCKWIDTH16;
        blockheight = BLOCKHEIGHT16;
    }
    else // 24 or 32
    {
        pagewidth   = PAGEWIDTH32;
        pageheight  = PAGEHEIGHT32;
        blockwidth  = BLOCKWIDTH32;
        blockheight = BLOCKHEIGHT32;
    }
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutmipmont(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('M','O','N','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00" );
        strcpy(about->namestr,      "mipmont" );
        strcpy(about->longnamestr,  "Mipmont Image" );
        about->numoptions      =         1;
        about->option[0].type  =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "IsGst");
    }
    return about;
}

FPUBLIC int openmipmont(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx = 0;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }
    *filterInstance = fx;
    return ok;
}

FPUBLIC GINFO* infomipmont(FINSTANCE* fx, int framenum)
{
    GINFO *ginfo;
    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        int width      = 0;
        int height     = 0;
        int blockwidth = 0;
        int blockheight= 0;
        int pagewidth  = 0;
        int pageheight = 0;
        int bpp        = 0;
        int isgst      = 0;
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        if(fx->numoptions >= 1) isgst = fx->option[0].i;

        bpp = ginfo->originalbpp;
        GetWidths(bpp,pagewidth,pageheight,blockwidth,blockheight,isgst);

        width         =  ALIGNTO(ginfo->width,pagewidth);
        height        =  ALIGNTO(ginfo->height,pageheight);
            //height        += ALIGNTO(height>>1,blockheight);

        int pagesx = 1;
        int pagesy = 1;
        if(ginfo->width > (pagewidth>>1) || ginfo->height > pageheight)
        {

            int size = 0;
            while (width>0 && height>0)
            {

                if (height<=pageheight && width<=(pagewidth>>1))
                {
                    size += pagewidth*height;
                    break;
                }
                size += width*height;
                width >>= 1;
                height >>= 1;
            }
            int totalpages = (int)ceil((double)size/(pagewidth*pageheight));
            pagesx         = ((int)(ginfo->width/pagewidth))+(ginfo->width%pagewidth?1:0);
            pagesy         = (int)ceil((double)totalpages/pagesx);
        }
        if(ISPOWER2(ginfo->width) && ISPOWER2(ginfo->height))
        {
            ginfo->width   =  pagesx*pagewidth;
            ginfo->height  =  pagesy*pageheight;
        }
        else
            printf("ERROR: Base texture is not a power of 2 image will not be mipmont.\n");


    }
    return(ginfo);
}

FPUBLIC int filtermipmont(FINSTANCE* fx, GINFO* ginfo, char* dest, int rowbytes)
{
    char *source       = (char *)fx->source[0].image;
    int sourcerowbytes = fx->source[0].rowbytes;
    int width          = fx->source[0].ginfo->width;
    int height         = fx->source[0].ginfo->height;
    int bpp            = ginfo->bpp;
    char *d            = dest;
    int pagewidth      = 0;
    int pageheight     = 0;
    int blockwidth     = 0;
    int blockheight    = 0;
    char *f            = 0;
    char *f0           = 0;
    int x              = 0;
    int y              = 0;
    int level          = 0;
    int pagesx         = 0;
    int isgst          = 0;

    // base texture must be a power of 2
    if(ISPOWER2(width) && ISPOWER2(height))
    {

        if(fx->numoptions >= 1) isgst = fx->option[0].i;

            // store the width and height in the first hotspot entry
        ginfo->numhotspots      = 1;
        ginfo->hotspottbl[0][0] = width;
        ginfo->hotspottbl[0][1] = height;

            // clear destination all to transparent
        {
            int dbpl           = ginfo->width*bpp/8;
            for (int i = 0; i<ginfo->height; ++i)
            {
                memset(d,0,dbpl);
                d += rowbytes;
            }
        }

        GetWidths(ginfo->originalbpp,pagewidth,pageheight,blockwidth,blockheight,isgst);
        pagesx                  = ((int)(ginfo->width/pagewidth))+(ginfo->width%pagewidth?1:0);
        int pagedwidth          = pagesx*pagewidth*ginfo->originalbpp/8;

        d = dest;
        while (width>=1 && height>=1)
        {
            MIPMONT_chop(d,rowbytes,source,sourcerowbytes,width,height,bpp,pageheight,pagewidth,pagesx);

                // don't need offset for base texture
            if(level != 0)
                MIPMONT_setoffset(x,y,ginfo,pagewidth,pageheight,blockwidth,blockheight,pagedwidth,isgst);

            d = MIPMONT_nextposition(x,y,width,height,dest,rowbytes,bpp,pagewidth,pageheight,blockwidth,blockheight,level,pagesx);


            if (width>=1 && height>=1)
            {
                f = (char *) galloc(width*bpp/8*height);

                MIPMONT_halfsize(f,width*bpp/8, source,sourcerowbytes,width,height,bpp/8, ginfo);

                if (f0)
                    gfree(f0);
                source = f0 = f;
                sourcerowbytes = width*bpp/8;
            }
        }

        if (f)
            gfree(f);
    }
    else
        memcpy(dest,source,rowbytes*height);

    return(1);
}

FPUBLIC int closemipmont(FINSTANCE* fx)
{
    return gfree((void*)fx);
}


#undef ALIGNTO
#undef ISPOWER2
