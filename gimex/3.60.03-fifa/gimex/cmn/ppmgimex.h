// (c) 1994-2007 Electronic Arts Inc.

#ifndef __PPMGIMEX
#define __PPMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  PPM Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PPM_about(void);
int      GCALL PPM_is(GSTREAM *g);

/* Import Functions */

int      GCALL PPM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL PPM_info(GINSTANCE *gx, int framenum);
int      GCALL PPM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL PPM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL PPM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL PPM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL PPM_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

