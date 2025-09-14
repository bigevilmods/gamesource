// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*         Compression Decompression Exchange(CODEX) v3.00          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                    Header Module - Mar 18, 2008                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990824 FB codex API seperated from huff tool             */
/*    1.01 010427 FB fb6 32 bit size header                         */
/*    1.02 011011 FB c++ defaults                                   */
/*    2.00 011015 FB bool, dest/source, new about struct,no QPUBLIC */
/*    2.01 030331 FB islossy for dxt                                */
/*    2.02 050605 FB standalone                                     */
/*    2.03 060426 FB ps3 port                                       */
/*    3.00 080318 FB size_t for 64 bit systems                      */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __CODEX_H
#define __CODEX_H 1

#if defined(_MSC_VER)
#pragma once
#endif

#define CODEX_VERSION 203

/****************************************************************/
/*  Data Types                                                  */
/****************************************************************/

/* Info structure describing bitmaps */

typedef struct
{
    int signature;              /* signature of codex ie 'tga ' (optional) */
    int size;                   /* size of CODEXABOUT structure */
    int version;                /* version number of CODEXABOUT structure (200) */

    unsigned int decode    :1;  /* supports decoding */
    unsigned int encode    :1;  /* supports encoding */
    unsigned int size32    :1;  /* support 32 bit size field */
    unsigned int islossy   :1;  /* module is lossy (ie dxt) */
    unsigned int pad       :28;

    char versionstr[8];          /* version number of codex module ie 1.00 */
    char shorttypestr[8];        /* 3 or 4 character type string ie ref */
    char longtypestr[16];        /* full name of data format ie Refpack */
} CODEXABOUT;

#define QMAKEID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

#if !defined(GCALL)
#if defined(_MSC_VER) && !defined(_XBOX)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif

#include <stddef.h>

typedef struct QFUNCTIONS
{
    CODEXABOUT * (GCALL * CODEX_about)(void);
    bool         (GCALL * CODEX_is)(const void *compressed);
    size_t       (GCALL * CODEX_size)(const void *compressed);
    size_t       (GCALL * CODEX_decode)(void *dest, const void *source, size_t *sourcesizeptr);
    size_t       (GCALL * CODEX_encode)(void *dest, const void *source, size_t sourcesize, int *opts);
} QFUNCTIONS;

extern struct QFUNCTIONS qfunctions[];

/****************************************************************/
/*  Codex Module Example Prototypes                             */
/****************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Information Functions */

CODEXABOUT *GCALL CODEX_about(void);
bool        GCALL CODEX_is(const void *source);
size_t      GCALL CODEX_size(const void *source);

/* Decode/Encode Functions */

#ifdef __cplusplus
size_t GCALL CODEX_decode(void *dest, const void *source, size_t *sourcesizeptr=0);
size_t GCALL CODEX_encode(void *dest, const void *source, size_t sourcesize, int *opts=0);
#else
size_t GCALL CODEX_decode(void *dest, const void *source, size_t *sourcesizeptr);
size_t GCALL CODEX_encode(void *dest, const void *source, size_t sourcesize, int *opts);
#endif

#ifdef __cplusplus
}
#endif
#endif

