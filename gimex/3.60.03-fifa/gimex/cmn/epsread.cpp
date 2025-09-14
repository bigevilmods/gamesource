// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               EPS - Encapsulated Postscript Gimex                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940531 FB Experimental alchemy tool based Gimex          */
/*    1.01 940601 FB Call common batch                              */
/*    1.02 940628 FB updated to Gimex 1.5                           */
/*    1.03 940703 FB converted to stand alone                       */
/*    1.04 950529 FB ginstance signature                            */
/*    1.05 980106 KO iseps() returns 80 so isai() can override      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* grey scale output only                                           */
/* Reentrant                                                        */
/* Files: epsread.cpp epswrite.cpp epsgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* EPS files support 1,2,4,8,12 bits per channel with 1 channel     */
/* for grey scale and up to 4 for colour.                           */
/* The data can be ascii or binary, and a JPEG variant is supported */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "gimex.h"
#include "epsgimex.h"

#include <string.h>
#include <stdarg.h>

#define SIGNATURE     GMAKEID('.','E','P','S')
#define EXTENSION0    ".eps"
#define EXTENSION1    ".ps"
#define EXTENSION2    ""
#define EXTENSION3    ""
#define MACTYPE0      GMAKEID('E','P','S','F')
#define MACTYPE1      0
#define MACTYPE2      0
#define MACTYPE3      0
#define SHORTTYPE     "EPSF"
#define WORDTYPE      "EPSF"
#define LONGTYPE      "Encapsulated Postscript"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL EPS_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','E','P','S');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 0;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('E','P','S','F');   /* mac file system types used */
        info->mactype[1]      = GMAKEID('E','P','S','I');
        strcpy(info->extensions[0], ".eps");                /* extensions */
        strcpy(info->extensions[1], ".epsf");
        strcpy(info->extensions[2], ".ps");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.05");                /* version # */
        strcpy(info->shorttypestr,  "EPS");                 /* type */
        strcpy(info->wordtypestr,   "EPSF");                /* wordtype */
        strcpy(info->longtypestr,   "Encapsulated Postscript"); /* longtype */
    }
    return(info);
}

int GCALL EPS_is(GSTREAM *g)
{
    char epsbuf[4];
    int ok=0;

    gseek(g, 0);
    if (gread(g, epsbuf, 4))
    {
        if (epsbuf[0]=='%'
         && epsbuf[1]=='!'
         && epsbuf[2]=='P'
         && epsbuf[3]=='S')
            ok = 80;    /* it's "Probably" a EPS. */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL EPS_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    return(0);
}


GINFO * GCALL EPS_info(GINSTANCE *gx, int framenum)
{
    return((GINFO *)0);
}

int GCALL EPS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    return(0);
}

int GCALL EPS_close(GINSTANCE *gx)
{
    return(0);
}

