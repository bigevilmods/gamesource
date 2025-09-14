// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FUNEMBM_H
#define __FUNEMBM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Embm Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutunembm(void);

FPUBLIC int     openunembm(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infounembm(FINSTANCE *fx, int framenum);
FPUBLIC int     filterunembm(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeunembm(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

