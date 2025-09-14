// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FDOT3_H
#define __FDOT3_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Dot3 Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutdot3(void);

FPUBLIC int     opendot3(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infodot3(FINSTANCE *fx, int framenum);
FPUBLIC int     filterdot3(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closedot3(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

