// (c) 2000-2007 Electronic Arts Inc.

#ifndef __NEWGIMEX
#define __NEWGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  New Image Generator Gimex                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL NEW_about(void);
int      GCALL NEW_is(GSTREAM *g);

/* Import Functions */

int      GCALL NEW_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL NEW_info(GINSTANCE *gx, int framenum);
int      GCALL NEW_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL NEW_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL NEW_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL NEW_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL NEW_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

