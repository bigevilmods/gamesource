// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   WU - Wu's CRapi Module v1.08                   */
/*                                                                  */
/*         by Xiaolin Wu, F.Barchard, and A.Brownsword, EAC         */
/*                                                                  */
/*                    Main Module - Mar 24, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 95xxxx FB based on WU (U of W. Ontario) by Jeff Dyke     */
/*    1.01 95xxxx GS CRapi 1.10 changes                             */
/*    1.02 951201 FB abstract for web doc.                          */
/*    1.03 960617 GS crapi 1.20 update                              */
/*    1.04 961118 FB remove 256 colour restriction                  */
/*    1.05 990426 FB compile for 555 or 666                         */
/*    1.06 990703 LC inversetable struct CRINVERSETABLE             */
/*    1.07 010718 FB 565 varient                                    */
/*    1.08 030324 ID respectalpha                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Wu's colour cube.  See Graphic Gems II, article 3.2 pg 126       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "wu.h"
#include <string.h>
#include <stdlib.h>

#define static

#ifndef WURBITS
#define WURBITS 6
#define WUGBITS 6
#define WUBBITS 6
#endif

#if WURBITS==5 && WUGBITS==5 && WUBBITS==5
#define WU_about WU555_about
#define WU_open  WU555_open
#define WU_read  WU555_read
#define WU_close WU555_close
#define WU WU555
#define WUINTERNAL WU555INTERNAL
#elif WURBITS==7 && WUGBITS==7 && WUBBITS==7
#define WU_about WU777_about
#define WU_open  WU777_open
#define WU_read  WU777_read
#define WU_close WU777_close
#define WU WU777
#define WUINTERNAL WU777INTERNAL
#endif


#define WURLEVELS (1<<WURBITS)
#define WUGLEVELS (1<<WUGBITS)
#define WUBLEVELS (1<<WUBBITS)
#define WURLEVELS1  ((1<<WURBITS)+1)
#define WUGLEVELS1  ((1<<WUGBITS)+1)
#define WUBLEVELS1  ((1<<WUBBITS)+1)
#define WUREDMULT   (WUGLEVELS1*WUBLEVELS1)
#define WUGREENMULT WUBLEVELS1

#define WURED       2
#define WUGREEN     1
#define WUBLUE      0

/****************************************************************/
/*  WUs Colour Quantizer                                        */
/****************************************************************/

namespace WUINTERNAL
{

typedef struct
{
    int r0;          /* min value, exclusive */
    int r1;          /* max value, inclusive */
    int g0;
    int g1;
    int b0;
    int b1;
    int vol;
} WUBOX;

/* Histogram is in elements 1..HISTSIZE along each axis, */
/* element 0 is for base or marginal value */
/* NB: these must start out 0! */

typedef struct
{
    float       (*gm2)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1];
    int         (*wt)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1];
    int         (*mr)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1];
    int         (*mg)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1];
    int         (*mb)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1];

    char        *pixels;
    int         width;
    int         height;
    int         rowbytes;

    char *      wuMemory;
} WU;

/* build 3-D colour histogram of counts, r/g/b, c^2 */

static __inline void Hist3d(WU *wu, int *vwt, int *vmr, int *vmg, int *vmb,float *m2)
{
    int i, y, table[256];

    for(i=0; i<256; ++i)
        table[i]=i*i;

    for(y=0; y<wu->height; ++y)
    {
        ARGB *pixel = (ARGB *)(wu->pixels + y  *wu->rowbytes);
        int x = wu->width;
        while (x)
        {
            int r = pixel->r;
            int g = pixel->g;
            int b = pixel->b;

            int inr = (r>>(8-WURBITS))+1;
            int ing = (g>>(8-WUGBITS))+1;
            int inb = (b>>(8-WUBBITS))+1;
            int ind = inr*WUREDMULT + ing*WUGREENMULT + inb;

            ++vwt[ind];
            vmr[ind] += r;
            vmg[ind] += g;
            vmb[ind] += b;
            m2[ind] += (float)(table[r]+table[g]+table[b]);

            ++pixel;
            --x;
        }
    }
}


/* At conclusion of the histogram step, we can interpret */
/*   wt[r][g][b] = sum over voxel of P(c) */
/*   mr[r][g][b] = sum over voxel of r*P(c)  ,  similarly for mg, mb */
/*   m2[r][g][b] = sum over voxel of c^2*P(c) */
/* Actually each of these should be divided by wu->WUsize to give the usual */
/* interpretation of P() as ranging from 0 to 1, but we neednt do that here. */


/* We now convert histogram into moments so that we can rapidly calculate */
/* the sums of the above quantities over any desired WUbox. */


