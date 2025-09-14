// (c) 1995-2007 Electronic Arts Inc.

#ifndef __PSort_H
#define __PSort_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* PSort Filter                                                 */
/****************************************************************/

FABOUT* aboutpsort(void);

int    openpsort(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
GINFO* infopsort(FINSTANCE* fx, int framenum);
int    filterpsort(FINSTANCE* fx, GINFO* ginfo, char *dest, int rowbytes);
int    closepsort(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif

#endif  // __PSort_H

