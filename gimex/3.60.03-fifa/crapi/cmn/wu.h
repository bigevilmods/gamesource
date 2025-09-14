// (c) 1994-2007 Electronic Arts Inc.

#ifndef __WU_H
#define __WU_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Wu CRapi Module                                              */
/****************************************************************/

CRABOUT *   GCALL WU_about(void);

CRINSTANCE *GCALL WU_open(void);
int         GCALL WU_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL WU_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
