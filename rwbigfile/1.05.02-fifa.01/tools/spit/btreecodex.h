// (c) Electronic Arts. All Rights Reserved.

#ifndef __BTRCODEX
#define __BTRCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CODEX_H
#error "Include codex.h before btreecodex.h"
#endif

/****************************************************************/
/*  BTR Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL BTREE_about(void);
bool        GCALL BTREE_is(const void *compresseddata);

/* Decode Functions */

size_t     GCALL BTREE_size(const void *compresseddata);
#ifdef __cplusplus
size_t     GCALL BTREE_decode(void *dest, const void *compresseddata, size_t *compressedsize=0);
#else
size_t     GCALL BTREE_decode(void *dest, const void *compresseddata, size_t *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
size_t     GCALL BTREE_encode(void *compresseddata, const void *source, size_t sourcesize, int *opts=0);
#else
size_t     GCALL BTREE_encode(void *compresseddata, const void *source, size_t sourcesize, int *opts);
#endif

#ifdef __cplusplus
}
#endif
#endif

