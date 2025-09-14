// (c) 2002-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*        Outline - Outline around transparent images (fonts)       */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 020705 FB based on halfsize and inspired by fontwiz      */
/*    1.01 020709 FB translucient, offset (for shadow)              */
/*    1.02 031215 FB check for 1x1 specifically                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* outline(int x, int y, int width, int height, int colour)         */
/* (same prototype as rect & crop)                                  */
/* int x,y;    position of outline (normally 0,0)                   */
/* int width;  horizontal size of outline.  normally 1              */
/* int height; vertical size of outline.  allows for aspect ratio   */
/* int colour; ARGB value of shadow.  normally 0xff000000           */
/*                                                                  */
/* Outline is intended for putting black outlines around fonts to   */
/* make them more readable against backgrounds and on NTSC monitors */
/* The image is made into a shadow (black) and stamped down at      */
/* various offsets and then the original stamped in the middle.     */
/*                                                                  */
/* The colour can be translucient creating a soft outline.          */
/* Normally colour would be BLACK but WHITE or any colour is fine   */
/* and will be blended with                                         */
/*                                                                  */
/* The resulting image will be larger than the original based on    */
/* width and height.  The x/y offset also affects this.             */
/* A special case exists for 1x1 transparent images (empty)         */
/*                                                                  */
/* Position meta information is updated in a way that reflects      */
/* font positioning.  Adding an outline should not affect font      */
/* spacing/kerning.                                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "foutline.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define GABS(x) ((x)>=0?(x):-(x))

void OUTLINE_apply(const GINFO *ginfo, void *dest, int rowbytes,
                   const GINFO *sginfo, void *source, int srowbytes,
                   int outx,int outy,int outw,int outh,int outc)
{
    int x,y;
    int cx=gmax(outx-outw,outw);
    int cy=gmax(outy-outh,outh);

    for (y=0; y<ginfo->height; ++y)
    {
        void *d = (void *)((char *)dest+y*rowbytes);
        memset(d,0,ginfo->width*sizeof(ARGB));
    }

    // stamp outlines
    //   x
    //  xxx
    // xxxxx
    //  xxx
    //   x
    for (int gridy=-outh; gridy<=outh; ++gridy)
    {
        for (int gridx=-outw; gridx<=outw; ++gridx)
        {
            if (GABS(gridx*gridy) <= GABS(outh*outw/2))
            {
                for (y=0; y<sginfo->height; ++y)
                {
                    for (x=0; x<sginfo->width; ++x)
                    {
                        ARGB *d = (ARGB *)((char *)dest+(y+cy+gridy)*rowbytes+(x+cx+gridx)*sizeof(ARGB));
                        const ARGB *s = (const ARGB *)((char *)source+y*srowbytes+x*sizeof(ARGB));

                        int sa = (s->a * ((outc>>24)&255))/255;
                        if (sa >= d->a)
                        {
                            d->a = sa;
                            d->r = (unsigned char) ((outc>>16)&255);
                            d->g = (unsigned char) ((outc>>8)&255);
                            d->b = (unsigned char) ((outc>>0)&255);
                        }
                    }
                }
            }
        }
    }

    // stamp in middle
    for (y=0; y<sginfo->height; ++y)
    {
        for (x=0; x<sginfo->width; ++x)
        {
            ARGB *d = (ARGB *)((char *)dest+(y+gmax(0,outh-outy))*rowbytes+(x+gmax(0,outw-outx))*sizeof(ARGB));
            const ARGB *s = (const ARGB *)((char *)source+y*srowbytes+x*sizeof(ARGB));
            if (s->a)
            {
                d->a = (unsigned char) (255-((255-s->a)*(255-d->a)/255));

                d->r = (unsigned char) ((s->a*s->r+(255-s->a)*(d->r))/255);
                d->g = (unsigned char) ((s->a*s->g+(255-s->a)*(d->g))/255);
                d->b = (unsigned char) ((s->a*s->b+(255-s->a)*(d->b))/255);

            }
        }
    }
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutoutline(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('O','U','T','L');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.01" );
        strcpy(about->namestr,      "Outline" );
        strcpy(about->longnamestr,  "Outline Image" );
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

        about->option[4].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "Colour");
        strcpy(about->option[4].enumlist,  "");
    }
    return about;
}

FPUBLIC int openoutline(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
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

FPUBLIC GINFO* infooutline(FINSTANCE* fx, int framenum)
{
    GINFO *ginfo;

    int outx=0;
    int outy=0;
    int outw=1;
    int outh=1;
    unsigned int outc=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) outx = fx->option[0].i;
        if (fx->numoptions>=2) outy = fx->option[1].i;
        if (fx->numoptions>=3) outw = fx->option[2].i;
        if (fx->numoptions>=4) outh = fx->option[3].i;
        if (fx->numoptions>=5) outc = fx->option[4].i;
    }
    if (outx<-outw) outx=-outw;
    if (outy<-outh) outy=-outh;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        if (ginfo->width>0 && ginfo->height>0 && !(ginfo->width==1 && ginfo->height==1))
        {
            ginfo->width  += gmax(outx,outw*2);
            ginfo->height += gmax(outy,outh*2);
            ginfo->defaultx += gmin(0,-outw+outx);
            ginfo->defaulty += gmin(0,-outh+outy);
        }
    }
    return(ginfo);
}

FPUBLIC int filteroutline(FINSTANCE* fx, GINFO* ginfo, char* dest, int rowbytes)
{
    int outx=0;
    int outy=0;
    int outw=1;
    int outh=1;
    unsigned int outc=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) outx = fx->option[0].i;
        if (fx->numoptions>=2) outy = fx->option[1].i;
        if (fx->numoptions>=3) outw = fx->option[2].i;
        if (fx->numoptions>=4) outh = fx->option[3].i;
        if (fx->numoptions>=5) outc = fx->option[4].i;
    }
    if (outx<-outw) outx=-outw;
    if (outy<-outh) outy=-outh;

    if (ginfo->width>0 && ginfo->height>0 && !(ginfo->width==1 && ginfo->height==1))
    {
        OUTLINE_apply(ginfo, dest, rowbytes,
            fx->source[0].ginfo, fx->source[0].image, fx->source[0].rowbytes,
            outx,outy,outw,outh,outc);
    }
    else
    {
        ((ARGB *)dest)->a = 0;
        ((ARGB *)dest)->r = 0;
        ((ARGB *)dest)->g = 0;
        ((ARGB *)dest)->b = 0;
    }

    return(1);
}

FPUBLIC int closeoutline(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
