// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 CPair - Colour Pair Clustering                   */
/*                                                                  */
/*                      by Ian Davids, EAC                          */
/*                                                                  */
/*                    Main Module - June 11, 2003                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 030611 ID Uses pairing and error measure to build clut   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "cpair.h"
#include <string.h>
#include <stdlib.h>


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



typedef struct Pair
{
    float error;
} Pair;

typedef struct XHIST
{
    unsigned int argb;
    int    count;
    float error;
        // Rows when the XHIST becomes a matrix.
    Pair * RowList;
    int numinrow;
    int minIndex;
    int memlen;
    float minError;
} XHIST;

const int HASHSIZE=50000;

typedef struct XACT
{
    int numcolours;
    int histcolours;
    int histsize;
    int hashmin[HASHSIZE];
    int hashmax[HASHSIZE];
    int minIndexI;
    int minIndexJ;
    float minError;
    XHIST *hist;
} XACT;

typedef struct Cpair
{
    Pair *  minPair;
    int     minPairI;
    int     minPairJ;
    XHIST * newColour;
}Cpair;

// ------------------------------------------------------------
static void delete_xhist(XHIST * x,int count)
{
    for(int i = 0; i < count; i++)
    {
        if(x[i].memlen > 0)
            gfree(x[i].RowList);
    }
}
// ------------------------------------------------------------
static int hashfunction(unsigned int p)
{
    return (((p>>24)+100)+(p>>16)+(p>>8)+p) % HASHSIZE;
}
// ------------------------------------------------------------
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
        memset(hist,0,xact->histsize*sizeof(XHIST));
        if (xact->hist)
        {
            memcpy(hist,xact->hist,i*sizeof(XHIST));
            gfree(xact->hist);
        }
        xact->hist = hist;
    }

    // add new colour
    xact->hist[i].argb  = p;
    xact->hist[i].count = newcount;
    xact->hist[i].error = 0;
    xact->histcolours = i+1;
}
// ------------------------------------------------------------
static int xhistcmp(const void *h1, const void *h2)
{
    return((*(XHIST **)h2)->count - (*(XHIST **)h1)->count);
}
// ------------------------------------------------------------
// j is the root index of the other colour where I should
// get the colour from ie i + linked_list_index + 1
static inline void create_pair(XACT * xact, Pair * p , int i , int j)
{
    int fi = xact->hist[i].count;
    int fj = xact->hist[j].count;
    ARGB ci = *((ARGB *)(&(xact->hist[i].argb)));
    ARGB cj = *((ARGB *)(&(xact->hist[j].argb)));
        // build the pair values.
//    p->x          = i;
//    p->y          = j;
//    p->totalCount = fi + fj;
//    p->a_accume   = fi*ci.a + fj*cj.a;
//    p->r_accume   = fi*ci.r + fj*cj.r;
//    p->g_accume   = fi*ci.g + fj*cj.g;
//    p->b_accume   = fi*ci.b + fj*cj.b;
    p->error      = (float)((fj*fi)/(fj+fi)) * (float)(square_table[ci.r - cj.r + 256]+ square_table[ci.g - cj.g + 256]+
                                                       square_table[ci.b - cj.b + 256]+ square_table[ci.a - cj.a + 256]);
}
/*
// ------------------------------------------------------------
// j is the index in the linked list where I should add this item
static inline void insert_pair(XACT * xact, Pair * p, int i, int j)
{
    XHIST * t = &(xact->hist[i]);
    if(t->RowList == NULL || t->memlen <= j)
    {
        Pair * temp = (Pair *)galloc(sizeof(Pair)*t->memlen+10);
        t->memlen += 10;
        memcpy(temp,t->RowList,sizeof(Pair)*t->numinrow);
        gfree(t->RowList);
        t->RowList = temp;
    }


    Pair * previous = xact->hist[i].RowList;

    for(int c = 1; c < j; c++)
    {
        previous = previous->next;
    }
    if(previous->next == NULL)
        previous->next = p;
    else
    {
        Pair * next    = previous->next;
        previous->next = p;
        p->next        = next;
    }
    t->numinrow++;
    if(p->error < t->minError)
    {
        t->minError = p->error;
        t->minIndex = j;
    }
}
*/
// ------------------------------------------------------------
static inline void eval_min(XACT * xact,int i)
{
    XHIST * t = &(xact->hist[i]);
    if(t->minError < xact->minError)
    {
        xact->minError  = t->minError;
        xact->minIndexI = i;
        xact->minIndexJ = t->minIndex;
    }
}
// ------------------------------------------------------------
static void build_matrix(XACT * xact)
{
    xact->minError  = 999999999.0f;
    xact->minIndexI = 0;
    xact->minIndexJ = 0;

    for(int i = 0; i < xact->histcolours-1; i++)
    {
        xact->hist[i].RowList   = (Pair *)galloc(sizeof(Pair)*(xact->histcolours-i-1));
        xact->hist[i].memlen    = xact->histcolours-i-1;
        xact->hist[i].numinrow  = xact->histcolours-i-1;
        for(int j = i+1; j < xact->histcolours; j++)
        {
            create_pair(xact,&(xact->hist[i].RowList[j-i-1]),i,j);
            eval_min(xact,i);
        }
    }
}
// ------------------------------------------------------------
static inline void select_colourpair(XACT * xact,Cpair * X)
{
    XHIST * t = &(xact->hist[xact->minIndexI]);
        // get the minimum pair from the matrix;
    X->minPair  = &(t->RowList[xact->minIndexJ]);
    X->minPairI = xact->minIndexI;
    X->minPairJ = xact->minIndexJ;  // note this is the linked list index and not the real item in the root list
}
// ------------------------------------------------------------
static inline void merge_colour_pair(XACT * xact,Cpair * X)
{

    int rj = xact->minIndexI + xact->minIndexJ + 1;  // have to use the real index j and not the linked list index.
        // Calculate the new colour entry from the old pair information
    ARGB nc;
    int fi       = xact->hist[xact->minIndexI].count;
    int fj       = xact->hist[rj].count;
    ARGB ci      = *((ARGB *)(&(xact->hist[xact->minIndexI].argb)));
    ARGB cj      = *((ARGB *)(&(xact->hist[rj].argb)));
    float ei    = xact->hist[xact->minIndexI].error;
    float ej    = xact->hist[rj].error;
        // precompute the coefficients for amalgamated colour computation
    float finv  = 1/(float)(fi+fj);
    float coi   = fi*finv;
    float coj   = fj*finv;
        // compute the amalgamated colour using frequency information
    nc.a         = (unsigned char)(coi*ci.a + coj*cj.a);
    nc.r         = (unsigned char)(coi*ci.r + coj*cj.r);
    nc.g         = (unsigned char)(coi*ci.g + coj*cj.g);
    nc.b         = (unsigned char)(coi*ci.b + coj*cj.b);
    X->newColour = (XHIST *)galloc(sizeof(XHIST));
    XHIST * t    = X->newColour;
    t->argb      = *((unsigned int *)(&nc));
    t->count     = fi+fj;
    t->error     = X->minPair->error + ei + ej;
    t->RowList   = NULL;
    t->numinrow  = 0;
    t->minIndex  = 0;
    t->minError  = 0;
    t->memlen    = 0;
}
// ------------------------------------------------------------
static inline void delete_matrix_item(XACT *xact,int i)
{
    XHIST * t = &(xact->hist[i]);
    gfree(t->RowList);

        // do a pushback on this item
    memcpy(&(xact->hist[i]),&(xact->hist[i+1]),sizeof(XHIST)*(xact->histcolours-1-i));

        // keep track of the fact we have one less column
    xact->histcolours--;

        // delete the rows with this this item.
    for(int m = 0; m < xact->histcolours; m++)
    {
        int fj = i-m-1; // we need the list index of this item

        t = &(xact->hist[m]);
        if(t->numinrow > fj && fj >= 0)
        {
            memcpy(&(t->RowList[fj]),&(t->RowList[fj+1]),sizeof(Pair)*t->numinrow-fj-1);
            t->numinrow--;
        }
        else
            break;
    }
}
// ------------------------------------------------------------
static inline void recalc_min(XHIST * x)
{
    int i = 0;
    x->minError = 999999999.0f;
    for(int i = 0; i < x->numinrow; i++)
    {
        if(x->RowList[i].error < x->minError)
        {
            x->minError = x->RowList[i].error;
            x->minIndex = i;
        }
    }
}
// ------------------------------------------------------------
static inline void add_item(XACT * xact,Cpair * X)
{
    int end = xact->histcolours;
        // add new col to end of matrix
    xact->hist[end] = *(X->newColour);
    xact->histcolours++;
        // cleanup the Cpair structure
    gfree(X->newColour);
        // add new pair to each row
    for(int i = 0; i < end; i++)
    {
        XHIST * t = &(xact->hist[i]);
            // as I add the new item recalculate the min of that col
        if(t->memlen == t->numinrow)
        {
            Pair * temp = (Pair *)galloc(sizeof(Pair)*(t->numinrow+(xact->histcolours-255)+5));
            memcpy(temp,t->RowList,sizeof(Pair)*t->numinrow);
            if(t->RowList && t->memlen > 0)
                gfree(t->RowList);
            t->RowList = temp;
            t->memlen += (xact->histcolours-255)+1;
        }
        create_pair(xact,&(t->RowList[end-i-1]),i,end);
        if(t->minIndex == X->minPairI || t->minIndex == X->minPairJ)
            recalc_min(t);
            //insert_pair(xact,p,i,end-i-1);  // note need linked list index for j so convert
    }
        // recalculate the min of the overall matrix
    xact->minError = 999999999.0f;
    for(int i = 0; i < end; i++)
        eval_min(xact,i);
}
// ------------------------------------------------------------
static inline void replace_old_pairs(XACT * xact,Cpair * X)
{
        // First delete the two old colour entries
    delete_matrix_item(xact,X->minPairI);
    delete_matrix_item(xact,X->minPairJ+X->minPairI+1);  // convert to root index from linked list index
        // Add the new colour, colour pair
    add_item(xact,X);
}
// ------------------------------------------------------------
static int reduce_colourcount(XACT * xact,int colourcount)
{
    Cpair X;
    while(xact->histcolours > colourcount)
    {
            // find the colour pair with the minimum error
        select_colourpair(xact,&X);
            // take that colour pair and use the original colours to create
            // a new colour entry.
        merge_colour_pair(xact,&X);
            // add the new entry to the matrix and recompute the min entry
        replace_old_pairs(xact,&X);
    }
    return xact->histcolours;
}
// ------------------------------------------------------------
static ARGB * build_palette(XACT * xact,int numcolours,int realcolours)
{
        // we assume that numcolours is >= xact->histsize
    unsigned int * clut = (unsigned int *)galloc(sizeof(ARGB)*numcolours);
    int j = 0;
    for(int i = 0; i < xact->histcolours; i++)
        clut[i] = xact->hist[i].argb;

    return ((ARGB *)clut);
}

