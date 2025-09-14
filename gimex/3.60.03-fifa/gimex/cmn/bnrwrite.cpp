// (c) 1994-2007 Electronic Arts Inc.

#ifndef __BNRWRITE
#define __BNRWRITE 1

#include <string.h>
#include "gimex.h"
#include "bnrgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

static const int width=96;
static const int height=32;

// pixel lookup function for 16bpp GameCube Shapes.

static int BNR_SwizzleGetPixelOffset16( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}

// bit depth conversion function for GSH.
static unsigned short BNRWpixel8To5(const ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int argb;

    if (aRgb8->a<0xe0) // 3444 - only 3 bits of alpha significant
    {
        a = (int)aRgb8->a>>(8-3);
        r = (int)aRgb8->r>>(8-4);
        g = (int)aRgb8->g>>(8-4);
        b = (int)aRgb8->b>>(8-4);
        argb = (0<<15)|(a<<12)|(r<<8)|(g<<4)|b;
    }
    else    // 555
    {
        r = (int)aRgb8->r>>(8-5);
        g = (int)aRgb8->g>>(8-5);
        b = (int)aRgb8->b>>(8-5);

        argb = (1<<15)|(r<<10)|(g<<5)|b;
    }
    return((unsigned short) argb);
}


int GCALL BNR_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL BNR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int x;
    int y;
    int pixoffset;
    const char *s;
    int ok;
    ARGB def  = {0,0,0,0};


    BNRHeaderStruct bnrheader;

    memset(&bnrheader,0,sizeof(bnrheader));

    s = "EA Sports";
    if (ginfo->framename[0])
        s = &ginfo->framename[0];
    gputm(bnrheader.id,GMAKEID('B','N','R','1'),4);
    strncpy(bnrheader.shortTitle, s,                 sizeof(bnrheader.shortTitle)); /* Short game title shown in IPL menu          */
    strncpy(bnrheader.shortMaker, "Electronic Arts", sizeof(bnrheader.shortMaker)); /* developer names shown in IPL menu           */
    strncpy(bnrheader.longTitle,  s,                 sizeof(bnrheader.longTitle));  /* game title shown in IPL game start screen   */
    strncpy(bnrheader.longMaker,  "Electronic Arts", sizeof(bnrheader.longMaker));  /* developer name shown in IPL start screen    */
    strncpy(bnrheader.comment,    ginfo->comment,    sizeof(bnrheader.comment));    /* Game description shown in IPL game start    */

    for (y=0; y<height; ++y)
    {
        s = source;
        for (x=0; x<width; ++x)
        {
            pixoffset = BNR_SwizzleGetPixelOffset16(x,y,width);
            if(y >= ginfo->height || x >= ginfo->width)
                gputm((void *) &bnrheader.image[pixoffset],BNRWpixel8To5(&def),2);
            else
            {
                gputm((void *) &bnrheader.image[pixoffset],BNRWpixel8To5((const ARGB *) s),2);
                s += 4;
            }
        }
        source += rowbytes;
    }

    ok = gwrite(gx->gstream, (char *) &bnrheader, 6496)==6496;   /* write file */
    return(ok);
}

int GCALL BNR_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

#endif


