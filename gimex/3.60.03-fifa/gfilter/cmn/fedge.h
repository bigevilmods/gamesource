// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FEDGE_H
#define __FEDGE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Edge Cleaner Filter                                          */
/****************************************************************/

FPUBLIC FABOUT* aboutedge(void);

FPUBLIC int     openedge(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infoedge(FINSTANCE* fx, int framenum);
FPUBLIC int     filteredge(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closeedge(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
