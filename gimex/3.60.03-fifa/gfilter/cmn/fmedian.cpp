// (c) 1995-2007 Electronic Arts Inc.


/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*              Median - Median Pixel Selection Filter              */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 971021 FB based on fblur                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* median(float radius)                                             */
/*                                                                  */
/* Median takes a matrix (3x3) and sorts values, then takes         */
/* the pixel value at the index half way down the list.             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fmedian.h"
#include <string.h>
#include <math.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static double MEDIANh(double x,double y,double MU)
{
     return(1.0/(2.0*3.14159*MU*MU)*exp(-0.5*(x*x+y*y)/(MU*MU)));
}

static void MEDIANmakematrix(double *owt,int MATRIX,double MU)
{
    int x,y;
    double sum;

    sum = 0.0;
    for (y=0; y<MATRIX; ++y)
    {
        for (x=0; x<MATRIX; ++x)
        {
            sum += MEDIANh((x-((MATRIX-1)/2)),(y-((MATRIX-1)/2)),MU);
        }
    }

    for (y=0; y<MATRIX; ++y)
    {
        for (x=0; x<MATRIX; ++x)
        {
            *owt++ = MEDIANh((x-((MATRIX-1)/2)),(y-((MATRIX-1)/2)),MU)/sum;
        }
    }
}

#define MEDIANreadpixel(x,y) (unsigned char)*(source+channel+x*4+y*srowbytes)
#define MEDIANwritepixel(x,y,c) *(dest+channel+x*4+y*drowbytes)=(char)(unsigned char)c

static void MEDIANwaliasf(char *dest,int drowbytes,char *source,int srowbytes, int width, int height, int MATRIX,double *owt)
{
    int x,y;
    int sx,sy;
    int cx,cy;
    double ap;
    double *wt;
    int channel;

    for (channel=0; channel<4; ++channel)
    {
        for (y=0; y<height; ++y)
        {
            for (x=0; x<width; ++x)
            {
                ap = 0;
                wt = owt;
                for (sy=0; sy<MATRIX; ++sy)
                {
                    for (sx=0; sx<MATRIX; ++sx)
                    {
                        cx = x+sx-(MATRIX-1)/2;
                        if (cx<0) cx=0;
                        if (cx>=width) cx=width-1;
                        cy = y+sy-(MATRIX-1)/2;
                        if (cy<0) cy=0;
                        if (cy>=height) cy=height-1;

                        ap += MEDIANreadpixel(cx,cy) * *wt++;
                    }
                }
                MEDIANwritepixel(x,y,ap);
            }
        }
    }
}

#undef MEDIANreadpixel
#undef MEDIANwritepixel

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutmedian(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('B','L','U','R');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Median");
        strcpy(about->longnamestr,  "Gausian Median");
        about->numoptions =         1;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           0.1;
        about->option[0].max.f =           256.0;
        about->option[0].def.f =           4.0;
        strcpy(about->option[0].name,      "Radius");
        strcpy(about->option[0].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openmedian(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    /* allocate instance */
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

FPUBLIC GINFO *infomedian(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filtermedian(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    double *comatrix;
    int MATRIX;
    int ok=0;
    double MU=4.0;

    if (fx->option)
    {
        MU = fx->option[0].f;
    }

    MATRIX = (int)(MU*2.5)+3;
    comatrix = (double *) galloc(MATRIX*MATRIX*sizeof(double));
    if (comatrix)
    {
        MEDIANmakematrix(comatrix,MATRIX,MU);
        MEDIANwaliasf(dest,rowbytes,(char *)fx->source[0].image,fx->source[0].rowbytes,
                    ginfo->width, ginfo->height,MATRIX,comatrix);
        gfree(comatrix);
        ok = 1;
    }

    return(ok);
}

FPUBLIC int closemedian(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

