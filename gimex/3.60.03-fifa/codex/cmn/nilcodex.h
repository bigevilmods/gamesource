// (c) 1995-2007 Electronic Arts Inc.

#ifndef __NILCODEX
#define __NILCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  NIL Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL CNIL_about(void);
bool        GCALL CNIL_is(const void *compresseddata);

/* Decode Functions */

int        GCALL CNIL_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL CNIL_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL CNIL_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL CNIL_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL CNIL_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

#ifdef __cplusplus
}
#endif
#endif

