// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 XColour - eXchange Colours Filter                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on invert                                */
/*    1.01 960409 FB 4 options                                      */
/*    1.02 970708 FB fop to NOT search                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* xcolour(ARGB search,ARGB replace,ARGB smask,ARGB dmask,int not)  */
/*                                                                  */
/* Searches for an ARGB colour and replaces with another.           */
/* if ((pixel&smask==search)^not)                                   */
/*     pixel=(pixel&~dmask)|(replace&dmask)                         */
/*                                                                  */
/* Future consideration: tolerance                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fxcolour.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static unsigned int colourtoargblong(unsigned int search)
{
    ARGB sargb;
    unsigned int s;

    sargb.a = (unsigned char) (search>>24);
    sargb.r = (unsigned char) (search>>16);
    sargb.g = (unsigned char) (search>> 8);
    sargb.b = (unsigned char) (search>> 0);
    s = *(unsigned int *)&sargb;
    return(s);
}

static void xcolour(ARGB *dest,int n,unsigned int search,unsigned int replace,
                    unsigned int searchmask,unsigned int replacemask, int snot)
{
    int i;
    unsigned int *d;
    unsigned int s;
    unsigned int r;
    unsigned int sm;
    unsigned int rm;

    s = colourtoargblong(search);
    r = colourtoargblong(replace);
    sm = colourtoargblong(searchmask);
    rm = colourtoargblong(replacemask);

    d = (unsigned int *)dest;

    for (i=0; i<n; ++i)
    {
        if (((*d&sm)==s)^snot)
            *d = (*d&~rm)|(r&rm);

        ++d;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutxcolour(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('X','C','O','L');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.02");
        strcpy(about->namestr,      "Xcolour");
        strcpy(about->longnamestr,  "Xcolour Image");
        about->numoptions =         5;

        about->option[0].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Search");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Replace");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[2].min.i =           0;
        about->option[2].max.i =           0xffffffff;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "SearchMask");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[3].min.i =           0;
        about->option[3].max.i =           0xffffffff;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "ReplaceMask");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           0;
        about->option[4].max.i =           1;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "SearchNot");
        strcpy(about->option[4].enumlist,  "");


    }
    return about;
}

FPUBLIC int openxcolour(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoxcolour(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    unsigned int search=0;
    unsigned int replace=0;
    unsigned int searchmask=0xffffffff;
    unsigned int replacemask=0xffffffff;
    int searchnot=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) search = fx->option[0].i;
        if (fx->numoptions>=2) replace = fx->option[1].i;
        if (fx->numoptions>=3) searchmask = fx->option[2].i;
        if (fx->numoptions>=4) replacemask = fx->option[3].i;
        if (fx->numoptions>=5) searchnot = fx->option[4].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        xcolour(ginfo->colourtbl,256,search,replace,searchmask,replacemask,searchnot);
    }
    return(ginfo);
}

FPUBLIC int filterxcolour(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;

    unsigned int search=0;
    unsigned int replace=0;
    unsigned int searchmask=0xffffffff;
    unsigned int replacemask=0xffffffff;
    int searchnot=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) search = fx->option[0].i;
        if (fx->numoptions>=2) replace = fx->option[1].i;
        if (fx->numoptions>=3) searchmask = fx->option[2].i;
        if (fx->numoptions>=4) replacemask = fx->option[3].i;
        if (fx->numoptions>=5) searchnot = fx->option[4].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            xcolour((ARGB *)dest,width,search,replace,searchmask,replacemask,searchnot);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closexcolour(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

