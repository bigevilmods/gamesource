// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSTEREO_H
#define __FSTEREO_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Stereo Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutstereo(void);

FPUBLIC int     openstereo(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infostereo(FINSTANCE *fx, int framenum);
FPUBLIC int     filterstereo(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closestereo(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

