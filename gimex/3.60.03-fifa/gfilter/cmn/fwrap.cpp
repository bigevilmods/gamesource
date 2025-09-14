// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Wrap - Pan Image, Wrapping at Edges                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951101 FB based on xcolour and ln wrap concept           */
/*    1.01 951204 FB 32 bit wrap fixed and tested.                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* wrap(int xdisplace, int ydisplay)                                */
/*                                                                  */
/* Pan image by specified pixels with X and Y displacements,        */
/* wrapping around at the edges.                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fwrap.h"
#include <string.h>

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutwrap(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('W','R','A','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "Wrap");
        strcpy(about->longnamestr,  "Wrap Image");
        about->numoptions =         2;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0x80000000;
        about->option[0].max.i =           0x7fffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "X");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0x80000000;
        about->option[1].max.i =           0x7fffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Y");
        strcpy(about->option[1].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openwrap(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infowrap(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int  xoff=0;
    int  yoff=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) xoff = fx->option[0].i;
        if (fx->numoptions>=2) yoff = fx->option[1].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterwrap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int sx;
    int sy;
    char *s;
    char *d;
    int  width  = ginfo->width;
    int  height = ginfo->height;
    int  bpp    = ginfo->bpp;
    int  xoff=0;
    int  yoff=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) xoff = fx->option[0].i;
        if (fx->numoptions>=2) yoff = fx->option[1].i;
    }


    for (y=0; y<height; ++y)
    {
        d = dest;
        for (x=0; x<width; ++x)
        {
            sx = (((int)(x-xoff)%width)+width*2)%width;
            sy = (((int)(y-yoff)%height)+height*2)%height;

            s = (char *)fx->source[0].image+sy*fx->source[0].rowbytes+sx*bpp/8;

            if (bpp==8)
                *d++ = *s;
            else
            {
                *(int *)d = *(int *)s;
                d += 4;
            }
        }
        dest += rowbytes;
    }
    return(1);
}

FPUBLIC int closewrap(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

