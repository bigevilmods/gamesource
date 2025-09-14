// (c) 2000-2007 Electronic Arts Inc.

#ifndef __FFASTBLUR_H
#define __FFASTBLUR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Faster Blur Filter (slightly less accurate)                  */
/****************************************************************/

FPUBLIC FABOUT *aboutfastblur   (void);

FPUBLIC int     openfastblur    ( FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infofastblur    ( FINSTANCE *fx, int framenum);
FPUBLIC int     filterfastblur  ( FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closefastblur   ( FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

