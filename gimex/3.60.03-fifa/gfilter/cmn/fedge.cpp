// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       Edge - Edge Cleaner                        */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960229 GS initial version                                */
/*    1.01 960305 GS added options and cleaned up                   */
/*    1.02 960812 FB renamed to edge (again)                        */
/*    1.03 960815 GS 8 bit support                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* edge(color,numpixels,operation)                                  */
/* int color        color of background                             */
/* int numpixels    number of pixs to search for 5th matching color */
/* int operation    2=remove pits, 1=remove spikes, 0=remove both   */
/*                                                                  */
/* Removes the spikes and pits around the background of an image.   */
/* This filter was desgined to be used after the background of an   */
/* image was removed.  This filter will clean up the edges around   */
/* the image by clipping single pixel peaks and fill single pixel   */
/* holes.  By increasing the numpixels value you can effect more    */
/* pixels.  Each call to the filter will remove more pixels and is  */
/* recommended if numpixels > 1.                                    */
/*                                                                  */
/* Note, the 8 bit version will search for the best matching color  */
/* instead of interpolating the surrounding colors                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fedge.h"
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Internal Functions 32 bit version                            */
/****************************************************************/

#define readpixel32(x,y) \
    ((ARGB*)((char *)src->image + (y)*src->rowbytes + (x)*src->ginfo->bpp/8))

