// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 KSOM - Kohonen Neural Network                    */
/*                                                                  */
/*                      by Ian Davids, EAC                          */
/*                                                                  */
/*                    Main Module - June 16, 2003                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 030616 ID uses a som with a few mods to build a clut     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ksom.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>


// if we are working with msvc make sure my inline functions
// get inlined... The distance function is the most important
// of all of them to get inlined
#if defined(_MSC_VER)
#define INLINE __forceinline
#else
#define INLINE inline
#endif

typedef struct KSOM
{
    ARGB   * clut;     // array to hold my current neural net
    int      count;    // count of the number of entries in the net
    double * bias;     // bias array for the net. used to help calculate nearest
    double * freq;     // frequency information, used to calculate bias at each step
        // constants used to control matrix behaviour
    double beta;
    double gamma;
    double alphaExp;
    double rStart;
    double rExp;
        // in flux values while the algorithm is running
    double alpha;
    double radius;
    int    time;
}KSOM;


static unsigned int square_table[512];
static int          squareinit=0;

// ------------------------------------------------------------
static void square_init()
{
    if(!squareinit)
    {
        // Fill up square_table
        for (int i=0; i<512; ++i)
            square_table[i] = (i-256)*(i-256);
        squareinit = 1;
    }
}

// ------------------------------------------------------------
static void init_nn(KSOM * K,int count)
{
        // Build memory buffers for NN
    K->clut     = (ARGB *)galloc(sizeof(ARGB)*count);
    K->count    = count;
    K->bias     = (double *)galloc(sizeof(double)*count);
    K->freq     = (double *)galloc(sizeof(double)*count);
        // Initialize the memory buffers for NN
    memset(K->bias,0,sizeof(double)*count);
    memset(K->freq,0,sizeof(double)*count);
    for(int i = 0; i < count; i++)  // set the initial list to a grey ramp
    {
        unsigned char val = (unsigned char)((double)i/(double)count*255.0);
        K->clut[i].a = 255;
        K->clut[i].r = val;
        K->clut[i].g = val;
        K->clut[i].b = val;
    }
        // Set constant control values for program execution
    K->beta     = 1.0/1024.0; // beta and gamma are used to calculate next
    K->gamma    = 1024;       // bias and frequency values
    K->alphaExp = -0.03;      // alpha exponent is used to calculate the next alpha value
    K->rStart   = 32;         // radius start value
    K->rExp     = -0.0325;    // radius exponent control value
        // Set start values for in flux variables
    K->alpha    = 1;
    K->radius   = K->rStart;
    K->time     = 0;
}

// returns the number of scans and number of pixels in each scan.
// this is here so I can later try a different randomization than
// using linear scans of the image.
static int get_numpass(KSOM * K,int n,int * pc)
{
    int possible_strides[] = {487,491,499,503};
    for(int i = 0; i < 4; i++)
    {
        if(n%possible_strides[i] != 0)
        {
            *pc = possible_strides[i];
            break;
        }
    }
    return 100;
}

// this function could later be a more complex lookup function to give a better
// odering to the image scan.
static INLINE ARGB get_pixel(ARGB * image,int n,int P,int *curp)
{

    ARGB temp = image[(*curp)];
    *curp     = (*curp + P)%n;
    return temp;
}

static INLINE double dist_32(ARGB a,ARGB b,double bias)
{
    return (  square_table[a.r - b.r + 256]
            + square_table[a.g - b.g + 256]
            + square_table[a.b - b.b + 256]
            + square_table[a.a - b.a + 256] + bias*bias);
}

static INLINE double dist_24(ARGB a,ARGB b,double bias)
{
    return (  square_table[a.r - b.r + 256]
            + square_table[a.g - b.g + 256]
            + square_table[a.b - b.b + 256] + bias*bias);
}

static INLINE void learn_pixel_24(KSOM * K, ARGB input,double * radinf)
{
    unsigned char bestmatch = 0;
    double bestdist         = dist_24(input,K->clut[0],K->bias[0]);
    double curdist          = 0;
    double gb               = K->gamma*K->beta;
        // Find best match
    for(int i = 1; i < K->count; i++)
    {
        curdist = dist_24(input,K->clut[i],K->bias[i]);
        if(curdist < bestdist)
        {
            bestmatch = i;
            bestdist  = curdist;
        }
        K->bias[i] += K->freq[i];    // * gamma * beta = 1
        K->freq[i] -= K->beta*K->freq[i];
    }
        // update the item with the best distance's bias etc.
    K->bias[bestmatch] -= gb;
    K->freq[bestmatch] += K->beta;



    int rad     = (int)(K->radius*.25);
    int r1      = bestmatch-rad<0?rad+(bestmatch-rad):rad;
    int r2      = bestmatch+rad>=K->count?rad-(bestmatch+rad-K->count+1):rad;
        //K->clut[bestmatch].a -= (unsigned char)(K->alpha*(K->clut[bestmatch].a - input.a));
    K->clut[bestmatch].r -= (unsigned char)(K->alpha*(K->clut[bestmatch].r - input.r));
    K->clut[bestmatch].g -= (unsigned char)(K->alpha*(K->clut[bestmatch].g - input.g));
    K->clut[bestmatch].b -= (unsigned char)(K->alpha*(K->clut[bestmatch].b - input.b));
    for(int j = 1,b=bestmatch-1; j < r1; j++,b--)
    {
        register double t = radinf[j];
        K->clut[b].r -= (unsigned char)(t*(K->clut[b].r - input.r));
        K->clut[b].g -= (unsigned char)(t*(K->clut[b].g - input.g));
        K->clut[b].b -= (unsigned char)(t*(K->clut[b].b - input.b));
    }
    for(int j = 1,a=bestmatch+1; j < r2; j++,a++)
    {
        register double t = radinf[j];
        K->clut[a].r -= (unsigned char)(t*(K->clut[a].r - input.r));
        K->clut[a].g -= (unsigned char)(t*(K->clut[a].g - input.g));
        K->clut[a].b -= (unsigned char)(t*(K->clut[a].b - input.b));
    }

        /*
        double radinf             = 1 - (double)(square_table[bestmatch-i + 256])/(double)(square_table[(int)K->radius + 256]);
        radinf = radinf < 0?0:(radinf > 1?1:radinf);
        radinf                    = K->alpha*radinf;
        K->clut[i].a -= (unsigned char)(radinf*(K->clut[i].a - input.a));
        K->clut[i].r -= (unsigned char)(radinf*(K->clut[i].r - input.r));
        K->clut[i].g -= (unsigned char)(radinf*(K->clut[i].g - input.g));
        K->clut[i].b -= (unsigned char)(radinf*(K->clut[i].b - input.b));
            */

}

