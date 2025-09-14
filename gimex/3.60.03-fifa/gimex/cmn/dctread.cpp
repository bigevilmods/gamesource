// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                          DCT - Gimex                             */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941229 FB based on Y44 4x4 gimex                         */
/*    1.01 950102 FB 8 bit grey scale, pad out to 16,pixel align    */
/*    1.02 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,24,32 bit                                                      */
/* Reentrant                                                        */
/* Frame buffered                                                   */
/* Files: dctread.cpp dctwrite.cpp dctgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* 1, 3 or 4 channels                                               */
/* DCT files are DCT applied to YCbCr with subsampling on CbCr.     */
/* Width and Height are aligned to 16 due to 8x8 blocks on          */
/* samples chominance channels.                                     */
/*                                                                  */
/* RGB -> YCbCr (with CCIR 601-1 specs)                             */
/* Y=  0.2990*Red+0.5870*Green+0.1140*Blue                          */
/* Cb=-0.1687*Red-0.3313*Green+0.5000*Blue                          */
/* Cr= 0.5000*Red-0.4187*Green-0.0813*Blue                          */
/*                                                                  */
/* YCbCr -> RGB                                                     */
/* Red=  Y+0.0000*Cb+1.4020*Cr                                      */
/* Green=Y-0.3441*Cb-0.7141*Cr                                      */
/* Blue= Y+1.7720*Cb+0.0000*Cr                                      */
/*                                                                  */
/* I assume Y is within the range [0;1], and Cb and Cr are within   */
/* the range  [-0.5;0.5].                                           */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include <math.h>
#include "gimex.h"
#include "dctgimex.h"

void initiquant(int *quant,int quality)
{
    int x;
    int y;

    int l;
    int h;

    l = 8  +(100-quality)*(30-8)/100; /* 8..10 -> 30..60 */
    h = 10 +(100-quality)*(60-12)/100;

    for (y=0; y<8; ++y)
        for (x=0; x<8; ++x)
            *quant++ = (int) ((x+y)*(h-l)/14+l);
}

/****************************************************************/
/*  Inverse Discrete Cosine Transform                           */
/****************************************************************/

#define W1 5681 /* 4096*sqrt(2)*cos(1*pi/16) */
#define W2 5352 /* 4096*sqrt(2)*cos(2*pi/16) */
#define W3 4816 /* 4096*sqrt(2)*cos(3*pi/16) */
#define W5 3218 /* 4096*sqrt(2)*cos(5*pi/16) */
#define W6 2217 /* 4096*sqrt(2)*cos(6*pi/16) */
#define W7 1130 /* 4096*sqrt(2)*cos(7*pi/16) */

static void idctrow(int *src,int *dst)
{
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;

    /* shortcut */
    if (!src[0]  /* longs compare 2 shorts at a time */
     && !src[1]
     && !src[2]
     && !src[3]
     && !src[4]
     && !src[5]
     && !src[6]
     && !src[7])
    {
        dst[0] = dst[1] = dst[2] = dst[3] =
        dst[4] = dst[5] = dst[6] = dst[7] = 0;
        return;
    }

    /* first stage */
    x0 = src[0];
    x1 = src[4];
    x2 = src[6];
    x3 = src[2];
    x4 = src[1];
    x5 = src[7];
    x6 = src[5];
    x7 = src[3];
    x8 = W7*(x4+x5);
    x4 = x8 + (W1-W7)*x4;
    x5 = x8 - (W1+W7)*x5;
    x8 = W3*(x6+x7);
    x6 = x8 - (W3-W5)*x6;
    x7 = x8 - (W3+W5)*x7;

    /* second stage */
    x8 = ((x0+x1)<<12) + 16; /* +16 for proper rounding in the fourth stage */
    x0 = ((x0-x1)<<12) + 16;
    x1 = W6*(x3+x2);
    x2 = x1 - (W2+W6)*x2;
    x3 = x1 + (W2-W6)*x3;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;

    /* third stage */
    x7 = x8 + x3;
    x8 -= x3;
    x3 = x0 + x2;
    x0 -= x2;
    x2 = (181*(x4+x5)+128)>>8;
    x4 = (181*(x4-x5)+128)>>8;

    /* fourth stage */
    dst[0] = (x7+x1)>>5;
    dst[1] = (x3+x2)>>5;
    dst[2] = (x0+x4)>>5;
    dst[3] = (x8+x6)>>5;
    dst[4] = (x8-x6)>>5;
    dst[5] = (x0-x4)>>5;
    dst[6] = (x3-x2)>>5;
    dst[7] = (x7-x1)>>5;
}

