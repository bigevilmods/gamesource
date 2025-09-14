// (c) 1996-2007 Electronic Arts Inc.

#define DEBUGTEMP 1
#if DEBUGTEMP
#include <stdio.h>
#endif

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               Temporalfix - WMV Temporal fixup Filter            */
/*                                                                  */
/*                       by Frank Barchard, EAC                     */
/*                                                                  */
/*                    Filter Module - Aug 02, 2006                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 060802 FB initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* temporalfix()                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* WMV with avisynth replaces missing frames due to temporal        */
/* compression with grey frames.                                    */
/* This filter detects that and replaces the frame with the         */
/* previous one.                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ftemporalfix.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *abouttemporalfix(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('T','E','M','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Temporalfix");
        strcpy(about->longnamestr,  "Temporal fix");
        about->numoptions =         0;
    }
    return(about);
}

GBITMAP *gbitmap=NULL;
int gNumTemporalFix=0;
void InitTemporal()
{
    gbitmap = (GBITMAP *) galloc(sizeof(GBITMAP));
    gbitmap->ginfo = NULL;
    gbitmap->image = NULL;
    gbitmap->rowbytes = 0;
}

void RestoreTemporal()
{
    if (gbitmap)
    {
        gfree((void *)gbitmap->ginfo);
        gfree((void *)gbitmap->image);
        gfree((void *)gbitmap);
    }
}


FPUBLIC int opentemporalfix(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

        if (!gbitmap)
            InitTemporal();

        ok = 1;
    }
    *filterInstance = fx;
    return(ok);
}


FPUBLIC GINFO *infotemporalfix(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

#define ISGREY(v) (v>=0x70 && v<=0x90) || v==0
bool IsTemporal(ARGB *s)
{
    return ISGREY(s->r) && ISGREY(s->g) && ISGREY(s->b);
}

void CopyImage(GBITMAP *dest, GBITMAP *source)
{
    memcpy(dest->ginfo, source->ginfo, source->ginfo->size);
    memcpy(dest->image, source->image, source->rowbytes*source->ginfo->height);
    dest->rowbytes = source->rowbytes;
}

FPUBLIC int filtertemporalfix(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=1;
    int evenlines=0;

    if (!gbitmap->ginfo)
    {
        gbitmap->ginfo = (GINFO *) galloc(ginfo->size);
        gbitmap->image = galloc(rowbytes*ginfo->height);
        gbitmap->rowbytes = rowbytes;
        CopyImage(gbitmap, &fx->source[0]);
        memset(gbitmap->image,0,rowbytes*ginfo->height); // set to black if its the first frame
    }

    /* scan for temporal image */
    bool istemporal=true;
    for (int y = 0; y < ginfo->height && istemporal; ++y)
    {
        char *s = (char *) fx->source[0].image + y*rowbytes;
        for (int x = 0; x < ginfo->width; ++x)
        {
            if (!IsTemporal((ARGB *)s))
            {
                istemporal = false;
                break;
            }
            s += 4;
        }
    }

    GBITMAP destbitmap;
    destbitmap.ginfo = ginfo;
    destbitmap.image = dest;
    destbitmap.rowbytes = rowbytes;

    if (istemporal)
    {
        CopyImage(&destbitmap, gbitmap);
        ++gNumTemporalFix;
#if DEBUGTEMP
        printf("temporal fix on frame %d\n",ginfo->framenum);
#endif

    }
    else
    {
        CopyImage(&destbitmap,&fx->source[0]);
        CopyImage(gbitmap,&fx->source[0]);
    }

    return(ok);
}

FPUBLIC int closetemporalfix(FINSTANCE *fx)
{
    return(gfree((void*)fx));
}


