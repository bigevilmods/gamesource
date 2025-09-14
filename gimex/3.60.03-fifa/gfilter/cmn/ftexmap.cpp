// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*       MkTextureMap - Create Facial Texture Map                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010730 ID crops out center line and composits            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* blend(method,colour,amount0,amount1,mask,palmethod)              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ftexmap.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *abouttexmap(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('T','X','M','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         2;
        about->filterclass =        FCLASS_COMPOSITE;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "FTxMp");
        strcpy(about->longnamestr,  "Facial Texture Map");
        about->numoptions =         0;
    }
    return(about);
}

FPUBLIC int opentexmap(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }
    *filterInstance = fx;
    return(ok);
}

FPUBLIC GINFO *infotexmap(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int image1=0;

    if (fx->numsources>1) image1 = 1;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        ginfo->width  = 1;
        ginfo->defaultx = 0;
        ginfo->defaulty = 0;
        ginfo->centerx  = 0;

        ginfo->numcolours = gmax(ginfo->numcolours,fx->source[image1].ginfo->numcolours);
    }
    if(image1)
    {
        ginfo->width  = fx->source[1].ginfo->width+1;
        ginfo->height = gmax(fx->source[1].ginfo->height,ginfo->height);
        ginfo->centerx  = ginfo->width/2;
    }
    return(ginfo);
}

FPUBLIC int filtertexmap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y     =0;
    int i     =0;
    int image0=0;
    int image1=0;
    int width =0;
    int height=0;
    int dif   =0;
    int pos   =0;
    int dpos  =0;
    int offset=0;
    int row   =0;
    int srow  =0;

    char * composite_image    = (char *)fx->source[image0].image;
    GINFO* composite_ginfo    = ginfo;
    int    composite_rowbytes = fx->source[image0].rowbytes;
    char * cd = (char *)dest;
    char * cs = (char *)fx->source[image0].image;
    ARGB * d  = (ARGB *)dest;
    ARGB * s  = (ARGB *)fx->source[image0].image;

    if (fx->numsources>1) image1 = 1;
    if(image1)
    {
        composite_ginfo = (GINFO *)fx->source[image1].ginfo;
        composite_image = (char *)fx->source[image1].image;
        composite_rowbytes = fx->source[image1].rowbytes;
    }
    if(ginfo->bpp == 32)
    {
        for(i = 0; i < ginfo->width*ginfo->height;i++)
        {
            d[i].a = 255;
            d[i].r = d[i].g = d[i].b = 0;
        }
    }
    else
        memset(dest,0,ginfo->width*ginfo->height);

    // try to handle different heights gracefully.
    // not supposed to have different heights but if I get them
    // I don't want to bomb.
    height  = gmin(fx->source[image0].ginfo->height,composite_ginfo->height);
    width   = ginfo->width;
    dif = fx->source[image0].ginfo->height - composite_ginfo->height;
    dif = (dif < 0)?-dif:dif;
    dif = (int)(dif/2.0);
    pos = (int)(fx->source[image0].ginfo->width/2.0);

    // if the images are not the same height then we have a problem.
    // try to deal with it semi elegantly here.
    if(image1 && (fx->source[image0].ginfo->height != composite_ginfo->height) )
    {
        // if new image is larger than all images seen so far.
        if(fx->source[image0].ginfo->height > composite_ginfo->height)
        {
                pos    += (fx->source[image0].ginfo->width*dif);  // use this for regular copy (has center offset included)
                offset += (fx->source[image0].ginfo->width*dif);  // use this for composite image.
        }
        else
                dpos   += (width*dif);
    }

    for (y=0; y<height; ++y)
    {
        row = y*width;
        srow = y*fx->source[image0].ginfo->width;
        if(ginfo->bpp == 32)
        {
            if(image1)   // copy over old image
            {
                memcpy((char *)(&d[row + dpos]),(char *)(&composite_image[y*composite_rowbytes]),composite_rowbytes);
            }
            // copy over central column of this frame to last column of new image.
            d[row+dpos+width-1] = s[srow + pos];
        }
        else
        {
            if(image1)
            {
                memcpy((char *)(&cd[row + dpos]),(char *)(&composite_image[y*composite_rowbytes]),composite_rowbytes);
            }
            cd[row+dpos+width-1] = cs[srow + pos];
        }
    }

    return(1);
}

FPUBLIC int closetexmap(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

