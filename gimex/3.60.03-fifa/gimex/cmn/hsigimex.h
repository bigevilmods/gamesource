// (c) 1994-2007 Electronic Arts Inc.

#ifndef __HSIGIMEX
#define __HSIGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  HSI Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL HSI_about(void);
int      GCALL HSI_is(GSTREAM *g);

/* Import Functions */

int      GCALL HSI_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL HSI_info(GINSTANCE *gx, int framenum);
int      GCALL HSI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL HSI_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL HSI_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL HSI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL HSI_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

