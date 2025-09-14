// (c) 2003-2007 Electronic Arts Inc.

#ifndef __FSHADE_H
#define __FSHADE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Shade Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT *aboutshade(void);

FPUBLIC int     openshade(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoshade(FINSTANCE *fx, int framenum);
FPUBLIC int     filtershade(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeshade(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

