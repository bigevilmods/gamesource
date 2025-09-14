// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                          Resample v1.04                          */
/*                                                                  */
/*                       by Steve Kilthau, EAC                      */
/*                                                                  */
/*                    Main program - May 02, 2001                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 980124 SK It resizes and changes bit depth of images.    */
/*    1.01 980407 KO Change CRAPI_read call to use new prototype.   */
/*    1.02 980618 FB CRAPI_open/CRAPI_close numcolours 1.30 API     */
/*    1.03 991022 LC CRINVERSETABLE								    */
/*    1.04 010402 FB fixed warnings								    */
/*                                                                  */
/*------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resample.h"
#include "gfilter.h"
#include "fopen.h"
#include "gimex.h"
#include "crapi.h"
#include "copen.h"

#define S(x) (x)

typedef struct
{
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} ARGBBits;

static GBITMAP *convertSize(GBITMAP *in, int width, int height);
static GBITMAP *convertDown(GBITMAP *gin, int bpp, ARGBBits *argb);
static GBITMAP *convertUp(GBITMAP *gin, int bpp, ARGBBits *argb);
static int intpow(int base, int exponent);
static void setARGBBits(GBITMAP *gb, int bpp, ARGBBits *argb);
static int getARGBBits(GINFO *ginfo, ARGBBits *argb);

/* Copied from gcolour.c */
static void gseed(unsigned int seed);
static unsigned int grandom(void);
static int gdith2(int v, int m);
static int gdith(int v, int m);
static int filterdither32(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes,int dither);

GBITMAP *resampleimage(GBITMAP *sourceimage, int width, int height, int bpp)
{
    GBITMAP *returnimage;
    int newbpp;
    int newwidth;
    int newheight;

    if (width < 0)
        width = 0;
    if (height < 0)
        height = 0;

    /* Determine width and height of image to be returned */
    if (width == 0 && height != 0)
    {
        newwidth = (int)((float)(sourceimage->ginfo->width)  * (float)(height) / (float)(sourceimage->ginfo->height) + 0.5);
        newheight = height;
    }
    else if (width != 0 && height == 0)
    {
        newwidth = width;
        newheight = (int)((float)(sourceimage->ginfo->height) * (float)(width) / (float)(sourceimage->ginfo->width) + 0.5);
    }
    else if (width != 0 && height != 0)
    {
        newwidth = width;
        newheight = height;
    }
    else
    {
        newwidth = sourceimage->ginfo->width;
        newheight = sourceimage->ginfo->height;
    }

    /* Determine bpp of image to be returned */
    if (bpp > 32)
        bpp = 32;
    if (bpp < 0)
        bpp = 0;
    
    if (bpp != 0)
        newbpp = bpp;
    else
        newbpp = sourceimage->ginfo->bpp;

    /* If changing bpp, or changing image size, then convert to 32 bit, then make changes, then reduce colour depth */
    if (newbpp != sourceimage->ginfo->originalbpp || sourceimage->ginfo->height != newheight || sourceimage->ginfo->width != newwidth)
    {
        GBITMAP *imageUp;
        GBITMAP *imageResized;
        ARGBBits argb;
        
        getARGBBits(sourceimage->ginfo, &argb);
        if (sourceimage->ginfo->height != newheight || sourceimage->ginfo->width != newwidth || sourceimage->ginfo->originalbpp <= 8)
        {
            int sumbits = argb.a + argb.r + argb.g + argb.b;
            /* If changing size or original bpp <= 8 then convert to 32bit format before resizing */
            if (sourceimage->ginfo->originalbpp <= 8)
            {
                imageUp = convertUp(sourceimage, sumbits, &argb);
                imageResized = convertSize(imageUp, newwidth, newheight);
                gfree(imageUp->ginfo);
                gfree(imageUp->image);
                gfree(imageUp);
            }
            else
            {
                imageResized = convertSize(sourceimage, newwidth, newheight);
            }

            if (imageResized->ginfo->originalbpp > newbpp)
            {
                /* If going to below 8bpp then respect original argb */
                if (newbpp <= 8)
                    returnimage = convertDown(imageResized, newbpp, &argb);
                else
                    returnimage = convertDown(imageResized, newbpp, (ARGBBits *)0);
            }
            else
            {
                if (newbpp <= 8)
                    returnimage = convertUp(imageResized, newbpp, &argb);
                else
                    returnimage = convertUp(imageResized, newbpp, (ARGBBits *)0);
            }
            gfree(imageResized->ginfo);
            gfree(imageResized->image);
            gfree(imageResized);
        }
        else
        {
            if (sourceimage->ginfo->originalbpp > newbpp)
            {
                if (newbpp <= 8)
                    returnimage = convertDown(sourceimage, newbpp, &argb);
                else
                    returnimage = convertDown(sourceimage, newbpp, (ARGBBits *)0);
            }
            else
            {
                if (newbpp <= 8)
                    returnimage = convertUp(sourceimage, newbpp, &argb);
                else
                    returnimage = convertUp(sourceimage, newbpp, (ARGBBits *)0);
            }
        }
    }
    else
    {
        returnimage = galloc(sizeof(GBITMAP));
        returnimage->ginfo = galloc(sourceimage->ginfo->size);
        memcpy(returnimage->ginfo, sourceimage->ginfo, sourceimage->ginfo->size);
        returnimage->image = galloc(sourceimage->rowbytes * sourceimage->ginfo->height);
        memcpy(returnimage->image, sourceimage->image, sourceimage->rowbytes * sourceimage->ginfo->height);
        returnimage->rowbytes = sourceimage->rowbytes;
    }

    return returnimage;
}


