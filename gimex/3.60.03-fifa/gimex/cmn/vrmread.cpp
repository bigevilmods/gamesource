// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               VRM - PSX VRAM Allocation Viewer Gimex             */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940914 FB Based on TGA Gimex                             */
/*    1.01 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "vrmgimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL VRM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('V','R','A','M');
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
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('V','R','A','M');   /* mac file system types used */

        strcpy(info->extensions[0], ".vrm");                /* extensions */
        strcpy(info->extensions[1], ".vram");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.01");                /* version # */
        strcpy(info->shorttypestr,  "VRM");                 /* type */
        strcpy(info->wordtypestr,   "VRAM");                /* wordtype */
        strcpy(info->longtypestr,   "PSX VRAM Viewer"); /* longtype */
    }
    return(info);
}

int GCALL VRM_is(GSTREAM *g)
{
    return(0);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL VRM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    return(0);
}


GINFO * GCALL VRM_info(GINSTANCE *gx, int framenum)
{
    return((GINFO *)0);
}

int GCALL VRM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    return(0);
}

int GCALL VRM_close(GINSTANCE *gx)
{
    return(0);
}


