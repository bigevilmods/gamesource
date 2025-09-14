// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   Acenter - Auto Center Point                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960509 FB based on shrink to fit                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* acenter(void)                                                    */
/*                                                                  */
/* Acenter sets the center point for an image, by scanning around   */
/* the edge for 2 pixels that triangulate the center point.  The    */
/* 2 pixels are replaced with the colour from the upper left hand   */
/* corner.                                                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "facenter.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static int ACENgetpixel(GBITMAP *source, int x, int y)
{
    GINFO *ginfo=source->ginfo;
    int p;
    unsigned char *ptr;

    ptr = (unsigned char *) source->image + y*source->rowbytes + x*ginfo->bpp/8;
    if (ginfo->bpp==8)
    {
        p = (int)*ptr;
    }
    else
    {
        p = *(int *)ptr;
    }
    return(p);
}

static void acenter(int *rect,GBITMAP *source)
{
    int x;
    int y;
    int FrameX;
    int FrameY;
    int FrameU;
    int FrameV;
    int FrameColour;
    GINFO *ginfo=source->ginfo;

    FrameX = 0;
    FrameY = 0;
    FrameU = 0;
    FrameV = 0;
    FrameColour = ACENgetpixel(source,0,0);

    y = 0;
    for (x=1; x<ginfo->width-1; ++x)
    {
        if (FrameColour!=ACENgetpixel(source,x,y))
        {
            FrameU = x;
            FrameV = y;
            break;
        }
    }

    y = ginfo->height-1;
    for (x=1; x<ginfo->width-1; ++x)
    {
        if (FrameColour != ACENgetpixel(source,x,y))
        {
            FrameU = x;
            FrameV = y;
            break;
        }
    }

    x = 0;
    for (y=0; y<ginfo->height; ++y)
    {
        if (FrameColour != ACENgetpixel(source,x,y))
        {
            FrameX = x;
            FrameY = y;
            break;
        }
    }

    x = ginfo->width-1;
    for (y=0; y<ginfo->height; ++y)
    {
        if (FrameColour != ACENgetpixel(source,x,y))
        {
            FrameX = x;
            FrameY = y;
            break;
        }
    }

    rect[0] = FrameX;   // return the center points
    rect[1] = FrameY;
    rect[2] = FrameU;
    rect[3] = FrameV;
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutacenter(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('A','C','N','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Acenter");
        strcpy(about->longnamestr,  "Auto Center");
        about->numoptions =         0;
    }
    return(about);
}

FPUBLIC int openacenter(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoacenter(FINSTANCE *fx, int framenum)
{
    int acenterrect[4];
    int acenterx;
    int acentery;
    int acenteru;
    int acenterv;
    GINFO *ginfo;

    acenter(acenterrect,fx->source);
    acenterx = acenterrect[0];
    acentery = acenterrect[1];
    acenteru = acenterrect[2];
    acenterv = acenterrect[3];

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        ginfo->centerx  = acenteru;
        ginfo->centery  = acentery;
    }
    return(ginfo);
}

FPUBLIC int filteracenter(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int acenterrect[4];
    int acenterx;
    int acentery;
    int acenteru;
    int acenterv;
    int acentercolour;
    int bpp = ginfo->bpp;
    char *s;
    char *d;

    acentercolour = ACENgetpixel(fx->source,0,0);

    acenter(acenterrect,fx->source);
    acenterx = acenterrect[0];
    acentery = acenterrect[1];
    acenteru = acenterrect[2];
    acenterv = acenterrect[3];

    for (y=0; y<ginfo->height; ++y)
    {
        d = dest;
        for (x=0; x<ginfo->width; ++x)
        {
            s = (char *)fx->source[0].image+y*fx->source[0].rowbytes+x*bpp/8;
            if ((x==acenterx && y==acentery) || (x==acenteru && y==acenterv))
            {
                if (bpp==8)
                {
                    *d++ = (char)acentercolour;
                }
                else
                {
                    *(int *)d = acentercolour;
                    d += 4;
                }
            }
            else
            {
                if (bpp==8)
                {
                    *d++ = *s;
                }
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

FPUBLIC int closeacenter(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

