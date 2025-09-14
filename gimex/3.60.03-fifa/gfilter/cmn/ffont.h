// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FFONT_H
#define __FFONT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Tex Drawing Filter                                           */
/****************************************************************/

FPUBLIC FABOUT* aboutfont(void);

FPUBLIC int     openfont(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infofont(FINSTANCE *fx, int framenum);
FPUBLIC int     filterfont(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closefont(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
