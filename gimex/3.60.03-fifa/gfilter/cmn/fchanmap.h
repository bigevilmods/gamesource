// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FCHANMAP_H
#define __FCHANMAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Chanmap Filter                                               */
/****************************************************************/

FPUBLIC FABOUT* aboutchanmap(void);

FPUBLIC int     openchanmap(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infochanmap(FINSTANCE *fx, int framenum);
FPUBLIC int     filterchanmap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closechanmap(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
