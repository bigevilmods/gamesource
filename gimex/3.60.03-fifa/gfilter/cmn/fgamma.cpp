// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 Gamma - Gamma Correction Filter                  */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 980317 FB based on grey                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* gamma(float alpha, float red, float green, float blue)           */
/* float alpha; gamma correction on alpha channel                   */
/* float red;   gamma correction on red channel                     */
/* float green; gamma correction on green channel                   */
/* float blue;  gamma correction on blue channel                    */
/*                                                                  */
/* Gamma corrects image by normalizing channels 0..1, then raising  */
/* to power and renormalizizing back to 0..255                      */
/*                                                                  */
/* Hector Yee @ ears uses this formula                              */
/*                                                                  */
/* void DX8Wrapper::Set_Gamma(float gamma,float bright,float        */
/* contrast,bool calibrate)                                         */
/* {                                                                */
/*  bright=Bound(bright,-0.5f,0.5f);                                */
/*  contrast=Bound(contrast,0.5f,2.0f);                             */
/*  gamma=Bound(gamma,0.6f,6.0f);                                   */
/*  float oo_gamma=1.0f/gamma;                                      */
/*                                                                  */
/*  DX8_Assert();                                                   */
/*  number_of_DX8_calls++;                                          */
/*                                                                  */
/*  DWORD flag=(calibrate?D3DSGR_CALIBRATE:D3DSGR_NO_CALIBRATION);  */
/*                                                                  */
/*  D3DGAMMARAMP ramp;                                              */
/*                                                                  */
/*  float limit=(contrast-1)/2*contrast;                            */
/*                                                                  */
/*  for (int i=0; i<255; i++)                                       */
/*  {                                                               */
/*   float in,out;                                                  */
/*   in=i/255.0f;                                                   */
/*   float x=in-limit;                                              */
/*   x=Bound(x,0.0f,1.0f);                                          */
/*   x=powf(x,oo_gamma);                                            */
/*   out=contrast*x+bright;                                         */
/*   out=Bound(out,0.0f,1.0f);                                      */
/*   ramp.red[i]=(int) (out*65535);                                 */
/*   ramp.green[i]=(int) (out*65535);                               */
/*   ramp.blue[i]=(int) (out*65535);                                */
/*  }                                                               */
/*                                                                  */
/*  if (DX8Caps::Support_Gamma())                                   */
/*  {                                                               */
/*   DX8Wrapper::_Get_D3D_Device8()->SetGammaRamp(flag,&ramp);      */
/*  } else                                                          */
/*  {                                                               */
/*   HWND hwnd = GetDesktopWindow();                                */
/*   HDC hdc = GetDC(hwnd);                                         */
/*   if (hdc)                                                       */
/*   {                                                              */
/*    BOOL bRes = SetDeviceGammaRamp (hdc, &ramp);                  */
/*    ReleaseDC (hwnd, hdc);                                        */
/*   }                                                              */
/*  }                                                               */
/* }                                                                */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fgamma.h"
#include <string.h>
#include <math.h>


#if defined(__APPLE__)
inline float powf(float b, float e)
{
    return (float) pow((double) b, (double) e);
}
#endif

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void gammachannel(unsigned char *dest,int pix,float gamma)
{
    int i;

    for (i=0; i<pix; ++i)
    {
        *dest = (unsigned char)(powf(((float)(*dest))/255.0f,gamma)*255.0f);
        dest += sizeof(ARGB);
    }
}

static void gammarow(ARGB *dest,int pix,float ga,float gr,float gg,float gb)
{
    gammachannel(&dest->a,pix,ga);
    gammachannel(&dest->r,pix,gr);
    gammachannel(&dest->g,pix,gg);
    gammachannel(&dest->b,pix,gb);
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutgamma(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('G','A','M','A');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Gamma" );
        strcpy(about->longnamestr,  "Gamma Correct" );
        about->numoptions =         4;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           -4.0f;
        about->option[0].max.f =           4.0f;
        about->option[0].def.f =           1.0f;
        strcpy(about->option[0].name,      "AlphaGamma");

        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -4.0f;
        about->option[1].max.f =           4.0f;
        about->option[1].def.f =           1.0f;
        strcpy(about->option[1].name,      "RedGamma");

        about->option[2].type =            FOP_FLOAT;
        about->option[2].min.f =           -4.0f;
        about->option[2].max.f =           4.0f;
        about->option[2].def.f =           1.0f;
        strcpy(about->option[2].name,      "GreenGamma");

        about->option[3].type =            FOP_FLOAT;
        about->option[3].min.f =           -4.0f;
        about->option[3].max.f =           4.0f;
        about->option[3].def.f =           1.0f;
        strcpy(about->option[3].name,      "BlueGamma");
     }
    return(about);
}

FPUBLIC int opengamma(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infogamma(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    float ga=1.0f;
    float gr=1.0f;
    float gg=1.0f;
    float gb=1.0f;

    if (fx->option)
    {
        if (fx->numoptions>=1) ga = (float) fx->option[0].f;
        if (fx->numoptions>=2) gr = (float) fx->option[1].f;
        if (fx->numoptions>=3) gg = (float) fx->option[2].f;
        if (fx->numoptions>=4) gb = (float) fx->option[3].f;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        gammarow(ginfo->colourtbl,256,ga,gr,gg,gb);
    }
    return(ginfo);
}

FPUBLIC int filtergamma(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    float ga=1.0f;
    float gr=1.0f;
    float gg=1.0f;
    float gb=1.0f;

    if (fx->option)
    {
        if (fx->numoptions>=1) ga = (float) fx->option[0].f;
        if (fx->numoptions>=2) gr = (float) fx->option[1].f;
        if (fx->numoptions>=3) gg = (float) fx->option[2].f;
        if (fx->numoptions>=4) gb = (float) fx->option[3].f;
    }


    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            gammarow((ARGB *)dest,width,ga,gr,gg,gb);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closegamma(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

