// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FPHOTOSHOP_H
#define __FPHOTOSHOP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Photoshop Filter                                                  */
/****************************************************************/
FPUBLIC FABOUT *aboutphotoshop(void);
FPUBLIC int     openphotoshop(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infophotoshop(FINSTANCE *fx, int framenum);
FPUBLIC int     filterphotoshop(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closephotoshop(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

