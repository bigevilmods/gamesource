// (c) 1994-2007 Electronic Arts Inc.

#ifndef __ALPHA_H
#define __ALPHA_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"


/****************************************************************/
/* Alpha CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL ALPHA_about(void);

CRINSTANCE *GCALL ALPHA_open(void);
int         GCALL ALPHA_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL ALPHA_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
