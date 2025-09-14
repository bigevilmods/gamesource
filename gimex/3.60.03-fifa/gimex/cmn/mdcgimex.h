// (c) 1995-2007 Electronic Arts Inc.

#ifndef __MDCGIMEX
#define __MDCGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  MDEC Gimex                                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL MDC_about(void);
int      GCALL MDC_is(GSTREAM *g);

/* Import Functions */

int      GCALL MDC_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL MDC_info(GINSTANCE *gx, int framenum);
int      GCALL MDC_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL MDC_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL MDC_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL MDC_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL MDC_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif



