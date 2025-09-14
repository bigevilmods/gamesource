// (c) 2004-2007 Electronic Arts Inc.
// ungst.cpp : Defines the entry point for the console application.

// set to 0 to turn off un-halfalpha.  Should match the gst setting
#define HALFALPHA 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "twizzle.h"
#include "gimex.h"

namespace Gst
{
    
static const int DETAILALIGNMENT=15;


/* little endian SHAPE */
struct Shape
{
    unsigned int mType :8;
    signed int   mNextOffset :24;
    short        mWidth;
    short        mHeight;
    short        centerx;
    short        centery;
    int          mShapeX      :12;
    unsigned int referenced   :1;
    unsigned int swizzled     :1;
    unsigned int transposed   :1;
    unsigned int reserved     :1;
    int          mShapeY      :12;
    unsigned int mMipmaps     :4;
};


void GetTypeInfo(unsigned int shptype, int &BlockWidth, int &BlockHeight, int &DetailBpp)
{
	// new shape types 8..f    0000_01dww where d=detail, w=width.
	if (shptype>=0x8 && shptype<=0xf)
	{
		BlockWidth  = 1<<(shptype&3);
		BlockHeight = 2;
		DetailBpp   = (shptype&4)?2:1;
	}
	// old shape types 0x80..0x89.
	else if (shptype>=0x80)
	{
		BlockWidth = ((shptype&~1)==0x80 || (shptype&~1)==0x88)?2:(((shptype&~1)==0x84)?8:4);
		BlockHeight = ((shptype&~1)==0x86)?4:(((shptype&~1)==0x88)?1:2);
		DetailBpp = (shptype&1)?2:1;
	}
}



unsigned char * GSTDecompressImage(unsigned char * inshape)
{
    Shape *shpin    = (Shape *)inshape;
	int BlockWidth  = 4;
    int BlockHeight = 2;
    int DetailBpp   = 2;
    
	GetTypeInfo(shpin->mType,BlockWidth,BlockHeight,DetailBpp);

    int sizeTexture       = sizeof(Shape)+shpin->mWidth*shpin->mHeight;
	Shape *shpout         = (Shape *)galloc(sizeTexture);
	memcpy(shpout,shpin,sizeof(Shape));                                  // copy shape header
	shpout->mType         = 2;                                           // type2=8 bpp SSH
    
	unsigned char *d      = (unsigned char *)shpout+sizeof(Shape);
    
	int SizeOfBase        = (shpin->mWidth/BlockWidth) * (shpin->mHeight/BlockHeight);
	int SizeOfDetail      = shpin->mWidth*shpin->mHeight*DetailBpp/8;

	unsigned char *base   = (unsigned char *)shpin+sizeof(Shape);
	unsigned char *detail = base+((SizeOfBase+DETAILALIGNMENT)&~DETAILALIGNMENT);
	
	if (shpin->swizzled)
	{
		// unswizzle base texture
		unsigned char *unbase = (unsigned char *) galloc(SizeOfBase);
		memset(unbase,0,SizeOfBase);
		UntwiddleBase(base, unbase, shpin->mWidth, shpin->mHeight, BlockWidth, BlockHeight);
		base = unbase;

		// unswizzle detail texture
		unsigned char *undetail = (unsigned char *) galloc(SizeOfDetail);
		if (DetailBpp==2)
			UntwizzleDetail2(detail, undetail, shpin->mWidth, shpin->mHeight);
		else
			UntwizzleDetail1(detail, undetail, shpin->mWidth, shpin->mHeight);
		detail = undetail;
	}
    
    // Decompress the image take base and add the detail bits to modify where the base points to
	unsigned char *pdetail = detail;
	for (int y=0; y<shpin->mHeight; ++y)
	{
		if (DetailBpp==2)
		{
			for (int blockx=0; blockx<shpin->mWidth; blockx+=4)
			{
				for (int x=0; x<4; ++x)
				{
					unsigned char *pbase = base+(shpin->mWidth/BlockWidth)*(y/BlockHeight)+((blockx+x)/BlockWidth);
					*d++ = *pbase + ((*pdetail>>(x*2)) & 3);
				}
				++pdetail;
			}
		}
		else // if (DetailBpp==1)
		{
			for (int blockx=0; blockx<shpin->mWidth; blockx+=8)
			{
				for (int x=0; x<8; ++x)
				{
					unsigned char *pbase = base+(shpin->mWidth/BlockWidth)*(y/BlockHeight)+((blockx+x)/BlockWidth);
					*d++ = *pbase + ((*pdetail>>(x)) & 1);
				}
				++pdetail;
			}
		}
	}
    
	if (shpin->swizzled)
	{
		gfree(base);
		gfree(detail);
		shpout->swizzled = 0;
	}

	return (unsigned char *)shpout;
}

}
