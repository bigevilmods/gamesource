// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    UNEmbm - Bumpmap UnGen Filter                 */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010510 ID Original Spun off from frank stereogram        */
/*    1.01 010601 ID added the new gsh format                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* unembm(int type)                                                 */
/* int type;     type of bump map. 1=uvl 888                        */
/*                                 2=uv  88                         */
/*                                 3=uvl 556                        */
/*                                 6=biased by 128 unsigned char gsh*/
/* Converts embm bumpmap to height field                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "funembm.h"
#include <string.h>


#define EMBM_UVL_888 1
#define EMBM_UV_88   2
#define EMBM_UVL_556 3

#define EMBM_GSH     6

//#define CLAMP(x) ((((signed int)(x)) < 0)?0:((x>255)?255:x))


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutunembm(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('E','M','B','M');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "UnEmbm" );
        strcpy(about->longnamestr,  "UnEmbm Image" );
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "mode");

     }
    return(about);
}

FPUBLIC int openunembm(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infounembm(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int mode = EMBM_UV_88;
    if (fx->option)
        if (fx->numoptions>=1 && fx->option[0].i > 0) mode = fx->option[0].i;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
                    // always 32bpp after an UnEMBM conversion.
    }
    return(ginfo);
}

FPUBLIC int filterunembm(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    unsigned int  height   = 127;
    unsigned char *s       = ((unsigned char *) fx->source[0].image);
    ARGB *left             = 0;
    ARGB *top              = 0;
    ARGB *d                = ((ARGB *) dest);
    ARGB *destCol          = 0;
    ARGB *destColV         = 0;

    int mode = EMBM_UV_88;
    if (fx->option)
    {
        if (fx->numoptions>=1 && fx->option[0].i > 0) mode = fx->option[0].i;
        if (fx->numoptions>=2 && fx->option[1].i > 0)
        {
            height = fx->option[1].i;
            if(height)
                height = (int)(255.0*((float)height/100.0));
            else
                height = 127;
        }
    }

    memset(d,0,ginfo->width*ginfo->height*4);
    for (y=0; y<ginfo->height; ++y)
    {
        left    = (ARGB *)s - 1;                      // get the right pixel.
        top     = (ARGB *)s - ginfo->width;
        destCol  = d - 2;
        destColV = d - (ginfo->width*2);
        for (x=0; x<ginfo->width; ++x)               // now process this row.
        {
            signed int u = 0;
            signed int v = 0;
            signed int currentCol = height;
            if(x == 0 || (x == 1 && y == 0))
            {
                d->r = (unsigned char)height;
                d->g = (unsigned char)height;
                d->b = (unsigned char)height;
                d->a = (unsigned char)255;
            }
            else if(x == 1)
            {
                if(mode == EMBM_UV_88)                      // do the IA version.
                {
                    u  = height  - (signed char)(left->a)/2;
                    if(y > 1)
                        v  = destColV->r - (signed char)(top->r);
                }
                else if(mode == EMBM_UVL_888)               // RGB values get UVL
                {
                    u  = height  - (signed char)(left->r)/2;
                    if(y > 1)
                        v  = destColV->r - (signed char)(top->g);

                }
                else if(mode == EMBM_UVL_556)               // L gets 6 bits strange!
                {
                    u  = height  - ((signed char)(left->r))/2;
                    if(y > 1)
                        v  = destColV->r - ((signed char)(top->g));

                }
                else if(mode == EMBM_GSH)
                {
                    u  = height  + (((int)(left->r) - 128))/2;
                    if(y > 1)
                        v  = destColV->r + (((int)(top->g) - 128));

                }

                if(y > 1)
                {
                    currentCol = (u+v)/2;
                    if(currentCol < 0) currentCol = 0;
                    if(currentCol > 255) currentCol = 255;

                    d->r = (unsigned char)(currentCol);
                    d->g = (unsigned char)(currentCol);
                    d->b = (unsigned char)(currentCol);
                    d->a = (unsigned char)255;

                }
                else
                {
                    currentCol = u;
                    if(currentCol < 0) currentCol = 0;
                    if(currentCol > 255) currentCol = 255;

                    d->r = (unsigned char)(currentCol);
                    d->g = (unsigned char)(currentCol);
                    d->b = (unsigned char)(currentCol);
                    d->a = (unsigned char)255;
                }
            }
            else
            {
                if(mode == EMBM_UV_88)                      // do the IA version.
                {
                    u  = destCol->r  - (signed char)(left->a);
                    if(y > 1)
                        v  = destColV->r - (signed char)(top->r);
                }
                else if(mode == EMBM_UVL_888)               // RGB values get UVL
                {
                    u  = (unsigned char)destCol->r  - (signed char)(left->r);
                    if(y > 1)
                        v  = destColV->r - (signed char)(top->g);

                }
                else if(mode == EMBM_UVL_556)               // L gets 6 bits strange!
                {
                    u  = destCol->r  - (signed char)(left->r);
                    if(y > 1)
                        v  = destColV->r - (signed char)(top->g);

                }
                else if(mode == EMBM_GSH)               // RGB values get UVL
                {
                    u  = destCol->r  + ((int)(left->r) - 128);
                    if(y > 1)
                        v  = destColV->r + ((int)(top->g) - 128);

                }

                if(y > 1)
                {
                    currentCol = (u+v)/2;
                    if(currentCol < 0) currentCol = 0;
                    if(currentCol > 255) currentCol = 255;

                    d->r = (unsigned char)(currentCol);
                    d->g = (unsigned char)(currentCol);
                    d->b = (unsigned char)(currentCol);
                    d->a = (unsigned char)255;

                }
                else
                {
                    currentCol = u;
                    if(currentCol < 0) currentCol = 0;
                    if(currentCol > 255) currentCol = 255;

                    d->r = (unsigned char)(currentCol);
                    d->g = (unsigned char)(currentCol);
                    d->b = (unsigned char)(currentCol);
                    d->a = (unsigned char)255;
                }
            }
            d++;
            destColV++;
            destCol++;
            left++;
            top++;
        }
        s += rowbytes;                                  // jump to next line.
    }
    return(1);                                          // return success.
}

FPUBLIC int closeunembm(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

#undef EMBM_UVL_888
#undef EMBM_UV_88
#undef EMBM_UVL_556
#undef EMBM_GSH
