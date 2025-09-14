// (c) 1995-2007 Electronic Arts Inc.

#ifndef __PALSORT_H
#define __PALSORT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* PalSort Helpers                                              */
/****************************************************************/

FPALETTE *palcountsort(GBITMAP *source);

/****************************************************************/
/* PalSort Filter                                               */
/****************************************************************/

FPUBLIC FABOUT *aboutpalsort(void);

FPUBLIC int     openpalsort(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infopalsort(FINSTANCE *fx, int framenum);
FPUBLIC int     filterpalsort(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closepalsort(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
