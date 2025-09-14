// (c) 1994-2007 Electronic Arts Inc.

#ifndef __CLPGIMEX
#define __CLPGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Clp Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL CLP_about(void);
int      GCALL CLP_is(GSTREAM *g);

/* Import Functions */

int      GCALL CLP_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL CLP_info(GINSTANCE *gx, int framenum);
int      GCALL CLP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL CLP_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL CLP_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL CLP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL CLP_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

