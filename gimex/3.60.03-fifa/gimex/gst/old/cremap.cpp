// (c) 1995-2007 Electronic Arts Inc.

//#define FASTDISTANCE 1
//#include <stdlib.h>
//#include <minmax.h>

/*------------------------------------------------------------------*/
/*                                                                  */
/*                          CRAPI Remap					    		*/
/*                                                                  */
/*           by Frank Barchard, Gerry Shaw, Jeff Lefferts           */
/*                  Kaz Okuda & Lydia Chan, EAC                     */
/*                                                                  */
/*                          May 02, 2003                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 991008 LC crapi's version of remap						*/
/*    1.01 000506 FB stripped down, alpha fix, no copy or galloc	*/
/*    1.02 010801 FB alpha attenuation less severe                  */
/*    1.03 030304 FB getcolor added to eventually replace findcolor */
/*    1.04 030502 FB getcolor hash                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*		                                                            */
/* Notes		                                                    */
/* -------------                                                    */
/*                                                                  */
/* In a effort to make the crapi library more complete and          */
/* independent remap has been included in the crapi library	        */
/* build.                                                           */
/* The API for crapi remap is low level, not requiring any memory   */
/* management.  The user is responsible for allocation, and filling */
/* in all structures.  No error checking is performed.              */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "crapi.h"
#include <string.h>



