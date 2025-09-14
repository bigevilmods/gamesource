// (c) Electronic Arts. All Rights Reserved.

#ifndef __REFCODEX
#define __REFCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CODEX_H
#error "Include codex.h before refcodex.h"
#endif

/****************************************************************/
/*  REF Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL REF_about(void);
bool        GCALL REF_is(const void *compresseddata);

/* Decode Functions */

size_t        GCALL REF_size(const void *compresseddata);
#ifdef __cplusplus
size_t        GCALL REF_decode(void *dest, const void *compresseddata, size_t *compressedsize=0);
#else
size_t        GCALL REF_decode(void *dest, const void *compresseddata, size_t *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
size_t        GCALL REF_encode(void *compresseddata, const void *source, size_t sourcesize, int *opts=0);
#else
size_t        GCALL REF_encode(void *compresseddata, const void *source, size_t sourcesize, int *opts);
#endif

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

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

