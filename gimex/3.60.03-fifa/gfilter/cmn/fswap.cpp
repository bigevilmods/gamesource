// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*              Swap Colours - Swap two colour values               */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on fxcolour                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* swap(int index1,int index2)                                      */
/*                                                                  */
/* swaps colours index1 and index2 in both the palette and bitmap.  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fswap.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void swapcolours(char *source,char *dest,int n,unsigned int search,unsigned int replace)
{
    int i;
    char c;

    for (i=0; i<n; ++i)
    {
        c = *source++;

        if (c==(char)search)
            c = (char)replace;
        else if (c==(char)replace)
            c = (char)search;

        *dest++ = c;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutswap(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','W','A','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           0;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Swap");
        strcpy(about->longnamestr,  "Swap Colours");
        about->numoptions =         2;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           255;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Colour1");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           255;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Colour2");
        strcpy(about->option[1].enumlist,  "");
    }
    return about;
}

FPUBLIC int openswap(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoswap(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    unsigned int search=0;
    unsigned int replace=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) search = fx->option[0].i;
        if (fx->numoptions>=2) replace = fx->option[1].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->colourtbl[search] = fx->source[0].ginfo->colourtbl[replace];
        ginfo->colourtbl[replace] = fx->source[0].ginfo->colourtbl[search];
    }
    return(ginfo);
}

FPUBLIC int filterswap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  width  = ginfo->width;
    unsigned int search=0;
    unsigned int replace=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) search = fx->option[0].i;
        if (fx->numoptions>=2) replace = fx->option[1].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        swapcolours(source,dest,width,search,replace);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closeswap(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

