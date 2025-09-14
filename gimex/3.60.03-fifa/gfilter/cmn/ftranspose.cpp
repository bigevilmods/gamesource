// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    transpose - Filter                            */
/*                                                                  */
/*                    by Ian H. Davids, EAC                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010529 ID Original                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* transpose()                                                      */
/*                                                                  */
/* Same as doing a vflip and a 270 degree rotate                    */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ftranspose.h"
#include <string.h>

//
/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *abouttrans(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('T','R','A','N');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Trans" );
        strcpy(about->longnamestr,  "transposeImage" );
        about->numoptions =         0;
/*
        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "mode");
*/
     }
    return(about);
}

FPUBLIC int opentrans(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infotrans(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    int temp;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        temp = ginfo->width;                      // swap width and height.
        ginfo->width  = ginfo->height;            // but everything else is
        ginfo->height = temp;                     // the same.
    }
    return(ginfo);
}

FPUBLIC int filtertrans(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    //int rowbytesold = ginfo->width*ginfo->bpp/8;
    //int rowbytesnew = ginfo->height*ginfo->bpp/8;
    unsigned char *cs       = ((unsigned char *) fx->source[0].image);
    unsigned char *cd       = ((unsigned char *) dest);
    ARGB *sd                = ((ARGB *) dest);
    ARGB *ss                = ((ARGB *) fx->source[0].image);
    for (y=0; y<ginfo->width; ++y)                                 // work in original image so width and height are flipped.
    {
        for (x=0; x<ginfo->height; ++x)                            // now process this row.
        {
            if(ginfo->bpp <= 8)
                cd[x*ginfo->width + y] = cs[y*ginfo->height + x];  // ginfo->height is the width for the source bitmap.
            else
                sd[x*ginfo->width + y] = ss[y*ginfo->height + x];
        }

    }
    return(1);                                                     // return success.
}

FPUBLIC int closetrans(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

