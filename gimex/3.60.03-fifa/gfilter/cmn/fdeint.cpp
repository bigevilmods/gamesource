// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Deint - Deinterlacer Filter                   */
/*                                                                  */
/*                         by Gerry Shaw, EAC                       */
/*                                                                  */
/*                    Filter Module - Apr 30, 1996                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960430 GS initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* deint(scanline)                                                  */
/* int scanline; 0=remove odd lines,1=remove even lines             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Future Additions:                                                */
/*                                                                  */
/* add method option; 0=duplicate,1=interpolate                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fdeint.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutdeint(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('D','I','N','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "G Shaw");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Deinterlace");
        strcpy(about->longnamestr,  "Deinterlace");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT | FOP_ENUM;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Scan Line");
        strcpy(about->option[0].enumlist,  "Odd,Even");
    }
    return(about);
}

FPUBLIC int opendeint(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

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
    return(ok);
}


FPUBLIC GINFO *infodeint(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filterdeint(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=1;
    int evenlines=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) evenlines = fx->option[0].i;
    }

    /* copy src into dst, 2 lines at a time */
    {
        char *s = (char *) fx->source[0].image;
        char *d = dest;
        int y;
        int bpl = ginfo->width*ginfo->bpp/8;

        if (evenlines)
            s += fx->source[0].rowbytes;

        for (y = 0; y < ginfo->height; y += 2)
        {
            memcpy(d,s,bpl);
            d += rowbytes;
            memcpy(d,s,bpl);
            d += rowbytes;

            s += fx->source[0].rowbytes;
            s += fx->source[0].rowbytes;
        }
    }

    return(ok);
}

FPUBLIC int closedeint(FINSTANCE *fx)
{
    return(gfree((void*)fx));
}


