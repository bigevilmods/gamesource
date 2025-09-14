// (c) 2004-2007 Electronic Arts Inc.
// ungst.cpp : Defines the entry point for the console application.

// set to 0 to turn off un-GSTHALFALPHA.  Should match the gst setting

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gst.h"
#include "twizzle.h"
#include "gimex.h"	// for galloc/gfree

namespace Gst
{

////////////////////////////////////////////////////////
// PUBLIC API
////////////////////////////////////////////////////////

// return the size that the compressed gst texture data will be
int GSTEncodeSize(GstType GstType, int ImageWidth, int ImageHeight)
{
	const int BaseBpp = 8;
	int BlockWidth=4, BlockHeight=2, DetailBpp=2;
	GetTypeInfo(GstType,&BlockWidth,&BlockHeight,&DetailBpp);

	int sizeofBaseTextureData   = (ImageWidth*ImageHeight/(BlockWidth*BlockHeight)*BaseBpp/8+(GSTDETAILALIGNMENT-1))&~(GSTDETAILALIGNMENT-1);
	int sizeofDetailTextureData = ImageWidth*ImageHeight*DetailBpp/8;
	int sizeofTexture = sizeofBaseTextureData + sizeofDetailTextureData;

	return sizeofTexture;
}

void GetTypeInfo(GstType GstType, int *BlockWidth, int *BlockHeight, int *DetailBpp)
{
	// types 8..f    0000_01dww where d=detail, w=width
	if (GstType>=GST1 && GstType<=GST8)
	{
		*BlockWidth  = 1<<((int)GstType&3);
		*BlockHeight = 2;
		*DetailBpp   = ((int)GstType&4)?2:1;
	}
}

// determine GstType for given block size
GstType GSTEncodeType(int BlockWidth,int BlockHeight,int DetailBpp)
{
	unsigned int t=0x8;

	if (BlockWidth==1) t = 0x8;
	else if (BlockWidth==2) t = 0x9;
	else if (BlockWidth==4) t = 0xa;
	else if (BlockWidth==8) t = 0xb;
	else t = 0;

	if (DetailBpp==2 && t>=0x8 && t<=0xf)
		t+=4;

	return (GstType) t;
}

} // end of Gst namespace
