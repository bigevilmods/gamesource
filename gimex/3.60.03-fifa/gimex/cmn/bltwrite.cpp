// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "bltgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL BLT_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = pathname;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}


int GCALL BLT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    unsigned char bltheader[4];
    int i;
    int x;
    int y;
    int ok;
    char fname[512];
    char pal[768];
    GSTREAM *f;

    gputi(bltheader+0,ginfo->width,2);   /* Width of bitmap in pixels */
    gputi(bltheader+2,ginfo->height,2);  /* Height of bitmap in pixels */
    ok = gwrite(gx->gstream,bltheader,4);

    for (y=0; y<ginfo->height; ++y)
    {
        ok = gwrite(gx->gstream,source+rowbytes*y,ginfo->width);
    }

    if (ok)
    {
        strcpy(fname,gx->gref);
        if (strrchr(fname,'.'))
            *strrchr(fname,'.') = 0;
        strcat(fname,".pal");

        for (i=0; i<256; ++i)
        {
            pal[i*3+0] = ginfo->colourtbl[i].r;
            pal[i*3+1] = ginfo->colourtbl[i].g;
            pal[i*3+2] = ginfo->colourtbl[i].b;
        }

        f = gwopen(fname);
        if (f)
        {
            ok = gwrite(f,pal,768);
            gclose(f);
        }
    }
    return(ok);
}

int GCALL BLT_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


