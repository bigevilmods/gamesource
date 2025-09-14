// (c) 1995-2007 Electronic Arts Inc.

#ifndef __NIL_H
#define __NIL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Nil CRapi Module                                             */
/****************************************************************/

CRABOUT *   GCALL NILCR_about(void);

CRINSTANCE *GCALL NILCR_open(void);
int         GCALL NILCR_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL NILCR_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
