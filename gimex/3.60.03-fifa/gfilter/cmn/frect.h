// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FRECT_H
#define __FRECT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Rect Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutrect(void);

FPUBLIC int     openrect(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * inforect(FINSTANCE *fx, int framenum);
FPUBLIC int     filterrect(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closerect(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

