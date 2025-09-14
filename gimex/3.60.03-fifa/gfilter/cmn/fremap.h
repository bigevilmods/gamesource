// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FREMAP_H
#define __FREMAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Remap Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT* aboutremap(void);

FPUBLIC int     openremap(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * inforemap(FINSTANCE *fx, int framenum);
FPUBLIC int     filterremap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeremap(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
