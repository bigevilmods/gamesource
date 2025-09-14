// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  SCR - 3DO 24 bit Screen Gimex                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940606 FB first version based on raw                     */
/*    1.01 940614 FB updated to Gimex 1.5                           */
/*    1.02 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* 32 bit only                                                      */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: scrread.cpp scrwrite.cpp scrgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "scrgimex.h"

#if 0

#define RGBL(x) ((x&7)+((x&128)>>4))
#define RGBR(y) (31-(((y>>3)&15)+((y&128)>>7)))

void writepixel24(int x, int y, int colour)
{
    int r,g,b;
    unsigned int pixl,pixr;

    r = (colour>>16)&255;
    g = (colour>>8 )&255;
    b = (colour    )&255;

    pixl = (RGBL(r)<<10) | (RGBL(g)<<5) | (RGBL(b)<<0);
    pixr = (RGBR(r)<<10) | (RGBR(g)<<5) | (RGBR(b)<<0);

    *(unsigned int *)(buf+((y*320+x)<<2)) = ((pixl << 16) | pixr);
}
 f = (31-(((y>>3)&15)+((y&128)>>7)))
 y = (((31-f)-((y&128)>>7))&15)<<3

#endif


unsigned char decodergb(char *lineptr,int chshift)
{
    unsigned int pixl;
    unsigned int pixr;

    pixl = (ggetm(lineptr+2,2)>>chshift)&31;
    pixr = (ggetm(lineptr+0,2)>>chshift)&31;

    pixl = (pixl&7)+((pixl<<4)&128);
    pixr = (((31-pixr)-((pixl&128)>>7))&15)<<3;

    return((unsigned char)(pixl|pixr));
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL SCR_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','S','C','R');
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
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','S','C','R');   /* mac file system types used */
        strcpy(info->extensions[0], ".scr");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.02");                /* version # */
        strcpy(info->shorttypestr,  "SCR");                 /* type */
        strcpy(info->wordtypestr,   "Screen");              /* wordtype */
        strcpy(info->longtypestr,   "3DO 24 bit Screen");   /* longtype */
    }
    return(info);
}


/* check for reasonable header: */
/* size = 307200 */

int GCALL SCR_is(GSTREAM *g)
{
    int flen;

    flen = glen(g);

    if (flen==307200)
        return(10); /* 10% odds its a scr */

    return(0);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/


int GCALL SCR_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','S','C','R');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}


GINFO * GCALL SCR_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;

    ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('.','R','A','W');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Number of frames */
        ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
        /*ginfo->indexed          = 0;    \* 0 no, 1 yes (indexed meaning it uses a palette) *\ */
        ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
        /*ginfo->originalindexed  = 0;    \* 0 no, 1 yes (indexed meaning it used a palette) *\ */
        ginfo->numcolours  = 0;    /* Number of colours in original indexed palette */
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
        ginfo->width            = 320;  /* Width of bitmap in pixels */
        ginfo->height           = 240;  /* Height of bitmap in pixels */
    }
    return(ginfo);
}

int GCALL SCR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpl;
    int x;
    int y;
    int ok=0;
    ARGB *d;

    gseek(gx->gstream,(GPOS)0);

    bpl = ginfo->width*4;
    linebuf = (char *) galloc(bpl);

    if (linebuf)
    {
        ok = 1;
        for (y=0; y<ginfo->height && ok; ++y)
        {
            ok = gread(gx->gstream,linebuf,bpl);
            lineptr = linebuf;
            d = (ARGB *) (dest+rowbytes*y);

            for (x=0; x<ginfo->width; ++x)
            {
                d->a = 255;
                d->r = decodergb(lineptr,10);
                d->g = decodergb(lineptr,5);
                d->b = decodergb(lineptr,0);
                ++d;
                lineptr += 4;
            }
        }
        gfree(linebuf);
    }

    return(ok);
}

int GCALL SCR_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


