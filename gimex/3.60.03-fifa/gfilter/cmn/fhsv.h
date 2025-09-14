// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FHSV_H
#define __FHSV_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* HSV Filter                                                   */
/****************************************************************/

FPUBLIC FABOUT *abouthsv(void);

FPUBLIC int     openhsv(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infohsv(FINSTANCE *fx, int framenum);
FPUBLIC int     filterhsv(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closehsv(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

