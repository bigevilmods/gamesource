// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 NEW - New Image Generator Gimex                  */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 000302 FB Generate blank image                           */
/*    1.01 020409 FB Export to 'null'                               */
/*    1.02 020507 FB renamed to NULL internally                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit import/export                                           */
/* Reentrant                                                        */
/* Frame buffered                                                   */
/* Files: newread.c newgimex.h                                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Fills in default ginfo with default 256x256x32 image             */
/* Application can change ginfo size and then call read             */
/* to generate image of certain size.  Contains all 0's for pixels  */
/* Will return one image, but can be called multiple times          */
/*                                                                  */
/* For exporting, this is an alternative to -w0, but behaves the    */
/* same as actually writting a file                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "newgimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL NEW_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('N','U','L','L');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 0;    /* module is file based */
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->resizable       = 1;    /* read respects ginfo width and height */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('N','U','L','L');   /* mac file system types used */
        strcpy(info->extensions[0], ".new");                /* extensions */
        strcpy(info->extensions[1], ".null");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.02");                /* version # */
        strcpy(info->shorttypestr,  "New");                 /* type */
        strcpy(info->wordtypestr,   "New8");                /* wordtype (use this filename to get 8 bit ginfo) */
        strcpy(info->longtypestr,   "New Image");           /* longtype */
    }
    return(info);
}

int GCALL NEW_is(GSTREAM *g)
{
    return(1);  // ok, but only if all else fails
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL NEW_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('N','U','L','L');
        gx->size=sizeof(GINSTANCE);
        gx->gref = NULL;

        if (!strcmp(pathname,"new8"))  // tell ginfo to give 8 bit
            gx->gref = (void *) 1;

        gx->frames = ok = 1;
    }

    return(ok);
}

GINFO * GCALL NEW_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
   	int w;
    int h;
    int bpp;

    w = 256;
    h = 256;
    bpp = 32;
    if (gx->gref)
        bpp = 8;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('N','U','L','L');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->width            = w;
        ginfo->height           = h;
        ginfo->originalbpp      = bpp;
        ginfo->bpp              = bpp;
        ginfo->numcolours       = bpp==8?256:0;   /* Number of colours in original indexed palette */
        ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
        ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = 8;    /* Number of bits in alpha channel */
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

int GCALL NEW_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int ok=1;
    int bpl=ginfo->width*ginfo->bpp/8;
    ARGB *d;

    for (y=0; y<ginfo->height; ++y)
    {
        d = (ARGB *) (dest+rowbytes*y);
        memset(d,0,bpl);
    }

    return(ok);
}

int GCALL NEW_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        gx->gref = 0;
        ok = gfree((void *) gx);
    }
    return(ok);
}

