// (c) 1999-2007 Electronic Arts Inc.

#ifndef __WU777_H
#define __WU777_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"


/****************************************************************/
/* Wu777 CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL WU777_about(void);

CRINSTANCE *GCALL WU777_open(void);
int         GCALL WU777_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL WU777_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif

