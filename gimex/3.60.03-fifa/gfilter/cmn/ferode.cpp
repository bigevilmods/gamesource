// (c) 2000-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                     Erode - Maximize Filter                      */
/*                                                                  */
/*                   by Alistair Milne, Bullfrog                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990816 AM converted to gimex format                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* erode(int radius)                                                */
/*                                                                  */
/* Choose the darkest channel value in a (radius*2)+1 size box      */
/* surrounding the current pixel.                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ferode.h"
#include <string.h>


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *abouterode(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('E','R','O','D');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Alistair Milne");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Erode");
        strcpy(about->longnamestr,  "Erode (Minimize)");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.f =           1;
        about->option[0].max.f =           256;
        about->option[0].def.f =           1;
        strcpy(about->option[0].name,      "Radius");
        strcpy(about->option[0].enumlist,  "");
    }
    return(about);
}

FPUBLIC int openerode(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    /* allocate instance */
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

FPUBLIC GINFO *infoerode(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filtererode(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    unsigned char *buffer, *rSrc, *rDst, *pSrc, *pDst, *pEnd;
    int channel, x, y, min;
    int ok=0;
    int size=1;
    int width  = ginfo->width;
    int height = ginfo->height;

    if (fx->option)
    {
        size = fx->option[0].i;
    }

    buffer = (unsigned char*)( galloc(height));
    if (buffer)
    {
        ok = 1;


        for ( channel=0; channel<4; ++channel)
        {
            /* pass 1 - horizontal minimize */
            rDst = (unsigned char*)(dest) + channel;
            rSrc = (unsigned char*)(fx->source[0].image) + channel;

            for ( y=0; y<height; ++y)
            {
                pDst = rDst;

                for ( x=0; x<width; ++x)
                {
                    pSrc = rSrc + 4*gmax( x-size, 0);
                    pEnd = rSrc + 4*gmin( x+size, width-1);

                    min = 0xFF;
                    while ( pSrc <= pEnd)
                    {
                        if ( *pSrc < min)
                            min = *pSrc;
                        pSrc += 4;
                    }

                    *pDst = min;
                    pDst += 4;
                }

                rDst += rowbytes;
                rSrc += fx->source[0].rowbytes;
            }

            /* pass 2 - vertical minimize */
            rDst = (unsigned char*)(dest) + channel;

            for ( x=0; x<width; ++x)
            {
                pDst = rDst;
                pSrc = rDst;

                for ( y=0; y<height; ++y)
                    buffer[y] = pSrc[y * fx->source[0].rowbytes];

                for ( y=0; y<height; ++y)
                {
                    pSrc = buffer + gmax( y-size, 0);
                    pEnd = buffer + gmin( y+size, height-1);

                    min = 0xFF;
                    while ( pSrc <= pEnd)
                    {
                        if ( *pSrc < min)
                            min = *pSrc;
                        pSrc++;
                    }

                    *pDst = min;
                    pDst += rowbytes;
                }

                rDst += 4;
            }
        }

        gfree( (void*)(buffer));
    }

    return(ok);
}

FPUBLIC int closeerode(FINSTANCE *fx)
{
    return (gfree((void*)(fx)));
}

