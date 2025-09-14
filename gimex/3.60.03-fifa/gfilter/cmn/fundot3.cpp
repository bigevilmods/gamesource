// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    UnDot3 - UnBumpMap Gen Filter                 */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010510 ID Original Spun off from embm filter             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* dot3(int mode)                                                   */
/* Converts normalmap to heightfield                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fundot3.h"
#include <string.h>
#if defined(_MSC_VER)
#pragma warning(disable:4514)
#endif
#include <math.h>

#define DOT3_LUMINANCE 0
#define DOT3_UNIFORM   1


#define CONVTOGREY32(s)               ((((ARGB *) s)->r*299+((ARGB *) s)->g*587+((ARGB *) s)->b*114)/1000)
#define CONVTOGREY8(s,p)              ((p[s].r*299+p[s].g*587+p[s].b*114)/1000)
#define Cross(A,V1,V2)                {(A)->x = ((V1)->y*(V2)->z - (V1)->z*(V2)->y);                                 \
                                       (A)->y = ((V1)->z*(V2)->x - (V1)->x*(V2)->z);                                 \
                                       (A)->z = ((V1)->x*(V2)->y - (V1)->y*(V2)->x);                                 }
#define Normalize(V1,V2,Temp)         {(Temp = 1.0/sqrt((V2)->x*(V2)->x + (V2)->y*(V2)->y + (V2)->z*(V2)->z));(V1)->x = (float)((V2)->x*Temp);(V1)->y = (float)((V2)->y*Temp);(V1)->z = (float)((V2)->z*Temp);}


#define CopyVec(V1,V2)                {(V1).x = (V2).r; (V1).y = (V2).g; (V1).z = (V2).b;}
#define FillVec(V1,V2)                {(V1).x = (V2); (V1).y = (V2); (V1).z = (V2);}



typedef struct Vec
{
    float x;
    float y;
    float z;
}Vec;


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutundot3(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('U','D','O','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "UnDot3" );
        strcpy(about->longnamestr,  "UnDot3 NormalMap to height" );
        about->numoptions =         0;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "mode");

     }
    return(about);
}

FPUBLIC int openundot3(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoundot3(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->bpp = 32;                     // always assume 32bpp after an DOT3 conversion.
    }
    return(ginfo);
}




FPUBLIC int filterundot3(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i = 0;
    int j = 0;
    int width = ginfo->width;
    int height = ginfo->height;
    double fCurHeightx   = .5;
    double fCurHeighty   = .5;
    double fCurTop       = .5;
    double defaultv      = .5;

    ARGB* pSrc = (ARGB*)fx->source[0].image;
    ARGB* pDst = (ARGB*)dest;
    ARGB* pSrcTop = (ARGB*)dest;
    ARGB* pDstTop = (ARGB*)dest;
    Vec vNormalL;
    Vec vNormal;
    Vec vTop;
    if (fx->option)
        if (fx->numoptions>=1) defaultv = fCurHeightx = fCurHeighty = fx->option[0].f;

    // for each pixel, generate a vector normal that represents
    // the change in the height field at that pixel
    CopyVec(vNormal,*pSrc);
    CopyVec(vNormalL,*pSrc);
    CopyVec(vTop,*pSrc);
    for( j = 0; j< height; j++  )
    {
        if(j == 0)
        {
            fCurTop = defaultv;
            fCurHeightx = defaultv;
        }
        else//(j !=0)
        {
            pSrcTop = (pSrc - width);
            pDstTop = (pDst - width);
            vTop = vNormalL;
            fCurTop = fCurHeighty;

        fCurHeighty =  fCurHeighty - (-((vNormalL.y - 128.0)/127.0))/((vNormalL.z - 128.0)/127.0);
        if(fCurHeighty > 1)
            fCurHeighty = 1;
        if(fCurHeighty < 0)
            fCurHeighty = 0;
        fCurHeightx = fCurHeighty;
        }
        CopyVec(vNormalL,*pSrc);
        for( i = 0; i < width; i++ )
        {
            //if(i == 0)
            //{
            //    fCurHeightx = .5;
            //}
            if(i != 0)            // first time through the loop use the down vector of the item above.
            {
                fCurHeightx = fCurHeightx - (-((vNormal.x - 128.0)/127.0))/((vNormal.z - 128.0)/127.0);
                if(fCurHeightx > 1)
                    fCurHeightx = 1;
                if(fCurHeightx < 0)
                    fCurHeightx = 0;
            }
            if(j == 0)
            {
                fCurHeightx = (fCurTop + fCurHeightx)/2.0;
                if(fCurHeightx > 1)
                    fCurHeightx = 1;
                if(fCurHeightx < 0)
                    fCurHeightx = 0;

            }
            else //(j != 0)
            {
                fCurTop = (pDstTop->r/255.0) - (-((vTop.y - 128.0)/127.0))/((vTop.z - 128.0)/127.0);
                fCurHeightx = (fCurTop + fCurHeightx)/2.0;
                pSrcTop++;
                pDstTop++;
                CopyVec(vTop,*pSrcTop);
                if(fCurHeightx > 1)
                    fCurHeightx = 1;
                if(fCurHeightx < 0)
                    fCurHeightx = 0;

            }
            pDst->a = 255;
            pDst->r = pDst->g = pDst->b = (unsigned char)(fCurHeightx * 255.0);
            CopyVec(vNormal,*pSrc);
            pDst++;
            pSrc++;
        }
    }
    return(1);                                          // return success.
}

FPUBLIC int closeundot3(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

#undef DOT3_LUMINANCE
#undef DOT3_UNIFORM
#undef CONVTOGREY32
#undef CONVTOGREY8
#undef Cross
#undef Normalize
#undef CopyVec
