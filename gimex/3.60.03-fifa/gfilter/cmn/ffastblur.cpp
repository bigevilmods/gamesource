// (c) 2000-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  FastBlur - Gausian Blur Filter                  */
/*                                                                  */
/*                   by Alistair Milne, Bullfrog                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990816 AM based on fblur, using 2-pass approach          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* fastblur(double radius)                                          */
/*                                                                  */
/* Does the same as the fblur module, but does it much faster at    */
/* at the cost of a small amount of accuracy.                       */
/*                                                                  */
/* Make sure there's room at the edges because it doesn't cope      */
/* with them particularly well.                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ffastblur.h"
#include <string.h>
#include <math.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static double FASTBLURh(double x,double MU)
{
     return ( exp(-0.5*(x*x)/(MU*MU)) / (2.0*3.14159*MU*MU) );
}

static void FASTBLURmakematrix(double *owt,int MATRIX,double MU)
{
    int x, half;
    double sum;
    double *f;

    half = (MATRIX-1)/2;

    f = owt;
    sum = 0.0;
    for (x=-half; x < MATRIX-half; ++x)
    {
        *f = FASTBLURh(x,MU);
        sum += *f++;
    }

    f = owt;
    for (x=0; x<MATRIX; ++x)
    {
        *f++ /= sum;
    }
}

static void FASTBLURwaliasf(char *dest,int dpitch,char *source,int spitch, int width, int height, int MATRIX, double *owt)
{
    int x,y;
    int start;
    int channel, half;
    double ap;
    double *f;
    unsigned char *buffer, *rDst, *rSrc, *pDst, *pSrc, *pEnd;

    half = (MATRIX-1)/2;
    buffer = (unsigned char*)(galloc(height));

    for (channel=0; channel<4; ++channel)
    {
        /* pass 1 - horizontal blur */
        rDst = (unsigned char*)(dest) + channel;
        rSrc = (unsigned char*)(source) + channel;

        for (y=0; y<height; ++y)
        {
            pDst = rDst;

            for (x=0; x<width; ++x)
            {
                start = gmax( x-half, 0);
                pSrc = rSrc + start*4;
                pEnd = rSrc + gmin( x-half+MATRIX-1, width-1);
                f = owt + start - (x-half);

                ap = 0.0;
                while ( pSrc <= pEnd)
                {
                    ap += (*f++) * (*pSrc);
                    pSrc += 4;
                }

                *pDst = (unsigned char)(ap);
                pDst += 4;
            }

            rDst += dpitch;
            rSrc += spitch;
        }


        /* pass 2 - vertical blur */
        rDst = (unsigned char*)(dest) + channel;

        for (x=0; x<width; ++x)
        {
            pSrc = rDst;
            pDst = rDst;

            for (y=0; y<height; ++y)
                buffer[y] = pSrc[y*spitch];

            for (y=0; y<height; ++y)
            {
                start = gmax( y-half, 0);
                pSrc = buffer + start;
                pEnd = buffer + gmin( y-half+MATRIX-1, height-1);
                f = owt + start - (y-half);

                ap = 0.0;
                for (sx = sx_s; sx <= sx_e; ++sx)
                {
                    ap += (*f++) * (*pSrc++);
                }

                *pDst = (unsigned char)(ap);
                pDst += dpitch;
            }

            rDst += 4;
        }
    }

    gfree((void*)(buffer));
}



/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutfastblur(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','B','L','R');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Alistair Milne");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Fast Blur");
        strcpy(about->longnamestr,  "Fast Gaussian Blur");
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

FPUBLIC int openfastblur(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infofastblur(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterfastblur(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
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
    comatrix = (double*)( galloc(MATRIX*sizeof(double)));
    if (comatrix)
    {
        FASTBLURmakematrix( comatrix,MATRIX,MU);
        FASTBLURwaliasf( dest,rowbytes,fx->source[0].image,fx->source[0].rowbytes,
                         ginfo->width, ginfo->height,MATRIX,comatrix);
        gfree((void*)(comatrix));
        ok = 1;
    }

    return(ok);
}

FPUBLIC int closefastblur(FINSTANCE *fx)
{
    return (gfree((void*)(fx)));
}

