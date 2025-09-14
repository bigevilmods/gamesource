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

#ifndef __IDECDECODE_H
#define __IDECDECODE_H 1

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

#if defined(_MSC_VER) && !defined(_XBOX)
#define CDECL __cdecl
#else
#define CDECL
#endif

#define IDECFLAG_SONYIPU 1
#define IDECFLAG_MPEG2 2
#define IDECFLAG_INTRAVLC 4



int CDECL IDEC_is_idec(const void *sourcebuffer);

int CDECL IDEC_decodeinfo(const void *sourcebuffer, int *width, int *height, int *alpha_depth,int *quant, int *compressedsize);

// return 1=success, 0=failure

int CDECL IDEC_decode( void *destbuffer, int destrowbytes,const void *sourcebuffer,int width, int height, int flags);

#ifdef R5900


#define IDECLOADIMAGE_DMA_WAITBEFORE	(1<<0)
#define IDECLOADIMAGE_DMA_WAITCOMPLETED	(1<<1)


int CDECL IDEC_Init_Decoder(void);

int CDECL IDEC_Load_Image(const unsigned char *sourcebuffer,const unsigned char *Image_Buffer,int dest_addr, int width, int height,int dest_width,int PSM,int dxoffset,int dyoffset, unsigned int flags);


#endif

// return 1=success, 0=failure


#ifdef __cplusplus
}
#endif

#endif

