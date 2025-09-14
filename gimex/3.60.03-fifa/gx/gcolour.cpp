// (c) 1995-2004 Electronic Arts Inc.

/*
references
Floyd Stucci colour reduction
http://vbaccelerator.com/codelib/gfx/clrman2.htm
*/

/****************************************************************/
/*  Support Functions                                           */
/****************************************************************/

#define GFILTER_STATIC 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gimex.h"
#include "gcolour.h"
#include "crapi.h"
#include "copen.h"
//#include "wu.h"
#include "gxopt.h"
#include "invtable.c"

/****************************************************************/
/*  Colour Match Function                                       */
/****************************************************************/

ARGB *gfindpal=0;
int gmaxcolours=256;
CRINVERSETABLE *ginversetbl;
//extern int OPTquantmethod;
//extern int OPTdither;
//extern int OPTverbose;

#if 1
unsigned char GIMEX_destcolourmask[256]=
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};
#endif

#define SQR(x) (x)*(x)

#define dstatic

#if 0
dstatic int gxgetcolour(ARGB *rgbval)
{
    int i, n, BestDiff, best;
    ARGB *clr;
    unsigned int finda = (int) rgbval->a;
    unsigned int findr = (int) rgbval->r;
    unsigned int findg = (int) rgbval->g;
    unsigned int findb = (int) rgbval->b;
    unsigned int ind;
    unsigned int inr, ing, inb;

    if (ginversetbl->table)
    {
        inr=(findr>>(8-ginversetbl->redbits));
        ing=(findg>>(8-ginversetbl->greenbits));
        inb=(findb>>(8-ginversetbl->bluebits));

        ind= inr*(1<<ginversetbl->redbits)*(1<<ginversetbl->redbits) + ing*(1<<ginversetbl->greenbits) + inb;

        return(ginversetbl->table[ind]);
    }

    /* full search for best colour */

    BestDiff = 0x7fffffff;
    n = gmaxcolours;
    for (i = 0, clr = gfindpal; (i < n) && (BestDiff != 0); ++i, ++clr)
    {
        int a = (int)clr->a;
        int r = (int)clr->r;
        int g = (int)clr->g;
        int b = (int)clr->b;
        int Diff = SQR((finda-a)*4)+SQR(findr-r)+SQR(findg-g)+SQR(findb-b);
        if (destcolourmask[i]>0 && Diff<BestDiff)
        {
            BestDiff = Diff;
            best = i;
        }
    }

    return(best);
}
#endif

void gxinitinversetable(ARGB *pal)
{
    unsigned char *tbl = NULL;
	int WuBits = 6; // hardcoded to match Wu's inversetable specs

    if (ginversetbl)
		gfree(ginversetbl);
    gfindpal = pal;

/*	ginversetbl = INVERSETABLE_create(pal->a, pal->r, pal->g, pal->b, "INVG");  \\ from inversetable.c in crapi */
	ginversetbl = (CRINVERSETABLE *) galloc(sizeof(CRINVERSETABLE) + sizeof(unsigned char)*(1<<WuBits)*(1<<WuBits)*(1<<WuBits));
	ginversetbl->signature = GMAKEID('I','N','V','G');  /* G for general? */
	ginversetbl->size      = sizeof(CRINVERSETABLE) + sizeof(unsigned char)*(1<<WuBits)*(1<<WuBits)*(1<<WuBits);
	ginversetbl->alphabits = (unsigned char) 0;
	ginversetbl->redbits   = (unsigned char) WuBits;
	ginversetbl->greenbits = (unsigned char) WuBits;
	ginversetbl->bluebits  = (unsigned char) WuBits;

	tbl = (unsigned char *)galloc((1<<ginversetbl->alphabits)*(1<<ginversetbl->redbits)*(1<<ginversetbl->greenbits)*(1<<ginversetbl->bluebits));
    if (tbl)
    {
		/* using ginversetbl->redbits because inversetable used to be for WU only and WUBITS == redbits == greenbits == bluebits */
        unsigned int* temp_buf = (unsigned int*)galloc((1<<ginversetbl->redbits*3)*sizeof(unsigned int));
        inv_cmap(0,gmaxcolours, gfindpal, (unsigned int)ginversetbl->redbits, temp_buf, tbl);
        gfree(temp_buf);
    }

	memcpy((unsigned char*)ginversetbl + sizeof(CRINVERSETABLE), tbl, (1<<ginversetbl->alphabits)*(1<<ginversetbl->redbits)*(1<<ginversetbl->greenbits)*(1<<ginversetbl->bluebits));
	gfree(tbl);
}


void gxfreeinversetable(void)
{
   	if (ginversetbl)
		gfree(ginversetbl);
    ginversetbl = 0;
}