/* compute cumulative moments. */

static void M3d( int *vwt, int *vmr, int *vmg, int *vmb, float *m2)
{
    unsigned int ind1, ind2;
    unsigned char i, r, g, b;
    int line, line_r, line_g, line_b;
    int area[WUBLEVELS1], area_r[WUBLEVELS1], area_g[WUBLEVELS1], area_b[WUBLEVELS1];
    float    line2, area2[WUBLEVELS1];

    for(r=1; r<=WURLEVELS; ++r)
    {
        for(i=0; i<=WURLEVELS; ++i)
            area2[i] = (float)(area[i] = area_r[i] = area_g[i] = area_b[i] = 0);
        for(g=1; g<=WUGLEVELS; ++g)
        {
            line2 = (float)(line = line_r = line_g = line_b = 0);
            for(b=1; b<=WUBLEVELS; ++b)
            {
                ind1 = r*WUREDMULT + g*WUGREENMULT + b; /* [r][g][b] */
                line += vwt[ind1];
                line_r += vmr[ind1];
                line_g += vmg[ind1];
                line_b += vmb[ind1];
                line2 += m2[ind1];
                area[b] += line;
                area_r[b] += line_r;
                area_g[b] += line_g;
                area_b[b] += line_b;
                area2[b] += line2;
                ind2 = ind1 - WUGLEVELS1*WUBLEVELS1; /* [r-1][g][b] */
                vwt[ind1] = vwt[ind2] + area[b];
                vmr[ind1] = vmr[ind2] + area_r[b];
                vmg[ind1] = vmg[ind2] + area_g[b];
                vmb[ind1] = vmb[ind2] + area_b[b];
                m2[ind1] = m2[ind2] + area2[b];
            }
        }
    }
}


/* Compute sum over a WUbox of any given statistic */

static int Vol(WUBOX *cube, int (*mmt)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1])
{
    return( (*mmt)[cube->r1][cube->g1][cube->b1]
       -(*mmt)[cube->r1][cube->g1][cube->b0]
       -(*mmt)[cube->r1][cube->g0][cube->b1]
       +(*mmt)[cube->r1][cube->g0][cube->b0]
       -(*mmt)[cube->r0][cube->g1][cube->b1]
       +(*mmt)[cube->r0][cube->g1][cube->b0]
       +(*mmt)[cube->r0][cube->g0][cube->b1]
       -(*mmt)[cube->r0][cube->g0][cube->b0] );
}

/* The next two routines allow a slightly more efficient calculation */
/* of Vol() for a proposed subWUbox of a given WUbox.  The sum of Top() */
/* and Bottom() is the Vol() of a subWUbox split in the given direction */
/* and with the specified new upper bound. */

/* Compute part of Vol(cube, mmt) that doesnt depend on r1, g1, or b1 */
/* (depending on dir) */

static int Bottom(WUBOX *cube,unsigned char dir, int (*mmt)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1])
{
    switch(dir)
    {
        case WURED:
            return( -(*mmt)[cube->r0][cube->g1][cube->b1]
                +(*mmt)[cube->r0][cube->g1][cube->b0]
                +(*mmt)[cube->r0][cube->g0][cube->b1]
                -(*mmt)[cube->r0][cube->g0][cube->b0] );
        case WUGREEN:
            return( -(*mmt)[cube->r1][cube->g0][cube->b1]
                +(*mmt)[cube->r1][cube->g0][cube->b0]
                +(*mmt)[cube->r0][cube->g0][cube->b1]
                -(*mmt)[cube->r0][cube->g0][cube->b0] );
        case WUBLUE:
            return( -(*mmt)[cube->r1][cube->g1][cube->b0]
                +(*mmt)[cube->r1][cube->g0][cube->b0]
                +(*mmt)[cube->r0][cube->g1][cube->b0]
                -(*mmt)[cube->r0][cube->g0][cube->b0] );
    }
    return(0);
}



/* Compute remainder of Vol(cube, mmt), substituting pos for */
/* r1, g1, or b1 (depending on dir) */

static int Top(WUBOX *cube,unsigned char dir,int pos, int (*mmt)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1])
{
    switch(dir)
    {
        case WURED:
            return( (*mmt)[pos][cube->g1][cube->b1]
               -(*mmt)[pos][cube->g1][cube->b0]
               -(*mmt)[pos][cube->g0][cube->b1]
               +(*mmt)[pos][cube->g0][cube->b0] );

        case WUGREEN:
            return( (*mmt)[cube->r1][pos][cube->b1]
               -(*mmt)[cube->r1][pos][cube->b0]
               -(*mmt)[cube->r0][pos][cube->b1]
               +(*mmt)[cube->r0][pos][cube->b0] );

        case WUBLUE:
            return( (*mmt)[cube->r1][cube->g1][pos]
               -(*mmt)[cube->r1][cube->g0][pos]
               -(*mmt)[cube->r0][cube->g1][pos]
               +(*mmt)[cube->r0][cube->g0][pos] );
    }
    return(0);
}

