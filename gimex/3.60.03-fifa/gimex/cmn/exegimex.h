// (c) 1994-2007 Electronic Arts Inc.

#ifndef __EXEGIMEX
#define __EXEGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  JPeg 2000 Gimex                                             */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL EXE_about(void);
int      GCALL EXE_is(GSTREAM *g);

/* Import Functions */

int      GCALL EXE_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL EXE_info(GINSTANCE *gx, int framenum);
int      GCALL EXE_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL EXE_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL EXE_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL EXE_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL EXE_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

