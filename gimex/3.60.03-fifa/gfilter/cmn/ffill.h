// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FFILL_H
#define __FFILL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Flood Fill Filter                                            */
/****************************************************************/

FPUBLIC FABOUT *aboutfill(void);

FPUBLIC int     openfill(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infofill(FINSTANCE *fx, int framenum);
FPUBLIC int     filterfill(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closefill(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

