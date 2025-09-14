// (c) 2004-2007 Electronic Arts Inc.

#include <stdio.h>
#include <string.h>
#include "twizzle.h"
#include "encode.h"


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
    unsigned int mSwizzled    :1; // csm2 cluts too
    unsigned int transposed   :1;
    unsigned int reserved     :1;
    int          mShapeY      :12;
    unsigned int mMipmaps     :4;
};
    
// ------------------------------------------------------------
int Csm2Index(int pj)
{
	if ((pj&0x18)==0x08 || (pj&0x18)==0x10)
		return pj ^ 0x18;
	return pj;
}

// ------------------------------------------------------------
void CreateBaseTexture(GSTEncoder * encoder,unsigned char * basedata,int SwizzleTexture)
{
    unsigned char * d = basedata;
    int index         = 0;
#ifdef DEBUG
    printf("BaseHeight: %d\n BaseWidth: %d\n",encoder->mBaseHeight,encoder->mBaseWidth);
#endif   
    for(int y = 0; y < encoder->mBaseHeight; y++)
    {
        int sy = y*encoder->mBaseWidth;
        for(int x = 0; x < encoder->mBaseWidth; x++,index++)
        {
            if(SwizzleTexture)
            {
                index = BaseOffset(x,y,encoder->mBaseWidth,encoder->mBlockWidth,encoder->mBlockHeight);
                index >>=3;
            }
            basedata[index] = (encoder->mBlock[sy+x].cluster)<<encoder->mDetailBpp;
        }
    }
}

// ------------------------------------------------------------
void CreateDetailTexture(GSTEncoder * encoder,unsigned char * d ,int SwizzleTexture)
{
    unsigned char * s = encoder->mPaletteIndex;
    int height        = encoder->mHeight;
    int width         = encoder->mWidth;
#ifdef DEBUG
    printf("%s\n",(SwizzleTexture)?"Swizzle Turned On":"Swizzle NOT on");
#endif
    if(SwizzleTexture) // swizzled I have to do a getadr lookup each pixel
    {
        if (encoder->mDetailBpp==2)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; x += 4, s += 4)
                {
                    for (int xfine = 0; xfine < 4; ++xfine)
                    {
                        int offset = Detail2Offset(x+xfine, y, width);
                        Set2Bits(d, offset, s[xfine]&3);
                    }
                }
            }
        }
        else // (DetailBpp==1)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; x += 8,s += 8)
                {
                    for (int xfine = 0; xfine < 8; ++xfine)
                    {
                    
                        int offset = Detail1Offset(x+xfine, y, width);
                        Set1Bit(d, offset, s[xfine]&1);
                    }
                }
            }
        }
    }
    else // not swizzled. Just pack the 1 or 2 bits per pixel into the detail buffer
    {
        if (encoder->mDetailBpp==2)
        {
            for (int i=0; i<width*height; i+=4)
            {
                *d =  (s[0]&3)
                    | ((s[1]&3)<<2)
                    | ((s[2]&3)<<4)
                    | ((s[3]&3)<<6);
                ++d;
                s+=4;
            }
        }
        else // (DetailBpp==1)
        {
            for (int i=0; i<width*height; i+=8)
            {
                *d =  (s[0]&1)
                    | ((s[1]&1)<<1)
                    | ((s[2]&1)<<2)
                    | ((s[3]&1)<<3)
                    | ((s[4]&1)<<4)
                    | ((s[5]&1)<<5)
                    | ((s[6]&1)<<6)
                    | ((s[7]&1)<<7);
                ++d;
                s+=8;
            }
        }
    }
}


// ------------------------------------------------------------
// Choose my iterations and running time based on the fastness
// parameter.
static void SetupRunningSpeed(int & iterations,float & threshold,int Fastness)
{
	if (Fastness>=3)	    // super fast
	{
        iterations = 1;
        threshold  = 4;
	}
	else if (Fastness>=2)	// fast
	{
        threshold  = 2;
        iterations = 1;
	}
	else if (Fastness>=1)	// normal
	{
        iterations = 3;
        threshold  = 2;
	}
	else	                // slow
	{
        iterations = 10;
        threshold  = 0;
	}
}


