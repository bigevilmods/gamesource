// (c) 1994-2007 Electronic Arts Inc.

#ifndef __FLMGIMEX
#define __FLMGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  FilmStrip Gimex                                             */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL FLM_about(void);
int      GCALL FLM_is(GSTREAM *g);

/* Import Functions */

int      GCALL FLM_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL FLM_info(GINSTANCE *gx, int framenum);
int      GCALL FLM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL FLM_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL FLM_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL FLM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL FLM_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* FILMSTRIP FILE HEADER structure 36 bytes at the end of file*/

    char signature[4];          /* 'Rand' */
    char numframes[4];          /* number of frames in file */
    char packing[2];            /* packing method (0) */
    char eaformat[2];           /* reserved, should be 0. ea extension sets to 2 for 1555 */
    char width[2];              /* image width in pixels */
    char height[2];             /* image height in pixels */
    char leading[2];            /* horiz gap between frames */
    char framespersec[2];       /* frame rate */
    char spare[16];             /* some spare data */
} FLMHeaderStruct;

#define FLMGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define FLMPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

