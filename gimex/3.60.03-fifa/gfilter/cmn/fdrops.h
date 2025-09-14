// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FDROPS_H
#define __FDROPS_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Embm Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutdrops(void);
FPUBLIC int     opendrops(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infodrops(FINSTANCE *fx, int framenum);
FPUBLIC int     filterdrops(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closedrops(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

