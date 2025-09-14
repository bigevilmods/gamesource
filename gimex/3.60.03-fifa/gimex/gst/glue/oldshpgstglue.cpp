// (c) 1994-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                OLDSHP Gst Gimex Glue Functions
//               ---------------------------------
// This module holds all the shape1 specific functions
// for use in Gimex.
// ------------------------------------------------------------

#include "oldshpgimex.h"
#include "oldshpcmn.h"
#include "gstglue.h"
#include "oldshpgstglue.h"

void OLDgputx(void *memptr, unsigned int val, int numbytes, int type);
unsigned int OLDggetx(const void *memptr, int numbytes, int type);

namespace Gst
{

  #define OLDSHPPUTFIELD(field,data) OLDgputx((void *)&field,(unsigned int)(data),sizeof(field), type)
  #define OLDSHPGETFIELD(field) OLDggetx((void *)&field, sizeof(field), type)

  // ------------------------------------------------------------
  // fill the shape header fields. 
  unsigned int OldBuildShape(OLDSHPHeaderStruct ** image,GstType gstType,GstImageInfo *info, void *gstdata, int sizeofGstData,int swizzleTexture,int type)
  {
	const int BaseBpp           = 8;
	int blockWidth              = 4;
    int blockHeight             = 2;
    int detailBpp               = 2;
	int width                   = info->mWidth;
	int height                  = info->mHeight;
	int swizzleModifier         = swizzleTexture?0x2000:0;
	GetTypeInfo(gstType,&blockWidth,&blockHeight,&detailBpp);

	unsigned int sizeofShape    = sizeof(OLDSHPHeaderStruct) + sizeofGstData;

	*image = (OLDSHPHeaderStruct *) galloc(sizeofShape);
	OLDSHPHeaderStruct * output = *image;
	memset(output,0,sizeofShape);

    OLDSHPPUTFIELD(output->type   , GSTEncodeType(blockWidth,blockHeight,detailBpp));
    OLDSHPPUTFIELD(output->width  , width);
    OLDSHPPUTFIELD(output->height , height);
    OLDSHPPUTFIELD(output->shapex ,(blockWidth|swizzleModifier));
    OLDSHPPUTFIELD(output->shapey , blockHeight);

	// copy base texture
	unsigned char *basedata = (unsigned char *)output+sizeof(OLDSHPHeaderStruct);
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
  unsigned int OLDSHPSetGstShape(OLDSHPHeaderStruct ** image,GINFO * ginfo,const unsigned char * source, int type)
  {
	GstImageInfo info;
	unsigned int sizeofOutput  = 0;
	unsigned int sizeofGstData = 0;
	void * gstdata             = DoGstEncode(ginfo,&info,source,&sizeofGstData,type);
	GstType gstType            = GetGstType(ginfo->packed,type);

	sizeofOutput = OldBuildShape(image,gstType,&info, gstdata, sizeofGstData,ginfo->subtype & SUBSWIZZLE,type);
	gfree(gstdata);

	return sizeofOutput;
  }

  // ------------------------------------------------------------
  void OLDSHPDecompressGstShape(::OLDSHPHeaderStruct * shpin,char * shpout,int type)
  {
    GstType gstCode            = (GstType)shpin->type;
	int width                  = OLDSHPGETFIELD(shpin->width);
	int height                 = OLDSHPGETFIELD(shpin->height);
	int isSwizzled             = (OLDSHPGETFIELD(shpin->shapex)&0x2000)?1:0;

	GSTDecode(shpout,(char *)shpin+sizeof(::OLDSHPHeaderStruct),gstCode,width,height,isSwizzled);
  }

 
  // ------------------------------------------------------------
  bool OLDSHPIsGst(OLDSHPHeaderStruct * AdrShape)
  {
    if(AdrShape->type >= 0x08 && AdrShape->type <= 0x0f)
	  return true;
    return false;
  }

  // ------------------------------------------------------------
  // returns true if the image can be GST compressed
  // the type field is intended to control what shape types
  // can be gst compressed
  bool OLDSHPIsGst(const GINFO * ginfo,int type)
  {
	if(type == TYPEOLDSSH || type == TYPEOLDFSH)
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
  void OLDSHPGstValidate(GINFO * ginfo,int type)
  {
	if(OLDSHPIsGst(ginfo,type))
	{
	  // never compress mipmaps and we don't collapse them
	  if(OLDSHPismipmap((char *)ginfo->framename,0))
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