static GBITMAP *convertSize(GBITMAP *gin, int width, int height)
{
    FINSTANCE *finstance = NULL;
    GBITMAP *gout = galloc(sizeof(GBITMAP));
    FOP optionlist[4];

    if (gin->ginfo->width != width || gin->ginfo->height != height)
    {
        optionlist[0].i = width;
        optionlist[1].i = height;
        optionlist[2].i = rsResizeMethod;
        optionlist[3].i = 0;

        FILTER_find("Size");
    
        FILTER_open(&finstance, 1, gin, 4, optionlist);
        gout->ginfo = FILTER_info(finstance, 0);
        gout->rowbytes = gout->ginfo->width * gout->ginfo->bpp / 8;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        FILTER_filter(finstance, gout->ginfo, gout->image, gout->rowbytes);
        FILTER_close(finstance);
    }
    else
    {
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
        gout->rowbytes = gin->rowbytes;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        memcpy(gout->image, gin->image, gout->rowbytes * gout->ginfo->height);
    }

    return gout;
}


/* Converts input bitmap to specified bits per pixel */
/* If bpp is greater than gin->ginfo->bpp then it just copies image */
static GBITMAP *convertDown(GBITMAP *gin, int bpp, ARGBBits *argb)
{
    GBITMAP *gout = galloc(sizeof(GBITMAP));

    /* If attempt to convert up or to below 0 bpp then just copy image */
    if (gin->ginfo->bpp <= bpp || bpp <= 0)
    {
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
        gout->rowbytes = gin->rowbytes;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        memcpy(gout->image, gin->image, gout->rowbytes * gout->ginfo->height);
    }
    /* If going down but staying above 8bpp then just dither (quick) */
    else if (bpp > 8)
    {
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));

        /* Set output image fields */
        gout->ginfo->numcolours = 0;
        gout->ginfo->bpp = 32;
        gout->ginfo->originalbpp = bpp;
        setARGBBits(gout, bpp, argb);

        gout->rowbytes = gout->ginfo->width * gout->ginfo->bpp / 8;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);

        /* Dither image to: x bpp where 8 < x < 32 */
        filterdither32(gin->ginfo, gin->image, gin->rowbytes, gout->ginfo, gout->image, gout->rowbytes, rsDitherMethod);
    }
    /* But if going to 8bpp or below then use colour reduction */
    else if (bpp <= 8)
    {
        CRINSTANCE *crx;
        int numcolours = intpow(2, bpp) - 1;
        FPALETTE *fpal = galloc(sizeof(FPALETTE));
		CRINVERSETABLE *inversetbl;
		unsigned char colourmask[256];

        if (strcmp(rsColourReductionMethod, "4D Wu"))
            ++numcolours;

        CRAPI_find(rsColourReductionMethod);

        /* Reduce colour depth to: x <= 8 bpp */
        crx = CRAPI_open();
        CRAPI_read(crx, gin->ginfo, gin->image, gin->rowbytes);
		fpal->colourtbl = CRAPI_close(crx, numcolours, &inversetbl, NULL);

		if (!inversetbl)
			fpal->inversetbl = (unsigned char*)(inversetbl + sizeof(CRINVERSETABLE)); // the inversetable follows directly after the CRINVERSETABLE struct
		else
			fpal->inversetbl = 0;

		memset(colourmask,0xff,sizeof(colourmask));
		gout->ginfo = galloc(sizeof(GINFO));
		memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
		gout->ginfo->bpp = bpp;
		memcpy(gout->ginfo->colourtbl, fpal->colourtbl, numcolours);

		gout->rowbytes = gout->ginfo->width * gout->ginfo->bpp / 8;
		gout->image = galloc(gout->rowbytes * gout->ginfo->height);

		CRAPI_remap(gout->image, gout->ginfo, gout->rowbytes, gin->image, gin->rowbytes, gin->ginfo->bpp, colourmask, fpal->inversetbl, 1);
        
        if (fpal->inversetbl)
            gfree(fpal->inversetbl);
        gfree(fpal->colourtbl);
        gfree(fpal);

        /* If colour reduction method is CRTYPEALPHA then make alpha the last colour */
        if (!strcmp(rsColourReductionMethod, "4D Wu"))
        {
            gout->ginfo->colourtbl[numcolours].a = 0;
            gout->ginfo->colourtbl[numcolours].r = 0;
            gout->ginfo->colourtbl[numcolours].g = 0;
            gout->ginfo->colourtbl[numcolours].b = 0;
        }
        gout->ginfo->numcolours = numcolours;
        gout->ginfo->bpp = 8;
        gout->ginfo->originalbpp = bpp;
        setARGBBits(gout, bpp, argb);
    }

    return gout;
}


