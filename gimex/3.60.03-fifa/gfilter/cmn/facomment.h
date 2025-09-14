// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FALPHACOMMENT_H
#define __FALPHACOMMENT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Alphacomment Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutalphacomment(void);

FPUBLIC int     openalphacomment(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoalphacomment(FINSTANCE *fx, int framenum);
FPUBLIC int     filteralphacomment(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closealphacomment(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

