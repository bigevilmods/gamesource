// (c) 1997-2007 Electronic Arts Inc.

#ifndef __FDOUBLED_H
#define __FDOUBLED_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Double Dither Filter                                         */
/****************************************************************/

FPUBLIC FABOUT *aboutdoubled(void);

FPUBLIC int     opendoubled(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infodoubled(FINSTANCE *fx, int framenum);
FPUBLIC int     filterdoubled(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closedoubled(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

