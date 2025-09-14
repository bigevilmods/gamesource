// (c) 1994-2007 Electronic Arts Inc.

#ifndef __YUMGIMEX
#define __YUMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Yum Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL YUM_about(void);
int      GCALL YUM_is(GSTREAM *g);

/* Import Functions */

int      GCALL YUM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL YUM_info(GINSTANCE *gx, int framenum);
int      GCALL YUM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL YUM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL YUM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL YUM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL YUM_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct yumcoorddef
{
    int x;
    int y;
    int z;
} YUMCOORD;

#ifdef __cplusplus
}
#endif
#endif

