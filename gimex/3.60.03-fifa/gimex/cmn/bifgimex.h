// (c) 1994-2007 Electronic Arts Inc.

#ifndef __BIFGIMEX
#define __BIFGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  BIF Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL BIF_about(void);
int      GCALL BIF_is(GSTREAM *g);

/* Import Functions */

int      GCALL BIF_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL BIF_info(GINSTANCE *gx, int framenum);
int      GCALL BIF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL BIF_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL BIF_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL BIF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL BIF_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