/* Compute the weighted variance of a WUbox */
/* NB: as with the raw statistics, this is really the variance  *wu->WUsize */

static float Var(WU *wu,WUBOX *cube)
{
    float dr, dg, db, xx;

    dr = (float)Vol(cube, wu->mr);
    dg = (float)Vol(cube, wu->mg);
    db = (float)Vol(cube, wu->mb);
    xx =  (*wu->gm2)[cube->r1][cube->g1][cube->b1]
     -(*wu->gm2)[cube->r1][cube->g1][cube->b0]
     -(*wu->gm2)[cube->r1][cube->g0][cube->b1]
     +(*wu->gm2)[cube->r1][cube->g0][cube->b0]
     -(*wu->gm2)[cube->r0][cube->g1][cube->b1]
     +(*wu->gm2)[cube->r0][cube->g1][cube->b0]
     +(*wu->gm2)[cube->r0][cube->g0][cube->b1]
     -(*wu->gm2)[cube->r0][cube->g0][cube->b0];

    return( xx - (dr*dr+dg*dg+db*db)/(float)Vol(cube,wu->wt) );
}


/* We want to minimize the sum of the variances of two subWUboxes. */
/* The sum(c^2) terms can be ignored since their sum over both subWUboxes */
/* is the same (the sum for the whole WUbox) no matter where we split. */
/* The remaining terms have a minus sign in the variance formula, */
/* so we drop the minus sign and MAXIMIZE the sum of the two terms. */

static float Maximize(WU *wu,WUBOX *cube,unsigned char dir,
                    int first, int last, int *cut,
                    int whole_r, int whole_g, int whole_b, int whole_w)
{
    int half_r, half_g, half_b, half_w;
    int base_r, base_g, base_b, base_w;
    int i;
    float temp, max;

    base_r = Bottom(cube, dir, wu->mr);
    base_g = Bottom(cube, dir, wu->mg);
    base_b = Bottom(cube, dir, wu->mb);
    base_w = Bottom(cube, dir, wu->wt);
    max = (float)0.0;
    *cut = -1;
    for(i=first; i<last; ++i)
    {
        half_r = base_r + Top(cube, dir, i, wu->mr);
        half_g = base_g + Top(cube, dir, i, wu->mg);
        half_b = base_b + Top(cube, dir, i, wu->mb);
        half_w = base_w + Top(cube, dir, i, wu->wt);

        /* now half_x is sum over lower half of WUbox, if split at i */

        if (half_w == 0)    /* subWUbox could be empty of pixels! */
        {
          continue;             /* never split into an empty WUbox */
        }
        else
            temp = ((float)half_r*half_r + (float)half_g*half_g +
                    (float)half_b*half_b)/half_w;

        half_r = whole_r - half_r;
        half_g = whole_g - half_g;
        half_b = whole_b - half_b;
        half_w = whole_w - half_w;
        if (half_w == 0)
        {      /* subWUbox could be empty of pixels! */
            continue;             /* never split into an empty WUbox */
        }
        else
            temp += ((float)half_r*half_r + (float)half_g*half_g +
                     (float)half_b*half_b)/half_w;

        if (temp > max) {max=temp; *cut=i;}
    }
    return(max);
}

