// (c) 1994-2007 Electronic Arts Inc.

#ifndef __EPSGIMEX
#define __EPSGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  EPS Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL EPS_about(void);
int      GCALL EPS_is(GSTREAM *g);

/* Import Functions */

int      GCALL EPS_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL EPS_info(GINSTANCE *gx, int framenum);
int      GCALL EPS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL EPS_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL EPS_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL EPS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL EPS_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

