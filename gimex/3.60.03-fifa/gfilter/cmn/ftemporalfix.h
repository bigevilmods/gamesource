// (c) 1996-2007 Electronic Arts Inc.

#ifndef __FTEMPORALFIX_H
#define __FTEMPORALFIX_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Temporalfix Filter                                                 */
/****************************************************************/

FPUBLIC FABOUT *abouttemporalfix(void);

FPUBLIC int     opentemporalfix(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO  *infotemporalfix(FINSTANCE *fx, int framenum);
FPUBLIC int     filtertemporalfix(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closetemporalfix(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

