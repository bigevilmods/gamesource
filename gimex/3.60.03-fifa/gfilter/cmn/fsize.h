// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSIZE2_H
#define __FSIZE2_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Size Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT* aboutsize(void);

FPUBLIC int     opensize(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infosize(FINSTANCE *fx, int framenum);
FPUBLIC int     filtersize(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closesize(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
