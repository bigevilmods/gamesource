// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  Screen - Screen Image Colours                   */
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
/* screen(int orientation)                                          */
/*                                                                  */
/*   0   X -                                                        */
/*       - X                                                        */
/*                                                                  */
/*   1   - X                                                        */
/*       X -                                                        */
/*                                                                  */
/* Screens 1/2 pixels to transparent                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fscreen.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void screenapply(unsigned char *dest,unsigned int orientation,int n,int row, int bpp, unsigned int colour)
{
    int i;

    for (i=0; i<n; ++i)
    {
        if ((i^row)&1)
        {
            if (bpp<=8)
                *dest = (unsigned char)colour;
            else
                *(unsigned int *)dest = colour;
        }
        if (bpp<=8)
            dest += 1;
        else
            dest += 4;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutscreen(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','C','R','N');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Screen");
        strcpy(about->longnamestr,  "Screen Mask");
        about->numoptions =         2;

        about->option[0].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0xffffffff;
        strcpy(about->option[0].name,      "screen colour");
        strcpy(about->option[0].enumlist,  "");


        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           1;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "orientation");
        strcpy(about->option[1].enumlist,  "");

    }
    return(about);
}

FPUBLIC int openscreen(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoscreen(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    unsigned int colour=0;
    unsigned int orientation=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) colour      = fx->option[0].i;
        if (fx->numoptions>=2) orientation = fx->option[1].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterscreen(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    unsigned int colour=0;
    unsigned int orientation=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) colour      = fx->option[0].i;
        if (fx->numoptions>=2) orientation = fx->option[1].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        screenapply((unsigned char *)dest,orientation,width,i,ginfo->bpp,colour);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closescreen(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

