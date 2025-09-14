// (c) 1999-2007 Electronic Arts Inc.

#ifndef __WCTGIMEX
#define __WCTGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Wavelet Compressed Texture Gimex                            */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL WCT_about(void);
int      GCALL WCT_is(GSTREAM *g);

/* Import Functions */

int      GCALL WCT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL WCT_info(GINSTANCE *gx, int framenum);
int      GCALL WCT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL WCT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL WCT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL WCT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL WCT_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define WAVELET_VERSION 0x12

typedef struct
{
    /* Wavelet FILE HEADER structure */

    char version;
    char flags;
    char bpp;
    char mipmaps;  // 4 bytes

    char width[2];
    char height[2];  // 8 bytes
    char yquant[2];
    char prquant[2];  // 12 bytes
    char pbquant[2];
    char alphaquant[2]; // 16 bytes

    char rgbsize[4];
    char alphasize[4];  // 24 bytes

    char blue,green,red,alpha;
    char zlibsignature[4];  // 'ZLIB'
} WCTHeaderStruct;

#define WCTGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define WCTPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

