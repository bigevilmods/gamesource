// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FCREEP_H
#define __FCREEP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Creep Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT* aboutcreep(void);

FPUBLIC int     opencreep(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infocreep(FINSTANCE* fx, int framenum);
FPUBLIC int     filtercreep(FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closecreep(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
