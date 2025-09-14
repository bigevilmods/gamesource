// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FDEBAND_H
#define __FDEBAND_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

    /* Parameter values for best quality. */
#define RGBDIST_BEST    16
#define IMGSTEP_BEST    1
#define BLKSIZE_BEST    8
#define BLKSTEP_BEST    1
#define CLRAREA_BEST    8

    /* Parameter values for best performance with
       reasonable quality. */
#define RGBDIST_FAST    16
#define IMGSTEP_FAST    4
#define BLKSIZE_FAST    4
#define BLKSTEP_FAST    1
#define CLRAREA_FAST    8

    /* Defaults--go for quality. */
#define RGBDIST_DEF     RGBDIST_BEST
#define IMGSTEP_DEF     IMGSTEP_BEST
#define BLKSIZE_DEF     BLKSIZE_BEST
#define BLKSTEP_DEF     BLKSTEP_BEST
#define CLRAREA_DEF     CLRAREA_BEST

/****************************************************************/
/* Deband Filter                                                */
/****************************************************************/

FPUBLIC FABOUT* aboutdeband(void);

FPUBLIC int     opendeband(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infodeband(FINSTANCE *fx, int framenum);
FPUBLIC int     filterdeband(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closedeband(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif
