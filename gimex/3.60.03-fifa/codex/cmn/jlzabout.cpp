// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                     JLZ - Jasons LZ Codex                        */
/*                                                                  */
/*            by Jason Dorey and FrANK G. Barchard, EAC             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 070926 FB nfs08 code                                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: jlzread.c jlzwrite.c jlzcodex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* jlz comes from Need For Speed lzcompress.cpp                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "jlzcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL JLZ_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID(0,'J','L','Z');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 1;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.00");     /* version # */
        strcpy(info->shorttypestr,  "jlz");      /* type */
        strcpy(info->longtypestr,   "jlzpack");    /* longtype */
    }
    return(info);
}

