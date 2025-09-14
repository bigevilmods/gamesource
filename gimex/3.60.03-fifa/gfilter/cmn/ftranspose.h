// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FTRANSPOSE_H
#define __FTRANSPOSE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Transposition Filter                                         */
/****************************************************************/

FPUBLIC FABOUT *abouttrans(void);

FPUBLIC int     opentrans(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infotrans(FINSTANCE *fx, int framenum);
FPUBLIC int     filtertrans(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closetrans(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

