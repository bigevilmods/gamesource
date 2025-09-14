// (c) 2000-2007 Electronic Arts Inc.

#ifndef __FDILATE_H
#define __FDILATE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"


/****************************************************************/
/* Dilate (PSP) or Maximize (Photoshop) filter                  */
/****************************************************************/

FPUBLIC FABOUT* aboutdilate     (void);

FPUBLIC int     opendilate      ( FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infodilate      ( FINSTANCE* fx, int framenum);
FPUBLIC int     filterdilate    ( FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closedilate     ( FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