// ------------------------------------------------------------
unsigned int GST_ShapeType(int BlockWidth=4,int BlockHeight=2,int DetailBpp=2)
{
	unsigned int t=0x8;
	if (BlockWidth==1) t = 0x8;
	else if (BlockWidth==2) t = 0x9;
	else if (BlockWidth==4) t = 0xa;
	else if (BlockWidth==8) t = 0xb;
#ifdef DEBUG
	else printf("invalid block width %d\n",BlockWidth);
#endif

	if (DetailBpp==2 && t<=0xf)
		t+=4;
	return t;
}


// ------------------------------------------------------------
void GST_Save(GSTEncoder * encoder,unsigned char ** retclut, unsigned char ** retimage, int *sizeofPalette, int *sizeofTexture, int SwizzleTexture, int SwizzleClut)
{
	int BlockWidth              = encoder->mBlockWidth;
	int BlockHeight             = encoder->mBlockHeight;
	int DetailBpp               = encoder->mDetailBpp;
	int width                   = encoder->mWidth;
	int height                  = encoder->mHeight;
	int ClutCount               = 256;
	int sizeofBaseTextureData   = (encoder->mBaseWidth*encoder->mBaseHeight+DETAILALIGNMENT)&~DETAILALIGNMENT;
	int sizeofDetailTextureData = width*height*DetailBpp/8;
	int sizeofClutData          = ClutCount*4;
	int sizeofClut              = sizeof(Shape) + sizeofClutData;
	int sizeofShape             = sizeof(Shape) + sizeofBaseTextureData + sizeofDetailTextureData;
	
	Shape * shp                 = (Shape *) galloc(sizeofShape);
	Shape * clut                = (Shape *) galloc(sizeofClut);
	memset(shp,0,sizeofShape);
    memset(clut,0,sizeofClut);
	*sizeofTexture              = sizeofShape;
	*sizeofPalette              = sizeofClut;

	shp->mType                  = GST_ShapeType(BlockWidth,BlockHeight,DetailBpp); 
	shp->mWidth                 = width;
	shp->mHeight                = height;
	shp->mShapeX                = BlockWidth;
	shp->mShapeY                = BlockHeight;
	shp->mSwizzled              = SwizzleTexture;

	clut->mType                 = 0x21; // 32 bpp SSH
	clut->mWidth                = ClutCount;
	clut->mHeight               = 1;
	clut->mShapeX               = 0;
	clut->mShapeY               = 0;
	clut->mSwizzled             = SwizzleClut;
#ifdef DEBUG
    printf("Width: %d Height: %d\n",width,height);
    printf("BlockWidth: %d BlockHeight: %d DetailBpp: %d\n",BlockWidth,BlockHeight,DetailBpp);
#endif
	// copy base texture
	unsigned char *basedata     = (unsigned char *)shp+sizeof(Shape);
    // Create Base Texture
    CreateBaseTexture(encoder,basedata,SwizzleTexture);
    
    // CreateDetailTexture
	// pack detail into 1 or 2 bits
    CreateDetailTexture(encoder,basedata+sizeofBaseTextureData,SwizzleTexture);
    
	// copy clut
	unsigned char *clutdata=(unsigned char *)clut+sizeof(Shape);
	for (int i=0; i<256; ++i)
	{
        int pj = i;
        if (SwizzleClut)
            pj = Csm2Index(pj);
        clutdata[pj*4+0] = encoder->mPalette[i][0];
        clutdata[pj*4+1] = encoder->mPalette[i][1];
        clutdata[pj*4+2] = encoder->mPalette[i][2];
        clutdata[pj*4+3] = encoder->mPalette[i][3];
        //clutdata[pj*4+3] = (encoder->mPalette[i][3]+1)/2; // halfalpha
	}
	*retclut  = (unsigned char *)clut;
	*retimage = (unsigned char *)shp;
}

