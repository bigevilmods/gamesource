// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TQIGIMEX
#define __TQIGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TQI Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TQI_about(void);
int      GCALL TQI_is(GSTREAM *g);

/* Import Functions */

int      GCALL TQI_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TQI_info(GINSTANCE *gx, int framenum);
int      GCALL TQI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TQI_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TQI_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TQI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TQI_wclose(GINSTANCE *gx);

/* NOTE TQI lib is C++ */

#ifdef __cplusplus
}
#endif
#endif