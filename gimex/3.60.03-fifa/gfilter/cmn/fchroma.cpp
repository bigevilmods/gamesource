// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Chroma - Chroma Key Blue Screen Remover            */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960704 GS initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* chroma(tolerance)                                                */
/* int tolerance    amount pixels must differ to be left in image   */
/*                                                                  */
/* Removes the blue screen from an image.  The image will be left   */
/* with an alpha channel, use a blur filter on the alpha to smooth  */
/* the edges.                                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fchroma.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if defined(__APPLE__)
inline float sqrtf(float v)
{
    return (float) sqrt((double) v);
}
#endif

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define readpixel(x,y) \
    (*((ARGB*)((char *)src->image + (y)*src->rowbytes + (x)*src->ginfo->bpp/8)))

#define readmappixel(x,y) \
    (*((ARGB*)((char *)map->image + (y)*map->rowbytes + (x)*map->ginfo->bpp/8)))

#define writepixel(x,y,color) \
    ((*(ARGB*)((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8)) = (color))

#if 1

typedef int     (*CompareFunction)(ARGB* c, int Tolerance);
typedef enum    { TransparentPixel, OpaquePixel, EdgePixel } CHROMAPIXELTYPE;
typedef struct
{
    ARGB Color;
    int  x;
    int  y;
} MATCH;


static int TransparentCompare(ARGB* c, int Tol)
{
    int MaxRG = gmax(c->r, c->g);
    int B     = c->b;

    return MaxRG+Tol*2 < B;
}

static int TranslucentCompare(ARGB* c, int Tol)
{
    int MaxRG = gmax(c->r, c->g);
    int B     = c->b;

    return MaxRG+Tol < B;
}

static int OpaqueCompare(ARGB* c, int Tol)
{
    return !(TransparentCompare(c, Tol) || TranslucentCompare(c, Tol));
}

static CHROMAPIXELTYPE GetPixelType(ARGB* c, int Tolerance)
{
    if (TransparentCompare(c, Tolerance))
        return TransparentPixel;
    else if (TranslucentCompare(c, Tolerance))
        return EdgePixel;
    else
        return OpaquePixel;
}



#define AddMatch(c, sx,sy) \
{ \
    Match[NumMatches].Color = c; \
    Match[NumMatches].x = sx; \
    Match[NumMatches].y = sy; \
    NumMatches++;\
}

#define InBounds(sx,sy,w,h) \
    (sx >= 0 && sy >= 0 && sx < w && sy < h)

static void FindColor(GBITMAP* src, int X, int Y, int Tolerance,
                      ARGB* c, double* d, CompareFunction compare)
{
    // search in *circular* pattern to find closest matching color
    // if more than one color matches at the same distance then take the average
    // color of the matching colors

    int r;                      // current radius to search
    int sx,sy;                  // current search coordinates
    MATCH Match[32];            // list of matches found
    int NumMatches = 0;         // number of matches found
    ARGB TestColor;

    memset(Match, 0, sizeof(Match));

    r = 1;
    while (NumMatches == 0 && r < 10)
    {
        // search top row
        sy = Y-r;
        for (sx = X-r; sx < X+r; sx++)
        {
            if (InBounds(sx,sy, src->ginfo->width,src->ginfo->height))
            {
                TestColor = readpixel(sx, sy);
                if (compare(&TestColor, Tolerance))
                    AddMatch(TestColor, sx,sy);
            }
        }

        // search bottom row
        sy = Y+r;
        for (sx = X-r; sx < X+r; sx++)
        {
            if (InBounds(sx,sy, src->ginfo->width,src->ginfo->height))
            {
                TestColor = readpixel(sx, sy);
                if (compare(&TestColor, Tolerance))
                    AddMatch(TestColor, sx,sy);
            }
        }

        // search left column
        sx = X-r;
        for (sy = Y-r; sy < Y+r; sy++)
        {
            if (InBounds(sx,sy, src->ginfo->width,src->ginfo->height))
            {
                TestColor = readpixel(sx, sy);
                if (compare(&TestColor, Tolerance))
                    AddMatch(TestColor, sx,sy);
            }
        }

        // search right column
        sx = X+r;
        for (sy = Y-r; sy < Y+r; sy++)
        {
            if (InBounds(sx,sy, src->ginfo->width,src->ginfo->height))
            {
                TestColor = readpixel(sx, sy);
                if (compare(&TestColor, Tolerance))
                    AddMatch(TestColor, sx,sy);
            }
        }

        r++;
    }

    // calculate color and distance
    if (NumMatches > 0)
    {
        int i;
        int A = 0;
        int R = 0;
        int G = 0;
        int B = 0;
        double D = 0.0;

        for (i = 0; i < NumMatches; i++)
        {
            int dx = X - Match[i].x;
            int dy = Y - Match[i].y;

            D += sqrtf((float)(dx*dx + dy*dy));
            A += Match[i].Color.a;
            R += Match[i].Color.r;
            G += Match[i].Color.g;
            B += Match[i].Color.b;
        }

        *d = D/NumMatches;
        c->a = (unsigned char)(A/NumMatches);
        c->r = (unsigned char)(R/NumMatches);
        c->g = (unsigned char)(G/NumMatches);
        c->b = (unsigned char)(B/NumMatches);
    }
    else
    {
        *d = 00000;
        c->a = 0;
        c->r = 0;
        c->g = 255;
        c->b = 0;
    }
}


#define CLAMP(c)    (gmax(0, gmin(c, 255)))

static ARGB GetTransparentColor()
{
    ARGB TransparentColor;
    TransparentColor.a = 0;
    TransparentColor.r = 0;
    TransparentColor.g = 0;
    TransparentColor.b = 0;
    return TransparentColor;
}

static ARGB GetOpaqueColor(ARGB SourceColor)
{
    ARGB OpaqueColor;
    OpaqueColor.a = 255;
    OpaqueColor.r = SourceColor.r;
    OpaqueColor.g = SourceColor.g;
    OpaqueColor.b = SourceColor.b;
    return OpaqueColor;
}
#if 0
static ARGB GetEdgeColor(ARGB SourceColor)
{
    ARGB EdgeColor;
    EdgeColor.a = 128;
    EdgeColor.r = SourceColor.r;
    EdgeColor.g = SourceColor.g;
    EdgeColor.b = SourceColor.b;
    return EdgeColor;
}
#endif

static ARGB GetEdgeColor(GBITMAP* src, int x, int y, int Tolerance, ARGB SourceColor)
{
    ARGB EdgeColor;                 /* color of this edge pixel */
    ARGB TransparentColor;          /* color of closest transparent pixel */
    ARGB OpaqueColor;               /* color of closest opaque pixel */

    double TransparentDistance;     /* distance to closest transparent pixel */
    double OpaqueDistance;          /* distance to closest opaque pixel */

    double AlphaLevel;              /* alpha level of edge pixel in range [0,1] */

    /* find closest transparent and opaque pixels to this edge pixel */
    FindColor(src, x,y, Tolerance, &TransparentColor, &TransparentDistance, TransparentCompare);
    FindColor(src, x,y, Tolerance, &OpaqueColor,      &OpaqueDistance,      OpaqueCompare);

    /* calculate alpha level based on distance from opaque pixel */
    AlphaLevel = OpaqueDistance / (TransparentDistance + OpaqueDistance);

    /* calculate edge color based on transparent color and source color */
    EdgeColor.a = (unsigned char)((1-AlphaLevel) * 255);
    EdgeColor.r = (unsigned char)(CLAMP(SourceColor.r - (int)TransparentColor.r * AlphaLevel));
    EdgeColor.g = (unsigned char)(CLAMP(SourceColor.g - (int)TransparentColor.g * AlphaLevel));
    EdgeColor.b = (unsigned char)(CLAMP(SourceColor.b - (int)TransparentColor.b * AlphaLevel));

    return EdgeColor;
}

