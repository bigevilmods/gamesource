// (c) 2004-2007 Electronic Arts Inc.

// gstmap.cpp - functions to map an image (8 or 32 bpp) to a gst encoded palette

#define GSTDOALPHA 1
#define GSTNUMCHANNELS 4
//static const int GSTNUMCHANNELS=4;

#include <string.h> // for memcpy
#include "gst.h"
#include "twizzle.h"
#include "gimex.h"  // for galloc/gfree

namespace Gst
{

extern unsigned int GSTsquaretbl[512];

// pack detailmap into bits and twizzle base/detail to target buffer

static void GstPackBits(unsigned char *gstoutputbuffer, const unsigned char *BaseMap, const unsigned char *DetailMap, 
				 int Width, int Height, GstType GstType, int SwizzleTexture)
{

	int BlockWidth=4, BlockHeight=2, DetailBpp=2;
	GetTypeInfo(GstType,&BlockWidth,&BlockHeight,&DetailBpp);

	// base texture goes here
	unsigned char *BaseData =  gstoutputbuffer;

	// copy base texture

	int SizeOfBase = Width*Height/(BlockWidth*BlockHeight);
	if (SwizzleTexture)
		TwiddleBase(BaseMap,BaseData,Width,Height,BlockWidth,BlockHeight);
	else
		memcpy(BaseData,BaseMap,SizeOfBase);

	// detail output goes here
	unsigned char *DetailData = gstoutputbuffer+((SizeOfBase+(GSTDETAILALIGNMENT-1))&~(GSTDETAILALIGNMENT-1));

	// pack detail into 1 or 2 bits
	int SizeOfDetail = Width*Height*DetailBpp/8;
	unsigned char *PackedDetail = (unsigned char *)galloc(SizeOfDetail);
	const unsigned char *s=DetailMap;
	unsigned char *d=PackedDetail;

	if (DetailBpp==2)
	{
		for (int i=0; i<Width*Height; i+=4)
		{
			*d = s[0]
 		      | (s[1]<<2)
 		      | (s[2]<<4)
 		      | (s[3]<<6);
			++d;
			s+=4;
		}
	}
	else // (DetailBpp==1)
	{
		for (int i=0; i<Width*Height; i+=8)
		{
			*d = s[0]
 		      | (s[1]<<1)
 		      | (s[2]<<2)
 		      | (s[3]<<3)
 		      | (s[4]<<4)
 		      | (s[5]<<5)
 		      | (s[6]<<6)
 		      | (s[7]<<7);
			++d;
			s+=8;
		}
	}

	// copy base texture

	if (SwizzleTexture)
	{
		if (DetailBpp==2)
			TwizzleDetail2(PackedDetail,DetailData,Width,Height);
		else
			TwizzleDetail1(PackedDetail,DetailData,Width,Height);
	}
	else
		memcpy(DetailData,PackedDetail,SizeOfDetail);
	
	gfree(PackedDetail);
}


// *************************************
// Final Mapper
// *************************************

int FinalComparePixelToCenter(const GstArgb *cluster, const GstArgb *p, int BlockColours, unsigned char *DetailMap)
{
	int MinDiff=0x7fffffff;
	for (int i=0; i<BlockColours; ++i)
	{
#if GSTDOALPHA
		int Diff = GSTsquaretbl[(int)cluster[i].r - (int)p->r + 256]
			     + GSTsquaretbl[(int)cluster[i].g - (int)p->g + 256]
				 + GSTsquaretbl[(int)cluster[i].b - (int)p->b + 256]
				 + GSTsquaretbl[(int)cluster[i].a - (int)p->a + 256];
#else
		int Diff = GSTsquaretbl[(int)cluster[i].r - (int)p->r + 256]
			     + GSTsquaretbl[(int)cluster[i].g - (int)p->g + 256]
				 + GSTsquaretbl[(int)cluster[i].b - (int)p->b + 256];
#endif
		if (Diff<=MinDiff)
		{
			MinDiff = Diff;
			*DetailMap = (unsigned char) i;
		}
	}
	return MinDiff;
}


// compare a block of screen pixels to a block in the block list

int FinalCompareBlock(GstImageInfo *info, const GstArgb *cluster, unsigned char *DetailMap, int BlockX, int BlockY, int BlockWidth, int BlockHeight, int BlockColours)
{
	int DiffSum=0;
	
	unsigned char *DMap = DetailMap + (info->mWidth*(BlockY)) + BlockX;

	if (info->mBpp<=8)
	{
		const unsigned char *s = (unsigned char *) info->mImage + info->mRowBytes*(BlockY) + BlockX;

		for (int y=0; y<BlockHeight; ++y)
		{
			for (int x=0; x<BlockWidth; ++x)
			{
				int Diff = FinalComparePixelToCenter(cluster,info->mClut+s[x],BlockColours,DMap+x);
				DiffSum += Diff;
			}
			s += info->mRowBytes;
			DMap += info->mWidth;
		}
	}
	else
	{
		const GstArgb *s = (const GstArgb *)((unsigned char *)info->mImage + info->mRowBytes*(BlockY) + BlockX*GSTNUMCHANNELS);

		for (int y=0; y<BlockHeight; ++y)
		{
			for (int x=0; x<BlockWidth; ++x)
			{
				int Diff = FinalComparePixelToCenter(cluster,s+x,BlockColours,DMap+x);
				DiffSum += Diff;
			}
			s = (GstArgb *)((unsigned char *)s + info->mRowBytes);
			DMap += info->mWidth;
		}
	}
	return DiffSum;
}

// for the given screen block of pixels, find the best cluster to match to

unsigned char FinalMapBlock(GstImageInfo *info, GstType GstType, const GstArgb *clut, int numclutentries, int Quality, 
	unsigned char *DetailMap,
	int BlockX, int BlockY)
{
	int BestDiff=0x7fffffff;
	int BestIndex=0;

	int BlockWidth=4, BlockHeight=2, DetailBpp=2;
	GetTypeInfo(GstType,&BlockWidth,&BlockHeight,&DetailBpp);

	int BlockColours = 1<<DetailBpp;	// 2 or 4
	bool AddEnable = !((BlockX&8)&&(BlockY&&2));  // can do detail add except in bottom right 8x2 of 16x4
	int StepRate = (AddEnable && (Quality>=75))?1:BlockColours;	// disable for lower quality for performance reasons

	// step thru clut... for now, by BlockColours... later do overlap here
	for (int i=0; i<numclutentries; i+=StepRate)
	{
		int Diff = FinalCompareBlock(info,clut+i,DetailMap,BlockX,BlockY,BlockWidth,BlockHeight,BlockColours);
		if (Diff < BestDiff)
		{
			BestDiff = Diff;
			BestIndex = i;
		}
	}
	// once more for detail map
	FinalCompareBlock(info,clut+BestIndex,DetailMap,BlockX,BlockY,BlockWidth,BlockHeight,BlockColours);

	return (unsigned char) BestIndex;
}

////////////////////////////////////////////////////////
// PUBLIC API
////////////////////////////////////////////////////////

// Map image to the gst clut
// Quality is for speed of mapping.  May use overlap or GSTPREATTENuation to improve quality

bool GSTEncodeMap(GstImageInfo *info, GstType GstType, const GstArgb *GstPalette, int numclutentries, void *gstoutputbuffer, int Quality, int SwizzleTexture)
{
	// initialize L2 difference table

     for (int i=0; i<512; ++i)
         GSTsquaretbl[i] = (i-256)*(i-256);


	int Width = info->mWidth;
	int Height = info->mHeight;

	int BlockWidth=4, BlockHeight=2, DetailBpp=2;
	GetTypeInfo(GstType,&BlockWidth,&BlockHeight,&DetailBpp);

	unsigned char *BaseMap =   (unsigned char *) galloc(Width*Height/(BlockWidth*BlockHeight));
	unsigned char *DetailMap = (unsigned char *) galloc(Width*Height);

	// for each screen block, map to the best block in the list
	// return psnr as float for GSTDEBUG

	unsigned char *imap = BaseMap;
	// find best match and update IndexMap
	for (int y=0; y<Height; y+= BlockHeight)
		for (int x=0; x<info->mWidth; x+= BlockWidth)
			*imap++ = (unsigned char) FinalMapBlock(info, GstType, GstPalette, numclutentries, Quality, 
				DetailMap, 
				x,y);

	GstPackBits((unsigned char *)gstoutputbuffer, BaseMap, DetailMap, Width, Height, GstType, SwizzleTexture);
	gfree(BaseMap);
	gfree(DetailMap);
	return true;
}

} // end of Gst namespace

