// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                     CSMap - Color Space Remap                    */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960227 GS initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* csmap()                                                          */
/*                                                                  */
/* Remaps image to from RGB color space to a new color space.       */
/* The resulting image contains color space info in ARGB channels.  */
/*                                                                  */
/* HSV: A=Alpha, R=Hue, G=Saturation, B=Value                       */
/*                                                                  */
/* Colour is 8 or 32 bit.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fcsmap.h"
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#include "hsv.h"

static void ARGB_AHSV(ARGB* argb, ARGB* ahsv)
{
    double rgb[3];
    double hsv[3];

    rgb[0] = argb->r / 255.0;
    rgb[1] = argb->g / 255.0;
    rgb[2] = argb->b / 255.0;

    rgb_hsv(rgb,hsv);

    ahsv->a = argb->a;
    ahsv->r = (unsigned char)(hsv[0] * 255.0);
    ahsv->g = (unsigned char)(hsv[1] * 255.0);
    ahsv->b = (unsigned char)(hsv[2] * 255.0);
}

static void csimage(GBITMAP* src, GBITMAP* dst)
{
    int x;
    int y;
    int width  = src->ginfo->width;
    int height = src->ginfo->height;

    ARGB* s;
    ARGB* d;

    for (y = 0; y < height; y++)
    {
        s = (ARGB*)((char *)src->image + y*src->rowbytes);
        d = (ARGB*)((char *)dst->image + y*dst->rowbytes);

        for (x = 0; x < width; x++)
        {
            ARGB_AHSV(s,d);
            s++;
            d++;
        }
    }
}

static void cspal(GBITMAP* src, GBITMAP* dst)
{
    int i;
    int numcolors = src->ginfo->numcolours;

    ARGB* s = src->ginfo->colourtbl;
    ARGB* d = dst->ginfo->colourtbl;

    for (i = 0; i < numcolors; i++)
    {
        ARGB_AHSV(s,d);
        s++;
        d++;
    }
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutcsmap(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('H','M','A','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.00" );
        strcpy(about->namestr,      "CSMap" );
        strcpy(about->longnamestr,  "Color Space Remap" );

        about->numoptions = 0;
    }
    return about;
}

FPUBLIC int opencsmap(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
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

FPUBLIC GINFO* infocsmap(FINSTANCE* fx, int framenum)
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

FPUBLIC int filtercsmap(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    /* filter image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        if (ginfo->bpp > 8)
            csimage(&(fx->source[0]), &Destination);
        else
            cspal(&(fx->source[0]), &Destination);
    }

    return 1;
}

FPUBLIC int closecsmap(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
