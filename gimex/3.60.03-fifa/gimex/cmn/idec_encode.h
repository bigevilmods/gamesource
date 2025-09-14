// (c) 2000-2007 Electronic Arts Inc.
//
//
//		PSX2/PC IDEC Encoder/Decoder
//
//		Martin Griffiths, Bullfrog Productions Ltd.
//
//
//		Last Updated : December 6th 1999
//
//		V1.01
//


#ifndef __IDECENCODE_H
#define __IDECENCODE_H 1


#ifndef ARGB
#ifdef GIMEXFLOAT
typedef float GCHANNEL;
#else
typedef unsigned char GCHANNEL;
#endif

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


#ifdef __cplusplus
extern "C"
{
#endif

// 8 byte header, that preceeds IDEC data

typedef struct
{	unsigned short id;
	unsigned char width;
	unsigned char height;
	unsigned int size:20;
	unsigned int alpha_type:2;
	unsigned int alpha_quant:5;
	unsigned int flags:5;
} IDEC_HEADER;


#define IDEC_ALPHA8BITRAW (0)
#define IDEC_ALPHA4BITRAW (1)
//#define IDEC_ALPHA_IDEC	  (2)
//#define IDEC_ALPHA_BDEC   (3)

// some definitions for the above flags field etc.

#define IDEC_HEADER_ID (0x4D47)
#define IDECFLAG_HASALPHA		(1<<0)

#if defined(_MSC_VER) && !defined(_XBOX)
#define CDECL __cdecl
#else
#define CDECL
#endif

// idec_encode()
//
// image -> 32bit ARGB bitmap
// output -> memory for compressed data output
// width - bitmap width
// height - bitmap height
// sourcerowbytes - source bytes per row.
// flags - various (see above)
// mquant - quantization factor
// 
// Returns number of bytes encoded including header.

int CDECL IDEC_encode( const ARGB *image,unsigned char *output,int width,int height, int sourcerowbytes,int flags,int mquant,int alphatype );

#ifdef __cplusplus
};
#endif

#endif
