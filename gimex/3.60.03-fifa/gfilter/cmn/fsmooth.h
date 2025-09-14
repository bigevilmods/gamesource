// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FSMOOTH_H
#define __FSMOOTH_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Smooth Filter                                                */
/****************************************************************/

FPUBLIC FABOUT* aboutsmooth(void);

FPUBLIC int     opensmooth(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infosmooth(FINSTANCE* fx, int framenum);
FPUBLIC int     filtersmooth(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closesmooth(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
