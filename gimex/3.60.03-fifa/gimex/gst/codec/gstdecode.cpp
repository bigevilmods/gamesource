// (c) 2004-2007 Electronic Arts Inc.
// ungst.cpp : Defines the entry point for the console application.

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

bool GSTDecode(void *rawoutdatabuffer, const void *ingstdata, GstType gsttype, int width, int height, int IsSwizzleTexture)
{
	int BlockWidth=4, BlockHeight=2, DetailBpp=2;
	GetTypeInfo(gsttype,&BlockWidth,&BlockHeight,&DetailBpp);

	unsigned char *d = (unsigned char *)rawoutdatabuffer;

	int SizeOfBase = (width/BlockWidth) * (height/BlockHeight);
	int SizeOfDetail = width*height*DetailBpp/8;

	unsigned char *base = (unsigned char *)ingstdata;
	unsigned char *detail = base+((SizeOfBase+(GSTDETAILALIGNMENT-1))&~(GSTDETAILALIGNMENT-1));	

	if (IsSwizzleTexture)
	{
		// unswizzle base texture
		unsigned char *unbase = (unsigned char *) galloc(SizeOfBase);
		memset(unbase,0,SizeOfBase);
		UntwiddleBase(base, unbase, width, height, BlockWidth, BlockHeight);
		base = unbase;

		// unswizzle detail texture
		unsigned char *undetail = (unsigned char *) galloc(SizeOfDetail);
		if (DetailBpp==2)
			UntwizzleDetail2(detail, undetail, width, height);
		else
			UntwizzleDetail1(detail, undetail, width, height);
		detail = undetail;
	}

	unsigned char *pdetail = detail;

	// fast decompress for 422
	if (BlockWidth==4 && DetailBpp==2)
	{
		for (int y=0; y<height; ++y)
		{
			unsigned char *pbase = base+(width/4)*(y/BlockHeight);
			for (int blockx=0; blockx<width; blockx+=4)
			{
				unsigned int detailbits = (unsigned int) *pdetail++;
				unsigned char basevalue = *pbase++;
				d[0] = (unsigned char) (basevalue + ((detailbits>>0) & 3));
				d[1] = (unsigned char) (basevalue + ((detailbits>>2) & 3));
				d[2] = (unsigned char) (basevalue + ((detailbits>>4) & 3));
				d[3] = (unsigned char) (basevalue + ((detailbits>>6) & 3));
				d += 4;
			}
		}
	}
	// fast decompress for 222
	else if (BlockWidth==2 && DetailBpp==2)
	{
		for (int y=0; y<height; ++y)
		{
			unsigned char *pbase = base+(width/2)*(y/BlockHeight);
			for (int blockx=0; blockx<width; blockx+=4)
			{
				unsigned int detailbits = (unsigned int) *pdetail++;
				unsigned char basevalue = *pbase++;
				d[0] = (unsigned char) (basevalue + ((detailbits>>0) & 3));
				d[1] = (unsigned char) (basevalue + ((detailbits>>2) & 3));
				basevalue = *pbase++;
				d[2] = (unsigned char) (basevalue + ((detailbits>>4) & 3));
				d[3] = (unsigned char) (basevalue + ((detailbits>>6) & 3));
				d += 4;
			}
		}
	}
	else
	{
		for (int y=0; y<height; ++y)
		{
			if (DetailBpp==2)
			{
				for (int blockx=0; blockx<width; blockx+=4)
				{
					for (int x=0; x<4; ++x)
					{
						unsigned char *pbase = base+(width/BlockWidth)*(y/BlockHeight)+((blockx+x)/BlockWidth);
						*d++ = (unsigned char) (*pbase + ((*pdetail>>(x*2)) & 3));
					}
					++pdetail;
				}
			}
			else // if (DetailBpp==1)
			{
				for (int blockx=0; blockx<width; blockx+=8)
				{
					for (int x=0; x<8; ++x)
					{
						unsigned char *pbase = base+(width/BlockWidth)*(y/BlockHeight)+((blockx+x)/BlockWidth);
						*d++ = (unsigned char)(*pbase + ((*pdetail>>(x)) & 1));
					}
					++pdetail;
				}
			}
		}
	}
	if (IsSwizzleTexture)
	{
		gfree(base);
		gfree(detail);
	}

	return true;
}

} // end of Gst namespace
