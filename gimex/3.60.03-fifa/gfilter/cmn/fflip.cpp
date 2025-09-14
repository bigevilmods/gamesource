// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*            Flip - Horizontal/Vertical Image Flip Filter          */
/*                                                                  */
/*                         by Gerry Shaw, EAC                       */
/*                                                                  */
/*                    Filter Module - Oct 25, 1995                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950428 GS initial version                                */
/*    1.01 950510 GS updated for filter 1.00                        */
/*    1.02 950516 GS vertical flip supported                        */
/*    1.03 951025 FB flip center point/hot spots                    */
/*    1.04 951122 FB fop parsing simplified                         */
/*    1.05 010118 ID fixed multilayer page type defaultx & y calc.  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* flip(int direction)                                              */
/* int direction;  0=none,1=horizontal,2=vertical,3=both            */
/*                                                                  */
/* Flip an image horizontally and/or vertically.                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fflip.h"
#include <string.h>

#define FLIP_HORIZONTAL     1
#define FLIP_VERTICAL       2

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void fliphorizontal8(char *source, char *dest, int width, int height, int rowbytes)
{
    int row;
    int col;

    for (row = 0; row < height; row++)
    {
        char *dstRow = dest + row*rowbytes;
        char *srcRow = source + row*rowbytes;

        for (col = 0; col < width; col++)
            *(dstRow + col) = *(srcRow + width-1 - col);
    }
}

static void fliphorizontal32(char *source, char *dest, int width, int height, int rowbytes)
{
    int row;
    int col;

    for (row = 0; row < height; row++)
    {
        int *dstRow = (int*)(dest + row*rowbytes);
        int *srcRow = (int*)(source + row*rowbytes);

        for (col = 0; col < width; col++)
            *(dstRow + col) = *(srcRow + width-1 - col);
    }
}

static void fliphorizontal(char *source, char *dest, int width, int height, int rowbytes, int bpp)
{
    if (bpp == 8)
        fliphorizontal8(source, dest, width, height, rowbytes);
    else
        fliphorizontal32(source, dest, width, height, rowbytes);
}

static void flipvertical(char *source, char *dest, int height, int rowbytes)
{
    int row;

    for (row = 0; row < height; row++)
    {
        char *dstRow = (char*)dest + row*rowbytes;
        char *srcRow = (char*)source + (height - row - 1)*rowbytes;
        memcpy(dstRow, srcRow, rowbytes);
    }

}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutflip(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','L','I','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "G Shaw/F Barchard");
        strcpy(about->versionstr,   "1.04");
        strcpy(about->namestr,      "Flip");
        strcpy(about->longnamestr,  "Flip");
        about->numoptions =         1;

        about->numoptions =                1;
        about->option[0].type =            FOP_INT | FOP_ENUM;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "Direction");
        strcpy(about->option[0].enumlist,  "None,Horizontal,Vertical,Both");
    }
    return(about);
}

FPUBLIC int openflip(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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


FPUBLIC GINFO *infoflip(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int i;
    int flip=1;

    if (fx->option)
    {
        if (fx->numoptions>=1) flip = fx->option[0].i;
    }

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        if (flip&FLIP_HORIZONTAL) ginfo->centerx = ginfo->width-ginfo->centerx;
        if (flip&FLIP_VERTICAL)   ginfo->centery = ginfo->height-ginfo->centery;
        for (i=0; i<ginfo->numhotspots; ++i)
        {
            if (flip&FLIP_HORIZONTAL) ginfo->hotspottbl[i][0] = ginfo->width-ginfo->hotspottbl[i][0];
            if (flip&FLIP_VERTICAL)   ginfo->hotspottbl[i][1] = ginfo->height-ginfo->hotspottbl[i][1];
        }
        if(fx->numoptions == 4 && fx->option[3].i)
        {
            if(flip&FLIP_HORIZONTAL)
            {
                        ginfo->defaultx = fx->option[1].i -  (ginfo->width + ginfo->defaultx);
                        //psdFrameHeight - (ginfo->height + iy);
            }
            if(flip&FLIP_VERTICAL)
            {
                        //psdFrameWidth -  (ginfo->width + ix);
                        ginfo->defaulty = fx->option[2].i - (ginfo->height + ginfo->defaulty);
            }
        }


    }

    return ginfo;
}

FPUBLIC int filterflip(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=1;
    char *source=(char *)fx->source[0].image;
    int bpp    = fx->source[0].ginfo->bpp;
    int width  = fx->source[0].ginfo->width;
    int height = fx->source[0].ginfo->height;
    int flip=1;

    if (fx->option)
    {
        if (fx->numoptions>=1) flip = fx->option[0].i;
    }

    switch (flip)
    {
        case 0:
            break;

        case FLIP_HORIZONTAL:
            fliphorizontal(source, dest, width, height, rowbytes, bpp);
            break;

        case FLIP_VERTICAL:
            flipvertical(source, dest, height, rowbytes);
            break;

        case FLIP_HORIZONTAL+FLIP_VERTICAL:
        {
            char *tmp = (char *)galloc(rowbytes * height);
            if (tmp)
            {
                fliphorizontal(source, tmp, width, height, rowbytes, bpp);
                flipvertical(tmp, dest, height, rowbytes);
                gfree(tmp);
            }
            else
                ok = 0;
        }
    }

    return(ok);
}

FPUBLIC int closeflip(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}