/* Converts input bitmap to specified bits per pixel */
/* If bpp is less than gin->ginfo->bpp then it just copies image */
static GBITMAP *convertUp(GBITMAP *gin, int bpp, ARGBBits *argb)
{
    GBITMAP *gout = galloc(sizeof(GBITMAP));

    /* If attempt to convert down or past 32bpp, then just copy image */
    if (gin->ginfo->originalbpp >= bpp || bpp > 32)
    {
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
        gout->rowbytes = gin->rowbytes;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        memcpy(gout->image, gin->image, gout->rowbytes * gout->ginfo->height);
    }
    /* If going from below 8bpp to above 8bpp then use the fly remap functions */
    else if (gin->ginfo->originalbpp <= 8 && bpp > 8)
    {
        int y;
        int x;
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
        setARGBBits(gout, bpp, argb);
        gout->ginfo->numcolours = 0;
        gout->ginfo->bpp = 32;
        gout->ginfo->originalbpp = bpp;
        gout->rowbytes = gout->ginfo->width * gout->ginfo->bpp / 8;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        for (y = 0; y < gin->ginfo->height; ++y)
        {
            unsigned char* s = (unsigned char*)((unsigned char *)gin->image + gin->rowbytes * y);
            ARGB *d = (ARGB*) ((unsigned char *)gout->image + gout->rowbytes * y);
            
            for (x = 0; x < gin->ginfo->width; ++x)
            {
                *d = gin->ginfo->colourtbl[*s];
                d++;
                s++;
            }
        }
    }
    /* If staying above 8bpp then just adjust ginfo fields */
    else if (gin->ginfo->originalbpp > 8 && bpp > 8)
    {
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
        gout->rowbytes = gin->rowbytes;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        memcpy(gout->image, gin->image, gout->rowbytes * gout->ginfo->height);
        setARGBBits(gout, bpp, argb);
        gout->ginfo->originalbpp = bpp;
    }
    /* If staying less than or equal to 8bpp then just adjust ginfo fields */
    else if (gin->ginfo->originalbpp <= 8 && bpp <= 8)
    {
        int numcolours = intpow(2, bpp) - 1;
        gout->ginfo = galloc(sizeof(GINFO));
        memcpy(gout->ginfo, gin->ginfo, sizeof(GINFO));
        gout->rowbytes = gin->rowbytes;
        gout->image = galloc(gout->rowbytes * gout->ginfo->height);
        memcpy(gout->image, gin->image, gout->rowbytes * gout->ginfo->height);
        setARGBBits(gout, bpp, argb);
        gout->ginfo->originalbpp = bpp;
        gout->ginfo->bpp = 8;
        if (gout->ginfo->alphabits != 0)
            gout->ginfo->numcolours = numcolours + 1;
        else
            gout->ginfo->numcolours = numcolours;
    }

    return gout;
}


