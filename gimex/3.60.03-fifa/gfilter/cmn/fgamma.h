// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FGAMMA_H
#define __FGAMMA_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Gamma Correction Filter                                      */
/****************************************************************/

FPUBLIC FABOUT *aboutgamma(void);

FPUBLIC int     opengamma(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infogamma(FINSTANCE *fx, int framenum);
FPUBLIC int     filtergamma(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closegamma(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

