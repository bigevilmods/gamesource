// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSHRINK_H
#define __FSHRINK_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Shrink To Fit Filter                                         */
/****************************************************************/

FPUBLIC FABOUT *aboutshrink(void);

FPUBLIC int     openshrink(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoshrink(FINSTANCE *fx, int framenum);
FPUBLIC int     filtershrink(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeshrink(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

