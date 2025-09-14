// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "gimex.h"
#include "sctgimex.h"

/****************************************************************/
/*  Internal Function                                           */
/****************************************************************/

static void FormatAspectFloat(char *buf,float f)
{
    double e10;
    int il10;

    il10 = (int)log10((double)f)+1;
    e10 = pow(10.0,(double)il10);
    sprintf(buf,"%+.8fE%+03d",f/(e10),il10);
    strcpy(buf+1,buf+2);
}


/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL SCT_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL SCT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    unsigned char *linebuf;
    int awidth=(ginfo->width+3)&~3;
    int bpl;
    int colours;
    int x;
    int y;
    int ok=0;
    ARGB *s;

    if (ginfo->bpp>8)
    {
        colours = ginfo->numcolours;
        bpl = awidth*3;

/* 00000: 433a5c74 6f6f6c73 5c47696d 65785c62  C:\tools\Gimex\b */
/* 00010: 6f622e73 63742020 20202020 20202020  ob.sct */
/* 00020: 20202020 20202020 20202020 20202020 */
/* 00030: 20202020 20202020 20202020 20202020 */
/* 00040: 20202020 20202020 20202020 20202020 */
/* 00050: 43540000 00000000 00000000 00000000  CT.............. */
/* 00060: 00000000 00000000 00000000 00000000  ................ */
/* 00070: 00000000 00000000 00000000 00000000  ................ */
/* 00080: 00000000 00000000 00000000 00000000  ................ */
/* 00090: 00000000 00000000 00000000 00000000  ................ */
/* 003e0: 00000000 00000000 00000000 00000000  ................ */
/* 003f0: 00000000 00000000 00000000 00000000  ................ */
/* 00400: 01030007 2b2e3230 37333631 3333452b  ....+.20736133E+ */
/* 00410: 30312b2e 32303733 36313333 452b3031  01+.20736133E+01 */
/* 00420: 2b303030 30303030 30323536 2b303030  +00000000256+000 */
/* 00430: 30303030 30323536 00000000 00000000  00000256........ */
/* 00440: 00000000 00000000 00000000 00000000  ................ */
/* 00470: 00000000 00000000 00000000 00000000  ................ */
/* 007f0: 00000000 00000000 00000000 00000000  ................ */
/* 00800: 5a4a4a4a 6a41394a 6a6a6a5a 62623108  ZJJJjA9JjjjZbb1. */
/* 00810: 3110104a 20103118 18313939 4a4a7b6a  1..J .1..199JJ.j */
/* 00820: 62313120 10202018 20201018 4a393931  b11 .  .  ..J991 */
/* 00830: 62625a5a 62523939 4a4a4a6a 5a6a6a5a  bbZZbR99JJJjZjjZ */
/* 00840: 5a5a5a5a 5a5a626a 626a5a8b 7b7b7b6a  ZZZZZZbjbjZ....j */
/* 00850: 836a5a62 625a5a5a 5a5a5a5a 5a5a5a5a  .jZbbZZZZZZZZZZZ */
/* 00860: 6262626a                             bbbj */

        {
            unsigned char sctheader[2048];
            char aspectbuf[80];
            float faspect;

            if (ginfo->dpi)
                faspect = ginfo->width/ginfo->dpi;
            else
                faspect = ginfo->width/72.0f;

            memset(sctheader, 0, 2048);
            strncpy((char *)sctheader+0,ginfo->framename,0x50);
            gputm(sctheader+0x50,0x43540000,4);       // 'CT'
            gputm(sctheader+0x400,0x01030007,4);    // channel, planes, mask

//....+.35555555E+01+.35555555E+01

            FormatAspectFloat(aspectbuf,faspect);

            sprintf((char *)sctheader+0x404,"%s%s%+012g%+012g",aspectbuf,aspectbuf,(float)ginfo->height,(float)ginfo->width);

            ok = gwrite(gx->gstream,sctheader,2048);
        }

        linebuf = (unsigned char *) galloc(bpl);
        if (linebuf)
        {
            ok = 1;
            for (y=0; y<ginfo->height && ok; ++y)
            {
                s = (ARGB *) (source+rowbytes*y);

#if 0
                if (bpp==8)
                {
                    ok = gwrite(gx->gstream,(char *)s,bpl);
                    }
                else
#endif
                {
                    for (x=0; x<ginfo->width; ++x)
                    {
                        linebuf[awidth*0+x] = s->r;
                        linebuf[awidth*1+x] = s->g;
                        linebuf[awidth*2+x] = s->b;
                        ++s;
                    }
                    ok = gwrite(gx->gstream,linebuf,bpl);
                }
            }

            gfree(linebuf);
        }
        ++gx->framenum;
    }

    return(ok);
}

int GCALL SCT_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

