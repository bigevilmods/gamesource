// (c) 1994-2007 Electronic Arts Inc.

#ifndef __ACTGIMEX
#define __ACTGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  ACT Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL ACT_about(void);
int      GCALL ACT_is(GSTREAM *g);

/* Import Functions */

int      GCALL ACT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL ACT_info(GINSTANCE *gx, int framenum);
int      GCALL ACT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL ACT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL ACT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL ACT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL ACT_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

