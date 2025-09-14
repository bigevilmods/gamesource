// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FXCOLOUR_H
#define __FXCOLOUR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Xcolour Filter                                               */
/****************************************************************/

FPUBLIC FABOUT *aboutxcolour(void);

FPUBLIC int     openxcolour(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoxcolour(FINSTANCE *fx, int framenum);
FPUBLIC int     filterxcolour(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closexcolour(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

