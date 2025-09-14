// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       HSV - HSV Adjust                           */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960204 FB based on invert                                */
/*    1.01 960209 FB based on cix module for colour conversion      */
/*    1.02 970710 GS if sat is zero, don't touch add to it          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* hsv(float H, float S, float B, float V)                          */
/* H  Hue adjust angle (-180..180)                                  */
/* S  Saturation adjust level (-100..100)                           */
/* B  Brightness adjust level (-100..100)                           */
/* V  Value adjust level (-100..100)                                */
/* For S,B,V -100 means 100 percent toward minimum value            */
/*            100 means 100 percent toward maximum value            */
/* Value of 0 for H,S,B,V means no change                           */
/*                                                                  */
/* HSV/HSB adjust ARGB values (either palette or image)             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fhsv.h"
#include "hsv.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static void hsv(ARGB *dest,int n,double dh, double ds, double db, double dv)
{
    int i;
    double rgb[3];
    double hsv[3];

    for (i=0; i<n; ++i)
    {
        rgb[0] = dest->r/255.0;
        rgb[1] = dest->g/255.0;
        rgb[2] = dest->b/255.0;

        rgb_hsv(rgb,hsv);

        hsv[0] = hsv[0]+dh/360.0;
        if (hsv[0] <  0.0) hsv[0] += 1.0;
        if (hsv[0] >= 1.0) hsv[0] -= 1.0;

        if (ds<0)
            hsv[1] =  hsv[1]*(ds+100.0)/100.0;
        else
        {
            if (hsv[2] != 0.0)
            {
                /* only add to saturation if saturation is not zero */
                /* adding saturation to a number with no saturation makes no sense and */
                /* will end up adding an undefined tint to the color */
                hsv[1] = (hsv[1]*(100.0-ds)+ds)/100.0;
            }
        }
        hsv[1] = gmax(0.0,gmin(1.0,hsv[1]));

        if (dv<0)   hsv[2] =  hsv[2]*(dv+100.0)/100.0;
        else        hsv[2] = (hsv[2]*(100.0-dv)+dv)/100.0;
        hsv[2] = gmax(0.0,gmin(1.0,hsv[2]));

        hsv_rgb(hsv,rgb);


        if (db<0)
        {
            rgb[0] =  rgb[0]*(db+100.0)/100.0;
            rgb[1] =  rgb[1]*(db+100.0)/100.0;
            rgb[2] =  rgb[2]*(db+100.0)/100.0;
        }
        else
        {
            rgb[0] = (rgb[0]*(100.0-db)+db)/100.0;
            rgb[1] = (rgb[1]*(100.0-db)+db)/100.0;
            rgb[2] = (rgb[2]*(100.0-db)+db)/100.0;
        }
        rgb[0] = gmax(0.0,gmin(1.0,rgb[0]));
        rgb[1] = gmax(0.0,gmin(1.0,rgb[1]));
        rgb[2] = gmax(0.0,gmin(1.0,rgb[2]));

        dest->r = (unsigned char)(rgb[0]*255.0);
        dest->g = (unsigned char)(rgb[1]*255.0);
        dest->b = (unsigned char)(rgb[2]*255.0);

        ++dest;

    }
}



/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *abouthsv(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','H','S','V');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.02");
        strcpy(about->namestr,      "HSV");
        strcpy(about->longnamestr,  "HSV Adjust");
        about->numoptions =         4;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           -180.0;
        about->option[0].max.f =           180.0;
        about->option[0].def.f =           0;
        strcpy(about->option[0].name,      "Hue");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -100.0;
        about->option[1].max.f =           100.0;
        about->option[1].def.f =           0;
        strcpy(about->option[1].name,      "Saturation");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_FLOAT;
        about->option[2].min.f =           -100.0;
        about->option[2].max.f =           100.0;
        about->option[2].def.f =           0;
        strcpy(about->option[2].name,      "Brightness");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_FLOAT;
        about->option[3].min.f =           -100.0;
        about->option[3].max.f =           100.0;
        about->option[3].def.f =           0;
        strcpy(about->option[2].name,      "Value");
        strcpy(about->option[2].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openhsv(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *) galloc(sizeof(FINSTANCE));
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

FPUBLIC GINFO *infohsv(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    double h=0;
    double s=0;
    double b=0;
    double v=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) h = fx->option[0].f;
        if (fx->numoptions>=2) s = fx->option[1].f;
        if (fx->numoptions>=3) b = fx->option[2].f;
        if (fx->numoptions>=4) v = fx->option[3].f;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        hsv(ginfo->colourtbl,256,h,s,b,v);
    }
    return(ginfo);
}

FPUBLIC int filterhsv(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    double h=0;
    double s=0;
    double b=0;
    double v=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) h = fx->option[0].f;
        if (fx->numoptions>=2) s = fx->option[1].f;
        if (fx->numoptions>=3) b = fx->option[2].f;
        if (fx->numoptions>=4) v = fx->option[3].f;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            hsv((ARGB *)dest,width,h,s,b,v);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closehsv(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

