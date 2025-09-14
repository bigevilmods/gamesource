// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  VGA - Latenite Raw VGA Gimex                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940603 FB Based on BLT module                            */
/*    1.01 940614 FB updated to Gimex 1.5                           */
/*    1.02 950101 FB crop to center based on RGS gimex              */
/*    1.03 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* Reentrant                                                        */
/* Files: vgaread.cpp vgawrite.cpp vgagimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* VGA files are created by latenight and are a raw vga palette     */
/* followed by a raw bitmap.  There is no header.                   */
/*                                                                  */
/*  [palette] [bitmap]                                              */
/*     768      w*h                                                 */
/*                                                                  */
/* VGA file can only be one of the following sizes:                 */
/*   320 x 200                                                      */
/*   640 x 400                                                      */
/*   640 x 480                                                      */
/*   800 x 600                                                      */
/*  1024 x 768                                                      */
/*  1280 x 1024                                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "vgagimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL VGA_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','V','G','A');
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
        info->mactype[0]      = GMAKEID('.','V','G','A');   /* mac file system types used */
        strcpy(info->extensions[0], ".vga");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "VGA");                 /* type */
        strcpy(info->wordtypestr,   "VGA");                 /* wordtype */
        strcpy(info->longtypestr,   "Latenight VGA");       /* longtype */
    }
    return(info);
}


/* check for reasonable header: */
/* width*height+768 = len */

int GCALL VGA_is(GSTREAM *g)
{
    int flen;
    int ok=0;

    flen = glen(g);
    if (flen==320*200+768
     || flen==640*400+768
     || flen==640*480+768
     || flen==800*600+768
     || flen==1024*768+768
     || flen==1280*1024+768)
    {
        ok = 30;    /* 30% odds its a vga (not a very reliable method) */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

/* open reads pal and holds onto it with ref */

int GCALL VGA_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int frames=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','V','G','A');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        frames = 1;
    }
    return(frames);
}


GINFO * GCALL VGA_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int i;
    int flen;
    int width;
    int height;
    int ok=0;
    char pal[768];

    ok = gseek(gx->gstream,(GPOS)0);
    flen = glen(gx->gstream);

    if (gread(gx->gstream, pal, 768))
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','V','G','A');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;    /* Number of frames */
            ginfo->bpp              = 8;    /* Bits per pixel (8 or 32) */
            /*ginfo->indexed          = 1;    \* 0 no, 1 yes (indexed meaning it uses a palette) *\ */
            ginfo->originalbpp      = 8;    /* Bits per pixel in original image (1 to 32) */
            /*ginfo->originalindexed  = 1;    \* 0 no, 1 yes (indexed meaning it used a palette) *\ */
            ginfo->numcolours  = 256;  /* Number of colours in original indexed palette */
            ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
            ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 6;    /* Number of bits in red channel */
            ginfo->greenbits        = 6;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 6;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */

            if (flen==320*200+768) width=320;
            else if (flen==640*400+768 || flen==640*480+768) width=640;
            else if (flen==800*600+768) width=800;
            else if (flen==1024*768+768) width=1024;
            else width=1280;
            height = (flen-768)/width;

            ginfo->width  = width;                  /* Width of bitmap in pixels */
            ginfo->height = height;                 /* Height of bitmap in pixels */

            for (i=0; i<256; ++i)
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = (pal[i*3+0]*255+32)/63;
                ginfo->colourtbl[i].g = (pal[i*3+1]*255+32)/63;
                ginfo->colourtbl[i].b = (pal[i*3+2]*255+32)/63;
            }
        }
    }

    return(ginfo);
}

int GCALL VGA_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int ok=0;

    ok = gseek(gx->gstream,768);
    for (y=0; y<ginfo->height && ok; ++y)
    {
        ok = gread(gx->gstream,dest+rowbytes*y,ginfo->width);
    }
    return(ok);
}

int GCALL VGA_close(GINSTANCE *gx)
{
    int ok=0;
    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


