// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FACOPY_H
#define __FACOPY_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Alpha Copy Filter                                            */
/****************************************************************/

FPUBLIC FABOUT *aboutacopy(void);

FPUBLIC int     openacopy(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoacopy(FINSTANCE *fx, int framenum);
FPUBLIC int     filteracopy(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeacopy(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

