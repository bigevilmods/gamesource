// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FLINE_H
#define __FLINE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Line Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT* aboutline(void);

FPUBLIC int     openline(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infoline(FINSTANCE* fx, int framenum);
FPUBLIC int     filterline(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes);
FPUBLIC int     closeline(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
