// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FU_H
#define __FU_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Fu CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL FU_about(void);

CRINSTANCE *GCALL FU_open(void);
int         GCALL FU_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL FU_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
