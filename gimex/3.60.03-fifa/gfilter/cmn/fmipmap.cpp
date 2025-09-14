// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       mipmap - Filter                            */
/*                                                                  */
/*                    by Ian H. Davids, EAC                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 030403 ID Original                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* mipmap(method,levels,minlevels,width,height,clearLastN,backGroundColour,TransparentFlag,MipmapCollapseFlag) */
/*                                                                  */
/* mipmaps an image.  one image in, many mipmaps out                */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fmipmap.h"
#include "gfilter.h"
#include "fopen.h"
#include "../../crapi/cmn/crapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

enum SetupType {halfsize,resize};
enum E_CrapiQuantizers {nil,wu,fu,wu777,nil2,nil3,nil4,exact,alphawu,wu555};

// ugly globals but they have to be here for
// the global palette colour reduction to work right
static CRINVERSETABLE * M_ginversetbl                = NULL;
static CRINSTANCE *     M_CRx                        = NULL;
static int              M_CRpasscount                = -1;
static int              M_CRoldsubtype               = 0;
static int              M_CRrealnumcolours           = 0;
static ARGB             M_CRscolourtbl[256];



// ------------------------------------------------------------
// simple log base 2 on an integer
static int MipmapIntLog2(unsigned int l)
{
    if(l != 0)                  // don't allow someone to do a log of zero some machines can't handle the floating point divide by zero.
        return((int)(log((double)l)/log(2.0f)+0.5f));
    else
        return(0);
}

// ------------------------------------------------------------
// determines the number of unique colours in the image
// returns the number of colours if less than or equal to 256 or else returns 300
// the 300 figure should be enough to skew the choice to use exact unless there are
// several images with less than 256 colours.
static int isLE256Colours(unsigned char *base, int height, int width, int rowbytes)
{
    int x, y, i;
    int *pixel;
    int count, found;
    int colour[256];

    count = 0;
    found = 0;

    for (y = 0; y < height; y++)
    {
        pixel = (int *)(base + y*rowbytes);
        for (x = 0; x < width; x++)
        {
            found = 0;

            for (i = 0; i < count; i++)
            {
                if (colour[i]==*pixel)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                // the entire array has been searched and no match in colour - meaning there is a 257th colour
                if (count==256)
                    return(300);
                colour[count] = *pixel;
                count++;
            }
            pixel++;
        }
    }
    return (count);
}

// ------------------------------------------------------------
static void ScanTrans32(int *alphahistogram,ARGB *dest,ARGB *chromacolour, int *chromacount, int *transparentcount, int n)
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

// ------------------------------------------------------------
static void ScanTrans8(int *alphahistogram,char *dest,ARGB *colourtbl, ARGB *chromacolour, int *chromacount, int *transparentcount, int n)
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


// ------------------------------------------------------------
// returns 2 or more if bitmap contains any transparent pixels
// note for 8 bit, actually scans used pixels, not the palette
static void GetAlphaHistogram(GBITMAP *gbitmap, int *alphahistogram, ARGB *chromacolour, int *chromacount, int *transparentcount)
{
    int i;
    char *source = (char *) gbitmap->image;
    GINFO *ginfo = gbitmap->ginfo;

    memset(alphahistogram,0,256*sizeof(int));
    for (i=0; i<ginfo->height; ++i)
    {
        if (ginfo->bpp==8)
            ScanTrans8(alphahistogram,source,ginfo->colourtbl,chromacolour,chromacount,transparentcount,ginfo->width);
        else
            ScanTrans32(alphahistogram,(ARGB *)source,chromacolour,chromacount,transparentcount,ginfo->width);
        source += gbitmap->rowbytes;
    }
    return;
}

// ------------------------------------------------------------
// returns 1 if bitmap contains any transparent pixels alpha=0
// returns 2 if bitmap contains any translucient pixels alpha=1..255
static int HasTransparent(GBITMAP *gbitmap, ARGB *chromacolour, int *chromacount, int *transparentcount)
{
    int alphahist[256];
    int i;
    int lut=0;
    int PS2overalphaflow=0;

    *chromacount = 0;
    *(int *)chromacolour = 0xffffffff;
    if (transparentcount) *transparentcount = 0;

    GetAlphaHistogram(gbitmap, alphahist, chromacolour,chromacount, transparentcount);

    for (i=1; i<0xf0; ++i)
        lut += alphahist[i];
    for (i=129; i<256; ++i)
        PS2overalphaflow += alphahist[i];

    return((alphahist[0]?1:0) + (lut?2:0) + (PS2overalphaflow?4:0));
}


