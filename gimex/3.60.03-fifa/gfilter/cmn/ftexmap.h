// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FTEXMAP_H
#define __FTEXMAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Blend Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT *abouttexmap(void);

FPUBLIC int     opentexmap(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infotexmap(FINSTANCE *fx, int framenum);
FPUBLIC int     filtertexmap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closetexmap(FINSTANCE *fx);


#ifdef __cplusplus
}
#endif
#endif

