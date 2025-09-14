// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FBLUR_H
#define __FBLUR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Blur Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutblur(void);

FPUBLIC int     openblur(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoblur(FINSTANCE *fx, int framenum);
FPUBLIC int     filterblur(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeblur(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

