// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FSCALE_H
#define __FSCALE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Scale Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT* aboutscale(void);

FPUBLIC int     openscale(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infoscale(FINSTANCE* fx, int framenum);
FPUBLIC int     filterscale(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closescale(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