static void idctcol(unsigned char *iclp,int *src,int *dst)
{
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;

    /* first stage */
    x0 = src[8*0];
    x1 = src[8*4];
    x2 = src[8*6];
    x3 = src[8*2];
    x4 = src[8*1];
    x5 = src[8*7];
    x6 = src[8*5];
    x7 = src[8*3];

    x8 = W7*(x4+x5) + 2048;
    x4 = (x8+(W1-W7)*x4)>>12;
    x5 = (x8-(W1+W7)*x5)>>12;
    x8 = W3*(x6+x7) + 2048;
    x6 = (x8-(W3-W5)*x6)>>12;
    x7 = (x8-(W3+W5)*x7)>>12;

    /* second stage */
    x8 = x0 + x1;
    x0 -= x1;
    x1 = W6*(x3+x2) + 2048;
    x2 = (x1-(W2+W6)*x2)>>12;
    x3 = (x1+(W2-W6)*x3)>>12;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;

    /* third stage */
    x7 = x8 + x3 + 512;
    x8 += -x3 + 512;
    x3 = x0 + x2 + 512;
    x0 += -x2 + 512;
    x2 = (181*(x4+x5)+128)>>8;
    x4 = (181*(x4-x5)+128)>>8;

    /* fourth stage */
    dst[8*0] = iclp[(x7+x1)>>10];
    dst[8*1] = iclp[(x3+x2)>>10];
    dst[8*2] = iclp[(x0+x4)>>10];
    dst[8*3] = iclp[(x8+x6)>>10];
    dst[8*4] = iclp[(x8-x6)>>10];
    dst[8*5] = iclp[(x0-x4)>>10];
    dst[8*6] = iclp[(x3-x2)>>10];
    dst[8*7] = iclp[(x7-x1)>>10];
}

/* two dimensional inverse discrete cosine transform */

void idct(unsigned char *iclp, int *block)
{
    int i;
    int temp[64];     /* intermediate storage for 1D transform */

    for (i=0; i<8; i++)
        idctrow(block+8*i, temp+8*i);

    for (i=0; i<8; i++)
        idctcol(iclp,temp+i, block+i);
}

void initidct(unsigned char *iclp)
{
    int i;

    for (i= -512; i<512; i++)
        iclp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);
}


/****************************************************************/
/*  Dct to RGB                                                  */
/****************************************************************/

#if 0
/* looks simple, but takes 6.5 pages of floating point code */
/* fixed point gets it to less than 1. */

static void ycbcrtorgb(DCTCOORD *yuv, DCTCOORD *rgb)
{
    int Y;
    int Cb;
    int Cr;
    int Red;
    int Green;
    int Blue;

    Y  = yuv->x;
    Cb = yuv->y-128;
    Cr = yuv->z-128;

    Red    = gmax(0,gmin(255,Y+0.0000*Cb+1.4020*Cr));
    Green  = gmax(0,gmin(255,Y-0.3441*Cb-0.7141*Cr));
    Blue   = gmax(0,gmin(255,Y+1.7720*Cb+0.0000*Cr));

    rgb->x = Red;
    rgb->y = Green;
    rgb->z = Blue;
}

#define DCTFIXED(f) ((int)(f*0x100000))

static void ycbcrtorgb(DCTCOORD *yuv, DCTCOORD *rgb)
{
    int Y;
    int Cb;
    int Cr;
    int t;

    Y  = yuv->x;
    Cb = yuv->y-128;
    Cr = yuv->z-128;

    t = Y                                + ((DCTFIXED(1.4020)*Cr + DCTFIXED(0.0))>>20); rgb->x = gmax(0,gmin(255,t));
    t = ((Y<<20) -   DCTFIXED(0.3441)*Cb -   DCTFIXED(0.7141)*Cr + DCTFIXED(0.0))>>20;  rgb->y = gmax(0,gmin(255,t));
    t = Y        + ((DCTFIXED(1.7720)*Cb                         + DCTFIXED(0.0))>>20); rgb->z = gmax(0,gmin(255,t));
}
#endif

#define ZIG(a,b,c,d,e,f,g,h) zo[0]=a; zo[1]=b; zo[2]=c; zo[3]=d; zo[4]=e; zo[5]=f; zo[6]=g; zo[7]=h; zo+=8


void initizigorder(int *zo)
{
    ZIG( 0, 1, 5, 6,14,15,27,28);
    ZIG( 2, 4, 7,13,16,26,29,42);
    ZIG( 3, 8,12,17,25,30,41,43);
    ZIG( 9,11,18,24,31,40,44,53);
    ZIG(10,19,23,32,39,45,52,54);
    ZIG(20,22,33,38,46,51,55,60);
    ZIG(21,34,37,47,50,56,59,61);
    ZIG(35,36,48,49,57,58,62,63);
}

