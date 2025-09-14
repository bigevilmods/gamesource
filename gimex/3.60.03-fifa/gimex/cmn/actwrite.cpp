// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "actgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL ACT_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL ACT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=0;
    int i;
    unsigned char palbuf[768];

    if (ginfo->bpp<=8)
    {
        for (i=0; i<256; ++i)
        {
            palbuf[(i)*3+0] = ginfo->colourtbl[i].r;
            palbuf[(i)*3+1] = ginfo->colourtbl[i].g;
            palbuf[(i)*3+2] = ginfo->colourtbl[i].b;
        }

        ok = gwrite(gx->gstream, (char *) palbuf, 768);
    }

    return(ok);
}

int GCALL ACT_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

