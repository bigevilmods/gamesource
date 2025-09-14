// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSWAP_H
#define __FSWAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Swap Colours Filter                                          */
/****************************************************************/

FPUBLIC FABOUT *aboutswap(void);

FPUBLIC int     openswap(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoswap(FINSTANCE *fx, int framenum);
FPUBLIC int     filterswap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeswap(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

