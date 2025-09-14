// (c) 1994-2007 Electronic Arts Inc.

#if defined(_MSC_VER)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gimex.h"
#include "exegimex.h"

#include "bmpgimex.h"

#define EXETYPES 6

static struct
{
    const char *extensionstr;
    const char *optionstr;
} EXEexporttypes[EXETYPES]=
{
    "ras","-s",
    "rle","--u",
//    "ico","--O",
    "eps","-e",
    "mac","--t",
    "xbm","--b",
    "pdf","--d",
};

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL EXE_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    int ok=0;

#if defined(_MSC_VER)
    GINSTANCE *gx;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = (void *) pathname;
        *gxptr = gx;
        ok = 1;
    }
#endif
    return(ok);
}

int GCALL EXE_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=0;
#if defined(_MSC_VER)
    char sysline[512];
    char *pathname = (char *) gx->gref;
    GSTREAM *g;
    FILE *stream;
    int subtype=gmax(0,gmin(EXETYPES-1,ginfo->subtype));

    if (ginfo->bpp>8)
    {
        sprintf(sysline,"gxouttmp.bmp");

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
            stream = freopen(sysline, "w", stderr);

            sprintf(sysline,"alchemy gxouttmp.bmp %s -o gxouttmp.%s>%s.lst",EXEexporttypes[subtype].optionstr,EXEexporttypes[subtype].extensionstr,pathname);
            system(sysline);

            stream = freopen("con:", "w", stderr);

            sprintf(sysline,"gxouttmp.%s",EXEexporttypes[subtype].extensionstr);
            g = gopen(sysline);
            if (g)
            {
                int len;
                void *exe;

                len = (int) glen(g);
                if (len)
                {
                    exe = galloc(len);
                    if (exe)
                    {
                        ok = gread(g,exe,len);
                        if (ok)
                            ok = gwrite(gx->gstream,exe,len);
                        gfree(exe);
                    }
                }
                gclose(g);
            }
        }
        sprintf(sysline,"%s.err",pathname);
        remove(sysline);
        sprintf(sysline,"%s.lst",pathname);
        remove(sysline);
        sprintf(sysline,"gxouttmp.bmp");
        remove(sysline);
        sprintf(sysline,"gxouttmp.%s",EXEexporttypes[subtype].extensionstr);
        remove(sysline);
    }

#endif
    return(ok);
}

int GCALL EXE_wclose(GINSTANCE *gx)
{
    int ok = 0;

#if defined(_MSC_VER)
    if (gx)
    {
        ok = gfree((void*)gx);
    }
#endif
    return(ok);
}


