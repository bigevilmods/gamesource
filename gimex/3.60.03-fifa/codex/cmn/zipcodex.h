// (c) 1995-2007 Electronic Arts Inc.

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

/* Internal header structure - used to be shared with ZIPCHIMP module */
typedef struct
{
    char signature[4];
    char compressedsize[4];
    char uncompressedsize[4];
    char crc32[4];
} ZipInternalHeader;

#define ZIP_INTERNAL_SIG          0x7A697063
#define ZIP_INTERNAL_HEADER_SIZE  16

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

