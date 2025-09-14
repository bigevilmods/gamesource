// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FRADFILL_H
#define __FRADFILL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Radius Fill Filter                                           */
/****************************************************************/

FPUBLIC FABOUT* aboutradfill(void);

FPUBLIC int     openradfill(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  inforadfill(FINSTANCE* fx, int framenum);
FPUBLIC int     filterradfill(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closeradfill(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
