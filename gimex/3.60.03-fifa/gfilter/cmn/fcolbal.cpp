// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    ColBal - Color Balance                        */
/*                                                                  */
/*                      by Gerry Shaw, EAC                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960508 GS based on hsl                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* colbal(float R, float G, float B, int PreserveLum)               */
/* R  Red adjust (-100..+100)                                       */
/* G  Green adjust (-100..+100)                                     */
/* B  Blue adjust (-100..+100)                                      */
/* PreserveLum, TRUE if overall lumanice is to be preserved         */
/*                                                                  */
/* -100 means 100 percent toward minimum value                      */
/* +100 means 100 percent toward maximum value                      */
/*    0 means no change                                             */
/*                                                                  */
/* ColBal adjusts ARGB values (either palette or image)             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fcolbal.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void colorbalance(ARGB* argb, double r, double g, double b)
{
    argb->r = (unsigned char)((1.0 + r/100.0) * argb->r);
    argb->g = (unsigned char)((1.0 + g/100.0) * argb->g);
    argb->b = (unsigned char)((1.0 + b/100.0) * argb->b);
}

static void colbalimage(GBITMAP* dst, double r, double g, double b)
{
    int x;
    int y;
    int width  = dst->ginfo->width;
    int height = dst->ginfo->height;

    ARGB* d;

    for (y = 0; y < height; y++)
    {
        d = (ARGB*)((char *)dst->image + y*dst->rowbytes);

        for (x = 0; x < width; x++)
        {
            colorbalance(d, r,g,b);
            d++;
        }
    }
}

static void colbalpal(GBITMAP* dst, double r, double g, double b)
{
    int i;
    int numcolors = dst->ginfo->numcolours;

    ARGB* d = dst->ginfo->colourtbl;

    for (i = 0; i < numcolors; i++)
    {
        colorbalance(d, r,g,b);
        d++;
    }
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutcolbal(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('H','S','L',' ');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->canoverwritesource = 1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.00" );
        strcpy(about->namestr,      "ColBal");
        strcpy(about->longnamestr,  "Color Balance");

        about->numoptions = 4;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           -100.0;
        about->option[0].max.f =           +100.0;
        about->option[0].def.f =           0;
        strcpy(about->option[0].name,      "Red");

        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -100.0;
        about->option[1].max.f =           +100.0;
        about->option[1].def.f =           0;
        strcpy(about->option[1].name,      "Green");

        about->option[2].type =            FOP_FLOAT;
        about->option[2].min.f =           -100.0;
        about->option[2].max.f =           +100.0;
        about->option[2].def.f =           0;
        strcpy(about->option[2].name,      "Blue");

        about->option[3].type =            FOP_INT;
        about->option[3].min.f =           0;
        about->option[3].max.f =           1;
        about->option[3].def.f =           1;
        strcpy(about->option[3].name,      "Preserve Luminance");
    }
    return about;
}

FPUBLIC int opencolbal(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx = 0;

    /* insert code to validate options */

    /* allocate instance */
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

FPUBLIC GINFO* infocolbal(FINSTANCE* fx, int framenum)
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

FPUBLIC int filtercolbal(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    char* src = (char *)fx->source[0].image;

    double r=0;
    double g=0;
    double b=0;

    /* get options */
    if (fx->option)
    {
        if (fx->numoptions>=1) r = fx->option[0].f;
        if (fx->numoptions>=2) g = fx->option[1].f;
        if (fx->numoptions>=3) b = fx->option[2].f;
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

    /* filter image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        if (ginfo->bpp > 8)
            colbalimage(&Destination, r,g,b);
        else
            colbalpal(&Destination, r,g,b);
    }

    return 1;
}

FPUBLIC int closecolbal(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