static GBITMAP* gallocbitmap(int width, int height, int originalbpp)
{
    int bpp = (originalbpp > 8) ? 32 : 8;
    int rowbytes = width*bpp / 8;

    GBITMAP* dst = (GBITMAP*)galloc(sizeof(GBITMAP));
    memset(dst, 0, sizeof(GBITMAP));

    dst->ginfo = (GINFO*)galloc(sizeof(GINFO));
    memset(dst->ginfo, 0, sizeof(GINFO));

    dst->image = galloc(rowbytes * height);
    memset(dst->image, 0, rowbytes * height);

    dst->rowbytes = rowbytes;

    /* assign known GINFO fields */

    dst->ginfo->size        = sizeof(GINFO);
    dst->ginfo->version     = 200;
    dst->ginfo->bpp         = bpp;
    dst->ginfo->originalbpp = originalbpp;
    dst->ginfo->width       = width;
    dst->ginfo->height      = height;
    dst->ginfo->redbits     = gmin(8, bpp/3);
    dst->ginfo->greenbits   = gmin(8, bpp/3);
    dst->ginfo->bluebits    = gmin(8, bpp/3);
    dst->ginfo->alphabits   = bpp - gmin(8, bpp/3)*3;

    return dst;
}

static GBITMAP* BuildEdgeMap(GBITMAP* src, int Tolerance, GBITMAP* dst)
{
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    CHROMAPIXELTYPE PixelType;  /* type of pixel currently being processed */
    ARGB SourceColor;       /* color of pixel currently being processsed */

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            /* read color of pixel to process */
            SourceColor = readpixel(x,y);

            /* determine type of pixel based on color and tolerance */
            PixelType = GetPixelType(&SourceColor, Tolerance);

            /* write destination pixel based on source pixel type and color */
            if (PixelType == TransparentPixel)
            {
                ARGB Color;
                Color.a = Color.r = Color.g = Color.b = 0;
                writepixel(x,y, Color);
            }
            else if (PixelType == OpaquePixel)
            {
                ARGB Color;
                Color.a = Color.r = Color.g = Color.b = 255;
                writepixel(x,y, Color);
            }
            else /* PixelType == EdgePixel */
            {
                ARGB Color;
                Color.a = Color.r = Color.g = Color.b = 128;
                writepixel(x,y, Color);
            }
        }
    }
    return dst;
}