/****************************************************************/
/*  8 to 32 bit                                                 */
/****************************************************************/

GINFO *info8to32(GINFO *gin, char *source, int rowbytes, int bpp)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memcpy((void *) ginfo, (void *) gin, sizeof(GINFO));

        ginfo->signature        = GMAKEID('C','O','L','R'); /* Type of ginfo */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
        ginfo->width            = gin->width;               /* Width of bitmap in pixels */
        ginfo->height           = gin->height;              /* Height of bitmap in pixels */
        ginfo->subtype          = 0;                        /* subtype */
        ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
        ginfo->bpp              = 32;                       /* Bits per pixel (8 or 32) */
        ginfo->numcolours       = 0;                        /* Number of colours in original indexed palette */
        ginfo->originalbpp      = bpp;
        ginfo->redbits          = gmin(8,(bpp+1)/3);
        ginfo->greenbits        = gmin(8,(bpp+2)/3);
        ginfo->bluebits         = gmin(8,(bpp+0)/3);
        ginfo->alphabits        = gmin(8,gmax(0,bpp-24));
    }
    return(ginfo);
}

int filter8to32(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes, int bpp)
{
    int width;
    int height;
    int x;
    int y;
    int ok=1;
    unsigned char *s;
    ARGB *d;

    width = ginfo->width;
    height = ginfo->height;
    for (y=0; y<height; ++y)
    {
        s = (unsigned char *) source + y*rowbytes;
        d = (ARGB *) (dest + y*destrowbytes);
        for (x=0; x<width; ++x)
        {
            *d++ = ginfo->colourtbl[(int) (*s++)];
        }
    }

    return(ok);
}

/****************************************************************/
/*  Dither 8 to 32 bit                                          */
/****************************************************************/

GINFO *infodither32(GINFO *gin, char *source, int rowbytes, int alphabits, int redbits, int greenbits, int bluebits)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memcpy((void *) ginfo, (void *) gin, sizeof(GINFO));

        ginfo->signature        = GMAKEID('D','I','T','H'); /* Type of ginfo */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
        ginfo->width            = gin->width;               /* Width of bitmap in pixels */
        ginfo->height           = gin->height;              /* Height of bitmap in pixels */
        ginfo->subtype          = 0;                        /* subtype */
        ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
        ginfo->bpp              = 32;                       /* Bits per pixel (8 or 32) */
        ginfo->alphabits        = alphabits;                /* Number of bits in alpha channel */
        ginfo->redbits          = redbits;                  /* Number of bits in red channel */
        ginfo->greenbits        = greenbits;                /* Number of bits in green channel */
        ginfo->bluebits         = bluebits;                 /* Number of bits in blue channel */
        ginfo->numcolours       = 0;                        /* Number of colours in original indexed palette */
    }
    return(ginfo);
}

dstatic void gseed(unsigned int seed)
{
    srand(seed);
}

dstatic unsigned int grandom(void)
{
    return(rand()>>8);
}

dstatic int gdith2(int v, int m)
{
    v = (v&~(m-1)) + (grandom()&(m-1));
    return(v);
}

dstatic int gdith(int v, unsigned int m)
{
    int r = v;

    if (m<128)
    {
        r = gmax(0,(int)(v-m/2));
        if ((grandom()&(m-1)) < (r&((m)-1)))
            r += m;
    }

    r = gmax(0,gmin(255,r));

    return(r);
}

int filterdither32(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes,int dither)
{
    int width;
    int height;
    int x;
    int y;
    int a;
    int r;
    int g;
    int b;
    int am;
    int rm;
    int gm;
    int bm;
    int ab;
    int rb;
    int gb;
    int bb;
    int ok=1;
    ARGB *s;
    ARGB *d;

    ab = 8-ginfo->alphabits;
    rb = 8-ginfo->redbits;
    gb = 8-ginfo->greenbits;
    bb = 8-ginfo->bluebits;

    am = 1<<ginfo->alphabits;
    rm = 1<<ginfo->redbits;
    gm = 1<<ginfo->greenbits;
    bm = 1<<ginfo->bluebits;
    if (!ginfo->alphabits)
        am = 1;

    width = ginfo->width;
    height = ginfo->height;
    gseed(0xfb123456);
    for (y=0; y<height; ++y)
    {
        s = (ARGB *) (source + y*rowbytes);
        d = (ARGB *) (dest + y*destrowbytes);

        for (x=0; x<width; ++x, ++s, ++d)
        {
            a = s->a;
            r = s->r;
            g = s->g;
            b = s->b;

            switch (dither)
            {
                default:
                case -1:
                case 0:
                    if (am>1) a = gmin(255,((a>>ab) * 255 + (am/2-1))/(am-1));
                    if (rm>1) r = gmin(255,((r>>rb) * 255 + (rm/2-1))/(rm-1));
                    if (gm>1) g = gmin(255,((g>>gb) * 255 + (gm/2-1))/(gm-1));
                    if (bm>1) b = gmin(255,((b>>bb) * 255 + (bm/2-1))/(bm-1));
                    break;

                case 1:
                case 2:
                    a = gdith(a,1<<ab);
                    r = gdith(r,1<<rb);
                    g = gdith(g,1<<gb);
                    b = gdith(b,1<<bb);
//                    if (am>1) a = gmin(255,((a>>ab) * 255 + (am/2-1))/(am-1));
//                    if (rm>1) r = gmin(255,((r>>rb) * 255 + (rm/2-1))/(rm-1));
//                    if (gm>1) g = gmin(255,((g>>gb) * 255 + (gm/2-1))/(gm-1));
//                    if (bm>1) b = gmin(255,((b>>bb) * 255 + (bm/2-1))/(bm-1));
                    break;

//                case 2:
//                    a = gdith2(a,am);
//                    r = gdith2(r,rm);
//                    g = gdith2(g,gm);
//                    b = gdith2(b,bm);
                    break;
            }

            d->a = (unsigned char) a;
            d->r = (unsigned char) r;
            d->g = (unsigned char) g;
            d->b = (unsigned char) b;
        }
    }

    return(ok);
}