void DCTDeCompress(int *iquant,unsigned char *iclip,unsigned char *dctdata,unsigned char *AYCbCr,int width,int height,int step)
{
    int bx;
    int by;
    int y;
    int prevdc=128;
    int dc;
    unsigned char *d;
    int *s;
    int block[8*8];
    int zigorder[8*8];

    initizigorder(zigorder);

    for (by=0; by<height; by+=step*8)
    {
        for (bx=0; bx<width; bx+=step*8)
        {
            /* zigzag input */

            for (y=0; y<8; ++y)
            {
                block[y*8+0] = (int) ((signed char) dctdata[zigorder[y*8+0]]);
                block[y*8+1] = (int) ((signed char) dctdata[zigorder[y*8+1]]);
                block[y*8+2] = (int) ((signed char) dctdata[zigorder[y*8+2]]);
                block[y*8+3] = (int) ((signed char) dctdata[zigorder[y*8+3]]);
                block[y*8+4] = (int) ((signed char) dctdata[zigorder[y*8+4]]);
                block[y*8+5] = (int) ((signed char) dctdata[zigorder[y*8+5]]);
                block[y*8+6] = (int) ((signed char) dctdata[zigorder[y*8+6]]);
                block[y*8+7] = (int) ((signed char) dctdata[zigorder[y*8+7]]);
            }
            dctdata += 64;

            /* delta decode AC component */

            dc = block[0];
            block[0] = prevdc = (dc+prevdc)&255;

            /* inverse quantize */

            for (y=0; y<8; ++y)
            {
                block[y*8+0] *= iquant[y*8+0];
                block[y*8+1] *= iquant[y*8+1];
                block[y*8+2] *= iquant[y*8+2];
                block[y*8+3] *= iquant[y*8+3];
                block[y*8+4] *= iquant[y*8+4];
                block[y*8+5] *= iquant[y*8+5];
                block[y*8+6] *= iquant[y*8+6];
                block[y*8+7] *= iquant[y*8+7];
            }

            /* inverse DCT */

            idct(iclip,block);

            /* copy to 8x8 block */

            s = block;
            for (y=0; y<8; ++y)
            {
                d = AYCbCr+((by+y*step)*width+bx)*4;

                *d = s[0]; d += step*4;
                *d = s[1]; d += step*4;
                *d = s[2]; d += step*4;
                *d = s[3]; d += step*4;
                *d = s[4]; d += step*4;
                *d = s[5]; d += step*4;
                *d = s[6]; d += step*4;
                *d = s[7];
                s += 8;
            }
        }
    }

    return;
}

#define DCTFIXED(f) ((int)(f*0x100000))

void ConvertToRGB(unsigned char *iclp,ARGB *source,char *dest,int gwidth, int gheight, int rowbytes,int channels,int width,int height)
{
    ARGB *s;
    ARGB *d;
    char *db;
    int x;
    int y;
    int Y;
    int Cb;
    int Cr;
    int t;

    /* convert entire frame to RGB */

    s = source;
    if (channels==1)
    {
        for (y=0; y<gheight; ++y)
        {
            db = dest+y*rowbytes;
            for (x=0; x<gwidth; ++x)
            {
                *db++ = (char) s->r;
                ++s;
            }
            s+=width-gwidth;
        }
    }
    else
    {
        for (y=0; y<gheight; ++y)
        {
            d = (ARGB *) (dest+y*rowbytes);
            for (x=0; x<gwidth; ++x)
            {
                d->a = s->a;

                Y  = (int) s->r;
                Cb = (int) s->g - 128;
                Cr = (int) s->b - 128;

                d->r = iclp[Y                                + ((DCTFIXED(1.4020)*Cr + DCTFIXED(0.0))>>20)];
                d->g = iclp[((Y<<20) -   DCTFIXED(0.3441)*Cb -   DCTFIXED(0.7141)*Cr + DCTFIXED(0.0))>>20 ];
                d->b = iclp[Y        + ((DCTFIXED(1.7720)*Cb                         + DCTFIXED(0.0))>>20)];

                ++s;
                ++d;
            }
            s+=width-gwidth;
        }
    }

    return;
}

/* chrominance subsample width and height to 1\2 */

void UpSample(unsigned char *source, int width, int height)
{
    int x;
    int y;
    unsigned char *s;
    unsigned char n;
    unsigned char p;
    unsigned char i;

    s = source;

    for (y=0; y<height; y+=2)
    {
        p = *s;
        for (x=0; x<width; x+=2)
        {
            n = *s;
            i = (unsigned char) (((int)p+(int)n)>>1);

            s[0] = i;       /* filter left pixels */
            s[width*4] = i;
            s[1*4] = n;
            s[width*4+1*4] = n;

            p = n;

            s += 8; /* step by 2 pixels */
        }
        s += width*4;   /* skip a row */
    }
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL DCT_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','D','C','T');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','D','C','T');   /* mac file system types used */
        strcpy(info->extensions[0], ".dct");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.02");                /* version # */
        strcpy(info->shorttypestr,  "DCT");                 /* type */
        strcpy(info->wordtypestr,   "DCT");                 /* wordtype */
        strcpy(info->longtypestr,   "DCT YCbCr Compressed"); /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* id 'DCTy'*/

