// (c) 1997-2007 Electronic Arts Inc.

#ifndef __DXTCODEX
#define __DXTCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  DXT Codex                                                    */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL DXT_about(void);
bool        GCALL DXT_is(const void *compresseddata);

/* Decode Functions */

int        GCALL DXT_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL DXT_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL DXT_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL DXT_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL DXT_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

/* dxt extensions */

int        GCALL DXT3_is(const void *compresseddata);
int        GCALL DXT5_is(const void *compresseddata);
int        GCALL DXT1B_is(const void *compresseddata);

/* Possible options for opts[0] */
/* Defines taken from s3_intrf.h */

// RGB encoding types (bit mask)
#define DXT_ENCODE_RGB_FULL                0x0
#define DXT_ENCODE_RGB_COLOR_KEY           0x1
#define DXT_ENCODE_RGB_ALPHA_COMPARE       0x2
#define DXT_ENCODE_RGB_555                 0x4 // PS2
#define DXT_ENCODE_RGB_ATTENUATE           0x8
#define _DXT_ENCODE_RGB_MASK               0xff

// alpha encoding types
#define DXT_ENCODE_ALPHA_NONE              0x000
#define DXT_ENCODE_ALPHA_EXPLICIT          0x100
#define DXT_ENCODE_ALPHA_INTERPOLATED      0x200
#define _DXT_ENCODE_ALPHA_MASK             0xff00

// additional alpha encoding flags
#define DXT_ENCODE_ALPHA_NEED0             0x10000
#define DXT_ENCODE_ALPHA_NEED1             0x20000
#define DXT_DXT1B_ENCODEOPTION             8    // use this if you would like to
                                                 // do the bigendian format.
/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

/* ARGB structure used for palettes/pixels */

// experimental float channel

#ifdef GIMEXFLOAT
typedef float GCHANNEL;
#else
typedef unsigned char GCHANNEL;
#endif

#ifndef ARGB
#define ARGB ARGB
#if (defined(_MSC_VER) && !defined(_M_PPC)) || defined(__i386__)

    typedef struct
    {
        GCHANNEL b,g,r,a;
    } ARGB;

#elif defined(__R5900) || defined(SGI) /* PS2 */

    typedef struct
    {
        GCHANNEL r,g,b,a;
    } ARGB;

#else /* PS3/XBox360/GameCube/Mac */

    typedef struct
    {
        GCHANNEL a,r,g,b;
    } ARGB;
#endif
#endif

#define DXT_DXT1_CODE  0x18fb
#define DXT_DXT3_CODE  0x1afb
#define DXT_DXT5_CODE  0x1cfb
#define DXT_DXT1B_CODE 0x28fb
#define DXT_BIGDXT1_CODE  0x98fb
#define DXT_BIGDXT3_CODE  0x9afb
#define DXT_BIGDXT5_CODE  0x9cfb
#define DXT_BIGDXT1B_CODE 0xa8fb

//#define DXT_DXT3B_CODE 0x2afb
//#define DXT_DXT5B_CODE 0x2cfb

typedef enum
{
    DXT_MODE_RGB = 0,
    DXT_MODE_ALPHA_EXPLICIT,
    DXT_MODE_ALPHA_INTERPOLATED,
    DXT_MODE_RGB555 = 4 // flag

} DXTMODE;

typedef struct
{
    char code[2];
    char size[3];
    char mode[1];
    char reserved[2];

} DXTHEADER;

typedef struct
{
    char code[2];
    char size[4];               // bigger size field.
    char mode[1];
    char reserved[1];

} BIGDXTHEADER;


#ifdef __cplusplus
}
#endif
#endif

