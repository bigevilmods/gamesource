// (c) 1996-2007 Electronic Arts Inc.

#ifndef __MPGGIMEX
#define __MPGGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  MPEG Gimex                                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL MPG_about(void);
int      GCALL MPG_is(GSTREAM *g);

/* Import Functions */

int      GCALL MPG_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL MPG_info(GINSTANCE *gx, int framenum);
int      GCALL MPG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL MPG_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL MPG_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL MPG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL MPG_wclose(GINSTANCE *gx);


#ifdef __cplusplus
};
#endif
#endif
