// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FPIXEL_H
#define __FPIXEL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Pixel Filter                                               */
/****************************************************************/

FPUBLIC FABOUT *aboutpixel(void);

FPUBLIC int     openpixel(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infopixel(FINSTANCE *fx, int framenum);
FPUBLIC int     filterpixel(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closepixel(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

