// (c) 1997-2007 Electronic Arts Inc.

#ifndef __QOPEN_H
#define __QOPEN_H   1

#include "codex.h"

#ifdef __cplusplus
extern "C" {
#endif

bool        GCALL CODEX_init(void);
void        GCALL CODEX_restore(void);

CODEXABOUT* GCALL CODEX_about(void);
bool        GCALL CODEX_is(const void *compressed);
int         GCALL CODEX_size(const void *compressed);
int         GCALL CODEX_decode(void *uncompressed, const void *compressed, int *compressedsizeptr);
int         GCALL CODEX_encode(void *compressed, const void *uncompressed, int uncompressedsize, int *opts);

int         GCALL CODEX_find(const char *name);
void        GCALL CODEX_set(int typenum);     /* choose a codex, ie: CODEX_set(HUFCODEX); */
int         GCALL CODEX_get(void);            /* return codex ID */
int         GCALL CODEX_count(void);         /* return number of modules in codex library */

/* CODEX_best will find and CODEX_set to the best matching module based on the */
/* the given data, ie int CodexModule = CODEX_best(pDataBuffer); */
int         GCALL CODEX_best(char* compressed);

#ifdef __cplusplus
}
#endif
#endif
