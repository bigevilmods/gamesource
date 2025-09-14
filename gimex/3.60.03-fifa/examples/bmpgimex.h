// (c) 1994-2007 Electronic Arts Inc.

#ifndef __BMPGIMEX
#define __BMPGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  BMP Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL BMP_about(void);
int      GCALL BMP_is(GSTREAM *g);

/* Import Functions */

int      GCALL BMP_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL BMP_info(GINSTANCE *gx, int framenum);
int      GCALL BMP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL BMP_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL BMP_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL BMP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL BMP_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* BIT MAP FILE HEADER structure 14 bytes */
    char bfType[2];             /* The bytes "BM" (for bitmap) */
    char bfSize[4];             /* Total size of the file */
    char bfReserved1[2];        /* Set to 0 */
    char bfReserved2[2];        /* Set to 0 */
    char bfOffbits[4];          /* Offset to the bits from beginning of the file */

    union
    {

        struct
        {
            /* BIT MAP CORE INFO HEADER structure 12 bytes */

            char bcSize[4];             /* Size of the BITMAPINFOHEADER structure in bytes */
            char bcWidth[2];            /* Width of the bitmap in pixels */
            char bcHeight[2];           /* Height of the bitmap in pixels */
            char bcPlanes[2];           /* Set to 1 */
            char bcBitCount[2];         /* Colour bits per pixel 1,4,8,24 */
        } os2;

        /* BIT MAP INFO HEADER structure 40 bytes */

        struct
        {
            char biSize[4];             /* Size of the BITMAPINFOHEADER structure in bytes (40 or 108) */
            char biWidth[4];            /* Width of the bitmap in pixels */
            char biHeight[4];           /* Height of the bitmap in pixels */
            char biPlanes[2];           /* Set to 1 */
            char biBitCount[2];         /* Colour bits per pixel */
            char biCompression[4];      /* Compression scheme (0 or 3 for none) */
            char biSizeImage[4];        /* Size of bitmap bits in bytes (only required if compression is used) */
            char biXPelsPerMeter[4];    /* Horizontal resolution in pixels per meter */
            char biYPelsPerMeter[4];    /* Vertical resolution in pixels per meter */
            char biClrUsed[4];          /* Number of colours used in image */
            char biClrImportant[4];     /* Number of important colours used in image */

            /* ADDITIONAL BIT MAP INFO FIELDS FOR WIN95 structure +68 bytes */

            char biRedMask[4];          /* Mask identifying bits of red channel */
            char biGreenMask[4];        /* Mask identifying bits of green channel */
            char biBlueMask[4];         /* Mask identifying bits of blue channel */
            char biAlphaMask[4];        /* Mask identifying bits of alpha channel */
            char biCSType[4];           /* Colour space type (1=normal) */
            char biRedX[4];             /* X coordinate of red endpoint */
            char biRedY[4];             /* Y coordinate of red endpoint */
            char biRedZ[4];             /* Z coordinate of red endpoint */
            char biGreenX[4];           /* X coordinate of green endpoint */
            char biGreenY[4];           /* Y coordinate of green endpoint */
            char biGreenZ[4];           /* Z coordinate of green endpoint */
            char biBlueX[4];            /* X coordinate of blue endpoint */
            char biBlueY[4];            /* Y coordinate of blue endpoint */
            char biBlueZ[4];            /* Z coordinate of blue endpoint */
            char biGammaRed[4];         /* Gamma red coordinate scale value */
            char biGammaGreen[4];       /* Gamma green coordinate scale value */
            char biGammaBlue[4];        /* Gamma blue coordinate scale value */

        } win;
    } bmi;
} BMPHeaderStruct;


#define BMPWIN31    0
#define BMPOS2      1
#define BMPWIN95    2
#define BMPWINNT    3
#define BMPWIN31a   4


#define BMPGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define BMPPUTFIELD(field,data) gputi((void *)&field,(unsigned long)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

