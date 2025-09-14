// (c) 1994-2007 Electronic Arts Inc.

#ifndef __JP2GIMEX
#define __JP2GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  JPeg 2000 Gimex                                             */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL JP2_about(void);
int      GCALL JP2_is(GSTREAM *g);

/* Import Functions */

int      GCALL JP2_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL JP2_info(GINSTANCE *gx, int framenum);
int      GCALL JP2_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL JP2_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL JP2_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL JP2_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL JP2_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

