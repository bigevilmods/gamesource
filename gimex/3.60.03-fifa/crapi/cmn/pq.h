// (c) 1996-2007 Electronic Arts Inc.

#ifndef __PQ_H
#define __PQ_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Pq CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL PQ_about(void);

CRINSTANCE *GCALL PQ_open(void);
int         GCALL PQ_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL PQ_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
