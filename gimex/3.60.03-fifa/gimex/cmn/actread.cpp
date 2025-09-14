// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  ACT - Photoshop Palette Gimex                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941018 FB Based on HSI gimex.                            */
/*    1.01 950529 FB ginstance signature                            */
/*    1.02 950915 FB isact consider size better                     */
/*    1.03 001207 FB read 768, not 4096                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit palette                                                    */
/* Reentrant                                                        */
/* Files: actread.cpp actwrite.cpp actgimex.h                           */
/*                                                                  */
/* Palettes are imported as a 256x1x8 bitmap with the bitmap        */
/* containing the values 0..255                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* ACT Palette files are written by Photoshop from the Color Table  */
/* editor.  They are always 256 colours, stored as RGB 8 bit values */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "actgimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL ACT_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','A','C','T');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('8','B','C','T');   /* mac file system types used */
        strcpy(info->extensions[0], ".act");                /* extensions */
        strcpy(info->extensions[1], ".pal");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "ACT");                 /* type */
        strcpy(info->wordtypestr,   "ACT");                 /* wordtype */
        strcpy(info->longtypestr,   "ACT Photoshop Palette\n");/* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* filelength 768 */
/* Macintized palette? colour 0 white, colour 255 black */

int GCALL ACT_is(GSTREAM *g)
{
    int flen;
    int ok=0;
    int i=true;
    unsigned int f;
    unsigned int l;
    unsigned char actheader[768];

    flen = (int)glen(g);
    gseek(g,(GPOS)0);
    if (gread(g, actheader, gmin(flen,768)))
    {
        if (flen==768)
        {
            ok = 10;   /* 10% odds its an act photoshop palette */

            /* Macintized palette? */

            f = ggetm(actheader,3);
            l = ggetm(actheader+768-3,3);

            if ((f==0x000000 && l==0xffffff)
            ||  (f==0xffffff && l==0x000000))
            {
                ok += 10;
            }

            /* if all triplets except first or last are non-zero and non-ffff */
            /* better the odds. */

            for (i=1; i<255; ++i)
            {
                if (i==15)      /* skip EGA white */
                    continue;
                f = ggetm(actheader+3*i,3);
                if (f==0x000000 || f==0xffffff)
                    break;
            }
            if (i==255)
            {
                ok *= 2;
            }
        }
    }

    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL ACT_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','A','C','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL ACT_info(GINSTANCE *gx, int framenum)
{
    unsigned char actheader[768];
    GINFO *ginfo=0;
    int i;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, actheader, 768))
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','A','C','T');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;    /* Number of frames */
            ginfo->bpp              = 8;    /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 8;    /* Bits per pixel in original image (1 to 32) */
            ginfo->numcolours       = 256;  /* Number of colours in original indexed palette */
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

            ginfo->width            = 256;  /* Width of bitmap in pixels */
            ginfo->height           = 1;    /* Height of bitmap in pixels */

            for (i=0; i<256; ++i)
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = actheader[(i)*3+0];
                ginfo->colourtbl[i].g = actheader[(i)*3+1];
                ginfo->colourtbl[i].b = actheader[(i)*3+2];
#if 0
                ginfo->colourtbl[i].r = actheader[(255-i)*3+0];
                ginfo->colourtbl[i].g = actheader[(255-i)*3+1];
                ginfo->colourtbl[i].b = actheader[(255-i)*3+2];
#endif
            }
        }
    }

    return(ginfo);
}

int GCALL ACT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;

    for (x=0; x<ginfo->width; ++x)
        *(dest+x) = (char) x;

    return(1);
}

int GCALL ACT_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}



