// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*            Deband - Paletted Image Debanding Filter              */
/*                                                                  */
/*                         by Yggy King, EAC                        */
/*                                                                  */
/*                    Filter Module - April 25, 1996                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960425 YK initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit only                                                       */
/*                                                                  */
/* deband(                                                          */
/*          int rgbdist,    max RGB distance for band detection     */
/*          int imgstep,    check for bands every nth pixel         */
/*          int blksize,    size of block to be dithered            */
/*          int blkstep,    dither every nth pixel within block     */
/*          int clrarea     neighbourhood for colour stats          */
/*       )                                                          */
/*                                                                  */
/* Checks for banding in an 8 bit image and reduces its effects     */
/* by juggling pixels around in the neighbourhood of the bands.     */
/*                                                                  */
/* Given an 8-bit image, the algorithm considers adjacent pixels    */
/* to be banded when their colours are different, but the distance  */
/* between the colours in RGB space is less than rgbdist. The       */
/* distance metric used is |R - R'| + |G - G'| + |B - B'|.          */
/* Increasing rgbdist allows more colour combinations through the   */
/* threshold and thus the dither process will be applied more often.*/
/*                                                                  */
/* The algorithm steps through the image looking for banded pixels. */
/* When a band is detected between adjacent colours c1 and c2, a    */
/* block of pixels around the band is dithered by selectively       */
/* replacing c1 pixels with c2 pixels. The likelihood of replacing  */
/* a given pixel is dependent on the relative occurence of the      */
/* colours c1 and c2 in the neighbourhood of the band. The relative */
/* occurrence is factored in by, for each pixel in the block to be  */
/* dithered, looking a random distance away from the pixel (within  */
/* clrarea/2 of the pixel) to a second pixel in the local           */
/* area. If the second pixel is the opposite colour of the pixel    */
/* to be dithered (i.e. one is c1, the other is c2), then the       */
/* pixel is replaced with the other colour. This approach preserves */
/* the relative occurrence of c1 and c2, thereby keeping the same   */
/* overall colour in the local area.                                */
/*                                                                  */
/* Quality is best when every pixel in the image is checked for     */
/* banding, and every pixel in the dither block is (potentially)    */
/* dithered. This makes the process too slow for real-time use, and */
/* so the image can be stepped through imgstep pixels at a time in  */
/* x and y, and the block can be stepped through blkstep pixels at  */
/* a time.                                                          */
/*                                                                  */
/* blksize must be a power of two; blkstep must be a smaller        */
/* power of two. imgstep should be less than or equal to blksize.   */
/* clrarea should be similar to blksize. Typical rgbdist values are */
/* in the range of about 6-35, but may meed to be adjusted for      */
/* particular images.                                               */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fdeband.h"
#include <string.h>

/* Internal defines */

#define NBLOCKS     11          /* Number of randomly generated
                                   dither blocks (used to look in
                                   local area for band colours). */
/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define MYRAND()    ((myrand = myrand * 1103515245 + 12345) & 0x7fff)
#define MYSRAND(S)  (myrand = (S))

    /* Measure distance in RGB space. */
static int rgbdistance(ARGB a, ARGB b)
{
    int dr, dg, db;

    dr = a.r - b.r;
    if (dr < 0) dr = -dr;
    dg = a.g - b.g;
    if (dg < 0) dg = -dg;
    db = a.b - b.b;
    if (db < 0) db = -db;

    return dr + dg + db;
}

    /* Create array of dither blocks used to look at
       colours in local area. */
static int *ditherinit(int bsize, int bstep, int rowbytes, int clrarea)
{
    int i, x, y;
    int dx, dy;
    int *dither;
    int *dblock;        /* Dither block. */
    int dithersize;     /* Number of pixels in block that actually get dithered. */
    int myrand;

    dithersize = (bsize*bsize)/(bstep*bstep);
    dither = (int *)galloc(NBLOCKS*dithersize*sizeof(int));
    if (!dither)
        return NULL;

        /* Seed random number generator so that applying
           filter twice to same image will give same results. */
    MYSRAND(0x4321);

    for (i = 0; i < NBLOCKS; ++i)
    {
        dblock = dither + i*dithersize;

        for (y = 0; y < bsize; y += bstep)
        {
            for (x = 0; x < bsize; x += bstep)
            {
                    /* Each entry in a dither block is a delta
                       that offsets from the top left corner of
                       a block of pixels to some other pixel in
                       the local area. Deltas are computed at
                       each x and y in the pixel block in order
                       to look at a neighbouring pixel within
                       a clrarea*clrarea block of pixels centered
                       on (x,y). */
                dx = x + (MYRAND()%(clrarea + 1)) - (clrarea >> 1);
                dy = y + (MYRAND()%(clrarea + 1)) - (clrarea >> 1);

                *dblock++ = dx + dy*rowbytes;
            }
        }
    }

    return dither;
}

    /* Apply dithering algorithm to one block of pixels. */
static void ditherblock(
                 unsigned char *src, unsigned char *dst,      /* pixel data */
                 int blksize, int blkstep,  /* block size, increment */
                 unsigned char band0, unsigned char band1,    /* banding colours */
                 int *dither,               /* dither area array */
                 int rowbytes)              /* applies to src and dst */
{
    int     xcnt, ycnt;
    int     steps = blksize/blkstep;
    unsigned char    spix, dpix;

    rowbytes *= blkstep;
    ycnt = steps;
    while (ycnt)
    {
        --ycnt;

            /* Algorithm ensures that only band0 and band1
               pixels are modified, and only by replacing
               with the other colour. */
        xcnt = steps;
        while (xcnt)
        {
            --xcnt;
            dpix = *dst;
            if ((dpix == band0) || (dpix == band1))
            {
                spix = *(src + *dither);
                if ((spix == band0) || (spix == band1))
                {
                    *dst = spix;
                }
            }
            ++dither;
            dst += blkstep;
        }

        dst += rowbytes - blksize;
    }
}

    /* Detect bands in image and fix by dithering. */
