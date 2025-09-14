// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                DTL - PSX H201 Artists Card Gimex                 */
/*                                                                  */
/*                         by Gerry Shaw                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ------ -- --------                                       */
/*    1.00 950510 GS inital version                                 */
/*    1.01 950525 FB added closedtl stub                            */
/*    1.02 950529 FB ginstance signature                            */
/*    1.03 950614 GS subtype defines PSX video mode                 */
/*    1.04 950616 GS faster                                         */
/*    1.05 950817 FB gmax for SGIs                                  */
/*    1.06 950831 FB watcom 10.5 requires ebx be saved in cdecl     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Supports 8 and 32 bit                                            */
/* Needs dtl.obj linked with app                                    */
/* Only available on PC                                             */
/* Writes to ADS card, not a file                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "dtlgimex.h"

/****************************************************************/
/* Local Functions                                              */
/****************************************************************/


/****************************************************************/
/* Information Functions                                        */
/****************************************************************/

GABOUT* DTL_about(void)
{
    GABOUT* info;

    info = (GABOUT*) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','D','T','L');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 0;    /* supports importing */
        info->importpacked    = 0;    /* supports importing packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->exportpacked    = 0;    /* supports exporting packed */
#ifdef MSDOS
        info->canexport          = 1;    /* supports exporting */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
#else
        info->canexport          = 0;    /* supports exporting */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
#endif
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 1;    /* module uses external resources */
        info->usesfile        = 0;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        strcpy(info->extensions[0], ".dtl");                /* extensions */
        strcpy(info->authorstr,     "Gerry Shaw");          /* author */
        strcpy(info->versionstr,    "1.06");                /* version # */
        strcpy(info->shorttypestr,  "DTL");                 /* type */
        strcpy(info->wordtypestr,   "PSXADS");              /* wordtype */
        strcpy(info->longtypestr,   "PSX Artist Download"); /* longtype */
    }
    return(info);
}


int GCALL DTL_is(GSTREAM* g)
{
    return(0);
}

/****************************************************************/
/* Import Functions                                             */
/****************************************************************/

int GCALL DTL_open(GINSTANCE** gxptr, GSTREAM* g, const char* pathname,bool framecountflag)
{
    return(0);
}

GINFO * GCALL DTL_info(GINSTANCE* gx, int framenum)
{
    return((GINFO *)0);
}

int GCALL DTL_read(GINSTANCE* gx, GINFO* ginfo, char* dst, int rowbytes)
{
    return(0);
}

int GCALL DTL_close(GINSTANCE* gx)
{
    return(0);
}

