// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FMEDIAN_H
#define __FMEDIAN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Median Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutmedian(void);

FPUBLIC int     openmedian(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infomedian(FINSTANCE *fx, int framenum);
FPUBLIC int     filtermedian(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closemedian(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

