// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSCREEN_H
#define __FSCREEN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Screen Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutscreen(void);

FPUBLIC int     openscreen(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoscreen(FINSTANCE *fx, int framenum);
FPUBLIC int     filterscreen(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closescreen(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