static void ditherbands(
                    unsigned char *src, unsigned char *dst,
                    int width, int height, int rowbytes,
                    ARGB *palette,
                    int rgbdist,
                    int imgstep,
                    int blksize, int blkstep,
                    int *dither)
{
    int    y, cnt;
    int    blk = 0;
    int    offset;
    int    dithersize = (blksize*blksize)/(blkstep*blkstep);
    unsigned char   *pix, *nextrow, *dest;
    unsigned char   band0, band1;

        /* Assume this applies to fsrc and fdst. */
    offset = (blksize/2 - 1)*(rowbytes + 1);

        /* Start far enough into the image, and finish far
           enough from the end, so that we don't clobber memory
           outside the shape data when a block is offset from
           the current position. */
    for (y = blksize; y < height - blksize; y += imgstep)
    {
        pix = src + y*rowbytes + blksize;
        nextrow = pix + imgstep*rowbytes;
        dest = dst + y*rowbytes + blksize - offset;
        cnt = width - 2*blksize;

        while (cnt)
        {
            --cnt;

                /* Check for vertical banding. */
            band0 = *pix;
            band1 = *(pix + imgstep);
            if (rgbdistance(palette[band0], palette[band1]) <= rgbdist)
            {
                ditherblock(pix - offset, dest,
                            blksize, blkstep,
                            band0, band1,
                            dither + blk, rowbytes);
                blk += dithersize;
                if (blk >= NBLOCKS*dithersize)
                    blk = 0;
            }
            else
            {
                    /* Check for vertical banding. */
                band1 = *nextrow;
                if (rgbdistance(palette[band0], palette[band1]) <= rgbdist)
                {
                    ditherblock(pix - offset, dest,
                                blksize, blkstep,
                                band0, band1,
                                dither + blk, rowbytes);
                    blk += dithersize;
                    if (blk >= NBLOCKS*dithersize)
                        blk = 0;
                }
            }

            dest += imgstep;
            pix += imgstep;
            nextrow += imgstep;
        }
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutdeband(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('D','B','N','D');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           0;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Y. King");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Deband");
        strcpy(about->longnamestr,  "Deband");

        about->numoptions =                5;

        strcpy(about->option[0].name,      "RGB distance");
        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           (255 + 255 + 255);
        about->option[0].def.i =           RGBDIST_DEF;

        strcpy(about->option[1].name,      "Image step");
        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           1;
        about->option[1].max.i =           64;
        about->option[1].def.i =           IMGSTEP_DEF;

        strcpy(about->option[2].name,      "Block size");
        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           1;
        about->option[2].max.i =           64;
        about->option[2].def.i =           BLKSIZE_DEF;

        strcpy(about->option[3].name,      "Block step");
        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           1;
        about->option[3].max.i =           64;
        about->option[3].def.i =           BLKSTEP_DEF;

        strcpy(about->option[4].name,      "Colour area");
        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           1;
        about->option[4].max.i =           128;
        about->option[4].def.i =           CLRAREA_DEF;
    }

    return(about);
}

FPUBLIC int opendeband(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int         ok = 0;
    FINSTANCE   *fx = NULL;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = 1;            /* Only a single source allowed. */
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }

    *filterInstance = fx;
    return(ok);
}


FPUBLIC GINFO *infodeband(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

        /* framenum is ignored, this is strictly a 1:1
           filter. We just make a copy of the ginfo
           structure of the source image. */
    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }

    return ginfo;
}

FPUBLIC int filterdeband(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int     ok = 0;
    int     width  = fx->source[0].ginfo->width;
    int     height = fx->source[0].ginfo->height;

    int     rgbdist = RGBDIST_DEF;
    int     imgstep = IMGSTEP_DEF;
    int     blksize = BLKSIZE_DEF;
    int     blkstep = BLKSTEP_DEF;
    int     clrarea = CLRAREA_DEF;

    int     *ditherblks;

        /* Over-ride defaults with any user-supplied options. */
    if (fx->option)
    {
        if (fx->numoptions >= 1)    rgbdist = fx->option[0].i;
        if (fx->numoptions >= 2)    imgstep = fx->option[1].i;
        if (fx->numoptions >= 3)    blksize = fx->option[2].i;
        if (fx->numoptions >= 4)    blkstep = fx->option[3].i;
        if (fx->numoptions >= 5)    clrarea = fx->option[4].i;
    }

        /* Initialize dither blocks. */
    ditherblks = ditherinit(blksize, blkstep, rowbytes, clrarea);

        /* Copy source image to destination (assumes same rowbytes). */
    memcpy(dest, fx->source[0].image, height*rowbytes);

        /* Deband the image. */
    if (ditherblks)
    {
        ditherbands((unsigned char *)fx->source[0].image, (unsigned char *)dest,
                    width, height, rowbytes,
                    &ginfo->colourtbl[0],
                    rgbdist, imgstep, blksize, blkstep,
                    ditherblks);
        gfree(ditherblks);

        ok = 1;
    }

    return ok;
}

FPUBLIC int closedeband(FINSTANCE *fx)
{
    return gfree(fx);
}

