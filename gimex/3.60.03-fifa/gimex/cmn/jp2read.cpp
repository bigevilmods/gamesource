// (c) 1994-2007 Electronic Arts Inc.


/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       JP2 - JPeg2000 Gimex                       */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 000328 FB jpeg2000 using external tool based on clp      */
/*    1.01 000826 FB about disable if tools not found               */
/*    1.02 000908 FB disable about tristate                         */
/*    1.03 020729 FB quote for pathnames with spaces                */
/*    1.04 020730 FB jp2 ratio moved earlier to avoid redirect      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: jp2read.cpp jp2write.cpp jp2gimex.h                       */
/* writes intermediate files and spawns a tool                      */
/*                                                                  */
/* To avoid console opening in P2, _spawnl( _P_DETACH, ... )        */
/* could be used                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

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

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

const char *JP2enc="jp2enc";
static const char *JP2dec="jp2dec";

GABOUT * GCALL JP2_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','J','P','2');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 1;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 75;   /* default pack quality */
        info->mactype[0]      = GMAKEID( '.','J','P','2' );   /* mac file system types used */
        strcpy(info->extensions[0], ".jp2");                /* extensions */
        strcpy(info->authorstr,     "Frank Barchard");      /* author */
        strcpy(info->versionstr,    "1.04");                /* version # */
        strcpy(info->shorttypestr,  "JP2");                 /* type */
        strcpy(info->wordtypestr,   "JPeg2000");            /* wordtype */
        strcpy(info->longtypestr,   "JPeg2000");            /* longtype */

        {
            static int plugintested = 0;
            static char encbuf[512]={0,};
            static char decbuf[512]={0,};

            if (!plugintested)
            {
                char plugpath[512];
                plugintested = 1;
                if (SearchPath(NULL,"jp2enc.exe",NULL,512,plugpath,NULL))
                {
                    sprintf(encbuf,"\"%s\"",plugpath);
                    JP2enc = encbuf;
                }
                else
                {
                    plugintested = -1;
                }
                if (SearchPath(NULL,"jp2dec.exe",NULL,512,plugpath,NULL))
                {
                    sprintf(decbuf,"\"%s\"",plugpath);
                    JP2dec = decbuf;
                }
                else
                {
                    plugintested = -1;
                }
            }
            if (plugintested<0)
            {
                info->canimport       = 0;    /* supports importing */
                info->importpacked    = 0;    /* supports importing packed */
                info->import8         = 0;    /* supports importing 8 bit indexed */
                info->import32        = 0;    /* supports importing 32 bit direct rgb */
                info->canexport       = 0;    /* supports exporting */
                info->exportpacked    = 0;    /* supports exporting packed */
                info->export8         = 0;    /* supports exporting 8 bit indexed */
                info->export32        = 0;    /* supports exporting 32 bit direct rgb */
            }
        }
    }
    return(info);
}

int GCALL JP2_is(GSTREAM *g)
{
    char buf[4];
    int ok=0;
    gseek(g, 0);
    ok = gread(g,buf,4);
    if (ok)
    {
        ok = 0;
        if (ggetm(buf,4)==0xff4fff51)
            ok = 100;
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL JP2_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    int ok=0;
    char sysline[512];
    FILE *stream;

    sprintf(sysline,"%s.err",pathname);

    if (_fileno(stderr))
        stream = freopen(sysline, "w", stderr);

    sprintf(sysline,"%s --input %s --output %s.bmp --output-format bmp>%s.lst",JP2dec,pathname,pathname,pathname);
    system(sysline);

    if (_fileno(stderr))
        stream = freopen("con:", "w", stderr);

    sprintf(sysline,"%s.bmp",pathname);

    g = gopen(sysline);
    if (g)
    {
        ok = BMP_open(gxptr,g,sysline,0);
        if (ok)
            (*gxptr)->gref = (void *) pathname;
    }
    return(ok);
}

GINFO * GCALL JP2_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo;
    const char *pathname;
    GSTREAM *g;

    ginfo = BMP_info(gx,framenum);
    ginfo->packed = 1;

    pathname = (const char *) gx->gref;
    g = gopen(pathname);
    if (g)
    {
        ginfo->quality = 101-((ginfo->width*ginfo->height*3)/(int)glen(g));
        ginfo->signature = GMAKEID('.','J','P','2');
        gclose(g);
    }
    return(ginfo);
}

int GCALL JP2_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok;
    ginfo->packed = 0;
    ok = BMP_read(gx,ginfo,dest,rowbytes);
    ginfo->packed = 1;
    return(ok);
}

int GCALL JP2_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {

        if (gx->gstream)
        {
            gclose(gx->gstream);
            gx->gstream = NULL;
        }

        {
            const char *pathname;
            char sysline[512];

            pathname = (const char *) gx->gref;

            sprintf(sysline,"%s.err",pathname);
            remove(sysline);
            sprintf(sysline,"%s.lst",pathname);
            remove(sysline);
            sprintf(sysline,"%s.bmp",pathname);
            remove(sysline);
        }
        ok = gfree((void*)gx);
    }
    return(ok);
}


