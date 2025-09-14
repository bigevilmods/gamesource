// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 Nil - Nil Filter (Returns Image Asis)            */
/*                                                                  */
/*                by Frank Barchard and Gerry Shaw, EAC             */
/*                                                                  */
/*                    Header Module - Nov 11, 1995                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950510 GS initial version                                */
/*    1.01 951107 FB rewrote to return image; updated to 1.04 API   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* void fnil(void)                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fnil.h"
#include <string.h>

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutfnil(void)
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
        strcpy(about->namestr,      "FNil");
        strcpy(about->longnamestr,  "Nil");
        about->numoptions =         0;
    }
    return(about);
}

FPUBLIC int openfnil(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infofnil(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterfnil(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
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

FPUBLIC int closefnil(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}


