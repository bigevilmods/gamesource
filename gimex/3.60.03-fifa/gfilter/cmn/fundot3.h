// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FUNDOT3_H
#define __FUNDOT3_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Dot3 Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutundot3(void);

FPUBLIC int     openundot3(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infoundot3(FINSTANCE *fx, int framenum);
FPUBLIC int     filterundot3(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closeundot3(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

