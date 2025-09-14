// (c) 1994-2007 Electronic Arts Inc.

#ifndef __KOLB_H
#define __KOLB_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"


/****************************************************************/
/* Alpha CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL KOLB_about(void);

CRINSTANCE *GCALL KOLB_open(void);
int         GCALL KOLB_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL KOLB_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
