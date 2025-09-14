// (c) 2001-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  BNR - Nintendo Gamecube Banner                  */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 010810 FB First banner gimex based on tga                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 16 (5a3) import/export                                           */
/* Reentrant                                                        */
/* Files: bnrread.cpp bnrwrite.cpp bnrgimex.h                           */
/*                                                                  */
/* framename is used for both short and long titles. ie FIFA 2002   */
/* comment is 2 lines of text description of the game or a          */
/*   copyright notice                                               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* BNR files support 5a3 image format                               */
/*                                                                  */
/* // Game banner file must be placed in the root directory         */
/* #define DVD_BANNER_FILENAME         "opening.bnr"                */
/*                                                                  */
/* #define DVD_BANNER_WIDTH            96                           */
/* #define DVD_BANNER_HEIGHT           32                           */
/*                                                                  */
/* typedef struct DVDBanner                                         */
/* {                                                                */
/*  u32 id;                                                         */
/*  u32 padding[7];                                                 */
/*  u8  image[2 * DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT];  5a3 image */
/*  u8  shortTitle[32]; Short game title shown in IPL menu          */
/*  u8  shortMaker[32]; developer names shown in IPL menu           */
/*  u8  longTitle[64];  game title shown in IPL game start screen   */
/*  u8  longMaker[64];  developer name shown in IPL start screen    */
/*  u8  comment[128];   Game description shown in IPL game start    */
/*                      in two lines. Comment can include a '\n'.   */
/* } DVDBanner;                                                     */
/*                                                                  */
/* // DVDBanner.id must be set to 'BNR1'                            */
/* #define DVD_BANNER_ID               'BNR1'                       */
/*                                                                  */
/* make banner expects:                                             */
/* banner.tpl format: 96x32,RGB5A3                                  */
/* banner.txt format (separated by newline):                        */
/* title(32 bytes) Dolphin SDK                                      */
/* maker(32 bytes) Nintendo                                         */
/* title(64 bytes) Dolphin Software Development Kit                 */
/* maker(64 bytes) Nintendo                                         */
/* comment(128 bytes) (C)1999-2001 Nintendo. All rights reserved.   */
/* note: comment can be two lines.                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "bnrgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static const int width=96;
static const int height=32;

// pixel lookup function for 16bpp GameCube Shapes.

static int BNR_SwizzleGetPixelOffset16( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}


// convert GameCube 16 bit entry to 32 bit ARGB entry

static int BNRRpixel5To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

    if (aRgb5&0x8000)  // rgb555
    {
        aRgb8.a = (unsigned char)(255);
        aRgb8.r = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
        aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
        aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
    }
    else // argb3444
    {
        aRgb8.a = (unsigned char)((((aRgb5>>12)&7) *255+4)/7);
        aRgb8.r = (unsigned char)((((aRgb5>>8 )&15)*255+8)/15);
        aRgb8.g = (unsigned char)((((aRgb5>>4 )&15)*255+8)/15);
        aRgb8.b = (unsigned char)((((aRgb5>>0 )&15)*255+8)/15);
    }
    return(*(int *)&aRgb8);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL BNR_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','B','N','R');
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
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->maxwidth        = width;  /* maximum width in pixels */
        info->maxheight       = height; /* maximum height in pixels */
        info->alignwidth      = width;  /* width must be multiple of this in pixels */
        info->alignheight     = height; /* height must be multiple of this in pixels */
        info->mactype[0]      = GMAKEID('.','B','N','R');   /* mac file system types used */
        strcpy(info->extensions[0], ".bnr");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.00");                /* version # */
        strcpy(info->shorttypestr,  "BNR");                 /* type */
        strcpy(info->wordtypestr,   "NGC Banner");              /* wordtype */
        strcpy(info->longtypestr,   "Nintendo Banner");     /* longtype */
    }
    return(info);
}


/* check for reasonable header: */
/* 'BNR1' id */

int GCALL BNR_is(GSTREAM *g)
{
    char bnrbuf[4];
    int ok=0;

    gseek(g, 0);
    if (glen(g)==6496)
    {
        ok = 10;
        if (gread(g, bnrbuf, 4))
        {
            if (ggetm(bnrbuf,4)==GMAKEID('B','N','R','1'))
                ok = 100;    /* it's a banner. */
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL BNR_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','B','N','R');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL BNR_info(GINSTANCE *gx, int framenum)
{
    BNRHeaderStruct bnrheader;
    GINFO *ginfo=0;
    int ok=0;

    gseek(gx->gstream,(GPOS)0);
    memset(&bnrheader,0,sizeof(bnrheader));
    ok = gread(gx->gstream, (char *) &bnrheader, 6496)==6496;   /* read file */
    if (ok)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','B','N','R');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;            /* Current frame num */
            ginfo->width            = width;        /* Width of bitmap in pixels */
            ginfo->height           = height;       /* Height of bitmap in pixels */
            ginfo->bpp              = 32;           /* Height of bitmap in pixels */
            ginfo->originalbpp      = 15;           /* Height of bitmap in pixels */
            ginfo->framesize        = 0;
            ginfo->subtype          = 0;            /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 3;            /* Number of bits in alpha channel */
            ginfo->redbits          = 5;            /* Number of bits in red channel */
            ginfo->greenbits        = 5;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 5;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            strncpy(ginfo->framename, bnrheader.longTitle, gmin(sizeof(ginfo->framename)-1,sizeof(bnrheader.longTitle)));
            strncpy(ginfo->comment, bnrheader.comment, gmin(sizeof(ginfo->comment)-1,sizeof(bnrheader.comment)));
            ok = 1;
        }
    }

    return(ginfo);
}

int GCALL BNR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int pixoffset;
    char *d;
    int ok;
    BNRHeaderStruct bnrheader;

    gseek(gx->gstream,(GPOS)0);
    memset(&bnrheader,0,sizeof(bnrheader));
    ok = gread(gx->gstream, (char *) &bnrheader, 6496)==6496;   /* read file */
    if (ok)
    {
        for (y=0; y<height; ++y)
        {
            d = dest;
            for (x=0; x<width; ++x)
            {
                pixoffset = BNR_SwizzleGetPixelOffset16(x,y,width);
                *(int *)(d) = BNRRpixel5To8(ggetm((const unsigned char *)&bnrheader.image[pixoffset],2));
                d += 4;
            }
            dest += rowbytes;
        }
    }
    return(ok);
}

int GCALL BNR_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

