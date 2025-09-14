// (c) 1994-2007 Electronic Arts Inc.

#ifndef __YU2GIMEX
#define __YU2GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Yu2 Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL YU2_about(void);
int      GCALL YU2_is(GSTREAM *g);

/* Import Functions */

int      GCALL YU2_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL YU2_info(GINSTANCE *gx, int framenum);
int      GCALL YU2_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL YU2_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL YU2_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL YU2_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL YU2_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct yu2coorddef
{
    int x;
    int y;
    int z;
} YU2COORD;

#ifdef __cplusplus
}
#endif
#endif

