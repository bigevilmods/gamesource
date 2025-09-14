// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*         Shrink - Shrink To Fit (Crop to Minimum Rectangle)       */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951109 FB based on crop. shrinks to fit latenight style  */
/*    1.01 951110 FB use pixel in upper left corner for shrink      */
/*    1.02 001211 FB option to use upper left (default to off)      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* shrink(int method)                                               */
/* int method  1=transparent only, 2=upper left pixel, 0/3=both     */
/*                                                                  */
/* Shrink an image to minimum rectangle.                            */
/* if method=1 removes transparent pixels around the image.         */
/* if method=2, uses colour of pixel in upper left corner and       */
/* removes that colour around edges.                                */
/* if method=3, uses colour of pixel in upper left corner and       */
/* removes that colour around edges, as well as transparent pixels. */
/*                                                                  */
/* Consider adding colour and mask                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fshrink.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static unsigned int getpixel(GBITMAP *source, int x, int y)
{
    GINFO *ginfo=source->ginfo;
    ARGB *p;
    unsigned char *ptr;

    ptr = (unsigned char *) source->image + y*source->rowbytes + x*ginfo->bpp/8;
    if (ginfo->bpp==8)
    {
        p = &ginfo->colourtbl[*ptr];
    }
    else
    {
        p = (ARGB *) ptr;
    }
    return(GMAKEID(p->a,p->r,p->g,p->b));
}

static int istransparent(GBITMAP *source, int x, int y,int FrameColour, int shrinkmethod)
{
    unsigned int p = getpixel(source,x,y);
    int ok=0;
    if (shrinkmethod&1)
        ok |= ((p>>24)==0);
    if (shrinkmethod&2)
        ok |= p==(unsigned int)FrameColour;
    return(ok);
}

static int CheckColumn(GBITMAP *source, int col, int FrameY,int FrameH,int FrameColour, int shrinkmethod)
{
    int y;
    int ok=1;

    for (y=FrameY; y<FrameY+FrameH && ok; ++y)
    {
        ok = istransparent(source,col,y,FrameColour,shrinkmethod);
    }

    return(ok);
}

static int CheckRow(GBITMAP *source, int row, int FrameX,int FrameW,int FrameColour, int shrinkmethod)
{
    int x;
    int ok=1;

    for (x=FrameX; x<FrameX+FrameW && ok; ++x)
    {
        ok = istransparent(source,x,row,FrameColour,shrinkmethod);
    }

    return(ok);
}

static void shrinktofit(int *rect,GBITMAP *source,int shrinkmethod)
{
    int i;
    int FrameX;
    int FrameY;
    int FrameW;
    int FrameH;
    int FrameColour;

    FrameX = 0;
    FrameY = 0;
    FrameW = source->ginfo->width;
    FrameH = source->ginfo->height;

    if (!shrinkmethod)
        shrinkmethod = 1;
    FrameColour = 0;
    if (shrinkmethod&2)
        FrameColour = getpixel(source,0,0);

    for (i = FrameX; i < FrameX+FrameW; ++i)
    {
        if (!CheckColumn(source, i, FrameY,FrameH,FrameColour, shrinkmethod))
        {
            FrameW -= i - FrameX;
            FrameX = i;
            break;
        }
    }

    for (i = FrameX+FrameW-1; i >= FrameX; --i)
    {
        if (!CheckColumn(source, i, FrameY,FrameH,FrameColour, shrinkmethod))
        {
            ++i;
            FrameW = i - FrameX;
            break;
        }
    }


    for (i = FrameY; i < FrameY+FrameH; ++i)
    {
        if (!CheckRow(source, i, FrameX,FrameW,FrameColour, shrinkmethod))
        {
            FrameH -= i - FrameY;
            FrameY = i;
            break;
        }
    }

    for (i = FrameY+FrameH-1; i >= FrameY; --i)
    {
        if (!CheckRow(source, i, FrameX,FrameW,FrameColour, shrinkmethod))
        {
            FrameH = i - FrameY + 1;
            break;
        }
    }

    rect[0] = FrameX;   // return the rectangle
    rect[1] = FrameY;
    rect[2] = FrameW;
    rect[3] = FrameH;
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutshrink(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','H','N','K');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.02");
        strcpy(about->namestr,      "Shrink");
        strcpy(about->longnamestr,  "Shrink To Fit");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "method");
        strcpy(about->option[0].enumlist,  "transparent,upperleft");

    }
    return(about);
}

FPUBLIC int openshrink(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoshrink(FINSTANCE *fx, int framenum)
{
    int i;
    int x;
    int y;
    GINFO *ginfo;
    int shrinkrect[4];
    int shrinkx;
    int shrinky;
    int shrinkw;
    int shrinkh;
    int shrinkmethod=3;

    if (fx->option)
    {
        if (fx->numoptions>=1) shrinkmethod = fx->option[0].i;
    }

    shrinktofit(shrinkrect,fx->source,shrinkmethod);
    shrinkx = shrinkrect[0];
    shrinky = shrinkrect[1];
    shrinkw = shrinkrect[2];
    shrinkh = shrinkrect[3];

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        x = shrinkx;
        y = shrinky;
        ginfo->width  = shrinkw;
        ginfo->height = shrinkh;

        ginfo->defaultx += x;
        ginfo->defaulty += y;
        ginfo->centerx  -= x;
        ginfo->centery  -= y;
        for (i=0; i<ginfo->numhotspots; ++i)
        {
            ginfo->hotspottbl[i][0] -= x;
            ginfo->hotspottbl[i][1] -= y;
        }
    }
    return(ginfo);
}

FPUBLIC int filtershrink(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int sx;
    int sy;
    char *s;
    char *d;
    int  bpp = ginfo->bpp;
    int shrinkrect[4];
    int shrinkx;
    int shrinky;
    int shrinkw;
    int shrinkh;
    int shrinkmethod=3;

    if (fx->option)
    {
        if (fx->numoptions>=1) shrinkmethod = fx->option[0].i;
    }

    shrinktofit(shrinkrect,fx->source,shrinkmethod);
    shrinkx = shrinkrect[0];
    shrinky = shrinkrect[1];
    shrinkw = shrinkrect[2];
    shrinkh = shrinkrect[3];

    for (y=0; y<ginfo->height; ++y)
    {
        d = dest;
        for (x=0; x<ginfo->width; ++x)
        {
            sx = shrinkx+x;
            sy = shrinky+y;

            if (sx>=0 && sx<fx->source[0].ginfo->width
             && sy>=0 && sy<fx->source[0].ginfo->height)
            {
                s = (char *)fx->source[0].image+sy*fx->source[0].rowbytes+sx*bpp/8;
                if (bpp==8)
                    *d++ = *s;
                else
                {
                    *(int *)d = *(int *)s;
                    d += 4;
                }
            }
        }
        dest += rowbytes;
    }
    return(1);
}

FPUBLIC int closeshrink(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

