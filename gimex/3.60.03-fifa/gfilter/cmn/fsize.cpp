// (c) 1996-2007 Electronic Arts Inc.

#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#endif

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Size - General Filtered Image Rescaling              */
/*                                                                  */
/*              by Dale Schumacher and Gerry Shaw, EAC              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960624 GS initial version based on Graphics Gem III.1.2  */
/*    1.01 960710 GS arranged methods from best to worst            */
/*    1.02 960723 FB removed white/black and fixed fop order        */
/*    1.03 970818 KO small scale value made filter mask > imagesize */
/*    1.04 970925 GS clamping introduced round off errors           */
/*    1.05 990915 FB transparent colour specifiable (chromakey)     */
/*    1.06 000329 FB position scaled                                */
/*    1.07 001012 FB floats, casting, lanczos optimizations         */
/*    1.08 001017 FB clamp filter generator                         */
/*    1.09 010118 ID size each layer wrt bottom layer               */
/*    1.10 010313 FB clamp to 1 by 1                                */
/*    1.11 020321 ID fix of center point calculation off by 1/2 pix */
/*    1.12 020338 ID a bit better but still not good for the 1/2 pix*/
/*    1.13 020408 FB size up 1/2 pixel fix, and clip vs ring        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* size(width,height,method,respect_alpha,xsharpness,ysharpness);   */
/* int width           Width of resulting image                     */
/* int height          Height of resulting image                    */
/* int method          Filter function (see below)                  */
/* int respect_alpha   0=default,1=keep sharp transparent edges     */
/* int chromakey       transparent colour (0 default)               */
/* float xsharpness   <1=sharp >1=blury x<=0=crash (1.0=default)    */
/* float ysharpness   <1=sharp >1=blury x<=0=crash (1.0=default)    */
/*                                                                  */
/* NOTE: As of version 1.10 sharpness options are being ignored.    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Filter Functions:                                                */
/* ----------------                                                 */
/*                                                                  */
/* The reconstruction filter is defined by the filter method and is */
/* one of the following methods                                     */
/*                                                                  */
/* 0 Lanczos    Good choice (sharper than Mitchell)                 */
/* 1 Mitchell   Good choice (B=1/3 C=1/3)                           */
/* 2 Cubic      Good choice (2|t|^3 - 3|t|^2 + 1 for -1 <= t <= 1)  */
/* 3 Triangle   Linear interpolation                                */
/* 4 Box        Point sample with interpolation                     */
/* 5 Bell       Blurry                                              */
/* 6 B-Spline   Blurry                                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* References:                                                      */
/* -----------                                                      */
/*                                                                  */
/* Graphics Gems III, 1.2, "General Filtered Image Rescaling",      */
/* Dale Schumacher, pages 8-16                                      */
/*                                                                  */
/* 1988 Siggraph Proceedings, "Reconstruction Filters in Computer   */
/* Graphics", Don P. Mitchell and Arun N. Netravali, pages 221-228  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fsize.h"
#include <string.h>
#include <math.h>
#if defined(_MSC_VER) || defined(__R5900)
#include <malloc.h>
#elif defined(__PPU__)
#include <alloca.h>
#else
#include <stdlib.h>
#endif

#define SIZECLAMP 1

/****************************************************************/
/* Internal Functions - Reusable Bitmap Functions               */
/****************************************************************/

static int fround(float r)
{
    int ir = (int)r;
    float rem = r - ir;
    if(rem >= 0.5) return(ir+1);
    else           return(ir);
}

static GBITMAP* SIZE_allocbitmap(int width, int height, int originalbpp)
{
    int bpp = (originalbpp > 8) ? 32 : 8;
    int rowbytes = width*bpp / 8;

    GBITMAP* dst = (GBITMAP*)galloc(sizeof(GBITMAP));
    memset(dst, 0, sizeof(GBITMAP));

    dst->ginfo = (GINFO*)galloc(sizeof(GINFO));
    memset(dst->ginfo, 0, sizeof(GINFO));

    dst->image = galloc(rowbytes * height);
    memset(dst->image, 0, (size_t)(rowbytes * height));

    dst->rowbytes = rowbytes;

    /* assign known GINFO fields */

    dst->ginfo->size        = sizeof(GINFO);
    dst->ginfo->version     = 200;
    dst->ginfo->bpp         = bpp;
    dst->ginfo->originalbpp = originalbpp;
    dst->ginfo->numcolours  = 256;
    dst->ginfo->width       = width;
    dst->ginfo->height      = height;
    dst->ginfo->redbits     = gmin(8, bpp/3);
    dst->ginfo->greenbits   = gmin(8, bpp/3);
    dst->ginfo->bluebits    = gmin(8, bpp/3);
    dst->ginfo->alphabits   = bpp - gmin(8, bpp/3)*3;

    return dst;
}

