// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                          RGB to HSV Cix                          */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960207 FB based on fhsv filter                           */
/*    1.01 960209 FB rewrote in float based on stolfi m3 code       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* hsv(rgb, hsv)                                                    */
/* H  Hue angle 0 red, 1/3 green, 2/3 blue                          */
/* S  Saturation 0 grey, 1 colour                                   */
/* V  Value 0 black, 1 maximum max(r,g,b)                           */
/*                                                                  */
/* ie hues break into 3 ranges                                      */
/*    R = 1 (max)                                                   */
/*    RGB 1,1,0  gc=0,bc=1 h=1   normalize to 1/6                   */
/*    RGB 1,0,0  gc=1,bc=1 h=0                                      */
/*    RGB 1,0,1  gc=1,bc=0 h=-1  normalize to -1/6                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <math.h>
#include "gfilter.h"
#include "hsv.h"

/****************************************************************/
/* CiX Functions                                                */
/****************************************************************/

void rgb_hsv(const double rgb[3], double hsv[3])
{
    double r,g,b;
    double h,s,v;
    double max, min;

    r = rgb[0];
    g = rgb[1];
    b = rgb[2];

    max = gmax(r,gmax(g,b));
    min = gmin(r,gmin(g,b));
    v = max;

    if (max != 0.0)  s = (max - min) / max;
    else             s = 0.0;

    if (s == 0.0)    h = 0.0;
    else
    {
        if (r == max)       h = (g - b) / (max - min);
        else if (g == max)  h = 2.0 + (b - r) / (max - min);
        else                h = 4.0 + (r - g) / (max - min);
        h = h / 6.0;
        if (h < 0.0) h = h + 1.0;
    }

    hsv[0] = gmax(0.0,gmin(1.0,h));
    hsv[1] = gmax(0.0,gmin(1.0,s));
    hsv[2] = gmax(0.0,gmin(1.0,v));
}

#define triplet(x,y,z) r=x; g=y; b=z

void hsv_rgb(const double hsv[3], double rgb[3])
{
    double h, s, v;
    double p, q, t;
    double r,g,b;
    double f;
    int i;

    r = 0;
    g = 0;
    b = 0;

    h = hsv[0];
    s = hsv[1];
    v = hsv[2];

    if (v<=0)
    {
        triplet(0, 0, 0);
    }
    else
    {
        if (s<=0)
        {
            triplet(v, v, v);           /* achromatic case */
        }
        else
        {                               /* chromatic color: hue */
            i = (int) floor(h*6.0);
            f = (h*6.0 - (double)i); /* fractional part of h (0..1)*/

            p = v * (1.0 - s);
            q = v * (1.0 - s * f);
            t = v * (1.0 - s * (1.0-f));

            switch (i)
            {                               /* triplet assignment */
                case 0: triplet(v, t, p); break;
                case 1: triplet(q, v, p); break;
                case 2: triplet(p, v, t); break;
                case 3: triplet(p, q, v); break;
                case 4: triplet(t, p, v); break;
                case 5: triplet(v, p, q); break;
            }
        }
    }

    rgb[0] = gmax(0.0,gmin(1.0,r));
    rgb[1] = gmax(0.0,gmin(1.0,g));
    rgb[2] = gmax(0.0,gmin(1.0,b));
}

