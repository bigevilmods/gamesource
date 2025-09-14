// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Grey - Grey Scale Filter                      */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951030 FB based on finvert                               */
/*    1.01 960508 FB fop for number of colours                      */
/*    1.02 961216 YK fixed GREY macro for levels != 256             */
/*    1.03 970905 GS added blend alpha option                       */
/*    1.04 000223 FB intlog round up                                */
/*    1.05 010824 FB rewritten based on invert                      */
/*    1.06 020206 FB set bits per channel to 8800 or 0800           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* grey()                                                           */
/*                                                                  */
/* Note, old module supported levels and would make into 8 bit      */
/* indexed with that many entries.  Thats still desirable, but      */
/* doesnt mesh as well with this simple in place grey               */
/* The user should probabily be thinking in terms of bits per       */
/* channel for IA8 vs IA4, rather than n level grey scale           */
/*                                                                  */
/* Old API                                                          */
/* grey(int greys, int blendalpha)                                  */
/* int greys;  number of grey scale values 2..256                   */
/* int blendalpha; if true alpha gets blended, else alpha=255       */
/*                                                                  */
/* Converts image to grey scale.  Note grey==intensity              */
/* Preserves alpha channel                                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fgrey.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static const int levels=256;
#define GREY8(c) ((c)->r*0.29900f + (c)->g*0.58700f +  (c)->b*0.11400f)
#define GREY(c) (unsigned char) ((GREY8(c)*levels + 128)/256)

static void grey(ARGB *dest, int greenbits, int n)
{
    int i;
    unsigned char intensity;

    for (i=0; i<n; ++i)
    {
//        if (greenbits)
            intensity = GREY(dest);
//        else
//            intensity = dest->r;    // if already intensity, copy red to other channels

        dest->r = dest->g = dest->b = intensity;
        ++dest;
    }
}

#undef GREY8
#undef GREY

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutgrey(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('G','R','E','Y');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.06");
        strcpy(about->namestr,      "grey");
        strcpy(about->longnamestr,  "Grey Scale ARGB");
        about->numoptions =         0;
    }
    return(about);
}

FPUBLIC int opengrey(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infogrey(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        grey(ginfo->colourtbl,ginfo->greenbits,256);

        // set bits per channel such that we preserve alpha, and call
        // red the luminance

        ginfo->greenbits = 0;
        ginfo->bluebits = 0;

    }
    return(ginfo);
}

FPUBLIC int filtergrey(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            grey((ARGB *)dest,ginfo->greenbits,width);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closegrey(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