static void RemoveBackground(GBITMAP* src, GBITMAP* map, GBITMAP* dst, int Tolerance)
{
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    ARGB SourceColor;   /* color of pixel currently being processsed */
    ARGB MapColor;      /* map color of pixel currently being processsed */

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            /* read color of pixel to process */
            SourceColor = readpixel(x,y);
            MapColor = readmappixel(x,y);

            /* write destination pixel based on source pixel type and color */
            if (MapColor.g == 0)
            {
                ARGB TransparentColor = GetTransparentColor();
                writepixel(x,y, TransparentColor);
            }
            else if (MapColor.g == 255)
            {
                ARGB OpaqueColor = GetOpaqueColor(SourceColor);
                writepixel(x,y, OpaqueColor);
            }
            else /* EdgePixel */
            {
                //ARGB EdgeColor = GetEdgeColor(SourceColor);
                ARGB EdgeColor = GetEdgeColor(src, x,y, Tolerance, SourceColor);
                writepixel(x,y, EdgeColor);
            }
        }
    }
}

#include "fsmooth.h"


static GBITMAP* SmoothEdgeMap(GBITMAP* src)
{
    GBITMAP* dst = gallocbitmap(src->ginfo->width,src->ginfo->height,32);

    FINSTANCE* fx;
    GINFO* ginfo;
    opensmooth(&fx, 1, src, 0, NULL);
    ginfo = infosmooth(fx, 0);

    // call smooth to place result into new image
    filtersmooth(fx, ginfo, (char *)dst->image, dst->rowbytes);
    closesmooth(fx);

    gfree(dst->ginfo);
    dst->ginfo = ginfo;

    return dst;
}

