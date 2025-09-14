// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                FYI - Filter YIq/alpha adjustment                 */
/*                                                                  */
/*            by FrANK G. Barchard and James Hall, EAC              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 021019 FB based on hsv & James Hall expressions          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* fyi(YScale, YOffset, CScale, COffset, AScale, AOffset)           */
/* YScale   Luminance Scale                                         */
/* YOffset  Luminance Offset                                        */
/* CScale   Chrominance Scale                                       */
/* COffset  Chrominance Offset                                      */
/* AScale   Alpha Scale                                             */
/* AOffset  Alpha Offset                                            */
/*                                                                  */
/* YIQ adjust image for ps2 to/from xbox adjustment                 */
/*
;
; Xbox -> PS2 GX texture conversion filter
; Written by James Hall (jamesh@ea.com) for Triple Play 2003
; Based on an application used for 007, helpfully supplied by
; Colin Doncaster.
;
; This filter converts all the pixels in a texture using the following
; equations from the Microsoft Tech document outlining the differences
; between the PS2 and Xbox regarding video signals:
;
; Luminance:    PS2Value = 0.925 * XboxValue + 19.25
; Chrominance:  PS2Value = XboxValue / 1.115
;
; (for future reference, 1/1.115 ~= 0.8968)
;
; Since texture data is in RGB space, it is first converteed to YIQ
; space using the equations
;
; Y = 0.299R + 0.587G + 0.114B
; I = 0.596R - 0.275G - 0.321B
; Q = 0.212R - 0.523G + 0.311B
;
; The above luminance equation is then applied to the Y channel, the
; above chrominance equation is applied to the I and Q channels.
;
; The pixels are then converted back to RGB space using:
;
; R = Y + 0.956I + 0.621Q
; G = Y - 0.272I - 0.647Q
; B = Y - 1.105I + 1.702Q
;
; Unfortunately, GX's "-pixel" command does not allow you to create
; variables, so we must merge all the above math into a single
; command.  Using basic algebra, we get:
;
; Y' = 0.925Y + 19.25
; I' = 0.8968I
; Q' = 0.8968Q
;
; R = Y' + 0.956I' + 0.621Q'
;   = ( 0.925Y + 19.25 ) + ( 0.956 * 0.8968 * I ) + ( 0.621 * 0.8968 * Q )
;   = ( 0.925Y + 19.25 ) + ( 0.8573408 * I ) + ( 0.5569128 * Q )
;   = ( 0.925 * ( 0.299R + 0.587G + 0.114B ) + 19.25 )
;   + ( 0.8573408 * ( 0.596R - 0.275G - 0.321B ) )
;   + ( 0.5569128 * ( 0.212R - 0.523G + 0.311B ) )
;   = ( 0.276575R + 0.542975G + 0.10545B + 19.25 )
;   + ( 0.5109751168R - 0.23576872G - 0.2752063968B )
;   + ( 0.1180655136R - 0.2912653944G + 0.1731998808B )
;   = 0.9056156304R + 0.0159408856G + 0.003443484B + 19.25
;
; G = Y' - 0.272I' - 0.647Q'
;   = ( 0.925Y + 19.25 ) - ( 0.272 * 0.8968 * I ) - ( 0.647 * 0.8968 * Q )
;   = ( 0.925Y + 19.25 ) - ( 0.2439296 * I ) - ( 0.5802296 * Q )
;   = ( 0.925 * ( 0.299R + 0.587G + 0.114B ) + 19.25 )
;   - ( 0.2439296 * ( 0.596R - 0.275G - 0.321B ) )
;   - ( 0.5802296 * ( 0.212R - 0.523G + 0.311B ) )
;   = ( 0.276575R + 0.542975G + 0.10545B + 19.25 )
;   - ( 0.1453820416R - 0.06708064G - 0.0783014016B )
;   - ( 0.1230086752R - 0.3034600808G + 0.1804514056B )
;   = 0.0081842832R + 0.9135157208G + 0.003299996B + 19.25
;
; B = Y' - 1.1105I' + 1.702Q'
;   = ( 0.925Y + 19.25 ) - ( 1.105 * 0.8968 * I ) + ( 1.702 * 0.8968 * Q )
;   = ( 0.925Y + 19.25 ) - ( 0.990964 * I ) + ( 1.5263536 * Q )
;   = ( 0.925 * ( 0.299R + 0.587G + 0.114B ) + 19.25 )
;   - ( 0.990964 * ( 0.596R - 0.275G - 0.321B ) )
;   + ( 1.5263536 * ( 0.212R - 0.523G + 0.311B ) )
;   = ( 0.276575R + 0.542975G + 0.10545B + 19.25 )
;   - ( 0.590614544R - 0.2725151G - 0.318099444B )
;   + ( 0.3235869632R - 0.7982829328G + 0.4746959696B )
;   = 0.0095474192R + 0.0172071672G + 0.8982454136B + 19.25
;
; Leading to this GX -pixel command:
;
"-pixela,'(0.9056156304*r)+(0.0159408856*g)+(0.003443484*b)+19.25','(0.0081842832*r)+(0.9135157208*g)+(0.003299996*b)+19.25','(0.0095474192*r)+(0.0172071672*g)+(0.8982454136*b)+19.25'"
*/
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fyi.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static __inline unsigned char YIQ_round(float v)
{
    int i = (int)(v);   //+0.5f);  scale factors from microsoft have rounding included

    return (unsigned char)(gmax(0,gmin(255,i)));
}


