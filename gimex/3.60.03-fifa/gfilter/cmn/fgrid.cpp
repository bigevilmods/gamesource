// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                        Grid - Draw Grid                          */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990722 FB draw a grid over an image                      */
/*    1.01 010118 ID draw only on lowest layer for multilayered img.*/
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* grid(int x, int y, int width, int height, int colour)            */
/*                                                                  */
/* Draw a grid of lines starting at x and incrementing by width     */
/* and starting at y and incrementing by height, in colour          */
/* Can overwrite source.                                            */
/* Colour is 8 or 32 bit.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fgrid.h"
#include <string.h>

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutgrid(void)
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
        strcpy(about->versionstr,   "1.00" );
        strcpy(about->namestr,      "Grid" );
        strcpy(about->longnamestr,  "Draw Grid" );
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

FPUBLIC int opengrid(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infogrid(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filtergrid(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    char *s;
    char *d;
    char *source= (char *)fx->source[0].image;
    int  bpp    = ginfo->bpp;
    int  bpl    = ginfo->width*ginfo->bpp/8;

    int gridx=0;
    int gridy=0;
    int gridw=1;
    int gridh=1;
    unsigned int gridc=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) gridx = fx->option[0].i;
        if (fx->numoptions>=2) gridy = fx->option[1].i;
        if (fx->numoptions>=3) gridw = fx->option[2].i;
        if (fx->numoptions>=4) gridh = fx->option[3].i;
        if (fx->numoptions>=5) gridc = fx->option[4].i;
    }
    if (!gridw)
        gridw = ginfo->width;
    if (!gridh)
        gridh = ginfo->height;

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
//            printf("WARNING: Grid will only be drawn on lowest layer for page format files\n");
    }
    /* draw horizontal lines */

    for (y=gridy; y<ginfo->height; y += gridh)
    {
        d = dest+y*rowbytes;
        for (x=0; x<ginfo->width; ++x)
        {
            if (bpp==8)
            {
                *d++ = (char)gridc;
            }
            else
            {
                ((ARGB *)d)->a = (unsigned char)(gridc>>24);
                ((ARGB *)d)->r = (unsigned char)(gridc>>16);
                ((ARGB *)d)->g = (unsigned char)(gridc>> 8);
                ((ARGB *)d)->b = (unsigned char)(gridc>> 0);
                d += 4;
            }
        }
    }

    /* draw vertical lines */

    for (x=gridx; x<ginfo->width; x += gridw)
    {
        d = dest+x*bpp/8;
        for (y=0; y<ginfo->height; ++y)
        {
            if (bpp==8)
            {
                *d = (char)gridc;
            }
            else
            {
                ((ARGB *)d)->a = (unsigned char)(gridc>>24);
                ((ARGB *)d)->r = (unsigned char)(gridc>>16);
                ((ARGB *)d)->g = (unsigned char)(gridc>> 8);
                ((ARGB *)d)->b = (unsigned char)(gridc>> 0);
            }
            d += rowbytes;
        }
    }

    return(1);
}

FPUBLIC int closegrid(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

