// (c) 2000-2007 Electronic Arts Inc.

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/


/*
 *		TQI444 - Transformed Quantized Image Decoder Library
 *
 *		Martin Griffiths,
 *
 *
 *		(c) Electronic Arts / Bullfrog Productions Ltd, 1997-99
 *
 *
 *		Last updated : 12th April 1999.
 */

#ifndef __TQI444DECODE_H__
#define __GIMEX_H 1

#define STAND_ALONE

#ifndef STAND_ALONE

#define tqi444_set_dither_mode tqi444_set_dither_mode
#define tqi444_set_pixeldecode_format tqi444_set_pixeldecode_format
#define tqi444_decode_header tqi444_decode_header
#define tqi444_decode_image tqi444_decode_image

#else

#define tqi444_set_dither_mode tqi_set_dither_mode
#define tqi444_set_pixeldecode_format tqi_set_pixeldecode_format
#define tqi444_decode_header tqi_decode_header
#define tqi444_decode_image tqi_decode_image

#endif

static int          tqi444_Quant[64];
static unsigned char tqi444_ColorKey_BitPlane[8];
static unsigned int tqi444_clipA[1024];
static unsigned int tqi444_clipR[1024];
static unsigned int tqi444_clipG[1024];
static unsigned int tqi444_clipB[1024];
//static unsigned int tqi444_yuvluty[512];
//static unsigned int tqi444_yuvlutcb[512];
//static unsigned int tqi444_yuvlutcr[512];
//static unsigned int tqi444_yuvlutr[256*2];
//static unsigned int tqi444_yuvlutg[256*2];
//static unsigned int tqi444_yuvlutb[256*2];
static unsigned int tqi444_ColorKey16;
static unsigned int tqi444_ColorKey32;
static unsigned int tqi444_Alpha_Mask;
static unsigned int tqi444_Red_Mask;
static unsigned int tqi444_Green_Mask;
static unsigned int tqi444_Blue_Mask;
static unsigned int tqi444_Alpha_Slide;
static unsigned int tqi444_Red_Slide;
static unsigned int tqi444_Green_Slide;
static unsigned int tqi444_Blue_Slide;
static unsigned int tqi444_Alpha_Upper;
static unsigned int tqi444_Red_Upper;
static unsigned int tqi444_Green_Upper;
static unsigned int tqi444_Blue_Upper;
static unsigned int tqi444_Alpha_Shift;
static unsigned int tqi444_Red_Shift;
static unsigned int tqi444_Green_Shift;
static unsigned int tqi444_Blue_Shift;
static unsigned int tqi444_TAlpha_Lower[8];
static unsigned int tqi444_TRed_Lower[8];
static unsigned int tqi444_TBlue_Lower[8];
static unsigned int tqi444_TGreen_Lower[8];
static unsigned int tqi444_hasALPHA;
static unsigned int tqi444_hasCOLOURKEY;
static unsigned int tqi444_dither_mode;
static unsigned int tqi444_is_YUV;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{ 
	unsigned int Abits,Rbits,Gbits,Bbits;
	unsigned int Ashifts,Rshifts,Gshifts,Bshifts;
	unsigned int AlphaMask;
	unsigned int BitsPerPixel;
	unsigned int dwRBitMask;
	unsigned int dwGBitMask;
	unsigned int dwBBitMask;
} PIXELFORMAT_DESC;

typedef enum  { TQI_NODITHER=0,TQI_FLOYD_DITHER }DitherMode;

/* */
/* tqi444_load_bitmap() loads a specified tqi444 file and creates a DirectDraw Surface, */
/* (returning also the width and height of the image that was decompressed or NULL if image load failed) */
/* */

/*LPDIRECTDRAWSURFACE __cdecl tqi444_load_bitmap (char *filen,LPDIRECTDRAW lpDD,int *width,int *height); */

/* The following three functions provide a lower level interface to image decoding. */


/* tqi444_set_pixeldecode_format is used to specify the output format of uncompressed image data. */
/* e.g. for 8888 textures use (0xff,0xff0000,0xff000000,0xff00000000) */
/*		for 4444 textures use (0xf,0xf0,0xf000,0xf000) */

void tqi444_set_pixeldecode_format(unsigned int dwRBitMask,unsigned int dwGBitMask,unsigned int dwBBitMask,unsigned int dwABitMask);

/* tqi444_decode_header decodes the width and height of an image from the start of the compressed data) */

int tqi444_decode_header(unsigned char *compressed_image_data, int *real_width, int *real_height);

/* tqi444_decode_image decompresses the compressed image data(src) into the destination buffer(dst), */
/* with the specified line stride, width and height. */

int tqi444_decode_image(unsigned char *src,unsigned char * dst,unsigned int stride, int real_width, int real_height);


void tqi444_initdecode( unsigned char *);
void tqi444_decodeblock( unsigned short *dst,int row_width,int width,int height );
void tqi444_decodeblock_yuv( unsigned short *dst,int row_width,int width,int height );
void tqi444_decodeblock32( unsigned short *dst,int row_width,int width,int height );
void tqi444_decodeblock32_yuv( unsigned short *dst,int row_width,int width,int height );

#ifdef __cplusplus
}
#endif

#endif

#define TQIGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TQIPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))
