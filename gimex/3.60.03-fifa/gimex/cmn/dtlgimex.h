// (c) 1994-2007 Electronic Arts Inc.

#ifndef __DTLGIMEX
#define __DTLGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/* DTL Gimex                                                    */
/****************************************************************/

/* Information Functions */

GABOUT*  DTL_about(void);
int      GCALL DTL_is(GSTREAM* g);

/* Import Functions */

int      GCALL DTL_open(GINSTANCE** gx, GSTREAM* g, const char* pathname,bool framecountflag);
GINFO *  GCALL DTL_info(GINSTANCE* gx, int framenum);
int      GCALL DTL_read(GINSTANCE* gx, GINFO* ginfo, char* dst, int rowbytes);
int      GCALL DTL_close(GINSTANCE* gx);

/* Export Functions */

int      GCALL DTL_wopen(GINSTANCE** gx, GSTREAM* g, const char* pathname, int numframes);
int      GCALL DTL_write(GINSTANCE* gx, const GINFO* ginfo, char* source, int rowbytes);
int      GCALL DTL_wclose(GINSTANCE* gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

/* subtype defines video mode when writing */

#define PSX_RESOLUTION_320x240  1
#define PSX_RESOLUTION_512x240  2
#define PSX_RESOLUTION_MIN      1
#define PSX_RESOLUTION_MAX      2
#define PSX_RESOLUTION_DEFAULT  0


#ifdef __cplusplus
}
#endif
#endif