static int Cut(WU *wu,WUBOX *set1, WUBOX *set2)
{
    unsigned char dir;
    int cutr, cutg, cutb;
    float maxr, maxg, maxb;
    int whole_r, whole_g, whole_b, whole_w;

    whole_r = Vol(set1, wu->mr);
    whole_g = Vol(set1, wu->mg);
    whole_b = Vol(set1, wu->mb);
    whole_w = Vol(set1, wu->wt);

    maxr = Maximize(wu,set1, WURED, set1->r0+1, set1->r1, &cutr,
            whole_r, whole_g, whole_b, whole_w);
    maxg = Maximize(wu,set1, WUGREEN, set1->g0+1, set1->g1, &cutg,
            whole_r, whole_g, whole_b, whole_w);
    maxb = Maximize(wu,set1, WUBLUE, set1->b0+1, set1->b1, &cutb,
            whole_r, whole_g, whole_b, whole_w);

    if( (maxr>=maxg)&&(maxr>=maxb) )
    {
        dir = WURED;
        if (cutr < 0) return(0); /* cant split the WUbox */
    }
    else
    if( (maxg>=maxr)&&(maxg>=maxb) )
        dir = WUGREEN;
    else
        dir = WUBLUE;

    set2->r1 = set1->r1;
    set2->g1 = set1->g1;
    set2->b1 = set1->b1;

    switch (dir)
    {
        case WURED:
            set2->r0 = set1->r1 = cutr;
            set2->g0 = set1->g0;
            set2->b0 = set1->b0;
            break;
        case WUGREEN:
            set2->g0 = set1->g1 = cutg;
            set2->r0 = set1->r0;
            set2->b0 = set1->b0;
            break;
        case WUBLUE:
            set2->b0 = set1->b1 = cutb;
            set2->r0 = set1->r0;
            set2->g0 = set1->g0;
            break;
    }
    set1->vol=(set1->r1-set1->r0)*(set1->g1-set1->g0)*(set1->b1-set1->b0);
    set2->vol=(set2->r1-set2->r0)*(set2->g1-set2->g0)*(set2->b1-set2->b0);
    return(1);
}

static void Mark(WUBOX *cube,int label,CRINVERSETABLE *inversetbl)
{
    int r, g, b;

    unsigned char *table = (unsigned char*)(inversetbl) + sizeof(CRINVERSETABLE);

    for(r=cube->r0; r<cube->r1; ++r)
        for(g=cube->g0; g<cube->g1; ++g)
            for(b=cube->b0; b<cube->b1; ++b)
                table[r*WUGLEVELS*WUBLEVELS + g*WUBLEVELS + b] = (char)label;
}

static WU *AllocWuData(void)
{
    int memsize = sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1 +
                  sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1 +
                  sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1 +
                  sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1;

    WU *wu = (WU *)galloc(sizeof(WU));
    if (!wu)
        return(0);

    memset(wu, 0, sizeof(WU));

    /* allocate from most persistant to least */
    wu->wuMemory = (char *) galloc(memsize);
    wu->gm2      = (float (*)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1]) galloc(sizeof(float)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1);


    if (!wu->wuMemory || !wu->gm2)
    {
        if (wu->wuMemory)   gfree(wu->wuMemory);
        if (wu->gm2)        gfree(wu->gm2);
        gfree(wu);
        return(0);
    }

    memset(wu->wuMemory, 0, memsize);
    memset(wu->gm2, 0, sizeof(float)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1);

    wu->wt  = (int (*)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1]) wu->wuMemory;
    wu->mr  = (int (*)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1]) ((char *)wu->wt  + sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1);
    wu->mg  = (int (*)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1]) ((char *)wu->mr  + sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1);
    wu->mb  = (int (*)[WURLEVELS1][WUGLEVELS1][WUBLEVELS1]) ((char *)wu->mg  + sizeof(int)*WURLEVELS1*WUGLEVELS1*WUBLEVELS1);

    return(wu);
}

}; // end of Wu namespace
using namespace WUINTERNAL;

/****************************************************************/
/* CRapi functions                                             */
/****************************************************************/

CRABOUT *GCALL WU_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('W','U','C','R');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       1;
        about->respectalpha =       0;
        strcpy(about->authorstr,    "Wu, F.Barchard, A.Brownsword");
        strcpy(about->versionstr,   "1.05");
#if WURBITS==5 && WUGBITS==5 && WUBBITS==5
        strcpy(about->namestr,      "Wu555");
        strcpy(about->longnamestr,  "Wu's 555");
#elif WURBITS==7 && WUGBITS==7 && WUBBITS==7
        strcpy(about->namestr,      "Wu777");
        strcpy(about->longnamestr,  "Wu's 777");
#else
        strcpy(about->namestr,      "Wu");
        strcpy(about->longnamestr,  "Wu's 666");
#endif
    }

    return(about);
}

CRINSTANCE *GCALL WU_open(void)
{
    CRINSTANCE *crx;

    /* allocate instance */
    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        WU *wuData = AllocWuData();
        if (wuData)
        {
            memset(crx, 0, sizeof(CRINSTANCE));
            crx->signature  = GMAKEID('W','U','C','R');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)wuData;
            return(crx);
        }
        gfree(crx);
    }
    return((CRINSTANCE *)0);
}

int GCALL WU_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    WU *wu = (WU *)(crx->cref);

    if (ginfo->bpp != 32)
        return(0);

    wu->width    = ginfo->width;
    wu->height   = ginfo->height;
    wu->rowbytes = rowbytes;
    wu->pixels   = (char *)source;

    Hist3d(wu, (int  *) wu->wt, (int  *) wu->mr, (int  *) wu->mg,
               (int  *) wu->mb, (float  *) wu->gm2);

    return(1);
}

