// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FCROP_H
#define __FCROP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Crop Filter                                                  */
/****************************************************************/

#define CROPFILL   0
#define CROPCLAMP  1
#define CROPWRAP   2
#define CROPMIRROR 3

FPUBLIC FABOUT *aboutcrop(void);

FPUBLIC int     opencrop(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infocrop(FINSTANCE *fx, int framenum);
FPUBLIC int     filtercrop(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closecrop(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

