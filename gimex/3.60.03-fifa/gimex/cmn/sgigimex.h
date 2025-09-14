// (c) 1994-2007 Electronic Arts Inc.

#ifndef __SGIGIMEX
#define __SGIGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  SGI Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL SGI_about(void);
int      GCALL SGI_is(GSTREAM *g);

/* Import Functions */

int      GCALL SGI_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL SGI_info(GINSTANCE *gx, int framenum);
int      GCALL SGI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL SGI_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL SGI_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL SGI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL SGI_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* SGI FILE HEADER structure 512 bytes */

    char    magic[2];               /* IRIS image file magic number (474) */
    char    storage[1];             /* Storage format 0=verbatim, 1=rle */
    char    bpc[1];                 /* Bytes per pixel component (1or2) */
    char    dimension[2];           /* Number of dimensions 1-3 */
    char    xsize[2];               /* X size in pixels */
    char    ysize[2];               /* Y size in pixels */
    char    zsize[2];               /* Number of channels */
    char    pixmin[4];              /* Minimum pixel value (0) */
    char    pixmax[4];              /* Maximum pixel value (255) */
    char    dummy[4];               /* Ignored */
    char    imagename[80];          /* Image name */
    char    colormap[4];            /* Colour map id 0=normal, 1=332, 2=indexed, 3=palette */
    char    pad[404];               /* Ignored */
} SGIHeaderStruct;

#define SGIGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define SGIPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif


