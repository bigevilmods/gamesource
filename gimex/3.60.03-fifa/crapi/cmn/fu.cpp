// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                Fu - Franks Wu Wrapper Module v1.03               */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*                    Main Module - Mar 24, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010724 FB first version place holder based on exact      */
/*    1.01 010731 FB call wu                                        */
/*    1.02 010801 FB median alpha, scale coefficients by alpha      */
/*    1.03 030324 ID respectalpha                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* Fu uses the alpha channel to partition the palette into subbands */
/* and use Wu on each, partitioning the palette based on a simple   */
/* alpha histogram                                                  */
/*                                                                  */
/* 3 bit is reasonable for Gamecube alpha, and is generally         */
/* acceptable from experience on antialiased images/fonts.          */
/* This algorithm will prefer to half a small number of partitions. */
/* Symetry around 0, 128 and 255 is desirable, but symetry around   */
/* 128 requires a non-power of 2 division.  It is observed that     */
/* the 3 bit value, 100 should be 10010010 which is 146             */
/* A stepping of 36.5 gives reasonable results, but is rounded up   */
/* to 37, giving the opaque colours a smaller range                 */
/*                                                                  */
/* >math -36/2..300+=37                                             */
/* -18                                                              */
/* 19                                                               */
/* 56                                                               */
/* 93                                                               */
/* 130                                                              */
/* 167                                                              */
/* 204                                                              */
/* 241                                                              */
/* 278                                                              */
/*                                                                  */
/* This give reasonable symetry around 146 as well as 0 and 255     */
/* The formula (alpha+37/2)/37 determines which bin to use          */
/*                                                                  */
/* FU ambiguously means                                             */
/* Franks Wu                                                        */
/* Four-d Wu                                                        */
/* FooBar                                                           */
/*                                                                  */
/* Using WU777 on the test image el12.ssh produced a PSNR of 53.42  */
/* vs WU 666 PSNR of 52.92.  The slight improvement is not worth    */
/* the complexity, memory and performance.                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fu.h"
#include "wu.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
int FUverbose=0;
}

#define MAXFUCUBES 8
typedef struct
{
    int count;
    int alphasum;
    CRINSTANCE *wu;

} FUCUBE;

typedef struct
{
    FUCUBE fucube[MAXFUCUBES];
    int alphacount[256];
} FU;

static void fuaddhist(FU *fu, const GINFO *ginfo, const ARGB *p)
{
    int alphabin = (p->a+16)/37;
    FUCUBE *fucube=&fu->fucube[alphabin];

    ++fucube->count;
    ++fu->alphacount[p->a];
    fucube->alphasum += p->a;
    if (!fucube->wu)
    {
        fucube->wu = WU_open();
    }
    WU_read(fucube->wu, ginfo, p, 0);
}

int FU_computemedianalpha(int bin,int *alphacount)
{
    int minbin = gmax(0,(bin*37)-16);
    int maxbin = gmin(256,((bin+1)*37)-16);
    int i;
    int totalbin=0;
    int medbin=0;

    for (i=minbin; i<maxbin; ++i)
        totalbin += alphacount[i];

    for (i=minbin; i<maxbin; ++i)
    {
        if (medbin + alphacount[i] > totalbin/2)
            break;
        medbin += alphacount[i];
    }
//    printf("%d %d %d %d %d\n",minbin,maxbin, totalbin, medbin, i);
    return(i);
}

/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRABOUT *GCALL FU_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('F','B','W','U');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       0;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.03");
        strcpy(about->namestr,      "Fu");
        strcpy(about->longnamestr,  "Franks Wu");
    }
    return(about);
}

CRINSTANCE *GCALL FU_open(void)
{
    CRINSTANCE *crx=0;

    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        FU *fu = (FU *) galloc(sizeof(FU));
        memset(crx, 0, sizeof(CRINSTANCE));
        if (fu)
        {
            memset(fu, 0, sizeof(FU));
            crx->signature  = GMAKEID('F','B','W','U');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)fu;
        }
        else
        {
            gfree(crx);
            crx = 0;
        }
    }
    return(crx);
}


int GCALL FU_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int x;
    int y;
    int ok=0;
    unsigned char *s;
    FU *fu = (FU *)(crx->cref);
    GINFO ginfo1;

    ginfo1 = *ginfo;
    ginfo1.width = 1;
    ginfo1.height = 1;

    if (fu)
    {
        for (y=0; y<ginfo->height; ++y)
        {
            s = (unsigned char *)source + y*rowbytes;
            for (x=0; x<ginfo->width; ++x)
            {
                if (ginfo->bpp==32)
                {
                    fuaddhist(fu,&ginfo1,(const ARGB *) s);
                    s += 4;
                }
                else
                {
                    fuaddhist(fu,&ginfo1,&ginfo->colourtbl[*s]);
                    s += 1;
                }
            }
        }

        ok = 1;
    }
    return(ok);
}

ARGB *GCALL FU_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    int i;
    FU *fu = (FU *)(crx->cref);
    ARGB *colourtbl;
    int realcolours=0;
    int slotsused=0;
    int slotlast=0;
    int slotcolours[MAXFUCUBES];
    int sumslotcolours=0;

    colourtbl = (ARGB *) galloc(256*sizeof(ARGB));

    // count slots
    for (i=0; i<MAXFUCUBES; ++i)
    {
        FUCUBE *fucube=&fu->fucube[i];
        slotcolours[i] = 0;
        if (slotsused>=numcolours)
            fucube->count = 0;
        if (fucube->count) ++slotsused, slotlast = i;
    }
    if (!slotsused) slotsused=1;

    // divide up slots, scaling by alpha
    for (i=0; i<MAXFUCUBES; ++i)
    {
        if (fu->fucube[i].count)
        {
            slotcolours[i] = gmax(1,numcolours/slotsused * (i+MAXFUCUBES));
            if (i==slotlast)
                slotcolours[i] *= 3;
            sumslotcolours += slotcolours[i];
        }
    }

    // normalize
    if (sumslotcolours)
    {
        for (i=0; i<MAXFUCUBES; ++i)
        {
            if (fu->fucube[i].count)
            {
                slotcolours[i] = slotcolours[i] * numcolours/sumslotcolours;
            }
        }
    }

    for (i=0; i<MAXFUCUBES; ++i)
    {
        FUCUBE *fucube=&fu->fucube[i];
        if (FUverbose)
            printf("FU alpha slot %d count %d ",i,fucube->count);
        if (fucube->wu)
        {
            int medianalpha = FU_computemedianalpha(i,fu->alphacount);
            ARGB *nextpal;
            int nextcolours=slotcolours[i];
            int nextgotcolours=0;
            int extracolours=0;
            int j;
            if (i==slotlast)
                nextcolours = numcolours-realcolours;

            if (nextcolours)
                nextpal = WU_close(fucube->wu,nextcolours, NULL, &nextgotcolours);
            else
            {
                nextpal = WU_close(fucube->wu,1, NULL, &nextgotcolours);
                nextgotcolours = 0;
            }

            // redistribute any extra evenly
            j = MAXFUCUBES-1;
            extracolours = nextcolours-nextgotcolours;

            while (extracolours>0)
            {
                if (slotcolours[j])
                  ++slotcolours[j];
                --extracolours;
                --j;
                if (j<=i)
                    j = MAXFUCUBES-1;
            }

            for (j=0; j<nextgotcolours; ++j)
                nextpal[j].a = (unsigned char) medianalpha;
            memcpy(&colourtbl[realcolours],nextpal,nextgotcolours*sizeof(ARGB));
            gfree(nextpal);

            if (FUverbose)
                printf("alpha %d want %d, got count %d ",medianalpha, nextcolours, nextgotcolours);

            realcolours += nextgotcolours;

        }
        if (FUverbose)
           printf("\n");
    }

    gfree(fu);
    gfree(crx);
    if (returncolours)
        *returncolours = realcolours;
    return((ARGB *)colourtbl);
}

