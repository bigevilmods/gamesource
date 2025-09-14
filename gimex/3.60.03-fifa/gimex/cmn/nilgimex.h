// (c) 1994-2007 Electronic Arts Inc.

#ifndef __NILGIMEX
#define __NILGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Nil Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL NIL_about(void);
int      GCALL NIL_is(GSTREAM *g);

/* Import Functions */

int      GCALL NIL_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL NIL_info(GINSTANCE *gx, int framenum);
int      GCALL NIL_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL NIL_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL NIL_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL NIL_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL NIL_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

