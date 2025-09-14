// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       Shade - Shade Filter                       */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 030916 FB based on grey, rect and pixel                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* shade(ARGB shadevalue, ARGB oldshade)                            */
/*                                                                  */
/* Shades an image (or clut).                                       */
/* Used for halfalpha, halfpixel and doublealpha                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fshade.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

// could be handle helper

static void ShadeARGB(ARGB *dest, int argbnew, int argbold, int n)
{
    int aNew=(argbnew>>24)&255;
    int rNew=(argbnew>>16)&255;
    int gNew=(argbnew>>8 )&255;
    int bNew=(argbnew>>0 )&255;

    int aOld=(argbold>>24)&255;
    int rOld=(argbold>>16)&255;
    int gOld=(argbold>>8 )&255;
    int bOld=(argbold>>0 )&255;

    if ((argbnew&0x00ffffff)==(argbold&0x00ffffff))
    {
        for (int i=0; i<n; ++i)
        {
            dest->a = (unsigned char) gmin(255,((dest->a*aNew + (aOld+1)/2) / aOld));
            ++dest;
        }
    }
    else
    {
        for (int i=0; i<n; ++i)
        {
            dest->a = (unsigned char) gmin(255,((dest->a*aNew + (aOld+1)/2) / aOld));
            dest->r = (unsigned char) gmin(255,((dest->r*rNew + (rOld+1)/2) / rOld));
            dest->g = (unsigned char) gmin(255,((dest->g*gNew + (gOld+1)/2) / gOld));
            dest->b = (unsigned char) gmin(255,((dest->b*bNew + (bOld+1)/2) / bOld));
            ++dest;
        }
    }


}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutshade(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','H','A','D');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "shade");
        strcpy(about->longnamestr,  "Shade ARGB");
        about->numoptions =         0;
        about->numoptions =         2;

        about->option[0].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0xffffffff;
        strcpy(about->option[0].name,      "argbnew");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0xffffffff;
        strcpy(about->option[1].name,      "argbold");
        strcpy(about->option[1].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openshade(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *) galloc(sizeof(FINSTANCE));
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

FPUBLIC GINFO *infoshade(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    int argbnew=0xffffffff;
    int argbold=0xffffffff;
    if (fx->option)
    {
        if (fx->numoptions>=1) argbnew = fx->option[0].i;
        if (fx->numoptions>=2) argbold = fx->option[1].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        ShadeARGB(ginfo->colourtbl,argbnew,argbold,256);
    }
    return(ginfo);
}

FPUBLIC int filtershade(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;

    int argbnew=0xffffffff;
    int argbold=0xffffffff;
    if (fx->option)
    {
        if (fx->numoptions>=1) argbnew = fx->option[0].i;
        if (fx->numoptions>=2) argbold = fx->option[1].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            ShadeARGB((ARGB *)dest,argbnew,argbold,width);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closeshade(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