static INLINE void learn_pixel_32(KSOM * K, ARGB input,double * radinf)
{
    unsigned char bestmatch = 0;
    double bestdist         = dist_32(input,K->clut[0],K->bias[0]);
    double curdist          = 0;
    double gb               = K->gamma*K->beta;
        // Find best match
    for(int i = 1; i < K->count; i++)
    {
        curdist = dist_32(input,K->clut[i],K->bias[i]);
        if(curdist < bestdist)
        {
            bestmatch = i;
            bestdist  = curdist;
        }
        K->bias[i] += K->freq[i];  // * gamma * beta = 1
        K->freq[i] -= K->beta*K->freq[i];
    }
        // update the item with the best distance's bias etc.
    K->bias[bestmatch] -= gb;
    K->freq[bestmatch] += K->beta;

/*
        // update radius
    double rStart = (bestmatch - K->radius*.5);
    rStart = rStart > 0?rStart:0;

    double rEnd = (bestmatch + K->radius*.5);
    rEnd = rEnd > K->count?K->count:rEnd;

    int irEnd   = (int)rEnd;
    int irStart = (int)ceil(rStart);
*/
    int rad     = (int)(K->radius*.25);

    K->clut[bestmatch].a -= (unsigned char)(K->alpha*(K->clut[bestmatch].a - input.a));
    K->clut[bestmatch].r -= (unsigned char)(K->alpha*(K->clut[bestmatch].r - input.r));
    K->clut[bestmatch].g -= (unsigned char)(K->alpha*(K->clut[bestmatch].g - input.g));
    K->clut[bestmatch].b -= (unsigned char)(K->alpha*(K->clut[bestmatch].b - input.b));

    for(int j = 1,b=bestmatch-1,a=bestmatch+1; j < rad; j++,b--,a++)
    {
        register double t = radinf[j];
        if(a < K->count)
        {
            K->clut[a].a -= (unsigned char)(t*(K->clut[a].a - input.a));
            K->clut[a].r -= (unsigned char)(t*(K->clut[a].r - input.r));
            K->clut[a].g -= (unsigned char)(t*(K->clut[a].g - input.g));
            K->clut[a].b -= (unsigned char)(t*(K->clut[a].b - input.b));
        }
        if(b >= 0)
        {
            K->clut[b].a -= (unsigned char)(t*(K->clut[b].a - input.a));
            K->clut[b].r -= (unsigned char)(t*(K->clut[b].r - input.r));
            K->clut[b].g -= (unsigned char)(t*(K->clut[b].g - input.g));
            K->clut[b].b -= (unsigned char)(t*(K->clut[b].b - input.b));
        }
            /*
        double radinf             = 1 - (double)(square_table[bestmatch-i + 256])/(double)(square_table[(int)K->radius + 256]);
        radinf = radinf < 0?0:(radinf > 1?1:radinf);
        radinf                    = K->alpha*radinf;
        K->clut[i].a -= (unsigned char)(radinf*(K->clut[i].a - input.a));
        K->clut[i].r -= (unsigned char)(radinf*(K->clut[i].r - input.r));
        K->clut[i].g -= (unsigned char)(radinf*(K->clut[i].g - input.g));
        K->clut[i].b -= (unsigned char)(radinf*(K->clut[i].b - input.b));
            */
    }
}

