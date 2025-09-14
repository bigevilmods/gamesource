// (c) 1994-2007 Electronic Arts Inc.

#ifndef __JPGWRITE
#define __JPGWRITE 1

#include <string.h>
#include <stdio.h>
#include "gimex.h"
#include "jpeglib.h"
#include "jpggimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int isgreyscale(ARGB *pal, int colours)
{
    int i;
    int ok=1;

    for (i=0; i<colours; ++i)
    {
        if (pal[i].r!=pal[i].g || pal[i].r!=pal[i].b)
        {
            ok = 0;
            break;
        }
    }

    return(ok);
}

void JPGwriteline(char *source, char *dest, int width, int colourspace, const GINFO *ginfo)
{
    unsigned char *d;
    unsigned char *s;
    int i;

    d = (unsigned char *) dest;
    s = (unsigned char *) source;

    switch (colourspace)
    {
        case JCS_RGB:
            if (ginfo->bpp == 32)   // 32 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *(d+0) = ((ARGB*)s)->r;
                    *(d+1) = ((ARGB*)s)->g;
                    *(d+2) = ((ARGB*)s)->b;
                    d+=3;
                    s+=sizeof(ARGB);
                }
            }
            else                    // 8 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *(d+0) = ginfo->colourtbl[*s].r;
                    *(d+1) = ginfo->colourtbl[*s].g;
                    *(d+2) = ginfo->colourtbl[*s].b;
                    d+=3;
                    s+=1;
                }
            }
            break;

        case JCS_GRAYSCALE:
            if (ginfo->bpp == 32)   // 32 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *d = ((ARGB*)s)->g;
                    d+=1;
                    s+=sizeof(ARGB);
                }
            }
            else                    // 8 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *d = ginfo->colourtbl[*s].g;
                    d+=1;
                    s+=1;
                }
            }
            break;

        case JCS_UNKNOWN:           // Use alpha
			if (ginfo->bpp == 32 && ginfo->alphabits>=1)   // 32 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *(d+0) = ((ARGB*)s)->a;
                    *(d+1) = ((ARGB*)s)->r;
                    *(d+2) = ((ARGB*)s)->g;
                    *(d+3) = ((ARGB*)s)->b;
                    d+=4;
                    s+=4;
                }
            }
			else if (ginfo->bpp == 32 && ginfo->alphabits==0)   // 24 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *(d+0) = ((ARGB*)s)->r;
                    *(d+1) = ((ARGB*)s)->g;
                    *(d+2) = ((ARGB*)s)->b;
                    d+=3;
                    s+=sizeof(ARGB);
                }
            }			
			
			else                    // 8 bpp
            {
                for (i=0; i<width; ++i)
                {
                    *(d+0) = ginfo->colourtbl[*s].a;
                    *(d+1) = ginfo->colourtbl[*s].r;
                    *(d+2) = ginfo->colourtbl[*s].g;
                    *(d+3) = ginfo->colourtbl[*s].b;
                    d+=4;
                    s+=1;
                }
            }
            break;
    }
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL JPG_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL JPG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int quality = ginfo->quality;
    int image_width=ginfo->width;
    int image_height=ginfo->height;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
    int row_stride;             /* physical row width in image buffer */
    char *linebuf;
    int y;
    int ok=0;
    int channels=3;
    J_COLOR_SPACE cspacein=JCS_RGB;
    J_COLOR_SPACE cspaceout=JCS_YCbCr;

    if (ginfo->subtype & JPGARGB)      // Always use alpha
    {
        if (ginfo->alphabits>=1)
			channels = 4;
        cspaceout  = JCS_RGB;
//        cspacein   = JCS_UNKNOWN;
    }
    else
    {
        /* Determine if greyscale */
        if (ginfo->bpp == 8)    // 8 bit
        {
            if (isgreyscale((ARGB*)ginfo->colourtbl, ginfo->numcolours))
            {
                channels = 1;
                cspacein = JCS_GRAYSCALE;
                cspaceout = JCS_GRAYSCALE;
			}
        }
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo, gx->gstream);

    cinfo.image_width = image_width;    /* image width and height, in pixels */
    cinfo.image_height = image_height;
    cinfo.input_components = channels;  /* # of color components per pixel */
    cinfo.in_color_space = cspacein;     /* colorspace of input image */
    cinfo.jpeg_color_space = cspaceout;  /* colorspace of output image */

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    if (ginfo->dpi)
    {
        cinfo.density_unit = 1;
        cinfo.X_density = (unsigned short) ginfo->dpi;
        cinfo.Y_density = (unsigned short) ginfo->dpi;
    }
    else
    {
        cinfo.density_unit = 0;
        cinfo.X_density = 1;
        cinfo.Y_density = 1;
    }

    // If the subtype is JPGNODHT, then do not emit the DHT when writing the JPEG file.
    if (ginfo->subtype & JPGNODHT)
        cinfo.emit_dht = FALSE;

    jpeg_start_compress(&cinfo, TRUE);

    if (ginfo->subtype & JPGARGB)
    {
        jpeg_write_marker(&cinfo, JPGGIMEXMARKERNUMBER, JPGGIMEXMARKER, JPGGIMEXMARKERSIZE);
    }

    row_stride = image_width * channels;       /* JSAMPLEs per row in image_buffer */
    linebuf = galloc(row_stride);
    if (linebuf)
    {
        for (y=0; y<(int)cinfo.image_height; ++y)
        {
            JPGwriteline(source+y*rowbytes, linebuf, image_width, cspacein, ginfo);
            row_pointer[0] = (unsigned char*)linebuf;
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        gfree(linebuf);
        ok = 1;
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    return(ok);
}

int GCALL JPG_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

#endif

