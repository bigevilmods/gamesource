// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FROTATE_H
#define __FROTATE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Rotate Filter                                                   */
/****************************************************************/

FPUBLIC FABOUT *aboutrotate(void);

FPUBLIC int     openrotate(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * inforotate(FINSTANCE *fx, int framenum);
FPUBLIC int     filterrotate(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closerotate(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

