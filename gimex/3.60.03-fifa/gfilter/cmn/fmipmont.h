// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FMIPMONT_H
#define __FMIPMONT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Mipmont Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT* aboutmipmont(void);

FPUBLIC int     openmipmont(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infomipmont(FINSTANCE* fx, int framenum);
FPUBLIC int     filtermipmont(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closemipmont(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