/****************************************************************/
/*  32 to 8 bit                                                 */
/****************************************************************/

CRINSTANCE *CRx = NULL;
int CRpasscount = -1;
int CRonlyonepass=0;
ARGB CRscolourtbl[256];

void CR_reset(void)
{
    if (CRx)
    {
        ARGB *colourtbl = 0;

        if (OPTverbose)
            printf("gx - WARNING: COLOUR REDUCTION RESOURCE LEAK\n");
		if (ginversetbl)
			gfree(ginversetbl);
		ginversetbl = 0;

        colourtbl = CRAPI_close(CRx, 256, NULL, NULL);
        if (colourtbl)
        {
            gfree(colourtbl);
        }
        CRx = 0;
    }
    CRpasscount = -1;
    CRonlyonepass = 0;
}

GINFO *info32to8(GINFO *gin, int numcolours, char *source, int rowbytes, int bpp, int pass,int OPTglobalpalette)
{
    GINFO *ginfo=0;

    if (( CRpasscount == -1) || CRonlyonepass || !OPTglobalpalette)
    {
        if (pass == 1)
        {
            /* This is the first call but it is in pass 1 so we can assume only 1 pass */
            CRonlyonepass = 1;
        }

        /* First call so start colour reduction */
        if (numcolours)
            CRx = CRAPI_open();

        CRpasscount = 0;
    }

    {
        CRABOUT *about=0;

        about = CRAPI_about();
        if (about)
        {
            {
                static int warncrapiversion=0;
                if (about->version<160 && !warncrapiversion && OPTverbose)
                {
	    			printf("gx - WARNING '%s' CRAPI IS VERSION %.2f. GX WANTS 1.60\n",about->longnamestr,(float)about->version/100.0f);
                    warncrapiversion = 1;
                }
            }

            if (CRx)
            {
                static int warncrapimismatch=0;
                if (CRx->signature!=about->signature && !warncrapimismatch && OPTverbose)
                {
    	    		printf("gx - INTERNAL ERROR.  CRAPI MODULE MISMATCH.\nHAVE HANDLE %lx, ABOUT HANDLE %lx\n",CRx->signature,about->signature);
                    warncrapimismatch = 1;
                }
            }

            gfree(about);
        }
    }


    /* Image in pass 0 */
    if ( ((CRpasscount == 0) && (pass == 0)) || CRonlyonepass )
    {
        if (CRx)
            CRAPI_read(CRx, gin, source, rowbytes);
    }

    /* First time through pass 1 */
    if ( ((CRpasscount == 0) && (pass == 1)) || CRonlyonepass || !OPTglobalpalette)
    {
        ARGB *colourtbl = 0;

        if (CRx)
        {
            /* Save colour table */
			if (ginversetbl)
				gfree(ginversetbl);
			ginversetbl = 0;

            colourtbl = CRAPI_close(CRx, numcolours, &ginversetbl, &numcolours);
            if (colourtbl)
            {
                memcpy(CRscolourtbl, colourtbl, numcolours * sizeof(ARGB));
                gfree(colourtbl);
            }
            CRx = 0;
        }
        CRpasscount = 1;
    }

    /* Image in pass 1 */
    if ( (CRpasscount == 1) || CRonlyonepass || !OPTglobalpalette)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {

            memcpy((void *) ginfo, (void *) gin, sizeof(GINFO));
            memset(ginfo->colourtbl,0,256*sizeof(ARGB));

            ginfo->signature        = GMAKEID('C','O','L','R'); /* Type of ginfo */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
            ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours       = numcolours;

            if (numcolours)
            {
                ginfo->numcolours = numcolours;

                memcpy(ginfo->colourtbl,CRscolourtbl,numcolours*sizeof(ARGB));
            }
        }
    }

    return(ginfo);
}

