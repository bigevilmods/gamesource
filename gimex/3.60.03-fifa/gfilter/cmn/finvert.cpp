// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  Invert - Invert Image Colours                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on alpha copy                            */
/*    1.01 021201 FB default mask does not include alpha            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* invert(ARGB mask)                                                */
/*                                                                  */
/* Inverts ARGB values (using XOR)                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "finvert.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void invert(ARGB *dest,unsigned int mask,int n)
{
    int i;
    int a=(mask>>24)&255;
    int r=(mask>>16)&255;
    int g=(mask>>8)&255;
    int b=(mask>>0)&255;

    for (i=0; i<n; ++i)
    {
        dest->a = (unsigned char)(a ^ dest->a);
        dest->r = (unsigned char)(r ^ dest->r);
        dest->g = (unsigned char)(g ^ dest->g);
        dest->b = (unsigned char)(b ^ dest->b);
        ++dest;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutinvert(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','I','N','V');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "invert");
        strcpy(about->longnamestr,  "Invert ARGB");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0x00ffffff;
        strcpy(about->option[0].name,      "mask");
        strcpy(about->option[0].enumlist,  "");

    }
    return(about);
}

FPUBLIC int openinvert(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoinvert(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    unsigned int mask=0xffffffff;
    if (fx->option)
    {
        if (fx->numoptions>=1) mask = fx->option[0].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        invert(ginfo->colourtbl,mask,256);
    }
    return(ginfo);
}

FPUBLIC int filterinvert(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    unsigned int mask=0xffffffff;
    if (fx->option)
    {
        if (fx->numoptions>=1) mask = fx->option[0].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            invert((ARGB *)dest,mask,width);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closeinvert(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

