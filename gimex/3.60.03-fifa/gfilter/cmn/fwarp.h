// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FWARP_H
#define __FWARP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Wrap Filter                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutwarp(void);

FPUBLIC int     openwarp(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infowarp(FINSTANCE *fx, int framenum);
FPUBLIC int     filterwarp(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closewarp(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

