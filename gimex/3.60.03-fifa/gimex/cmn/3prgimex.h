// (c) 2006-2007 Electronic Arts Inc.

#include "gimex.h"

#ifndef __TPRGIMEX
#define __TPRGIMEX 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_MAXFRAMENAME
#define XPR_MAXFRAMENAME 64
#endif

/****************************************************************/
/*  XPR Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TPR_about(void);
int      GCALL TPR_is(GSTREAM *g);

/* Import Functions */

int      GCALL TPR_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TPR_info(GINSTANCE *gx, int framenum);
int      GCALL TPR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TPR_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TPR_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TPR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TPR_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

