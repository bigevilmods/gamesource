// (c) 1994-2007 Electronic Arts Inc.

#ifndef __YUVGIMEX
#define __YUVGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Yuv Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL YUV_about(void);
int      GCALL YUV_is(GSTREAM *g);

/* Import Functions */

int      GCALL YUV_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL YUV_info(GINSTANCE *gx, int framenum);
int      GCALL YUV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL YUV_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL YUV_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL YUV_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL YUV_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct yuvcoorddef
{
    int x;
    int y;
    int z;
} YUVCOORD;

#ifdef __cplusplus
}
#endif
#endif

