// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                     Sharpen - Sharpen Image                      */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on blur                                  */
/*    1.01 030703 ID fixed, now uses a propper sharpening conv. ker.*/
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* sharpen(method)                                                  */
/* int method;    1 is typical.                                     */
/*                2 is supersharp                                   */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fsharpen.h"
#include <string.h>
#include <math.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/
/*
#define SHARPENPI 3.141592654

static double SHARPENh(double x,double y,double AMT)
{
    double dist;

    dist = sqrt(x*x+y*y);

    return(cos(0.95*SHARPENPI*dist)/(AMT*dist*dist+1));
}

#undef SHARPENPI

static void SHARPENmakematrix(double *owt,int MATRIX,double AMT)
{
    int x,y;
    double sum;

    sum = 0.0;
    for (y=0; y<MATRIX; ++y)
    {
        for (x=0; x<MATRIX; ++x)
        {
            sum += SHARPENh((x-((MATRIX-1)/2)),(y-((MATRIX-1)/2)),AMT);
        }
    }

    for (y=0; y<MATRIX; ++y)
    {
        for (x=0; x<MATRIX; ++x)
        {
            *owt = SHARPENh((x-((MATRIX-1)/2)),(y-((MATRIX-1)/2)),AMT)/sum;
            owt++;
        }
    }
}
*/
#define SHARPENreadpixel(x,y) (unsigned char)*(source+channel+x*4+y*srowbytes)
#define SHARPENwritepixel(x,y,c) *(dest+channel+x*4+y*drowbytes)=(char)(unsigned char)c

static void SHARPENwaliasf(char *dest,int drowbytes,char *source,int srowbytes, int width, int height, int MATRIX,double *owt)
{
    int x,y;
    int sx,sy;
    int cx,cy;
    double ap[4];
    unsigned char ip[4];
    double *wt;
    int channel;

    for (y=0; y<height; ++y)
    {
        for (x=0; x<width; ++x)
        {
            for (channel=0; channel<4; ++channel)
            {
                ap[channel] = 0;
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

                        ap[channel] += (double)SHARPENreadpixel(cx,cy) * *wt++;
                    }
                }
                if (ap[channel]<0) ap[channel] = 0;
                if (ap[channel]>255) ap[channel] = 255;
                ip[channel] = (unsigned char) ap[channel];
            }
            if (((ARGB *)&ip[0])->a<0xf0)
                for (channel=0; channel<4; ++channel)
                    ip[channel] = SHARPENreadpixel(x,y);

            for (channel=0; channel<4; ++channel)
                SHARPENwritepixel(x,y,ip[channel]);
        }
    }
}

#undef SHARPENreadpixel
#undef SHARPENwritepixel

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutsharpen(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','H','R','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids & Frank Barchard");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "Sharpen");
        strcpy(about->longnamestr,  "Sharpen");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           1;
        about->option[0].max.i =           2;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "Method");
        strcpy(about->option[0].enumlist,  "");
    }
    return(about);
}

FPUBLIC int opensharpen(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infosharpen(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filtersharpen(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{

    double normal[]     = {0,-1,0,-1,5,-1,0,-1,0};
    double supersharp[] = {-1,-2,-1,-2,13,-2,-1,-2,-1};
    double * comatrix   = normal;
    int MATRIX;
    int ok=0;
    double AMT=5;

    if (fx->option)
    {
        AMT = fx->option[0].i;
    }
    if(AMT == 2)
        comatrix = supersharp;

    MATRIX = 3;
    if (comatrix)
    {
        SHARPENwaliasf(dest,rowbytes,(char *)fx->source[0].image,fx->source[0].rowbytes,
                    ginfo->width, ginfo->height,MATRIX,comatrix);
        ok = 1;
    }

    return(ok);
}

FPUBLIC int closesharpen(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

