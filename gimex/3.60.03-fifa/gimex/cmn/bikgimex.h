// (c) 1994-2007 Electronic Arts Inc.

#ifndef __BIKGIMEX
#define __BIKGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Bink Gimex                                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL BIK_about(void);
int      GCALL BIK_is(GSTREAM *g);

/* Import Functions */

int      GCALL BIK_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL BIK_info(GINSTANCE *gx, int framenum);
int      GCALL BIK_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL BIK_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL BIK_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL BIK_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL BIK_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

