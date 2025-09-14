// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FGREY_H
#define __FGREY_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Grey Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT *aboutgrey(void);

FPUBLIC int     opengrey(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infogrey(FINSTANCE *fx, int framenum);
FPUBLIC int     filtergrey(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closegrey(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

