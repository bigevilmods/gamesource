// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Scale - A Fast Anamorphic Image Scale              */
/*                                                                  */
/*                by Frank Barchard & Gerry Shaw, EAC               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960220 GS based on ffill                                 */
/*    1.01 000329 FB position scaled                                */
/*    1.02 010118 ID scale each layer based on first page type      */
/*    1.03 010315 FB clamp to 1 by 1                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* scale(int width, int height)                                     */
/*                                                                  */
/* Scales given image to new width and new height                   */
/*                                                                  */
/* Schumacher, D. 1992. Graphics Gems II. Boston, MA: Academic      */
/* Press Professional, p 78 (2.4)                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fscale.h"
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static unsigned int SCALEreadpixel(GBITMAP* src, int x, int y)
{
    int   bpp      = src->ginfo->bpp;
    int   rowbytes = src->rowbytes;
    char *base     = (char *) src->image;

    return ggetm(base + y*rowbytes + x*bpp/8, bpp/8);
}

static void SCALEwritepixel(GBITMAP* dst, int x, int y, unsigned int value)
{
    int   bpp      = dst->ginfo->bpp;
    int   rowbytes = dst->rowbytes;
    char *base     = (char *) dst->image;

    gputm((base + y*rowbytes + x*bpp/8), value, bpp/8);
}

static void scale(GBITMAP* src, GBITMAP* dst, int dstwidth, int dstheight)
{
    int srcwidth  = src->ginfo->width;
    int srcheight = src->ginfo->height;

    int sx,sy,dx,dy;    /* source and destination coordinates */

    dy = 0;
    while (dy < dstheight)
    {
        sy = (dy * srcheight) / dstheight;
        dx = 0;
        while (dx < dstwidth)
        {
            sx = (dx * srcwidth) / dstwidth;
            SCALEwritepixel(dst, dx,dy, SCALEreadpixel(src, sx,sy));
            dx++;
        }
        dy++;
    }
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

// TODO: move this #define to gfilter.h
#define FILTEROPTION(INDEX, TYPE, MINVAL, MAXVAL, DEFVAL, OPTNAMESTR, ENUMLISTSTR) \
    about->option[INDEX].type =            TYPE; \
    about->option[INDEX].min.i =           MINVAL; \
    about->option[INDEX].max.i =           MAXVAL; \
    about->option[INDEX].def.i =           DEFVAL; \
    strcpy(about->option[INDEX].name,      OPTNAMESTR); \
    strcpy(about->option[INDEX].enumlist,  ENUMLISTSTR)


FPUBLIC FABOUT* aboutscale(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','C','A','L');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.03" );
        strcpy(about->namestr,      "Scale" );
        strcpy(about->longnamestr,  "Scale Image" );

        about->numoptions =         2;
        FILTEROPTION(0, FOP_INT, 0,0xffffffff,1, "Width", "");
        FILTEROPTION(1, FOP_INT, 0,0xffffffff,1, "Height", "");
    }
    return about;
}

FPUBLIC int openscale(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx = 0;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
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
    return ok;
}

#define S_PSDNUMOPTIONS 5

FPUBLIC GINFO* infoscale(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo;

    ginfo = (GINFO*) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        /* copy ginfo to destination */
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->framenum = framenum;
        ginfo->width    = fx->option[0].i;
        ginfo->height   = fx->option[1].i;
        if ((fx->numoptions == S_PSDNUMOPTIONS) && (fx->option[4].i)) // only if it's needed do we handle layers.
        {
            ginfo->width  = (int)((float) ginfo->width  * (float)((float)fx->source[0].ginfo->width/(float)fx->option[2].i));
            ginfo->height = (int)((float) ginfo->height * (float)((float)fx->source[0].ginfo->height/(float)fx->option[3].i));
        }
        if (ginfo->width<1)  ginfo->width = 1;
        if (ginfo->height<1) ginfo->height = 1;

        /* scale center point and hotspots */
        {
            /* calculate zoom ratio's in both directions */
            double widthZoom  = (double)ginfo->width  / (double)fx->source[0].ginfo->width;
            double heightZoom = (double)ginfo->height / (double)fx->source[0].ginfo->height;

            /* scale hotspots */
            int i;
            for (i=0; i<ginfo->numhotspots; ++i)
            {
                ginfo->hotspottbl[i][0] = (int)(ginfo->hotspottbl[i][0] * widthZoom);
                ginfo->hotspottbl[i][1] = (int)(ginfo->hotspottbl[i][1] * heightZoom);
            }

            /* scale center point */
            ginfo->centerx = (int)(ginfo->centerx * widthZoom);
            ginfo->centery = (int)(ginfo->centery * heightZoom);
            /* position */
            ginfo->defaultx = (int)(ginfo->defaultx * widthZoom);
            ginfo->defaulty = (int)(ginfo->defaulty * heightZoom);
        }
    }
    return ginfo;
}

FPUBLIC int filterscale(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    int width  = 0;
    int height = 0;

    if (fx->option)
    {
        if (fx->numoptions>=1) width  = fx->option[0].i;
        if (fx->numoptions>=2) height = fx->option[1].i;
    }
    if ((fx->numoptions == S_PSDNUMOPTIONS) && (fx->option[4].i)) // only if it's needed do we handle layers.
    {
        width  = (int)((float) width *  ((float)fx->source[0].ginfo->width/(float)fx->option[2].i));
        height = (int)((float) height * ((float)fx->source[0].ginfo->height/(float)fx->option[3].i));
    }
    /* scale image */
    {
        GBITMAP Destination;
        Destination.ginfo    = ginfo;
        Destination.image    = dst;
        Destination.rowbytes = rowbytes;

        scale(&(fx->source[0]), &Destination, width,height);
    }

    return 1;
}

FPUBLIC int closescale(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
