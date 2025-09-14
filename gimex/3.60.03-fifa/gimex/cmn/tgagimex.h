// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TGAGIMEX
#define __TGAGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TGA Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TGA_about(void);
int      GCALL TGA_is(GSTREAM *g);

/* Import Functions */

int      GCALL TGA_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TGA_info(GINSTANCE *gx, int framenum);
int      GCALL TGA_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TGA_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TGA_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TGA_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TGA_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* TARGA FILE HEADER structure 18 bytes */

    char identsize;             /* size of ID field that follows 18 byte header (0 usually) */
    char colourmaptype;         /* type of colour map 0=none, 1=has palette */
    char imagetype;             /* type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed */

    char colourmapstart[2];     /* first colour map entry in palette */
    char colourmaplength[2];    /* number of colours in palette */
    char colourmapbits;         /* number of bits per palette entry 15,16,24,32 */

    char xstart[2];             /* image x origin */
    char ystart[2];             /* image y origin */
    char width[2];              /* image width in pixels */
    char height[2];             /* image height in pixels */
    char bits;                  /* image bits per pixel 8,16,24,32 */
    char descriptor;            /* image descriptor bits (vh flip bits) */
} TGAHeaderStruct;

#define TGAGETSIGNEDFIELD(field) ((int)(ggeti((void *)&field, sizeof(field))<<(32-sizeof(field)*8))>>(32-sizeof(field)*8))

#define TGAGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TGAPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

