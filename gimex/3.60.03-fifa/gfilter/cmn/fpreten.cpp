// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Preten - Pretenuate Alpha                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on invert                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* preten(int method)                                               */
/*                                                                  */
/* Methods                                                          */
/* 0 preattenuate ARGB values                                       */
/* 1 unattenuate ARGB values                                        */
/*                                                                  */
/* Converts unattenuated ARGB values to pretenuatted ARGBs          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fpreten.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void preten(ARGB *dest,int method,int n)
{
    int i;
    int a;
    int r;
    int g;
    int b;

    if (method) // unattenuate
    {
        for (i=0; i<n; ++i)
        {
            a = dest->a;
            r = dest->r;
            g = dest->g;
            b = dest->b;
            dest->a = (unsigned char)a;
            if (a)
            {
                dest->r = (unsigned char)((r*255+128)/a);
                dest->g = (unsigned char)((g*255+128)/a);
                dest->b = (unsigned char)((b*255+128)/a);
            }
            ++dest;
        }
    }
    else    // preattenuate
    {
        for (i=0; i<n; ++i)
        {
            a = dest->a;
            r = dest->r;
            g = dest->g;
            b = dest->b;
            dest->a = (unsigned char)a;
            dest->r = (unsigned char)((r*a+128)/255);
            dest->g = (unsigned char)((g*a+128)/255);
            dest->b = (unsigned char)((b*a+128)/255);
            ++dest;
        }
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutpreten(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('A','T','E','N');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Preten");
        strcpy(about->longnamestr,  "Preattenuate ARGB");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "method");
        strcpy(about->option[0].enumlist,  "preattenuate,unattenuate");
    }
    return(about);
}

FPUBLIC int openpreten(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infopreten(FINSTANCE *fx, int framenum)
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
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        preten(ginfo->colourtbl,method,256);
    }
    return(ginfo);
}

FPUBLIC int filterpreten(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
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
            preten((ARGB *)dest,method,width);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closepreten(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