#define writepixel32(x,y,color) \
    ((*(ARGB*)((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8)) = *(color))

static int CE_Compare(ARGB* c1, ARGB* c2)
{
    return (c1->a == c2->a) && (c1->r == c2->r) && (c1->g == c2->g) && (c1->b == c2->b);
}

static int CE_ColorsMatch(GBITMAP* src, int x, int y, int dx, int dy, ARGB* Color, int NumPixels)
{
    ARGB* TestColor;

    while (NumPixels > 0)
    {
        x += dx;
        y += dy;
        TestColor = readpixel32(x,y);
        if (CE_Compare(TestColor,Color))
            return 1;
        NumPixels--;
    }
    return 0;
}

static int CE_CountMatches(GBITMAP* src, int x, int y, ARGB* BackColor, int NumPixels)
{
    int Matches = 0;

    if (CE_ColorsMatch(src, x,y, -1,-1, BackColor, NumPixels))  Matches++;  /* top,left */
    if (CE_ColorsMatch(src, x,y,  0,-1, BackColor, NumPixels))  Matches++;  /* top */
    if (CE_ColorsMatch(src, x,y, +1,-1, BackColor, NumPixels))  Matches++;  /* top, right */
    if (CE_ColorsMatch(src, x,y, -1, 0, BackColor, NumPixels))  Matches++;  /* left, */
    if (CE_ColorsMatch(src, x,y, +1, 0, BackColor, NumPixels))  Matches++;  /* right */
    if (CE_ColorsMatch(src, x,y, -1,+1, BackColor, NumPixels))  Matches++;  /* bottom, left */
    if (CE_ColorsMatch(src, x,y,  0,+1, BackColor, NumPixels))  Matches++;  /* bottom */
    if (CE_ColorsMatch(src, x,y, +1,+1, BackColor, NumPixels))  Matches++;  /* bottom, right */

    return Matches;
}

#define samplelocation(dx,dy) \
    if (!CE_ColorsMatch(src,x,y, (dx),(dy), BackColor,1)) \
    { \
        ARGB* argb = readpixel32(x+dx,y+dy); \
        a += argb->a; \
        r += argb->r; \
        g += argb->g; \
        b += argb->b; \
        Matches++; \
    }


static void cleanedge(GBITMAP* src, GBITMAP* dst, ARGB* BackColor, int NumPixels, int Operation)
{
    int RemovingSpikes = (Operation != 2);
    int RemovingPits   = (Operation != 1);
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    int Matches;    /* number of pixels matching color surrounding current pixel */

    for (y = NumPixels; y < height-NumPixels; y++)
    {
        for (x = NumPixels; x < width-NumPixels; x++)
        {
            Matches = CE_CountMatches(src,x,y, BackColor, NumPixels);

            if (RemovingSpikes && Matches >= 5)
                writepixel32(x,y, BackColor); /* write pixel with background color */

            if (RemovingPits && Matches <= 3 && CE_Compare(readpixel32(x,y), BackColor))
            {
                /* write pixel with average of surrounding non-background colors */

                int a = 0;      /* use ints to calc avg to avoid overflow */
                int r = 0;
                int g = 0;
                int b = 0;
                Matches = 0;    /* counts number of colors sampled */

                samplelocation(-1,-1);
                samplelocation( 0,-1);
                samplelocation( 1,-1);
                samplelocation(-1, 0);
                samplelocation( 1, 0);
                samplelocation(-1, 1);
                samplelocation( 0, 1);
                samplelocation( 1, 1);

                {
                    ARGB FillColor;
                    FillColor.a = (unsigned char)(a / Matches);
                    FillColor.r = (unsigned char)(r / Matches);
                    FillColor.g = (unsigned char)(g / Matches);
                    FillColor.b = (unsigned char)(b / Matches);

                    writepixel32(x,y, &FillColor);
                }
            }
        }
    }
}

#undef readpixel32
#undef writepixel32
#undef samplelocation


/****************************************************************/
/* Internal Functions 8 bit version                             */
/****************************************************************/

#define PIXEL8  unsigned char

#define readpixel8(x,y) \
    ((PIXEL8*)((char *)src->image + (y)*src->rowbytes + (x)*src->ginfo->bpp/8))

#define writepixel8(x,y,color) \
    ((*(PIXEL8*)((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8)) = *(color))

static int CE_Compare8(PIXEL8* c1, PIXEL8* c2)
{
    return (*c1 == *c2);
}

static int CE_ColorsMatch8(GBITMAP* src, int x, int y, int dx, int dy, PIXEL8* Color, int NumPixels)
{
    PIXEL8* TestColor;

    while (NumPixels > 0)
    {
        x += dx;
        y += dy;
        TestColor = readpixel8(x,y);
        if (CE_Compare8(TestColor,Color))
            return 1;
        NumPixels--;
    }
    return 0;
}

static int CE_CountMatches8(GBITMAP* src, int x, int y, PIXEL8* BackColor, int NumPixels)
{
    int Matches = 0;

    if (CE_ColorsMatch8(src, x,y, -1,-1, BackColor, NumPixels))  Matches++;  /* top,left */
    if (CE_ColorsMatch8(src, x,y,  0,-1, BackColor, NumPixels))  Matches++;  /* top */
    if (CE_ColorsMatch8(src, x,y, +1,-1, BackColor, NumPixels))  Matches++;  /* top, right */
    if (CE_ColorsMatch8(src, x,y, -1, 0, BackColor, NumPixels))  Matches++;  /* left, */
    if (CE_ColorsMatch8(src, x,y, +1, 0, BackColor, NumPixels))  Matches++;  /* right */
    if (CE_ColorsMatch8(src, x,y, -1,+1, BackColor, NumPixels))  Matches++;  /* bottom, left */
    if (CE_ColorsMatch8(src, x,y,  0,+1, BackColor, NumPixels))  Matches++;  /* bottom */
    if (CE_ColorsMatch8(src, x,y, +1,+1, BackColor, NumPixels))  Matches++;  /* bottom, right */

    return Matches;
}

static unsigned char CE_FindBestMatch(ARGB* color, ARGB* pal)
{
    int finda = color->a;
    int findr = color->r;
    int findg = color->g;
    int findb = color->b;

    int BestDiff = 99999;
    unsigned char BestIndex = 0;

    int i;

    /* full search for best colour */
    for (i = 0; i < 256; i++)
    {
        int a = pal->a;
        int r = pal->r;
        int g = pal->g;
        int b = pal->b;

        int Diff = (finda-a)*(finda-a)+(findr-r)*(findr-r)+(findg-g)*(findg-g)+(findb-b)*(findb-b);
        if (Diff < BestDiff)
        {
            BestDiff = Diff;
            BestIndex = (unsigned char)i;
        }
    }

    return BestIndex;
}


#define samplelocation8(dx,dy) \
    if (!CE_ColorsMatch8(src,x,y, (dx),(dy), BackColor,1)) \
    { \
        PIXEL8* index = readpixel8(x+dx,y+dy); \
        int i2 = (int)(*index); \
        ARGB* argb = &(src->ginfo->colourtbl[i2]); \
        a += argb->a; \
        r += argb->r; \
        g += argb->g; \
        b += argb->b; \
        Matches++; \
    }


static void cleanedge8(GBITMAP* src, GBITMAP* dst, PIXEL8* BackColor, int NumPixels, int Operation)
{
    int RemovingSpikes = (Operation != 2);
    int RemovingPits   = (Operation != 1);
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    int Matches;    /* number of pixels matching color surrounding current pixel */

    for (y = NumPixels; y < height-NumPixels; y++)
    {
        for (x = NumPixels; x < width-NumPixels; x++)
        {
            Matches = CE_CountMatches8(src,x,y, BackColor, NumPixels);

            if (RemovingSpikes && Matches >= 5)
                writepixel8(x,y, BackColor); /* write pixel with background color */

            if (RemovingPits && Matches <= 3 && CE_Compare8(readpixel8(x,y), BackColor))
            {
                /* write pixel with average of surrounding non-background colors */

                int a = 0;      /* use ints to calc avg to avoid overflow */
                int r = 0;
                int g = 0;
                int b = 0;
                Matches = 0;    /* counts number of colors sampled */

                samplelocation8(-1,-1);
                samplelocation8( 0,-1);
                samplelocation8( 1,-1);
                samplelocation8(-1, 0);
                samplelocation8( 1, 0);
                samplelocation8(-1, 1);
                samplelocation8( 0, 1);
                samplelocation8( 1, 1);

                {
                    PIXEL8 FillIndex;
                    ARGB FillColor;
                    FillColor.a = (unsigned char)(a / Matches);
                    FillColor.r = (unsigned char)(r / Matches);
                    FillColor.g = (unsigned char)(g / Matches);
                    FillColor.b = (unsigned char)(b / Matches);

                    /* find best match */
                    FillIndex = CE_FindBestMatch(&FillColor, src->ginfo->colourtbl);

                    writepixel8(x,y, &FillIndex);
                }
            }
        }
    }
}

#undef readpixel8
#undef writepixel8
#undef samplelocation8




/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutedge(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('C','L','N','E');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.03" );
        strcpy(about->namestr,      "Edge" );
        strcpy(about->longnamestr,  "Clean Edges" );

        about->numoptions = 3;

        about->option[0].type =            FOP_INT|FOP_TRUE_COLOR;
        strcpy(about->option[0].name,      "Back Color");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           10;
        about->option[1].def.i =           1;
        strcpy(about->option[1].name,      "Num Pixels");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           2;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "Operation");
        strcpy(about->option[0].enumlist,  "Spikes and Pits,Spikes,Pits");
    }
    return about;
}