// ------------------------------------------------------------
// ensure 32 bit and a multiple of 32x16
unsigned char * GST_GetAlignedImage(const unsigned char * image,int width,int height,int rowbytes,int & newwidth,int & newheight,int bpp,ARGB * colourtbl)
{
	char * image32  = 0;                  // working buffer
	char * timage   = (char *)image;      // temporary return buffer
	bool mustclean  = false;              // status of whether something has been done and we must cleanup after ourselves
	// ENSURE 32 bit
	if (bpp <= 8)
	{
#ifdef DEBUG
		printf("Bumping image from 8 to 32 bit\n");
#endif
		image32 = (char *)galloc(width*height*sizeof(ARGB));
		ARGB *d = (ARGB *)image32;
		
		for (int y=0; y<height; ++y)
		{
			unsigned char *s = (unsigned char *)timage + y*rowbytes;
			for (int x=0; x<width; ++x)
			{
				*d++ = colourtbl[*s++];
			}
		}
		rowbytes   = width * sizeof(ARGB);
		timage     = image32;
		mustclean  = true;
	}

	// ENSURE aligned
    int WidthAlign  = 32;
	int HeightAlign = 16;
	newwidth        = (width +(WidthAlign-1))  & ~(WidthAlign-1);
	newheight       = (height+(HeightAlign-1)) & ~(HeightAlign-1);

	if (newwidth!=width || newheight!=height)
	{
	  //#ifdef DEBUG
	  //		printf("WARNING Size misaligned.  Changing from %dx%d to %dx%d\n",width,height,newwidth,newheight);
	  //#endif
		image32       = (char*)galloc(newwidth * newheight * sizeof(ARGB));
		ARGB *d       = (ARGB *) image32;
		ARGB Filler;
		bool HaveFill = false;
		
		
		for (int y = 0; y < newheight; ++y)
		{
			int clampy = y<height?y:(height-1);
			ARGB *s = (ARGB *)((char *)timage + clampy*rowbytes);

			memset(d,0,newwidth*4);

			for (int x = 0; x < newwidth; ++x)
			{
				if (x<width)
					*d++ = *s++;
				else
				{
					if (!HaveFill)
					{
						--s;
						Filler = *s++;
						HaveFill = true;
					}
					*d++ = Filler;
				}
			}
		}
		if(mustclean)
			gfree(timage);
		timage    = image32;
		rowbytes  = width * sizeof(ARGB);
		mustclean = true;
	}
	// if we didn't do anything then copy the buffer anyways to ensure
	// we can cleanup properly later.
	if(!mustclean)
	{
		timage = (char *)galloc(sizeof(ARGB)*width*height);
		memcpy(timage,image,sizeof(ARGB)*width*height);
	}
	return (unsigned char *)timage;
}



// ------------------------------------------------------------
unsigned char *  GSTCompressImage(const unsigned char *image , int rowbytes      , int width            , int height          , 
	                              int *sizeofTexture         , int *sizeofPalette, unsigned char ** clut, int blockwidth=4    ,
					              int blockheight=2          , int detailbpp=2   , int BaseBpp=8        , int clutbpp=32      , 
					              int swizzletexture=1       , int swizzleclut=1 , int fastness=1       , int bpp=32          ,
								  char * colourtbl=0         )
{
    int   iterations  = 3;
    float threshold   = 2.0f;
    float lumaweight  = 0.5f;
    float alphaweight = 0.5f;
    int   newwidth    = width;
    int   newheight   = height;
    ARGB  * pixel     = 0;
    
    SetupRunningSpeed(iterations,threshold,fastness);
#ifdef DEBUG
    printf("Iterations: %d\nThreshold: %f\nLumaWeight: %f\nAlphaWeight: %f\n",iterations,threshold,lumaweight,alphaweight);
#endif
    
	pixel       = (ARGB *)GST_GetAlignedImage(image,width,height,rowbytes,newwidth,newheight,bpp,(ARGB *)colourtbl);
    width       = newwidth;
    height      = newheight;

    GSTEncoder encoder(pixel, width, height, blockwidth, blockheight, detailbpp);
    encoder.EstimateBase(1.0f, lumaweight, alphaweight, 2.0f);
    for (int i = 0; i < iterations; ++i)
    {
        encoder.ComputePalette();
        float meanBlockError = encoder.ComputeDetail();
        encoder.ImproveMapping(threshold*meanBlockError);
    }

    unsigned char * output;
    GST_Save(&encoder,clut,&output,sizeofPalette,sizeofTexture,swizzletexture,swizzleclut);

    if(pixel)
        gfree(pixel);
    return output;
}
 
}
