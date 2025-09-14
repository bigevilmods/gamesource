// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FYIQ_H
#define __FYIQ_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* YIQ Filter                                                   */
/****************************************************************/

FPUBLIC FABOUT *aboutfyi(void);

FPUBLIC int     openfyi(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infofyi(FINSTANCE *fx, int framenum);
FPUBLIC int     filterfyi(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closefyi(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

