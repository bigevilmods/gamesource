// (c) 1994-2007 Electronic Arts Inc.

#ifndef __TPLGIMEX
#define __TPLGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  TPL Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL TPL_about(void);
int      GCALL TPL_is(GSTREAM *g);

/* Import Functions */

int      GCALL TPL_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL TPL_info(GINSTANCE *gx, int framenum);
int      GCALL TPL_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL TPL_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL TPL_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL TPL_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL TPL_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

/********************************/
typedef enum
{
    GX_TF_I4     = 0x0,
    GX_TF_I8     = 0x1,
    GX_TF_IA4    = 0x2,
    GX_TF_IA8    = 0x3,
    GX_TF_RGB565 = 0x4,
    GX_TF_RGB5A3 = 0x5,
    GX_TF_RGBA8  = 0x6,

    GX_TF_C4    = 0x8,
    GX_TF_C8    = 0x9,
    GX_TF_C14X2 = 0xa,

    GX_TF_CMPR   = 0xE,
    GX_TF_Z8     = 0x1,
    GX_TF_Z16    = 0x3,
    GX_TF_Z24X8  = 0x6

} GXTexFmt;

/********************************/
typedef enum
{
    GX_TL_IA8    = 0x0,
    GX_TL_RGB565 = 0x1,
    GX_TL_RGB5A3 = 0x2,

    GX_TL_RGBA8  = 0x3      // EA extension

} GXTlutFmt;

typedef struct
{
    unsigned char NumberOfEntries[2];   /* clut entries */
    unsigned char Pad[2];               /* reserved */
    unsigned char ClutFormat[4];        /* clut format */
    unsigned char Offset[4];            /* offset to clut entries */
} TPLClutStruct;

typedef struct
{
    unsigned char Height[2];            /* height of image in pixels */
    unsigned char Width[2];             /* width of image in pixels */
    unsigned char ImageFormat[4];       /* image format */
    unsigned char Offset[4];            /* offset to texels */
    unsigned char WrapS[4];             /* wrap S */
    unsigned char WrapT[4];             /* wrap T */
    unsigned char MinFilter[4];         /* minification filtering */
    unsigned char MagFilter[4];         /* magnification filtering */
    unsigned char LODBias[4];           /* bias lod calculation */
    unsigned char EdgeLODEnable[1];     /* avoid LOD on edge texels? */
    unsigned char ImageMinLOD[1];       /* minimum LOD level */
    unsigned char ImageMaxLOD[1];       /* maximum LOD level */
    unsigned char Pad[1];               /* reserved */

} TPLImageStruct;

#define TPLGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define TPLPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif


