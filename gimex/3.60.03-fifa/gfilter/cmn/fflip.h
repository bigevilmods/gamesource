// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FFLIP_H
#define __FFLIP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Flip Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT* aboutflip(void);

FPUBLIC int     openflip(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoflip(FINSTANCE *fx, int framenum);
FPUBLIC int     filterflip(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeflip(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
