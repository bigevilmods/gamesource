// (c) 1994-2007 Electronic Arts Inc.

#ifndef __RAWGIMEX
#define __RAWGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Raw Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL RAW_about(void);
int      GCALL RAW_is(GSTREAM *g);

/* Import Functions */

int      GCALL RAW_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL RAW_info(GINSTANCE *gx, int framenum);
int      GCALL RAW_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL RAW_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL RAW_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL RAW_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL RAW_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

