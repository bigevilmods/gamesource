// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSAFE_H
#define __FSAFE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Safe Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutsafe(void);

FPUBLIC int     opensafe(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infosafe(FINSTANCE *fx, int framenum);
FPUBLIC int     filtersafe(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closesafe(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