int filter32to8(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes, int bpp, unsigned char *colourmask)
{
    int ok=1;
    int dither=OPTdither;

    if (dither<0) dither = 1;

	CRAPI_remap(dest, ginfo, ginfo->width*ginfo->bpp/8, source, gin->width*gin->bpp/8, gin->bpp, colourmask, ginversetbl, dither);

    return(ok);
}

/****************************************************************/
/*  NEW GColour Functions                                       */
/****************************************************************/

dstatic void scantrans32(int *alphahistogram,ARGB *dest,ARGB *chromacolour, int *chromacount, int *transparentcount, int n)
{
    int i;
    int tc=0;

    if (transparentcount)
        tc = *transparentcount;

    for (i=0; i<n; ++i)
    {
        if (!dest->a)
        {
            ++tc;
            if (*(int *)chromacolour != *(int *)dest)
            {
                ++*chromacount;
                *(int *)chromacolour = *(int *)dest;
            }
        }
        ++alphahistogram[dest->a];
        ++dest;
    }

    if (transparentcount)
        *transparentcount = tc;
}

dstatic void scantrans8(int *alphahistogram,char *dest,ARGB *colourtbl, ARGB *chromacolour, int *chromacount, int *transparentcount, int n)
{
    int i;
    int p;
    int tc=0;

    if (transparentcount)
        tc = *transparentcount;

    for (i=0; i<n; ++i)
    {
        p = (int) *(unsigned char *)dest;

        if (!colourtbl[p].a)
        {
            ++tc;
            if (*(int *)chromacolour != *(int *) &colourtbl[p])
            {
                ++*chromacount;
                *(int *)chromacolour = *(int *) &colourtbl[p];
            }
        }

        ++alphahistogram[colourtbl[p].a];
        ++dest;
    }

    if (transparentcount)
        *transparentcount = tc;
}


/* returns 2 or more if bitmap contains any transparent pixels */
/* note for 8 bit, actually scans used pixels, not the palette */

static void alphahistogram(GBITMAP *gbitmap, int *alphahistogram, ARGB *chromacolour, int *chromacount, int *transparentcount)
{
    int i;
    char *source = (char *) gbitmap->image;
    GINFO *ginfo = gbitmap->ginfo;

    memset(alphahistogram,0,256*sizeof(int));
    for (i=0; i<ginfo->height; ++i)
    {
        if (ginfo->bpp==8)
            scantrans8(alphahistogram,source,ginfo->colourtbl,chromacolour,chromacount,transparentcount,ginfo->width);
        else
            scantrans32(alphahistogram,(ARGB *)source,chromacolour,chromacount,transparentcount,ginfo->width);
        source += gbitmap->rowbytes;
    }
    return;
}

/* returns 1 if bitmap contains any transparent pixels alpha=0 */
/* returns 2 if bitmap contains any translucient pixels alpha=1..255 */

int imagetransparent(GBITMAP *gbitmap, ARGB *chromacolour, int *chromacount, int *transparentcount)
{
    int alphahist[256];
    int i;
    int lut=0;
    int PS2overalphaflow=0;

    *chromacount = 0;
    *(int *)chromacolour = 0xffffffff;
    if (transparentcount) *transparentcount = 0;

    alphahistogram(gbitmap, alphahist, chromacolour,chromacount, transparentcount);

    for (i=1; i<0xf0; ++i)
        lut += alphahist[i];
    for (i=129; i<256; ++i)
        PS2overalphaflow += alphahist[i];

    return((alphahist[0]?1:0) + (lut?2:0) + (PS2overalphaflow?4:0));
}


dstatic void scanindex8(int *indexhistogram,char *dest,int n)
{
    int i;
    int p;

    for (i=0; i<n; ++i)
    {
        p = (int) *(unsigned char *)dest;
        ++indexhistogram[p];
        ++dest;
    }
}

/* use to determine which clut entries are used */

void indexhistogram(GBITMAP *gbitmap, int *indexhistogram)
{
    int i;
    char *source = (char *) gbitmap->image;
    GINFO *ginfo = gbitmap->ginfo;

    memset(indexhistogram,0,256*sizeof(int));
    for (i=0; i<ginfo->height; ++i)
    {
        if (ginfo->bpp==8)
            scanindex8(indexhistogram,source,ginfo->width);
        source += gbitmap->rowbytes;
    }
    return;
}

