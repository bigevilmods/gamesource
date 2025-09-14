// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                        Nil - Nil Gimex                           */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940531 FB place holder Gimex that does nothing           */
/*    1.01 940614 FB updated to Gimex 1.5                           */
/*    1.02 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: nilread.cpp nilwrite.cpp nilgimex.h                           */
/* These functions can be used as substitutes when real gimex       */
/* functions are missing.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "nilgimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL NIL_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','N','I','L');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 0;    /* supports importing */
        info->canexport          = 0;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 0;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID( 0 , 0 , 0 , 0 );   /* mac file system types used */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.02");                /* version # */
        strcpy(info->shorttypestr,  "Nil");                 /* type */
        strcpy(info->wordtypestr,   "Nil");                 /* wordtype */
        strcpy(info->longtypestr,   "Nil Gimex");           /* longtype */
    }
    return(info);
}

int GCALL NIL_is(GSTREAM *g)
{
    return(0);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL NIL_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    return(0);
}


GINFO * GCALL NIL_info(GINSTANCE *gx, int framenum)
{
    return((GINFO *)0);
}

int GCALL NIL_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    return(0);
}

int GCALL NIL_close(GINSTANCE *gx)
{
    return(0);
}


