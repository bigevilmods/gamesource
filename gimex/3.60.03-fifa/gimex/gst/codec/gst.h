// (c) 2003-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*              Graphic Synthesizer Texture (GST) v1.00             */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                    Header Module - Nov 10, 2003                  */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef GST_H
#define GST_H

#if defined(_MSC_VER)
#pragma once
#endif

#define GST_VERSION 100

namespace Gst
{

// detail follows base texture, but with this much alignment introduced if necessary
static const int GSTDETAILALIGNMENT=16;

// pixel formats should match gimex/realshape

#if defined(_MSC_VER) || defined(__i386__)

    typedef struct
    {
        unsigned char b,g,r,a;
    } GstArgb;

#elif defined(__R5900) || defined(SGI) /* PS2 */

    typedef struct
    {
        unsigned char r,g,b,a;
    } GstArgb;

#else /* GameCube/Mac */

    typedef struct
    {
        unsigned char a,r,g,b;
    } GstArgb;
#endif

// types 8..f    0000_01dww where d=detail, w=width

enum GstType 
{				// block width, height & detail bpp
	GST1=0x8,	// 1x2x1
	GST2=0x9,	// 2x2x1
	GST3=0xa,	// 4x2x1
	GST4=0xb,	// 8x2x1
	GST5=0xc,	// 1x2x2
	GST6=0xd,	// 2x2x2
	GST7=0xe,	// 4x2x2
	GST8=0xf 	// 8x2x2
};

struct GstImageInfo
{
	// input image specification

	const void *mImage;		// source image (may be 8 or 32 bpp)
	int mRowBytes;			// source image rowbytes
	int mWidth;				// image width
	int mHeight;			// image height
	int mBpp;				// 8 or 32
	const GstArgb *mClut;	// source image clut if 8 bit
};


// Query Functions

void GetTypeInfo(GstType GstType, int *BlockWidth, int *BlockHeight, int *DetailBpp);	// return the block information based on a GstType
GstType GSTEncodeType(int BlockWidth=4,int BlockHeight=2,int DetailBpp=2); // determine GstType given block size
int GSTEncodeSize(GstType GstType, int width, int height); // return the size that the compressed gst texture data will be

// Decode functions

bool GSTDecode(void *rawoutdatabuffer, const void *ingstdata, GstType gsttype, int width, int height, int IsSwizzleTexture=1);  // decode raw texels of a gst texture.  Note that clut and other information is responsibility of the caller

// Encode functions

bool GSTEncodePalette(GstImageInfo *info, GstType GstType, GstArgb *outclut, int *numclutentriesout, int Quality=75);	// Calculate gst palette.  This is the main work in gst encoding
bool GSTEncodeMap(GstImageInfo *info, GstType GstType, const GstArgb *clut, int numclutentries, void *gstoutputbuffer, int Quality=75, int SwizzleTexture=1);  // Map image to the gst Palette

} // end of Gst namespace

#endif
