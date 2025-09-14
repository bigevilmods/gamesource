// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Fill - Seed Fill with RGB Tolerance                */
/*                                                                  */
/*              by FrANK Barchard and Gerry Shaw, EAC               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951107 FB based on frect and latenite                    */
/*    1.01 951125 FB 8 bit dont swap bytes; 32 bit compare int      */
/*    1.02 960220 GS rewritten from code in Graphics Gems I         */
/*    1.03 960627 GS use dynamic memory vs stack for fill stack     */
/*    1.04 961106 GS tolerance is defined better (0..255)           */
/*    1.05 010118 ID fixed multilayered page format fills           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* fill(int x, int y, int colour, int tolerence)                    */
/*                                                                  */
/* Code adapted from Paul Heckberts article in Graphics Gems I.     */
/* Flood fill starting at specified coordinate, filling with        */
/* specified colour.                                                */
/*                                                                  */
/* Colour is 8 or 32 bit.                                           */
/* Tolerance is 0..255.                                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "ffill.h"
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define RED(c)      ((c>>16) & 0x0ff)
#define GREEN(c)    ((c>>8)  & 0x0ff)
#define BLUE(c)     ((c)     & 0x0ff)

static unsigned int FILLreadpixel(GBITMAP* src, int x, int y)
{
    int   bpp      = src->ginfo->bpp;
    int   rowbytes = src->rowbytes;
    char* base     = (char *) src->image;

    return ggetm(base + y*rowbytes + x*bpp/8, bpp/8);
}

static void FILLwritepixel(GBITMAP* dst, int x, int y, unsigned int value)
{
    int   bpp      = dst->ginfo->bpp;
    int   rowbytes = dst->rowbytes;
    char* base     = (char *)dst->image;

    gputm((base + y*rowbytes + x*bpp/8), value, bpp/8);
}

/* compare colors to test if within tolerance */
static int FILLcompare(unsigned int c1, unsigned int c2, int Tolerance, GBITMAP* dst)
{
    if (dst->ginfo->bpp > 8)
        return (abs((int)RED(c1)- (int)RED(c2)) + abs((int)GREEN(c1)- (int)GREEN(c2)) + abs((int)BLUE(c1)- (int)BLUE(c2)) <= Tolerance);
    else
    {
        ARGB argb1 = dst->ginfo->colourtbl[c1];
        ARGB argb2 = dst->ginfo->colourtbl[c2];
        return (abs(argb1.r - argb2.r) + abs(argb1.g - argb2.g) + abs(argb1.b - argb2.b) <= Tolerance);
    }
}

#define STACK_DEPTH 10000       /* max depth of stack */

typedef struct
{
    int y;
    int xl;
    int xr;
    int dy;
} FILLSEGMENT;

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

// FloodFill4: set the pixel at (x,y) and all of its 4-connected neighbors
// with the same pixel value to the new pixel value FillColor.
// A 4-connected neighbor is a pixel above, below, left, or right of a pixel.

static void fill(GBITMAP* dst, int x, int y, unsigned int FillColor, int Tolerance)
{
    int width  = dst->ginfo->width;
    int height = dst->ginfo->height;

    /* stack of filled segments */
    FILLSEGMENT* Stack;
    FILLSEGMENT* sp;

    int l;
    int x1;
    int x2;
    int dy;

    unsigned int SeedColor = FILLreadpixel(dst, x,y);

    if (FILLcompare(SeedColor, FillColor, Tolerance, dst) || x<0 || x>width || y<0 || y>height)
        return;

    Stack = (FILLSEGMENT*)galloc(STACK_DEPTH * sizeof(FILLSEGMENT));
    sp = Stack;

    PUSH(y,   x, x,  1);        /* needed in some cases */
    PUSH(y+1, x, x, -1);        /* seed segment (popped 1st) */

    while (sp > Stack)
    {
        /* pop segment off stack and fill a neighboring scan line */
        POP(y, x1, x2, dy);

        /* segment of scan line y-dy for x1<=x<=x2 was previously filled, */
        /* now explore adjacent pixels in scan line y */
        for (x = x1; (x >= 0) && FILLcompare(FILLreadpixel(dst, x,y), SeedColor, Tolerance, dst); x--)
            FILLwritepixel(dst, x,y, FillColor);

        if (x >= x1)
            goto skip;

        l = x+1;
        if (l < x1)
            PUSH(y, l, x1-1, -dy);      /* leak on left? */

        x = x1+1;
        do
        {
            for (; (x < width) && FILLcompare(FILLreadpixel(dst, x,y), SeedColor, Tolerance, dst); x++)
                FILLwritepixel(dst, x,y, FillColor);

            PUSH(y, l, x-1, dy);

            if (x > x2+1)
                PUSH(y, x2+1, x-1, -dy);    /* leak on right? */

skip:
            for (x++; (x <= x2) && !FILLcompare(FILLreadpixel(dst, x,y), SeedColor, Tolerance, dst); x++)
                ;

            l = x;
        } while (x <= x2);
    }

    gfree(Stack);
}

#undef STACK_DEPTH
#undef PUSH
#undef POP

#undef RED
#undef GREEN
#undef BLUE

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutfill(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','I','L','L');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->canoverwritesource = 1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "G.Shaw, F.Barchard");
        strcpy(about->versionstr,   "1.04" );
        strcpy(about->namestr,      "Fill" );
        strcpy(about->longnamestr,  "Fill Image" );
        about->numoptions =         4;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "X");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Y");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[2].min.i =           0;
        about->option[2].max.i =           0xffffffff;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "Colour");
        strcpy(about->option[2].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           255;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Tolerance");
        strcpy(about->option[1].enumlist,  "");

    }
    return(about);
}

FPUBLIC int openfill(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
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
    return(ok);
}

FPUBLIC GINFO *infofill(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterfill(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    char* src = (char *)fx->source[0].image;

    int fillx=0;
    int filly=0;
    unsigned int color=0;
    int tolerance=0;
    int radius=1;

    if (fx->option)
    {
        if (fx->numoptions>=1) fillx     = fx->option[0].i;
        if (fx->numoptions>=2) filly     = fx->option[1].i;
        if (fx->numoptions>=3) color     = fx->option[2].i;
        if (fx->numoptions>=4) tolerance = (fx->option[3].i) * (fx->option[3].i);
        if (fx->numoptions>=5) radius    = fx->option[4].i;
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
    if((fx->numoptions == 4) && (fx->option[3].i))  // if this is a pages type the only draw lines
    {                                               // on lowest layer.
        if(ginfo->framenum != 0)
            return 1;
//        else
//            printf("WARNING: Fill will only be done on lowest layer for page format files\n");
    }

    /* flood fill image */
    {
        unsigned int c = color;

        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        if (ginfo->bpp > 8)
            color = ggetm(&c, sizeof(c));

        fill(&Destination, fillx, filly, color, tolerance);
    }

    return 1;
}

FPUBLIC int closefill(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

