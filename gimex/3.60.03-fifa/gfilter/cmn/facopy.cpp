// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*         FACopy - Copy Alpha Channel To/From RGB Channels         */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on fflip                                 */
/*    1.01 951107 FB renamed to acopy for consistany; API 1.04      */
/*    1.02 970128 FB copy to alpha                                  */
/*    1.03 970506 FB updated about                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* acopy(int method)                                                */
/*                                                                  */
/* Methods                                                          */
/* 0 copy alpha to red, green, blue                                 */
/* 1 copy max(red,green,blue)                                       */
/*                                                                  */
/* Copies Alpha value to R,G and B channels.  Useful exporting to   */
/* tools that only support 24 bit images or palettes.               */
/* Can be used to manipulate alpha channels for special effects.    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "facopy.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void acopy(ARGB *dest,int method,int n)
{
    int i;

    for (i=0; i<n; ++i)
    {
        switch (method)
        {
            default:
            case 0:
                dest->r = dest->g =  dest->b = dest->a;
                break;

            case 1:
                dest->a = (unsigned char)(gmax(dest->r,gmax(dest->g,dest->b)));
                break;
        }

        ++dest;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutacopy(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('A','C','P','Y');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.02");
        strcpy(about->namestr,      "ACopy");
        strcpy(about->longnamestr,  "Alpha Copy");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "method");
        strcpy(about->option[0].enumlist,  "rgb,alpha");
    }
    return(about);
}

FPUBLIC int openacopy(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoacopy(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int  method=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) method = fx->option[0].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, (size_t)(fx->source[0].ginfo->size));
        acopy(ginfo->colourtbl,method,256);
    }
    return(ginfo);
}

FPUBLIC int filteracopy(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    int  method=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) method = fx->option[0].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            acopy((ARGB *)dest,method,width);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closeacopy(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