// ------------------------------------------------------------
static void DoFilter32to8(GINFO *gin, char *source, int rowbytes, GINFO *ginfo, char *dest, int destrowbytes, int bpp, unsigned char *colourmask)
{
    CRAPI_remap(dest, ginfo, ginfo->width*ginfo->bpp/8, source, gin->width*gin->bpp/8, gin->bpp, colourmask, 0, 0);
}

// ------------------------------------------------------------
// colour reduction helper functions. Work directly with
// crapi.
static GINFO *GetGinfo32to8(GINFO *gin, int numcolours, char *source, int rowbytes, int bpp, int pass)
{
    GINFO *ginfo=0;

    if (M_CRpasscount == -1)
    {
        // First call so start colour reduction
        if (numcolours)
            M_CRx = CRAPI_open();

        M_CRpasscount = 0;
    }

    // Image in pass 0
    if ((M_CRpasscount == 0) && (pass == 0))
    {
        if (M_CRx)
            CRAPI_read(M_CRx, gin, source, rowbytes);
        if(!M_CRoldsubtype)
            M_CRoldsubtype = gin->subtype;
    }

    // First time through pass 1
    if ((M_CRpasscount == 0) && (pass == 1))
    {
        ARGB *colourtbl = 0;

        if (M_CRx)
        {
            // Save colour table
            if (M_ginversetbl)
                gfree(M_ginversetbl);
            M_ginversetbl = 0;

            colourtbl = CRAPI_close(M_CRx, numcolours, &M_ginversetbl, &M_CRrealnumcolours);
            if (colourtbl)
            {
                memcpy(M_CRscolourtbl, colourtbl, M_CRrealnumcolours * sizeof(ARGB));
                gfree(colourtbl);
            }
            M_CRx = 0;
            // Save colour table
            if (M_ginversetbl)
                gfree(M_ginversetbl);
            M_ginversetbl = 0;
        }
        M_CRpasscount = 1;
    }

    // Image in pass 1
    if (M_CRpasscount == 1)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {

            memcpy((void *) ginfo, (void *) gin, sizeof(GINFO));
            memset(ginfo->colourtbl,0,256*sizeof(ARGB));

            ginfo->signature        = GMAKEID('C','O','L','R'); /* Type of ginfo */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
            ginfo->subtype          = M_CRoldsubtype;           /* subtype */
            ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
            ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours       = M_CRrealnumcolours;
            if (numcolours)
            {
                memcpy(ginfo->colourtbl,M_CRscolourtbl,M_CRrealnumcolours*sizeof(ARGB));
            }
        }
    }
    return(ginfo);
}


// ------------------------------------------------------------
// choose the apropriate quantizer method. Note there is a little
// bit of magic here for attempting to choose exact. Exact is
// faster and when we can use it, it would be the preferred method
// since it will preserve the original clut as much as possible.
static E_CrapiQuantizers SetCrapiQuantizer(GINFO ** ginfomany,int ind,int levels,int exactcolours,int colours,int chromacount,int transluciencyflags)
{

    float temp                    = 0;
    E_CrapiQuantizers quantmethod = wu;

    // give a small chance of using exact if I don't have to many extra colours
    temp = (float)((levels>2)?((float)levels/3.0):1.0);
    if ((exactcolours && exactcolours<=colours*temp))
        quantmethod = exact;

    // if bits per channel is 555 or 666 use Wu5 or Wu6
    else if ((ginfomany[ind]->alphabits>1 && (transluciencyflags)) || (chromacount>1) )
        quantmethod = alphawu;

    // if bits per channel is 555 use Wu6
    else if (ginfomany[ind]->redbits<=5 && ginfomany[ind]->greenbits<=5 && ginfomany[ind]->bluebits<=5)
        quantmethod = wu;

    // default mode is Wu666
    else
        quantmethod = wu777;

    return quantmethod;
}


// ------------------------------------------------------------
// useful debug information for colour reduction
static void DebugOutReductionMethod(int ind,int pass,char * quantstr,int depth,int trans)
{
#ifdef DEBUG
    if (ind == 0)
    {
        if(pass == 0)
        {
            printf("Resampling to");
            printf(" %d bit indexed",depth);
            if (trans)
                printf(" (+1 transparent)");
            printf(" global palette");
            if (quantstr)
                printf(" using %s",quantstr);
            printf("\n");
        }
    }
#endif
}