FPUBLIC int openedge(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
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

FPUBLIC GINFO* infoedge(FINSTANCE* fx, int framenum)
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

FPUBLIC int filteredge(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    int  numpixels = 0;
    int  operation = 0;
    unsigned char backindex = 0;
    ARGB backcolor;
    backcolor.a = 0;
    backcolor.r = 0;
    backcolor.g = 0;
    backcolor.b = 255;


    /* get options */
    if (fx->option)
    {
        if (fx->numoptions>=1)
        {
            unsigned int color = fx->option[0].i;
            if (ginfo->bpp > 8)
            {
                backcolor.a = (unsigned char)(color>>24);
                backcolor.r = (unsigned char)(color>>16);
                backcolor.g = (unsigned char)(color>>8);
                backcolor.b = (unsigned char)(color);
            }
            else
                backindex = (unsigned char)color;
        }
        if (fx->numoptions>=2) numpixels = fx->option[1].i;
        if (fx->numoptions>=3) operation = fx->option[2].i;
    }


    /* copy src into dst */
    /* Note: filter can be done inplace */
    {
        char* s = (char *)fx->source[0].image;
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

    /* filter image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        if (ginfo->bpp > 8)
            cleanedge(&(fx->source[0]), &Destination, &backcolor, numpixels, operation);
        else
            cleanedge8(&(fx->source[0]), &Destination, &backindex, numpixels, operation);

    }

    return 1;
}

FPUBLIC int closeedge(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