static void SIZE_freebitmap(GBITMAP* bmp)
{
    gfree(bmp->ginfo);
    gfree(bmp->image);
    gfree(bmp);
}


/****************************************************************/
/* Internal Functions - Specfic Filter Functions                */
/****************************************************************/

#define PI  3.141592654f

static const float float255=254.5f;

/* clamp the input to the specified range */
#if defined(_MSC_VER)
static __inline unsigned char fsize_clamp(float value)
{
    int r;
    __asm
    {
        fld     dword ptr value
        fistp   dword ptr r
    }
    return(gmax(0,gmin(255,r)));
}
#else
static __inline unsigned char fsize_clamp(float value)
{
    int r = (int)(value + 0.5f);
    return(gmax(0,gmin(255,r)));
}
#endif

static __inline ARGB get_pixel(GBITMAP* image, int x, int y)
{
    ARGB* currentbase;

    currentbase = (ARGB*)((char *)image->image + y*image->rowbytes);

    return currentbase[x];
}

static __inline ARGB *get_row(GBITMAP* image, int y)
{
    return((ARGB *)((char *)image->image + (y * image->rowbytes)));
}

static void get_column(ARGB* column, GBITMAP* image, int x)
{
    int    row;
    ARGB* base;

    /* check boundary condition */
    if (x >= image->ginfo->width)
        return;

    row  = image->ginfo->height;
    base = (ARGB*)(image->image) + x;
    while (row > 0)
    {
        *column = *base;

        column++;
        base = (ARGB*)((unsigned char*)base + image->rowbytes);
        row--;
    }
}

static __inline void put_pixel(GBITMAP* image, int x, int y, ARGB data)
{
    ARGB *currentbase;

    currentbase  = (ARGB*)((char *)image->image + y*image->rowbytes);
    currentbase[x] = data;
}

static ARGB FindTransparentColor(GBITMAP* src)
{
    /* search src image for a transparent color and return the color */
    ARGB Color;

    for (int y = 0; y < src->ginfo->height; y++)
    {
        for (int x = 0; x < src->ginfo->width; x++)
        {
            Color = get_pixel(src, x,y);
            if (Color.a == 0)
                return Color;
        }
    }

    /* default color is cyan */
    /* loop should never get here and if it does then it doesn't matter */
    /* because the transparent color will never be used */
    Color.a = 0;
    Color.r = 0;
    Color.g = 255;
    Color.b = 255;

    return Color;
}

/****************************************************************/
/* Internal Functions - Filter Function Definitions             */
/****************************************************************/

#define cubic_support       (1.0f)

static float cubic_filter(float t)
{
    /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
    if(t < 0.0f) t = -t;
    if(t < 1.0f) return((2.0f * t - 3.0f) * t * t + 1.0f);
    return(0.0f);
}

#define box_support     (0.5f)

static float box_filter(float t)
{
    if((t > -0.5f) && (t <= 0.5f)) return(1.0f);
    return(0.0f);
}

#define triangle_support    (1.0f)

static float triangle_filter(float t)
{
    if(t < 0.0f) t = -t;
    if(t < 1.0f) return(1.0f - t);
    return(0.0f);
}

#define bell_support        (1.5f)

static float bell_filter(float t)     /* box (*) box (*) box */
{
    if(t < 0) t = -t;
    if(t < 0.5f) return(0.75f - (t * t));
    if(t < 1.5f) {
        t = (t - 1.5f);
        return(.5f * (t * t));
    }
    return(0.0f);
}

#define B_spline_support    (2.0f)

static float B_spline_filter(float t) /* box (*) box (*) box (*) box */
{
    float tt;

    if(t < 0) t = -t;
    if(t < 1) {
        tt = t * t;
        return((.5f * tt * t) - tt + (2.0f / 3.0f));
    } else if(t < 2) {
        t = 2 - t;
        return((1.0f / 6.0f) * (t * t * t));
    }
    return(0.0f);
}

#define Lanczos3_support    (3.0f)

