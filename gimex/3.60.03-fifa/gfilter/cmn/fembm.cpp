// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Embm - Bumpmap Gen Filter                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010510 ID Original Spun off from embm filter             */
/*    1.01 010809 ID fixed bug with 8bpp version                    */
/*    1.02 010810 ID added height scale to filter                   */
/*    1.03 010810 ID fixed small bug with init allow -scale         */
/*    1.04 020213 FB unsigned char palette indices                  */
/*    1.05 020215 FB height in alpha, replaced doubles with floats  */
/*    1.06 020218 FB -scale+default, doubles, clamp, filter 3x3     */
/*    1.07 020225 FB based on dot3 v106, embmv102 & ati duvu        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* embm(int type, float scale, int filter, int wrap)                */
/* int type;     type of bump map. 2=uv  88                         */
/*                                 1=uvl 888                        */
/*                                 3=uvl 556                        */
/*                                 6=bias by 128 unsigned char gsh  */
/* float scale;   scale height field.  600/width is good default    */
/* int filter;    0=3x3, 1=point                                    */
/* int wrap;      0=clamp, 1=wrap                                   */
/*                                                                  */
/* Converts height field to embm normalmap                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fembm.h"
#include <string.h>
#if defined(_MSC_VER)
#pragma warning(disable:4514)
#endif
#include <math.h>

#define Cross(A,V1,V2)                {(A)->x = ((V1)->y*(V2)->z - (V1)->z*(V2)->y);                                 \
                                       (A)->y = ((V1)->z*(V2)->x - (V1)->x*(V2)->z);                                 \
                                       (A)->z = ((V1)->x*(V2)->y - (V1)->y*(V2)->x);                                 }
#define Normalize(V1,V2,Temp)         {(Temp = 1.0f/sqrt((V2)->x*(V2)->x + (V2)->y*(V2)->y + (V2)->z*(V2)->z));(V1)->x = (float)((V2)->x*Temp);(V1)->y = (float)((V2)->y*Temp);(V1)->z = (float)((V2)->z*Temp);}


typedef struct Vec
{
    float x;
    float y;
    float z;
} Vec;


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define EMBM_UV_88   2
#define EMBM_UVL_888 1
#define EMBM_UVL_556 3

#define EMBM_GSH     6

// REAL2INT
static int EMBM_REAL2INT(float x)
{
    return ((int)(((float)(x))+((((float)(x))>=0.0f)?0.5f:-0.5f)));
}

static int EMBM_clamp(float f)
{
    int i = EMBM_REAL2INT(f);
    i = gmax(-127,gmin(127,i));
    return i;
}

float EMBM_getpixel(GBITMAP *g,int x, int y, bool grey, bool wrap)
{
    int bpp    = g->ginfo->bpp;
    int width  = g->ginfo->width;
    int height = g->ginfo->height;
    ARGB *s;

    if (wrap)
    {
        if (x>=width)   x = 0;
        if (x<0)        x = width-1;
        if (y>=height)  y = 0;
        if (y<0)        y = height-1;
    }
    else // clamp
    {
        if (x>=width)   x = width-1;
        if (x<0)        x = 0;
        if (y>=height)  y = height-1;
        if (y<0)        y = 0;
    }

    if (bpp<=8)
        s = g->ginfo->colourtbl+(*(((unsigned char *)g->image) + y*g->rowbytes + x));
    else
        s = (ARGB*)(((unsigned char *)g->image) + y*g->rowbytes + x*4);

    return (s->r*299 + s->g*587 + s->b*114)/1000.0f;
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutembm(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('E','M','B','M');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard & Ian Davids");
        strcpy(about->versionstr,   "1.07");
        strcpy(about->namestr,      "Embm" );
        strcpy(about->longnamestr,  "Embm NormalMap" );
        about->numoptions =         4;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "embm type");

        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -100.0f;
        about->option[1].max.f =           100.0f;
        about->option[1].def.f =           5.0f;
        strcpy(about->option[1].name,      "height scale");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           1;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "filter method");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           1;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "wrap mode");
     }
    return(about);
}

FPUBLIC int openembm(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoembm(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int mode = EMBM_UV_88;
    if (fx->option)
        if (fx->numoptions>=1 && fx->option[0].i > 0) mode = fx->option[0].i;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        if (mode == EMBM_UV_88 || mode == EMBM_GSH)
        {
            ginfo->alphabits   = 8;
            ginfo->redbits     = 8;
            ginfo->greenbits   = 0;
            ginfo->bluebits    = 0;
            ginfo->originalbpp = 16;
        }
//        else if (mode == EMBM_UVL_888 || mode == EMBM_GSH)
        else if (mode == EMBM_UVL_888)
        {
            ginfo->alphabits   = 0;
            ginfo->redbits     = 8;
            ginfo->greenbits   = 8;
            ginfo->bluebits    = 8;
            ginfo->originalbpp = 24;
        }
        else if (mode == EMBM_UVL_556)
        {
            ginfo->alphabits   = 0;
            ginfo->redbits     = 5;
            ginfo->greenbits   = 5;
            ginfo->bluebits    = 6;
            ginfo->originalbpp = 16;
        }
        ginfo->bpp = 32;                     // always 32bpp after an EMBM conversion.
    }
    return(ginfo);
}

FPUBLIC int filterembm(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x = 0;
    int y = 0;
    int width  = ginfo->width;
    int height = ginfo->height;
//  int bpp    = fx->source[0].ginfo->bpp;//ginfo->bpp;
//  int pixelBytes = (bpp / 8);

    int mode = 0;
    float scale  = 0;
    int filter   = 0;
    bool wrap    = false;
    if (fx->option)
    {
        if (fx->numoptions>=1) mode  = (int)fx->option[0].i;
        if (fx->numoptions>=2) scale = (float)fx->option[1].f;
        if (fx->numoptions>=3) filter = (int)fx->option[2].i;
        if (fx->numoptions>=4) wrap   = fx->option[3].i?true:false;
    }
    if (scale<=0.01f)
        scale = 600.0f/(float)fx->source[0].ginfo->width;

    // 8bpp and 32 bpp versions of the source.
    unsigned char* pcSrc = (unsigned char*)fx->source[0].image;
    ARGB* pSrc = (ARGB*)fx->source[0].image;
    ARGB* d = (ARGB*)dest;

    // for each pixel, generate a vector normal that represents
    // the change in the height field at that pixel
    for (y = 0; y< height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            float dX, dY, nX, nY, nZ, oolen;
            int iNX, iNY, iNZ;
            float scale255=scale/255.0f;

            if (!filter) // 3x3
            {
                // Do Y Sobel filter

                dY  = EMBM_getpixel(fx->source,x-1,y+1,true,wrap)*scale255 * -1.0f;
                dY += EMBM_getpixel(fx->source,x+0,y+1,true,wrap)*scale255 * -2.0f;
                dY += EMBM_getpixel(fx->source,x+1,y+1,true,wrap)*scale255 * -1.0f;
                dY += EMBM_getpixel(fx->source,x-1,y-1,true,wrap)*scale255 *  1.0f;
                dY += EMBM_getpixel(fx->source,x+0,y-1,true,wrap)*scale255 *  2.0f;
                dY += EMBM_getpixel(fx->source,x+1,y-1,true,wrap)*scale255 *  1.0f;

                // Do X Sobel filter

                dX  = EMBM_getpixel(fx->source,x-1,y-1,true,wrap)*scale255 * -1.0f;
                dX += EMBM_getpixel(fx->source,x-1,y+0,true,wrap)*scale255 * -2.0f;
                dX += EMBM_getpixel(fx->source,x-1,y+1,true,wrap)*scale255 * -1.0f;
                dX += EMBM_getpixel(fx->source,x+1,y-1,true,wrap)*scale255 *  1.0f;
                dX += EMBM_getpixel(fx->source,x+1,y+0,true,wrap)*scale255 *  2.0f;
                dX += EMBM_getpixel(fx->source,x+1,y+1,true,wrap)*scale255 *  1.0f;

                // Cross Product of components of gradient reduces to
                nX = -dX;
                nY = -dY;
                nZ = 1;

                // Normalize
                oolen = 1.0f/((float) sqrt(nX*nX + nY*nY + nZ*nZ));
                nX *= oolen;
                nY *= oolen;
                nZ *= oolen;

                iNX = EMBM_clamp(127.0f*nX);    // -128..127
                iNY = EMBM_clamp(127.0f*nY);
                iNZ = EMBM_clamp(127.0f*nZ);
            }
            else // 1x1 simple difference with scaling
            {
                dX =  EMBM_getpixel(fx->source,x+0,y+0,true,wrap)*scale *  1.0f;
                dX += EMBM_getpixel(fx->source,x-1,y+0,true,wrap)*scale * -1.0f;

                dY  = EMBM_getpixel(fx->source,x+0,y+0,true,wrap)*scale *  1.0f;
                dY += EMBM_getpixel(fx->source,x+0,y-1,true,wrap)*scale * -1.0f;

                float dZ =  EMBM_getpixel(fx->source,x+0,y+0,true,wrap);

                iNX = EMBM_clamp(dX);    // -128..127
                iNY = EMBM_clamp(dY);
                iNZ = EMBM_clamp(dZ);
            }

            switch (mode)
            {
                default:
                case EMBM_UV_88:                 // do the IA version.
                    d->a = (unsigned char)iNX;
                    d->r = (unsigned char)iNY;
                    d->g = (unsigned char)iNY;
                    d->b = (unsigned char)iNY;
                    break;

                case EMBM_UVL_888:               // RGB values get UVL
                    d->a = (unsigned char)255;
                    d->r = (unsigned char)iNX;
                    d->g = (unsigned char)iNY;
                    d->b = (unsigned char)iNZ;
                    break;

                case EMBM_UVL_556:               // iNZ gets 6 bits strange!
                    d->a = (unsigned char)255;
                    d->r = (char)(iNX>>3);
                    d->g = (char)(iNY>>3);
                    d->b = (char)(iNZ>>2);
                    break;

                case EMBM_GSH:                   // note 2 channel UV_88
                    d->a = (unsigned char)iNX+128;
                    d->r = (unsigned char)iNY+128;
                    d->g = (unsigned char)iNY+128;
                    d->b = (unsigned char)iNY+128;
                    break;

#if 0
                case EMBM_GSH:
                    iNX += 128;
                    iNY += 128;
                    if (iNX > 255)
                        iNX = 255;
                    if (iNY > 255)
                        iNY = 255;
                    if (iNX < 0)
                        iNX = 0;
                    if (iNY < 0)
                        iNY = 0;
                    d->a = (unsigned char)255;
                    d->r = (unsigned char)iNX;
                    d->g = (unsigned char)iNY;
                    d->b = (unsigned char)iNZ;
#endif
            }

            d++;
            pSrc++;
            pcSrc++;
        }
    }

    return(1);                                          // return success.
}

FPUBLIC int closeembm(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

#undef Cross
#undef Normalize

