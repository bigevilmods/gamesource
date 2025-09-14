// (c) 1996-2007 Electronic Arts Inc.

#ifndef __LLOYD_H
#define __LLOYD_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Lloyd CRapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL LLOYD_about(void);

CRINSTANCE *GCALL LLOYD_open(void);
int         GCALL LLOYD_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL LLOYD_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
