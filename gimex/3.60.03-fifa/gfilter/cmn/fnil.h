// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FNIL_H
#define __FNIL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Nil Filter                                                   */
/****************************************************************/

FPUBLIC FABOUT* aboutfnil(void);

FPUBLIC int     openfnil(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infofnil(FINSTANCE *fx, int framenum);
FPUBLIC int     filterfnil(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closefnil(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
