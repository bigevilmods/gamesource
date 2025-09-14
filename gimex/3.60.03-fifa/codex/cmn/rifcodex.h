// (c) 1995-2007 Electronic Arts Inc.

#ifndef __RIFCODEX
#define __RIFCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CODEX_H
#error "Include codex.h before rifcodex.h"
#endif

/****************************************************************/
/*  RIF Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL RIF_about(void);
bool        GCALL RIF_is(const void *compresseddata);

/* Decode Functions */

int        GCALL RIF_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL RIF_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL RIF_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL RIF_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL RIF_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
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

