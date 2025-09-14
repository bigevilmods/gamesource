// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FCSMAP_H
#define __FCSMAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Color Space Remap Filter                                     */
/****************************************************************/

FPUBLIC FABOUT* aboutcsmap(void);

FPUBLIC int     opencsmap(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infocsmap(FINSTANCE* fx, int framenum);
FPUBLIC int     filtercsmap(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closecsmap(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
