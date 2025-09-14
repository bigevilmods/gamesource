// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Zip Inflate/Deflate Codex                     */
/*                                                                  */
/*              by Steve Kilthau & Frank Barchard, EAC              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 980519 SK based on inflate/deflate but with new header   */
/*    1.01 020802 FB revived to codex 2 API                         */
/*    1.02 020812 FB updated to 1.1.4 and added gzip compatibility  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: zipread.c zipwrite.c zipcodex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Based on Inflate/Deflate for gzip but uses custom header.        */
/*                                                                  */
/* codex style header:                                              */
/*      *12fb  zip 24 bit                                           */
/*      *92fb  zip 32 bit size field                                */
/*                                                                  */
/*                                                                  */
/* header:                                                          */
/* [12fb] [totalunpacksize] [nextunpacksize]                        */
/*   2         3                  3                                 */
/* [92fb] [totalunpacksize] [nextunpacksize]                        */
/*   2         4                  4                                 */
/*                                                                  */
/* When writing to a zip file, the codex header must be             */
/* stripped off first.                                              */
/*                                                                  */
/* The inflate/deflate algorithm used in this codex module will     */
/* compress and decompress files from Zip/Unzip, PKZip/PKUnzip,     */
/* Gzip, and WinZip.                                                */
/*                                                                  */
/* The ZIP_encode function can be given an (int *) as an option     */
/* which specifies the compression method to use.  This integer     */
/* must be in the range 0..9 where 0 is no compression and 9 is     */
/* full compression.  Default is 6.                                 */
/*                                                                  */
/* gzip format documentation                                        */
/* ftp://swrinde.nde.swri.edu/pub/png/documents/zlib/rfc-gzip.html#file-format */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __ZIPREAD
#define __ZIPREAD 1

#include <string.h>
#include <stdlib.h>

#include "codex.h"
#include "zipcodex.h"
#include "xceedzip/zlib.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL ZIP_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) malloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID('Z','L','I','B');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 1;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.01");     /* version # */
        strcpy(info->shorttypestr,  "zip");      /* type */
        strcpy(info->longtypestr,   "Zip");    /* longtype */
    }
    return(info);
}

#endif

