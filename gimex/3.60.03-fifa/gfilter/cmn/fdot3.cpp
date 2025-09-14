// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Dot3 - Bumpmap Gen Filter                     */
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
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* dot3(float scale, int filter, int wrap)                          */
/* float scale;   scale height field.  600/width is good default    */
/* int filter;    0=3x3, 1=point                                    */
/* int wrap;      0=clamp, 1=wrap                                   */
/*                                                                  */
/* Converts height field to dot3 normalmap                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fdot3.h"
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

// REAL2INT
static int DOT3_REAL2INT(float x)
{
    return ((int)(((float)(x))+((((float)(x))>=0.0f)?0.5f:-0.5f)));
}

static unsigned char DOT3_clamp(float f)
{
    int i = DOT3_REAL2INT(f);
    i = gmax(0,gmin(255,i));
    return (unsigned char) i;
}

float DOT3_getpixel(GBITMAP *g,int x, int y, bool grey, bool wrap)
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

FPUBLIC FABOUT *aboutdot3(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('D','O','T','3');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard & Ian Davids");
        strcpy(about->versionstr,   "1.06");
        strcpy(about->namestr,      "Dot3" );
        strcpy(about->longnamestr,  "Dot3 NormalMap" );
        about->numoptions =         3;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           -100.0f;
        about->option[0].max.f =           100.0f;
        about->option[0].def.f =           5.0f;
        strcpy(about->option[0].name,      "height scale");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           1;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "filter method");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           1;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "wrap mode");
     }
    return(about);
}

FPUBLIC int opendot3(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infodot3(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->bpp = 32;                    // always 32bpp after an DOT3 conversion.
        ginfo->originalbpp = 32;
        ginfo->alphabits = 8;               // alpha stores height
        ginfo->redbits = 8;                 // x
        ginfo->greenbits = 8;               // y
        ginfo->bluebits = 8;                // z (typically nearly straight up = 255)
    }
    return(ginfo);
}




FPUBLIC int filterdot3(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i = 0;
    int j = 0;
    int width  = ginfo->width;
    int height = ginfo->height;
//  int bpp    = fx->source[0].ginfo->bpp;//ginfo->bpp;
//  int pixelBytes = (bpp / 8);
//  int rowBytes   = (width * pixelBytes);

    float scale  = 0;
    int filter   = 0;
    bool wrap    = false;
    if (fx->option)
    {
        if (fx->numoptions>=1) scale = (float)fx->option[0].f;
        if (fx->numoptions>=2) filter = (int)fx->option[1].i;
        if (fx->numoptions>=3) wrap   = fx->option[2].i?true:false;
    }
    if (scale<=0.01f)
        scale = 600.0f/(float)fx->source[0].ginfo->width;

    // 8bpp and 32 bpp versions of the source.
    unsigned char* pcSrc = (unsigned char*)fx->source[0].image;
    ARGB* pSrc = (ARGB*)fx->source[0].image;
    ARGB* pDst = (ARGB*)dest;
//  ARGB* P    = ginfo->colourtbl;

    double Mag = 0;

    // for each pixel, generate a vector normal that represents
    // the change in the height field at that pixel
    for( j = 0; j< height; j++  )
    {
        for( i = 0; i < width; i++ )
        {
            Vec vPoint00;
            Vec vPoint10;
            Vec vPoint01;
            Vec vPointn10;
            Vec vPointn01;
            Vec v10;
            Vec v01;
            Vec vNormal;
            unsigned int iHeight00 = 0;
            float fHeight00 = 0;
            float fHeight10 = 0;
            float fHeight01 = 0;
            float fHeightn10 = 0;
            float fHeightn01 = 0;

            iHeight00  = (int) DOT3_getpixel(fx->source,i+0,j+0,true,wrap);
            fHeight00  = DOT3_getpixel(fx->source,i+0,j+0,true,wrap)/255.0f;
            fHeight10  = DOT3_getpixel(fx->source,i+1,j+0,true,wrap)/255.0f;
            fHeight01  = DOT3_getpixel(fx->source,i+0,j+1,true,wrap)/255.0f;
            fHeightn10 = DOT3_getpixel(fx->source,i-1,j+0,true,wrap)/255.0f;
            fHeightn01 = DOT3_getpixel(fx->source,i+0,j-1,true,wrap)/255.0f;

            fHeight00 *= scale;
            fHeight10 *= scale;
            fHeight01 *= scale;
            fHeightn10 *= scale;
            fHeightn01 *= scale;

            vPoint00.x = i+0.0f;  vPoint00.y = j+0.0f;  vPoint00.z = fHeight00;
            vPoint10.x = i+1.0f;  vPoint10.y = j+0.0f;  vPoint10.z = fHeight10;
            vPoint01.x = i+0.0f;  vPoint01.y = j+1.0f;  vPoint01.z = fHeight01;
            vPointn10.x = i-1.0f;  vPointn10.y = j+0.0f;  vPointn10.z = fHeightn10;
            vPointn01.x = i+0.0f;  vPointn01.y = j-1.0f;  vPointn01.z = fHeightn01;

            if (!filter) // 3x3
            {
                v10.x = vPoint10.x - vPointn10.x;
                v10.y = vPoint10.y - vPointn10.y;
                v10.z = vPoint10.z - vPointn10.z;

                v01.x = vPoint01.x - vPointn01.x;
                v01.y = vPoint01.y - vPointn01.y;
                v01.z = vPoint01.z - vPointn01.z;
            }
            else // 1x1
            {
                v10.x = vPoint10.x - vPoint00.x;
                v10.y = vPoint10.y - vPoint00.y;
                v10.z = vPoint10.z - vPoint00.z;

                v01.x = vPoint01.x - vPoint00.x;
                v01.y = vPoint01.y - vPoint00.y;
                v01.z = vPoint01.z - vPoint00.z;
            }

            Cross( &vNormal, &v10, &v01 );

            vNormal.x =  vNormal.x;
            vNormal.y =  vNormal.y;
            vNormal.z =  vNormal.z;

            Normalize( &vNormal, &vNormal, Mag );

            // store the normal as an RGBA value in the normal map

            pDst->a = DOT3_clamp((float)iHeight00);
            pDst->r = DOT3_clamp((127.0f*vNormal.x) + 128.0f);
            pDst->g = DOT3_clamp((127.0f*vNormal.y) + 128.0f);
            pDst->b = DOT3_clamp((127.0f*vNormal.z) + 128.0f);
            pDst++;
            pSrc++;
            pcSrc++;
        }
    }

    return(1);                                          // return success.
}

FPUBLIC int closedot3(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

#undef Cross
#undef Normalize

