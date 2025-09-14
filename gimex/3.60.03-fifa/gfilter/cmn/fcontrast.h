// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FCONTRAST_H
#define __FCONTRAST_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Contrast Filter                                              */
/****************************************************************/

FPUBLIC FABOUT *aboutcontrast(void);

FPUBLIC int     opencontrast(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infocontrast(FINSTANCE *fx, int framenum);
FPUBLIC int     filtercontrast(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closecontrast(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