static float Lanczos3_filter(float t)
{
    if (t==0.0f) return(1.0f);
    if (t < 0) t = -t;
    if (t < 3.0f)
    {
        t *= PI;
        return((float)(sin(t)/t * sin(t/3.0f)*3.0f/t));
    }
    return(0.0f);
}

#define Mitchell_support    (2.0f)

#define B   (1.0f/3.0f)
#define C   (1.0f/3.0f)

static float Mitchell_filter(float t)
{
    float tt;

    tt = t * t;
    if(t < 0) t = -t;
    if(t < 1.0f) {
        t = (((12.0f - 9.0f * B - 6.0f * C) * (t * tt))
            + ((-18.0f + 12.0f * B + 6.0f * C) * tt)
            + (6.0f - 2 * B));
        return(t / 6.0f);
    } else if(t < 2.0f) {
        t = (((-1.0f * B - 6.0f * C) * (t * tt))
            + ((6.0f * B + 30.0f * C) * tt)
            + ((-12.0f * B - 48.0f * C) * t)
            + (8.0f * B + 24 * C));
        return(t / 6.0f);
    }
    return(0.0f);
}

#define Quadratic_support   (1.5f)
#define R                   (0.8f)

static float Quadratic_filter(float t)
{
    float tt = t*t;

    if(t<0)
        t=-t;
    if(t <= 0.5f)
        return (-2.0f)*R*tt+0.5f*(R+1.0f);
    else if(t<=1.5f)
        return R*tt+(-2.0f*R-0.5f)*t+(3.0f/4.0f)*(R+1);

    return (0.0f);

}


/****************************************************************/
/* Internal Functions - Image Rescaling Routine                 */
/****************************************************************/

typedef struct
{
    int     pixel;
    float   weight;
} CONTRIB;

typedef struct
{
    int      n;      /* number of contributors */
    CONTRIB* p;      /* pointer to list of contributions */
} CLIST;

CLIST* contrib;     /* array of contribution lists */

static int SIZETransparentPixel(ARGB p, ARGB TransparentColor)
{
    int istransparent=!p.a;
    if (*(int *) &TransparentColor)
        istransparent = (*(int *)&p==*(int *)&TransparentColor);

    /* return TRUE if pixel should not be used for interpolation */
    return(istransparent);
}

