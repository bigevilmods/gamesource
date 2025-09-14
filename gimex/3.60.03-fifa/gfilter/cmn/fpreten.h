// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FPRETEN_H
#define __FPRETEN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Preten Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutpreten(void);

FPUBLIC int     openpreten(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infopreten(FINSTANCE *fx, int framenum);
FPUBLIC int     filterpreten(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closepreten(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

