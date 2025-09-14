// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    YUV - ACOM Raw Yuv Gimex                      */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941005 FB Based on Erik's RAW Gimex and YUV tool         */
/*    1.01 941007 FB yuvwrite based on yuv2x2 code                  */
/*    1.02 941017 FB no statics, align width on write               */
/*    1.03 941217 FB 12.20 fixed point precission                   */
/*    1.04 941228 FB rewrote based on YCbCr using floats/no matrix  */
/*    1.05 941229 FB signed average/weighted by luminence           */
/*    1.06 950529 FB ginstance signature                            */
/*    1.07 970710 FB adapted to ACOM raw YUV format                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit only                                                      */
/* Reentrant                                                        */
/* Frame buffered                                                   */
/* Files: yuvread.cpp yuvwrite.cpp yuvgimex.h                           */
/*                                                                  */
/* Original YUV matrixes from Luc's NTSC are in Frank's video       */
/* binder and in EAC lib yuv.c.  Converted from right hand          */
/* transforms to left hand (transposed)                             */
/* See also work of Bart Besseling YIQ compressor for 3DO.          */
/* Current Matrix based on colour spec FAQ from internet for YCbCr  */
/* used in JPeg.  YCbCr is easier to convert to and from RGB with   */
/* accuracy.                                                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* YUV files where created by YUV compressor and are a lossy        */
/* compression method, originally for 3DO.  RGB pixels are          */
/* to YUV and the U and V channels are reduced to 1/2 width and     */
/* height and further quanitized.                                   */
/* The immediate compression ratio to 2:1, but the data can also be */
/* substantially compressed by lossless compression.                */
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
#include <stdlib.h>
#include "gimex.h"
#include "yuvgimex.h"

/****************************************************************/
/*  YUV to RGB                                                  */
/****************************************************************/

static void yuvtorgb(YUVCOORD *yuv, YUVCOORD *rgb)
{
    int Y;
    int Cb;
    int Cr;
    int Red;
    int Green;
    int Blue;

    Y =  gmax(0,((yuv->x-16)*255+(235-16)/2)/(235-16));
    Cb = ((yuv->y-128)*255+224/2)/224;
    Cr = ((yuv->z-128)*255+224/2)/224;

    Red    = (int)gmax(0,gmin(255,Y+0.0000*Cb+1.4020*Cr));
    Green  = (int)gmax(0,gmin(255,Y-0.3441*Cb-0.7141*Cr));
    Blue   = (int)gmax(0,gmin(255,Y+1.7720*Cb+0.0000*Cr));

    rgb->x = Red;
    rgb->y = Green;
    rgb->z = Blue;
}

static void YUVdecode(ARGB *d,unsigned int y,unsigned int u,unsigned int v)
{
    YUVCOORD rgb;
    YUVCOORD yuv;

    yuv.x = y;  /* y */
    yuv.y = v;  /* v */
    yuv.z = u;  /* u */
    yuvtorgb(&yuv,&rgb);
    d->a = 255;
    d->r = rgb.x;     /* r */
    d->g = rgb.y;     /* g */
    d->b = rgb.z;     /* b */
}

/* check for reasonable header: */
/* file size and similarity of first 4 Ys */

int YUVsimilarity(int y1,int y2)
{
    int a=0;

    if (abs(y2-y1)<0x10)
    {
        a = 5;
        if (y1>0x60 && y1<0xe0)
            a = 10;
    }
    return(a);
}
/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL YUV_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','Y','U','V');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('A','Y','U','V');   /* mac file system types used */
        strcpy(info->extensions[0], ".yuv");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.06");                /* version # */
        strcpy(info->shorttypestr,  "YUV");                 /* type */
        strcpy(info->wordtypestr,   "Yuv");                 /* wordtype */
        strcpy(info->longtypestr,   "ACOM YUV");            /* longtype */
    }
    return(info);
}

int GCALL YUV_is(GSTREAM *g)
{
    unsigned char yuvheader[8];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,yuvheader,8))
    {
        if (glen(g)==720*486*2)
        {
            ok = 20;

            /* 00000: 80108010 80108010 80108010 80108010  ................ */

            ok += YUVsimilarity(yuvheader[0],yuvheader[4]);
            ok += YUVsimilarity(yuvheader[2],yuvheader[6]);
            ok += YUVsimilarity(yuvheader[5],yuvheader[7]);
            ok += YUVsimilarity(yuvheader[1],yuvheader[3]);
            ok += YUVsimilarity(yuvheader[5],yuvheader[7]);
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL YUV_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('A','Y','U','V');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL YUV_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;

    ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        ginfo->signature        = GMAKEID('A','Y','U','V'); /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = 0;
        ginfo->width            = 720;
        ginfo->height           = 486;
        ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */
        ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
        ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
        ginfo->redbits          = 8;    /* Number of bits in red channel */
        ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
        ginfo->bluebits         = 8;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
    }
    return(ginfo);
}

int GCALL YUV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpl;
    int x;
    int y;
    int ok=0;
    ARGB *d;

    gseek(gx->gstream,(GPOS)0);
    bpl = ginfo->width*2;
    linebuf = (char *) galloc(bpl);

    if (linebuf)
    {
        for (y=0; y<ginfo->height; ++y)
        {
            ok = gread(gx->gstream,linebuf,bpl);
            lineptr = linebuf;
            d = (ARGB *) (dest+rowbytes*y);

            for (x=0; x<ginfo->width; x+=2)
            {
                YUVdecode(d+0,lineptr[1],lineptr[2],lineptr[0]);
                YUVdecode(d+1,lineptr[3],lineptr[2],lineptr[0]);
                d+=2;
                lineptr += 4;
            }
        }
        gfree(linebuf);
    }

    return(ok);
}

int GCALL YUV_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

