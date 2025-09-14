// (c) 2004-2007 Electronic Arts Inc.
#ifndef __ZIPCODEX
#define __ZIPCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CODEX_H
#error "Include codex.h before zipcodex.h"
#endif

/****************************************************************/
/*  ZIP Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL ZIP_about(void);
bool        GCALL ZIP_is(const void *compresseddata);

/* Decode Functions */

int        GCALL ZIP_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL ZIP_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL ZIP_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL ZIP_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL ZIP_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

// zip codex header for size+signature
typedef struct
{
    char signature[2];                    /* 0x12fb/0x92fb */
    char uncompressedsize[4];             // size stored big endian
} ZipInternalHeader;

#define ZIP_INTERNAL_SIG          0x92fb  /* 12fb */
#define ZIP_INTERNAL_HEADER_SIZE  6

#ifndef qmin
#define qmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef qmax
#define qmax(a,b) ((a)>(b)?(a):(b))
#endif

#ifdef __cplusplus
}
#endif
#endif