static void size(GBITMAP* src, GBITMAP* dst, float (*filterf)(float), float fwidth,
                 int RespectAlpha, int chromakey, float xsharp, float ysharp)
{
    ARGB TransparentColor = *(ARGB *)&chromakey;  /* color to use for transparent colors */
    if (RespectAlpha && !chromakey)
        TransparentColor = FindTransparentColor(src);

    /* ignore sharpness for now - not sure how to get it working (not that hard though) */
    xsharp = 1.0f;
    ysharp = 1.0f;

    /* create intermediate image to hold horizontal size */
    GBITMAP* tmp = SIZE_allocbitmap(dst->ginfo->width, src->ginfo->height,32);
    float xscale = (float) src->ginfo->width / (float) dst->ginfo->width;
    float yscale = (float) src->ginfo->height / (float) dst->ginfo->height;

    /* pre-calculate filter contributions for a row */
    contrib = (CLIST*)galloc(dst->ginfo->width * sizeof(CLIST));
    if (xscale > 1.0f)  // scale down
    {
        float width = gmin((float)fwidth * (float)xscale, src->ginfo->width-1);
        float fscale = (float)fwidth/(float)width;  // reciprical

        for (int i = 0; i < dst->ginfo->width; ++i)
        {
            contrib[i].p = (CONTRIB*) alloca((int)(width*2 + 1) * sizeof(CONTRIB));
        }

#if ENABLE_OMP
        #pragma omp parallel for
#endif
        for (int i = 0; i < dst->ginfo->width; ++i)
        {
            int k = 0;
            float tweight = 0.0f;
            //contrib[i].p = (CONTRIB*) alloca((int)(width*2 + 1) * sizeof(CONTRIB));

            float center = (float)i * xscale + xscale/2;
            float left   = (float)ceil(center - width);
            float right  = (float)floor(center + width);

            for (int j = (int)left; j <= right; ++j)
            {
#if SIZECLAMP
                int n = j;
                if (n<0)
                    n = 0;
                else if (j >= src->ginfo->width)
                    n = src->ginfo->width-1;
#else
                int n = j;
                if (n<0)
                {
                    n = -j;
                    if (n >= src->ginfo->width)
                        n = src->ginfo->width-1;
                }
                else if (j >= src->ginfo->width)
                {
                    n = (src->ginfo->width - 1) - (j - src->ginfo->width);
                    if (n<0)
                        n = 0;
                }
#endif
                contrib[i].p[k].pixel  = n;
                float weight = (*filterf)((center - (float)j)*fscale) * fscale;
                tweight += weight;
                contrib[i].p[k].weight = weight;
                ++k;
            }
            contrib[i].n = k;

            tweight = 1.0f/tweight;
            k = 0;
            for (int j = (int)left; j <= right; ++j)
            {
                contrib[i].p[k].weight *= tweight;
                ++k;
            }
        }
    }
    else
    {

        for (int i = 0; i < dst->ginfo->width; ++i)
        {
            contrib[i].p = (CONTRIB*)alloca((int)(fwidth*2 + 1) * sizeof(CONTRIB));
        }

#if ENABLE_OMP
        #pragma omp parallel for
#endif
        for (int i = 0; i < dst->ginfo->width; ++i)
        {
            int k = 0;
            float tweight = 0.0f;
            //contrib[i].p = (CONTRIB*)alloca((int)(fwidth*2 + 1) * sizeof(CONTRIB));

            float center;
            if (xscale==1.0f)
                center = (float)i * xscale;
            else
                center = (float)i * xscale - 0.5f;

//            center = (double)i * (double)xscale;
            float left   = (float)ceil(center - fwidth);
            float right  = (float)floor(center + fwidth);

            for (int j = (int)left; j <= right; ++j)
            {
#if SIZECLAMP
                int n = j;
                if (n<0)
                    n = 0;
                else if (j >= src->ginfo->width)
                    n = src->ginfo->width-1;
#else
                int n = j;
                if (n<0)
                {
                    n = -j;
                    if (n >= src->ginfo->width)
                        n = src->ginfo->width-1;
                }
                else if (j >= src->ginfo->width)
                {
                    n = (src->ginfo->width - 1) - (j - src->ginfo->width);
                    if (n<0)
                        n = 0;
                }
#endif

                contrib[i].p[k].pixel  = n;
                float weight = (*filterf)((center - (float)j));
                tweight += weight;
                contrib[i].p[k].weight = weight;
                ++k;
            }
            contrib[i].n = k;

            tweight = 1.0f/tweight;
            k = 0;
            for (int j = (int)left; j <= right; ++j)
            {
                contrib[i].p[k].weight *= tweight;
                ++k;
            }
        }
    }

    /* apply filter to size horizontally from src to tmp */
//    raster = (ARGB*)galloc(src->ginfo->width * sizeof(ARGB));

#if ENABLE_OMP
    #pragma omp parallel for
#endif
    for (int k = 0; k < tmp->ginfo->height; ++k)
    {
        ARGB* raster = get_row(src, k);
        if (dst->ginfo->alphabits)
        {
            for (int i = 0; i < tmp->ginfo->width; ++i)
            {
                ARGB p;
                float wa = 0.0f;    /* alpha weight */
                float wr = 0.0f;    /* red weight */
                float wg = 0.0f;    /* green weight */
                float wb = 0.0f;    /* blue weight */

                float TotalWeight = 0.0f;


                for (int j = 0; j < contrib[i].n; ++j)
                {
                    ARGB  SourcePixel;
                    float Weight      = contrib[i].p[j].weight;

                    SourcePixel = raster[contrib[i].p[j].pixel];

                    if (!RespectAlpha || (RespectAlpha && !SIZETransparentPixel(SourcePixel,TransparentColor)))
                    {
                        wa += SourcePixel.a * Weight;
                        wr += SourcePixel.r * Weight;
                        wg += SourcePixel.g * Weight;
                        wb += SourcePixel.b * Weight;

                        TotalWeight += Weight;
                    }
                }

                if (RespectAlpha && TotalWeight < 0.3)
                {
                    p = TransparentColor;
                }
                else
                {
                    float rTotalWeight=1.0f/TotalWeight;

                    p.a = fsize_clamp(wa * rTotalWeight);
                    p.r = fsize_clamp(wr * rTotalWeight);
                    p.g = fsize_clamp(wg * rTotalWeight);
                    p.b = fsize_clamp(wb * rTotalWeight);
                }

                put_pixel(tmp, i, k, p);
            }
        }
        else
        {
            for (int i = 0; i < tmp->ginfo->width; ++i)
            {
                ARGB p;
                float wr = 0.0f;    /* red weight */
                float wg = 0.0f;    /* green weight */
                float wb = 0.0f;    /* blue weight */

                for (int j = 0; j < contrib[i].n; ++j)
                {
                    ARGB  SourcePixel;
                    float Weight      = contrib[i].p[j].weight;

                    SourcePixel = raster[contrib[i].p[j].pixel];

                    wr += SourcePixel.r * Weight;
                    wg += SourcePixel.g * Weight;
                    wb += SourcePixel.b * Weight;

                }

                p.a = 255;
                p.r = fsize_clamp(wr);
                p.g = fsize_clamp(wg);
                p.b = fsize_clamp(wb);

                put_pixel(tmp, i, k, p);
            }
        }

    }
//    gfree(raster);
    gfree(contrib);


    /* pre-calculate filter contributions for a column */
    contrib = (CLIST*)galloc(dst->ginfo->height * sizeof(CLIST));
    if (yscale > 1.0f)
    {
        float width = gmin(fwidth * yscale, src->ginfo->height-1);
        float fscale = fwidth/width;

        for (int i = 0; i < dst->ginfo->height; ++i)
        {
            contrib[i].p = (CONTRIB*)alloca((int)(width * 2 + 1) * sizeof(CONTRIB));
        }


#if ENABLE_OMP
        #pragma omp parallel for
#endif
        for (int i = 0; i < dst->ginfo->height; ++i)
        {
            int k = 0;
            float tweight = 0.0f;
            //contrib[i].p = (CONTRIB*)alloca((int)(width * 2 + 1) * sizeof(CONTRIB));

            float center = (float) i * yscale + yscale/2;
            float left   = (float)ceil(center - width);
            float right  = (float)floor(center + width);

            for (int j = (int)left; j <= right; ++j)
            {
#if SIZECLAMP
                int n = j;
                if (n<0)
                    n = 0;
                else if (j >= src->ginfo->height)
                    n = src->ginfo->height-1;
#else
                int n = j;
                if (n<0)
                {
                    n = -j;
                    if (n >= src->ginfo->height)
                        n = src->ginfo->height-1;
                }
                else if (j >= src->ginfo->height)
                {
                    n = (src->ginfo->height - 1) - (j - src->ginfo->height);
                    if (n<0)
                        n = 0;
                }
#endif

                contrib[i].p[k].pixel  = n;
                float weight = (*filterf)((center - (float)j)*fscale) * fscale;
                tweight += weight;
                contrib[i].p[k].weight = weight;
                ++k;
            }
            contrib[i].n = k;

            tweight = 1.0f/tweight;
            k = 0;
            for (int j = (int)left; j <= right; ++j)
            {
                contrib[i].p[k].weight *= tweight;
                ++k;
            }
        }
    }
    else
    {

        for (int i = 0; i < dst->ginfo->height; ++i)
        {
            contrib[i].p = (CONTRIB*)alloca((int) (fwidth*2 + 1) * sizeof(CONTRIB));
        }

#if ENABLE_OMP
        #pragma omp parallel for
#endif
        for (int i = 0; i < dst->ginfo->height; ++i)
        {
            int k = 0;
            float tweight = 0.0f;
            //contrib[i].p = (CONTRIB*)alloca((int) (fwidth*2 + 1) * sizeof(CONTRIB));

            float center;
            if (yscale==1.0f)
                center = (float)i * yscale;
            else
                center = (float)i * yscale - 0.5f;

            float left   = (float)ceil(center - fwidth);
            float right  = (float)floor(center + fwidth);

            for (int j = (int)left; j <= right; ++j)
            {
#if SIZECLAMP
                int n = j;
                if (n<0)
                    n = 0;
                else if (j >= src->ginfo->height)
                    n = src->ginfo->height-1;
#else
                int n = j;
                if (n<0)
                {
                    n = -j;
                    if (n >= src->ginfo->height)
                        n = src->ginfo->height-1;
                }
                else if (j >= src->ginfo->height)
                {
                    n = (src->ginfo->height - 1) - (j - src->ginfo->height);
                    if (n<0)
                        n = 0;
                }
#endif


                contrib[i].p[k].pixel  = n;
                float weight = (*filterf)((center - (float)j));
                tweight += weight;
                contrib[i].p[k].weight = weight;
                ++k;
            }
            contrib[i].n = k;

            tweight = 1.0f/tweight;
            k = 0;
            for (int j = (int)left; j <= right; ++j)
            {
                contrib[i].p[k].weight *= tweight;
                ++k;
            }
        }
    }

    /* apply filter to size vertically from tmp to dst */
    ARGB* raster = (ARGB*)galloc(tmp->ginfo->height * sizeof(ARGB));


    for (int k = 0; k < dst->ginfo->width; ++k)
    {
        get_column(raster, tmp, k);
        if (dst->ginfo->alphabits)
        {
#if ENABLE_OMP
            #pragma omp parallel for
#endif
            for (int i = 0; i < dst->ginfo->height; ++i)
            {
                ARGB p;
                float wa = 0.0f;    /* alpha weight */
                float wr = 0.0f;    /* red weight */
                float wg = 0.0f;    /* green weight */
                float wb = 0.0f;    /* blue weight */

                float TotalWeight = 0.0f;

                for (int j = 0; j < contrib[i].n; ++j)
                {
                    ARGB  SourcePixel = raster[contrib[i].p[j].pixel];
                    float Weight      = contrib[i].p[j].weight;

                    if (!RespectAlpha || (RespectAlpha && !SIZETransparentPixel(SourcePixel,TransparentColor)))
                    {
                        wa += SourcePixel.a * Weight;
                        wr += SourcePixel.r * Weight;
                        wg += SourcePixel.g * Weight;
                        wb += SourcePixel.b * Weight;

                        TotalWeight += Weight;
                    }
                }

                if (RespectAlpha && TotalWeight < 0.3)
                {
                    p = TransparentColor;
                }
                else
                {
                    float rTotalWeight=1.0f/TotalWeight;

                    p.a = fsize_clamp(wa * rTotalWeight);
                    p.r = fsize_clamp(wr * rTotalWeight);
                    p.g = fsize_clamp(wg * rTotalWeight);
                    p.b = fsize_clamp(wb * rTotalWeight);
                }
                put_pixel(dst, k, i, p);
            }
        }
        else
        {
#if ENABLE_OMP
            #pragma omp parallel for
#endif

            for (int i = 0; i < dst->ginfo->height; ++i)
            {
                ARGB p;
                float wr = 0.0f;    /* red weight */
                float wg = 0.0f;    /* green weight */
                float wb = 0.0f;    /* blue weight */

//                float TotalWeight = 0.0f;

                for (int j = 0; j < contrib[i].n; ++j)
                {
                    ARGB  SourcePixel = raster[contrib[i].p[j].pixel];
                    float Weight      = contrib[i].p[j].weight;

                    wr += SourcePixel.r * Weight;
                    wg += SourcePixel.g * Weight;
                    wb += SourcePixel.b * Weight;

//                    TotalWeight += Weight;
                }

                {
//                    float rTotalWeight=1.0f/TotalWeight;

                    p.a = 255;
                    p.r = fsize_clamp(wr /* * rTotalWeight */);
                    p.g = fsize_clamp(wg /* * rTotalWeight */);
                    p.b = fsize_clamp(wb /* * rTotalWeight */);
                }
                put_pixel(dst, k, i, p);
            }
        }
    }
    gfree(raster);

    gfree(contrib);
    SIZE_freebitmap(tmp);
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutsize(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.13");
        strcpy(about->namestr,      "Size");
        strcpy(about->longnamestr,  "Image Size");
        about->numoptions =         7;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           1;
        strcpy(about->option[0].name,      "Width");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           1;
        strcpy(about->option[1].name,      "Height");

        about->option[2].type =            FOP_INT | FOP_ENUM;
        about->option[2].min.i =           0;
        about->option[2].max.i =           7;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "Filter Function");
        strcpy(about->option[2].enumlist,  "Lanczos3,Mitchell,Cubic,Linear,Point,Bell,B-Spline,Quadratic");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           1;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "Respect Alpha");

        about->option[4].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "Chromakey Colour");

        about->option[5].type =            FOP_FLOAT;
        about->option[5].min.f =           0;
        about->option[5].max.f =           1;
        about->option[5].def.f =           1;
        strcpy(about->option[5].name,      "X Sharpness");

        about->option[6].type =            FOP_FLOAT;
        about->option[6].min.f =           0;
        about->option[6].max.f =           1;
        about->option[6].def.f =           1;
        strcpy(about->option[6].name,      "Y Sharpness");

        about->option[6].type =            FOP_FLOAT;
        about->option[6].min.f =           0.0000001;
        about->option[6].max.f =           999999;
        about->option[6].def.f =           1;
        strcpy(about->option[6].name,      "Scale Ratio X");

        about->option[6].type =            FOP_FLOAT;
        about->option[6].min.f =           0.0000001;
        about->option[6].max.f =           999999;
        about->option[6].def.f =           1;
        strcpy(about->option[6].name,      "Scale Ratio Y");

    }
    return about;
}

