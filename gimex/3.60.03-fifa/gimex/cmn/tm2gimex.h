// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TM2GIMEX
#define __TM2GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TM2 Sony PS2 Texture Gimex                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TM2_about(void);
int      GCALL TM2_is(GSTREAM *g);

/* Import Functions */

int      GCALL TM2_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TM2_info(GINSTANCE *gx, int framenum);
int      GCALL TM2_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TM2_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TM2_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TM2_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TM2_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    /* TIM2 FILE HEADER structure 16 bytes */

    char FileId[4];      // File ID (always "T", "I", "M", "2")
    char FormatVersion;  // File format version number
    char FormatId;       // Format ID
    char Pictures[2];    // Number of picture data parts
    char Reserved[8];    // Padding (always 0x00)
} TM2HeaderStruct;

typedef struct
{
    /* TIM2 PICTURE HEADER structure 48 bytes */

    char TotalSize[4];   // Total size in bytes of all picture data
    char ClutSize[4];    // Size in bytes of CLUT data part
    char ImageSize[4];   // Size in bytes of image data part
    char HeaderSize[2];  // Size in bytes of header part
    char ClutColors[2];  // Total number of colors in CLUT data part
    char PictFormat;     // Picture format ID (always 0)
    char MipMapTextures; // Number of MIPMAP textures
    char ClutType;       // CLUT data part type
    char ImageType;      // Image data part type
    char ImageWidth[2];  // Picture width
    char ImageHeight[2]; // Picture height
    char GsTex0[8];      // GS TEX0 register data
    char GsTex1[8];      // GS TEX1 register data
    char GsRegs[4];      // GS TEXA, FBA, PABE register data
    char GsTexClut[4];   // GS TEXCLUT register data
} TIM2_PICTUREHEADER;

#define TM2GETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TM2PUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

