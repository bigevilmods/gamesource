// (c) 1997-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Doubled - Doubled and Dither to ARGB4444             */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 970612 FB based on invert                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* doubled(int abits,int rbits,int gbits,int bbits)                 */
/* abit - bits in alpha channel.                                    */
/* rbit - bits in red channel.                                      */
/* gbit - bits in green channel.                                    */
/* bbit - bits in blue channel.                                     */
/* all 4 default to 4 bits per channel                              */
/*                                                                  */
/* Doubles in horizontal but with dithering such that an image      */
/* can be bilinear filtered back to original size with effectively  */
/* an additional bit per channel.                                   */
/*                                                                  */
/* 8 bit requires 128 colours or less and will take each entry      */
/* and make 2 and then doubles the index values in the 8 bit        */
/* image.                                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fdoubled.h"
#include <string.h>
#include <stdlib.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static int DOUBLEtry(int i,int j,int c)
{
    int v;
    int d;
    int d1;
    int d2;

    v = (((int)i*255+8)/15+((int)j*255+8)/15)/2;
    d1 = abs((int)c-v);
    d2 = (int)abs(i-j);
    d = (int)(d1*1000.0f+d2);
    return(d);
}


static void DOUBLEfind(int c, unsigned char *p0, unsigned char *p1)
{
    int i;
    int j;
    int bi=0;
    int bj=0;
    int best=1000000;
    int diff;

    for (i=0; i<16; ++i)
    {
        for (j=0; j<16; ++j)
        {
            diff = DOUBLEtry(i,j,c);
            if (diff<best)
            {
                best = diff;
                bi = i;
                bj = j;
            }
        }
    }
    *p0 = (bi*255+8)/15;
    *p1 = (bj*255+8)/15;
}


static void doubled(ARGB *dest,ARGB *source,int abits,int rbits,int gbits,int bbits,int n)
{
    int i;

    for (i=0; i<n; i+=2)
    {
        DOUBLEfind(source->a,&dest[0].a,&dest[1].a);
        DOUBLEfind(source->r,&dest[0].r,&dest[1].r);
        DOUBLEfind(source->g,&dest[0].g,&dest[1].g);
        DOUBLEfind(source->b,&dest[0].b,&dest[1].b);
        source += 1;
        dest += 2;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutdoubled(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('D','U','B','D');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Doubled");
        strcpy(about->longnamestr,  "Doubled ARGB");
        about->numoptions =         4;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           8;
        about->option[0].def.i =           4;
        strcpy(about->option[0].name,      "abits");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           8;
        about->option[1].def.i =           4;
        strcpy(about->option[1].name,      "rbits");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           8;
        about->option[2].def.i =           4;
        strcpy(about->option[2].name,      "gbits");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           8;
        about->option[3].def.i =           4;
        strcpy(about->option[3].name,      "bbits");
        strcpy(about->option[3].enumlist,  "");
    }
    return(about);
}

FPUBLIC int opendoubled(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infodoubled(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    unsigned int a=4;
    unsigned int r=4;
    unsigned int g=4;
    unsigned int b=4;
    if (fx->option)
    {
        if (fx->numoptions>=1) a = fx->option[0].i;
        if (fx->numoptions>=2) r = fx->option[1].i;
        if (fx->numoptions>=3) g = fx->option[2].i;
        if (fx->numoptions>=4) b = fx->option[3].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->numcolours *= 2;
        ginfo->width *= 2;
        ginfo->alphabits = a;
        ginfo->redbits   = r;
        ginfo->greenbits = g;
        ginfo->bluebits  = b;

        doubled(ginfo->colourtbl,fx->source[0].ginfo->colourtbl,a,r,g,b,256);
    }
    return(ginfo);
}

FPUBLIC int filterdoubled(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    unsigned int a=4;
    unsigned int r=4;
    unsigned int g=4;
    unsigned int b=4;
    if (fx->option)
    {
        if (fx->numoptions>=1) a = fx->option[0].i;
        if (fx->numoptions>=2) r = fx->option[1].i;
        if (fx->numoptions>=3) g = fx->option[2].i;
        if (fx->numoptions>=4) b = fx->option[3].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        if (ginfo->bpp==32)
            doubled((ARGB *)dest,(ARGB *)source,a,r,g,b,width);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closedoubled(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

