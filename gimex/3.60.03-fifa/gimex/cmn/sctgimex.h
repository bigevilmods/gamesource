// (c) 1994-2007 Electronic Arts Inc.

#ifndef __SCTGIMEX
#define __SCTGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Scitex SCT Gimex                                            */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL SCT_about(void);
int      GCALL SCT_is(GSTREAM *g);

/* Import Functions */

int      GCALL SCT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL SCT_info(GINSTANCE *gx, int framenum);
int      GCALL SCT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL SCT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL SCT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL SCT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL SCT_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