static INLINE void recalc_radiusinf(KSOM * K,double * radinf)
{
    double rad2    = K->radius*K->radius;
    double gmul    = (1.0/rad2)*K->alpha;
    int rad        = (int)(K->radius*.25)+1;
    for(int i = 0; i < rad; i++)
        radinf[i] = gmul*(rad2 - square_table[i+256]);
}


static void learn_image_32(KSOM * K, ARGB * image, int width, int height,bool alpha)
{
    int P     = 0;
    int curp  = 0;
    int n     = width*height;
    int cycles = get_numpass(K,n,&P);
    int pixelcount = n/cycles;
    double radinf[32];
    if(alpha)
    {
        for(int i = 0; i < cycles; i++)
        {
            recalc_radiusinf(K,radinf);
            for(int j = 0; j < pixelcount; j++)
            {
                ARGB input = get_pixel(image,n,P,&curp);
                learn_pixel_32(K,input,radinf);
            }
            K->time++;
            K->alpha  = exp(K->alphaExp*K->time);
            K->radius = K->rStart*exp(K->rExp*K->time);
        }
    }
    else
    {
        for(int i = 0; i < cycles; i++)
        {
            recalc_radiusinf(K,radinf);
            for(int j = 0; j < pixelcount; j++)
            {
                ARGB input = get_pixel(image,n,P,&curp);
                learn_pixel_24(K,input,radinf);
            }
            K->time++;
            K->alpha  = exp(K->alphaExp*K->time);
            K->radius = K->rStart*exp(K->rExp*K->time);
        }
    }
}



/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRABOUT *GCALL KSOM_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('K','S','O','M');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       0;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "KSom");
        strcpy(about->longnamestr,  "K. Self Organizing Map");
    }
    return(about);
}

CRINSTANCE *GCALL KSOM_open(void)
{
    CRINSTANCE *crx=0;
    square_init();
    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        KSOM * K        = (KSOM *)galloc(sizeof(KSOM));
        if(K)
        {
            init_nn(K,256);
            crx->signature  = GMAKEID('K','S','O','M');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)K;
        }
    }
    return(crx);
}


int GCALL KSOM_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int ok=0;
    KSOM *K = (KSOM *)(crx->cref);

    if (K)
    {
        if (ginfo->bpp==32)
        {
            learn_image_32(K,(ARGB*)source,ginfo->width,ginfo->height,ginfo->alphabits > 0);
        }
        else
        {}
        ok = 1;
    }
    return(ok);
}

ARGB *GCALL KSOM_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    KSOM *K = (KSOM *)(crx->cref);
    if (K && K->clut)
    {
        if(K->bias)
            gfree(K->bias);
        if(K->freq)
            gfree(K->freq);
        if(returncolours)
            *returncolours = K->count;
        ARGB * temp = K->clut;
        gfree(crx);
        gfree(K);
        return temp;
    }
    return NULL;
}


