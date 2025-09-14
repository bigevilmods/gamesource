// (c) 1999-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*            WCT - Wavelet Compressed Texture Gimex                */
/*                                                                  */
/*       by Mark Feldman, EAUK and FrANK G. Barchard, EAC           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 990906 FB initial version based on SCT and Wavcomp       */
/*    1.01 990908 FB exporter                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: wctread.cpp wctwrite.cpp wctgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* WCT files are from ThemePark (EAUK).                             */
/* There is one texture per file.                                   */
/* 4th-order wavelet transform                                      */
/* YCbCr is used, with subsampling.                                 */
/*                                                                  */
/*	Images are converted from RGB to YPbPr using:                   */
/*                                                                  */
/*	|Y | = | 0.299      0.587      0.114   | |R|                    */
/*	|Pb| = |-0.168736  -0.331264   0.5     | |G|                    */
/*	|Pr| = | 0.5       -0.418688  -0.081312| |B|                    */
/*                                                                  */
/*  Images are converted from YPbPr to RGB using:                   */
/*                                                                  */
/*	|R| = | 1.0   0.         1.402    | |Y |                        */
/*	|G| = | 1.0  -0.344136  -0.714136 | |Pb|                        */
/*	|B| = | 1.0   1.772      0.0      | |Pr|                        */
/*                                                                  */
/* This is a dump of the header on an 24 bit example.               */
/*                                                                  */
/* 00000: 12001801 8002e001 06000a00 02001400  ......`.........     */
/* 00010: 13310200 00000000 6b7e82ff 42494c5a  .1......k...BILZ     */
/* 00020: 00001800 13310200 0f000000 09000000  .....1..........     */
/* 00030: 00000000 00000000 789cecbd 057b5be9  ........x.l=..[i     */
/*                                                                  */
/* typedef struct                                                   */
/* {                                                                */
/*     Wavelet FILE HEADER structure                                */
/*                                                                  */
/*     char version;    0x12                                        */
/*     char flags;                                                  */
/*     char bpp;                                                    */
/*     char mipmaps;     4 bytes                                    */
/*                                                                  */
/*     char width[2];                                               */
/*     char height[2];     8 bytes                                  */
/*     char yquant[2];                                              */
/*     char prquant[2];     12 bytes                                */
/*     char pbquant[2];                                             */
/*     char alphaquant[2];    16 bytes                              */
/*                                                                  */
/*     char rgbsize[4];                                             */
/*     char alphasize[4];     24 bytes                              */
/*                                                                  */
/*     char blue,green,red,alpha;                                   */
/*     char zlibsignature[4];     'ZLIB'                            */
/* } WCTHeaderStruct;                                               */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gimex.h"
#include "wctgimex.h"
#include "wavelet.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL WCT_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','W','C','T');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
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
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 80;   /* default pack quality */
        info->mactype[0]      = GMAKEID('.','W','C','T');   /* mac file system types used */
        strcpy(info->extensions[0], ".wct");                /* extensions */
        strcpy(info->authorstr,     "Mark Feldman");        /* author */
        strcpy(info->versionstr,    "1.01");                /* version # */
        strcpy(info->shorttypestr,  "WCT");                 /* type */
        strcpy(info->wordtypestr,   "WCT");                 /* wordtype */
        strcpy(info->longtypestr,   "Wavelet Compressed Texture"); /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* signature 'ZLIB' at 0x1c and version is 0x12
/* 00000: 12001801 8002e001 06000a00 02001400  ......`.........     */
/* 00010: 13310200 00000000 6b7e82ff 42494c5a  .1......k...BILZ     */
/* 00020: 00001800 13310200 0f000000 09000000  .....1..........     */
/* 00030: 00000000 00000000 789cecbd 057b5be9  ........x.l=..[i     */