FPUBLIC int opensize(FINSTANCE** filterInstance, int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }
    *filterInstance = fx;
    return ok;
}

FPUBLIC GINFO* infosize(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo;

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        /* copy ginfo to destination */
        memcpy(ginfo, fx->source[0].ginfo, (size_t)(fx->source[0].ginfo->size));
        ginfo->framenum = framenum;
        ginfo->width    = gmax(1,fx->option[0].i);
        ginfo->height   = gmax(1,fx->option[1].i);
        if (ginfo->width<1)  ginfo->width = 1;
        if (ginfo->height<1) ginfo->height = 1;

        /* scale center point and hotspots */
        {
            /* calculate size ratio's in both directions */
            float zx = (float)ginfo->width  / (float)fx->source[0].ginfo->width;
            float zy = (float)ginfo->height / (float)fx->source[0].ginfo->height;
            if (fx->numoptions >= 9) // do we have a scale ratio? If we do then use that rather than new/old to preserve original sig figs
            {
                zx = (float)fx->option[7].f;
                zy = (float)fx->option[8].f;
            }

            /* scale hotspots */
            for (int i=0; i<ginfo->numhotspots; ++i)
            {
                ginfo->hotspottbl[i][0] = (int)(ginfo->hotspottbl[i][0] * zx);
                ginfo->hotspottbl[i][1] = (int)(ginfo->hotspottbl[i][1] * zy);
            }

            /* scale center point */
            ginfo->centerx = fround((float)ginfo->centerx * zx);
            ginfo->centery = fround((float)ginfo->centery * zy);
            /* position */
            ginfo->defaultx = fround((float)ginfo->defaultx * zx);
            ginfo->defaulty = fround((float)ginfo->defaulty * zy);
        }
    }
    return ginfo;
}

