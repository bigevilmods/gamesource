// (c) 1999-2007 Electronic Arts Inc.

#ifndef __WU555_H
#define __WU555_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Wu5 CRapi Module                                             */
/****************************************************************/

CRABOUT *   GCALL WU555_about(void);

CRINSTANCE *GCALL WU555_open(void);
int         GCALL WU555_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL WU555_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
