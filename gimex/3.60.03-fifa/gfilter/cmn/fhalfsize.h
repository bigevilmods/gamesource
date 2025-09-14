// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FHALFSIZE_H
#define __FHALFSIZE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Halfsize Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT* abouthalfsize(void);

FPUBLIC int     openhalfsize(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infohalfsize(FINSTANCE* fx, int framenum);
FPUBLIC int     filterhalfsize(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closehalfsize(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
