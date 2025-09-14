// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FINVERT_H
#define __FINVERT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Invert Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutinvert(void);

FPUBLIC int     openinvert(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoinvert(FINSTANCE *fx, int framenum);
FPUBLIC int     filterinvert(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeinvert(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

