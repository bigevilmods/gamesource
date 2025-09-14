// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                      Smooth - Edge Cleaner                       */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960715 GS initial version based on Dust & Scratches      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* smooth(radius,threshold)                                         */
/* int radius       color of background                             */
/* int threshold    number of pixs to search for 5th matching color */
/*                                                                  */
/* Smooth looks for "defects" (areas that are markedly different    */
/* from their surroundings) and blurs the surrounding pixles into   */
/* the dfects to eliminate them.  The threshold setting determines  */
/* how different from the surrounding pixels something has to be in */
/* order fto be detected as a defect; a setting between 70 and 128  */
/* often works well.  The radius setting determines how far from    */
/* the edge of the defect the filter goest to get the pixels used   */
/* in the blur.                                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fsmooth.h"
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define readpixel(x,y) \
    (*((ARGB*)((char *)src->image + (y)*src->rowbytes + (x)*src->ginfo->bpp/8)))

#define writepixel(x,y,color) \
    ((*(ARGB*)((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8)) = (color))


#define INRANGE(x,y) \
    ((x-1 >= 0) && (y-1 >= 0) && (x-1 < src->ginfo->width) && (y-1 < src->ginfo->height))

#define ADDCOLOR(x,y) \
{ \
    ARGB Color = readpixel(x,y); \
    A += Color.a; \
    R += Color.r; \
    G += Color.g; \
    B += Color.b; \
    Contributors++; \
}

static ARGB GetAverageColor(GBITMAP* src, int x, int y, int Radius)
{
    ARGB AverageColor;

    int A = 0;
    int R = 0;
    int G = 0;
    int B = 0;
    int Contributors = 0;

    int dx;
    int dy;

    while (Radius > 0)
    {
        // top row
        dy = -Radius;
        for (dx = -Radius; dx <= Radius; dx++)
        {
            if (INRANGE(x+dx,y+dy))
                ADDCOLOR(x+dx,y+dy);
        }

        // bottom row
        dy = Radius;
        for (dx = -Radius; dx <= Radius; dx++)
        {
            if (INRANGE(x+dx,y+dy))
                ADDCOLOR(x+dx,y+dy);
        }

        // left row
        dx = -Radius;
        for (dy = -Radius+1; dy <= Radius-1; dy++)
        {
            if (INRANGE(x+dx,y+dy))
                ADDCOLOR(x+dx,y+dy);
        }

        // right row
        dx = Radius;
        for (dy = -Radius+1; dy <= Radius-1; dy++)
        {
            if (INRANGE(x+dx,y+dy))
                ADDCOLOR(x+dx,y+dy);
        }

        Radius--;
    }

    AverageColor.a = (unsigned char)(A/Contributors);
    AverageColor.r = (unsigned char)(R/Contributors);
    AverageColor.g = (unsigned char)(G/Contributors);
    AverageColor.b = (unsigned char)(B/Contributors);

    return AverageColor;
}

static double RGBDistance(ARGB c1, ARGB c2)
{
    double Distance  =                  // calc distance between colors
        (c1.r - c2.r) * (c1.r - c2.r) +
        (c1.g - c2.g) * (c1.g - c2.g) +
        (c1.b - c2.b) * (c1.b - c2.b);

    return Distance;
}

#define ANALYZE(sx,sy) \
{ \
    if (INRANGE(x+sx,y+sy)) \
    { \
        ARGB TestColor = readpixel(x+sx,y+sy); \
        double Distance = RGBDistance(AverageColor, TestColor); \
        if (Distance < BestDistance) \
        { \
            BestColor = TestColor; \
            BestDistance = Distance; \
        } \
    } \
}

static ARGB GetBestMatchColor(GBITMAP* src, ARGB AverageColor, int x, int y, int Radius)
{
    ARGB   BestColor={0,0,0,0};
    double BestDistance = 255*255*3;    // closest distance found so far

    int dx;
    int dy;

    while (Radius > 0)
    {
        // top row
        for (dx = -Radius; dx <= Radius; dx++)
            ANALYZE(dx,Radius);

        // bottom row
        for (dx = -Radius; dx <= Radius; dx++)
            ANALYZE(dx,-Radius);

        // left row
        for (dy = -Radius+1; dy <= Radius-1; dy++)
            ANALYZE(-Radius,dy);

        // right row
        for (dy = -Radius+1; dy <= Radius-1; dy++)
            ANALYZE(Radius,dy);

        Radius--;
    }

    return BestColor;
}

static int ColorsAreEqual(ARGB TestColor, ARGB AverageColor, int T)
{
    double Threshold = T/255.0;         // normalize it to 0..1
    double Distance  = RGBDistance(TestColor, AverageColor);    // calc distance between colors
    double Normal    = 255*255*3;       // value to normalize distance to 0..1

    return (Distance/Normal < Threshold);
}

static void smooth(GBITMAP* src, GBITMAP* dst, int Threshold, int Radius)
{
    int x;
    int y;

    for (y = 0; y < src->ginfo->height; y++)
    {
        for (x = 0; x < src->ginfo->width; x++)
        {
            ARGB CurrentColor = readpixel(x,y);
            ARGB AverageColor = GetAverageColor(src, x,y, Radius);
            if (ColorsAreEqual(AverageColor, CurrentColor, Threshold))
                writepixel(x,y, CurrentColor);
            else
            {
                ARGB BestMatchColor = GetBestMatchColor(src, AverageColor, x,y, Radius);
                writepixel(x,y, BestMatchColor);
            }
        }
    }
}

#undef readpixel
#undef writepixel

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutsmooth(void)
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
        strcpy(about->versionstr,   "1.00" );
        strcpy(about->namestr,      "Smooth" );
        strcpy(about->longnamestr,  "Smooth Edges" );

        about->numoptions = 2;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           255;
        about->option[0].def.i =           70;
        strcpy(about->option[0].name,      "Threshold");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           1;
        about->option[1].max.i =           16;
        about->option[1].def.i =           1;
        strcpy(about->option[1].name,      "Radius");
    }
    return about;
}

FPUBLIC int opensmooth(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
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

FPUBLIC GINFO* infosmooth(FINSTANCE* fx, int framenum)
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

FPUBLIC int filtersmooth(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    int threshold = 45;
    int radius = 3;

    /* get options */
    if (fx->option)
    {
        if (fx->numoptions>=1) threshold = fx->option[0].i;
        if (fx->numoptions>=2) radius    = fx->option[1].i;
    }

    /* filter image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        smooth(&(fx->source[0]), &Destination, threshold, radius);
    }

    return 1;
}

FPUBLIC int closesmooth(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
