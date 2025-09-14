// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                          BLT - Gimex                             */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940524 FB Based on CMP module                            */
/*    1.01 940531 FB Updated to Gimex 1.03 spec                     */
/*    1.02 940614 FB updated to Gimex 1.5                           */
/*    1.03 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* Reentrant                                                        */
/* Files: bltread.cpp bltwrite.cpp bltgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* BLT files were originally used at EASM by John Ratcliffe, and    */
/* later incorporated into Latenight.  The format is a very simple  */
/* 256 entry, 8 bit palette file (.PAL - 768 bytes) and an unpacked */
/* bitmap with in a seperate file.  Its format is:                  */
/*                                                                  */
/*  [width] [height] [bitmap]                                       */
/*     2       2       w*                                           */
/*                                                                  */
/* width/height are intel                                           */
/* no line padding.                                                 */
/* bitmap is rightside up                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "bltgimex.h"

static char *BLTloadfile(char *basename, char *ext)
{
    char *ptr;
    char fname[512];
    GSTREAM *f;
    int flen;

    strcpy(fname,basename);
    if (strrchr(fname,'.'))
        *strrchr(fname,'.') = 0;
    strcat(fname,ext);

    f = gopen(fname);
    ptr = 0;
    if (f)
    {
        flen = glen(f);
        ptr = galloc(flen);
        if (ptr)
            gread(f,ptr,flen);
        gclose(f);
    }
    return(ptr);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL BLT_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','B','L','T');
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
        info->multifile       = 1;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','B','L','T');   /* mac file system types used */
        strcpy(info->extensions[0], ".blt");                /* extensions */
        strcpy(info->extensions[1], ".pal");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "BLT");                 /* type */
        strcpy(info->wordtypestr,   "BLT");                 /* wordtype */
        strcpy(info->longtypestr,   "Ratcliff BLT");        /* longtype */
    }
    return(info);
}



/* check for reasonable header: */
/* width*height+4 = len */

int GCALL BLT_is(GSTREAM *g)
{
    unsigned char bltheader[4];
    int width;
    int height;
    int clen;
    int flen;
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,bltheader,4))
    {
        width  = ggeti(bltheader+0,2);          /* Width of bitmap in pixels */
        height = ggeti(bltheader+2,2);          /* Height of bitmap in pixels */

        clen = width*height+4;
        flen = glen(g);

        if (clen==flen && width>=1 && width<=8192 && height>=1 && height<=8192)
            ok = 95;    /* 95% odds its a blt */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

/* open reads pal and holds onto it with ref */

int GCALL BLT_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','B','L','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->gref = pathname;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}


GINFO * GCALL BLT_info(GINSTANCE *gx, int framenum)
{
    unsigned char bltheader[4];
    GINFO *ginfo=0;
    char *pal;
    int i;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, bltheader, 4))
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','B','L','T');   /* Gimex type */
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
            ginfo->alphabits        = 1;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */

            ginfo->width  = ggeti(bltheader+0,2);   /* Width of bitmap in pixels */
            ginfo->height = ggeti(bltheader+2,2);   /* Height of bitmap in pixels */

            pal = BLTloadfile(gx->gref,".pal");
            if (pal)
            {
                for (i=0; i<256; ++i)
                {
                    ginfo->colourtbl[i].a = (i==0)?0:255;
                    ginfo->colourtbl[i].r = pal[i*3+0];
                    ginfo->colourtbl[i].g = pal[i*3+1];
                    ginfo->colourtbl[i].b = pal[i*3+2];
                }
                gfree(pal);
            }
        }
    }

    return(ginfo);
}

int GCALL BLT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int ok=0;

    for (y=0; y<ginfo->height; ++y)
    {
        ok = gread(gx->gstream,dest+rowbytes*y,ginfo->width);
    }
    return(ok);
}

int GCALL BLT_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


