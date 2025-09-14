// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FSHARPEN_H
#define __FSHARPEN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Sharpen Filter                                               */
/****************************************************************/

FPUBLIC FABOUT *aboutsharpen(void);

FPUBLIC int     opensharpen(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infosharpen(FINSTANCE *fx, int framenum);
FPUBLIC int     filtersharpen(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closesharpen(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