// ------------------------------------------------------------
// the most ugly of the functions. This attempts to determine
// the best match quantizer and then do a global palette
// colour reduction accross all the mipmaps.
static void Mipmap_DoGlobalPaletteColourReduction(GINFO ** ginfomany,char *** rfmany,SavedItems * SI,int levels)
{

    int pass         = 0;
    int ind          = 0;
    int colours      = SI->SavedOriginalNumColours;
    int exactcolours = 0;
    int depth        = SI->SavedOriginalBpp;
    char ** fmany    = *rfmany;
    char * f0        = 0;
    GINFO * ginfo0   = 0;
    E_CrapiQuantizers quantmethod = nil;


    // count colour usage so I can use Exact if
    // the colour count is low enough
    for(ind = 0; ind < levels; ind++)
        exactcolours += isLE256Colours((unsigned char *)fmany[ind], ginfomany[ind]->height, ginfomany[ind]->width, ginfomany[ind]->width*ginfomany[ind]->bpp/8);

    for(pass = 0; pass < 2; pass++)
    {
        for(ind = 0; ind < levels; ind++)
        {
            GBITMAP source;
            int exactcolours=0;
            int transluciencyflags=0;
            int transparentcolours=0;
            ARGB chromacolour;
            int chromacount;
            unsigned char colourmask[256];
            char *quantstr=NULL;
            CRABOUT *crapiabout=0;
            source.ginfo    = ginfomany[ind];
            source.image    = fmany[ind];
            source.rowbytes = ginfomany[ind]->width*ginfomany[ind]->bpp/8;

            memset(colourmask,0xff,sizeof(colourmask));
            // returns a bit field 1 = alpha of 0 exists, 2 = alpha 1..255 exists
            transluciencyflags = HasTransparent(&source,&chromacolour,&chromacount,NULL);
            transparentcolours = transluciencyflags&1;

            if (quantmethod==0)
            {
                quantmethod = SetCrapiQuantizer(ginfomany,ind,levels,exactcolours,colours,chromacount,transluciencyflags);
            }
            CRAPI_set(quantmethod);
            crapiabout = CRAPI_about();
            quantstr   = crapiabout->namestr;

            DebugOutReductionMethod(ind,pass,quantstr,depth,transparentcolours);
            ginfo0 = GetGinfo32to8(ginfomany[ind],colours,fmany[ind],ginfomany[ind]->width*ginfomany[ind]->bpp/8,depth,pass);

            if (ginfo0)
            {
                // module doesn't handle alpha so set the bits on the new ginfo
                // for the remap and handle halfalpha situation.
                if(!crapiabout->respectalpha)
                    ginfo0->alphabits = 0;
                else
                    ginfo0->alphabits = SI->SavedABits;

                // restore bit depth of the original image for each channel
                ginfo0->redbits   = SI->SavedRBits;
                ginfo0->greenbits = SI->SavedGBits;
                ginfo0->bluebits  = SI->SavedBBits;

 #ifdef DEBUG
                if(pass == 1)
                {
                    printf("* Setting mipmaps to original channel depth: %d%d%d%d\n",ginfo0->alphabits,ginfo0->redbits,ginfo0->greenbits,ginfo0->bluebits);
                    printf("* BPP: %d/%d\n",ginfo0->bpp,ginfo0->originalbpp);
                    printf("* NC:  %d\n",ginfo0->numcolours);
                }

#endif

                // only on the last pass should I remap since
                // only then will the real palette be valid.
                if(pass == 1)
                {
                    f0 = (char *) galloc(ginfo0->width*ginfo0->height*ginfo0->bpp/8);
                    if (f0)
                    {
 #ifdef DEBUG

                {
                    printf("-------------------------\n");
                    printf("Doing Remap on frame: %d\n",ind);
                    printf("Setting mipmaps to original channel depth: %d%d%d%d\n",ginfo0->alphabits,ginfo0->redbits,ginfo0->greenbits,ginfo0->bluebits);
                    printf("BPP: %d/%d\n",ginfo0->bpp,ginfo0->originalbpp);
                    printf("NC:  %d\n",ginfo0->numcolours);
                }

#endif
                        DoFilter32to8(ginfomany[ind],fmany[ind],ginfomany[ind]->width*ginfomany[ind]->bpp/8,ginfo0,f0,ginfo0->width*ginfo0->bpp/8,depth, colourmask);
                        gfree(fmany[ind]);
                        (*rfmany)[ind] = f0;
                         memcpy(ginfomany[ind],ginfo0,ginfo0->size);
                         gfree(ginfo0);
                    }
                }
                else
                {
                    if(ginfo0)
                        gfree(ginfo0);
                }
            }
            if (crapiabout)
                gfree(crapiabout);
            if (pass == 1 && ginfomany[ind]->bpp==8 && depth>0 && depth<=8)
            {
                ginfomany[ind]->originalbpp = depth;
                ginfomany[ind]->numcolours  = colours;
            }
        }// end for ind
    }// end for pass
    // reset these for any future passes through
    M_CRpasscount                = -1;
    M_CRoldsubtype               = 0;
    M_CRrealnumcolours           = 0;
}