ARGB *GCALL WU_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    int i,k,next;
    int weight;
    ARGB *colourtable;
    WU *wu = (WU *)crx->cref;
    WUBOX *cube=0;
    float *vv=0;
    float temp;
    int realcolours;

    cube = (WUBOX *)galloc(sizeof(WUBOX)*numcolours);
    vv = (float *)galloc(sizeof(float)*numcolours);

    /* make histogram into moments */

    M3d((int  *) wu->wt, (int  *) wu->mr, (int  *) wu->mg, (int  *) wu->mb, (float  *) wu->gm2);

    cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
    cube[0].r1 = WURLEVELS;
    cube[0].g1 = WUGLEVELS;
    cube[0].b1 = WUBLEVELS;
    next = 0;

    realcolours = numcolours;
    for(i=1; i<realcolours; ++i)
    {
        if (Cut(wu, &cube[next], &cube[i]))
        {
            /* volume test ensures we wont try to cut one-cell WUbox */

            vv[next] = (cube[next].vol>1) ? Var(wu, &cube[next]) : (float)0.0;
            vv[i] = (cube[i].vol>1) ? Var(wu, &cube[i]) : (float)0.0;
        }
        else
        {
            vv[next] = (float)0.0;  /* dont try to split this WUbox again */
            i--;                    /* didnt create WUbox i */
        }

        next = 0; temp = vv[0];
        for(k=1; k<=i; ++k)
            if (vv[k] > temp)
            {
                temp = vv[k]; next = k;
            }

        if (temp <= 0.0)
        {
            realcolours = i+1;
            break;
        }
    }

    gfree((char *)wu->gm2);
    wu->gm2 = NULL;

    /* alloc enough memory to hold struct, colourtable and *inversetable */

    colourtable = (ARGB *)galloc(numcolours*sizeof(ARGB));

    if (inversetable)
    {
        int isize=sizeof(CRINVERSETABLE) + sizeof(unsigned char)*WURLEVELS*WUGLEVELS*WUBLEVELS;
        *inversetable = (CRINVERSETABLE *) galloc(isize);
#ifdef DEBUG
        memset(*inversetable,0xea,isize);
#endif
        (*inversetable)->signature = GMAKEID('I','N','V','W');
        (*inversetable)->size      = sizeof(CRINVERSETABLE) + sizeof(unsigned char)*WURLEVELS*WUGLEVELS*WUBLEVELS;
        (*inversetable)->alphabits = 0;
        (*inversetable)->redbits   = WURBITS;
        (*inversetable)->greenbits = WUGBITS;
        (*inversetable)->bluebits  = WUBBITS;
    }
    if (colourtable)
    {
        memset(colourtable, 0, numcolours*sizeof(ARGB));
        if (inversetable && *inversetable)
        {
            // setting the table to 0
            memset((unsigned char *)(*inversetable)+sizeof(CRINVERSETABLE), 0, WURLEVELS*WUGLEVELS*WUBLEVELS);
        }

        for(k=0; k < realcolours; ++k)
        {
            /* update inversetable */
            if (inversetable && *inversetable)
                Mark(&cube[k], k, *inversetable);

            /* update colour table */
            weight = Vol(&cube[k], wu->wt);
            if (weight)
            {
                colourtable[k].a = 255;
                colourtable[k].r = (unsigned char)(Vol(&cube[k], wu->mr) / weight);
                colourtable[k].g = (unsigned char)(Vol(&cube[k], wu->mg) / weight);
                colourtable[k].b = (unsigned char)(Vol(&cube[k], wu->mb) / weight);
            }
        }
    }

    if (cube) gfree(cube);
    if (vv) gfree(vv);
    if (wu->wuMemory) gfree(wu->wuMemory);
    if (wu->gm2)      gfree(wu->gm2);
    gfree(wu);
    gfree(crx);
    if (returncolours)
        *returncolours = realcolours;

    return(colourtable);
}

#ifdef aboutwu
#undef openwu
#undef readwu
#undef closewu
#undef WU
#endif
#undef WURBITS
#undef WUGBITS
#undef WUBBITS
#undef WURLEVELS
#undef WUGLEVELS
#undef WUBLEVELS
#undef WURLEVELS1
#undef WUGLEVELS1
#undef WUBLEVELS1
#undef WUINTERNAL

#undef WUREDMULT
#undef WUGREENMULT
#undef WURED
#undef WUGREEN
#undef WUBLUE

