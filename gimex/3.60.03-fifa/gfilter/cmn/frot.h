// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FROT_H
#define __FROT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Rot Filter                                                   */
/****************************************************************/

FPUBLIC FABOUT *aboutrot(void);

FPUBLIC int     openrot(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * inforot(FINSTANCE *fx, int framenum);
FPUBLIC int     filterrot(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closerot(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