// ------------------------------------------------------------
// quick and dirty upsample from <8bpp up to 32 bit.
// image must be 8bpp.
static void Do_Upsample8to32(GINFO ** ginfoS,char ** f,int depth)
{
    GINFO * ginfo = (GINFO *)galloc((*ginfoS)->size);
    if (ginfo)
    {
        memcpy(ginfo,*ginfoS,(*ginfoS)->size);
        ginfo->signature        = GMAKEID('C','O','L','R'); /* Type of ginfo */
        ginfo->bpp              = 32;                       /* Bits per pixel (8 or 32) */
        ginfo->numcolours       = 0;                        /* Number of colours in original indexed palette */
        ginfo->originalbpp      = depth;
            //ginfo->redbits          = gmin(8,(depth+1)/3);
            //ginfo->greenbits        = gmin(8,(depth+2)/3);
            //ginfo->bluebits         = gmin(8,(depth+0)/3);
            //ginfo->alphabits        = gmin(8,gmax(0,depth-24));
        char * f0 = (char *) galloc(ginfo->width*ginfo->height*4);
        if (f0)
        {
            int width        = ginfo->width;
            int height       = ginfo->height;
            int rowbytes     = ginfo->width;
            int destrowbytes = ginfo->width*4;
            for (int y=0; y<height; ++y)
            {
                char * s = (*f) + y*rowbytes;
                ARGB * d = (ARGB *) (f0 + y*destrowbytes);
                for (int x=0; x<width; ++x)
                {
                    *d++ = ginfo->colourtbl[*((unsigned char *)s)]; s++;

                }
            }
            *ginfoS = ginfo;
            *f      = f0;
        }
    }
}

// ------------------------------------------------------------
// sets the framename to the GX mipmap naming convention
static void __inline SetMipmapFrameName(GINFO * ginfo,char * base_texturename,int level)
{
    // Top level keeps original name, rest get renamed to "base_mm01", "base_mm02", etc.
    if(level != 0)
        sprintf(ginfo->framename, "%s_mm%02d",base_texturename,level);
}

// ------------------------------------------------------------
static void __inline SetupSource(int level, GBITMAP * source, GINFO ** ginfomany, char ** fmany,SetupType set_t)
{
    if(set_t == resize)
    {
        source[0].ginfo    = ginfomany[0];
        source[0].image    = fmany[0];
        source[0].rowbytes = ginfomany[0]->width*ginfomany[0]->bpp/8;
    }
    else
    {
        source[0].ginfo    = ginfomany[level-1];
        source[0].image    = fmany[level-1];
        source[0].rowbytes = ginfomany[level-1]->width*ginfomany[level-1]->bpp/8;
    }
}

// ------------------------------------------------------------
// Worker function for all filtering. Note that I maintain the
// internal filter number. This is important since we are inside
// a filter right now and if we change the number and then
// exit, the user will get junk results when they call the
// next filter function expecting it to work.
static void ApplyFilter(char * fname,int nsource,GBITMAP * source,int nfop,FOP * fop,GINFO ** dest_ginfo,char ** destf)
{
    FINSTANCE *fx;
    int old_filter_num = FILTER_get();
    GINFO * ginfo;

    FILTER_find(fname);
    FILTER_open(&fx,nsource,source,nfop,fop);

    *dest_ginfo = FILTER_info(fx, 0);
    ginfo = *dest_ginfo;
    int drowbytes = ginfo->width * ginfo->bpp/8;

    *destf = (char *) galloc(drowbytes * ginfo->height);
    FILTER_filter(fx, ginfo, *destf, drowbytes);
    FILTER_close(fx);
    FILTER_set(old_filter_num);
}

