// (c) 1997-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 NilCodex - Null Crash Prevention                 */
/*                                                                  */
/*                      by Gerry Shaw, EAC                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970203 GS initial version for copen                      */
/*    1.01 980325 KO Changed names from nilc to nil                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "nilcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL CNIL_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID('.','N','I','L');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 0;      /* supports decoding */
        info->encode          = 0;      /* supports encoding */
        info->size32          = 0;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.01");     /* version # */
        strcpy(info->shorttypestr,  "nil");      /* type */
        strcpy(info->longtypestr,   "Nil");      /* longtype */
    }
    return(info);
}

