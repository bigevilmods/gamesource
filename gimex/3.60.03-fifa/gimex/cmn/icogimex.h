// (c) 1994-2007 Electronic Arts Inc.

#ifndef __ICOGIMEX
#define __ICOGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  ICONS AND CURSOS Gimex                                      */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL ICO_about(void);
int      GCALL ICO_is(GSTREAM *g);

/* Import Functions */

int      GCALL ICO_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL ICO_info(GINSTANCE *gx, int framenum);
int      GCALL ICO_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL ICO_close(GINSTANCE *gx);

int      GCALL ICO_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL ICO_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL ICO_wclose(GINSTANCE *gx);



//------------------------------------------------------------------
// STRUCTURES

//----------------------------
// windows version of the ARGB struct.
typedef struct
{
    char rgbBlue;
    char rgbGreen;
    char rgbRed;
    char rgbReserved;
}GRGBQUAD;

//----------------------------
// common bitmap header.
// (%) means actually used in icons.
typedef struct
{
    char biSize[4];                // (%) bytes required by the structure.
    char biWidth[4];               // (%) width of bitmap in pixels.
    char biHeight[4];              // (%) height, positive-bottom up negative-topdown... combined height of XOR and AND masks
    char biPlanes[2];              // (%) number of planes for the target device (must be 1)
    char biBitCount[2];            // (%) bpp.
    char biCompression[4];         // type of compression used or requested.
    char biSizeImage[4];           // (%) size in bytes of the image. (0) for uncompressed RGB bitmaps.
    char biXPelsPerMeter[4];       // horizontal resolution in pix per meter.
    char biYPelsPerMeter[4];       // vertical resolution in pix per meter.
    char biClrUsed[4];             // # of color entries in pallete actually used.
    char biClrImportant[4];        // # of color indices considered important for displaying bitmap (0 = all)

}GBITMAPINFOHEADER;


//----------------------------
// used in the directory at the start of every icon file, one for each
// resource entry in the file. 
typedef struct
{
    char bWidth;                   // Width in pixels of the image.
    char bHeight;                  // Height in pixels of the image.
    char bColorCount;              // Number of colors in image (zero if >=8bpp)
    char bReserved;                // Must be zero.
    char wPlanes[2];               // Color Planes.
    char wBitCount[2];             // bpp.
    char dwBytesInRes[4];          // Byte counte for resource data.
    char dwImageOffset[4];         // Location of image in file.
}GICONDIRENTRY;


//----------------------------
// main icon resource header. 
typedef struct
{
    char idReserved[2];            // Must be zero.
    char idType[2];                // Resource type. 1 for icons.
    char idCount[2];               // How many images.
}GICONDIR;



//----------------------------
// Image header.
typedef struct
{
    GBITMAPINFOHEADER icHeader;     // DIB header.
    GRGBQUAD          icColors[1];  // Color Table.
    char              icXOR[1];     // DIB bits for XOR mask.
    char              icAND[1];     // DIB bits for AND mask.
}GICONIMAGE;


//----------------------------
// Internal Processing
typedef struct
{                                   // for multiframe image support.
    int NumImagesWritten;           // lets me know where I need to go to get offset info.
    int NumImagesTotal;             // lets me know total number of images.
    int NextOffsetInFile;           // lets me know where I can start writing from.
}GICONINTERNAL;

#define ICOGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define ICOPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))



#define GICO_TYPEICON 1
#define GICO_TYPECURS 2


#ifdef __cplusplus
}
#endif
#endif

