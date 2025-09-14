// (c) 2001-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   Equalize - Equalize Filter                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010712 FB based on fgrey                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* equalize(int channels)                                           */
/* int channels;   which channels to equalize as a 4 bit mask argb  */
/*                                                                  */
/* Equalizes channels to maximum range                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fequalize.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define EQUALIZE8(c) ((c)->r*0.29900f + (c)->g*0.58700f +  (c)->b*0.11400f)
#define EQUALIZE(c) (unsigned char) ((EQUALIZE8(c)*levels + 128)/256)

static void equalize(unsigned char *dest, unsigned char *source, ARGB *colourtbl, int n, int bpp, int levels)
{
    int i;

    for (i=0; i<n; ++i)
    {
        if (bpp<=8)
        {
            *dest++ = EQUALIZE(colourtbl+*source);
            ++source;
        }
        else
        {
            *dest++ = EQUALIZE((ARGB *)source);
            source += 4;
        }
    }
}

#undef EQUALIZE
#undef EQUALIZE8

/* intlog2: returns integer log to the base 2 of a number. */

static int EQUALIZEintlog2(unsigned int l)
{
    int i;

    i = 0;
    while (l>1)
    {
        l >>= 1;
        ++i;
    }
    return(i);
}



/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutequalize(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('G','R','E','Y');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.04");
        strcpy(about->namestr,      "Equalize" );
        strcpy(about->longnamestr,  "Equalize Image" );
        about->numoptions =         2;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           2;
        about->option[0].max.i =           256;
        about->option[0].def.i =           256;
        strcpy(about->option[0].name,      "Levels");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           1;
        about->option[1].def.i =           0;
        strcpy(about->option[0].name,      "BlendAlpha");
     }
    return(about);
}

FPUBLIC int openequalize(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoequalize(FINSTANCE *fx, int framenum)
{
    int  levels=256;
    int  blendalpha=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) levels = fx->option[0].i;
        if (fx->numoptions>=2) blendalpha = fx->option[1].i;
        if (levels<2) levels=256;
    }

    GINFO *ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ginfo->originalbpp = EQUALIZEintlog2(levels-1)+1;
        ginfo->bpp         = 8;
        ginfo->startcolour = 0;
        ginfo->numcolours  = levels;
        ginfo->alphabits   = 0;
        ginfo->redbits     = 8;
        ginfo->greenbits   = 8;
        ginfo->bluebits    = 8;

        memset(ginfo->colourtbl,0,sizeof(ginfo->colourtbl));

        for (int i=0; i<levels; ++i)
        {
            if (blendalpha)
                ginfo->colourtbl[i].a = (unsigned char) ((i*255+levels/2)/(levels-1));
            else
                ginfo->colourtbl[i].a = 255;

            ginfo->colourtbl[i].r = (unsigned char) ((i*255+levels/2)/(levels-1));
            ginfo->colourtbl[i].g = (unsigned char) ((i*255+levels/2)/(levels-1));
            ginfo->colourtbl[i].b = (unsigned char) ((i*255+levels/2)/(levels-1));
        }
    }

    return(ginfo);
}



FPUBLIC int filterequalize(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  levels=256;

    if (fx->option)
    {
        if (fx->numoptions>=1) levels = fx->option[0].i;
        if (levels<2) levels=256;
    }


    for (i=0; i<ginfo->height; ++i)
    {
        equalize((unsigned char *) dest,(unsigned char *) source,fx->source[0].ginfo->colourtbl,width,fx->source[0].ginfo->bpp,levels);
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closeequalize(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

