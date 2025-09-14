// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FHSL_H
#define __FHSL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* HSL Filter                                                   */
/****************************************************************/

FPUBLIC FABOUT *abouthsl(void);

FPUBLIC int     openhsl(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infohsl(FINSTANCE *fx, int framenum);
FPUBLIC int     filterhsl(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closehsl(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

