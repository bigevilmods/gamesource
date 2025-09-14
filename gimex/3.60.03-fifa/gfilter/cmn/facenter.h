// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FACENTER_H
#define __FACENTER_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Auto Center Filter                                           */
/****************************************************************/

FPUBLIC FABOUT *aboutacenter(void);

FPUBLIC int     openacenter(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoacenter(FINSTANCE *fx, int framenum);
FPUBLIC int     filteracenter(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeacenter(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

