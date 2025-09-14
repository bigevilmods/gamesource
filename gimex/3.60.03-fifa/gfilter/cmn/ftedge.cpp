// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  Tedge - Transparent Edge Extrude                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951025 FB based on alpha copy                            */
/*    1.01 021219 FB use alpha for contribution factor              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* tedge(method)                                                    */
/* int method - reserved for future use. pass 0 or 1                */
/*                                                                  */
/* Blends RGB of surrounding colours into any transparent pixels    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ftedge.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static int GetScaler(int a)
{
    int scaler = 1;
    if (a>=64)
    {
        scaler = 2;
    }
    return scaler;
}

static void tedge(ARGB *dest,int width,int height,int rowbytes)
{
    int RTotal=0, GTotal=0, BTotal=0, scaler=1;
    ARGB *d;
    unsigned char *pByte;
    unsigned char R, G, B;
    unsigned char *pPixelProcessed, bAtLeastOne=0;
    int     i, j, nDivider=0;

    /* Allocate array of processed pixels */
    pPixelProcessed = (unsigned char *) galloc(width*height);
    memset(pPixelProcessed,0,width*height);

    /* Initialise array */
    for (i=0; i<height; ++i)
    {
        d = (ARGB *)(((unsigned char *)dest)+i*rowbytes);

        for (j=0; j<width; ++j)
        {
            /* If alpha is non-zero, pixel is processed */
            if (d->a)
            {
                pPixelProcessed[i*width + j] = 1;
                bAtLeastOne = 1;
            }
            ++d;
        }
    }

    /* Do multiple passes so that every pixel in the image is processed */
    while (bAtLeastOne)
    {
        /* Assume that no pixel is transparent in the beginning of the loop */
        bAtLeastOne=0;

        for (i=0; i<height; ++i)
        {
            /* Set pointers */
            pByte=((unsigned char *)dest+i*rowbytes);

            for (j=0; j<width; j++)
            {
                /* Is pixel already processed or not ? */
                if (!pPixelProcessed[i*width + j])
                {
                    /* RGB color needs to be set correctly to avoid "color bleeding" */
                    nDivider=0;
                    RTotal = GTotal = BTotal=0;

                    /* Get right pixels RGB */
                    if (j<(width-1))
                    {
                        /* Middle right */
                        if (pPixelProcessed[i*width + j + 1])
                        {
                            scaler = GetScaler(((ARGB *)(pByte + 4))->a);
                            RTotal+=((ARGB *)(pByte + 4))->r*scaler;
                            GTotal+=((ARGB *)(pByte + 4))->g*scaler;
                            BTotal+=((ARGB *)(pByte + 4))->b*scaler;
                            nDivider += scaler;
                        }

                        if (i>0)
                        {
                            /* Top right */
                            if (pPixelProcessed[(i-1)*width + j + 1])
                            {
                                scaler = GetScaler(((ARGB *)(pByte - rowbytes + 4))->a);
                                RTotal+=((ARGB *)(pByte - rowbytes + 4))->r*scaler;
                                GTotal+=((ARGB *)(pByte - rowbytes + 4))->g*scaler;
                                BTotal+=((ARGB *)(pByte - rowbytes + 4))->b*scaler;
                                nDivider += scaler;
                            }
                        }

                        if (i<(height-1))
                        {
                            /* Bottom right */
                            if (pPixelProcessed[(i+1)*width + j + 1])
                            {
                                scaler = GetScaler(((ARGB *)(pByte + rowbytes + 4))->a);
                                RTotal+=((ARGB *)(pByte + rowbytes + 4))->r*scaler;
                                GTotal+=((ARGB *)(pByte + rowbytes + 4))->g*scaler;
                                BTotal+=((ARGB *)(pByte + rowbytes + 4))->b*scaler;
                                nDivider += scaler;                            }
                        }
                    }

                    /* Get left pixels RGB */
                    if (j>0)
                    {
                        /* Middle left */
                        if (pPixelProcessed[i*width + j - 1])
                        {
                            scaler = GetScaler(((ARGB *)(pByte - 4))->a);
                            RTotal+=((ARGB *)(pByte - 4))->r*scaler;
                            GTotal+=((ARGB *)(pByte - 4))->g*scaler;
                            BTotal+=((ARGB *)(pByte - 4))->b*scaler;
                            nDivider += scaler;
                        }

                        if (i>0)
                        {
                            /* Top left */
                            if (pPixelProcessed[(i-1)*width + j - 1])
                            {
                                scaler = GetScaler(((ARGB *)(pByte - rowbytes - 4))->a);
                                RTotal+=((ARGB *)(pByte - rowbytes - 4))->r*scaler;
                                GTotal+=((ARGB *)(pByte - rowbytes - 4))->g*scaler;
                                BTotal+=((ARGB *)(pByte - rowbytes - 4))->b*scaler;
                                nDivider += scaler;
                            }
                        }

                        if (i<(height-1))
                        {
                            /* Bottom left */
                            if (pPixelProcessed[(i+1)*width + j - 1])
                            {
                                scaler = GetScaler(((ARGB *)(pByte + rowbytes - 4))->a);
                                RTotal+=((ARGB *)(pByte + rowbytes - 4))->r*scaler;
                                GTotal+=((ARGB *)(pByte + rowbytes - 4))->g*scaler;
                                BTotal+=((ARGB *)(pByte + rowbytes - 4))->b*scaler;
                                nDivider += scaler;
                            }
                        }
                    }

                    /* Get bottom pixel RGB */
                    if (i<(height-1))
                    {
                        if (pPixelProcessed[(i+1)*width + j])
                        {
                            scaler = GetScaler(((ARGB *)(pByte + rowbytes))->a);
                            RTotal+=((ARGB *)(pByte + rowbytes))->r*scaler;
                            GTotal+=((ARGB *)(pByte + rowbytes))->g*scaler;
                            BTotal+=((ARGB *)(pByte + rowbytes))->b*scaler;
                            nDivider += scaler;
                        }
                    }

                    /* Get top pixel RGB */
                    if (i>0)
                    {
                        if (pPixelProcessed[(i-1)*width + j])
                        {
                            scaler = GetScaler(((ARGB *)(pByte - rowbytes))->a);
                            RTotal+=((ARGB *)(pByte - rowbytes))->r*scaler;
                            GTotal+=((ARGB *)(pByte - rowbytes))->g*scaler;
                            BTotal+=((ARGB *)(pByte - rowbytes))->b*scaler;
                            nDivider += scaler;
                        }
                    }

                    /* We now have the neighbour pixels, so average the red, green
                       and blue value for transparent pixel */
                    if (nDivider)       /* To avoid division by zero */
                    {
                        float fOneOverDivider=1.0f/nDivider;

                        /* Average color channels of neighbouring texels */
                        R=(unsigned char)(RTotal*fOneOverDivider);
                        G=(unsigned char)(GTotal*fOneOverDivider);
                        B=(unsigned char)(BTotal*fOneOverDivider);

                        /* Write resulting color */
                        ((ARGB *)pByte)->r = R;
                        ((ARGB *)pByte)->g = G;
                        ((ARGB *)pByte)->b = B;

                        /* Pixel has been processed */
                        pPixelProcessed[i*width + j] = 1;
                    }

                    /* This pixel was completely transparent */
                    bAtLeastOne=1;
                }

                pByte += 4;
            }
        }
    }

    /* Release memory */
    if (pPixelProcessed)
    {
        gfree(pPixelProcessed);
        pPixelProcessed=NULL;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *abouttedge(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('T','E','D','G');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "tedge");
        strcpy(about->longnamestr,  "Transparent Edge Extrude");
        about->numoptions =         1;
        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "method");
        strcpy(about->option[0].enumlist,  "");
    }
    return(about);
}

FPUBLIC int opentedge(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infotedge(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    unsigned int method=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) method = fx->option[0].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return(ginfo);
}

FPUBLIC int filtertedge(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *)fx->source[0].image;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    unsigned int method=0;
    char *d = dest;
    if (fx->option)
    {
        if (fx->numoptions>=1) method = fx->option[0].i;
    }

    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(d,source,bpl);
        source += fx->source[0].rowbytes;
        d      += rowbytes;
    }
    tedge((ARGB *)dest,ginfo->width,ginfo->height,rowbytes);
    return(1);
}

FPUBLIC int closetedge(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

