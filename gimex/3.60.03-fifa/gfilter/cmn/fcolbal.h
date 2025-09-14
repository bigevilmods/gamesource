// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FCOLBAL_H
#define __FCOLBAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Color Balance Filter                                         */
/****************************************************************/

FPUBLIC FABOUT* aboutcolbal(void);

FPUBLIC int     opencolbal(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infocolbal(FINSTANCE* fx, int framenum);
FPUBLIC int     filtercolbal(FINSTANCE* fx, GINFO* ginfo, char* dest, int rowbytes);
FPUBLIC int     closecolbal(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
