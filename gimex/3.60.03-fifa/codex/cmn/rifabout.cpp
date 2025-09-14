// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               RifPack - Backward Riference Codex                 */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.10 941010 FB First codex prototype module                   */
/*    0.90 941019 FB #includable                                    */
/*    1.00 950108 FB frozen API at 1.00                             */
/*    1.01 010426 FB 32 size field                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* used hash table and link table for speed                         */
/* Reentrant                                                        */
/* Files: rifread.c rifwrite.c rifcodex.h (rifmatch.asm pc)         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* rifpack is a sliding window (131k) lzss method, with byte        */
/* oriented coding.                                                 */
/*                                                                  */
/* huff fb6 style header:                                           */
/*      *10fb  fb6      rifpack 1.0  riference pack                 */
/*      *90fb  fb6      rifpack 1.01 32 bit riference pack          */
/*                                                                  */
/*                                                                  */
/* header:                                                          */
/* [10fb] [totalunpacksize] [nextunpacksize]                        */
/*   2         3                  3                                 */
/* [90fb] [totalunpacksize] [nextunpacksize]                        */
/*   2         4                  4                                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* format is:                                                       */
/* ----------                                                       */
/* 0ffnnndd_ffffffff          short rif, f=0..1023,n=3..10,d=0..3   */
/* 10nnnnnn_ddffffff_ffffffff long rif, f=0..16384,n=4..67,d=0..3   */
/* 110fnndd_f.._f.._nnnnnnnn  very long,f=0..131071,n=5..1028,d=0..3*/
/* 111ddddd                   literal, d=4..112                     */
/* 111111dd                   eof, d=0..3                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "rifcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL RIF_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID(0,'R','E','F');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 1;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.01");     /* version # */
        strcpy(info->shorttypestr,  "rif");      /* type */
        strcpy(info->longtypestr,   "Rifpack");    /* longtype */
    }
    return(info);
}

