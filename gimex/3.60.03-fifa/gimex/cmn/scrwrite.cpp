// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "scrgimex.h"

#define RGBL(y) (31-(((y>>3)&15)+((y&128)>>7)))
#define RGBR(x) ((x&7)+((x&128)>>4))

void encodergb(char *d, ARGB *s)
{
    int r,g,b;
    unsigned int pixl,pixr;

    r = s->r;
    g = s->g;
    b = s->b;

    pixl = (RGBL(r)<<10) | (RGBL(g)<<5) | (RGBL(b)<<0);
    pixr = (RGBR(r)<<10) | (RGBR(g)<<5) | (RGBR(b)<<0);

    gputm(d, ((pixl << 16) | pixr), 4);
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL SCR_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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


int GCALL SCR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpl;
    int x;
    int y;
    int ok;
    int cx;
    int cy;
    ARGB *s;
    ARGB black;

    black.a = 0;
    black.r = 0;
    black.g = 0;
    black.b = 0;
    bpl = 320*4;
    linebuf = (char *) galloc(bpl);
    if (linebuf)
    {
        cx = ginfo->width/2-320/2;
        cy = ginfo->height/2-240/2;
        for (y=cy; y<cy+240; ++y)
        {
            lineptr = linebuf;
            if (y<0 || y>=ginfo->height)
            {
                for (x=0; x<320; ++x)
                {
                    encodergb(lineptr,&black);
                    lineptr += 4;
                }
            }
            else
            {
                s = (ARGB *) (source+y*rowbytes+cx*4);
                for (x=cx; x<cx+320; ++x)
                {
                    if (x<0 || x>=ginfo->width)
                    {
                        encodergb(lineptr,&black);
                    }
                    else
                    {
                        encodergb(lineptr,s);
                    }
                    ++s;
                    lineptr += 4;
                }
            }
            ok = gwrite(gx->gstream,linebuf,bpl);
        }
        gfree(linebuf);
    }

    return(ok);
}

int GCALL SCR_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


