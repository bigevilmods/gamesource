// (c) 1994-2007 Electronic Arts Inc.

#ifndef __T16GIMEX
#define __T16GIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  T16 Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL T16_about(void);
int      GCALL T16_is(GSTREAM *g);

/* Import Functions */

int      GCALL T16_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL T16_info(GINSTANCE *gx, int framenum);
int      GCALL T16_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL T16_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL T16_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL T16_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL T16_wclose(GINSTANCE *gx);

/****************************************************************/
/*  T16 KeyFrame structures. FILE HEADER structure 20 bytes     */
/****************************************************************/

typedef struct
{
    char chunkid[4];            /* signature is 'T16k' */
    char chunksize[4];          /* size of chunk, including header */
    char frameWidth[2];         /* image width in pixels */
    char frameHeight[2];        /* image height in pixels */
    char reserved[4];           /* pad header to 20 bytes */
} T16KeyStruct;

/****************************************************************/
/*  T16 Frame structures.                                       */
/****************************************************************/

typedef struct
{
    char    chunkid[4];         /* signature is 'T16f' */
    char    chunksize[4];       /* size of chunk, including header */
    char    indexBits[2];       /* number of bits in index matrix */
    char    numMotion[2];       /* number of motion vector in this frame */
    char    numHighLiteral[2];  /* number of literal 16 in this frame */
    char    numLowLiteral[2];   /* number of low res literal in this frame */
    char    pad[4];             /* pad header to 20 bytes */
} T16FrameStruct;

/****************************************************************/
/*  Read structure                                              */
/****************************************************************/

#define kSizeStoreLiteral  (16*sizeof(unsigned int))
#define kSizeHighLiteral  (16*sizeof(unsigned short))
#define kSizeLowLiteral  ((4*sizeof(unsigned short))+(sizeof(unsigned int)))

#define MOTIONBITS 10

typedef struct
{
    int     x;
    int     y;
} T16MOTION;

typedef struct
{
    int        chunkSize;      /* size of last chunk read */
    int         frameWidth;     /* image width in pixels */
    int         frameHeight;    /* image height in pixels */
    int         packed;         /* 0=no,keyframe, 1=motion */
    int        keyoffset;      /* file offset to last keyframe */
    int        keyframe;       /* frame number of last keyframe */
    int         numMotion;      /* number of motion vector in this frame */
    int         numHightLiteral;/* number of high res literal in this frame */
    int         numLowLiteral;  /* number of low res literal in this frame */
    int         indexBits;      /* number of bits in index matrix */
    int         maxMotion;      /* number of motion vectors allocated */
    int         maxLiteral;     /* number of literals allocated */
    char        *current;       /* current frame pointer */
    char        *previous;      /* previous frame pointer */
    int         *indexMatrix;   /* index matrix */
    int         *rowtbl;        /* row table */
    char        *literal;       /* literal table (blocks of 4x4) */
    T16MOTION   *motion;        /* motion vectors */
    int         *offset;        /* motion offsets */
} T16INFO;

/****************************************************************/
/*  Convertion                                                  */
/****************************************************************/

#define C5to8(Rgb5,Rgb8) \
{\
    Rgb8=((Rgb5&0xF800)<<8)|((Rgb5&0xE000)<<3);\
    Rgb8|=((Rgb5&0x07E0)<<5)|((Rgb5&0x0060)<<3);\
    Rgb8|=((Rgb5&0x001F)<<3)|((Rgb5&0x001C)>>2);\
}

#define C8to5(Rgb8,Rgb5) \
{\
    Rgb5=(unsigned short)((Rgb8&0x00F80000)>>8); \
    Rgb5|=(unsigned short)((Rgb8&0x0000FC00)>>5); \
    Rgb5|=(unsigned short)((Rgb8&0x000000F8)>>3); \
}

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define CHUNKLOAD  0
#define CHUNKSKIP  1

#define T16SIGN GMAKEID('T','1','6','i')
#define T16K    GMAKEID('T','1','6','k')
#define T16F    GMAKEID('T','1','6','f')

#define T16GETFIELD(field) ggeti((void *)&field, sizeof(field))
#define T16PUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif
