// (c) 1994-2007 Electronic Arts Inc.

#ifndef __QT3GIMEX
#define __QT3GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  MOV Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL QT3_about(void);
int      GCALL QT3_is(GSTREAM *g);

/* Import Functions */

int      GCALL QT3_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL QT3_info(GINSTANCE *gx, int framenum);
int      GCALL QT3_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL QT3_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL QT3_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL QT3_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL QT3_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

#define QT3GETFIELD(field) ggetm((void *)&field, sizeof(field))
#define QT3PUTFIELD(field,data) gputm((void *)&field,(unsigned long)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

