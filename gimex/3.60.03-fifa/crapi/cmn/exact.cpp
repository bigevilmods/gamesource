// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*      Exact - Exact Popular Histogram CRapi Module  v1.06         */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*                    Main Module - Mar 24, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 961118 FB gives most popular exact colours (lossless)    */
/*    1.01 961121 FB index sort to maintain original order          */
/*    1.02 000606 FB masked by bits per channel                     */
/*    1.03 001202 FB removed masking - 0 bits issues                */
/*    1.04 010417 FB only what is in image                          */
/*    1.05 020218 FB bpp,reverse,hash                               */
/*    1.06 030324 ID respectalpha                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* Exact keeps list of all colours used and if more than 256, gives */
/* most popular 256 (but exact, not blended)                        */
/* May return fewer colours than asked for, or may have wanted to   */
/* return more.  Both are indicated by a modified CRINSTANCE        */
/* numcolours field after readexact (but before closeexact) and     */
/* readexact returns 2 instead of 1.                                */
/* Adds the original palette (really for 8 bit, but does for 32 bit */
/* assuming colour table may still be available after 8 to 32       */
/* conversion)                                                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "exact.h"
#include <string.h>
#include <stdlib.h>

typedef struct
{
    unsigned int argb;
    int count;
} XHIST;

const int HASHSIZE=50000;

typedef struct
{
    int numcolours;
    int histcolours;
    int histsize;
    int hashmin[HASHSIZE];
    int hashmax[HASHSIZE];
    XHIST *hist;
} XACT;

static int hashfunction(unsigned int p)
{
    return (((p>>24)+100)+(p>>16)+(p>>8)+p) % HASHSIZE;
}

static void xaddhist(XACT *xact,unsigned int p, int newcount)
{
    int i;

    // find colour and count

    int hash=hashfunction(p);
    int hmin=xact->hashmin[hash];
    int hmax=xact->hashmax[hash];

    for (i=hmax; i>=hmin; --i)
    {
        if (xact->hist[i].argb==p)
        {
            xact->hist[i].count += newcount;
            return;
        }
    }

    // adding new colour, realloc histogram if full

    i = xact->histcolours;
    if (i<hmin)
        xact->hashmin[hash] = i;
    if (i>hmax)
        xact->hashmax[hash] = i;

    if (i>=xact->histsize)
    {
        XHIST *hist;

        xact->histsize += 16384;
        hist = (XHIST *) galloc(xact->histsize*sizeof(XHIST));
        if (xact->hist)
        {
            memcpy(hist,xact->hist,i*sizeof(XHIST));
            gfree(xact->hist);
        }
        xact->hist = hist;
    }

    // add new colour

    xact->hist[i].argb = p;
    xact->hist[i].count = newcount;
    xact->histcolours = i+1;
}

static int xhistcmp(const void *h1, const void *h2)
{
    return((*(XHIST **)h2)->count - (*(XHIST **)h1)->count);
}

/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRABOUT *GCALL EXACT_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('X','A','C','T');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       0;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.06");
        strcpy(about->namestr,      "Exact");
        strcpy(about->longnamestr,  "Exact Histogram");
    }
    return(about);
}

CRINSTANCE *GCALL EXACT_open(void)
{
    CRINSTANCE *crx=0;

    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        XACT *xref = (XACT *) galloc(sizeof(XACT));
        if (xref)
        {
            memset(xref, 0, sizeof(XACT));
            crx->signature  = GMAKEID('X','A','C','T');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)xref;
            for (int i=0; i<HASHSIZE; ++i)
            {
                xref->hashmax[i] = -1;
                xref->hashmin[i] = 0x7fffffff;
            }
        }
        else
        {
            gfree(crx);
            crx = 0;
        }
    }
    return(crx);
}


int GCALL EXACT_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int i;
    int x;
    int y;
    int ok=0;
    unsigned char *s;
    XACT *xact = (XACT *)(crx->cref);

    if (xact)
    {
        for (i=0; i<256; ++i)
            xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[i]),0);

        for (y=0; y<ginfo->height; ++y)
        {
            s = (unsigned char *)source + y*rowbytes;
            if (ginfo->bpp==32)
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    xaddhist(xact,(*(unsigned int *) s),1);
                    s += 4;
                }
            }
            else
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[*s]),1);
                    s += 1;
                }
            }
        }

        ok = 1;
    }
    return(ok);
}

ARGB *GCALL EXACT_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    int i;
    int j;
    int ok=0;
    int realcolours=0;
    unsigned int *colourtbl=0;
    XHIST **histindex;
    XACT *xact = (XACT *)(crx->cref);

    if (xact)
    {
        if (xact->hist)
        {
            histindex = (XHIST **) galloc(xact->histcolours*sizeof(XHIST *));
            if (histindex)
            {
                for (i=0; i<xact->histcolours; ++i)
                    histindex[i] = xact->hist+i;

                qsort(histindex,xact->histcolours,sizeof(XHIST *),xhistcmp);

                realcolours = gmin(numcolours,xact->histcolours);

                for (i=0; i<realcolours; ++i)
                    histindex[i]->count = -histindex[i]->count;

                colourtbl = (unsigned int *) galloc(numcolours*sizeof(ARGB));
                if (colourtbl)
                {
                    memset(colourtbl,0,numcolours*sizeof(ARGB));
                    for (i=0,j=0; i<xact->histcolours; ++i)
                        if (xact->hist[i].count<0)
                            colourtbl[j++] = xact->hist[i].argb;

                    ok = 1;
                }
                gfree(histindex);
            }
        }
        if (xact->hist) gfree(xact->hist);
        if (xact)       gfree(xact);
        if (crx)        gfree(crx);
    }
    if (returncolours)
        *returncolours = realcolours;
    return((ARGB *)colourtbl);
}


