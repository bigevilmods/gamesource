// (c) 1994-2007 Electronic Arts Inc.

#ifndef __VGAGIMEX
#define __VGAGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  VGA Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL VGA_about(void);
int      GCALL VGA_is(GSTREAM *g);

/* Import Functions */

int      GCALL VGA_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL VGA_info(GINSTANCE *gx, int framenum);
int      GCALL VGA_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL VGA_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL VGA_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL VGA_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL VGA_wclose(GINSTANCE *gx);

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifdef __cplusplus
}
#endif
#endif

