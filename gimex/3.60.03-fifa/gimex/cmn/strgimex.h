// (c) 1995-2007 Electronic Arts Inc.

#ifndef __STRGIMEX
#define __STRGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  STR Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL STR_about(void);
int      GCALL STR_is(GSTREAM *g);

/* Import Functions */

int      GCALL STR_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL STR_info(GINSTANCE *gx, int framenum);
int      GCALL STR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL STR_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL STR_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL STR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL STR_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif


typedef signed char STR_BYTE;
typedef unsigned char STR_UBYTE;
typedef signed short STR_WORD;
typedef unsigned short STR_UWORD;
typedef signed int STR_LONG;
typedef unsigned int STR_ULONG;

#define MDEC3 1
#define USRSIZE (512-8)

typedef struct
{
    STR_UWORD   id;
    STR_UWORD   type;
    STR_UWORD   secCount;
    STR_UWORD   nSectors;
    STR_ULONG   frameCount;
    STR_ULONG   frameSize;

    STR_UWORD   width;
    STR_UWORD   height;

    STR_UBYTE   reserved[12];
    STR_ULONG   data[USRSIZE];
} CDSECTOR;

typedef struct
{
    char id[4];
    char frameSize[4];
    char crc[4];
    char width[2];
    char height[2];

} DCTCHUNK;

typedef struct
{
    /* exporter */

    GSTREAM *outFile;
    STR_ULONG width;
    STR_ULONG height;
    STR_ULONG mWidth;
    STR_ULONG mHeight;
    STR_ULONG rowbytes;
    STR_ULONG sectorsPerFrame;
    STR_ULONG components;
    int   QFactor;
    int   PrevQFactor;
    STR_ULONG frameSize;
    STR_ULONG frameNumber;
    STR_ULONG reqFrameSize;
    STR_ULONG movieSize;
    ARGB  *sourceFrame;
    void *compBuff;
    double c[8][8]; /* dct coefficients */
    int   alphaSize;
    void *alphaBuff;

    /* importer */
    int   frames;
    int   framenum;
    int   subtype;
    int   indexmax;
    STR_ULONG  *index;
    signed char *QuickLen1;
    unsigned int *QuickCode1;
    signed char *QuickLen2;
    unsigned int *QuickCode2;
    signed char *QuickLen3;
    unsigned int *QuickCode3;
    signed char *QuickLen4;
    unsigned int *QuickCode4;

    int *QuickYUV;
    int *DCTTab;
} STRINFO;

#define STRGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define STRPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

/* For subtype */
#define STR_STR  0x00
#define STR_XA   0x01
#define STR_REF  0x02
#define STR_DCT  0x04
#define STR_ARGB 0x08
#define STR_DCTA 0x10

/* Different versions support different number of blocks */
#define YCC_NUM_BLOCKS 6
#define YCCA_NUM_BLOCKS 10

#ifdef __cplusplus
}
#endif
#endif

