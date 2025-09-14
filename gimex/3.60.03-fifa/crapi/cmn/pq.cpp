// (c) 2003-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Pq - Pat's Quantizer CRapi Module  v1.00             */
/*                                                                  */
/*             by Patrick Ratto and Frank Barchard, EAC             */
/*                                                                  */
/*                    Main Module - Sep 16, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE Vectory                                        */
/* ------- ------ -- -------                                        */
/*    1.00 030916 FB based on exact and gst quant4                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* Pq is a clustering technique that partitions along the axis with */
/* most variance at a point that produces minimum variance after    */
/* the split along that axis.                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "pq.h"
#include "quantize.h"
#include "helper.h"
#include <string.h>
#include <stdlib.h>

#if 1

struct PQ
{
    int numcolours;
    int maxcolours;
    InputVector *vect;
};


/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRABOUT *GCALL PQ_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('P','A','T','Q');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       0;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Pat Ratto and Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "PQ");
        strcpy(about->longnamestr,  "Pat Quant");
    }
    return(about);
}

CRINSTANCE *GCALL PQ_open(void)
{
    CRINSTANCE *crx=0;

    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        PQ *xref = (PQ *) galloc(sizeof(PQ));
        if (xref)
        {
            memset(xref, 0, sizeof(PQ));
            crx->signature  = GMAKEID('P','A','T','Q');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)xref;
        }
        else
        {
            gfree(crx);
            crx = 0;
        }
    }
    return(crx);
}


int GCALL PQ_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int x;
    int y;
    int ok=0;
    PQ *pq = (PQ *)(crx->cref);

    if (pq)
    {
        // move image into InputVector

        int newcolours=ginfo->width*ginfo->height;
        InputVector *v;
        if (newcolours+pq->numcolours > pq->maxcolours)
        {
            const int ADDCOLOURS=8192;
            int maxcolours=gmax(newcolours,ADDCOLOURS);
            InputVector *newVect = (InputVector *)galloc((pq->maxcolours+maxcolours) * sizeof(InputVector));
            v = newVect;

            if (pq->vect)
            {
                memcpy(newVect,pq->vect,pq->numcolours*sizeof(InputVector));
                gfree(pq->vect);
                v = newVect+pq->numcolours;
                newcolours += pq->numcolours;
                maxcolours += pq->maxcolours;
            }
            pq->vect = newVect;
            pq->numcolours = newcolours;
            pq->maxcolours = maxcolours;
        }
        else
        {
            v = pq->vect+pq->numcolours;
            pq->numcolours += newcolours;
        }

//        for (i=0; i<256; ++i)
//            xaddvect(PQ,(*(unsigned int *)&ginfo->colourtbl[i]),0);

        for (y=0; y<ginfo->height; ++y)
        {
            unsigned char *s = (unsigned char *)source + y*rowbytes;
            if (ginfo->bpp==32)
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    const ARGB *p = (const ARGB *) s;
                    v->val[0] = p->r;
                    v->val[1] = p->g;
                    v->val[2] = p->b;
                    v->val[3] = p->a;
                    ++v;
                    s += 4;
                }
            }
            else
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    const ARGB *p = (const ARGB *) &ginfo->colourtbl[*s];
                    v->val[0] = p->r;
                    v->val[1] = p->g;
                    v->val[2] = p->b;
                    v->val[3] = p->a;
                    ++v;
                    s += 1;
                }
            }
        }

        ok = 1;
    }
    return(ok);
}

ARGB *GCALL PQ_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
//    int ok=0;
    int realcolours=0;
    ARGB *colourtbl=NULL;
    PQ *pq = (PQ *)(crx->cref);

    if (pq)
    {
        Cluster *clust = (Cluster*) galloc(numcolours*sizeof(Cluster));

        int clusters = Cluster4DVectors(pq->vect, pq->numcolours, clust, numcolours);

        realcolours = gmin(numcolours,clusters);

        colourtbl = (ARGB *) galloc(numcolours*sizeof(ARGB));
        if (colourtbl)
        {
            memset(colourtbl,0,numcolours*sizeof(ARGB));

            for (int i=0; i<realcolours; ++i)
            {
                colourtbl[i].r = clust[i].val[0];
                colourtbl[i].g = clust[i].val[1];
                colourtbl[i].b = clust[i].val[2];
                colourtbl[i].a = clust[i].val[3];
            }
//            ok = 1;
        }

        gfree(clust);
        if (pq->vect) gfree(pq->vect);
        if (pq)       gfree(pq);
        if (crx)      gfree(crx);
    }
    if (returncolours)
        *returncolours = realcolours;
    return((ARGB *)colourtbl);
}


#endif
