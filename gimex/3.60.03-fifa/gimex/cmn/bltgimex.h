// (c) 1994-2007 Electronic Arts Inc.

#ifndef __BLTGIMEX
#define __BLTGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  BLT Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL BLT_about(void);
int      GCALL BLT_is(GSTREAM *g);

/* Import Functions */

int      GCALL BLT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL BLT_info(GINSTANCE *gx, int framenum);
int      GCALL BLT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL BLT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL BLT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL BLT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL BLT_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

