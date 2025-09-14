// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "rawgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL RAW_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL RAW_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    unsigned char rawheader[4];
    char *linebuf;
    char *lineptr;
    int bpl;
    int x;
    int y;
    int ok=0;
    ARGB *s;

    if (ginfo->bpp>8)
    {
        gputi(rawheader+0,ginfo->width,2);   /* Width of bitmap in pixels */
        gputi(rawheader+2,ginfo->height,2);  /* Height of bitmap in pixels */
        ok = gwrite(gx->gstream,rawheader,4);

        bpl = ginfo->width*3;
        if (ginfo->alphabits)
            bpl = ginfo->width*4;
        linebuf = (char *) galloc(bpl);
        if (linebuf)
        {
            for (y=0; y<ginfo->height; ++y)
            {
                lineptr = linebuf;
                s = (ARGB *) (source+rowbytes*y);

                for (x=0; x<ginfo->width; ++x)
                {
                    lineptr[0] = s->r;
                    lineptr[1] = s->g;
                    lineptr[2] = s->b;
                    if (ginfo->alphabits)
                    {
                        lineptr[3] = s->a;
                        lineptr += 4;
                    }
                    else
                        lineptr += 3;

                    ++s;
                }
                ok = gwrite(gx->gstream,linebuf,bpl);
            }
            gfree(linebuf);
        }
    }

    return(ok);
}

int GCALL RAW_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


