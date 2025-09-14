// (c) 1994-2007 Electronic Arts Inc.

#ifndef __SCRGIMEX
#define __SCRGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  SCR Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL SCR_about(void);
int      GCALL SCR_is(GSTREAM *g);

/* Import Functions */

int      GCALL SCR_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL SCR_info(GINSTANCE *gx, int framenum);
int      GCALL SCR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL SCR_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL SCR_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL SCR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL SCR_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

