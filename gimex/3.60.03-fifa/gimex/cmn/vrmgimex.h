// (c) 1994-2007 Electronic Arts Inc.

#ifndef __VRMGIMEX
#define __VRMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  VRM Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL VRM_about(void);
int      GCALL VRM_is(GSTREAM *g);

/* Import Functions */

int      GCALL VRM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL VRM_info(GINSTANCE *gx, int framenum);
int      GCALL VRM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL VRM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL VRM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL VRM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL VRM_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

