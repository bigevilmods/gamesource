// (c) 2000-2007 Electronic Arts Inc.

#ifndef __RAMGIMEX
#define __RAMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Ram Image Generator Gimex                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL RAM_about(void);
int      GCALL RAM_is(GSTREAM *g);

/* Import Functions */

int      GCALL RAM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL RAM_info(GINSTANCE *gx, int framenum);
int      GCALL RAM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL RAM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL RAM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL RAM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL RAM_wclose(GINSTANCE *gx);


/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct RAMFRAME
{
    GINFO *ginfo;
    void *image;
    struct RAMFRAME *next;
} RAMFRAME;

#define RAMMAXFILENAME 512

typedef struct RAMFILE
{
    char name[RAMMAXFILENAME];
    int framenum;
    RAMFRAME *frame;
    struct RAMFILE *next;
} RAMFILE;

extern RAMFILE *ramfiles;

#ifdef __cplusplus
}
#endif
#endif

