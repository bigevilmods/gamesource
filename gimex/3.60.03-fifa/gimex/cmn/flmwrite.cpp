// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "flmgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL FLM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = galloc(sizeof(GINFO));
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL FLM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpp;
    int bpl;
    int x;
    int y;
    int ok=0;
    ARGB *s;

    if (ginfo->bpp>8)
    {
        bpp = 32;

        if (ginfo->alphabits<=1 && ginfo->redbits<=5 && ginfo->greenbits<=5 && ginfo->bluebits<=5)
            bpp = 16;
        else
            bpp=32;
        bpl = ginfo->width*bpp/8;
        linebuf = (char *)galloc(bpl);
        if (linebuf)
        {
            ok = 1;
            for (y=0; y<ginfo->height && ok; ++y)
            {
                lineptr = linebuf;
                s = (ARGB *) (source+rowbytes*y);

                if (bpp==16)
                {
                    int a,r,g,b;
                    int aRgb5;

                    for (x=0; x<ginfo->width; ++x)
                    {
                        a = ((int)s->a*1+128)/255;
                        r = ((int)s->r*31+128)/255;
                        g = ((int)s->g*31+128)/255;
                        b = ((int)s->b*31+128)/255;
                        aRgb5 = (a<<15)|(b<<10)|(g<<5)|(r);
                        gputi(lineptr,aRgb5,2);
                        ++s;
                        lineptr += 2;
                    }
                }
                else
                {
                    for (x=0; x<ginfo->width; ++x)
                    {
                        lineptr[0] = s->r;
                        lineptr[1] = s->g;
                        lineptr[2] = s->b;
                        lineptr[3] = s->a;
                        ++s;
                        lineptr += 4;
                    }
                }

                ok = gwrite(gx->gstream,linebuf,bpl);
            }

            gfree(linebuf);
        }
        if (!gx->framenum)
            memcpy(gx->gref,ginfo,sizeof(GINFO));
        ++gx->framenum;
    }

    return(ok);
}

int GCALL FLM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        int bpp;
        int framespersec;
        FLMHeaderStruct flmheader;
        GINFO *ginfo = (GINFO *)gx->gref;

        bpp = ginfo->originalbpp;
        if (bpp<16 || (bpp==16 && ginfo->greenbits<=5)) bpp=16;
        else bpp=32;

        framespersec = (int)ginfo->fps;
        if (framespersec<0 || framespersec>1000)
            framespersec = 15;

        memset(&flmheader,0,sizeof(FLMHeaderStruct));

        FLMPUTFIELD(flmheader.signature,GMAKEID('R','a','n','d')); /* 'Rand' */
        FLMPUTFIELD(flmheader.numframes,gx->framenum);      /* number of frames in file */
        FLMPUTFIELD(flmheader.packing,0);                   /* packing method (0) */
        FLMPUTFIELD(flmheader.eaformat,bpp==16?0xea02:0);   /* reserved, should be 0. ea extension sets to 0xea02 for 1555 */
        FLMPUTFIELD(flmheader.width,ginfo->width);          /* image width in pixels */
        FLMPUTFIELD(flmheader.height,ginfo->height);        /* image height in pixels */
        FLMPUTFIELD(flmheader.leading,0);                   /* horiz gap between frames */
        FLMPUTFIELD(flmheader.framespersec,framespersec);   /* frame rate */

        ok = gwrite(gx->gstream,&flmheader,sizeof(FLMHeaderStruct));

        if (gx->gref) gfree(gx->gref);
        ok = gfree((void *) gx);
    }

    return(ok);
}

