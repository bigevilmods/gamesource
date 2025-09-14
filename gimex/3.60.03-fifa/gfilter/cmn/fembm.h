// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FEMBM_H
#define __FEMBM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Embm Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutembm(void);

FPUBLIC int     openembm(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoembm(FINSTANCE *fx, int framenum);
FPUBLIC int     filterembm(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeembm(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