int GCALL DCT_is(GSTREAM *g)
{
    unsigned char dctheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,dctheader,4))
    {
        if (ggetm(dctheader,4)==GMAKEID('D','C','T','y'))
            ok = 100;
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL DCT_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','D','C','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL DCT_info(GINSTANCE *gx, int framenum)
{
    DCTHeaderStruct dctheader;
    GINFO *ginfo=0;
    int width;
    int height;
    int quality;
    int channels;
    int orgbpp;
    int bpp;
    int indexed;
    int colours;
    int alphabits;
    int i;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream,&dctheader,16))
    {
        width    = DCTGETFIELD(dctheader.framewidth);   /* Width of bitmap in pixels */
        height   = DCTGETFIELD(dctheader.frameheight);  /* Height of bitmap in pixels */
        quality  = DCTGETFIELD(dctheader.quality);
        channels = DCTGETFIELD(dctheader.channels);
        switch (channels)
        {
            default:
            case 3: orgbpp = 24; bpp = 32; indexed = 0; colours = 0;   alphabits = 0; break;
            case 4: orgbpp = 32; bpp = 32; indexed = 0; colours = 0;   alphabits = 8; break;
            case 1: orgbpp = 8;  bpp = 8;  indexed = 1; colours = 256; alphabits = 0; break;
        }

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','D','C','T');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;            /* Current frame */
            ginfo->width            = width;        /* Width of bitmap in pixels */
            ginfo->height           = height;       /* Height of bitmap in pixels */
            ginfo->bpp              = bpp;          /* Bits per pixel (8 or 32) */
            /*ginfo->indexed          = indexed;      \* 0 no, 1 yes (indexed meaning it uses a palette) *\ */
            ginfo->originalbpp      = orgbpp;       /* Bits per pixel in original image (1 to 32) */
            /*ginfo->originalindexed  = indexed;      \* 0 no, 1 yes (indexed meaning it used a palette) *\ */
            ginfo->numcolours  = colours;      /* Number of colours in original indexed palette */
            ginfo->subtype          = 0;            /* Internal format sub-type 0-default */
            ginfo->packed           = 1;            /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = quality;      /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */

            for (i=0; i<colours; ++i)
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = i;
                ginfo->colourtbl[i].g = i;
                ginfo->colourtbl[i].b = i;
            }
        }
    }
    return(ginfo);
}

int GCALL DCT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;
    int width;
    int height;
    int channels;
    char *dctdata;
    ARGB *AYCbCr;
    unsigned char icliptbl[1024]; /* clipping table */
    unsigned char *iclip;
    int iquant[8*8];

    iclip = icliptbl+512;
    initidct(iclip);
    initiquant(iquant,ginfo->quality);

    gseek(gx->gstream,16);  /* skip header */

    width =  (ginfo->width+15)&~15;  /* 16 due to 8x8 blocks and chrominance subsampling */
    height = (ginfo->height+15)&~15;
    channels = 3;
    if (ginfo->originalbpp<=8)        channels = 1;
    else if (ginfo->originalbpp>24)   channels = 4;

    AYCbCr = galloc(width*height*4);
    if (AYCbCr)
    {
        dctdata = galloc(width*height);

        if (dctdata)
        {

            ok = gread(gx->gstream, dctdata, width*height);
            DCTDeCompress(iquant,iclip,(unsigned char*)dctdata,&AYCbCr->r,width,height,1); /* Y plane */

            if (channels>1 && ok)
            {
                ok = gread(gx->gstream, dctdata, width*height/4);
                DCTDeCompress(iquant,iclip,(unsigned char*)dctdata,&AYCbCr->g,width,height,2); /* Cb plane */

                ok = gread(gx->gstream, dctdata, width*height/4);
                DCTDeCompress(iquant,iclip,(unsigned char*)dctdata,&AYCbCr->b,width,height,2); /* Cr plane */

                if (channels>3 && ok)
                {
                    ok = gread(gx->gstream, dctdata, width*height);
                    DCTDeCompress(iquant,iclip,(unsigned char*)dctdata,&AYCbCr->a,width,height,1); /* A plane */
                }
                if (ok)
                {
                    UpSample(&AYCbCr->g,width,height); /* Cb upsample */
                    UpSample(&AYCbCr->b,width,height); /* Cr upsample */
                }
            }
            if (ok)
            {
                ConvertToRGB(iclip,AYCbCr,dest,ginfo->width,ginfo->height,rowbytes,channels,width,height);
            }

            gfree(dctdata);
        }
        gfree(AYCbCr);
    }

    return(ok);
}

int GCALL DCT_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

