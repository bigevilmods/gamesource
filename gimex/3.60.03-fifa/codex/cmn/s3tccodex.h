// (c) 1997-2007 Electronic Arts Inc.

#ifndef __S3TCCODEX
#define __S3TCCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  S3TC Codex                                                    */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL S3TC_about(void);
bool        GCALL S3TC_is(const void *compresseddata);

/* Decode Functions */

int        GCALL S3TC_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL S3TC_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL S3TC_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL S3TC_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL S3TC_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

/* s3tc extensions */

int        GCALL S3TC3_is(const void *compresseddata);
int        GCALL S3TC5_is(const void *compresseddata);
int        GCALL S3TC1B_is(const void *compresseddata);

/* Possible options for opts[0] */
/* Defines taken from s3_intrf.h */

// RGB encoding types (bit mask)
#define S3TC_ENCODE_RGB_FULL                0x0
#define S3TC_ENCODE_RGB_COLOR_KEY           0x1
#define S3TC_ENCODE_RGB_ALPHA_COMPARE       0x2
#define S3TC_ENCODE_RGB_555                 0x4 // PS2
#define S3TC_ENCODE_RGB_ATTENUATE           0x8
#define S3TC_ENCODE_RGB_NONE                0x10 /* reserve for dxt3a/5a */
#define _S3TC_ENCODE_RGB_MASK               0xff

// alpha encoding types
#define S3TC_ENCODE_ALPHA_NONE              0x000
#define S3TC_ENCODE_ALPHA_EXPLICIT          0x100
#define S3TC_ENCODE_ALPHA_INTERPOLATED      0x200
#define S3TC_ENCODE_ALPHA_INTERPOLATED_FROM_RED 0x400 /* reserve for dxn */
#define _S3TC_ENCODE_ALPHA_MASK             0xff00

// additional alpha encoding flags
#define S3TC_ENCODE_ALPHA_NEED0             0x10000
#define S3TC_ENCODE_ALPHA_NEED1             0x20000
#define S3TC_S3TC1B_ENCODEOPTION             8    // use this if you would like to
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

#define S3TC_S3TC1_CODE  0x18fb
#define S3TC_S3TC3_CODE  0x1afb
#define S3TC_S3TC5_CODE  0x1cfb
#define S3TC_S3TC1B_CODE 0x28fb
#define S3TC_BIGS3TC1_CODE  0x98fb
#define S3TC_BIGS3TC3_CODE  0x9afb
#define S3TC_BIGS3TC5_CODE  0x9cfb
#define S3TC_BIGS3TC1B_CODE 0xa8fb

//#define S3TC_S3TC3B_CODE 0x2afb
//#define S3TC_S3TC5B_CODE 0x2cfb

typedef enum
{
    S3TC_MODE_RGB = 0,
    S3TC_MODE_ALPHA_EXPLICIT,
    S3TC_MODE_ALPHA_INTERPOLATED,
    S3TC_MODE_RGB555 = 4 // flag

} S3TCMODE;

typedef struct
{
    char code[2];
    char size[3];
    char mode[1];
    char reserved[2];

} S3TCHEADER;

typedef struct
{
    char code[2];
    char size[4];               // bigger size field.
    char mode[1];
    char reserved[1];

} BIGS3TCHEADER;


#ifdef __cplusplus
}
#endif
#endif

