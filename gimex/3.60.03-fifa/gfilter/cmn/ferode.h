// (c) 2000-2007 Electronic Arts Inc.

#ifndef __FERODE_H
#define __FERODE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"


/****************************************************************/
/* Erode (PSP) or Minimize (Photoshop) filter                   */
/****************************************************************/

FPUBLIC FABOUT* abouterode      (void);

FPUBLIC int     openerode       ( FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*  infoerode       ( FINSTANCE* fx, int framenum);
FPUBLIC int     filtererode     ( FINSTANCE* fx, GINFO* ginfo, char* base, int rowbytes);
FPUBLIC int     closeerode      ( FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif
