// (c) 1994-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                Common Gst Gimex Glue Functions
//               ---------------------------------
// This module holds all the common functions for use by
// both the old and new shape gst glue functions.
// ------------------------------------------------------------


#include "gstglue.h"

namespace Gst
{
  // ------------------------------------------------------------
  // Create the FourCC packed code from the shape type.
  // FourCC is GST# where # ranges from 0..7 with bits
  // representing:
  // 0011 0bww
  // (b)  = detail bpp  0 = 1 bpp 1 = 2 bpp
  // (ww) = block width 0 = 1 pixels
  //                    1 = 2 pixels
  //                    2 = 4 pixels
  //                    3 = 8 pixels
  unsigned int GetGstFourCC(unsigned char stype)
  {
	unsigned int packed = 0x47535400;
	unsigned char mask  = '0';
	if(stype > 0x0b)     // if detail bpp == 2
	{
		mask |= 0x4;     // set detail bpp = 2
		stype -= 4;      // set range 0x08..0x0b
	}
	stype  -= 0x08;      // set range 0..3
	mask   |= stype;     // put block width into FourCC mask
	packed |= mask;      // put number into FourCC
	return packed;       // return the FourCC
  }
  // ------------------------------------------------------------
  // NOTE: type is not specified for now since I don't need it
  // at the moment
  GstType GetGstType(unsigned int packed,int )
  {
	unsigned int type    = (packed&0xFF) - '0';
	type += 0x08;
	return (GstType)type;
  }

  // ------------------------------------------------------------
  // returns true if the ginfo pack code referes to a GST type
  bool IsGstFourCC(unsigned int packed)
  {
	if((packed&0xFFFFFF00) == 0x47535400) // if the fourCC == 'GST '
	  return true;
	return false;
  }

  // ------------------------------------------------------------
  // do the gst encode operation, shape type independant
  // returns a buffer with the compressed data and the ginfo
  // has been modified to have the new clut.
  void * DoGstEncode(GINFO * ginfo,GstImageInfo * info,const unsigned char * insource,unsigned int * sizeofGstData,int type)
  {

	unsigned char * source     = const_cast<unsigned char *>(insource);
	bool            didconvert = false;
	// temporary bump up to 32 bit till
	// frank fixes the gst compression lib to
	// handle 8bit.
	if(ginfo->bpp == 8)
	{
	  didconvert              = true;
	  source                  = (unsigned char *)galloc(ginfo->width*ginfo->height*4);
	  unsigned int  * d       = (unsigned int  *)source;
	  const unsigned char * s = insource;
	  int             l       = ginfo->width*ginfo->height;

	  for(int i = 0; i < l; i++)
	  {
		*d = *((unsigned int *)(&(ginfo->colourtbl[*s])));
		d++;
		s++;
	  }
	}

	info->mWidth    = ginfo->width;
	info->mHeight   = ginfo->height;
	info->mBpp      = 32;
	info->mClut     = NULL;
	info->mImage    = source;
	info->mRowBytes = 4*ginfo->width;  // these 3 params are temporarily fixed to 32 bit see conversion above
	GstType gstType = (GstType)GetGstType(ginfo->packed,type);
	int quality     = ginfo->quality;
	int swizzle     = ginfo->subtype&SUBSWIZZLE?1:0;
	
	// Encode a GST palette
	// this is the main work of gst... will take awhile to compute

	GstArgb GstPalette[256];
	int numclutentries = 0;
	// Calculate gst palette.  This is the main work in gst encoding
	GSTEncodePalette(info, gstType,&GstPalette[0], &numclutentries, quality);

	// allocate target buffer
	// return the size that the compressed gst texture data will be
	*sizeofGstData    = GSTEncodeSize(gstType, ginfo->width, ginfo->height);
	void *gstdata     = galloc(*sizeofGstData);

	// Map image to the gst Palette
	GSTEncodeMap(info, gstType,&GstPalette[0], numclutentries, gstdata, quality,ginfo->subtype&SUBSWIZZLE?1:0);
	  
	// copy the clut data back to the ginfo so I can set the clut
	// data in the regular shape code.
	memcpy(ginfo->colourtbl,GstPalette,256*sizeof(ARGB));
	ginfo->numcolours = numclutentries;
    ginfo->bpp        = 8;

	if(didconvert)
	  gfree(source);
	return gstdata;
  }

}