// ------------------------------------------------------------
// convenience function this function doesn't work on two
// slots in the array but will resize into the ginfomany
// array. It is here mainly to keep the large DoMipmap function
// to a readable state.
static void  InitialSize (GINFO * ginfo,char * f,GINFO ** ginfomany,char **fmany,int sizex,int sizey,int sizealpha,int sizemethod)
{
    GBITMAP source[1];
    FOP fop[7];

    source[0].ginfo    = ginfo;
    source[0].image    = f;
    source[0].rowbytes = ginfo->width*ginfo->bpp/8;
    fop[0].i           = sizex;                            // int width           Width of resulting image
    fop[1].i           = sizey;                            // int height          Height of resulting image
    fop[2].i           = sizemethod;                       // int method          Filter function (see below)
    fop[3].i           = sizealpha;                        // int respect_alpha   0=default,1=keep sharp transparent edges
    fop[4].i           = 0;                                // int chromakey       0=default
    fop[5].f           = 1;                                // double xsharpness   <1=sharp >1=blury x<=0=crash (1.0=default)
    fop[6].f           = 1;                                // double ysharpness   <1=sharp >1=blury x<=0=crash (1.0=default)
    ApplyFilter("Size",1,source,7,fop,&(ginfomany[0]),&(fmany[0]));
}

// ------------------------------------------------------------
// The best method overall but it requires an upsample to 32
// bit to work.
static void  ApplySize (int level,GINFO ** ginfomany,char **fmany,int sizex,int sizey,int sizealpha,int sizemethod)
{
    GBITMAP source[1];
    FOP fop[7];

    SetupSource(level,source,ginfomany,fmany,resize);
    fop[0].i = sizex;                                /* int width           Width of resulting image */
    fop[1].i = sizey;                                /* int height          Height of resulting image */
    fop[2].i = sizemethod;                           /* int method          Filter function (see below) */
    fop[3].i = sizealpha;                            /* int respect_alpha   0=default,1=keep sharp transparent edges */
    fop[4].i = 0;                                    /* int chromakey       0=default */
    fop[5].f = 1;                                    /* double xsharpness   <1=sharp >1=blury x<=0=crash (1.0=default) */
    fop[6].f = 1;                                    /* double ysharpness   <1=sharp >1=blury x<=0=crash (1.0=default) */
    ApplyFilter("Size",1,source,7,fop,&(ginfomany[level]),&(fmany[level]));
}

// ------------------------------------------------------------
// simple point sampling method. This is often good for images
// with a low bit depth (such as 4bpp) to preserve original
// colours and still generate semi decent mipmaps.
static void ApplyScale (int level,GINFO ** ginfomany,char **fmany,int sizex,int sizey)
{
    GBITMAP source[1];
    FOP fop[2];

    SetupSource(level,source,ginfomany,fmany,resize);
    fop[0].i = sizex;                                    /* int width           Width of resulting image */
    fop[1].i = sizey;                                    /* int height          Height of resulting image */
    ApplyFilter("Scale",1,source,2,fop,&(ginfomany[level]),&(fmany[level]));
}

// ------------------------------------------------------------
// plain vanilla copy of source image to dest image including
// ginfo's
static void __inline CopyImage(GINFO * sourceg,char * sourcef,GINFO ** destg,char ** destf)
{
    int datasize = (sourceg->width*sourceg->height*sourceg->bpp + 7)/8;

    // create new data buffers
    *destg     = (GINFO *)galloc(sourceg->size);
    *destf     = (char  *)galloc(datasize);

    // Doing a direct copy of the base texture
    // no resize involved.
    memcpy(*destg, sourceg, sourceg->size);
    memcpy(*destf, sourcef, datasize);
}

// ------------------------------------------------------------
// the old way of doing mipmaps. It is not necessarily the best
// way since it builds the next mipmap off the previous one
// accumulating error as it goes.
static void ApplyHalfSize(int level,GINFO ** ginfomany,char **fmany,bool isdot3)
{
    GBITMAP source[1];
    FOP fop[7];

    SetupSource(level,source,ginfomany,fmany,halfsize);
    fop[0].i = 3;          // direction=3;
    fop[1].i = 0;          // vscale=0;
    fop[2].i = 0;          // hscale=0;
    fop[3].i = 0;          // vstep=0;
    fop[4].i = 0;          // hstep=0;
    fop[5].i = 0;          // edge=0;
    fop[6].i = isdot3?1:0; // dot3=0;
    ApplyFilter("Halfsize",1,source,7,fop,&(ginfomany[level]),&(fmany[level]));
}

