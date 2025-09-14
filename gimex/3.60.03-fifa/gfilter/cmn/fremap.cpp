// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Remap - Remap Image to Specified Palette           */
/*                                                                  */
/*                by Frank Barchard and Gerry Shaw, EAC             */
/*                                                                  */
/*                    Filter Module - Jul 17, 1996                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950524 GS initial version based on gcolour module        */
/*    1.01 950831 GS added remap helper function                    */
/*    1.02 951107 FB 1.04 API and doc                               */
/*    1.03 951113 FB MrC warning fixes                              */
/*    1.04 960717 GS startcolor respected when inversetable present */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* remap(FPALETTE *pal)                                             */
/* FPALETTE *pal;    palette to map to                              */
/*                                                                  */
/* Takes a 32 bit bitmap and palette, returns 8 bit image remaped   */
/* to the given palette.                                            */
/*                                                                  */
/* Notes                                                            */
/* should update to make fop optional. remap to default palette     */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "fremap.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define SQR(x) (x)*(x)

static int REMAPgetcolour(ARGB *rgbval, FPALETTE *pal)
{
    ARGB *  clr;
    int     i, n, BestDiff, best=0;
    unsigned int    finda = (int) rgbval->a;
    unsigned int    findr = (int) rgbval->r;
    unsigned int    findg = (int) rgbval->g;
    unsigned int    findb = (int) rgbval->b;
    unsigned int    ind;
    unsigned int    inr, ing, inb;

    char *inversetbl = (char*)(pal->inversetbl);

    #define WUBITS 6
    #define WULEVELS    (1<<WUBITS)

    if (pal->inversetbl)
    {
        inr=(findr>>(8-WUBITS));
        ing=(findg>>(8-WUBITS));
        inb=(findb>>(8-WUBITS));

        ind= inr*WULEVELS*WULEVELS + ing*WULEVELS + inb;

        return(inversetbl[ind]) + pal->startcolour;
    }

    /* full search for best colour */

    BestDiff = 0x7fffffff;
    n = pal->numcolours;
    for (i = 0, clr = pal->colourtbl; (i < n) && (BestDiff != 0); ++i, ++clr)
    {
        int a = (int)clr->a;
        int r = (int)clr->r;
        int g = (int)clr->g;
        int b = (int)clr->b;
        int Diff = SQR((finda-a)*4)+SQR(findr-r)+SQR(findg-g)+SQR(findb-b);
        if (Diff < BestDiff)
        {
            BestDiff = Diff;
            best = i;
        }
    }

    return(best+pal->startcolour);
}

static int filter32to8(GBITMAP *source, GBITMAP *dest, FPALETTE *pal)
{
    int width  = dest->ginfo->width;
    int height = dest->ginfo->height;

    int x;
    int y;

    for (y = 0; y < height; ++y)
    {
        ARGB *s = (ARGB*)((char *)(source->image) + y*(source->rowbytes));
        char *d = (char *)dest->image + y*(dest->rowbytes);

        for (x = 0; x < width; ++x)
        {
            *d++ = (char)REMAPgetcolour(s, pal);
            ++s;
        }
    }

    return(1);
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutremap(void)
{
    FABOUT* about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('R','E','M','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "F.Barchard & G.Shaw");
        strcpy(about->versionstr,   "1.04");
        strcpy(about->namestr,      "Remap");
        strcpy(about->longnamestr,  "Remap 32 to 8");
        about->numoptions =         1;

        about->option[0].type =            FOP_PTR | FOP_FPALETTE;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Destination Palette");
        strcpy(about->option[0].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openremap(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *inforemap(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        /* modify unique fields of dest */
        ginfo->bpp             = 8;
        ginfo->originalbpp     = 8;
        ginfo->numcolours      = 256;

        memset(ginfo->colourtbl, 0, 256*sizeof(ARGB));

        /* copy palette into ginfo */
        {
            FPALETTE *pal = (FPALETTE*)fx->option[0].p;

            char * dest   = (char*)(ginfo->colourtbl) + pal->startcolour*sizeof(ARGB);
            char * source = (char*)(pal->colourtbl);
            size_t count  = pal->numcolours*sizeof(ARGB);

            memcpy(dest, source, count);
        }

    }
    return(ginfo);
}

FPUBLIC int filterremap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    GBITMAP dbm;

    dbm.ginfo    = ginfo;
    dbm.image    = dest;
    dbm.rowbytes = rowbytes;

    return(filter32to8(fx->source, &dbm, (FPALETTE*)(fx->option->p)));
}

FPUBLIC int closeremap(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

