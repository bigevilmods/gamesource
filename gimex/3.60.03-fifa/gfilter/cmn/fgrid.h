// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FGRID_H
#define __FGRID_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Grid Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutgrid(void);

FPUBLIC int     opengrid(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infogrid(FINSTANCE *fx, int framenum);
FPUBLIC int     filtergrid(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closegrid(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