namespace Gst
{
    
// mmx helps amd athlon mp, but hurts pentium 4
#define MMXOPTIMIZE 0


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

typedef struct
{
    int width, height;
    int xOffset;        /* never a Y offset */
    int parts;
    int cell[15];       /* Increase this number if larger dither matrix needed */
} CRAPIDITHER;

enum { DITHER_NONE, DITHER_FLOYD, DITHER_BURKES, DITHER_STUCKI };

/* Dither matrices (increase size of cell array in CRAPIDITHER above if adding larger matrix) */
static const CRAPIDITHER rmfloyd = {3, 2, -1, 16,                /* width, height, xOffet, parts */
{
    0, 0, 7,
    3, 5, 1,
}};

static const CRAPIDITHER rmburkes = {5, 2, -2, 32,               /* width, height, xOffet, parts */
{
    0, 0, 0, 8, 4,
    2, 4, 8, 4, 2,
}};

static const CRAPIDITHER rmstucki = {5, 3, -2, 42,               /* width, height, xOffet, parts */
{
    0, 0, 0, 8, 4,
    2, 4, 8, 4, 2,
    1, 2, 4, 2, 1,
}};

static const CRAPIDITHER *diffusionmatrices[4]=
{
    (CRAPIDITHER *)0,
    &rmfloyd,
    &rmburkes,
    &rmstucki,
};

/**************************************************************************/
/* Distribute color error to surrounding pixels (error diffusion dither). */
/**************************************************************************/

static void CRAPI_diffuseerror(ARGB *pixel, const ARGB *mappixel, int xPos, int yPos,
                              int width, int height, int rowbytes, const CRAPIDITHER *dither)
{
    int i;
    int j;
    int errorPct;
    int parts;
    int red;
    int green;
    int blue;
    int matrixW;
    int matrixH;
    const int *cell;
    int redError;
    int greenError;
    int blueError;
    ARGB *xPtr;
    ARGB *pixelPtr;

    /* Get color error between substituted color and mappixel color */
    redError   = (int) pixel->r - (int) mappixel->r;
    greenError = (int) pixel->g - (int) mappixel->g;
    blueError  = (int) pixel->b - (int) mappixel->b;

    // limit the amount of error
    if (redError>64)   redError   = 64;
    if (greenError>64) greenError = 64;
    if (blueError>64)  blueError  = 64;

    /* Point to upper left pixel of dither matrix */
    pixelPtr = pixel + dither->xOffset;

    /* Scan through the dither matrix */
    parts = dither->parts;
    xPos += dither->xOffset;
    matrixW = dither->width;
    matrixH = dither->height;

    cell = &dither->cell[0];

    for (i=0; (i<matrixH) && (yPos+i<height); ++i)
    {
        xPtr = pixelPtr;

        /* Do a line of the dither matrix */
        for (j=0; j<matrixW; ++j)
        {
            errorPct = *cell++;
            if (errorPct && (xPos+j)>=0 && (xPos+j<width))
            {
                /* Add in the proper error amount */
                if (redError<0) red = (int) gmax(0,xPtr->r + ((redError * errorPct-parts/2) / parts));
                else            red = (int) gmin(255,xPtr->r + ((redError * errorPct+parts/2) / parts));
                xPtr->r = (unsigned char)red;

                if (greenError<0) green = (int) gmax(0,xPtr->g + ((greenError * errorPct-parts/2) / parts));
                else              green = (int) gmin(255,xPtr->g + ((greenError * errorPct+parts/2) / parts));
                xPtr->g = (unsigned char)green;

                if (blueError<0) blue = (int) gmax(0,xPtr->b + ((blueError * errorPct-parts/2) / parts));
                else             blue = (int) gmin(255,xPtr->b + ((blueError * errorPct+parts/2) / parts));
                xPtr->b = (unsigned char)blue;
            }
            ++xPtr;                 /* point to next pixel */
        }
        pixelPtr = (ARGB *)((char *) pixelPtr + rowbytes);  /* point to next line */
    }
}

/******************************************************************************/
/* Find a palette index for a particular RGBA color.                          */
/******************************************************************************/
/* djw optimized for speed.                                                   */
/* (1) Look for previous color value; if so, return previous color index.     */
/* (2) Use a table of square in 0-255 range                                   */
/* With this, it's now 3-1/3 times faster than before, using my sample image. */
/******************************************************************************/

#define HUGEDIST 1000000

static __inline int scalealpha(int a)
{
    return(a/2+128);
}


int CRAPI_findcolour(const ARGB *palette, int numcolours, const unsigned char *mask, const ARGB *color)
{
    unsigned int closeDist=HUGEDIST;
    unsigned int dist;
    unsigned int multiplier = 4;
    int i;
    const ARGB *pal;
    const ARGB *closepal;

    ARGB col;
    col.a = color->a;                                   // preattenuate the colour.
    col.r = (unsigned char)((scalealpha(color->a)*color->r + 128)/255);
    col.g = (unsigned char)((scalealpha(color->a)*color->g + 128)/255);
    col.b = (unsigned char)((scalealpha(color->a)*color->b + 128)/255);

#if !MMXOPTIMIZE

    static unsigned int CRsquare[512];
    static unsigned int CRmonosquare[512];
    static int CRsquareinit=0;

    /* Fill up CRsquare table */
    if (!CRsquareinit)
    {
        for (i=0; i<512; ++i)
            CRsquare[i] = (i-256)*(i-256);

        for (i=0; i<512; ++i)
            CRmonosquare[i] = (i<256) ? (i-256)*(i-256):(i-256)*(i-256)*10;

        CRsquareinit = 1;
    }
#endif

    /* Check all palette entries for closest color */

    pal = closepal = palette;

    // don't make alpha as important all the time, if it is within
    // a critical range then we MUST match it, otherwise we can get a
    // semi good match in the alpha channel and be okay with that.

    if (col.a==0)   // transparent is very important to maintain at all costs to the colour channels
        multiplier = 4; // times 16;
    else if ((col.a<=16)
          || (col.a>=(128-16) && col.a<=(128+16))
          || (col.a>=240))
        multiplier = 2; // times 4;
    else
        multiplier = 1; // times 2

#if MMXOPTIMIZE
    __int64 mmxmultiplier;
    static const __int64 mmxmultiplier2 = 0x0002000100010001i64;
    static const __int64 mmxmultiplier4 = 0x0004000100010001i64;
    static const __int64 mmxwordmask = 0x000000000000ffffi64;

    if (multiplier==2)
        mmxmultiplier = mmxmultiplier2;
    else
        mmxmultiplier = mmxmultiplier4;
#endif

    for (i=0; i<numcolours; ++i,++pal)
    {
        /* distance = C(a1-a2)^2 + (r1-r2)^2 + (g1-g2)^2 + (b1-b2)^2 */

#if MMXOPTIMIZE
asm {
        // can move outside loop
        pxor   mm2,mm2
        movq   mm3,mmxmultiplier
        movq   mm4,mmxwordmask
        movd   mm5,col
        punpcklbw mm5,mm2

        mov    eax,pal
        movd   mm0,dword ptr [eax]

        punpcklbw mm0,mm2
        psubw  mm0,mm5
        pmullw mm0,mm0
        pmullw mm0,mm3

        pshufw mm1,mm0,0xe
        paddusw  mm1,mm0

        pshufw mm0,mm1,0x1
        paddusw  mm0,mm1
        pand     mm0,mm4
        movd   dist,mm0
}
#else
        if (numcolours<=2)
        {
            dist   = CRmonosquare[col.r - pal->r + 256]
                   + CRmonosquare[col.g - pal->g + 256]
                   + CRmonosquare[col.b - pal->b + 256]
                  + (CRmonosquare[col.a - pal->a + 256]<<multiplier);
        }
        else
        {
#ifdef FASTDISTANCE
			int da=abs(col.a - pal->a);
			int dr=abs(col.r - pal->r);
			int dg=abs(col.g - pal->g);
			int db=abs(col.b - pal->b);
			dist = (da+dr+dg+db);
			if (dist < closeDist)
			{
				dist += max(max(da,dr),max(dg,db));
				
			}			
#else
			dist   = CRsquare[col.r - pal->r + 256]
                   + CRsquare[col.g - pal->g + 256]
                   + CRsquare[col.b - pal->b + 256]
                  + (CRsquare[col.a - pal->a + 256]<<multiplier);
#endif
		}
#endif

        if (dist < closeDist)             // the version with no alpha weight
        {
            /* Check if the current colour can be used */
            if (mask)
                if (!mask[i])
                    continue;

            closeDist = dist;
            closepal = pal;
            if (!closeDist)
                break;
        }
    }
#if MMXOPTIMIZE
asm {
        emms
}
#endif

    /* Return the index of the best match */
    return((int)(closepal - palette));
}


// DJB2 hash
// http://www.cs.yorku.ca/~oz/hash.html
// NOTE require int to be 32 bits

static unsigned int hash(const unsigned char *str, int count, unsigned int hash = 5381)
{
	const unsigned char *s = (const unsigned char *) str;
	int c;

    while (count--)
	{
		c = *s++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

    return hash;
}

// flags for future alpha channel options

int CRAPI_getcolour(ARGB color, int numcolours, const ARGB *palette, int flags)
{
    unsigned int closeDist=HUGEDIST;
    unsigned int dist;
    unsigned int multiplier = 4;
    int i;
    const ARGB *pal;
    const ARGB *closepal;


    static unsigned int CRsquare[512];
    static unsigned int CRmonosquare[512];
    static int CRsquareinit=0;

    /* Fill up CRsquare table */
    if (!CRsquareinit)
    {
        for (i=0; i<512; ++i)
            CRsquare[i] = (i-256)*(i-256);

        for (i=0; i<512; ++i)
            CRmonosquare[i] = (i<256) ? (i-256)*(i-256):(i-256)*(i-256)*10;

        CRsquareinit = 1;
    }


    ARGB col;
    if(flags&GC_RESPECTALPHA)
    {
    	col.a = color.a;                                   // preattenuate the colour.
    	col.r = (unsigned char)((scalealpha(color.a)*color.r + 128)/255);
    	col.g = (unsigned char)((scalealpha(color.a)*color.g + 128)/255);
    	col.b = (unsigned char)((scalealpha(color.a)*color.b + 128)/255);
    }
    else // don't do preattenuation of the colour value
	{
	    col.a = color.a;
    	col.r = color.r;
    	col.g = color.g;
    	col.b = color.b;

	}    

    /* Check all palette entries for closest color */

    pal = closepal = palette;

    // don't make alpha as important all the time, if it is within
    // a critical range then we MUST match it, otherwise we can get a
    // semi good match in the alpha channel and be okay with that.
    if(flags&GC_RESPECTALPHA)
    {
    	if (col.a==0)   // transparent is very important to maintain at all costs to the colour channels
        	multiplier = 4; // times 16;
    	else if ((col.a<=16)
        	  || (col.a>=(128-16) && col.a<=(128+16))
          	  || (col.a>=240))
        	multiplier = 2; // times 4;
    	else
        	multiplier = 1; // times 2
    }
    else
        multiplier = 0;

    for (i=0; i<numcolours; ++i,++pal)
    {
        /* distance = C(a1-a2)^2 + (r1-r2)^2 + (g1-g2)^2 + (b1-b2)^2 */

        if (numcolours<=2)
        {
            dist   = CRmonosquare[col.r - pal->r + 256]
                   + CRmonosquare[col.g - pal->g + 256]
                   + CRmonosquare[col.b - pal->b + 256]
                  + (CRmonosquare[col.a - pal->a + 256]<<multiplier);
        }
        else
        {
			dist   = CRsquare[col.r - pal->r + 256]
                   + CRsquare[col.g - pal->g + 256]
                   + CRsquare[col.b - pal->b + 256]
                  + (CRsquare[col.a - pal->a + 256]<<multiplier);
        }

        if (dist < closeDist)             // the version with no alpha weight
        {

            closeDist = dist;
            closepal = pal;
            if (!closeDist)
                break;
        }
    }

	int bestIndex = (int)(closepal - palette);

    /* Return the index of the best match */
    return bestIndex;
}


/****************************************************************************************
; ABSTRACT
;
; CRAPI_remap - remaps an image to the specified palette
;
;
; SUMMARY
;
; #include "crapi.h"                Required for structure declarations
;
; int CRAPI_remap(destpixels, destginfo, destrowbytes, sourcepixels, sourcerowbytes, sourcebpp, colourmask, ginversetbl, OPTdither)
; void *destpixels				A pointer to address space of the destination image
; GINFO *destginfo				The destination ginfo structure
; int destrowbytes				The destination rowbytes
; const void *sourcepixels		A pointer to address space of the source image
; int sourcerowbytes 			The source ginfo structure
; int sourcebpp					The bpp of the source image
; unsigned char *colourmask		The colour mask for the palette
; CRINVERSETABLE *ginversetbl	The inversetable (if one was created)
; int OPTdither					The dither option.  (0=None, 1=Floyd, 2=Burkes, 3=Stucki)

;
;
; DESCRIPTION
;
; The CRAPI_remap function remaps the image to the new colour palette.
;
;
; RETURN VALUE
;
; none
;
;
; SEE ALSO
;
; CRAPI_close
;
;
; EXAMPLE
;
; colourreduce_eg.c
;
;
; END ABSTRACT
****************************************************************************************/

void remap(
    void *destpixels,
    const GINFO *destginfo,
    int destrowbytes,
    void *sourcepixels,
    int sourcerowbytes,
    int sourcebpp,
    const unsigned char *colourmask,
    int dither)
{
    int respectalpha   = (destginfo->alphabits > 0)?GC_RESPECTALPHA:GC_NONE;
    ARGB * PreattenPal = (ARGB *)galloc(sizeof(ARGB)*GIMEX_COLOURTBL_SIZE);

    if(respectalpha&GC_RESPECTALPHA)
    {
	    for (int i = 0; i < GIMEX_COLOURTBL_SIZE;i++)
	    {
	        PreattenPal[i].a = destginfo->colourtbl[i].a;
	        PreattenPal[i].r = (char)((scalealpha(destginfo->colourtbl[i].a)*destginfo->colourtbl[i].r + 128)/255);
	        PreattenPal[i].g = (char)((scalealpha(destginfo->colourtbl[i].a)*destginfo->colourtbl[i].g + 128)/255);
	        PreattenPal[i].b = (char)((scalealpha(destginfo->colourtbl[i].a)*destginfo->colourtbl[i].b + 128)/255);
	    }
	}
	else
	{
		for (int i = 0; i < GIMEX_COLOURTBL_SIZE;i++)
	    {
	        PreattenPal[i].a = destginfo->colourtbl[i].a;
	        PreattenPal[i].r = destginfo->colourtbl[i].r;
	        PreattenPal[i].g = destginfo->colourtbl[i].g;
	        PreattenPal[i].b = destginfo->colourtbl[i].b;
	    }
	}

    if (dither<0 || dither>3) dither=DITHER_FLOYD;

    for (int y=0; y<destginfo->height; ++y)
    {
        unsigned char *s = (unsigned char *) sourcepixels + sourcerowbytes*y;
        unsigned char *d = (unsigned char *) destpixels   + destrowbytes*y;

        if (sourcebpp<=8)
        {
            for (int x=0; x<destginfo->width; ++x)
            {
                *d = (unsigned char)Gst::CRAPI_getcolour(*((ARGB *)(destginfo->colourtbl+*s)), destginfo->numcolours,PreattenPal,respectalpha);

                // no dither - dither requires 32 bit source buffer to diffuse error into
                ++s;
                ++d;
            }
        }
        else
        {
            for (int x=0; x<destginfo->width; ++x)
            {
                *d = (unsigned char)Gst::CRAPI_getcolour(*((ARGB *)s), destginfo->numcolours,PreattenPal,respectalpha);

                if (dither)
                    CRAPI_diffuseerror((ARGB *)s, destginfo->colourtbl+*d, x, y, destginfo->width, destginfo->height, sourcerowbytes, diffusionmatrices[dither]);

                ++d;
                s += sizeof(ARGB);
            }
        }
    }
    if(PreattenPal)
        gfree(PreattenPal);
}
 
}