/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRABOUT *GCALL CPAIR_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('C','P','A','R');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       0;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "CPair");
        strcpy(about->longnamestr,  "Colour Pair Clustering");
    }
    return(about);
}

CRINSTANCE *GCALL CPAIR_open(void)
{
    CRINSTANCE *crx=0;
    square_init();
    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        XACT *xref = (XACT *) galloc(sizeof(XACT));
        if (xref)
        {
            memset(xref, 0, sizeof(XACT));
            crx->signature  = GMAKEID('C','P','A','R');
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


int GCALL CPAIR_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int x;
    int y;
    int ok=0;
    unsigned char *s;
    unsigned int v;
    XACT *xact = (XACT *)(crx->cref);

    if (xact)
    {
            //for (i=0; i<256; ++i)
            //xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[i]),0);

        for (y=0; y<ginfo->height; ++y)
        {
            s = (unsigned char *)source + y*rowbytes;
            if (ginfo->bpp==32)
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    v = *((unsigned int *) s);
                    v &= 0x00F8FCF8;
                    v |= 0xFF000000;
                    xaddhist(xact,v,1);
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

ARGB *GCALL CPAIR_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{

    int realcolours         = 0;
    ARGB *colourtbl         = 0;
    XACT *xact = (XACT *)(crx->cref);

    if (xact)
    {
        if (xact->hist)
        {

            build_matrix(xact);
            realcolours = reduce_colourcount(xact,numcolours);
            colourtbl = build_palette(xact,numcolours,realcolours);
            delete_xhist(xact->hist,xact->histcolours);
            if (xact)       gfree(xact);
            if (crx)        gfree(crx);
            if (returncolours)
                *returncolours = realcolours;
        }
    }
    return((ARGB *)colourtbl);
}