// ------------------------------------------------------------
// fill a mipmap with a solid colour. This is useful for
// small mipmaps
static void ApplyDrawrect(int level,GINFO ** ginfomany,char **fmany,int x, int y, int width, int height, int colour)
{
    GBITMAP source[1];
    FOP fop[5];
    // have to keep the originals around so I can free them
    // after the filtering operation.
    GINFO * orig_ginfo  = ginfomany[level];
    char  * framebuffer = fmany[level];

    source[0].ginfo     = ginfomany[level];
    source[0].image     = fmany[level];
    source[0].rowbytes  = ginfomany[level]->width*ginfomany[level]->bpp/8;

    fop[0].i = x;
    fop[1].i = y;
    fop[2].i = width;
    fop[3].i = height;
    fop[4].i = colour;
    ApplyFilter("Rect",1,source,5,fop,&(ginfomany[level]),&(fmany[level]));

    // cleanup the original buffers, they are leaked otherwise
    gfree(framebuffer);
    gfree(orig_ginfo);
}

// ------------------------------------------------------------
// pregenerate all my mipmaps for return when the user
// calls info and filter.
void DoMipmap(MipmapInternals * MI,GINFO * ginfoS,char * fS)
{
    int sizex               = MI->sizex;
    int sizey               = MI->sizey;
    GINFO ** ginfomany      = 0;
    GINFO *  ginfo          = ginfoS;
    char  ** fmany          = 0;
    char  *  f              = fS;
    char * base_texturename = ginfo->framename;
    bool isdot3             = false;
    SavedItems SI;

    memset(&SI,0,sizeof(SavedItems));
    /* Default is to size until either width or height
    hits 1. If a non-default value is given for the
    number of levels then we'll let the number of
    levels grow to the point where both width and
    height are 1. */


    // if we are using the size colour reduce filter then
    // we will quickly upsample here
    if(MI->OPTmipmapmethod==OPTMIPMAPSIZECOLREDUCE && ginfo->bpp<=8 )
    {
        SI.SavedOriginalBpp        = ginfo->originalbpp;
        SI.SavedOriginalNumColours = ginfo->numcolours;
        SI.SavedABits              = ginfo->alphabits;
        SI.SavedRBits              = ginfo->redbits;
        SI.SavedGBits              = ginfo->greenbits;
        SI.SavedBBits              = ginfo->bluebits;
        SI.did_upsample            = true;
        Do_Upsample8to32(&ginfo,&f,32);
    }

    // if we are using the size filter then the image has to be > 32bit
    if (MI->OPTmipmapmethod==OPTMIPMAPSIZE && ginfo->bpp<=8)
        MI->OPTmipmapmethod = OPTMIPMAPBOX;

    if (MI->totalmipmaps)
    {
        ginfomany = (GINFO **)galloc(MI->totalmipmaps*sizeof(GINFO *));
        if (ginfomany)
        {
            fmany = (char **)galloc(MI->totalmipmaps*sizeof(char *));
            if (fmany)
            {
                for(int level = 0; level < MI->totalmipmaps; level++,sizex>>=1,sizey>>=1)
                {
#ifdef DEBUG
                    printf("level %d: %dx%d\n", level, sizex, sizey);
#endif
                    // level 0 can be either resized or
                    // left asis. This depends on what
                    // the user requested
                    if (level==0)
                    {
                        // if the size requested is that of the base
                        // texture
                        if (sizex==ginfo->width && sizey==ginfo->height)
                        {
                            CopyImage(ginfo,f,&(ginfomany[0]),&(fmany[0]));
                        }
                        // we need to resize base texture to fit users request
                        else
                        {
                            InitialSize (ginfo,f,ginfomany,fmany,sizex,sizey,
                                        MI->OPTmipmaptrans,MI->OPTmipmapmethod==OPTMIPMAPBOX?4:0);
                                        // default = 0 BOX = 4
                        }
                    } // end if level == 0

                    else // levels > 0 are all halfsized
                    {
                        if (MI->OPTmipmapmethod==OPTMIPMAPBOX)
                        {
                            ApplyHalfSize(level,ginfomany,fmany,isdot3);
                        }
                        else if(MI->OPTmipmapmethod==OPTMIPMAPSCALE)
                        {
                            ApplyScale (level,ginfomany,fmany,sizex,sizey);
                        }
                        else  // both OPTMIPMAPSIZE and OPTMIPMAPSIZECOLREDUCE go through here
                        {
                            ApplySize (level,ginfomany,fmany,sizex,sizey,
                                        MI->OPTmipmaptrans,MI->OPTmipmapmethod==OPTMIPMAPBOX?4:0);
                                        // default = 0 BOX = 4
                        }
                    }
                    SetMipmapFrameName(ginfomany[level],base_texturename,level);
                    // does the user want all smaller mipmaps cleared to a specific colour?
                    if ((MI->totalmipmaps-level)<=MI->OPTmipmapclearlast)
                    {
                        ApplyDrawrect(level,ginfomany,fmany,0,0,sizex,sizey,MI->OPTbackground);
                    }
                    // set the no mipmap collapse option if the user requested it false means no collapse
                    if(MI->OPTmipmapcollapse)
                        ginfomany[level]->subtype &= ~M_SUBNOMIPMAPCOLLAPSE;
                    else
                        ginfomany[level]->subtype |=  M_SUBNOMIPMAPCOLLAPSE;

                } // end for each mipmap

                // if we are doing a mipmap operation and the user requested to keep
                // the original bit depth then restore it here.
                if(MI->OPTmipmapmethod==OPTMIPMAPSIZECOLREDUCE && SI.did_upsample)
                {
                    Mipmap_DoGlobalPaletteColourReduction(ginfomany,&fmany,&SI,MI->totalmipmaps);
                    // cleanup because I created a new memory buffer in the Upsample process because
                    // I couldn't delete the users original ginfo and framebuffer.
                    if(f) gfree(f);
                    if(ginfo) gfree(ginfo);
                }

            } // end if fmany
            else
            {
                gfree(ginfomany);
                ginfomany = NULL;
            }
        }
    } // end if totalmipmaps > 0
    MI->ginfomany = ginfomany;
    MI->fmany     = fmany;
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

// ------------------------------------------------------------
FPUBLIC FABOUT *aboutmipmap(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('M','I','P','M');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Mipmap" );
        strcpy(about->longnamestr,  "Mipmap" );
        about->numoptions =         9;
        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           1;
        about->option[0].max.i =           5;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "Method");
        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           1;
        about->option[1].max.i =           200;
        about->option[1].def.i =           10;
        strcpy(about->option[1].name,      "Levels");
        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           1;
        about->option[2].max.i =           20;
        about->option[2].def.i =           1;
        strcpy(about->option[2].name,      "MinLevels");
        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           1;
        about->option[3].max.i =           0x7FFFFFFF;
        about->option[3].def.i =           100;
        strcpy(about->option[3].name,      "Width");
        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           1;
        about->option[4].max.i =           0x7FFFFFFF;
        about->option[4].def.i =           100;
        strcpy(about->option[4].name,      "Height");
        about->option[5].type =            FOP_INT;
        about->option[5].min.i =           0;
        about->option[5].max.i =           199;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "ClearLastN");
        about->option[6].type =            FOP_INT;
        about->option[6].min.i =           0;
        about->option[6].max.i =           0xFFFFFFFF;
        about->option[6].def.i =           0;
        strcpy(about->option[6].name,      "Background Colour");
        about->option[7].type =            FOP_INT;
        about->option[7].min.i =           0;
        about->option[7].max.i =           1;
        about->option[7].def.i =           0;
        strcpy(about->option[7].name,      "Transparency Flag");
        about->option[8].type =            FOP_INT;
        about->option[8].min.i =           0;
        about->option[8].max.i =           1;
        about->option[8].def.i =           0;
        strcpy(about->option[8].name,      "Mipmap Collapse Flag");
     }
    return(about);
}

