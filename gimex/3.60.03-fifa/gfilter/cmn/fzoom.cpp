// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Zoom - Bicubic Resampling Zoom Filter                */
/*                                                                  */
/*    by Spencer Cathey, Gerry Shaw and FrANK G. Barchard, EAC      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950508 SC initial version                                */
/*    1.01 950512 SC added filter control, edge colour              */
/*    1.02 951006 FB started scale hot spots/center                 */
/*    1.03 951101 GS center point and hotspots scale correctly      */
/*    1.04 951107 FB updated to 1.04 API and name conventions       */
/*    1.05 951220 FB fix over sampling                              */
/*    1.06 000329 FB position scaled                                */
/*    1.07 010118 ID zoom each layer wrt bottom layer on page type. */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* zoom(int width, int height, int edgeextend, int colour,          */
/*      float kernalwidth, float kernalshape)                       */
/*                                                                  */
/* int width;          width to scale to in pixels                  */
/* int height;         height to scale to in pixels                 */
/* int edgeextend;     0 use colour, 1 use edge pixel               */
/* int colour;         colour to extend edge with                   */
/* float kernalwidth;  width of sampling kernal                     */
/* float kernalshape;  shape of sampling kernal                     */
/*                                                                  */
/* Bicubic resample image to new size.                              */
/*                                                                  */
/* Notes                                                            */
/* should update to make fops optional.                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fzoom.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

GBITMAP *imageclone(GBITMAP *source, int newwidth, int newheight);

/* #define ZOOM_DEBUG1 */
/* #define ZOOM_FILE */

#define KERNEL_ENTRIES 4096

#define Z_ARGS          6

#define Z_WIDTH         0
#define Z_HEIGHT        1
#define Z_EDGE_TYPE     2
#define Z_EDGE_COLOUR   3
#define Z_FILTER_WIDTH  4
#define Z_FILTER_SHAPE  5
#define Z_MULTIFRAME_WIDTH 6   // not inside array but inside options possibly...
#define Z_MULTIFRAME_HEIGHT  7
#define Z_TYPE          8


typedef struct tag_filterdata
{
    short*      kernel;
    FIXZ        filterwidth;
    /* image copied, scaled in-place, copied to destination */
    GBITMAP *   imagecopy;
    FOP         defargs[Z_ARGS];
} FILTERDATA;

/* clones an image into the lower left corner of the new image */
/* NOTE: the old image must fit inside the new width and height */

GBITMAP *imageclone(GBITMAP *source, int newwidth, int newheight)
{
    GBITMAP *   gb = NULL;
    GINFO *     gi;
    char *      picture;
    int        imagebytes;
    int         newrowbytes = newwidth*(source->ginfo->bpp)/8;
    /* remove the padding from rowbytes, call it oldrowsize */
//    int         oldrowsize = source->ginfo->width * (source->ginfo->bpp)/8;
    int         row;

    if (source->ginfo->width > newwidth ||
         source->ginfo->height > newheight)
    {
        return(gb);
    }
    imagebytes = newheight*newrowbytes;
    picture = (char *)galloc(imagebytes);
    if (!picture) goto EXITCLONE;

    gi = (GINFO *)galloc(source->ginfo->size);
    if (!gi)
    {
        gfree(picture);
        goto EXITCLONE;
    }

    gb = (GBITMAP *)galloc(sizeof(GBITMAP));
    if (!gb)
    {
        gfree(gi);
        gfree(picture);
        goto EXITCLONE;
    }

    /* init GBITMAP structure */

    gb->rowbytes = newrowbytes;
    gb->ginfo = gi;
    gb->image = picture;

    /* copy the GINFO structure, with new width and height */
    memcpy(gi, source->ginfo, source->ginfo->size);
    gi->width = newwidth;
    gi->height = newheight;

    /* copy the image, noting the old rowbytes */
    for (row = 0; row < source->ginfo->height; row++)
    {
        memcpy(picture+row*newrowbytes,
                (char *)(source->image) + row*source->rowbytes,
                source->rowbytes);
    }

EXITCLONE:;
    return(gb);
}


/******************* zoom implementation ****************/

/*
    This zoom function does direct convolution with some modest improvements.
    The essence of this technique is to use a lookup table for the filter
    kernel, so that user can have better control over the appearance of
    extreme zoom ratios and use a consistent form of the kernel for scale
    factors above and below 1.0 so that there's no DC shift or detail
    aberrations as the scale factor is animated through unity zoom.
    There are faster computational forms, but none with substantially
    more quality. I can look into faster later.

    features:
        requires no floating point arithmetic
        source center specified with sub-pixel accuracy
        output cropping, no unecessary calculation
        zoom factor from 0.001 to 1022.999
        support both planar and RGBA raster organization
        consistent computation for scales above and below unity,

    scale factor and positions are specified with 32-bit integers, where
    a presumed decimal point makes 10 bits of fraction.  So if you want
    a scale factor of 1.0, use (1 << 10)

    The lookup table could have less members, but then to accurately
    sample it, some kind of interpolation must be done.  So one speed
    up is to have a table with lots of entries, and no interpolation.

    The distance in an un-scaled raster from one pixel to the next
    is equal to FIXZ_VALUE, which is 1024.

    NOTICE: THIS MAY BE WEIRD TO YOU, but this works with a model
    of a raster that is interested in the space between the pixels.
    so 3 pixels scaled by 2.0 makes 5 pixels    *  *  *
                                                *  *  *  *  *
                                        (see how two areas become four)
    this means to turn 3 pixels to 6 pixels requires a scaling
    factor more than 2.0, and there is a wrapper function that
    figures it out(along with compensating for quantized integer
    scaling).  This might seem strange, but there are an infinite
    number of scale factors near 2.0 that can make 3 pixels into
    6, so whats sacred about 2.0?  So this will do what most people
    want, except guarantee impulse symmetry at integral scale factors.

*/
#if defined (ZOOM_DEBUG)
#include <stdio.h>
#endif

#if defined(ZOOM_DEBUG)
#define KERNEL_ENTRIES (FIXZ_VALUE*4)     /* MUST BE BINARY POWER */
short   anKernel[KERNEL_ENTRIES];
#endif

/*
    FIXZ lZoomFactor(int numsrcpixels, int numdstpixels)

    returns a scale factor suitably prepared for iScanZoom,
    given the number of source pixels and destination pixels.
*/
FIXZ lZoomFactor(int numsrcpixels, int numdstpixels)
{
    return(((FIXZ)(numdstpixels-1)*FIXZ_VALUE + FIXZ_VALUE/2)/(FIXZ)(gmax(1,numsrcpixels-1)));
}
/*
    arguments:

    int iScanZoom(BYTE* pbySrc,
                    FIXZ lSrcCenter,
                    WORD  wSrcPixelPitch,
                    short nSrcPixels,
                    BYTE* pbyDst,
                    WORD  wDstPixelPitch,
                    short nDstPixels,
                    FIXZ lScale,
                    FIXZ lKernelWidth,
                    WORD wKernelSamples,
                    short* pnKernel );

    return: number of pixels written to pbyDst, or 0 for bad arguments

    pbySrc is the source scanline
    lSrcCenter is the point in the source buffer that will appear in
        the center of the destination buffer, specified like dwScale,
        with 10 bits of fraction
    wSrcPixelPitch is the offset in bytes from one sample to the next, so
        if an RGBA buffer is used and we are zooming a row, the value is 4,
        and if zooming a column the value is the number of bytes per row.
        If we have a planar buffer and zooming a row, the value is 1.
    nSrcPixels is the count of input pixels
    pbyDst is the destination scanline
    wDstPixelPitch is the destination pitch, see wSrcPixelPitch above.
    nDstPixels is the count of output pixels
    lScale is the scale factor as an integer with 10 bits of fraction. If
        a scale factor of 1.243 is desired, use (DWORD)(1.243 * 1024.0).
    lKernelWidth is the number of unscaled pixels that are inside the
        positive half of the kernel, typically 2, range from 1/1024 to ???
    wKernelSamples is the number of entries in the kernel table, currently
        equal to the constant KERNEL_ENTRIES, but the kernel memory
        allocation may change
    pnKernel is a pointer to the filter lookup table

*/

int iScanZoom(BYTE* pbySrc,
            FIXZ  lSrcCenter,
            WORD  wSrcPixelPitch,
            short nSrcPixels,
            BYTE* pbyDst,
            WORD  wDstPixelPitch,
            short nDstPixels,
            FIXZ  lScale,
            FIXZ  lKernelWidth,
            WORD wKernelSamples,
            short* pnKernel)
{
    int ok = 0;    /* return value */
    int lOutPixels;    /* number of pixels written to output */
    FIXZ lDstCenter;    /* fractional destination array center  */
    int lIndex;        /* output pixel loop counter */
    FIXZ lFIndex;       /* scaled copy of lIndex */
    int lKernelInc;    /* how many filter indices make a pixel */
    int lFilterOffset; /* kernel index of nearest pixel to source */
    WORD wKernelMiddle = (WORD)(wKernelSamples/2);
    int lLastSrcOffset = (int)nSrcPixels*wSrcPixelPitch;

    if (wKernelSamples < 128 ||     /* not tested yet at these values */
        lScale <= 1 ||
        lKernelWidth < 64 ||
        nSrcPixels < 2 ||
        nDstPixels < 2 )
    {
        return(ok);
    }

    lKernelInc = (int)wKernelSamples*FIXZ_VALUE;
    lKernelInc /= lKernelWidth*2;
    if (lScale < FIXZ_VALUE)
    {
        lKernelInc *= lScale;
        lKernelInc /= FIXZ_VALUE;
    }
#if defined(ZOOM_DEBUG1)
printf("lKernelInc:%d\n",lKernelInc);
#endif

    lOutPixels = (FIXZ)(nSrcPixels - 1)*lScale;
    lOutPixels = lOutPixels / FIXZ_VALUE + 1;
    /* if cropped output, place source center in destination center */
    /* otherwise the output is left justified in the destination array */
    if (lOutPixels > nDstPixels)
        lOutPixels = (int)nDstPixels;

    lDstCenter = (lOutPixels-1) << (FIXZ_SHIFT-1);

    for (lIndex = 0, lFIndex = 0;   /* for every output pixel */
         lIndex < lOutPixels;
         lIndex++, lFIndex += FIXZ_VALUE)
    {
        int lSrcPixel; /* source pixel index in source array */
        int lSrcOffset;

        /* project current output pixel into source array */
        FIXZ lSrcPixelCtr = (lFIndex - lDstCenter) * FIXZ_VALUE;
        lSrcPixelCtr /= lScale;
        lSrcPixelCtr += lSrcCenter;

#if defined (ZOOM_DEBUG1)
printf("%d lSrcPixelCtr:%lf  lFIndex:%lf\n",
        lIndex,(double)lSrcPixelCtr/FIXZ_VALUE, (double)lFIndex/FIXZ_VALUE);
#endif

        /* find nearest source pixel's kernel index */
        lSrcOffset = lSrcPixelCtr & FIXZ_MASK;
        if (lSrcOffset <= FIXZ_VALUE/2)
        {
            lSrcOffset *= lKernelInc;
            lSrcOffset >>= FIXZ_SHIFT;
            lFilterOffset = wKernelMiddle - lSrcOffset;
            lSrcPixel = lSrcPixelCtr >> FIXZ_SHIFT;
        }
        else
        {
            lSrcOffset = FIXZ_VALUE - lSrcOffset;
            lSrcOffset *= lKernelInc;
            lSrcOffset >>= FIXZ_SHIFT;
            lFilterOffset = wKernelMiddle + lSrcOffset;
            lSrcPixel = (lSrcPixelCtr >> FIXZ_SHIFT) + 1;
        }
        lSrcPixel *= wSrcPixelPitch;

#if defined (ZOOM_DEBUG1)
printf("outpix %d\n",lIndex);
#endif

        {   /* convolution block */

            int lKIndex = lFilterOffset;   /* copy, then increment */
            int lSrcPix = lSrcPixel;       /* copy, then increment */
            FIXZ lSum = 0;          /* sum of pixel and kernel products */
            FIXZ lKSum = 0;         /* sum of kernel coefficients */

            /* could be restructured to eliminate conditionals in loop */

            /* head negative, until kernel runs out */
            while (lKIndex >= 0)
            {
                int coeff = pnKernel[lKIndex];
                int pixel;
                if (lSrcPix >= 0)
                    pixel = pbySrc[lSrcPix];
                else
                    pixel = 0;

#if defined(ZOOM_DEBUG1)
printf("lKIndex:%d lSrcPix:%d pixel:%d coeff:%d\n",
        lKIndex, lSrcPix, pixel, coeff);
#endif
                lKSum += coeff;
                lSum += coeff * pixel;
                lKIndex -= lKernelInc;
                lSrcPix -= wSrcPixelPitch;
            }
            /* head positive, until kernel runs out */
            lKIndex = lFilterOffset + lKernelInc;
            lSrcPix = lSrcPixel+wSrcPixelPitch;
            while (lKIndex < wKernelSamples)
            {
                int coeff = pnKernel[lKIndex];
                int pixel;
                if (lSrcPix < lLastSrcOffset)
                    pixel = pbySrc[lSrcPix];
                else
                    pixel = 0;

#if defined(ZOOM_DEBUG1)
printf("lKIndex:%d lSrcPix:%d pixel:%d coeff:%d\n",
        lKIndex, lSrcPix, pixel, coeff);
#endif
                lKSum += coeff;
                lSum += coeff * pixel;
                lKIndex += lKernelInc;
                lSrcPix += wSrcPixelPitch;
            }

            lSum += FIXZ_VALUE/2;   /* rounding */
            if (lKSum) lSum /= lKSum;
            /* clamp to ouput range */
            if (lSum < 0) lSum = 0;
            else if (lSum > 255) lSum = 255;

#if defined(ZOOM_DEBUG1)
printf("lSum:%d\n", lSum);
#endif

            *(pbyDst + lIndex*wDstPixelPitch) = (BYTE)lSum;
        } /* end of convolution block */

    } /* end of for each output pixel loop */

    return((int)lOutPixels);
}

/*
    iScanZoomARGB -
    derived from iScanZoom(), called identically, for pixels that are
    4 bytes wide.  Note that pixel pitch is 4 for adjacent pixels.
    This simply speeds up iScanZoom by not having to duplicate any
    calculations for ARGB images.  It also adds noise to the final
    image.
*/
int iScanZoomARGB(BYTE* pbySrc,
            FIXZ  lSrcCenter,
            WORD  wSrcPixelPitch,
            short nSrcPixels,
            BYTE* pbyDst,
            WORD  wDstPixelPitch,
            short nDstPixels,
            FIXZ  lScale,
            FIXZ  lKernelWidth,
            WORD wKernelSamples,
            short* pnKernel)
{
    int ok = 0;    /* return value */
    int lOutPixels;    /* number of pixels written to output */
    FIXZ lDstCenter;    /* fractional destination array center  */
    int lIndex;        /* output pixel loop counter */
    FIXZ lFIndex;       /* scaled copy of lIndex */
    int lKernelInc;    /* how many filter indices make a pixel */
    int lFilterOffset; /* kernel index of nearest pixel to source */
    WORD wKernelMiddle = (WORD)(wKernelSamples/2);
    int lLastSrcOffset = (int)nSrcPixels*wSrcPixelPitch;
    int lastsourceindex = lLastSrcOffset - wSrcPixelPitch;

    if (wKernelSamples < 128 ||     /* not tested yet at these values */
        lScale <= 1 ||
        lKernelWidth < 64 ||
        nSrcPixels < 2 ||
        nDstPixels < 2 )
    {
        return(ok);
    }

    lKernelInc = (int)wKernelSamples*FIXZ_VALUE;
    lKernelInc /= lKernelWidth*2;
    if (lScale < FIXZ_VALUE)
    {
        lKernelInc *= lScale;
        lKernelInc /= FIXZ_VALUE;
    }

    lOutPixels = (FIXZ)(nSrcPixels - 1)*lScale;
    lOutPixels = lOutPixels / FIXZ_VALUE + 1;
    /* if cropped output, place source center in destination center */
    /* otherwise the output is left justified in the destination array */
    if (lOutPixels > nDstPixels)
        lOutPixels = (int)nDstPixels;

    lDstCenter = (lOutPixels-1) << (FIXZ_SHIFT-1);

    for (lIndex = 0, lFIndex = 0;   /* for every output pixel */
         lIndex < lOutPixels;
         lIndex++, lFIndex += FIXZ_VALUE)
    {
        int lSrcPixel; /* source pixel index in source array */
        int lSrcOffset;

        /* project current output pixel into source array */
        FIXZ lSrcPixelCtr = (lFIndex - lDstCenter) * FIXZ_VALUE;
        lSrcPixelCtr /= lScale;
        lSrcPixelCtr += lSrcCenter;


        /* find nearest source pixel's kernel index */
        lSrcOffset = lSrcPixelCtr & FIXZ_MASK;
        if (lSrcOffset <= FIXZ_VALUE/2)
        {
            lSrcOffset *= lKernelInc;
            lSrcOffset >>= FIXZ_SHIFT;
            lFilterOffset = wKernelMiddle - lSrcOffset;
            lSrcPixel = lSrcPixelCtr >> FIXZ_SHIFT;
        }
        else
        {
            lSrcOffset = FIXZ_VALUE - lSrcOffset;
            lSrcOffset *= lKernelInc;
            lSrcOffset >>= FIXZ_SHIFT;
            lFilterOffset = wKernelMiddle + lSrcOffset;
            lSrcPixel = (lSrcPixelCtr >> FIXZ_SHIFT) + 1;
        }
        lSrcPixel *= wSrcPixelPitch;

        {   /* convolution block */

            int lKIndex = lFilterOffset;   /* copy, then increment */
            int lSrcPix = lSrcPixel;       /* copy, then increment */
            FIXZ lKSum = 0;        /* sum of kernel coefficients */
            FIXZ lASum = 0;        /* sum of pixel and kernel products */
            FIXZ lRSum = 0;
            FIXZ lGSum = 0;
            FIXZ lBSum = 0;

            /* could be restructured to eliminate conditionals in loop */

            /* head negative, until kernel runs out */
            while (lKIndex >= 0)
            {
                int coeff = pnKernel[lKIndex];
                int pixel;

                lKSum += coeff;

                if (lSrcPix >= 0)
                {
                    pixel = (int)(pbySrc[lSrcPix]);
                    lRSum += coeff * pixel;     /* may be red, doesn't matter */
                    pixel = (int)(pbySrc[lSrcPix+1]);
                    lGSum += coeff * pixel;
                    pixel = (int)(pbySrc[lSrcPix+2]);
                    lBSum += coeff * pixel;
                    pixel = (int)(pbySrc[lSrcPix+3]);
                    lASum += coeff * pixel;
                }
                else
                {
                    pixel = (int)(pbySrc[0]);
                    lRSum += coeff * pixel;     /* may be red, doesn't matter */
                    pixel = (int)(pbySrc[1]);
                    lGSum += coeff * pixel;
                    pixel = (int)(pbySrc[2]);
                    lBSum += coeff * pixel;
                    pixel = (int)(pbySrc[3]);
                    lASum += coeff * pixel;

                }

                lKIndex -= lKernelInc;
                lSrcPix -= wSrcPixelPitch;
            }
            /* head positive, until kernel runs out */
            lKIndex = lFilterOffset + lKernelInc;
            lSrcPix = lSrcPixel+wSrcPixelPitch;
            while (lKIndex < wKernelSamples)
            {
                int coeff = pnKernel[lKIndex];
                int pixel;

                lKSum += coeff;

                if (lSrcPix < lLastSrcOffset)
                {
                    pixel = (int)(pbySrc[lSrcPix]);
                    lRSum += coeff * pixel;
                    pixel = (int)(pbySrc[lSrcPix+1]);
                    lGSum += coeff * pixel;
                    pixel = (int)(pbySrc[lSrcPix+2]);
                    lBSum += coeff * pixel;
                    pixel = (int)(pbySrc[lSrcPix+3]);
                    lASum += coeff * pixel;
                }
                else
                {
                    /* extend edge color */
                    pixel = (int)(pbySrc[lastsourceindex]);
                    lRSum += coeff * pixel;
                    pixel = (int)(pbySrc[lastsourceindex+1]);
                    lGSum += coeff * pixel;
                    pixel = (int)(pbySrc[lastsourceindex+2]);
                    lBSum += coeff * pixel;
                    pixel = (int)(pbySrc[lastsourceindex+3]);
                    lASum += coeff * pixel;
                }

                lKIndex += lKernelInc;
                lSrcPix += wSrcPixelPitch;
            }

            lRSum += FIXZ_VALUE/2;   /* rounding */
            if (lKSum) lRSum /= lKSum;
            if (lRSum < 0) lRSum = 0;
            else if (lRSum > 255) lRSum = 255;
            *(pbyDst + lIndex*wDstPixelPitch) = (BYTE)lRSum;

            lGSum += FIXZ_VALUE/2;   /* rounding */
            if (lKSum) lGSum /= lKSum;
            if (lGSum < 0) lGSum = 0;
            else if (lGSum > 255) lGSum = 255;
            *(pbyDst + lIndex*wDstPixelPitch+1) = (BYTE)lGSum;

            lBSum += FIXZ_VALUE/2;   /* rounding */
            if (lKSum) lBSum /= lKSum;
            if (lBSum < 0) lBSum = 0;
            else if (lBSum > 255) lBSum = 255;
            *(pbyDst + lIndex*wDstPixelPitch+2) = (BYTE)lBSum;

            lASum += FIXZ_VALUE/2;   /* rounding */
            if (lKSum) lASum /= lKSum;
            if (lASum < 0) lASum = 0;
            else if (lASum > 255) lASum = 255;
            *(pbyDst + lIndex*wDstPixelPitch+3) = (BYTE)lASum;

        } /* end of convolution block */

    } /* end of for each output pixel loop */

    return((int)lOutPixels);
}

/*
    lFillKernelTable initializes an array pnKernel, having wKernelSamples
    number of entries.  The shape of the function varies smoothly from
    one shape to another, controlled by lFilter in the following table:

    FIXZ_VALUE*0    Mitchell and Netravali "Subjective Best" polynomial
    FIXZ_VALUE*1    1.5 pixel Bartlett window (linear interpolation)
    FIXZ_VALUE*2    1 pixel box

    RETURN
    The width of the filter kernel (support) varies depending on what
    shape the kernel function is, so it returns a number from
    2*FIXZ_VALUE to FIXZ_VALUE/2.


    References: 1988 Siggraph Proceedings, "Reconstruction Filters
    in Computer Graphics", Don P. Mitchell and Arun N. Netravali pp.221-228

*/

/* didn't get around to removing the floating point from this... SOC */
static double dMitchNetFilter(double x, double B, double C)
{
    double value = 0.0;
    double x1 = (x < 0.0) ? -x : x;
    double x2 = x1 * x1;
    double x3 = x2 * x1;

    /* there is a bug in the floating point parser under VC 2.1 that */
    /* flags double test = -18.0 as an overflow.  By using -18 and */
    /* 0.0 - 12.0*B the warning goes away */

    if (x >= 0.0 && x < 1.0)
    {
        value = (12.0 - 9.0*B - 6.0*C) * x3 +
                (-18 + 12.0*B + 6.0*C) * x2 +
                (6.0 - 2.0 * B);
        value /= 6.0;
    }
    else if (x >= 1.0 && x < 2.0)
    {
        value = (-B - 6.0*C) * x3 +
                (6.0*B + 30.0*C) * x2 +
                (0.0 - 12.0*B - 48.0*C) * x +
                (8.0 * B + 24.0*C);
        value /= 6.0;
    }

    return(value);
}

FIXZ lFillKernelTable(short pnKernel[], WORD wKernelSamples, FIXZ lFilter)
{
    int lowvalue = 0;
    int highvalue = 0;
    int value = 0;
    int lowwidth = 0;
    int highwidth = 0;
    int width = 0;
    int lFilterWeight;

    double t = 0.0;
    int i;

    if (!pnKernel) return(0);

    if (lFilter < 0)
        lFilter = 0;
    else if (lFilter > FIXZ_VALUE*2)
        lFilter = FIXZ_VALUE*2;

    if (lFilter >= 0 && lFilter <= FIXZ_VALUE)
        lFilterWeight = lFilter;
    else
        lFilterWeight = lFilter - FIXZ_VALUE;


    for (i = 0; i < (wKernelSamples/2); i++)
    {
        t = (double)i/(((wKernelSamples/2)-1.0)/2.0); /* 0.0 to 2.0 */

        if (lFilter >= 0 && lFilter <= FIXZ_VALUE)
        {
            lowvalue = (int)(FIXZ_VALUE *
                            dMitchNetFilter(t, 0.333333, 0.333333));

            t = 1.5 - t;
            if (t < 0.0) t = 0.0;
            highvalue = (int)(FIXZ_VALUE*t/1.5);
        }
        else
        {
            if (t <= 1.0)
                highvalue = FIXZ_VALUE;
            else
                highvalue = 0;

            t = 1.5 - t;
            if (t < 0.0) t = 0.0;
            lowvalue = (int)(FIXZ_VALUE*t/1.5);

        }
        value = lowvalue * (FIXZ_VALUE-lFilterWeight) +
                highvalue * lFilterWeight;
        value /= FIXZ_VALUE;
        pnKernel[i+(wKernelSamples/2)] = (short)value;
        pnKernel[(wKernelSamples/2 - 1)-i] = (short)value;
    }
    if (lFilter >= 0 && lFilter <= FIXZ_VALUE)
    {
        highwidth = 3*FIXZ_VALUE/2;
        lowwidth = 2*FIXZ_VALUE;
    }
    else
    {
        highwidth = FIXZ_VALUE;
        lowwidth = 3*FIXZ_VALUE/2;
    }
    width = lowwidth * (FIXZ_VALUE-lFilterWeight) +
            highwidth * lFilterWeight;
    width /= FIXZ_VALUE;
    return(width);
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutzoom(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('Z','O','O','M');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Spencer Cathey");
        strcpy(about->versionstr,   "1.06");
        strcpy(about->namestr,      "Zoom");
        strcpy(about->longnamestr,  "Bicubic Zoom");
        about->numoptions =         6;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           2;
        about->option[0].max.i =           4096;
        about->option[0].def.i =           320;
        strcpy(about->option[0].name,      "Width");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           2;
        about->option[1].max.i =           4096;
        about->option[1].def.i =           240;
        strcpy(about->option[1].name,      "Height");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT | FOP_ENUM;
        about->option[2].min.i =           1;
        about->option[2].max.i =           2;
        about->option[2].def.i =           1;
        strcpy(about->option[2].name,      "Edge Extend");
        strcpy(about->option[2].enumlist,  "Solid,Boundary");

        about->option[3].type =            FOP_INT | FOP_TRUE_COLOR;
        about->option[3].min.i =           0;
        about->option[3].max.i =           -1;   /* all 0xFF */
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "Solid Colour");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_FLOAT;
        about->option[4].min.f =           0.5;
        about->option[4].max.f =           4.0;
        about->option[4].def.f =           2.0;
        strcpy(about->option[4].name,      "Kernel Width");
        strcpy(about->option[4].enumlist,  "");

        about->option[5].type =            FOP_FLOAT;
        about->option[5].min.f =           0.0;
        about->option[5].max.f =           2.0;
        about->option[5].def.f =           0.0;
        strcpy(about->option[5].name,      "Kernel Shape");
        strcpy(about->option[5].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openzoom(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx;

    fx = (FINSTANCE *) galloc(sizeof(FINSTANCE));
    if (fx)
    {
        FILTERDATA *fd;
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        *filterInstance = fx;

        fx->fref = (char *)galloc(sizeof(FILTERDATA));
        if (fx->fref ==  NULL)
        {
            gfree(*filterInstance);
            *filterInstance = NULL;
            ok = 0;
        }
        else
        {
            fd = (FILTERDATA *)(fx->fref);
            fd->kernel = (short *) galloc(KERNEL_ENTRIES*sizeof(short));

            if (!fd->kernel)
            {
                gfree(fx->fref);
                gfree(*filterInstance);
                *filterInstance = NULL;
                ok = 0;
            }
            else
            {
                int hmax = sources[0].ginfo->height;
                int wmax = sources[0].ginfo->width;

                if (options)
                {
                    fd->defargs[Z_WIDTH].i = options[Z_WIDTH].i;
                    fd->defargs[Z_HEIGHT].i = options[Z_HEIGHT].i;
                    fd->defargs[Z_EDGE_TYPE].i = options[Z_EDGE_TYPE].i;
                    fd->defargs[Z_FILTER_WIDTH].f = options[Z_FILTER_WIDTH].f;
                    fd->defargs[Z_FILTER_SHAPE].f = options[Z_FILTER_SHAPE].f;
                    if ((numoptions >= 9) && (options[Z_TYPE].i))
                    {
                            fd->defargs[Z_WIDTH].i  = (int)((float)fd->defargs[Z_WIDTH].i * (float)((float)fx->source[0].ginfo->width/(float)options[Z_MULTIFRAME_WIDTH].i));
                            fd->defargs[Z_HEIGHT].i = (int)((float)fd->defargs[Z_HEIGHT].i * (float)((float)fx->source[0].ginfo->height/(float)options[Z_MULTIFRAME_HEIGHT].i));
                    }
                    if ((fd->defargs[Z_EDGE_TYPE].i < 1) ||
                        (fd->defargs[Z_EDGE_TYPE].i > 2))
                        fd->defargs[Z_EDGE_TYPE].i = 1;
                    if ((fd->defargs[Z_FILTER_WIDTH].f < 0.5))
                        fd->defargs[Z_FILTER_WIDTH].f = 0.5;
                    if ((fd->defargs[Z_FILTER_WIDTH].f > 2.0))
                        fd->defargs[Z_FILTER_WIDTH].f = 2.0;
                    if ((fd->defargs[Z_FILTER_SHAPE].f < 0.0))
                        fd->defargs[Z_FILTER_SHAPE].f = 0.0;
                    if ((fd->defargs[Z_FILTER_SHAPE].f > 2.0))
                        fd->defargs[Z_FILTER_SHAPE].f = 2.0;
                }
                else
                {
                    fd->defargs[Z_WIDTH].i = wmax/2;
                    fd->defargs[Z_HEIGHT].i = hmax/2;
                    fd->defargs[Z_EDGE_TYPE].i = 1;
                    fd->defargs[Z_EDGE_COLOUR].argb.a = 0;
                    fd->defargs[Z_EDGE_COLOUR].argb.r = 0;
                    fd->defargs[Z_EDGE_COLOUR].argb.g = 0;
                    fd->defargs[Z_EDGE_COLOUR].argb.b = 0;
                    fd->defargs[Z_FILTER_WIDTH].f = 1.0;
                    fd->defargs[Z_FILTER_SHAPE].f = 0.0;
                }
                fx->option = (FOP *)&(fd->defargs);
                if (wmax < fd->defargs[Z_WIDTH].i)
                    wmax = fd->defargs[Z_WIDTH].i;
                if (hmax < fd->defargs[Z_HEIGHT].i)
                    hmax = fd->defargs[Z_HEIGHT].i;

                fx->numoptions = numoptions;  // used to be Z_ARGS

                fd->imagecopy = imageclone(sources, wmax, hmax);
                if (fd->imagecopy)
                {
                    ok = 1;
                }
                else
                {
                    gfree(fd->kernel);
                    gfree(fx->fref);
                    gfree(*filterInstance);
                    *filterInstance = NULL;
                    ok = 0;
                }
            }
        }
    }
    else
    {
        *filterInstance = NULL;
        ok = 0;
    }

    if (ok)
    {
        FILTERDATA* fd = (FILTERDATA *)(fx->fref);
        FIXZ shape = (FIXZ)(fd->defargs[Z_FILTER_SHAPE].f * FIXZ_VALUE);

        fd->filterwidth =
            lFillKernelTable(fd->kernel, KERNEL_ENTRIES, shape);
        fd->filterwidth *=
            (FIXZ)(fd->defargs[Z_FILTER_WIDTH].f * FIXZ_VALUE);
        fd->filterwidth /= FIXZ_VALUE;
    }

    return(ok);
}

FPUBLIC GINFO *infozoom(FINSTANCE *fx, int framenum)
{
    int i;
    GINFO *ginfo;

    ginfo = (GINFO *)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        /* copy ginfo to destination */
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->framenum = framenum;

        ginfo->width    = fx->option[0].i;
        ginfo->height   = fx->option[1].i;
        if (ginfo->width<1)  ginfo->width = 1;
        if (ginfo->height<1) ginfo->height = 1;

        /* scale center point and hotspots */
        {
            /* calculate zoom ratio's in both directions */

            double widthZoom  = (double)ginfo->width  / (double)fx->source[0].ginfo->width;
            double heightZoom = (double)ginfo->height / (double)fx->source[0].ginfo->height;

            /* scale hotspots */

            for (i=0; i<ginfo->numhotspots; ++i)
            {
                ginfo->hotspottbl[i][0] = (int)(ginfo->hotspottbl[i][0] * widthZoom);
                ginfo->hotspottbl[i][1] = (int)(ginfo->hotspottbl[i][1] * heightZoom);
            }

            /* scale center point */
            ginfo->centerx = (int)(ginfo->centerx * widthZoom);
            ginfo->centery = (int)(ginfo->centery * heightZoom);
            /* position */
            ginfo->defaultx = (int)(ginfo->defaultx * widthZoom);
            ginfo->defaulty = (int)(ginfo->defaulty * heightZoom);
        }

    }
    return(ginfo);
}

FPUBLIC int filterzoom(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int row, col;
    int iPixels;
    FILTERDATA* fd = (FILTERDATA *)(fx->fref);
    GBITMAP *source = fx->source;
    int srcrowbytes = source->rowbytes;
    int srcwidth = source->ginfo->width;
    int srcheight = source->ginfo->height;
    BYTE* srcimage = (BYTE *)(source->image);

    /* isrc is intermediate image */
    GBITMAP *isrc = fd->imagecopy;
    int isrcrowbytes = isrc->rowbytes;
//    int isrcwidth = isrc->ginfo->width;
    int isrcheight = ginfo->height;   /* intermediate size is dest height */
    BYTE* isrcimage = (BYTE *)(isrc->image);

    int dstwidth = ginfo->width;
    int dstheight = ginfo->height;

    FIXZ filter_support = fd->filterwidth;
    short* filter_kernel = fd->kernel;
    FIXZ lHorZoom;
    FIXZ lVertZoom;

    /* handle exception case */
    if (srcwidth == 1 || srcheight == 1)
    {
        /* TODO: put code to do a simple zoom here */
        memset(dest, 0, rowbytes*ginfo->height);
        return(1);
    }

    lHorZoom = lZoomFactor(srcwidth, dstwidth);
    lVertZoom = lZoomFactor(srcheight, dstheight);

    /* vertical pass */
    for (col = 0; col < srcwidth; col++)
    {
        iPixels = iScanZoomARGB(srcimage+col*4,
                (srcheight-1)*FIXZ_VALUE / 2,
                (WORD)srcrowbytes,
                (short)srcheight,
                isrcimage+col*4,
                (WORD)isrcrowbytes,
                (short)isrcheight,
                lVertZoom,
                filter_support,
                (WORD)KERNEL_ENTRIES,
                filter_kernel);
    }

    /* horizontal pass */
    for (row = 0; row < dstheight; row++)
    {
        iPixels = iScanZoomARGB(isrcimage+row*isrcrowbytes,
                (srcwidth-1)*FIXZ_VALUE / 2,
                (WORD)4,
                (short)srcwidth,
                ((BYTE *)dest)+row*rowbytes,
                (WORD)4,
                (short)dstwidth,
                lHorZoom,
                filter_support,
                KERNEL_ENTRIES,
                filter_kernel);
    }
    return(1);
}

FPUBLIC int closezoom(FINSTANCE *fx)
{
    if (fx->fref)
    {
        GBITMAP *bitmap = ((FILTERDATA *)(fx->fref))->imagecopy;

        if (bitmap)
        {
            gfree(bitmap->ginfo);
            gfree(bitmap->image);
            gfree(bitmap);
        }

        if (((FILTERDATA *)(fx->fref))->kernel)
            gfree(((FILTERDATA *)(fx->fref))->kernel);

        gfree(fx->fref);   /* free filter memory */
    }

    return(gfree((void *)fx));
}

