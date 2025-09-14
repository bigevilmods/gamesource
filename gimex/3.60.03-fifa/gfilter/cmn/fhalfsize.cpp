// (c) 2000-2007 Electronic Arts Inc.

#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#endif

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*              Halfsize - A Fast Bilinear Image Halfsize           */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 000218 FB based on fscale                                */
/*    1.01 000223 FB scale and step for deinterlace                 */
/*    1.02 000225 FB framesize updated                              */
/*    1.03 000329 FB position scaled                                */
/*    1.04 010314 FB clamp to 1 pixel minimum                       */
/*    1.05 020221 FB dot3 renormalize                               */
/*    1.06 020222 FB dot3 double angle                              */
/*    1.07 020722 FB omp complient by moving variables inside loop  */
/*    1.08 030226 FB 8 bit map once using CRAPI_getcolour           */
/*    1.09 070605 FB omp, alloca, edge else bug, get/put 32 bit     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* halfsize(int direction, int vscale,vstep,hstep,edge,dot3)        */
/* int direction; 0=none, 1=vertical, 2=horizontal, 3=both          */
/* int vscale; 0=auto, 1=asis, 2=half vertical height               */
/* int hscale; 0=auto, 1=asis, 2=half vertical width                */
/* int vstep;  0=auto, n=n source lines per dest (1 or 2)           */
/* int hstep;  0=auto, n=n source pixels per dest (1 or 2)          */
/* int edge;   0=no edge sharpen, 1=sharpen                         */
/* int dot3;   0=no, 1=dot3 renormalize x/y/z                       */
/*                                                                  */
/* Halfsize reduces an image to half size with option for           */
/* vertical, horizontal or both.                                    */
/* Good for:                                                        */
/*   mipmapping                                                     */
/*   cramming                                                       */
/*   deinterlacing                                                  */
/*   mipmap noise comparison                                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fhalfsize.h"
#include "../../crapi/cmn/crapi.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER) || defined(__R5900)
#include <malloc.h>
#elif defined(__PPU__)
#include <alloca.h>
#else
#include <stdlib.h>
#endif

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define BLEND2(dest,source,source2) *(unsigned char *)(dest) = (unsigned char) ((int)(*(unsigned char *)(source) + (int)*(unsigned char *)(source2))>>1)


#define Normalize(V1,V2,Temp)         {(Temp = 1.0f/sqrt((V2)->x*(V2)->x + (V2)->y*(V2)->y + (V2)->z*(V2)->z));(V1)->x = (float)((V2)->x*Temp);(V1)->y = (float)((V2)->y*Temp);(V1)->z = (float)((V2)->z*Temp);}
typedef struct Vec
{
    float x;
    float y;
    float z;
} Vec;

static unsigned char DOT3_clamp(float f)
{
    int i = (int) f;
    i = gmax(0,gmin(255,i));
    return (unsigned char) i;
}

static void NORMALIZEDOT3(ARGB *colour)
{
    double Mag = 0.0;

    Vec vNormal;

    vNormal.x =  ((float)colour->r)-128.0f;
    vNormal.y =  ((float)colour->g)-128.0f;
    vNormal.z =  (((float)colour->b)-128.0f)/2.0f;
//    vNormal.z =  ((float)colour->b)-128.0f;

    Normalize( &vNormal, &vNormal, Mag );

    // store the normal as an RGBA value in the normal map

//    colour->a = DOT3_clamp(iHeight00);
    colour->r = DOT3_clamp((127.0f*vNormal.x) + 128.0f);
    colour->g = DOT3_clamp((127.0f*vNormal.y) + 128.0f);
    colour->b = DOT3_clamp((127.0f*vNormal.z) + 128.0f);
}
#undef Normalize

// convert 8/32 to 32
static char * HALF_getline(char *dest, const char *source, GINFO *ginfo)
{
    // 32 bpp blend
    if (ginfo->bpp>8)
    {
        return (char *)source;
//        memcpy(dest,source,ginfo->width*sizeof(ARGB));
    }
    else
    {
        char *d = dest;
        for (int i=0; i<ginfo->width; ++i)
        {
            *((ARGB *) d) = ginfo->colourtbl[*(unsigned char *)source];
            d += 4;
            source += 1;
        }
        return dest;
    }
}

// convert 32 to 8/32
static void HALF_putline(char *dest, const char *source, GINFO *ginfo)
{
    // 32 bpp blend
    if (ginfo->bpp>8)
    {
        memcpy(dest,source,ginfo->width*sizeof(ARGB));
    }
    else
    {
        for (int i=0; i<ginfo->width; ++i)
        {
            *dest = CRAPI_getcolour(*(ARGB *)source, ginfo->numcolours, ginfo->colourtbl);
            dest += 1;
            source += 4;
        }
    }
}


#define POINTSAMPLE_THRESH 32
static void HALF_blend2(char *dest,const char *source,const char *source2, int pixeladvance,int numpixels,int edge,int dot3)
{
    if (!edge && !dot3) // simple & optimized case
    {
        // simple box filter

        for (int i=0; i<numpixels; ++i)
        {
            BLEND2(&((ARGB *)dest)->a, &((ARGB *)source)->a, &((ARGB *)source2)->a);
            BLEND2(&((ARGB *)dest)->r, &((ARGB *)source)->r, &((ARGB *)source2)->r);
            BLEND2(&((ARGB *)dest)->g, &((ARGB *)source)->g, &((ARGB *)source2)->g);
            BLEND2(&((ARGB *)dest)->b, &((ARGB *)source)->b, &((ARGB *)source2)->b);

            dest += 4;
            source += pixeladvance;
            source2 += pixeladvance;
        }
    }
    else
    {
        // 32 bpp blend

        for (int i=0; i<numpixels; ++i)
        {
            int a=((ARGB *)source)->a;
            int a2=((ARGB *)source2)->a;
            if (edge && a<POINTSAMPLE_THRESH && a2>256-POINTSAMPLE_THRESH)
            {
                BLEND2(&((ARGB *)dest)->a, &((ARGB *)source)->a, &((ARGB *)source2)->a);
                ((ARGB *)dest)->r = ((ARGB *)source2)->r;
                ((ARGB *)dest)->g = ((ARGB *)source2)->g;
                ((ARGB *)dest)->b = ((ARGB *)source2)->b;
            }
            else if (edge && a2<POINTSAMPLE_THRESH && a>256-POINTSAMPLE_THRESH)
            {
                BLEND2(&((ARGB *)dest)->a, &((ARGB *)source)->a, &((ARGB *)source2)->a);
                ((ARGB *)dest)->r = ((ARGB *)source)->r;
                ((ARGB *)dest)->g = ((ARGB *)source)->g;
                ((ARGB *)dest)->b = ((ARGB *)source)->b;
            }
            else
            {
                BLEND2(&((ARGB *)dest)->a, &((ARGB *)source)->a, &((ARGB *)source2)->a);
                BLEND2(&((ARGB *)dest)->r, &((ARGB *)source)->r, &((ARGB *)source2)->r);
                BLEND2(&((ARGB *)dest)->g, &((ARGB *)source)->g, &((ARGB *)source2)->g);
                BLEND2(&((ARGB *)dest)->b, &((ARGB *)source)->b, &((ARGB *)source2)->b);
                if (dot3)
                    NORMALIZEDOT3((ARGB *)dest);
            }
            dest += 4;
            source += pixeladvance;
            source2 += pixeladvance;
        }
    }
}

#undef BLEND2

static void HALF_image(char *d, char *source, int sourcerowbytes, GINFO *sourceginfo, GINFO *ginfo, int direction, int edge, int dot3, int y, int vstep)
{
    int  bpl    = ginfo->width*ginfo->bpp/8;
    int  bpl32  = sourceginfo->width*4;

    char *linebuf0 = (char *) alloca(bpl32);
    char *linebuf1 = (char *) alloca(bpl32);
    char *lineptr0;
    char *lineptr1;

    char *destbuf = d;
    if (ginfo->bpp==8)
        destbuf =  (char *) alloca(bpl32);

    switch (direction)
    {
        case 0:
            memcpy(d,source,bpl);
            break;

        case 1: // vertical
            lineptr0 = HALF_getline(linebuf0,source,sourceginfo);
            lineptr1 = HALF_getline(linebuf1,source+sourcerowbytes,sourceginfo);
            HALF_blend2(destbuf,lineptr0,lineptr1,sizeof(ARGB),ginfo->width,edge,dot3);
            if (ginfo->bpp==8)
                HALF_putline(d,destbuf,ginfo);
            break;

        case 2: // horizontal
            lineptr0 = HALF_getline(linebuf0,source,sourceginfo);
            HALF_blend2(destbuf,lineptr0,lineptr0+sizeof(ARGB),sizeof(ARGB)*2,ginfo->width,edge,dot3);
            if (ginfo->bpp==8)
                HALF_putline(d,destbuf,ginfo);
            break;

        case 3:
        {
            lineptr0 = HALF_getline(linebuf0,source,sourceginfo);
            if (y==(ginfo->height-1) && vstep==1) // odd height last line
            {
                HALF_blend2(destbuf,lineptr0,lineptr0+sizeof(ARGB),sizeof(ARGB)*2,ginfo->width,edge,dot3);
            }
            else
            {
                lineptr1 = HALF_getline(linebuf1,source+sourcerowbytes,sourceginfo);
                HALF_blend2(linebuf0,lineptr0,lineptr1,sizeof(ARGB),ginfo->width*2,edge,dot3);
                HALF_blend2(destbuf,linebuf0,linebuf0+sizeof(ARGB),sizeof(ARGB)*2,ginfo->width,edge,dot3);
            }
            if (ginfo->bpp==8)
                HALF_putline(d,destbuf,ginfo);
            break;
        }
    }
//    gfree(destbuf);
//    gfree(linebuf1);
//    gfree(linebuf0);
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* abouthalfsize(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('H','A','L','F');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.09" );
        strcpy(about->namestr,      "Halfsize" );
        strcpy(about->longnamestr,  "Halfsize Image" );

        about->numoptions =         7;
        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           3;
        strcpy(about->option[0].name,      "direction");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           2;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "vscale");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           2;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "hscale");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           2;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "vstep");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           0;
        about->option[4].max.i =           2;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "hstep");
        strcpy(about->option[4].enumlist,  "");

        about->option[5].type =            FOP_INT;
        about->option[5].min.i =           0;
        about->option[5].max.i =           1;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "edge");
        strcpy(about->option[5].enumlist,  "");

        about->option[6].type =            FOP_INT;
        about->option[6].min.i =           0;
        about->option[6].max.i =           1;
        about->option[6].def.i =           0;
        strcpy(about->option[6].name,      "dot3");
        strcpy(about->option[6].enumlist,  "");
    }
    return about;
}

FPUBLIC int openhalfsize(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok = 0;
    FINSTANCE* fx = 0;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
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
    return ok;
}

FPUBLIC GINFO* infohalfsize(FINSTANCE* fx, int framenum)
{
    GINFO *ginfo;
    unsigned int direction=3;
    unsigned int vscale=0;
    unsigned int hscale=0;
    unsigned int vstep=0;
    unsigned int hstep=0;
    unsigned int edge=0;
    unsigned int dot3=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) direction = fx->option[0].i;
        if (fx->numoptions>=2) vscale = fx->option[1].i;
        if (fx->numoptions>=3) hscale = fx->option[2].i;
        if (fx->numoptions>=4) vstep = fx->option[3].i;
        if (fx->numoptions>=5) hstep = fx->option[4].i;
        if (fx->numoptions>=6) edge = fx->option[5].i;
        if (fx->numoptions>=7) dot3 = fx->option[6].i;
    }
    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        int i;
        if (!vscale) vscale = (direction&1) ? 2 : 1;
        if (!hscale) hscale = (direction&2) ? 2 : 1;
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        ginfo->width /= hscale;
        ginfo->height /= vscale;
        ginfo->centerx /= hscale;
        ginfo->centery /= vscale;
        ginfo->defaultx /= hscale;
        ginfo->defaulty /= vscale;
        ginfo->framesize = ginfo->framesize/(vscale*hscale);
        if (ginfo->width<1)  ginfo->width = 1;
        if (ginfo->height<1) ginfo->height = 1;

        for (i=0; i<ginfo->numhotspots; ++i)
        {
            ginfo->hotspottbl[i][0] /= hscale;
            ginfo->hotspottbl[i][1] /= vscale;
        }
    }
    return(ginfo);
}

FPUBLIC int filterhalfsize(FINSTANCE* fx, GINFO* ginfo, char* dest, int rowbytes)
{
    unsigned int direction=3;
    unsigned int vscale=0;
    unsigned int hscale=0;
    unsigned int vstep=0;
    unsigned int hstep=0;
    unsigned int edge=0;
    unsigned int dot3=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) direction = fx->option[0].i;
        if (fx->numoptions>=2) vscale = fx->option[1].i;
        if (fx->numoptions>=3) hscale = fx->option[2].i;
        if (fx->numoptions>=4) vstep = fx->option[3].i;
        if (fx->numoptions>=5) hstep = fx->option[4].i;
        if (fx->numoptions>=6) edge = fx->option[5].i;
        if (fx->numoptions>=7) dot3 = fx->option[6].i;
    }

    if (!vscale) vscale = (direction&1) ? 2 : 1;
    if (!hscale) hscale = (direction&2) ? 2 : 1;
    if (!vstep) vstep = (direction&1) ? 2 : 1;
    if (!hstep) hstep = (direction&2) ? 2 : 1;

#if ENABLE_OMP
    #pragma omp parallel for
#endif
    for (int y=0; y<ginfo->height; ++y)
    {
        // inside loop for openmp.  could use private instead
        char *source= (char *)fx->source[0].image+y*fx->source[0].rowbytes*vstep;
        char *d=dest+y*rowbytes;

        HALF_image(d,source, fx->source[0].rowbytes, fx->source[0].ginfo, ginfo, direction, edge, dot3, y, vstep);

    }

    return(1);
}

FPUBLIC int closehalfsize(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
