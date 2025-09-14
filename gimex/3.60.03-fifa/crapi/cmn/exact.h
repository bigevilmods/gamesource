// (c) 1996-2007 Electronic Arts Inc.

#ifndef __EXACT_H
#define __EXACT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Exact CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL EXACT_about(void);

CRINSTANCE *GCALL EXACT_open(void);
int         GCALL EXACT_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL EXACT_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
