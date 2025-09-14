// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FDEINT_H
#define __FDEINT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Deint Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT *aboutdeint(void);

FPUBLIC int     opendeint(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO  *infodeint(FINSTANCE *fx, int framenum);
FPUBLIC int     filterdeint(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closedeint(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

