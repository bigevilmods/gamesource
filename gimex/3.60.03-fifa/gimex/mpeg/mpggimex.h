// (c) 1996-2007 Electronic Arts Inc.

#ifndef __MPGGIMEX
#define __MPGGIMEX 1

#include "gimex.h"

#if GIMEX_VERSION < 321
#error module needs GIMEX_VERSION 3.21 or later
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  MPEG Gimex                                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL MPG_about(void);
int      GCALL MPG_is(GSTREAM *g);

/* Import Functions */

int      GCALL MPG_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL MPG_info(GINSTANCE *gx, int framenum);
int      GCALL MPG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL MPG_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL MPG_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL MPG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL MPG_wclose(GINSTANCE *gx);

#define SUBTYPE_TYPEMASK 7
#define SUBTYPE_MPC 0
#define SUBTYPE_M2V 1
#define SUBTYPE_MPA 2
#define SUBTYPE_MPG 3
#define SUBTYPE_MPV 4
#define SUBTYPE_VCD 5


typedef struct  
{
   int Identifier;
   int Flags;
   int Offset;
   int Length;
}AviIndex;


#ifdef __cplusplus
};
#endif
#endif
