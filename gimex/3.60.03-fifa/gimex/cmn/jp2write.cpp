// (c) 1994-2007 Electronic Arts Inc.


#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gimex.h"
#include "jp2gimex.h"
#include "bmpgimex.h"

extern const char *JP2enc;

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL JP2_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = (void *)pathname;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL JP2_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=0;
    char sysline[512];
    const char *pathname = (const char *) gx->gref;
    GSTREAM *g;
    FILE *stream;

    if (ginfo->bpp>8)
    {
        sprintf(sysline,"%s.bmp",pathname);

        g = gwopen(sysline);
        if (g)
        {
            GINSTANCE *bmpgx;
            ok = BMP_wopen(&bmpgx,g,sysline,0);
            if (ok)
            {
                GINFO bmpginfo;
                bmpginfo = *ginfo;
                bmpginfo.packed = 0;
                bmpginfo.quality = 0;
                bmpginfo.subtype = 0;
                bmpginfo.originalbpp = 24;
                bmpginfo.alphabits = 0;
                bmpginfo.redbits = 8;
                bmpginfo.greenbits = 8;
                bmpginfo.bluebits = 8;
                ok = BMP_write(bmpgx,&bmpginfo,source,rowbytes);
                BMP_wclose(bmpgx);
            }
            gclose(g);
        }

        if (ok)
        {
            sprintf(sysline,"%s.err",pathname);
            if (_fileno(stderr))
                stream = freopen(sysline, "w", stderr);

            sprintf(sysline,"%s --input %s.bmp --output %s.jp2 --ratio %d --input-format bmp >%s.lst",JP2enc,pathname,pathname,101-ginfo->quality,pathname);
            system(sysline);

            if (_fileno(stderr))
                stream = freopen("con:", "w", stderr);

            sprintf(sysline,"%s.jp2",pathname);
            g = gopen(sysline);
            if (g)
            {
                int len;
                void *jp2;

                len = (int)glen(g);
                if (len)
                {
                    jp2 = galloc(len);
                    if (jp2)
                    {
                        ok = gread(g,jp2,len);
                        if (ok)
                            ok = gwrite(gx->gstream,jp2,len);
                        gfree(jp2);
                    }
                }
                gclose(g);
            }
        }
        sprintf(sysline,"%s.err",pathname);
        remove(sysline);
        sprintf(sysline,"%s.lst",pathname);
        remove(sysline);
        sprintf(sysline,"%s.bmp",pathname);
        remove(sysline);
        sprintf(sysline,"%s.jp2",pathname);
        remove(sysline);
    }

    return(ok);
}

int GCALL JP2_wclose(GINSTANCE *gx)
{
    int ok = 0;

    if (gx)
    {
        ok = gfree((void*)gx);
    }

    return(ok);
}