static void CHROMA_gfreebitmap(GBITMAP *gm)
{
    int isone;

    if (gm)
    {
        isone = 0;
        if ((char *)gm->ginfo == (char *)gm+sizeof(GBITMAP))
            isone = 1;

        if (!isone)
        {
            if (gm->image)
                gfree(gm->image);
            if (gm->ginfo)
                gfree(gm->ginfo);
        }
        gfree(gm);
    }
}


static void chromakey(GBITMAP* src, GBITMAP* dst, int Tolerance)
{
    // build edge map from source and tolerance
    GBITMAP* edgemap = gallocbitmap(src->ginfo->width, src->ginfo->height, 32);
    GBITMAP* em2;
    BuildEdgeMap(src, Tolerance, edgemap);
    //memcpy(dst->image, edgemap->image, dst->ginfo->height*dst->rowbytes);

    // smooth edge map using fsmooth filter
    em2 = SmoothEdgeMap(edgemap);
    CHROMA_gfreebitmap(edgemap);

    // build dst using edge map
//  memcpy(dst->image, em2->image, dst->ginfo->height*dst->rowbytes);
    RemoveBackground(src, em2, dst, Tolerance);
    CHROMA_gfreebitmap(em2);
}

#else

static void chromakey_old(GBITMAP* src, GBITMAP* dst, int Tolerance)
{
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    ARGB TransparentColor = {0,0,0,0};
    ARGB TranslucentColor;
    ARGB OpaqueColor = { 0,0,0,255};

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            ARGB TestColor = readpixel(x,y);

            int MaxRG = gmax(TestColor.r,TestColor.g);
            int B     = TestColor.b;

            if ((MaxRG + Tolerance*2) < B)
            {
                writepixel(x,y, TransparentColor);
            }
            else if ((MaxRG + Tolerance) < B)
            {
                int    a = (B - (MaxRG+Tolerance))*100/Tolerance;
                double A = 1.0 - a/100.0;

                TranslucentColor.a = (unsigned char)a;
                TranslucentColor.r = (unsigned char)((A*TestColor.r));
                TranslucentColor.g = (unsigned char)((A*TestColor.r));// + ((1.0-A)*255.0));
                TranslucentColor.b = (unsigned char)((A*TestColor.r));

                writepixel(x,y, TranslucentColor);
            }
            else
            {
                OpaqueColor.r = TestColor.r;
                OpaqueColor.g = TestColor.g;
                OpaqueColor.b = TestColor.b;

                writepixel(x,y, OpaqueColor);
            }
        }
    }
}
#endif

#undef readpixel
#undef writepixel


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutchroma(void)
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
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Chroma");
        strcpy(about->longnamestr,  "Chroma Key");

        about->numoptions = 1;

        about->option[0].type =         FOP_INT;
        about->option[0].min.i =        0;
        about->option[0].max.i =        255;
        about->option[0].def.i =        45;
        strcpy(about->option[0].name,   "Tolerance");
    }
    return about;
}

FPUBLIC int openchroma(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx = 0;

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

FPUBLIC GINFO* infochroma(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo;

    ginfo = (GINFO*) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        /* copy ginfo to destination */
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->framenum = framenum;
    }
    return ginfo;
}

FPUBLIC int filterchroma(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    int tolerance = 45;

    /* get options */
    if (fx->option)
    {
        if (fx->numoptions>=1) tolerance = fx->option[0].i;
    }

    /* filter image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        tolerance = 20;
        //chromakey_old(&(fx->source[0]), &Destination, tolerance);
        chromakey(&(fx->source[0]), &Destination, tolerance);
    }

    return 1;
}

FPUBLIC int closechroma(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