static void fyi(ARGB *dest,int n,float YScale, float YOffset, float CScale, float COffset, float AScale, float AOffset)
{
    for (int i=0; i<n; ++i)
    {
        float A = (float)dest->a;
        float Y = 0.299f * (float)dest->r + 0.587f * (float)dest->g + 0.114f * (float)dest->b;
        float I = 0.596f * (float)dest->r - 0.275f * (float)dest->g - 0.321f * (float)dest->b;
        float Q = 0.212f * (float)dest->r - 0.523f * (float)dest->g + 0.311f * (float)dest->b;

        A = A*AScale + AOffset;
        Y = Y*YScale + YOffset;
        I = I*CScale + COffset;
        Q = Q*CScale + COffset;

        float R = Y + 0.956f*I + 0.621f*Q;
        float G = Y - 0.272f*I - 0.647f*Q;
        float B = Y - 1.105f*I + 1.702f*Q;

        dest->a = YIQ_round(A);
        dest->r = YIQ_round(R);
        dest->g = YIQ_round(G);
        dest->b = YIQ_round(B);
        ++dest;
    }
}



/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutfyi(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','Y','I','Q');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "FYI");
        strcpy(about->longnamestr,  "FYI Adjust Luma/Chroma/Alpha");
        about->numoptions =         6;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           -2.0;
        about->option[0].max.f =           2.0;
        about->option[0].def.f =           1;
        strcpy(about->option[0].name,      "Luminance Scale");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -256.0;
        about->option[1].max.f =           256.0;
        about->option[1].def.f =           0;
        strcpy(about->option[1].name,      "Luminance Offset");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_FLOAT;
        about->option[2].min.f =           -2.0;
        about->option[2].max.f =           2.0;
        about->option[2].def.f =           1;
        strcpy(about->option[2].name,      "Chrominance Scale");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_FLOAT;
        about->option[3].min.f =           -256.0;
        about->option[3].max.f =           256.0;
        about->option[3].def.f =           0;
        strcpy(about->option[2].name,      "Chrominance Offset");
        strcpy(about->option[2].enumlist,  "");

        about->option[4].type =            FOP_FLOAT;
        about->option[4].min.f =           -2.0;
        about->option[4].max.f =           2.0;
        about->option[4].def.f =           1;
        strcpy(about->option[4].name,      "Alpha Scale");
        strcpy(about->option[4].enumlist,  "");

        about->option[5].type =            FOP_FLOAT;
        about->option[5].min.f =           -256.0;
        about->option[5].max.f =           256.0;
        about->option[5].def.f =           0;
        strcpy(about->option[5].name,      "Alpha Offset");
        strcpy(about->option[5].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openfyi(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infofyi(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    float YScale  = 1.0f;
    float YOffset = 0.0f;
    float CScale  = 1.0f;
    float COffset = 0.0f;
    float AScale  = 1.0f;
    float AOffset = 0.0f;

    if (fx->option)
    {
        if (fx->numoptions>=1) YScale  = (float) fx->option[0].f;
        if (fx->numoptions>=2) YOffset = (float) fx->option[1].f;
        if (fx->numoptions>=3) CScale  = (float) fx->option[2].f;
        if (fx->numoptions>=4) COffset = (float) fx->option[3].f;
        if (fx->numoptions>=5) AScale  = (float) fx->option[4].f;
        if (fx->numoptions>=6) AOffset = (float) fx->option[5].f;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        fyi(ginfo->colourtbl,256,YScale,YOffset,CScale,COffset,AScale,AOffset);
    }
    return(ginfo);
}

FPUBLIC int filterfyi(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int  width  = ginfo->width;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    float YScale  = 1.0f;
    float YOffset = 0.0f;
    float CScale  = 1.0f;
    float COffset = 0.0f;
    float AScale  = 1.0f;
    float AOffset = 0.0f;

    if (fx->option)
    {
        if (fx->numoptions>=1) YScale  = (float) fx->option[0].f;
        if (fx->numoptions>=2) YOffset = (float) fx->option[1].f;
        if (fx->numoptions>=3) CScale  = (float) fx->option[2].f;
        if (fx->numoptions>=4) COffset = (float) fx->option[3].f;
        if (fx->numoptions>=5) AScale  = (float) fx->option[4].f;
        if (fx->numoptions>=6) AOffset = (float) fx->option[5].f;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32)
            fyi((ARGB *)dest,width,YScale,YOffset,CScale,COffset,AScale,AOffset);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closefyi(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