// ------------------------------------------------------------
FPUBLIC int openmipmap(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok             = 0;
    FINSTANCE *fx      = 0;
    int maxlevels      = 0;
    int level          = 1;

    // reset ugly globals for colour reduction
    M_ginversetbl      = NULL;
    M_CRx              = NULL;
    M_CRpasscount      = -1;
    M_CRoldsubtype     = 0;
    M_CRrealnumcolours = 0;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    MipmapInternals * MI = (MipmapInternals *)galloc(sizeof(MipmapInternals));
    memset(MI,0,sizeof(MipmapInternals));

    if(numoptions >= 1) MI->OPTmipmapmethod    = options[0].i;
    else                MI->OPTmipmapmethod    = OPTMIPMAPDEFAULT;
    if(numoptions >= 2) MI->OPTmipmap          = options[1].i;
    else                MI->OPTmipmap          = 10;
    if(numoptions >= 3) MI->OPTmipmapmin       = options[2].i;
    else                MI->OPTmipmapmin       = 1;
    if(numoptions >= 4) MI->sizex              = options[3].i;
    else                MI->sizex              = sources[0].ginfo->width;
    if(numoptions >= 5) MI->sizey              = options[4].i;
    else                MI->sizey              = sources[0].ginfo->height;
    if(numoptions >= 6) MI->OPTmipmapclearlast = options[5].i;
    else                MI->OPTmipmapclearlast = 0;
    if(numoptions >= 7) MI->OPTbackground      = options[6].i;
    else                MI->OPTbackground      = 0;
    if(numoptions >= 8) MI->OPTmipmaptrans     = options[7].i;
    else                MI->OPTmipmaptrans     = 0;
    if(numoptions >= 9) MI->OPTmipmapcollapse  = options[8].i;
    else                MI->OPTmipmapcollapse  = 0;

    int sizex = MI->sizex;
    int sizey = MI->sizey;
    if (MI->OPTmipmap == DEFAULT_NUM_MIPMAPLEVELS)
        maxlevels = gmin(MipmapIntLog2(sizex), MipmapIntLog2(sizey)) + 1;
    else
        maxlevels = gmax(MipmapIntLog2(sizex), MipmapIntLog2(sizey)) + 1;

    if (MI->OPTmipmap)
        maxlevels = gmin(MI->OPTmipmap, maxlevels);
    if (MI->OPTmipmapmin)
        maxlevels = gmin(maxlevels,MipmapIntLog2(gmin(sizex,sizey))-MipmapIntLog2(MI->OPTmipmapmin)+1);
    // count up the number of levels
    while ((level < maxlevels) &&
    ((sizex >= 1) && (sizey >= 1)))
    {
        ++level;
        sizex >>= 1;
        sizey >>= 1;
    }

    MI->totalmipmaps = level;
    DoMipmap(MI,sources[0].ginfo,(char *)sources[0].image);

    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = level;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        fx->fref        = MI;
        ok = 1;
    }
    *filterInstance = fx;
    return(fx->frames);
}

