// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                      CLP - ClipBoard Gimex                       */
/*                                                                  */
/*              by Frank Barchard and Kaz Okuda, EAC                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970814 KO Initial version                                */
/*    1.01 970822 KO Supports importing                             */
/*    1.02 970828 KO export sets ginfo->numcolours to 256 for BMP   */
/*    1.03 980325 KO openclp returns 0 if not CF_DIB in clipboard   */
/*    1.04 991105 FB CLP_write respect const GINFO                  */
/*    1.05 001012 FB export 4 bit (16 colours)                      */
/*    1.06 030108 FB import open/info detect DIB correctly          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: clpread.cpp clpwrite.cpp clpgimex.h                       */
/* Windows clipboard access gimex.                                  */
/* Converts to/from BMP so only BMP formats supported               */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string.h>
#include "gimex.h"
#include "clpgimex.h"


static int CLPdestroyrmembuf(void);
static int CLPmemread(GSTREAM *g, void *buf, int size);
static int CLPmemseek(GSTREAM *g, int offset);
static int CLPopen(void);
static int CLPread(unsigned int Format);
static int CLPclose(void);


/*
 * Include bmpread.c replacing the file input with a read from a file buffer
 * bmpread only uses gread and gseek, no gtell.
 */
#define BMPCLIPBOARD 1

/* Used by bmpread.c */
#define gread       CLPmemread
#define gtell(g) 0
#define gseek       CLPmemseek

/* Defined by bmpread.c */
#define BMP_is      CLPisbmp
#define BMP_about   CLPaboutbmp
#define BMP_open    CLPopenbmp
#define BMP_info    CLPinfobmp
#define BMP_read    CLPreadbmp
#define BMP_close   CLPclosebmp

#include "bmpgimex.h"
#include "bmpread.cpp"

#undef BMPCLIPBOARD
#undef gread
#undef gtell
#undef gseek
#undef BMP_is
#undef BMP_about
#undef BMP_open
#undef BMP_info
#undef BMP_read
#undef BMP_close

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static char *CLPrfilebuf=0;
static char *CLPrfileptr=0;
static HANDLE CLPrhandle=0;

static int CLPdestroyrmembuf(void)
{
    int ok;

    if (CLPrfilebuf)
    {
        GlobalUnlock(CLPrhandle);
        CLPrhandle = 0;
        CLPrfilebuf = 0;
        CLPrfileptr = 0;
        ok = 1;
    }
    return(ok);
}

static int CLPmemread(GSTREAM *g, void *buf, int size)
{
    int ok=0;

    if (CLPrfileptr)
    {
        if (memcpy(buf, CLPrfileptr, size))
        {
            CLPrfileptr+=size;
            ok = size;
        }
    }
    return(ok);
}

static int CLPmemseek(GSTREAM *g, int offset)
{
    CLPrfileptr = CLPrfilebuf + offset;
    return(offset);
}

static int CLPopen(void)
{
    int ok = 0;

    if (OpenClipboard(NULL))
    {
        ok = 1;
    }
    return(ok);
}

static int CLPread(unsigned int Format)
{
    HANDLE handle;
    int ok = 0;

    handle = GetClipboardData(Format);
    if (handle)
    {
        CLPrhandle = handle;
        CLPrfilebuf = (char *) GlobalLock(handle);
        CLPrfileptr = CLPrfilebuf;
        ok = 1;
    }

    return(ok);
}

static int CLPclose(void)
{
    int ok=0;

    ok = CloseClipboard();

    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL CLP_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('C','L','I','P');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 1;    /* module uses external resources */
        info->usesfile        = 0;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID( 'C','L','I','P' );   /* mac file system types used */
        strcpy(info->extensions[0], ".clp");                /* extensions */
        strcpy(info->authorstr,     "Frank Barchard");      /* author */
        strcpy(info->versionstr,    "1.06");                /* version # */
        strcpy(info->shorttypestr,  "CLP");                 /* type */
        strcpy(info->wordtypestr,   "Clip");                /* wordtype */
        strcpy(info->longtypestr,   "ClipBoard");           /* longtype */
    }
    return(info);
}

int GCALL CLP_is(GSTREAM *g)
{
    int ok=0;
    if (IsClipboardFormatAvailable(CF_DIB))
        ok = 40;
    return ok;
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL CLP_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    int ok=0;
    GINSTANCE *gx;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        *gxptr = gx;

        if (CLPopen())
        {
            if (IsClipboardFormatAvailable(CF_DIB))
//            if (GetClipboardData(CF_DIB))
            {
                gx->frames = ok = 1;
            }
            CLPclose();
        }
    }

    return(ok);
}

GINFO * GCALL CLP_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;

    if (framenum < gx->frames)
    {
        if (CLPopen())
        {
            if (CLPread(CF_DIB))
            {
                ginfo = CLPinfobmp(gx, 1);
                CLPdestroyrmembuf();
                ginfo->signature        = GMAKEID('C','L','I','P');   /* Type of file */
            }
            CLPclose();
        }
    }
    return(ginfo);
}

int GCALL CLP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;

    if (CLPopen())
    {
        if (CLPread(CF_DIB))
        {
            ok = CLPreadbmp(gx, ginfo, dest, rowbytes);
            CLPdestroyrmembuf();
        }
        CLPclose();
    }
    return(ok);
}

int GCALL CLP_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        ok = gfree((void*)gx);
    }
    return(ok);
}


