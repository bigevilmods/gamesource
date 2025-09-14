// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FWRAP_H
#define __FWRAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Wrap Filter                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutwrap(void);

FPUBLIC int     openwrap(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infowrap(FINSTANCE *fx, int framenum);
FPUBLIC int     filterwrap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closewrap(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

