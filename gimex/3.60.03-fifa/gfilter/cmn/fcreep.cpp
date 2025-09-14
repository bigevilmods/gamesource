// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   Creep - A Really Creepy Filter                 */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960223 GS initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* creep(unsigned int color, int numpixels)                        */
/*                                                                  */
/* Creeps given color by numpixels pixels.  This has the effect of  */
/* expanding the area covered by the given color                    */
/*                                                                  */
/* Colour is 8 or 32 bit.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fcreep.h"
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define readpixel(x,y) \
    (ggetm((char *)src->image + (y)*src->rowbytes + (x)*src->ginfo->bpp/8, src->ginfo->bpp/8))

#define writepixel(x,y,color) \
    (gputm((char *)dst->image + (y)*dst->rowbytes + (x)*dst->ginfo->bpp/8, (color), dst->ginfo->bpp/8))


static void creep(GBITMAP* src, GBITMAP* dst, unsigned int CreepColor, int NumPixels)
{
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    unsigned int TestColor;    /* color currently being looked at */

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            TestColor = readpixel(x,y);
            if (TestColor == CreepColor)
            {
                /* write NumPixels in all four directions */
                int n;

                /* creep left */
                n = NumPixels;
                while (n > 0 && x-n >= 0)
                {
                    writepixel(x-n,y, CreepColor);
                    n--;
                }

                /* creep right */
                n = NumPixels;
                while (n > 0 && x+n < width)
                {
                    writepixel(x+n,y, CreepColor);
                    n--;
                }

                /* creep up */
                n = NumPixels;
                while (n > 0 && y-n >= 0)
                {
                    writepixel(x,y-n, CreepColor);
                    n--;
                }

                /* creep down */
                n = NumPixels;
                while (n > 0 && y+n < height)
                {
                    writepixel(x,y+n, CreepColor);
                    n--;
                }
            }
        }
    }
}

#undef readpixel
#undef writepixel


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutcreep(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('C','R','E','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.00" );
        strcpy(about->namestr,      "Creep" );
        strcpy(about->longnamestr,  "Creep Image" );

        about->numoptions = 2;

        strcpy(about->option[0].name, "Colour");
        about->option[0].type  = FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[0].min.i = 0;
        about->option[0].max.i = 0xffffffff;
        about->option[0].def.i = 0;

        strcpy(about->option[1].name, "Amount");
        about->option[1].type  = FOP_INT;
        about->option[1].min.i = 0;
        about->option[1].max.i = 0xffffffff;
        about->option[1].def.i = 1;
    }
    return about;
}

FPUBLIC int opencreep(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
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

FPUBLIC GINFO* infocreep(FINSTANCE* fx, int framenum)
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

FPUBLIC int filtercreep(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    unsigned int color = 0;
    int amount = 1;

    if (fx->option)
    {
        if (fx->numoptions>=1) color  = fx->option[0].i;
        if (fx->numoptions>=2) amount = fx->option[1].i;
    }

    /* copy source buffer to destination buffer */
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

    /* creep image */
    {
        unsigned int c = color;

        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        color = ggetm(&c, sizeof(c));

        creep(&(fx->source[0]), &Destination, color, amount);
    }

    return 1;
}

FPUBLIC int closecreep(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
