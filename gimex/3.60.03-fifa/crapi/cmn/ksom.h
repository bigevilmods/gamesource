// (c) 1996-2007 Electronic Arts Inc.

#ifndef __KSOM_H
#define __KSOM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

/****************************************************************/
/* Kohenen Self Organizing Map crapi Module                                           */
/****************************************************************/

CRABOUT *   GCALL KSOM_about(void);

CRINSTANCE *GCALL KSOM_open(void);
int         GCALL KSOM_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes);
ARGB *      GCALL KSOM_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours);

#ifdef __cplusplus
}
#endif
#endif
