// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TIFWRITE
#define __TIFWRITE 1

#include <string.h>
#include "gimex.h"
#include "tifgimex.h"
#include "tiffio.h"
#include "tiffiop.h"
#include "tif_dir.h"


/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL TIF_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;
    TIFF *tif;

    gseek(g,(GPOS)0);
    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;

        tif = TIFFOpen(pathname, g, "w");
        if (tif)
        {
            gx->gref = (char *) tif;
            *gxptr = gx;
            ok = 1;
        }
        else
            gfree(gx);
    }
    return(ok);
}

int GCALL TIF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    TIFF *tif=(TIFF *)gx->gref;
    int width;
    int height;
    int ok=0;
    int x;
    int y;
    int alpha;
    char *linebuf;
    char *s;
    char *d;

    if (tif)
    {
        if (ginfo->bpp>8)
        {
            width = ginfo->width;
            height = ginfo->height;
#if defined(__i386__)
            alpha = 1;  // libtiff bug requires us to use 32 for now
#else
            alpha = ginfo->alphabits>2?1:0;
#endif
            TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,  width);
            TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
            TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, alpha?4:3);
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
            TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
            if (alpha)
            {
                int v[1];
                v[0] = EXTRASAMPLE_UNASSALPHA;
                TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, (uint16 *) &v[0]);
            }
            if (ginfo->dpi)
            {
                TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
                TIFFSetField(tif, TIFFTAG_XRESOLUTION, ginfo->dpi);
                TIFFSetField(tif, TIFFTAG_YRESOLUTION, ginfo->dpi);
            }

            TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
            TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif,-1));

            linebuf = galloc(width*4);
            if (linebuf)
            {
                ok = 1;
                for (y=0; y<height && ok; ++y)
                {
                    s = source+y*rowbytes;
                    d = linebuf;

                    for (x=0; x<width; ++x) // ARGB -> rgba
                    {
#if defined(__i386__)
                        if (alpha)
                        {
                            d[0] = ((ARGB *)s)->g;
                            d[1] = ((ARGB *)s)->r;
                            d[2] = ((ARGB *)s)->a;
                            d[3] = ((ARGB *)s)->b;
                            d += 4;
                        }
                        else
                        {
                            d[0] = ((ARGB *)s)->r;
                            d[1] = ((ARGB *)s)->g;
                            d[2] = ((ARGB *)s)->b;
                            d += 3;
                        }
#else
                        if (alpha)
                        {
                            d[0] = ((ARGB *)s)->r;
                            d[1] = ((ARGB *)s)->g;
                            d[2] = ((ARGB *)s)->b;
                            d[3] = ((ARGB *)s)->a;
                            d += 4;
                        }
                        else
                        {
                            d[0] = ((ARGB *)s)->r;
                            d[1] = ((ARGB *)s)->g;
                            d[2] = ((ARGB *)s)->b;
                            d += 3;
                        }
#endif

                        s += 4;
                    }

                    if (TIFFWriteScanline(tif, linebuf, y, 0) < 0)
                        ok = 0;
                }

                gfree(linebuf);
            }
        }
    }

    return(ok);
}

int GCALL TIF_wclose(GINSTANCE *gx)
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

#endif


