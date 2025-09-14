// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FEQUALIZE_H
#define __FEQUALIZE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Equalize Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutequalize(void);

FPUBLIC int     openequalize(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoequalize(FINSTANCE *fx, int framenum);
FPUBLIC int     filterequalize(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeequalize(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