int GCALL WCT_is(GSTREAM *g)
{
    WCTHeaderStruct wctheader;
    int flen;
    int ok=0;

    flen = glen(g);
    if (flen>32)
    {
        gseek(g,(GPOS)0);
        if (gread(g,&wctheader,32))
        {
            if (WCTGETFIELD(wctheader.zlibsignature)==GMAKEID('Z','L','I','B'))
            {
                if (WCTGETFIELD(wctheader.version)>=0x10)
                    ok = 10;
                if (WCTGETFIELD(wctheader.version)>=0x11)
                    ok = 50;
                if (WCTGETFIELD(wctheader.version)==0x12)
                    ok = 100;
            }
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL WCT_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','W','C','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL WCT_info(GINSTANCE *gx, int framenum)
{
    WCTHeaderStruct wctheader;
    GINFO *ginfo=0;
    int ok=0;
    int bpp;
    int quant;
    int width;
    int height;

    gseek(gx->gstream,(GPOS)0);
    ok = gread(gx->gstream, &wctheader, 32);   /* read header */
    if (ok)
    {
        width       = WCTGETFIELD(wctheader.width);   /* Width of bitmap in pixels */
        height      = WCTGETFIELD(wctheader.height);  /* Height of bitmap in pixels */
        bpp         = WCTGETFIELD(wctheader.bpp) ;    /* Colour bits per pixel */
        quant       = WCTGETFIELD(wctheader.yquant);  // Luminence quantize (quality)

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','W','C','T');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Current frame num */
            ginfo->width            = width;             /* Width of bitmap in pixels */
            ginfo->height           = height;            /* Height of bitmap in pixels */
            ginfo->framesize        = 0;
            ginfo->subtype          = 0;                 /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100-quant;         /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = gmax(0,bpp-24);    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
            ginfo->bpp              = 32;           /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours       = 0;        /* Number of colours in original indexed palette */
            ginfo->dpi              = 0.0f;

        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}



int GCALL WCT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    WCTHeaderStruct wctheader;
    int width=ginfo->width;
    int bpl;
    int x;
    int y;
    int ok=0;
    int fsize;
    int m_nrgbsize;
    int m_nalphasize;
    int m_nYQuant;
    int m_nPrQuant;
    int m_nPbQuant;
    int m_nAlphaQuant;
    void *s;
    char *dest24;
    ARGB *d;

    dest24 = (char *) galloc(ginfo->width*3*ginfo->height);
    if (dest24)
    {
        gseek(gx->gstream,(GPOS)0);
        ok = gread(gx->gstream, &wctheader, 28);   /* read header */
        if (ok)
        {
            m_nYQuant     = WCTGETFIELD(wctheader.yquant);
            m_nPrQuant    = WCTGETFIELD(wctheader.prquant);
            m_nPbQuant    = WCTGETFIELD(wctheader.pbquant);
            m_nAlphaQuant = WCTGETFIELD(wctheader.alphaquant);
            m_nrgbsize    = WCTGETFIELD(wctheader.rgbsize);
            m_nalphasize  = WCTGETFIELD(wctheader.alphasize);

            fsize = glen(gx->gstream)-28;
            s = galloc(fsize);
            if (s)
            {
                ok = gread(gx->gstream,s,fsize);
                if (ok)
                {
            		// Convert to something we can use
            		float fYQuant     = 1.0f + 50.0f * (float)m_nYQuant     / 100.0f;
            		float fPrQuant    = 1.0f + 25.0f * (float)m_nPrQuant    / 100.0f;
            		float fPbQuant    = 1.0f + 25.0f * (float)m_nPbQuant    / 100.0f;
            		float fAlphaQuant = 1.0f + 100.0f * (float)m_nAlphaQuant / 100.0f;

                	CWavelet * pWavelet;
                	pWavelet = CreateWavelet();
                    if (pWavelet)
                    {
                        char *s1;

                    	pWavelet->Decompress(ginfo->width,ginfo->height, true, (const unsigned char *) s, (unsigned char *)dest24, fYQuant, fPbQuant, fPrQuant);

                        s1 = dest24;
                        for (y=0; y<ginfo->height && ok; ++y)
                        {
                            d = (ARGB *) (dest+rowbytes*y);

                            for (x=0; x<width; ++x)
                            {
                                d->a = 255;
                                d->r = s1[2];
                                d->g = s1[1];
                                d->b = s1[0];

                                s1 += 3;
                                ++d;
                            }
                        }
                        if (m_nalphasize)
                        {
                        	pWavelet->Decompress(ginfo->width,ginfo->height, false, (const unsigned char *) s+m_nrgbsize, (unsigned char *)dest24, fAlphaQuant, fAlphaQuant, fAlphaQuant);

                            s1 = dest24;
                            for (y=0; y<ginfo->height && ok; ++y)
                            {
                                d = (ARGB *) (dest+rowbytes*y);

                                for (x=0; x<width; ++x)
                                {
                                    d->a = *s1;
                                    ++s1;
                                    ++d;
                                }
                            }
                        }

                    	pWavelet->Release();
                    }
                }
                gfree(s);
            }
        }

        gfree(dest24);
    }

    return(ok);
}

int GCALL WCT_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