// ------------------------------------------------------------
FPUBLIC GINFO *infomipmap(FINSTANCE *fx, int framenum)
{
    GINFO * ginfo;
    MipmapInternals * MI = (MipmapInternals *)fx->fref;
    if(framenum < MI->totalmipmaps && framenum >= 0)
    {
        ginfo = (GINFO *)galloc(MI->ginfomany[framenum]->size);
        memcpy(ginfo,MI->ginfomany[framenum],MI->ginfomany[framenum]->size);
        return ginfo;
    }
    else
        return 0;
}

// ------------------------------------------------------------
FPUBLIC int filtermipmap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    MipmapInternals * MI = (MipmapInternals *)fx->fref;
    if(MI->current < MI->totalmipmaps && MI->current >= 0)
    {
        GINFO * g = MI->ginfomany[MI->current];
        char  * f = MI->fmany[MI->current];
        int introwbytes = g->width*g->bpp/8;

        for(int i = 0; i < ginfo->height; i++)
        {
            memcpy(&(dest[i*rowbytes]),&(f[i*introwbytes]),introwbytes);
        }
        MI->current++;
        return 1;                                                     // return success.
    }
    else
        return 0;
}

// ------------------------------------------------------------
FPUBLIC int closemipmap(FINSTANCE *fx)
{
    MipmapInternals * MI = (MipmapInternals *)fx->fref;
    if(MI)
    {
        // cleanup the ginfo's
        if(MI->ginfomany)
        {
            for(int i = 0; i < MI->totalmipmaps; i++)
            {
                if(MI->ginfomany[i])
                    gfree(MI->ginfomany[i]);
            }
            gfree(MI->ginfomany);
        }
        // cleanup the framebuffers
        if(MI->fmany)
        {
            for(int i = 0; i < MI->totalmipmaps; i++)
            {
                if(MI->fmany[i])
                    gfree(MI->fmany[i]);
            }
            gfree(MI->fmany);
        }
        gfree(MI);
    }
    return(gfree((void *)fx));
}

