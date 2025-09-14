// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       HSL - HSL Adjust                           */
/*                                                                  */
/*              by Gerry Shaw and Frank Barchard, EAC               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960306 GS based on hsv                                   */
/*    1.01 970610 FB rewritten based on hsv info/read hsl ordering  */
/*    1.02 970710 GS if sat is zero, don't touch add to it          */
/*    1.03 970829 GS palette mask indicates what colors to change   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* hsl(float H, float S, float L)                                   */
/* H  Hue adjust angle (-180..180)                                  */
/* S  Saturation adjust level (-100..100)                           */
/* L  Lightness adjust level (-100..100)                            */
/* For S,L -100 means 100 percent toward minimum value              */
/*         +100 means 100 percent toward maximum value              */
/* For H,S,L 0 means no change                                      */
/*                                                                  */
/* HSL adjust ARGB values (either palette or image)                 */
/*                                                                  */
/* mask can be NULL (default) or a pointer to 256 int table         */
/* indicating what colors in the palette to change, mask is not     */
/* used with truecolor images                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fhsl.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static double SetHSLValue(double n1, double n2, double hue)
{
    while (hue < 0.0)
        hue += 360.0;

    while (hue >= 360.0)
        hue -= 360.0;

    if (hue < 60.0)
        return n1 + (n2 - n1) * hue / 60.0;
    else if (hue < 180.0)
        return n2;
    else if (hue < 240.0)
        return n1 + (n2 - n1) * (240.0 - hue) / 60.0;

    return n1;
}


/****************************************************************/
/* CiX Functions                                                */
/****************************************************************/

static void rgb_hsl(const double rgb[3], double hsl[3])
{
    double r = rgb[0];
    double g = rgb[1];
    double b = rgb[2];

    double max = gmax(r, gmax(g, b));
    double min = gmin(r, gmin(g, b));

    double h=0.0;               /* hue */
    double l = (max + min)/2;   /* luminance */
    double s;                   /* saturation */

    if (max == min) /* achromatic case, because r == g == b */
    {
        s = 0.0;
        h = 0.0;    /* hue is undefined, should be NaN */
    }
    else /* chromatic case */
    {
        double delta;

        /* calculate saturation */
        if (l <= 0.5)
            s = (max - min)/(max + min);
        else
            s = (max - min)/(2 - max - min);

        /* calculate hue */
        delta = max - min;
        if (r == max)
            h = (g - b)/delta;      /* resulting colour is between yellow and magenta */
        else if (g == max)
            h = 2 + (b - r)/delta;  /* resulting colour is between cyan and yellow */
        else if (b == max)
            h = 4 + (r - g)/delta;  /* resulting colour is between magenta and cyan */

        /* convert h to be in [0,1] */
        h = h * 60;         /* convert to degrees */
        if (h < 0.0)
            h = h + 360;    /* make degrees be nonegative */
        h = h/360;
    }

    hsl[0] = h;
    hsl[1] = s;
    hsl[2] = l;
}

static void hsl_rgb(const double hsl[3], double rgb[3])
{
    double h = hsl[0];
    double s = hsl[1];
    double l = hsl[2];

    double m1;
    double m2;

    double r;
    double g;
    double b;


    if (l <= 0.5)
        m2 = l * (1.0 + s);
    else
        m2 = l + s - l*s;

    m1 = 2.0*l - m2;

    if (s == 0.0) /* achromatic case, there is no hue */
    {
        r = l;
        g = l;
        b = l;
    }
    else /* chromatic case, there is a hue */
    {
        h *= 360.0;

        r = SetHSLValue(m1, m2, h + 120.0);
        g = SetHSLValue(m1, m2, h);
        b = SetHSLValue(m1, m2, h - 120.0);
    }

    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

static void hsl(ARGB *dest,int n,double dh, double ds, double dl)
{
    int i;
    double rgb[3];
    double hsl[3];

    for (i=0; i<n; ++i)
    {
        rgb[0] = dest->r/255.0;
        rgb[1] = dest->g/255.0;
        rgb[2] = dest->b/255.0;

        rgb_hsl(rgb,hsl);

        hsl[0] = hsl[0]+dh/360.0;
        if (hsl[0] <  0.0) hsl[0] += 1.0;
        if (hsl[0] >= 1.0) hsl[0] -= 1.0;

        if (ds<0)
            hsl[1] =  hsl[1]*(ds+100.0)/100.0;
        else
        {
            if (hsl[1] != 0.0)
            {
                /* only add to saturation if saturation is not zero */
                /* adding saturation to a number with no saturation makes no sense and */
                /* will end up adding an undefined tint to the color */
                hsl[1] = (hsl[1]*(100.0-ds)+ds)/100.0;
            }
        }
        hsl[1] = gmax(0.0,gmin(1.0,hsl[1]));

        if (dl<0) hsl[2] =  hsl[2]*(dl+100.0)/100.0;
        else      hsl[2] = (hsl[2]*(100.0-dl)+dl)/100.0;
        hsl[2] = gmax(0.0,gmin(1.0,hsl[2]));

        hsl_rgb(hsl,rgb);

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

FPUBLIC FABOUT *abouthsl(void)
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
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.03" );
        strcpy(about->namestr,      "HSL");
        strcpy(about->longnamestr,  "HSL Adjust");
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
        strcpy(about->option[2].name,      "Lightness");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_PTR;
        about->option[3].def.f =           0;
        strcpy(about->option[3].name,      "Mask");
    }
    return(about);
}

FPUBLIC int openhsl(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infohsl(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    double h=0;
    double s=0;
    double l=0;
    int   *mask = NULL;

    if (fx->option)
    {
        if (fx->numoptions>=1) h = fx->option[0].f;
        if (fx->numoptions>=2) s = fx->option[1].f;
        if (fx->numoptions>=3) l = fx->option[2].f;
        if (fx->numoptions>=4) mask = (int*)(fx->option[3].p);
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        if (mask)
        {
            int i;
            for (i = 0; i < 256; i++)
            {
                if (mask[i])
                {
                    hsl(&(ginfo->colourtbl[i]),1,h,s,l);
                }
            }
        }
        else
        {
            hsl(ginfo->colourtbl,256,h,s,l);
        }
    }
    return(ginfo);
}

FPUBLIC int filterhsl(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    double h=0;
    double s=0;
    double l=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) h = fx->option[0].f;
        if (fx->numoptions>=2) s = fx->option[1].f;
        if (fx->numoptions>=3) l = fx->option[2].f;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            hsl((ARGB *)dest,width,h,s,l);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closehsl(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

