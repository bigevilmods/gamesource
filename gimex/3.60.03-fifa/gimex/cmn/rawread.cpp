// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    RAW - Erik Kiss Raw Gimex                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940428 FB Erik's Raw format                              */
/*    1.01 940430 FB Simplified further                             */
/*    1.02 940531 FB inforaw,writeraw,error checking                */
/*    1.03 940614 FB updated to Gimex 1.5                           */
/*    1.04 950529 FB ginstance signature                            */
/*    1.05 010524 FB 32 bit support                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit only                                                      */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: rawread.cpp rawwrite.cpp rawgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* RAW files where created by Erik Kiss by his 3d rendering         */
/* program for PC.  They are the simpliest form of 24 bit           */
/* possible, containing width, height and 24 bit rgb data.          */
/*                                                                  */
/*  [width]     2 bytes intel.  width of image in pixels            */
/*  [height]    2 bytes intel.  height of image in pixels           */
/*  [rgb data]  width*height*3 bytes R G B ordering                 */
/*                                                                  */
/* The image is right side up, left to right, no padding.           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "rawgimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL RAW_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','R','A','W');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','R','A','W');   /* mac file system types used */
        strcpy(info->extensions[0], ".raw");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.05");                /* version # */
        strcpy(info->shorttypestr,  "Raw");                 /* type */
        strcpy(info->wordtypestr,   "Raw");                 /* wordtype */
        strcpy(info->longtypestr,   "Erik Kiss RAW");       /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* width*height*3+4 = len */

int GCALL RAW_is(GSTREAM *g)
{
    unsigned char rawheader[4];
    int width;
    int height;
    int flen;
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,rawheader,4))
    {
        width  = ggeti(rawheader+0,2);          /* Width of bitmap in pixels */
        height = ggeti(rawheader+2,2);          /* Height of bitmap in pixels */

        flen = (int) glen(g);
        if ((flen==width*height*3+4 || flen==width*height*4+4)
          && width>=1 && height>=1
          && width<40000 && height<40000)
        {
            ok = 50;
            if (width<=8192 && height<=8192)
                ok = 95;
        }
    }

    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL RAW_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','R','A','W');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL RAW_info(GINSTANCE *gx, int framenum)
{
    unsigned char rawheader[4];
    GINFO *ginfo=0;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream,rawheader,4))
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','R','A','W');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;    /* Current frame */
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

            ginfo->width  = ggeti(rawheader+0,2);   /* Width of bitmap in pixels */
            ginfo->height = ggeti(rawheader+2,2);   /* Height of bitmap in pixels */

            if ((int)glen(gx->gstream)==ginfo->width*ginfo->height*4+4)
            {
                ginfo->originalbpp      = 32;   /* Bits per pixel in original image (1 to 32) */
                ginfo->alphabits        = 8;    /* Number of bits in alpha channel */
            }
        }
    }
    return(ginfo);
}

int GCALL RAW_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpl;
    int x;
    int y;
    int ok=0;
    int bpp;
    ARGB *d;

    gseek(gx->gstream,4);
    bpp = ginfo->originalbpp;
    bpl = ginfo->width*bpp/8;
    linebuf = (char *)galloc(bpl);

    if (linebuf)
    {
        for (y=0; y<ginfo->height; ++y)
        {
            ok = gread(gx->gstream,linebuf,bpl);
            lineptr = linebuf;
            d = (ARGB *) (dest+rowbytes*y);

            for (x=0; x<ginfo->width; ++x)
            {
                d->r = lineptr[0];
                d->g = lineptr[1];
                d->b = lineptr[2];
                if (bpp>=32)
                {
                    d->a = lineptr[3];
                    lineptr += 4;
                }
                else
                {
                    d->a = 255;
                    lineptr += 3;
                }
                ++d;
            }
        }
        gfree(linebuf);
    }

    return(ok);
}

int GCALL RAW_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

