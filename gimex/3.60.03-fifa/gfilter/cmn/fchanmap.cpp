// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Chanmap - Chanmap Filter (Returns Image Asis)        */
/*                                                                  */
/*                by Frank Barchard and Gerry Shaw, EAC             */
/*                                                                  */
/*                    Header Module - Nov 11, 1995                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    0.00 96xxxx GS no history                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* void fchanmap(void)                                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fchanmap.h"
#include <string.h>

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutchanmap(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','L','I','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "FBarchard,GShaw");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "FChanmap");
        strcpy(about->longnamestr,  "Chanmap");
        about->numoptions =         0;
    }
    return(about);
}

FPUBLIC int openchanmap(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infochanmap(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterchanmap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  bpl    = ginfo->width*ginfo->bpp/8;

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closechanmap(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}


