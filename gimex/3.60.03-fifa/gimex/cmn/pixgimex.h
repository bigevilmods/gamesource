// (c) 1994-2007 Electronic Arts Inc.

#ifndef __PIXGIMEX
#define __PIXGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  PIX Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PIX_about(void);
int      GCALL PIX_is(GSTREAM *g);

/* Import Functions */

int      GCALL PIX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL PIX_info(GINSTANCE *gx, int framenum);
int      GCALL PIX_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL PIX_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL PIX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL PIX_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL PIX_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* ALIAS PIX FILE HEADER structure 10 bytes */

    char width[2];          /* width in pixels */
    char height[2];         /* height in pixels */
    char xoffset[2];        /* unused */
    char yoffset[2];        /* unused */
    char bpp[2];            /* bits per pixel (24 or 8) */
} PIXHeaderStruct;

#define PIXGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define PIXPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif


