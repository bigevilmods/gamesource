// (c) 1994-2007 Electronic Arts Inc.

#ifndef __PNGGIMEX
#define __PNGGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Png Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PNG_about(void);
int      GCALL PNG_is(GSTREAM *g);

/* Import Functions */

int      GCALL PNG_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL PNG_info(GINSTANCE *gx, int framenum);
int      GCALL PNG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL PNG_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL PNG_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL PNG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL PNG_wclose(GINSTANCE *gx);

#ifdef PNG_GIMEX_INTERNAL

    /*************************
     * Internal Declarations *
     *************************/

#   include "png.h"

    typedef struct
    {
        png_structp png_ptr;
        png_infop info_ptr;

    } PNGINFO;

#endif


#ifdef __cplusplus
}
#endif
#endif

