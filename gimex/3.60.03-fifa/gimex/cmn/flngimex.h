// (c) 1994-2007 Electronic Arts Inc.

#ifndef __FLNGIMEX
#define __FLNGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  FLN Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL FLN_about(void);
int      GCALL FLN_is(GSTREAM *g);

/* Import Functions */

int      GCALL FLN_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL FLN_info(GINSTANCE *gx, int framenum);
int      GCALL FLN_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL FLN_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL FLN_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL FLN_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL FLN_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define FLNGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define FLNPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