static void setARGBBits(GBITMAP *gb, int bpp, ARGBBits *argb)
{
    if (bpp>8 && bpp<=24)
    {
        gb->ginfo->redbits =   gmin(8,bpp/3);
        gb->ginfo->greenbits = gmin(8,bpp-bpp/3*2);
        gb->ginfo->bluebits =  gmin(8,bpp/3);
        gb->ginfo->alphabits = 0;
    }
    else if (bpp>24 && bpp<=32)
    {
        gb->ginfo->redbits =   gmin(8,bpp/3);
        gb->ginfo->greenbits = gmin(8,bpp/3);
        gb->ginfo->bluebits =  gmin(8,bpp/3);
        gb->ginfo->alphabits = gb->ginfo->originalbpp-gmin(8,bpp/3)*3;
    }
    else if (bpp>0 && bpp<=8)
    {
        gb->ginfo->redbits = 8;
        gb->ginfo->greenbits = 8;
        gb->ginfo->bluebits = 8;
        gb->ginfo->alphabits = 8;
    }

    if (argb)
    {
        gb->ginfo->alphabits = argb->a;
        gb->ginfo->redbits = argb->r;
        gb->ginfo->greenbits = argb->g;
        gb->ginfo->bluebits = argb->b;
    }
    
    return;
}


static int getARGBBits(GINFO *ginfo, ARGBBits *argb)
{
    int ok = 0;
    if (ginfo && argb)
    {
        ok = 1;
        argb->a = (unsigned char)ginfo->alphabits;
        argb->r = (unsigned char)ginfo->redbits;
        argb->g = (unsigned char)ginfo->greenbits;
        argb->b = (unsigned char)ginfo->bluebits;
    }
    return ok;
}


static int intpow(int base, int exponent)
{
    if (exponent == 0)
        return 1;
    else
        return base * intpow(base, exponent - 1);
}



/* The following functions were taken from gcolour.c */


static void gseed(unsigned int seed)
{
    srand(seed);
}

static unsigned int grandom(void)
{
    return(rand()>>8);
}

static int gdith2(int v, int m)
{
    v = (v&~(m-1)) + (grandom()&(m-1));
    return(v);
}

static int gdith(int v, int m)
{
        v -= m/2;

    if ((grandom()&(unsigned int)(S(m)-1)) < (unsigned int)S(v&((m)-1)))
                v += m;

    v = gmax(0,gmin(255,v));

    return(v);
}

static int filterdither32(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes,int dither)
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
    int ok=1;
    ARGB *s;
    ARGB *d;

    am = 1<<(8-ginfo->alphabits);
    rm = 1<<(8-ginfo->redbits);
    gm = 1<<(8-ginfo->greenbits);
    bm = 1<<(8-ginfo->bluebits);
    if (!ginfo->alphabits)
        am = 1;
    width = ginfo->width;
    height = ginfo->height;
    s = (ARGB *) source;
    d = (ARGB *) dest;
    gseed(0xfb123456);
    for (x=0; x<width; ++x)
    {
        for (y=0; y<height; ++y)
        {
            s = (ARGB *) (source + y*rowbytes)+x;
            d = (ARGB *) (dest + y*destrowbytes)+x;

            a = s->a;
            r = s->r;
            g = s->g;
            b = s->b;

            switch (dither)
            {
                case 0:
                    a &= ~(am-1);
                    r &= ~(rm-1);
                    g &= ~(gm-1);
                    b &= ~(bm-1);
                    break;

                case 1:
                    a = gdith(a,am);
                    r = gdith(r,rm);
                    g = gdith(g,gm);
                    b = gdith(b,bm);
                    a &= ~(am-1);
                    r &= ~(rm-1);
                    g &= ~(gm-1);
                    b &= ~(bm-1);
                    break;

                case 2:
                    a = gdith2(a,am);
                    r = gdith2(r,rm);
                    g = gdith2(g,gm);
                    b = gdith2(b,bm);
                    break;
            }

            d->a = (unsigned char)a;
            d->r = (unsigned char)r;
            d->g = (unsigned char)g;
            d->b = (unsigned char)b;
        }
    }

    return(ok);
}
