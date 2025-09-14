// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*              Crop - Crop Image to Specified Rectangle            */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    0.90 950619 SC initial version                                */
/*    1.00 950906 GS cleaned up, added helper function              */
/*    1.01 951106 FB rewritten based on frot                        */
/*    1.02 951107 FB use crop colour                                */
/*    1.03 000718 FB respect default position                       */
/*    1.04 000811 FB clamp modes                                    */
/*    1.05 010118 ID fix crop for layered images                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* crop(int x,int y,int width,int height,int colour, clampx,clampy) */
/* int clamp; 0=fill, 1=clamp, 2=wrap, 3=mirror                     */
/*                                                                  */
/* Crop an image to specified size, filling with colour for areas   */
/* outside original image.                                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fcrop.h"
#include <string.h>

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutcrop(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('C','R','O','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.04");
        strcpy(about->namestr,      "Crop");
        strcpy(about->longnamestr,  "Crop");
        about->numoptions =         8;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           -32000;
        about->option[0].max.i =           +32000;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "X");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           -32000;
        about->option[1].max.i =           +32000;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Y");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           +32000;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "Width");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           +32000;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "Height");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "Colour");
        strcpy(about->option[4].enumlist,  "");

        about->option[5].type =            FOP_INT;
        about->option[5].min.i =           0;
        about->option[5].max.i =           3;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "ClampX");
        strcpy(about->option[5].enumlist,  "Fill,Clamp,Wrap,Mirror");

        about->option[6].type =            FOP_INT;
        about->option[6].min.i =           0;
        about->option[6].max.i =           3;
        about->option[6].def.i =           0;
        strcpy(about->option[6].name,      "ClampY");
        strcpy(about->option[6].enumlist,  "Fill,Clamp,Wrap,Mirror");

        about->option[7].type =            FOP_INT;
        about->option[7].min.i =           0;
        about->option[7].max.i =           1;
        about->option[7].def.i =           0;
        strcpy(about->option[7].name,      "Layered Flag");
        strcpy(about->option[7].enumlist,  "");

    }
    return(about);
}

FPUBLIC int opencrop(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infocrop(FINSTANCE *fx, int framenum)
{
    int i;
    int dx;
    int dy;
    GINFO *ginfo;
    int cropx=0;
    int cropy=0;
    int cropw=1;
    int croph=1;
    int layered = 1;
    unsigned int cropc=0;
    unsigned int cropclampx=0;
    unsigned int cropclampy=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) cropx = fx->option[0].i;
        if (fx->numoptions>=2) cropy = fx->option[1].i;
        if (fx->numoptions>=3) cropw = fx->option[2].i;
        if (fx->numoptions>=4) croph = fx->option[3].i;
        if (fx->numoptions>=5) cropc = fx->option[4].i;
        if (fx->numoptions>=6) cropclampx = fx->option[5].i;
        if (fx->numoptions>=7) cropclampy = fx->option[6].i;
        if (fx->numoptions>=8) layered = fx->option[7].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        if(layered)   // if this is a layered image then we need to use defaultx and defaulty information.
        {

            dx = cropx-ginfo->defaultx;                   // calculate the offset
            dy = cropy-ginfo->defaulty;

            ginfo->defaultx = cropx;                      // update default x and y only if we need to.
            ginfo->defaulty = cropy;
        }
        else
        {
            dx = 0;
            dy = 0;
        }
        ginfo->width  = cropw;
        ginfo->height = croph;

        ginfo->centerx  -= dx;
        ginfo->centery  -= dy;
        for (i=0; i<ginfo->numhotspots; ++i)
        {
            ginfo->hotspottbl[i][0] -= dx;
            ginfo->hotspottbl[i][1] -= dy;
        }
    }
    return(ginfo);
}

FPUBLIC int filtercrop(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int sx;
    int sy;
    unsigned char *s;
    unsigned char *d;
    int  bpp = ginfo->bpp;
    int cropx=0;
    int cropy=0;
    int cropw=1;
    int croph=1;
    int layered = 1;
    unsigned int cropc=0;
    unsigned int cropclampx=0;
    unsigned int cropclampy=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) cropx = fx->option[0].i;
        if (fx->numoptions>=2) cropy = fx->option[1].i;
        if (fx->numoptions>=3) cropw = fx->option[2].i;
        if (fx->numoptions>=4) croph = fx->option[3].i;
        if (fx->numoptions>=5) cropc = fx->option[4].i;
        if (fx->numoptions>=6) cropclampx = fx->option[5].i;
        if (fx->numoptions>=7) cropclampy = fx->option[6].i;
        if (fx->numoptions>=8) layered = fx->option[7].i;
    }
    for (y=0; y<ginfo->height; ++y)
    {
        d = (unsigned char*)dest;
        for (x=0; x<ginfo->width; ++x)
        {
            if(layered)
                sx = cropx+x-fx->source[0].ginfo->defaultx;
            else
                sx = cropx+x;
            switch (cropclampx)
            {
                case CROPCLAMP:
                    sx = gmax(0,gmin(fx->source[0].ginfo->width-1,sx));
                    break;

                case CROPWRAP:
                    if (sx<0)
                        sx = (fx->source[0].ginfo->width - 1) - (-sx % fx->source[0].ginfo->width);
                    else
                        sx = sx % fx->source[0].ginfo->width;
                    break;

                case CROPMIRROR:
                    if (sx<0)
                    {
                        if ((sx/fx->source[0].ginfo->width)&1)
                            sx = fx->source[0].ginfo->width - 1 - (-sx % fx->source[0].ginfo->width);
                        else
                            sx = -sx % fx->source[0].ginfo->width;
                    }
                    else
                    {
                        if ((sx/fx->source[0].ginfo->width)&1)
                            sx = fx->source[0].ginfo->width - 1 - (sx % fx->source[0].ginfo->width);
                        else
                            sx = sx % fx->source[0].ginfo->width;
                    }

                    break;

                default: break;
            }
            if(layered)
                sy = cropy+y-fx->source[0].ginfo->defaulty;
            else
                sy = cropy+y;
            switch (cropclampy)
            {
                case CROPCLAMP:
                    sy = gmax(0,gmin(fx->source[0].ginfo->height-1,sy));
                    break;

                case CROPWRAP:
                    if (sy<0)
                        sy = fx->source[0].ginfo->height - 1 - (-sy % fx->source[0].ginfo->height);
                    else
                        sy = sy % fx->source[0].ginfo->height;
                    break;

                case CROPMIRROR:
                    if (sy<0)
                    {
                        if ((sy/fx->source[0].ginfo->height)&1)
                            sy = fx->source[0].ginfo->height - 1 - (-sy % fx->source[0].ginfo->height);
                        else
                            sy = -sy % fx->source[0].ginfo->height;
                    }
                    else
                    {
                        if ((sy/fx->source[0].ginfo->height)&1)
                            sy = fx->source[0].ginfo->height - 1 - (sy % fx->source[0].ginfo->height);
                        else
                            sy = sy % fx->source[0].ginfo->height;
                    }
                    break;

                default: break;
            }

            if (sx>=0 && sx<fx->source[0].ginfo->width
             && sy>=0 && sy<fx->source[0].ginfo->height)
            {
                s = (unsigned char *)(fx->source[0].image)+sy*fx->source[0].rowbytes+sx*bpp/8;
                if (bpp==8)
                    *d++ = *s;
                else
                {
                    *(int *)d = *(int *)s;
                    d += 4;
                }
            }
            else
            {
                if((fx->numoptions >= 8) && (layered) && fx->source[0].ginfo->framenum != 0)  // I do not want to fill on any layer but the bottom layer
                {                                                                             // this is due to the face layers can be different sizes
                    if (bpp==8)                                                               // and upper layers should not obscure lower layers.
                         *d++ = (unsigned char)0;
                    else
                    {
                        *(unsigned int *)d = 0;
                            d += 4;
                    }
                }
                else
                {
                    if (bpp==8)
                        *d++ = (unsigned char)cropc;
                    else
                    {
                        *(unsigned int *)d = cropc;
                        d += 4;
                    }
                }
            }
        }
        dest += rowbytes;
    }
    return(1);
}

FPUBLIC int closecrop(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

