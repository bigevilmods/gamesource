// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                      PIX - Alias PIX Gimex                       */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 950613 FB Based on RLA                                   */
/*    1.01 980929 FB Renamed to PIX (easier to work with photoshop) */
/*    1.02 000718 FB Flipped to top down                            */
/*    1.03 000720 FB 8 bit export convert to grey scale             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit grey scale and 32 bit                                      */
/* decompresses/compresses                                          */
/* Reentrant                                                        */
/* Files: pixread.cpp pixwrite.cpp pixgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "pixgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int PIXreadline(GSTREAM *g,char *linebase,int width,int bpp)
{
    int ok=0;
    int len;
    ARGB *p;
    int x=0;
    unsigned char buf[4];

    if (bpp==32)
    {
        p = (ARGB *) linebase;
        while (x<width)
        {
            ok = gread(g,buf,4);
            len = buf[0];
            while (len--)
            {
                p->a = 255;
                p->r = buf[3];
                p->g = buf[2];
                p->b = buf[1];
                ++p;
                ++x;
            }

        }
    }
    else
    {
        while (x<width)
        {
            ok = gread(g,buf,2);
            len = buf[0];
            while (len--)
            {
                *linebase++ = (char)buf[1];
                ++x;
            }

        }
    }
    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL PIX_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('A','L','I','S');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
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
        info->mactype[0]      = GMAKEID('.','P','I','X');   /* mac file system types used */
        strcpy(info->extensions[0], ".pix");                /* extensions */
        strcpy(info->extensions[1], ".als");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "PIX");                 /* type */
        strcpy(info->wordtypestr,   "Alias");               /* wordtype */
        strcpy(info->longtypestr,   "Alias Pix");           /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* window values should be left/right top/bottom */
/* number of channels should be 1..4 */

int GCALL PIX_is(GSTREAM *g)
{
    PIXHeaderStruct pixheader;
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,(void *)&pixheader,10))
    {
        if (PIXGETFIELD(pixheader.width) > 0
         && PIXGETFIELD(pixheader.width) < 16384
         && PIXGETFIELD(pixheader.height) > 0
         && PIXGETFIELD(pixheader.height) < 16384
         && (PIXGETFIELD(pixheader.bpp) == 24
          || PIXGETFIELD(pixheader.bpp) ==8))
            ok = 95;    /* 95% odds its a pix */
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL PIX_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('A','L','I','S');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL PIX_info(GINSTANCE *gx, int framenum)
{
    PIXHeaderStruct pixheader;
    GINFO *ginfo=0;
    int width;
    int height;
    int bpp;
    int i;
    int ok=0;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, (char *) &pixheader, 10))   /* read header */
    {
        width  = PIXGETFIELD(pixheader.width);
        height = PIXGETFIELD(pixheader.height);
        bpp    = PIXGETFIELD(pixheader.bpp);

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('A','L','I','S');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Frame number */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->packed           = 1;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;            /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
            ginfo->bpp              = (bpp==8)?8:32; /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours  = (bpp==8)?256:0;/* Number of colours in original indexed palette */

            for (i=0; i<256; ++i)
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = i;
                ginfo->colourtbl[i].g = i;
                ginfo->colourtbl[i].b = i;
            }
            ok = 1;
        }
    }

    return(ginfo);
}

int GCALL PIX_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int width;
    int height;
    int ok=0;
    char *linebase;

    ok = gseek(gx->gstream,10);
    width   = ginfo->width;
    height  = ginfo->height;

    if (ok)
    {
        for (y=0; y<height && ok; ++y)
        {
            linebase = dest + rowbytes*y;

            ok = PIXreadline(gx->gstream,linebase,width,ginfo->bpp);
        }
    }
    return(ok);
}


int GCALL PIX_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


