// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                      Rect - Draw Rectangle                       */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951103 FB based on frot                                  */
/*    1.01 960109 FB removed rectanglecoord unused fn               */
/*    1.02 960209 GS added canoverwritesource optimization          */
/*    1.03 010118 ID only on bottom layer for page type.            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* rect(int x, int y, int width, int height, int colour)            */
/*                                                                  */
/* (same prototype as eac lib)                                      */
/* Draw a clipped rectangle of solid colour into image.             */
/* Can overwrite source.                                            */
/* Colour is 8 or 32 bit.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "frect.h"
#include <string.h>

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutrect(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('R','E','C','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->canoverwritesource = 1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.02" );
        strcpy(about->namestr,      "Rect" );
        strcpy(about->longnamestr,  "Rect Image" );
        about->numoptions =         5;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "X");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Y");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           0xffffffff;
        about->option[2].def.i =           1;
        strcpy(about->option[2].name,      "Width");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           0xffffffff;
        about->option[3].def.i =           1;
        strcpy(about->option[3].name,      "Height");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "Colour");
        strcpy(about->option[4].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openrect(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *inforect(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterrect(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    char *s;
    char *d;
    char *source= (char *) fx->source[0].image;
    int  bpp    = ginfo->bpp;
    int  bpl    = ginfo->width*ginfo->bpp/8;

    int rectx=0;
    int recty=0;
    int rectw=1;
    int recth=1;
    unsigned int rectc=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) rectx = fx->option[0].i;
        if (fx->numoptions>=2) recty = fx->option[1].i;
        if (fx->numoptions>=3) rectw = fx->option[2].i;
        if (fx->numoptions>=4) recth = fx->option[3].i;
        if (fx->numoptions>=5) rectc = fx->option[4].i;
    }

    /* copy over bitmap, unless overwriting source */
    if (dest != source)
    {
        d = dest;
        s = source;
        for (y=0; y<ginfo->height; ++y)
        {
            memcpy(d,s,bpl);
            s += fx->source[0].rowbytes;
            d += rowbytes;
        }
    }
    if((fx->numoptions == 6) && (fx->option[5].i))  // if this is a pages type the only draw rect.
    {                                               // on lowest layer.
        if(ginfo->framenum != 0)
            return 1;
//        else
//            printf("WARNING: Rectangle will only be drawn on lowest layer for page format files\n");
    }

    /* draw rectangle into it */

    for (y=recty; y<recty+recth; ++y)
    {
        for (x=rectx; x<rectx+rectw; ++x)
        {
            d = dest+y*rowbytes+x*bpp/8;

            if (x>=0 && x<ginfo->width
             && y>=0 && y<ginfo->height)
            {
                if (bpp==8)
                    *d = (char)rectc;
                else
                {
                    ((ARGB *)d)->a = (unsigned char)(rectc>>24);
                    ((ARGB *)d)->r = (unsigned char)(rectc>>16);
                    ((ARGB *)d)->g = (unsigned char)(rectc>> 8);
                    ((ARGB *)d)->b = (unsigned char)(rectc>> 0);
                }
            }
        }
    }
    return(1);
}

FPUBLIC int closerect(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

