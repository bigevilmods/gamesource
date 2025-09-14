// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FOUTLINE_H
#define __FOUTLINE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Outline Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT* aboutoutline(void);

FPUBLIC int     openoutline(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infooutline(FINSTANCE* fx, int framenum);
FPUBLIC int     filteroutline(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closeoutline(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
