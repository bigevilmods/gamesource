// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "hsigimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL HSI_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL HSI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpp;
    int bpl;
    int colours;
    int x;
    int y;
    int ok=0;
    int i;
    ARGB *s;

    colours = (ginfo->bpp==8)?ginfo->numcolours:0;
    bpp = (ginfo->bpp==8)?8:24;
    bpl = ginfo->width*bpp/8;

/* 00000: 6d687761 6e680004 020b01de 0000ffff  mhwanh.....^....     */
/* 00010: ffff0064 00000000 00000000 00000000  ...d............     */

    if (gx->framenum==0)
    {
        unsigned char hsiheader[32];
        unsigned char palbuf[256*3];

        memset(hsiheader, 0, 32);
        gputm(hsiheader+0,0x6d687761,4);        /* id */
        gputm(hsiheader+4,0x6e680004,4);        /* id/version */
        gputm(hsiheader+8,ginfo->width,2);      /* width */
        gputm(hsiheader+10,ginfo->height,2);    /* height */
        gputm(hsiheader+12,colours,2);          /* colours or 0 for rgb */
        gputm(hsiheader+14,0,2);                /* x aspect unknown */
        gputm(hsiheader+16,0,2);                /* y aspect unknown */
        gputm(hsiheader+18,0,2);                /* gamma unknown */

        ok = gwrite(gx->gstream,hsiheader,32);

        if (colours)
        {
            for (i=0; i<colours; ++i)
            {
                palbuf[i*3+0] = ginfo->colourtbl[i].r;
                palbuf[i*3+1] = ginfo->colourtbl[i].g;
                palbuf[i*3+2] = ginfo->colourtbl[i].b;
            }
            ok = gwrite(gx->gstream, (char *) palbuf, colours*3);
        }
    }

    linebuf = (char *) galloc(bpl);
    if (linebuf)
    {
        ok = 1;
        for (y=0; y<ginfo->height && ok; ++y)
        {
            lineptr = linebuf;
            s = (ARGB *) (source+rowbytes*y);

            if (bpp==8)
            {
                ok = gwrite(gx->gstream,(char *)s,bpl);
            }
            else
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    lineptr[0] = s->r;
                    lineptr[1] = s->g;
                    lineptr[2] = s->b;
                    ++s;
                    lineptr += 3;
                }
                ok = gwrite(gx->gstream,linebuf,bpl);
            }
        }

        gfree(linebuf);
    }
    ++gx->framenum;

    return(ok);
}

int GCALL HSI_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

