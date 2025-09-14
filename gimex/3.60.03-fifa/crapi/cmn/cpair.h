// (c) 1996-2007 Electronic Arts Inc.

#ifndef __CPAIR_H
#define __CPAIR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Cpair CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL CPAIR_about(void);

CRINSTANCE *GCALL CPAIR_open(void);
int         GCALL CPAIR_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL CPAIR_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
