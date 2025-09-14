// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*     Grb - grrrrbbb Palettize with Green and Red+Blue Pixels      */
/*                                                                  */
/*                        by Frank Barchard, EAC                    */
/*                                                                  */
/*                    Filter Module - Apr 07, 1999                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990409 FB initial version based on fflip                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* grb(int direction)                                               */
/* int direction;  0=none,1=horizontal,2=vertical,3=both            */
/*                                                                  */
/* Grb palettize an image horizontally and/or vertically.           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fgrb.h"
#include <string.h>

#define GRB_HORIZONTAL     1
#define GRB_VERTICAL       2

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutgrb(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('G','U','R','B');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "F Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Grb");
        strcpy(about->longnamestr,  "Grb");
        about->numoptions =         1;

        about->numoptions =                1;
        about->option[0].type =            FOP_INT | FOP_ENUM;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           2;
        strcpy(about->option[0].name,      "Direction");
        strcpy(about->option[0].enumlist,  "None,Horizontal,Vertical,Both");
    }
    return(about);
}

FPUBLIC int opengrb(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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


FPUBLIC GINFO *infogrb(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int i;
    int grb=2;

    if (fx->option)
    {
        if (fx->numoptions>=1) grb = fx->option[0].i;
    }

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        if (grb&GRB_HORIZONTAL)
        {
            ginfo->width *= 2;
            ginfo->centerx *= 2;
            for (i=0; i<ginfo->numhotspots; ++i)
                ginfo->hotspottbl[i][0] *= 2;
        }
        if (grb&GRB_VERTICAL)
        {
            ginfo->height *= 2;
            ginfo->centery *= 2;
            for (i=0; i<ginfo->numhotspots; ++i)
                ginfo->hotspottbl[i][1] *= 2;
        }
        ginfo->bpp = ginfo->originalbpp = 8;
        for (i=0; i<256; ++i)
        {
            if (i&128)
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = 0;
                ginfo->colourtbl[i].g = ((i&127)*255+64)/127;
                ginfo->colourtbl[i].b = 0;
            }
            else
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = (((i>>3)&15)*255+8)/15;
                ginfo->colourtbl[i].g = 0;
                ginfo->colourtbl[i].b = ((i&7)*255+4)/7;
            }
        }
    }
    return ginfo;
}

FPUBLIC int filtergrb(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x,y,sx,sy;
    char *source = (char *)fx->source[0].image;
    GINFO *sourceginfo = fx->source[0].ginfo;
    int bpp = sourceginfo->bpp;
    int grb=2;

    if (fx->option)
    {
        if (fx->numoptions>=1) grb = fx->option[0].i;
    }

    for (y=0; y<ginfo->height; ++y)
    {
        ARGB *p;
        unsigned char *d = (unsigned char *)dest;
        sy = y;
        if (grb&GRB_VERTICAL)
            sy /= 2;

        source = (char *)(fx->source[0].image) + fx->source[0].rowbytes*sy;
        for (x=0; x<ginfo->width; ++x)
        {
            sx = x;
            if (grb&GRB_HORIZONTAL)
                sx /= 2;
            if (bpp==8)
                p = &sourceginfo->colourtbl[*((unsigned char *)source+sx)];
            else
                p = ((ARGB *)source)+sx;
            if ((x+y)&1)
                *d = ((p->r>>4)<<3)+(p->b>>5);
            else
                *d = (p->g>>1)+128;
            ++d;
        }
        dest   += rowbytes;
    }
    return(1);
}


FPUBLIC int closegrb(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}


