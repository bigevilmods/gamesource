// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FGRB_H
#define __FGRB_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Grb grrrrbbb Palettizer Filter                               */
/****************************************************************/

FPUBLIC FABOUT* aboutgrb(void);

FPUBLIC int     opengrb(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infogrb(FINSTANCE *fx, int framenum);
FPUBLIC int     filtergrb(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closegrb(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
