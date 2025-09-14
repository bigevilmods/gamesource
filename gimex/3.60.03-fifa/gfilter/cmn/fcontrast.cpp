// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*              Contrast - Contrast Brightness Adjust               */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960318 FB based on hsv                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* contrast(float contrast,float brightness)                        */
/* contrast    Contrast adjust level (-100..100)                    */
/* brightness  Brightness adjust level (-100..100)                  */
/*             -100 means 100 percent toward minimum value          */
/*              100 means 100 percent toward maximum value          */
/*                0 means no change                                 */
/*                                                                  */
/* Contrast adjust ARGB values (either palette or image)            */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fcontrast.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static double Badjust(double v, double db)
{
    v = v+db*128.0/100.0+128.0;

    if (v<0.0) v = 0.0;
    if (v>255.0) v = 255.0;

    return(v);
}

static double Cadjust(double v, double dc)
{
    if (dc<0)
    {
        v = v*(dc+100.0)/100.0;
    }
    else
    {
        if (dc > 99.0) dc = 99.0;
        v = v*100.0/(100.0-dc);
    }

    return(v);
}



static void contrast(ARGB *dest,int n,double dc,double db)
{
    int i;
    double r;
    double g;
    double b;

    for (i=0; i<n; ++i)
    {
        r = (double) dest->r-128.0;
        g = (double) dest->g-128.0;
        b = (double) dest->b-128.0;

        r = Cadjust(r,dc);
        g = Cadjust(g,dc);
        b = Cadjust(b,dc);

        r = Badjust(r,db);
        g = Badjust(g,db);
        b = Badjust(b,db);

        dest->r = (unsigned char) r;
        dest->g = (unsigned char) g;
        dest->b = (unsigned char) b;

        ++dest;
    }
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutcontrast(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('C','O','N','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Contrast");
        strcpy(about->longnamestr,  "Contrast/Brightness Adjust");
        about->numoptions =         2;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           -100.0;
        about->option[0].max.f =           100.0;
        about->option[0].def.f =           0;
        strcpy(about->option[0].name,      "Contrast");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -100.0;
        about->option[1].max.f =           100.0;
        about->option[1].def.f =           0;
        strcpy(about->option[1].name,      "Brightness");
        strcpy(about->option[1].enumlist,  "");
    }
    return(about);
}

FPUBLIC int opencontrast(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infocontrast(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    double c=0;
    double b=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) c = fx->option[0].f;
        if (fx->numoptions>=2) b = fx->option[1].f;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        contrast(ginfo->colourtbl,256,c,b);
    }
    return(ginfo);
}

FPUBLIC int filtercontrast(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    double c=0;
    double b=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) c = fx->option[0].f;
        if (fx->numoptions>=2) b = fx->option[1].f;
    }


    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            contrast((ARGB *)dest,width,c,b);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closecontrast(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

