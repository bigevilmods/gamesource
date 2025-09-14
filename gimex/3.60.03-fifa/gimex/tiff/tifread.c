// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                         TIF - TIFF Gimex                         */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 961018 FB based on tga,mpg & tiff library                */
/*    1.01 980305 KO numcolours was always 0                        */
/*    1.02 980506 KO ginfo.import/export --> canimport/canexport    */
/*    1.03 980519 KO originalbpp > 8 since bpp always 32            */
/*    1.04 990823 FB dpi                                            */
/*    1.05 001017 FB upgraded from 3.4033 to 3.5.5                  */
/*    1.06 010122 FB TIF_is check version as well                   */
/*    1.07 010124 FB multiframe (TIFFSetDirectory)                  */
/*    1.08 010914 ID fixed photoshop tiff files and transparency    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1,4,8,32 bit import/export                                       */
/* compresses/decompresses                                          */
/* Reentrant?                                                       */
/* Files: tifread.c tifwrite.c tifgimex.h tif_*.c tif*.h            */
/*                                                                  */
/* Tiff library is 3.5.5                                            */
/*   http://www.libtiff.org/                                        */
/*                                                                  */
/* Changes to libtiff                                               */
/* fprintf,vfprintf changed to ferrprintf vferrprintf and stubbed   */
/* __APPLE__ ifdefs added for all __unix__ references               */
/* port.h hand tweeked for osx/irix                                 */
/* added GSTREAM to TIFFOpen                                        */
/* tiffcomp.h added little endian compiler checks to end            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* TIF files support 1,4,8,24,32 bit depths.                        */
/* Runlength,LZW,CCITT Fax3 compression                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "tifgimex.h"
#include "tiffio.h"
#include "tiffiop.h"
#include "tif_dir.h"


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL TIF_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID(0,'T','G','A');
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
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;
        info->mactype[0]      = GMAKEID('T','I','F','F');   /* mac file system types used */
        strcpy(info->extensions[0], ".tif");                /* extensions */
        strcpy(info->extensions[1], ".tiff");               /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.08");                /* version # */
        strcpy(info->shorttypestr,  "TIF");                 /* type */
        strcpy(info->wordtypestr,   "Tiff");                /* wordtype */
        strcpy(info->longtypestr,   "Tiff");                /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* 'MM' or 'II' id */

int GCALL TIF_is(GSTREAM *g)
{
    char tifbuf[4];
    int ok=0;

    gseek(g, 0);
    if (gread(g, tifbuf, 4))
    {
        if (ggetm(tifbuf,4)==0x49492a00
         || ggetm(tifbuf,4)==0x4d4d002a)
            ok = 100;    /* it's a tiff */
    }
    return(ok);
}



/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL TIF_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    TIFF *tif;

    gseek(g,(GPOS)0);
    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID(0,'T','G','A');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        tif = TIFFOpen(pathname, g, "r");
        if (tif)
        {
            int pages = (int) TIFFNumberOfDirectories(tif);

            gx->gref = (char *) tif;
            *gxptr = gx;
            ok = pages;
        }
        else
            gfree(gx);
    }
    return(ok);
}

GINFO * GCALL TIF_info(GINSTANCE *gx, int framenum)
{
    int width;
    int height;
    int resolutionunit=0;
    float dpi=0.0f;
    float resolutionx;
    int ok=0;
    int channels=3;
    int bitsperchannel=8;
    int bpp;
    GINFO *ginfo=0;
    TIFF *tif=(TIFF *)gx->gref;
    register TIFFDirectory *td;

    if (tif)
    {
                TIFFSetDirectory(tif,(uint16)framenum);
        td = &tif->tif_dir;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

        TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &resolutionunit);
        TIFFGetField(tif, TIFFTAG_XRESOLUTION, &resolutionx);
        if (resolutionunit==RESUNIT_CENTIMETER)
        {
            dpi = (float)((double)resolutionx*100.0 * 0.039370079);
        }
        else if (resolutionunit==RESUNIT_INCH)
        {
            dpi = (float)(resolutionx);
        }
        else if (resolutionunit!=RESUNIT_INCH)
            dpi = 0.0f;

        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &resolutionx);
        if (TIFFFieldSet(tif,FIELD_BITSPERSAMPLE))
            bitsperchannel = td->td_bitspersample;
        if (TIFFFieldSet(tif,FIELD_SAMPLESPERPIXEL))
            channels = td->td_samplesperpixel;
        bpp = channels*bitsperchannel;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('T','I','F','F'); /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Current frame num */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->framesize        = 0;
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100;             /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = channels>3?8:0;  /* Number of bits in alpha channel */
            ginfo->redbits          = bitsperchannel;  /* Number of bits in red channel */
            ginfo->greenbits        = bitsperchannel;  /* Number of bits in grreen channel */
            ginfo->bluebits         = bitsperchannel;  /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
            ginfo->bpp              = 32;           /* Bits per pixel (8 or 32) */
            ginfo->dpi              = dpi;

            /* Never set originalbpp to <= 8 if bpp = 32 */
            /* readtif cannot handle palettes so ginfo should not either */
            ginfo->originalbpp      = channels==1?24:bpp;
            ginfo->numcolours       = 0; /* bpp>8?0:1<<bpp; */           /* Number of colours in original indexed palette */
            ok = 1;
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL TIF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int width;
    int height;
    int framesize;
    char *frame;
    int ok=0;
    TIFF *tif=(TIFF *)gx->gref;

    width = ginfo->width;
    height = ginfo->height;

    framesize = width*4*height;
    frame = galloc(framesize);
    if (frame)
    {
        if (TIFFReadRGBAImage(tif, width, height, (uint32 *)frame, 0))
        {
            int x,y;
            char *s;
            char *d;

            for (y=0; y<height; ++y)
            {
                s = (frame)+(height-1-y)*width*4;
                d = (dest)+y*rowbytes;

                for (x=0; x<width; ++x) // rgba -> ARGB
                {
		    //#if defined(__i386__)
		    //((ARGB *)d)->g = s[0];
                    //((ARGB *)d)->r = s[1];
                    //((ARGB *)d)->a = s[2];
                    //((ARGB *)d)->b = s[3];
		    //#else
                    ((ARGB *)d)->r = s[0];
                    ((ARGB *)d)->g = s[1];
                    ((ARGB *)d)->b = s[2];
                    ((ARGB *)d)->a = s[3];
		    //#endif
                    d += 4;
                    s += 4;
                }
            }
            ok = 1;
        }
        gfree(frame);
    }
    return(ok);
}

int GCALL TIF_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if (gx->gref)
            TIFFClose((TIFF *)gx->gref);

        ok = gfree((void *) gx);
    }

    return(ok);
}

