// (c) 1994-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                NEWSHP Gst Gimex Glue Functions
//               ---------------------------------
// This module holds all the shape2 specific functions
// for use in Gimex.
// ------------------------------------------------------------

#include "shpgimex.h"
#include "shpcmn.h"
#include "newshpgstglue.h"
#include "gstglue.h"

void gputx(void *memptr, unsigned int val, int numbytes, int type);
unsigned int ggetx(const void *memptr, int numbytes, int type);

namespace Gst
{

  #define SHPPUTFIELD(field,data) gputx((void *)&field,(unsigned int)(data),sizeof(field), type)
  #define SHPGETFIELD(field) ggetx((void *)&field, sizeof(field), type)

  // ------------------------------------------------------------
  // fill the shape header fields. 
  unsigned int NewBuildShape(SHPHeaderStruct ** image,GstType gstType,GstImageInfo *info, void *gstdata, int sizeofGstData,int swizzleTexture,int type)
  {
	const int BaseBpp           = 8;
	int blockWidth              = 4;
    int blockHeight             = 2;
    int detailBpp               = 2;
	int width                   = info->mWidth;
	int height                  = info->mHeight;
	int swizzleModifier         = swizzleTexture?TEXTURE_FLAG_SWIZZLED:0;
	
	GetTypeInfo(gstType,&blockWidth,&blockHeight,&detailBpp);

	unsigned int sizeofShape    = sizeof(SHPHeaderStruct) + sizeofGstData;

	*image = (SHPHeaderStruct *) galloc(sizeofShape);
	SHPHeaderStruct * output    = *image;
	memset(output,0,sizeofShape);

    SHPPUTFIELD(output->type       , GSTEncodeType(blockWidth,blockHeight,detailBpp));
    SHPPUTFIELD(output->flags      , ELEMENT_FLAG_NEW_FORMAT|swizzleModifier);
    SHPPUTFIELD(output->width      , width);
    SHPPUTFIELD(output->height     , height);
    SHPPUTFIELD(output->shapex     , blockWidth);
    SHPPUTFIELD(output->shapey     , blockHeight);
	SHPPUTFIELD(output->DataOffset , sizeof(SHPHeaderStruct));
	SHPPUTFIELD(output->DataSize   , sizeofGstData);

	// copy base texture
	unsigned char *basedata = (unsigned char *)output+sizeof(SHPHeaderStruct);
	memcpy(basedata,gstdata,sizeofGstData);

	return sizeofShape;
  }

  // ------------------------------------------------------------
  // NB blockwidth and detail Bpp are stuffed in the fourCC
  // GST#
  // where # is a char that is built like so:
  // 0x0011 0bww
  // (b)  = detail bpp   0 = 1 bpp 1 = 2 bpp
  // (ww) = block width  0 = 1 pix
  //                     1 = 2 pix
  //                     2 = 4 pix
  //                     3 = 8 pix
  // to yield FourCC's 'GST0' .. 'GST7'
  unsigned int SHPSetGstShape(SHPHeaderStruct ** image,GINFO * ginfo,const unsigned char * source, int type)
  {
	GstImageInfo info;
	unsigned int sizeofOutput  = 0;
	unsigned int sizeofGstData = 0;
	void * gstdata             = DoGstEncode(ginfo,&info,source,&sizeofGstData,type);
	GstType gstType            = GetGstType(ginfo->packed,type);

	sizeofOutput = NewBuildShape(image,gstType,&info, gstdata, sizeofGstData,ginfo->subtype & SUBSWIZZLE,type);
	gfree(gstdata);

	return sizeofOutput;
  }

  // ------------------------------------------------------------
  void SHPDecompressGstShape(SHPHeaderStruct * shpin,char * shpout,int type)
  {
    GstType gstCode            = (GstType)shpin->type;
	int width                  = SHPGETFIELD(shpin->width);
	int height                 = SHPGETFIELD(shpin->height);
	int isSwizzled             = (SHPGETFIELD(shpin->flags)&TEXTURE_FLAG_SWIZZLED)?1:0;

	GSTDecode(shpout,(char *)shpin+sizeof(SHPHeaderStruct),gstCode,width,height,isSwizzled);
  }

 
  // ------------------------------------------------------------
  bool SHPIsGst(SHPHeaderStruct * AdrShape)
  {
    if(AdrShape->type >= 0x08 && AdrShape->type <= 0x0f)
	  return true;
    return false;
  }

  // ------------------------------------------------------------
  // returns true if the image can be GST compressed
  // the type field is intended to control what shape types
  // can be gst compressed
  bool SHPIsGst(const GINFO * ginfo,int type)
  {
	if(type == TYPESSH || type == TYPEFSH)
	{
		if(
		   ((unsigned char)(ginfo->packed >> 24)) == 'G' &&
		   ((unsigned char)(ginfo->packed >> 16)) == 'S' &&
		   ((unsigned char)(ginfo->packed >> 8))  == 'T'
		   )
		  return true;
	}
	return false;
  }

  // ------------------------------------------------------------
  // Handle exceptional cases for gst textures. (ie to small
  // or mipmaps.
  void SHPGstValidate(GINFO * ginfo,int type)
  {
	if(SHPIsGst(ginfo,type))
	{
	  // never compress mipmaps and we don't collapse them
	  if(SHPismipmap((char *)ginfo->framename,0))
	  {
		ginfo->subtype |= SUBNOMIPMAPCOLLAPSE;
		ginfo->packed  = 0;
	  }
	  // image MUST be a multiple of 32x16 at the minimum or
	  // else I will not compress it at all
	  if( ((ginfo->width  + 31) & ~31) != ginfo->width ||
	      ((ginfo->height + 15) & ~15) != ginfo->height )
	    ginfo->packed = 0;
	}
  }

}
