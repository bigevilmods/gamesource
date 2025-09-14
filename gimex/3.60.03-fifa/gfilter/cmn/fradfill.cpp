// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Radius Fill - Background Removal Filter            */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960222 GS based on ffill                                 */
/*    1.01 960301 GS fixed bug when checking hues, added options    */
/*    1.02 960312 GS fixed infinite loop bug                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* radfill(h+,h-, s+,s-, v+,v-, radius, fillcolor)                  */
/*                                                                  */
/* Removes background from given image.  Uses hotpoints as sampling */
/* points.  Seeds fill at top-left and top-right corners.           */
/*                                                                  */
/* h+,h- are hue delta's that are added to hue when testing         */
/* s+,s- are saturation delta's                                     */
/* v+,v- are value delta's                                          */
/* radius is the number of pixels to sample out from each hot point */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fradfill.h"
#include <string.h>
#include <stdlib.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define readpixel(x,y) \
    ((ARGB*)((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8))

#define writepixel(x,y,color) \
    ((*(ARGB*)((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8)) = *(color))

#define MAXSAMPLES  255
static int  m_NumSamples = 0;
static ARGB m_Sample[MAXSAMPLES];

#include "hsv.h"

int m_MaxDelta[3] = { +5, +100, +25 };
int m_MinDelta[3] = { -5, -100, -25 };
int m_Radius = 10;

static int RF_CompareHues(double theta, double hue, int MinDelta, int MaxDelta)
{
    double maxhue = hue + MaxDelta;
    double minhue = hue + MinDelta;

    if (minhue < 0)
        minhue = 360 + minhue;
    if (maxhue >= 360)
        maxhue = maxhue - 360;

    if (((maxhue >= minhue) && (theta >= minhue && theta <= maxhue)) ||
        ((minhue >  maxhue) && (theta >= minhue || theta <= maxhue)))
        return 1;
    else
        return 0;
}

static int RF_Compare(ARGB* c1, ARGB* c2)
{
    /* c1 is the color being tested */
    /* c2 is a color in the sample list */

    double rgb1[3];
    double rgb2[3];
    double hsv1[3];
    double hsv2[3];

    int ok = 0;

    rgb1[0] = c1->r / 255.0;
    rgb1[1] = c1->g / 255.0;
    rgb1[2] = c1->b / 255.0;
    rgb_hsv(rgb1,hsv1);

    rgb2[0] = c2->r / 255.0;
    rgb2[1] = c2->g / 255.0;
    rgb2[2] = c2->b / 255.0;
    rgb_hsv(rgb2,hsv2);

    hsv1[0] *= 360.0;   // hue is [0,360]
    hsv1[1] *= 100.0;   // saturation is [0..100]
    hsv1[2] *= 100.0;   // value is [0..100]

    hsv2[0] *= 360.0;
    hsv2[1] *= 100.0;
    hsv2[2] *= 100.0;


    /* check hue - check if angle is in a arc (kinda tricky) */
    if (RF_CompareHues(hsv1[0], hsv2[0], m_MinDelta[0], m_MaxDelta[0]))
    {
        /* check saturation */
        if ((hsv1[1] >= hsv2[1] + m_MinDelta[1]) && (hsv1[1] <= hsv2[1] + m_MaxDelta[1]))
        {
            /* check value */
            if ((hsv1[2] >= hsv2[2] + m_MinDelta[2]) && (hsv1[2] <= hsv2[2] + m_MaxDelta[2]))
            {
                ok = 1;

                if (!(hsv1[0]==hsv2[0] && hsv1[1]==hsv2[1] && hsv1[2]==hsv2[2]))
                {
                    ok = 1; // used to set a break point here
                }

            }
        }
    }
    return ok;
}

static int RF_InSampleList(ARGB* Color)
{
    /* return non-zero if color should be filled, zero if color should be left as is */
    /* compare each sample color withh color, if any match than replace color (return 1) */

    int i;
    for (i = 0; i< m_NumSamples; i++)
    {
        if (RF_Compare(Color, &(m_Sample[i])))
            return 1;
    }
    return 0;
}

static void RF_AddSample(ARGB* Color)
{
    /* only add color if not already in list */
    if (!RF_InSampleList(Color) && m_NumSamples < MAXSAMPLES)
    {
        m_Sample[m_NumSamples] = *Color;
        m_NumSamples++;
    }
}

static void RF_AddSampleRadius(GBITMAP* src, int X, int Y, int Radius)
{
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    ARGB* s;

    for (y = -Radius; y <= Radius; y++)
    {
        s = (ARGB*)((char *)src->image + (y+Y)*src->rowbytes);
        s += X;

        for (x = -Radius; x <= Radius; x++)
        {
            if ((x+X)>=0 && (x+X)<width && (y+Y)>=0 && (y+Y)<height)
                RF_AddSample(s);

            s++;
        }
    }
}


/* max depth of stack */
#define STACK_DEPTH 10000

/* push new segment on stack */
#define PUSH(Y, XL, XR, DY) \
{ \
    if ((sp < Stack + STACK_DEPTH) && (Y+(DY) >= 0) && (Y+(DY) < height)) \
    { \
        sp->y  = Y; \
        sp->xl = XL; \
        sp->xr = XR; \
        sp->dy = DY; \
        sp++; \
    } \
}

/* pop segment off stack */
#define POP(Y, XL, XR, DY) \
{ \
    sp--; \
    DY = sp->dy; \
    Y  = sp->y+DY; \
    XL = sp->xl; \
    XR = sp->xr; \
}

typedef struct
{
    int y;
    int xl;
    int xr;
    int dy;
} FILLSEGMENT;

static void RF_SeedFill(GBITMAP* dst, int x, int y, ARGB* FillColor)
{
    int width  = dst->ginfo->width;
    int height = dst->ginfo->height;

    /* stack of filled segments */
    FILLSEGMENT  Stack[STACK_DEPTH];
    FILLSEGMENT* sp = Stack;

    int l;
    int x1;
    int x2;
    int dy;

    if (x<0 || x>width || y<0 || y>height)
        return;

    /* hack to prevent infinte loop */
    /* not sure how the seed fill algorithm works (copied from Gems I) but there */
    /* a problem with my adaption.  It does not make sense to call this rad filter twice */
    /* even if tolerances have been adjusted since all the seed point areas will be filled */
    {
        ARGB* SeedColor = readpixel(x,y);
        if ((SeedColor->r == FillColor->r) && (SeedColor->g == FillColor->g) && (SeedColor->b == FillColor->b))
            return;
    }

    PUSH(y,   x, x,  1);        /* needed in some cases */
    PUSH(y+1, x, x, -1);        /* seed segment (popped 1st) */

    while (sp > Stack)
    {
        /* pop segment off stack and fill a neighboring scan line */
        POP(y, x1, x2, dy);

        /* segment of scan line y-dy for x1<=x<=x2 was previously filled, */
        /* now explore adjacent pixels in scan line y */
        for (x = x1; (x >= 0) && RF_InSampleList(readpixel(x,y)); x--)
            writepixel(x,y, FillColor);

        if (x >= x1)
            goto skip;

        l = x+1;
        if (l < x1)
            PUSH(y, l, x1-1, -dy);      /* leak on left? */

        x = x1+1;
        do
        {
            for (; (x < width) && RF_InSampleList(readpixel(x,y)); x++)
                writepixel(x,y, FillColor);

            PUSH(y, l, x-1, dy);

            if (x > x2+1)
                PUSH(y, x2+1, x-1, -dy);    /* leak on right? */

skip:
            for (x++; (x <= x2) && !RF_InSampleList(readpixel(x,y)); x++)
                ;

            l = x;
        } while (x <= x2);
    }
}

static void radfill(GBITMAP* dst, ARGB* FillColor)
{
    m_NumSamples = 0;

    /* add samples from hotspots */
    {
        int i;
        for (i = 0; i < dst->ginfo->numhotspots; i++)
            RF_AddSampleRadius(dst, dst->ginfo->hotspottbl[i][0], dst->ginfo->hotspottbl[i][1], m_Radius);
    }

    /* fill image */
    RF_SeedFill(dst, 0, 0, FillColor);                      /* seed top left */
    RF_SeedFill(dst, dst->ginfo->width-1, 0, FillColor);    /* seed top right */
}

#undef STACK_DEPTH
#undef PUSH
#undef POP

#undef readpixel
#undef writepixel


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutradfill(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('R','F','I','L');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.02");
        strcpy(about->namestr,      "RadFill");
        strcpy(about->longnamestr,  "Radius Fill");

        about->numoptions = 8;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           360;
        about->option[0].def.i =           5;
        strcpy(about->option[0].name,      "Hue+");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           -360;
        about->option[1].max.i =           0;
        about->option[1].def.i =           -5;
        strcpy(about->option[1].name,      "Hue-");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           100;
        about->option[2].def.i =           100;
        strcpy(about->option[2].name,      "Sat+");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           -100;
        about->option[3].max.i =           0;
        about->option[3].def.i =           -100;
        strcpy(about->option[3].name,      "Sat-");

        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           100;
        about->option[4].max.i =           0;
        about->option[4].def.i =           25;
        strcpy(about->option[4].name,      "Val+");

        about->option[5].type =            FOP_INT;
        about->option[5].min.i =           -100;
        about->option[5].max.i =           0;
        about->option[5].def.i =           -25;
        strcpy(about->option[5].name,      "Val-");

        about->option[6].type =            FOP_INT;
        about->option[6].min.i =           0;
        about->option[6].max.i =           50;
        about->option[6].def.i =           10;
        strcpy(about->option[6].name,      "Radius");

        about->option[7].type =            FOP_INT|FOP_TRUE_COLOR;
        strcpy(about->option[7].name,      "Fill Color");
    }
    return about;
}

FPUBLIC int openradfill(FINSTANCE** filterInstance, int numsources, GBITMAP* sources, int numoptions, FOP* options)
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

FPUBLIC GINFO* inforadfill(FINSTANCE* fx, int framenum)
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

FPUBLIC int filterradfill(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    char* src = (char *)fx->source[0].image;

    ARGB fillcolor;
    fillcolor.a = 0;
    fillcolor.r = 0;
    fillcolor.g = 0;
    fillcolor.b = 255;

    m_MaxDelta[0] = 5;
    m_MaxDelta[1] = 100;
    m_MaxDelta[2] = 25;
    m_MinDelta[0] = -5;
    m_MinDelta[1] = -100;
    m_MinDelta[2] = -25;
    m_Radius = 10;

    /* get options */
    if (fx->option)
    {
        if (fx->numoptions>=1) m_MaxDelta[0] = fx->option[0].i;
        if (fx->numoptions>=2) m_MinDelta[0] = fx->option[1].i;
        if (fx->numoptions>=3) m_MaxDelta[1] = fx->option[2].i;
        if (fx->numoptions>=4) m_MinDelta[1] = fx->option[3].i;
        if (fx->numoptions>=5) m_MaxDelta[2] = fx->option[4].i;
        if (fx->numoptions>=6) m_MinDelta[2] = fx->option[5].i;
        if (fx->numoptions>=7) m_Radius      = fx->option[6].i;
        if (fx->numoptions>=8)
        {
            unsigned int color = fx->option[7].i;
            fillcolor.a = (unsigned char)(color>>24);
            fillcolor.r = (unsigned char)(color>>16);
            fillcolor.g = (unsigned char)(color>>8);
            fillcolor.b = (unsigned char)(color);
        }
    }

    /* copy over bitmap, unless overwriting source */
    if (dst != src)
    {
        char* s = src;
        char* d = dst;
        int y;
        int bpl = ginfo->width*ginfo->bpp/8;

        for (y=0; y<ginfo->height; ++y)
        {
            memcpy(d,s,bpl);
            s += fx->source[0].rowbytes;
            d += rowbytes;
        }
    }

    /* radfill image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        radfill(&Destination, &fillcolor);
    }

    return 1;
}

FPUBLIC int closeradfill(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
