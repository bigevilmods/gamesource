// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FTEDGE_H
#define __FTEDGE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Tedge Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT *abouttedge(void);

FPUBLIC int     opentedge(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infotedge(FINSTANCE *fx, int framenum);
FPUBLIC int     filtertedge(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closetedge(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