FPUBLIC int filtersize(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    float (*filterf)(float t);
    float fwidth;

    int    method        = 0;
    int    respect_alpha = 0;
    int    chromakey     = 0;
    float  xsharp        = 1.0f;
    float  ysharp        = 1.0f;

    if (fx->option)
    {
        if (fx->numoptions>=3) method        = fx->option[2].i;
        if (fx->numoptions>=4) respect_alpha = fx->option[3].i;
        if (fx->numoptions>=5) chromakey     = fx->option[4].i;
        if (fx->numoptions>=6) xsharp        = (float)fx->option[5].f;
        if (fx->numoptions>=7) ysharp        = (float)fx->option[6].f;
    }

    /* get filter function based on method */
    switch (method)
    {
        case 0:     filterf = Lanczos3_filter;  fwidth = Lanczos3_support;  break;
        case 1:     filterf = Mitchell_filter;  fwidth = Mitchell_support;  break;
        case 2:     filterf = cubic_filter;     fwidth = cubic_support;     break;
        case 3:     filterf = triangle_filter;  fwidth = triangle_support;  break;
        case 4:     filterf = box_filter;       fwidth = box_support;       break;
        case 5:     filterf = bell_filter;      fwidth = bell_support;      break;
        case 6:     filterf = B_spline_filter;  fwidth = B_spline_support;  break;
        case 7:     filterf = Quadratic_filter;  fwidth = Quadratic_support;    break;

        default:    filterf = Lanczos3_filter;  fwidth = Lanczos3_support;  break;
    }

    /* size image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        size(&(fx->source[0]), &Destination, filterf, fwidth, respect_alpha, chromakey, xsharp,ysharp);
    }

    return 1;
}

FPUBLIC int closesize(FINSTANCE *fx)
{
    return gfree((void*)fx);
}

#undef PI
#undef cubic_support
#undef box_support
#undef triangle_support
#undef bell_support
#undef B_spline_support
#undef Lanczos3_support
#undef Mitchell_support
#undef B
#undef C
#undef Quadratic_filter
#undef R

