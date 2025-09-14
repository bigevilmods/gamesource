// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "pixgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int PIXgetpixel(const unsigned char *base, int x, int bpp)
{
    return(ggetm(base+x*bpp/8,bpp/8));
}

#define GREY8(c) ((c)->r*0.29900f + (c)->g*0.58700f +  (c)->b*0.11400f)

static int PIXwriteline(GSTREAM *g, const unsigned char *linebase, int width, int bpp, const ARGB *colourtbl)
{
    int  len;
    int  x=0;
    int  ok=1;
    unsigned char buf[4];

    while (x<width && ok)
    {
        /* determine run length */

        len = 1;
        while (((x+len)<width) && (len<255) && (PIXgetpixel(linebase,x,bpp)==PIXgetpixel(linebase,x+len,bpp)))
        {
            ++len;
        }

        /* encode */

        if (bpp==8)
        {
            buf[0] = (unsigned char) len;
            buf[1] = (unsigned char) GREY8(colourtbl+*(linebase+x));

            ok = gwrite(g,buf,2);
        }
        else
        {
            buf[0] = (unsigned char) len;
            buf[1] = ((ARGB *)linebase)[x].b;
            buf[2] = ((ARGB *)linebase)[x].g;
            buf[3] = ((ARGB *)linebase)[x].r;
            ok = gwrite(g,buf,4);
        }
        x += len;
    }
    return(ok);
}

#undef GREY8

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL PIX_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL PIX_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int width;
    int height;
    int y;
    int ok=0;
    const unsigned char *linebase;
    PIXHeaderStruct pixheader;

    width = ginfo->width;
    height = ginfo->height;

    /* PIX FILE HEADER structure 10 bytes */

    memset(&pixheader,0,sizeof(pixheader));
    PIXPUTFIELD(pixheader.width, width);
    PIXPUTFIELD(pixheader.height,height);
    PIXPUTFIELD(pixheader.xoffset,0);
    PIXPUTFIELD(pixheader.yoffset,height-1);
    PIXPUTFIELD(pixheader.bpp,ginfo->bpp==8?8:24);

    ok = gwrite(gx->gstream, (char *) &pixheader, 10);

    if (ok)
    {
        for (y=0; y<height && ok; ++y)
        {
//            linebase = source + rowbytes*(height-1-y);
            linebase = (const unsigned char *)source + rowbytes*y;

            ok = PIXwriteline(gx->gstream, linebase, width, ginfo->bpp, &ginfo->colourtbl[0]);
        }
    }
    return(ok);
}

int GCALL PIX_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


