// (c) 1994-2007 Electronic Arts Inc.

#ifndef __DctGIMEX
#define __DctGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Dct Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL DCT_about(void);
int      GCALL DCT_is(GSTREAM *g);

/* Import Functions */

int      GCALL DCT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL DCT_info(GINSTANCE *gx, int framenum);
int      GCALL DCT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL DCT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL DCT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL DCT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL DCT_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct
{
    /* DCT FILE HEADER structure 16 bytes */

    char chunkid[4];            /* chunk id 'DCTy' */
    char chunksize[4];          /* size of chunk, including header */
    char framewidth[2];         /* image width in pixels */
    char frameheight[2];        /* image height in pixels */
    char quality[1];            /* quality setting 0..100 */
    char channels[1];           /* number of channels 1,3,4 */
    char pad[2];                /* pad header to 16 */
} DCTHeaderStruct;

typedef struct DCTCOORDdef
{
    int x;
    int y;
    int z;
} DCTCOORD;

#define DCTGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define DCTPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

