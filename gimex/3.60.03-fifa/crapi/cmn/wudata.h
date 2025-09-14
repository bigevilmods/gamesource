// (c) 1994-2007 Electronic Arts Inc.

#ifndef __WUDA_H
#define __WUDA_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"


/****************************************************************/
/* Alpha CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL WUDA_about(void);

CRINSTANCE *GCALL WUDA_open(void);
int         GCALL WUDA_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL WUDA_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
