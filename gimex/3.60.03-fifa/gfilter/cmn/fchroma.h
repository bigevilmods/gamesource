// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FCHROMA_H
#define __FCHROMA_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Chroma Key Filter                                           */
/****************************************************************/

FPUBLIC FABOUT* aboutchroma(void);

FPUBLIC int     openchroma(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infochroma(FINSTANCE* fx, int framenum);
FPUBLIC int     filterchroma(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closechroma(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
