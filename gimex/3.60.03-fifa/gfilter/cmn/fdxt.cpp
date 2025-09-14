// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*            FDXT - Dxt compression quality analyser               */
/*                                                                  */
/*                         by Ian Davids, EAC                       */
/*                                                                  */
/*                    Filter Module - Mar 29, 2001                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010329 ID initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* fdxt(dxtmode)  - dxtmode is 1..5 or 0 which means automatic      */
/*                  selection of type.                              */
/*    the dxtcompressor is still the S3 version in the codex and    */
/*    is very slow at compressing but relatively fast at            */
/*    decompression.                                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fdxt.h"
#include "dxtread.cpp"
#include "dxtwrite.cpp"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

//---------------------------------------------------------------------------------------------------
// prepare to pass the image back to calling program in a continuous stream.
// function unorders blocks from the 4X4 block format it was currently in.
// function aquired from ddsread.c
static void FDXTunorderblocks(int width, int height, int rowbytes, const void *source, void *dest)
{
    int x,y,bx,by;
    const unsigned char *s = (const unsigned char *)source;

    for (y=0; y<height; y+=4)
    {
        for (x=0; x<width; x+=4)
        {
            for (by=0; by<4; ++by) // rows of block
            {
                for (bx=0; bx<4; ++bx) // columns of block
                {
                    if ((y+by)<height && (x+bx)<width)
                        *(int *) ((char *)dest + (y+by)*rowbytes + (x+bx)*4) = *(int *) (s + by*16 + bx*4);
                }
            }
            s += 64;
        }
    }
}


//---------------------------------------------------------------------------------------------------
// Take the data stream and chunk it into blocks of 4X4 for the encoder.
// function aquired from ddswrite.c
static void FDXTreorderblocks(int width, int height, int rowbytes, unsigned char *source, unsigned char *dest)
{
    int i;
    int j;
    int k;
    unsigned char *s;
    unsigned char *d = dest;
    int copywidth;
    int clearwidth;
    int copyheight;
    int clearheight;

    for (i=0; i<height; i+=4)
    {

        /* If height is not a multiple of 4 and we're on the last row of blocks */
        if (i+4 > height)
        {
            copyheight = height-i;
            clearheight = 4-copyheight;
        }
        else
        {
            copyheight = 4;
            clearheight = 0;
        }

        s = source + i*rowbytes;
        for (j=0; j<width; j+=4)
        {
            /* If width is not a multiple of 4 and we are on the last block of the line */
            if (j+4 > width)
            {
                copywidth = width-j;
                clearwidth = 4-copywidth;
            }
            else
            {
                copywidth = 4;
                clearwidth = 0;
            }

            /* Copy only what exists in the source */
            for (k=0; k<copyheight; k++)
            {

                /* Copy only what exists in the source */
                memcpy(d, s + k*rowbytes, copywidth * 4);

                /* Clear the rest */
                if (clearwidth)
                {
                    memset(d + copywidth * 4, 0, clearwidth * 4);
                }
                d += 4 * 4;
            }

            /* Clear the rest */
            for (k=0; k<clearheight; k++)
            {
                memset(d, 0, 4 * 4);
                d += 4 * 4;
            }

            s += 4 * 4;
        }
    }
}





/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

//---------------------------------------------------------------------------------------------------
FPUBLIC FABOUT *aboutdxt(void)
{
    FABOUT *about = (FABOUT*)galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','D','X','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "I Davids/F Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "FDxt");
        strcpy(about->longnamestr,  "FDxt");
        about->numoptions =         1;

        about->numoptions =                1;
        about->option[0].type =            FOP_INT | FOP_ENUM;
        about->option[0].min.i =           0;
        about->option[0].max.i =           5;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "DxtType");
        strcpy(about->option[0].enumlist,  "Default,DXT1,DXT2,DXT3,DXT4,DXT5");
    }
    return(about);
}

//---------------------------------------------------------------------------------------------------
FPUBLIC int opendxt(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

//---------------------------------------------------------------------------------------------------
FPUBLIC GINFO *infodxt(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    // nothing really changes about the image parameters so leave ginfo intact.
    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }

    return ginfo;
}

//---------------------------------------------------------------------------------------------------
FPUBLIC int filterdxt(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok              = 0;
    char *source        = (char *) fx->source[0].image;
    char *temp          = NULL;
    char *newsource     = NULL;
    int compressedsize  = 0;
    int size = ginfo->width*ginfo->height * 4;
    int dxtType = 0;

    // copy over my compression mode option... if zero then do automatic selection.
    if (fx->option)
        if (fx->numoptions>=1) dxtType = fx->option[0].i;

    temp = (char *) galloc(size);
    if (temp)
    {
        int options[1];
        newsource = (char *) galloc(size);
        if(newsource)
        {
            // block up the image data into 4x4 blocks.
            FDXTreorderblocks(ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, (unsigned char *)newsource);
            // choose compression mode (info stored in the dxtheader at the front of the compressed image.)
            if ((!dxtType && ginfo->alphabits > 1 && ginfo->alphabits != 3) ||
                 dxtType == 3 || dxtType == 2)                                  // DXT3 and DXT2
                options[0] = S3TC_ENCODE_RGB_FULL |S3TC_ENCODE_ALPHA_EXPLICIT;

            else if((!dxtType && ginfo->alphabits == 3) ||
                     dxtType == 4 || dxtType == 5)                              // DXT5 for now.
                options[0] = S3TC_ENCODE_RGB_FULL |S3TC_ENCODE_ALPHA_INTERPOLATED;

            else if (!dxtType && ginfo->alphabits == 1 ||
                      dxtType == 1)                                             // DXT1
                options[0] = S3TC_ENCODE_RGB_ALPHA_COMPARE;

            else                                                                // Also DXT1
                options[0] = S3TC_ENCODE_RGB_FULL;

            // Encode image data.
            size = DXT_encode(newsource, size, temp, options);
            gfree(newsource);
            // Now setup to decode again.
            newsource = (char *) galloc(DXT_size(temp));
            if (temp)
            {
                // store block format image data in newsource.
                memset(newsource, 0, DXT_size(temp));
                DXT_decode(temp, &compressedsize, (void*)newsource);
                // unblock image data into the destination buffer.
                FDXTunorderblocks(ginfo->width, ginfo->height, rowbytes, (void*)(newsource), dest);
                gfree(temp);
                ok = 1;
            }
            gfree(newsource);
        }
    }
    return(ok);
}

//---------------------------------------------------------------------------------------------------
FPUBLIC int closedxt(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}


