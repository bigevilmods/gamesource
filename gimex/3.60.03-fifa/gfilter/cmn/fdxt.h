// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FDXT_H
#define __FDXT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Flip Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT* aboutdxt(void);

FPUBLIC int     opendxt(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infodxt(FINSTANCE *fx, int framenum);
FPUBLIC int     filterdxt(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closedxt(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
