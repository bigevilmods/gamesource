// (c) 1994-2007 Electronic Arts Inc.

#define VENOM 1

#include <string.h>

#include "gimex.h"
#include "shpgimex.h"
#ifdef DEBUG
#include <stdio.h>
#endif

#include "gst.h"
#include "gstglue.h"
#include "newshpgstglue.h"
using Gst::SHPIsGst;
using Gst::SHPSetGstShape;
using Gst::SHPGstValidate;


#define SWIZZLE4BUG 0

void gputx(void *memptr, unsigned int val, int numbytes, int type);
unsigned int ggetx(const void *memptr, int numbytes, int type);
#define SHPGETFIELD(field) ggetx((void *)&field, sizeof(field), type)
#define SHPPUTFIELD(field,data) gputx((void *)&field,(unsigned int)(data),sizeof(field), type)

#define SHPFILEGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define SHPFILEPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#define SHPMIPMAPALIGN(x) (((x)+shpmipmapdataalignment)&~shpmipmapdataalignment)


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

#ifndef __SHPWRITE
#define __SHPWRITE 1
#include "codex.h"
#include "refcodex.h"
#include "dxtcodex.h"
#include "s3tccodex.h"
#include "idec_encode.h"
#include "idec_decode.h"
#include "shpcmn.h"

#include "shpwrite.h"

#if defined(_MSC_VER) && !defined(_XBOX)
#  define __MSWINDOWS__
#endif


#ifdef __MSWINDOWS__
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <vfw.h>
#include "avivfw.h"
#endif


static void SHPWOrderCubemaps(ShapeListPtr theShape,int type, int shpalignment, int shpmipmapdataalignment);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

bool MVCODEC_exists(const GINFO* ginfo)
{
#ifdef __MSWINDOWS__

	VfwCodecInfo info;
	VFWHANDLE *hCompress;
	bool correctCompressor;

	memset(&info,0,sizeof(VfwCodecInfo));

	info.width	= ginfo->width;
	info.height = ginfo->height;
	info.srcBpp = ginfo->originalbpp;
	info.fourCC = ggetm(&ginfo->packed,4);
	info.nColours = 0;
	info.destBpp = 0;
	info.flags = 0;
	info.quality = ginfo->quality*100;
	info.palette = NULL;

	/* Always call end if you call begin*/
	hCompress = VfwBeginCompress(&info,&correctCompressor);
	VfwEndCompress(hCompress, &info);

    if (hCompress && correctCompressor)
	//if (hCompress)
	{
		return true;
	}
	else
	{
		return false;
	}

#else

	return 0;
#endif
}

#ifdef __MSWINDOWS__
int MVCODEC_encode(const GINFO* ginfo, int rowbytes, const unsigned char *source, SHPHeaderStruct *dstShape)
{
	VfwCodecInfo info;
	VFWHANDLE *hCompress;
	bool correctCompressor;
	int shptype;

	unsigned char *dst = (unsigned char *) dstShape + sizeof(SHPHeaderStruct) + 8;	// +8 for compression header

	memset(&info,0,sizeof(VfwCodecInfo));

	info.width	   = ginfo->width;
	info.height    = ginfo->height;
	info.srcBpp    = ginfo->originalbpp;
	info.fourCC    = ggetm(&ginfo->packed,4);
	info.nColours  = 0;
	info.destBpp   = 0;
	info.flags     = 0;
	info.quality   = ginfo->quality?ginfo->quality*100:10000;
	info.framesize = ginfo->framesize?ginfo->framesize:10000;
	info.palette   = NULL;

	/* Always call End if call begin*/
	hCompress = VfwBeginCompress(&info,&correctCompressor);
    double tfps = ginfo->fps?ginfo->fps:25.0;

	VfwSetDataRate(hCompress,info.quality,info.framesize,(float)((1.0/tfps)*1000000.0));
    // If the user requested it the try to pop up the configure box for this
	// codec.
    if(ginfo->subtype&SUBCONFIGURE)
	  VfwConfigurePopup(hCompress);

	if (!hCompress)
		return 0;
    if (!correctCompressor)
		return 0;


	/* Should fail if src isn't 32 bit*/
	int sizeImage = VfwCompress(dst, source, hCompress, &info, rowbytes, 1, 0, 0);
	if (!sizeImage)
		return 0;
/*
	switch(info.destBpp)
	{

		case 16:
			shptype = 0x7e;
			break;
		case 24:
			shptype = 0x7d;
			break;
		case 32:
			shptype = 0x7d;
			break;

	}
*/
	shptype = 0x7d;	/* say the output is 32 bpp always */

	sizeImage += 8; // for compression header


	//SHPPUTFIELD(dstShape->type,shptype);
	dstShape->type = ggeti(&shptype,1);
    //	if (ggetm(&info.fourCC,4) != (unsigned int)ginfo->packed)
    *((int *)((char*) dstShape + sizeof(SHPHeaderStruct))) = info.fourCC;
	//gputm((char*) dstShape + sizeof(SHPHeaderStruct), ggeti(info.fourCC,4), 4);
	gputi((char*) dstShape + sizeof(SHPHeaderStruct) + 4, info.destBpp, 4);

	VfwEndCompress(hCompress, &info);

	return sizeImage;

}
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static void SHPWSetNext(char **prevptr, int *prevlen, char *newptr, int newlen, int type)
{
    if (newptr)
    {
        if (*prevptr)
        {
            int nextoffset = *prevlen;
            SHPPUTFIELD((((SHPHeaderStruct*)*prevptr)->mNext),nextoffset);
        }

        SHPPUTFIELD((((SHPHeaderStruct*)newptr)->mNext),0);
        *prevptr = newptr;
        *prevlen = newlen;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the rowbyes of the shape taking into account the alignment issues.
static int SHPWCalcRowBytes(int width, int bpp, int type)
{
    int SizeShape=0L;

    if (bpp == 15)
        bpp = 16;

    switch (type)
    {
        case TYPEPSH:
        case TYPEASH:
        case TYPESSH: SizeShape=((width*bpp+15)&~15)/8; break;
        case TYPEMSH: SizeShape=((width*bpp+127)&~127)/8; break; // PSP shape rows must be 16-byte aligned.
        default:      SizeShape=((width*bpp+7)&~7)/8; break;
    }

    return(SizeShape);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// note this has been a constant source of aggravation with alignment watch out
// when playing with shape alignment.

static int SHPWCalcSizeShape(int width,int height,int bpp, int type, int dxt)
{
    int    SizeShape=0L;

    if (dxt)
    {
        width = (width+3)&~3;
        height = (height+3)&~3;
    }
    SizeShape = SHPWCalcRowBytes(width, bpp, type);
    SizeShape *= height;

    return(SizeShape);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// unltimate source of information about shapetypes. Looks up based on
// shapetype the bpp of the Shape.
static int SHPWGetShpbpp(SHPHeaderStruct *theShape,int type)
{
    int     bpp=0;

    if (type==TYPEVSH)
    {
        switch (theShape->type)
        {
            case 0x22: bpp=24; break;
            case 0x24: bpp=24; break;
            default:   bpp=8;  break;
        }
    }
    else
    {
        switch (theShape->type&0x7f)
        {
            case 0x23: bpp=16; break;   /* PSH types */
            case 0x40: bpp=4;  break;
            case 0x41: bpp=8;  break;
            case 0x42: bpp=16; break;
            case 0x43: bpp=24; break;
            case 0x44: bpp=1;  break;

            case 0x30: bpp=16; break;   /* GSH types */
            case 0x31: bpp=16; break;
            case 0x32: bpp=15; break;
            case 0x10: bpp=4;  break;
            case 0x11: bpp=8;  break;
            case 0x12: bpp=8;  break;
            case 0x13: bpp=16; break;
            case 0x14: bpp=16; break;
            case 0x15: bpp=15; break;
            case 0x16: bpp=32; break;
            case 0x18: bpp=4;  break;
            case 0x19: bpp=8;  break;
            case 0x1a: bpp=16; break;
            case 0x1e: bpp=4;  break;  /* CMPR */
            case 0x1c: bpp=12; break;  /* YUV */    // 282 YUV (YUM) format.

            case 0x38: bpp=16; break;   /* MSH types */
            case 0x39: bpp=15; break;
            case 0x3a: bpp=16; break;
            case 0x3b: bpp=32; break;
            case 0x58: bpp=16; break;
            case 0x59: bpp=15; break;
            case 0x5a: bpp=16; break;
            case 0x5b: bpp=32; break;
            case 0x5c: bpp=4;  break;
            case 0x5d: bpp=8;  break;

            case 0x20: bpp=15; break;   /* SSH types */
            case 0x21: bpp=32; break;
            case 0x01: bpp=4;  break;
            case 0x02: bpp=8;  break;
            case 0x03: bpp=15; break;
            case 0x04: bpp=24; break;
            case 0x05: bpp=32; break;
            case 0x08: bpp=8;  break;   // GST 1x2x1
            case 0x09: bpp=8;  break;   // GST 2x1x1
            case 0x0a: bpp=8;  break;   // GST 4x2x1
            case 0x0b: bpp=8;  break;   // GST 8x2x1
            case 0x0c: bpp=8;  break;   // GST 1x2x2
            case 0x0d: bpp=8;  break;   // GST 2x2x2
            case 0x0e: bpp=8;  break;   // GST 4x2x2
            case 0x0f: bpp=8;  break;   // GST 8x2x2

            case 0x22: bpp=24; break;   /* FSH XSH VSH types */
            case 0x24: bpp=24; break;
            case 0x29: bpp=16; break;
            case 0x2a: bpp=32; break;
            case 0x2c: bpp=32; break;
            case 0x2d: bpp=16; break;
            case 0x2e: bpp=32; break;
            case 0x2f: bpp=24; break;
            case 0x60: bpp=4;  break;   /* DXT1 */
            case 0x61: bpp=8;  break;   /* DXT3 */
            case 0x62: bpp=8;  break;   /* DXT5 */
            case 0x63: bpp=8;  break;   /* L8   */
            case 0x64: bpp=8;  break;   /* A8   */
            case 0x68: bpp=16; break;   /* YUY2 */
            case 0x6c: bpp=12; break;   /* YUV  */  // 282 YUV (YUI) format.
            case 0x6d: bpp=16; break;
            case 0x79: bpp=4;  break;
            case 0x7a: bpp=4;  break;
            case 0x7b: bpp=8;  break;
            case 0x7d: bpp=32; break;
            case 0x78: bpp=16; break;
            case 0x67: bpp=16; break;   /* 655 */
            case 0x7e: bpp=15; break;
            case 0x7f: bpp=24; break;
            case 0x65: bpp=16; break;   /* AL_88 */
            case 0x66: bpp=24; break;   /* 6666 */
            case 0x6a: bpp=32; break;   /* 2101010 */
//            case 0x15: bpp=15; break;	/* 5a3 advanced */

                                        /* NSH */
            case 0x27: bpp=16; break;   /* RGBA clut */
            case 0x28: bpp=16; break;   /* IA clut */
//            case 0x60: bpp=1;  break;   /* CI */
//            case 0x61: bpp=4;  break;   /* I */
//            case 0x62: bpp=4;  break;   /* IA */
//            case 0x63: bpp=4;  break;   /* CI */
//            case 0x64: bpp=8;  break;   /* I */
//            case 0x65: bpp=8;  break;   /* IA */
//            case 0x66: bpp=8;  break;   /* CI */
//            case 0x67: bpp=16; break;   /* IA */
//            case 0x68: bpp=15; break;   /* RGBA */
//            case 0x69: bpp=16; break;   /* YUV */
//            case 0x6a: bpp=32; break;   /* RGBA */

            default:   bpp=8;  break;
        }
    }
    return(bpp);
}

//bool MVCODEC_is(const SHPHeaderStruct *shape);

//--------------------------------------------------------------------------------
// compression identifying function. Returns the type if any with which
// the data is compressed. Otherwise returns COMPRESS_NON which is
// a zero condition and thus evaluates to false.
static compress_type isCompressed(SHPHeaderStruct *theShape,int type)
{
    if (!(theShape) || !((int)(SHPGETFIELD(theShape->flags))&ELEMENT_FLAG_COMPRESSED))
        return COMPRESS_NON;
    if (DXT_is((char *)theShape+sizeof(SHPHeaderStruct)))
        return COMPRESS_DXT;
    else if (REF_is((char *)theShape+sizeof(SHPHeaderStruct)))
        return COMPRESS_REF;
    else if (IDEC_is_idec((char *)theShape+sizeof(SHPHeaderStruct)))
        return COMPRESS_MPG;
//    else if (MVCODEC_is((const SHPHeaderStruct *)((char *)theShape+sizeof(SHPHeaderStruct))))
//        return COMPRESS_MV;
    else
        return COMPRESS_NON;
}

//--------------------------------------------------------------------------------
// predicate function for identifying the compression type returns
// true if the data is compressed using the identified type.
static int compressedP(SHPHeaderStruct *theShape,compress_type ctype,int type)
{
    if (!(SHPGETFIELD(theShape->flags)&ELEMENT_FLAG_COMPRESSED))
        return 0;
    // check the codec header for the right compression pattern
    if (ctype == COMPRESS_DXT)
        return DXT_is((char *)theShape+sizeof(SHPHeaderStruct));
    else if (ctype == COMPRESS_REF)
        return REF_is((char *)theShape+sizeof(SHPHeaderStruct));
    else if (ctype == COMPRESS_MPG)
        return IDEC_is_idec((char *)theShape+sizeof(SHPHeaderStruct));
    return 0;
}


//--------------------------------------------------------------------------------
//   Put number of mipmap levels into shape header struct
//   in the upper 4 bits of the flags field.
static void SHPWSetMipmapLevels(SHPHeaderStruct *theShape, int levels, int type)
{
    int flags;

    if (levels==0)
        levels = 1;
    if (theShape)
    {
        // dxt shape wants to know the number of levels as well
        DXTHEADER *dxt = (DXTHEADER *)((char *)theShape+sizeof(SHPHeaderStruct));

        flags = SHPGETFIELD(theShape->flags) & (~TEXTURE_FLAG_NUM_MIP_MAPS);
        SHPPUTFIELD(theShape->flags, flags + ((levels-1) << 20));

        if (compressedP(theShape,COMPRESS_DXT,type))
        {
            dxt->mode[0] = (dxt->mode[0] & 0x0f) | ((levels-1) << 4);
        }
    }
}

//--------------------------------------------------------------------------------------------------------
//   Mipmap levels are stored in upper 4 bits of
//   flags. If 0 then shape has no mipmap levels;
//   non-zero records total number of levels,
//   including level 0.
static int  SHPWGetMipmapLevels(SHPHeaderStruct *theShape,int type)
{
    int flags;
    int levels;
    int width;

    if (!theShape)
        return 0;
    // extract the information from the header
    width  = SHPGETFIELD(theShape->width);
    flags  = SHPGETFIELD(theShape->flags);
    levels = (flags&TEXTURE_FLAG_NUM_MIP_MAPS) >> 20;
    // sanity check and augment the number of levels to include base texture
    if ( (levels == 0) | (levels == 15) )
        levels = 1;
    else
        levels += 1;
    // sanity check for mipmap level information.
    if ((1<<(levels-1)) > width)
        levels = 1;
    return(levels);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set the relative bit depth if shpout is DXT compressed.
static void GetDXTBpp(SHPHeaderStruct *shpout,SHPHeaderStruct *shp,int collapse,int * bpp,int *dxt,int type)
{
    // if the shape is packed, it's a DXT shape or DXT pack type then we will check further.
    if (compressedP(shpout,COMPRESS_DXT,type) || collapse)
    {
        if (DXT3_is((const char *)shpout+sizeof(SHPHeaderStruct)) || SHPGETFIELD((shp->type)) == 0x61)
        {
            *bpp = 8; // DXT3 treated as 8 bpp
            *dxt = 3;
        }
        else if (DXT5_is((const char *)shpout+sizeof(SHPHeaderStruct))|| SHPGETFIELD(shp->type) == 0x62)
        {
            *bpp = 8; // DXT5 treated as 8 bpp
            *dxt = 5;
        }
        else  // note this encompasses DXT_1B as well.
        {
            *bpp = 4; // DXT1 treated as 4 bpp
            *dxt = 1;
        }
    }
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Rewrite mipmapped shapes into single shape. The shape
//   header has the width and height of the top level; data
//   for following levels is concatenated to top level data.
static void SHPWCollapseMipmaps(ShapeListPtr theShape,int type, int shpalignment, int shpmipmapdataalignment)
{
    int             in, out;
    GShapeInfo      *sinfo, *sinfoout;
    SHPHeaderStruct *shp, *shpout;
    int levels = 0;
    char * newdata = 0;
    int newsize    = 0;
    int collapse   = 0;
    int notmipmap  = 0;
    /* Start at 1, since 0th frame has nothing to be appended to. */

    out = 0;
    in = 1;
    while (in < theShape->NumShape)
    {
        sinfoout = &theShape->ListShape[out];
        sinfo = &theShape->ListShape[in];
        shp = (SHPHeaderStruct *)sinfo->shape_Data;
        shpout = (SHPHeaderStruct *)sinfoout->shape_Data;

        /* Mipmaps identified by signature "mm<digit>". Add all such
           consecutive shapes to the current "output" shape. */

        notmipmap = 0;
        if (SHPismipmap(sinfo->shape_Name,0) && (SHPWGetShpbpp(shp, type) == SHPWGetShpbpp(shpout, type)))
        {
            int width=0, height=0, bpp=0, thissize=0, dxt=0;

            width    = SHPGETFIELD(shp->width);
            height   = SHPGETFIELD(shp->height);
            bpp      = SHPWGetShpbpp(shp, type);
            // figure out if this is a DXT shape type or a packtype dxt shape.. either way set bpp correctly.
            collapse = ((SHPGETFIELD(shpout->type) >= 0x60) && (SHPGETFIELD(shpout->type) <= 0x62) || (SHPGETFIELD(shpout->type) == 0x1e)); // 1e gsh comparable to dxt1.
            GetDXTBpp(shpout,shp,collapse,&bpp,&dxt,type);

            /* On first level we allocate space for combined data;
               assuming that there may be mipmaps all the way down
               to 1x1. */

            if (!levels)
            {
                int w = SHPGETFIELD(shpout->width);
                int h = SHPGETFIELD(shpout->height);

                if (dxt && type==TYPEGSH)
                {
                    w = (w+7)&~7;
                    h = (h+7)&~7;
                }

                if ((type == TYPEGSH || type == TYPESSH || type == TYPEMSH) && SHPGETFIELD(shpout->flags)&TEXTURE_FLAG_SWIZZLED && !dxt)  // is swizzled GSH?
                {
                    int realwidth  = 0;
                    int realheight = 0;
                    if (type == TYPEGSH)
                        RetGSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                    if (type == TYPESSH)
                        RetSSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                    if (type == TYPEMSH)
                        RetMSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                    thissize = newsize = SHPWCalcSizeShape(realwidth, realheight, bpp, type, dxt);
                }
                else
                    thissize = newsize = SHPWCalcSizeShape(w, h, bpp, type, dxt);

                while ((w > 1) || (h > 1))
                {
                    if (w > 1)  w >>= 1;
                    if (h > 1)  h >>= 1;
                    if ((type == TYPEGSH || type == TYPESSH || type == TYPEMSH) && SHPGETFIELD(shpout->flags)&TEXTURE_FLAG_SWIZZLED && !dxt)  // is swizzled GSH?
                    {
                        int realwidth  = 0;
                        int realheight = 0;
                        if (type == TYPEGSH)
                            RetGSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                        if (type == TYPESSH)
                            RetSSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                        if (type == TYPEMSH)
                            RetMSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                        newsize += SHPMIPMAPALIGN(thissize) + SHPWCalcSizeShape(realwidth, realheight, bpp, type, dxt);
                    }
                    else if (dxt && type==TYPEGSH)
                        newsize += SHPMIPMAPALIGN(thissize) + SHPWCalcSizeShape(((w+7)&~7), ((h+7)&~7), bpp, type, dxt);
                    else
                        newsize += SHPMIPMAPALIGN(thissize) + SHPWCalcSizeShape(w, h, bpp, type, dxt);
                }
                newsize += sizeof(SHPHeaderStruct);
                if (dxt && !collapse) // dont remove dxt header
                    newsize += 8;

                newdata = (char *)galloc(newsize*2+1024); // 2+1024 is a kludge for dxt3 corruption issue
                memset(newdata,0,newsize*2+1024);

                memcpy(newdata, shpout, sizeof(SHPHeaderStruct));

                newsize = sizeof(SHPHeaderStruct);

                /* Add top level of mipmap. */
                if (compressedP(shpout,COMPRESS_REF,type))
                    REF_decode(newdata + newsize, (char*)shpout + sizeof(SHPHeaderStruct), 0);
                else if (dxt && !collapse) // dont remove dxt header
                {
                    thissize += 8;  // copy header on first one
                    memcpy(newdata + newsize, (char*)shpout + sizeof(SHPHeaderStruct), thissize);
                }
                else
                    memcpy(newdata + newsize, (char*)shpout + sizeof(SHPHeaderStruct), thissize);
                newsize += thissize;
                levels = 1;
            }

            /* Append data for this shape to newdata for "out" shape. */
            if (dxt && !collapse) // dont remove dxt header
                newsize = SHPMIPMAPALIGN(newsize-(sizeof(SHPHeaderStruct)+8))+(sizeof(SHPHeaderStruct)+8);
            else
                newsize = SHPMIPMAPALIGN(newsize-sizeof(SHPHeaderStruct))+sizeof(SHPHeaderStruct);


            if ((type == TYPEGSH || type == TYPESSH || type == TYPEMSH) && SHPGETFIELD(shpout->flags)&TEXTURE_FLAG_SWIZZLED && !dxt)  // is swizzled GSH?
            {
                int realwidth  = 0;
                int realheight = 0;
                if (type == TYPEGSH)
                    RetGSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                if (type == TYPESSH)
                    RetSSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                if (type == TYPEMSH)
                    RetMSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                thissize = SHPWCalcSizeShape(realwidth, realheight, bpp, type, dxt);
            }
            else if (dxt && type==TYPEGSH)
            {
                width  = (width+7)&~7;
                height = (height+7)&~7;
                thissize = SHPWCalcSizeShape(width, height, bpp, type, dxt);
            }
            else
                thissize = SHPWCalcSizeShape(width, height, bpp, type, dxt);


            if (compressedP(shp,COMPRESS_REF,type))
                REF_decode(newdata + newsize,(char *)shp + sizeof(SHPHeaderStruct), 0);
            else if (dxt && !collapse)
                memcpy(newdata + newsize, (char *)shp + sizeof(SHPHeaderStruct) + 8, thissize);
            else
                memcpy(newdata + newsize, (char *)shp + sizeof(SHPHeaderStruct), thissize);

            /* Free this shape and mark as such. (Note that cluts and
               hot spots get lost for all but top level of mipmap). */

            if (sinfo->shape_Data)
                gfree(sinfo->shape_Data);
            if (sinfo->shape_Clut)
                gfree(sinfo->shape_Clut);
            if (sinfo->shape_Metal)
                gfree(sinfo->shape_Metal);
            if (sinfo->shape_DataBin)
                gfree(sinfo->shape_DataBin);
            if (sinfo->shape_HotSpot)
                gfree(sinfo->shape_HotSpot);
            if (sinfo->shape_Comment)
                gfree(sinfo->shape_Comment);
            if (sinfo->shape_Name)
                gfree(sinfo->shape_Name);

            sinfo->shape_SizeData    = 0;
            sinfo->shape_SizeClut    = 0;
            sinfo->shape_SizeMetal   = 0;
            sinfo->shape_SizeHotSpot = 0;
            sinfo->shape_SizeComment = 0;
            sinfo->shape_Name        = 0;

            /* Add size for mNext level of mipmap. */

            newsize += thissize;
            ++levels;
        } // END if SHPismipmap
        else
            notmipmap = 1;            // set this because we can't check shp if it's memory gets deleted later.

        /* If not a mipmap level or we're at the last shape
           then store out the results. */

        if ((in == theShape->NumShape - 1) || notmipmap)
        {
            /* If we just finished combining mipmaps then save
               the combined data in the output shape. */

            if (newdata)
            {
                // set the DataSize field to include mipmaps but no the padding
                SHPPUTFIELD(((SHPHeaderStruct *)newdata)->DataSize,newsize);
                newsize = SHPALIGN(newsize);

                /* Check whether to pack the new data. */

                if (compressedP(shpout,COMPRESS_REF,type))
                {
                    char *newdatapack = (char *)galloc(newsize*3/2+1024);

                    /* Pack data into newly allocated space. This is wasteful of memory, oh well. */

                    memcpy(newdatapack, newdata, sizeof(SHPHeaderStruct));
                    newsize = (REF_encode(newdatapack + sizeof(SHPHeaderStruct),
                                          newdata + sizeof(SHPHeaderStruct),
                                          newsize - sizeof(SHPHeaderStruct),
                                          0));
                    newsize += sizeof(SHPHeaderStruct);
                    gfree(newdata);
                    newdata = newdatapack;
                    // set the DataSize field to include the compressed mipmaps
                    SHPPUTFIELD(((SHPHeaderStruct *)newdata)->DataSize,newsize);
                }

                gfree(sinfoout->shape_Data);
                sinfoout->shape_Data = newdata;
                sinfoout->shape_SizeData = newsize;
                shpout = (SHPHeaderStruct *)newdata;

                SHPWSetMipmapLevels(shpout, levels, type);
            }

            /* mNext output shape will be this shape. */
            out = in;
            levels = 0;
            newdata = 0;
            newsize = 0;
        }

        ++in;
    }

    /* Second pass through shape list removes all the unused entries. */

    out = 0;
    for (in = 0; in < theShape->NumShape; ++in)
    {
        if (theShape->ListShape[in].shape_SizeData ||
            theShape->ListShape[in].shape_SizeClut ||
            theShape->ListShape[in].shape_SizeMetal ||
            theShape->ListShape[in].shape_SizeHotSpot ||
            theShape->ListShape[in].shape_SizeComment)
        {
            if (in != out)
            {
                memcpy(&theShape->ListShape[out],
                       &theShape->ListShape[in],
                       sizeof(GShapeInfo));
            }

            ++out;
        }
    }

    /* Reset total number of shapes to the number remaining after mipmap collapsing. */

    theShape->NumShape = out;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// includes header size.
static char * CreateCubemapBuffer(int width,int height,int bpp,int * newsize, int levels,int shpmipmapdataalignment,int shpalignment)
{
    int csize = sizeof(SHPHeaderStruct);
    // assume width is right and that height is divided by 6 to get real height.
    int rsize = height/6;
    int onesize = 0;
    int i = 0;
    int w = width;
    int h = rsize;
    char * newbuf = 0;
    // for each mipmaplevel.
    for(i = 0; i < levels; i++)
    {
        onesize = (int)(SHPMIPMAPALIGN(onesize) + ((float)bpp/8.0)*w*h);
        w = w>>1;
        h = h>>1;
        h = h<1?1:h;
        w = w<1?1:w;
    }
    // align that face.
    //onesize += SHPFACEALIGN(onesize); we will need to align to the face alignment in the future.
    onesize = onesize*6;
    csize += onesize;
        //SHPALIGN(csize);

    *newsize = csize;
    newbuf   = (char *)galloc(csize);
    return (newbuf);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static int getCubeOffset(int l,int s,int shpalignment,int shpmipmapdataalignment,int levels,int * lsize,int width,int height,int bpp)
{
    int w = width;
    int h = height;
    int levelsize = 0;
    int i = 0;
    int tsize = 0;
    // go down blocks of mipmaps till I get to my mipmap level.
    for(i = 0 ; i < l; i++)
    {
        levelsize += w*bpp/8*h;
        if (i != (levels-1))
            levelsize = SHPMIPMAPALIGN(levelsize);
        w = w>>1;
        h = h>>1;
    }
    // which face am I on?
    // jump to that face.
    h = h/6;
    *lsize = (int)(((float)bpp/8.0)*h*w);
    tsize = (*lsize)*s + levelsize;
    return(tsize);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static int locNextCube(int l,int current,int shpalignment,int shpmipmapdataalignment,int levels,int lsize)
{
    // add the written size to the current position.
    current = (current + lsize);
    // align, in the future we will have to do the face alignment but currently we can't as
    // we do not know what to align to,
    if (l < (levels-1))
        current = SHPMIPMAPALIGN(current);
    //else
    //    current = SHPFACEALIGN(current);
    return(current);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Rewrite mipmapped shapes into single shape. The shape
//   header has the width and height of the top level; data
//   for following levels is concatenated to top level data.
static void SHPWOrderCubemaps(ShapeListPtr theShape,int type, int shpalignment, int shpmipmapdataalignment)
{
    int             in        = 0;
    GShapeInfo      *sinfo    = 0;
    SHPHeaderStruct *shp      = 0;
    GShapeInfo      *sinfoout = 0;
    SHPHeaderStruct *shpout   = 0;
    int levels                = 0;
    int collapse              = 0;
    int size                  = 0;

    for(in = 0; in < theShape->NumShape;in++)
    {
        sinfo    = &theShape->ListShape[in];
        shp      = (SHPHeaderStruct *)sinfo->shape_Data;
        sinfoout = sinfo;             // just to make relevant code look like the collapse mipmaps code.
        shpout   = shp;
        levels   = SHPWGetMipmapLevels(shp,type);
        // cubemaps are already collapsed. They will look like a single shape with all 6 faces followed by stips of mipmaps. Each strip will contain
        // all the mipmaps for all the feaces for a specific level.
        // so if we effectively have some mipmap levels then we will have to reorder them here.
        if ((sinfo->subtype&SUBCUBEMAP)  && (levels > 1))
        {
            int width            = 0;
            int height           = 0;
            int bpp              = 0;
            int dxt              = 0;
            int s                = 0;
            int l                = 0;
            int lsize            = 0;
            int offset           = 0;
            unsigned int current = sizeof(SHPHeaderStruct);
            char * tempbuf       = 0;
            char * data          = (char *)shp + sizeof(SHPHeaderStruct);

            width  = SHPGETFIELD(shp->width);
            height = SHPGETFIELD(shp->height);
            bpp    = SHPWGetShpbpp(shp, type);
            // figure out if this is a DXT shape type or a packtype dxt shape.. either way set bpp correctly.
            collapse = ((SHPGETFIELD(shpout->type) >= 0x60) && (SHPGETFIELD(shpout->type) <= 0x62) || (SHPGETFIELD(shpout->type) == 0x1e)); // 1e gsh comparable to dxt1.
            GetDXTBpp(shpout,shp,collapse,&bpp,&dxt,type);
            // get size of new buffer and create.
            tempbuf = CreateCubemapBuffer(width,height,bpp,&size,levels,shpmipmapdataalignment,shpalignment);
            memset(tempbuf,0,size);
            memcpy(tempbuf,shp,sizeof(SHPHeaderStruct));
            unsigned int tempflags = SHPGETFIELD(((SHPHeaderStruct *)tempbuf)->flags);
            SHPPUTFIELD(((SHPHeaderStruct *)tempbuf)->flags,TEXTURE_FLAG_CUBE_MAP|tempflags);
            // for side of the cube.
            for(s = 0; s < 6; s++)
            {
                // for mipmap level.
                for(l = 0; l < levels;l++)
                {
                    // get offset of level from top.
                    // get size of level.
                    offset = getCubeOffset(l,s,shpalignment,shpmipmapdataalignment,levels,&lsize,width,height,bpp);
                    // copy over bitmap.
                    memcpy(tempbuf+current,data+offset,lsize);
                    // increment current position.
                    current = locNextCube(l,current,shpalignment,shpmipmapdataalignment,levels,lsize);
                }
            }
            // cleanup
            if (sinfo->shape_Data)
                gfree(sinfo->shape_Data);
            sinfo->shape_Data     = tempbuf;
            sinfo->shape_SizeData = size;
        }

    }  // for (in) over numshapes.
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// adjust size of collapsed shape to account for palette alignment
static void SHPWAlignPalette(ShapeListPtr theShape,int type, int shpclutdataalignment)
{
    int in;

    for (in = 0; in < theShape->NumShape; ++in)
    {
        if (theShape->ListShape[in].shape_Data &&
            theShape->ListShape[in].shape_SizeData &&
            theShape->ListShape[in].shape_SizeClut)
        {
            int padlen;
            int offset=theShape->ListShape[in].shape_SizeData;
            void *newdata;

            padlen = ((offset-sizeof(SHPHeaderStruct)+shpclutdataalignment+sizeof(SHPHeaderStruct))&~shpclutdataalignment)-offset;
            if (padlen>0)
            {
                newdata = galloc(offset+padlen);
                memset((char *)newdata+offset,0,padlen);
                memcpy(newdata,theShape->ListShape[in].shape_Data,offset);
                gfree(theShape->ListShape[in].shape_Data);
                theShape->ListShape[in].shape_Data = (char *) newdata;
                theShape->ListShape[in].shape_SizeData = offset+padlen;
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// mNext the clut, hotspot, comment to the shape
static int SHPWSetNexts(ShapeListPtr theShape,int type,int shppackeddataalignment,int shpdataalignment)
{
    int i;
    int padlen          = 0;

    for (i=0; i<theShape->NumShape; ++i)
    {
        char *prevptr=0;
        int  prevlen=0;

        SHPWSetNext(&prevptr,&prevlen,theShape->ListShape[i].shape_Data,theShape->ListShape[i].shape_SizeData, type);
        SHPWSetNext(&prevptr,&prevlen,theShape->ListShape[i].shape_Clut,theShape->ListShape[i].shape_SizeClut, type);
        SHPWSetNext(&prevptr,&prevlen,theShape->ListShape[i].shape_Metal,theShape->ListShape[i].shape_SizeMetal, type);
        SHPWSetNext(&prevptr,&prevlen,theShape->ListShape[i].shape_DataBin,theShape->ListShape[i].shape_SizeDataBin, type);
        SHPWSetNext(&prevptr,&prevlen,theShape->ListShape[i].shape_HotSpot,theShape->ListShape[i].shape_SizeHotSpot, type);
        SHPWSetNext(&prevptr,&prevlen,theShape->ListShape[i].shape_Comment,theShape->ListShape[i].shape_SizeComment, type);
    }
    return(padlen);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// handle all swizzling actions for all formats. returns a pointer to the swizzled data on
// success NULL on failure.
static char * SwizzleAll(unsigned char * MemPtr,int Width, int Height,int Shapebpp,int ShapeType,int type,int ShapeRowBytes)
{

    int x,y;
    unsigned int Value = 0;
    int bypp         = 1;
    int ShapeOffset  = 0;
    int MemOffset    = 0;
    int MemRowbase   = 0;
    int ShapeRowbase = 0;
    char * ShapePtr = (char *)galloc(ShapeRowBytes*Height+sizeof(SHPHeaderStruct));         // create a temp buffer to swizzle into.

    if (ShapePtr)
    {
        // clear the buffer for fsh 4bpp
        memset(ShapePtr,0,ShapeRowBytes*Height+sizeof(SHPHeaderStruct));

        // special case stuff for SSH  Functions Swizzle 8 or 4 and then unswizzle 32 for speed increase.
        if (type == TYPESSH)
        {
            if (Shapebpp == 8)
                SHAPE_swizzle8((unsigned char *)MemPtr,(unsigned char *)ShapePtr,Width,Height);

            else if (Shapebpp == 4)
                SHAPE_swizzle4((unsigned char *)MemPtr,(unsigned char *)ShapePtr,Width,Height);

            else if (Shapebpp == 16 || Shapebpp == 15)
                SHAPE_swizzle16((unsigned char *)MemPtr,(unsigned char *)ShapePtr,Width,Height);

            else if (Shapebpp == 32)
                memcpy(ShapePtr,MemPtr,ShapeRowBytes*Height);
            else if (Shapebpp == 24)   // not supported
            {
                if (ShapePtr) gfree(ShapePtr);
                return (NULL);
            }

            return(ShapePtr);
        }

        // other formats.
        if (type == TYPEGSH || type == TYPEFSH || type == TYPEXSH || type == TYPEMSH || type==TYPEZSH || type==TYPEYSH)
        {
            // have to set the byte per pixel amount. Most of the 4bpp formats can
            // be treated the same as the 8bpp format in the unswizzle, only
            // fsh and psh need special care.
            if (Shapebpp <= 8)       bypp = 1;
            else if (Shapebpp <= 16) bypp = 2;
            else if (Shapebpp <= 24) bypp = 3;
            else if (Shapebpp <= 32) bypp = 4;

            for (y=0;y<Height;++y)
            {
                MemOffset=MemRowbase;
                ShapeOffset=ShapeRowbase;

                for (x=0; x<Width; ++x)
                {
                    if (type==TYPEGSH)
                    {
                        ShapeOffset = SHPWswizzleGC(x,y,Width,Shapebpp);// this swizzles the dest not the src like PS2 swizzle
                    }
                    else if (type==TYPEFSH || type==TYPEXSH || type==TYPEZSH || type==TYPEYSH)
                    {
                        int PixOffset;
                        int tx,ty;

                        PixOffset = SHPWswizzleXSH(x,y,Width,Height);

                        ty = PixOffset/Width;
                        tx = PixOffset%Width;

                        // 4 bit kludge
                        if (ShapeType == 0x40 || ShapeType == 0x79 || ShapeType == 0x7a)
                            MemOffset = ty*ShapeRowBytes+tx/2;
                        else
                            MemOffset = ty*ShapeRowBytes+tx*bypp;            // do a regular pixel lookup
                    }
                    else if (type==TYPEMSH)
                    {
                        int PixOffset;
                        int tx,ty;

                        PixOffset = SHPWswizzleMSH(x,y,Width,Height,Shapebpp);

                        ty = PixOffset/Width;
                        tx = PixOffset%Width;

                        // 4 bit kludge
                        if(ShapeType == 0x5c)
                            ShapeOffset = ty*ShapeRowBytes+tx/2;
                        else
                            ShapeOffset = ty*ShapeRowBytes+tx*bypp;            // do a regular pixel lookup
					}

                    // GSH 32 bpp and
                    // GSH uncompressed DXT have a funny swizzle type for greater cache locality.
                    if (type == TYPEGSH && (ShapeType == 0x16 || ShapeType == 0x1e))
                    {
                        (*(ShapePtr+ShapeOffset+0))  = *(MemPtr+MemOffset+0);
                        (*(ShapePtr+ShapeOffset+1))  = *(MemPtr+MemOffset+1);
                        (*(ShapePtr+ShapeOffset+32)) = *(MemPtr+MemOffset+2);
                        (*(ShapePtr+ShapeOffset+33)) = *(MemPtr+MemOffset+3);
                    }

                    else if (ShapeType == 0x40 || ShapeType == 0x79 || ShapeType == 0x7a || ShapeType == 0x5c)
                    {                                                    // fsh's (and psh,msh) have to have each nibble extracted
                        Value=MemPtr[MemOffset];                         // and reinserted. Rather ugly but it's the way it
                        if (x&1)                                         // has to be done.
                            ShapePtr[ShapeOffset] |= (char)(Value&0xF0);
                        else
                            ShapePtr[ShapeOffset] |= (char)(Value&0x0F);
                    }

                    else                                                           // all other formats can just use a memcpy.
                        memcpy(ShapePtr+ShapeOffset,MemPtr+MemOffset,bypp);

                    if (Shapebpp != 4 || (Shapebpp == 4 && x&1))                    // only incerment every second pass for the bpp types otherwise always increment.
                    {
                        MemOffset     += bypp;
                        ShapeOffset   += bypp;
                    }

                }
                ShapeRowbase += ShapeRowBytes;
                MemRowbase   += ShapeRowBytes;

            }
        }// if type == GSH,XSH,FSH,ASH
        return(ShapePtr);
    }// if ShapePtr
    return(NULL);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// extract the details of writing a pixel at a given memory location into one function.
// this should hopefully allow for easier code maintainability.
inline static void SHPWWritePixel(const GINFO * ginfo,unsigned char * MemPtr,unsigned char * ShapePtr,int *pMemOffset,int * pShapeOffset,int shptype,int *px, int *py,int Width,int Height,int type)
{
    int Value       = 0;
    int i           = 0;
    int x           = *px;
    int y           = *py;
    int MemOffset   = *pMemOffset;
    int ShapeOffset = *pShapeOffset;
    switch (shptype)
    {
        case 0x44:      /* PSH 1 bpp */
        case 0x60:      /* NSH 1 bpp */
            Value = 0;
            for (i = 7; (i >= 0) && (x < Width); --i,++x)
            {
                Value |= (MemPtr[MemOffset++] & 0x01) << i;
            }
            ShapePtr[ShapeOffset++] = (char)Value;
            --x;
            break;

        case 0x01:      /* SSH */
        case 0x18:      /* GSH */
        case 0x5c:      /* MSH */
        case 0x40:      /* PSH */
        case 0x62:      /* NSH 4-bit intensity/alpha */
//        case 0x63:      /* NSH 4-bit colour-indexed */
        case 0x79:
        case 0x7a:
            if (shptype==0x01  /* SSH 4 bit */
             || shptype==0x40  /* PSH 4 bit */
             || shptype==0x5c  /* MSH 4 bit */
             || shptype==0x79) /* FSH 4 bit */
            {
                Value=MemPtr[MemOffset++]&15;
                if (++x<Width) Value |= (MemPtr[MemOffset++]&15)<<4;
            }
            else        /* GSH/NSH 4 bit */
            {
                Value=MemPtr[MemOffset++]<<4;
                if (++x<Width) Value |= (MemPtr[MemOffset++]&15);
            }

            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x10:      /* GSH 4-bit intensity--look up grey scale. */
            if (ginfo->bpp == 8)
            {
                Value = ggetm(MemPtr+(MemOffset++),1);
                Value = (ginfo->colourtbl[Value].r)/16;
                Value <<= 4;
                ++x;
                if (x < Width)
                {
                    int v2;
                    v2 = ginfo->colourtbl[ggetm(MemPtr+(MemOffset++),1)].r;
                    Value |= (v2/16) & 0x0f;
                }
            }
            else   // 32 bit conversion to intensity 4.
            {
                Value=(int)GSHWpixel8ToI((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
                Value <<= 4;
                ++x;
                if (x < Width)
                {
                    int v2;
                    v2=(int)GSHWpixel8ToI((const ARGB *)(MemPtr+MemOffset));
                    Value |= (v2/16) & 0x0f;
                    MemOffset+=4;
                }
            }
            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x61:      /* NSH 4-bit intensity--look up grey scale. */
            Value = ggetm(MemPtr+(MemOffset++),1);
            Value = (ginfo->colourtbl[Value].r)/16;
            Value <<= 4;
            ++x;
            if (x < Width)
            {
                int v2;
                v2 = ginfo->colourtbl[ggetm(MemPtr+(MemOffset++),1)].r;
                Value |= (v2/16) & 0x0f;
            }
            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x02:      /* SSH 8 bit */
        case 0x19:      /* GSH 8 bit */
        case 0x5d:      /* MSH 8 bit */
        case 0x41:      /* PSH 8 bit */
            ShapePtr[ShapeOffset++] = MemPtr[MemOffset++];
            break;

        case 0x63:      /* FSH/XSH 8 bit luminance */
			if (ginfo->bpp == 8)
			{
				Value = MemPtr[MemOffset++];
				ShapePtr[ShapeOffset++] = ginfo->colourtbl[Value].r;
			}
			else
			{
				ShapePtr[ShapeOffset++] = ((ARGB *)(MemPtr+MemOffset))->r;
				MemOffset += 4;
			}
            break;

		case 0x64:      /* FSH/XSH 8 bit alpha */
			if (ginfo->bpp == 8)
			{
				Value = MemPtr[MemOffset++];
				ShapePtr[ShapeOffset++] = ginfo->colourtbl[Value].a;
			}
			else
			{
				ShapePtr[ShapeOffset++] = ((ARGB *)(MemPtr+MemOffset))->a;
				MemOffset += 4;
			}
            break;

        case 0x7b:      /* FSH/VSH 8 bit */
            Value = MemPtr[MemOffset++];
            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x11:      /* GSH 8-bit intensity format */
            if (ginfo->bpp == 8)
			{
                Value=(int)GSHWpixel8ToI((const ARGB *)&(ginfo->colourtbl[*((unsigned char *)(MemPtr+MemOffset))]));
                MemOffset+=1;
	        }
	        else
	        {
                Value=(int)GSHWpixel8ToI((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
	        }
            gputm(ShapePtr+ShapeOffset,Value,1);
            ShapeOffset+=1;
            break;

        case 0x12:      /* GSH 8-bit intensity/alpha format*/
            if (ginfo->bpp == 8)
            {
                Value=(int)GSHWpixel4ToIA((const ARGB *)&(ginfo->colourtbl[*((unsigned char *)(MemPtr+MemOffset))]));
                MemOffset+=1;
            }
            else
            {
                Value=(int)GSHWpixel4ToIA((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
            }
            gputm(ShapePtr+ShapeOffset,Value,1);
            ShapeOffset+=1;
            break;

		case 0x6c: /* FSH 16 bpp YUV and FSH 16 bpp YUY2 formats not supported here */
		case 0x68:
			break;

        case 0x15:      /* GSH 15 bit */
            Value=(int)GSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputx(ShapePtr+ShapeOffset,Value,2,type);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x7e:      /* FSH 15 bit */
            Value=(int)FSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x03:      /* SSH 15 bit */
        case 0x59:      /* MSH 15 bit */
            Value=(int)SSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

//        case 0x68:      /* NSH 15-bit RGBA */
//            Value=(int)NSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
//            gputm(ShapePtr+ShapeOffset,Value,2);
//            ShapeOffset+=2;
//            MemOffset+=4;
//            break;

        case 0x14:      /* GSH 16 bit */
            Value=(int)FSHWpixel8To565((const ARGB *)(MemPtr+MemOffset));
            gputm(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x58:      /* MSH 16 bit */
            Value=(int)MSHWpixel8To565((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x78:      /* FSH 16 bit */
            Value=(int)FSHWpixel8To565((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x65:      /* XSH AL_88 */
            if (ginfo->bpp <= 8)
            {
                if (ginfo->redbits > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ginfo->colourtbl[(*(unsigned char *)(MemPtr+MemOffset))].r;
                else if (ginfo->greenbits > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ginfo->colourtbl[(*(unsigned char *)(MemPtr+MemOffset))].g;
                else if (ginfo->bluebits  > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ginfo->colourtbl[(*(unsigned char *)(MemPtr+MemOffset))].b;
                ((char *)(ShapePtr+ShapeOffset))[1] = ginfo->colourtbl[(*(unsigned char *)(MemPtr+MemOffset))].a;
                MemOffset+=1;
            }
            else
            {
                if (ginfo->redbits > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ((const ARGB *)(MemPtr+MemOffset))->r;
                else if (ginfo->greenbits > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ((const ARGB *)(MemPtr+MemOffset))->g;
                else if (ginfo->bluebits  > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ((const ARGB *)(MemPtr+MemOffset))->b;
                ((char *)(ShapePtr+ShapeOffset))[1] = ((const ARGB *)(MemPtr+MemOffset))->a;
                MemOffset+=4;
            }
            ShapeOffset+=2;
            break;

        case 0x67:      /* XSH 655 bit */
            Value=(int)FSHWpixel8To655((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x13:      /* GSH 16-bit intensity/alpha embm format*/
            if (ginfo->bpp <= 8)
            {
                Value=(int)GSHWpixel8ToIA((const ARGB *)&(ginfo->colourtbl[*((unsigned char *)(MemPtr+MemOffset))]));
                MemOffset+=1;
            }
            else
            {
                Value=(int)GSHWpixel8ToIA((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
            }
            gputm(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            break;

        case 0x42:      /* PSH 16 bit */
            Value=(int)PSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x5a:      /* MSH 4444 */
            Value=(int)MSHWpixel8To4444((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x6d:      /* FSH 16 bit argb (4444) */
            Value=(int)FSHWpixel8To4444((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x66:      /* FSH 24 bit argb (6666) */
            Value=(int)FSHWpixel8To6666((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,3);
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x6a:      /* FSH 32 bit argb (2101010) */
            Value=(int)FSHWpixel8To2101010((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,4);
            ShapeOffset+=4;
            MemOffset+=4;
            break;


        case 0x04:      /* SSH 24 bit */
        case 0x43:      /* PSH 24 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->b;
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x7f:      /* FSH 24 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->b;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->r;
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x7d:      /* FSH 32 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->b;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->a;
            ShapeOffset+=4;
            MemOffset+=4;
            break;

        case 0x05:      /* SSH 32 bit */
        case 0x5b:      /* MSH 32 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->b;
            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->a;
            ShapeOffset+=4;
            MemOffset+=4;
            break;

        case 0x16:      /* GSH 32 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->a;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->b;
            ShapeOffset+=4;
            MemOffset+=4;
            break;

//        case 0x6a:      /* NSH 32 bit RGBA */
//            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->r;
//            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
//            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->b;
//            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->a;
//            ShapeOffset+=4;
//            MemOffset+=4;
//            break;

        default:
            break;
    }
    *px           = x;                     // reset these values for mNext loop.
    *py           = y;
    *pMemOffset   = MemOffset;
    *pShapeOffset = ShapeOffset;
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Write the actual Bitmap, deals with swizzling and different shapetypes.
static void SHPWWriteBitmap(SHPHeaderStruct *HeaderPict,char *ShapePtr,const GINFO *ginfo,const char *SourceMemPtr,int MemRowBytes,int type, int subtype,int rw,int rh)
{
    int     Width;
    int     Height;
    int    ShapeRowBytes,Shapebpp;
    int    Membpp;
    int    MemOffset,ShapeOffset;
    int    MemRowbase,ShapeRowbase;
    int    x,y;
    unsigned char *temp = 0;
    unsigned char *MemPtr=(unsigned char *)SourceMemPtr;

    // get shape information.
    Shapebpp=SHPWGetShpbpp(HeaderPict,type);
    Width = ginfo->width;
    Height = ginfo->height;
    ShapeRowBytes=SHPWCalcRowBytes(Width,Shapebpp,type);


    if (subtype & SUBSWIZZLE && (type == TYPEGSH || type ==  TYPESSH || type ==  TYPEMSH))        // do we have a logically bigger shape?
        ShapeRowBytes=SHPWCalcRowBytes(rw,Shapebpp,type);


    Membpp = Shapebpp<=8?1:4;



    // be good and clear new shape buffer.
    if (subtype & SUBSWIZZLE && (type == TYPEGSH || type ==  TYPESSH || type ==  TYPEMSH))    // have to worry about logically bigger shapes!
        memset(ShapePtr,0,ShapeRowBytes*rh);                             // GSH has funny requirement of having larger shape in file
    else                                                                 // for the hardware to work with.
        memset(ShapePtr,0,ShapeRowBytes*Height);                         /* clear bmp to 0 */



    // loop over image and place in new image format.
    MemRowbase=0;
    ShapeRowbase=0;
    for (y=0;y<Height;++y)
    {
        MemOffset=MemRowbase;
        ShapeOffset=ShapeRowbase;

        for (x=0; x<Width; ++x)        // write out the pixel in the specified format.
            SHPWWritePixel(ginfo,MemPtr,(unsigned char *)ShapePtr,&MemOffset,&ShapeOffset,HeaderPict->type,&x, &y,Width,Height,type);

        ShapeRowbase += ShapeRowBytes; // Increment the pointer with a line align on 16 bits
        MemRowbase   += MemRowBytes;
    }



    // handle any swizzling that is necessary.
    if (subtype&SUBSWIZZLE)
    {
        if (type == TYPEGSH || type == TYPESSH || type ==  TYPEMSH)   // logically bigger shape in some cases!
            temp = (unsigned char *)SwizzleAll((unsigned char *)ShapePtr,rw,rh,Shapebpp,HeaderPict->type,type,ShapeRowBytes);
        else
            temp = (unsigned char *)SwizzleAll((unsigned char *)ShapePtr,Width,Height,Shapebpp,HeaderPict->type,type,ShapeRowBytes);
        if (temp)                       // just in case I cannot swizzle it save the data and clean up.
        {
            if (type == TYPEGSH || type == TYPESSH || type ==  TYPEMSH)
                memcpy(ShapePtr,temp,ShapeRowBytes*rh);
            else
                memcpy(ShapePtr,temp,ShapeRowBytes*Height);
            gfree(temp);
        }
    }


}


//-------------------------------------------------------------------------------------------------------

// game cube specific function for reordering swizzled blocks... blocks have to
// be in 12 56
//       34 78  order across screen (Note 4x4 block ordering.

// these are DXT block offsets. The x and y specify the DXT block in a linear scanline ordering.
// the width is the number of DXT blocks that I have in a scanline.
// returns a block ordering so multiply by 64 to get the byte offset.

static int SwizzleDXT1GetIndex( int x, int y, int width)
{
    int NumberOfBlocksX = (1+width)>>1;
    int XBlock = x>>1;
    int YBlock = y>>1;
    int XPix   = x&1;
    int YPix   = y&1;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<2) + // the offset to the current tile
            ((YPix<<1) + (XPix));                    // within the tile the current block (note * by 64 to get address)
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static void SHPWReorderDXTBlocks(int width, int height, int rowbytes, const unsigned char *source, unsigned char *dest, int type)
{
    int i;
    int j;
    int k;
	const int SIZEOFBLOCK=4*4*sizeof(ARGB);
    const unsigned char *s;
	unsigned char *d = dest;
    int copywidth;
    int clearwidth;
    int copyheight;
    int clearheight;

    for (i=0; i<height; i+=4)
    {

        /* If height is not a multiple of 4 and we're on the last row of blocks */
        if (i+4 > height)
        {
            copyheight = height-i;
            clearheight = 4-copyheight;
        }
        else
        {
            copyheight = 4;
            clearheight = 0;
        }

        s = source + i*rowbytes;
        for (j=0; j<width; j+=4)
        {
            /* If width is not a multiple of 4 and we are on the last block of the line */
            if (j+4 > width)
            {
                copywidth = width-j;
                clearwidth = 4-copywidth;
            }
            else
            {
                copywidth = 4;
                clearwidth = 0;
            }


			if (type == TYPEGSH)
	            d = dest+(SwizzleDXT1GetIndex(j>>2,i>>2,((width+7)&~7)>>2))*SIZEOFBLOCK;        // find the offset of this swizzled block.

            /* Copy only what exists in the source */
            for (k=0; k<copyheight; k++)
            {

                /* Copy only what exists in the source */
                memcpy(d, s + k*rowbytes, copywidth * 4);

                /* Clear the rest */
                if (clearwidth)
                {
                    memset(d + copywidth * 4, 0, clearwidth * 4);
                }
                d += 4 * 4;
            }

            /* Clear the rest */
            for (k=0; k<clearheight; k++)
            {
                memset(d, 0, 4 * 4);
                d += 4 * 4;
            }

            s += 4 * 4;
        }
    }
}

static unsigned char ROUNDCLAMPINT(float f)
{
    int i = (int) (f+0.5f);
    if (i<0) i=0;
    if (i>255) i=255;
    return (unsigned char) i;
}

// ms coefficients
//    Y = ( 0.256788f * R + 0.504129f * G + 0.097906f * B) +  16.0f;
//    U = (-0.148223f * R - 0.290993f * G + 0.439216f * B) + 128.0f;
//    V = ( 0.439216f * R - 0.367788f * G - 0.071427f * B) + 128.0f;


static void RGBtoYUV(float &Y, float &U, float &V, float R, float G, float B)
{
    Y = (float)(( 0.25678843 * R + 0.50412965 * G + 0.09790586 * B) +  16.0f);
    U = (float)((-0.14822295 * R - 0.29099280 * G + 0.43921576 * B) + 128.0f);
    V = (float)(( 0.43921557 * R - 0.36778826 * G - 0.07142731 * B) + 128.0f);
}
/*
static void YUVtoRGB(float &R, float &G, float &B, float Y, float U, float V)
{
    float C = Y - 16.0f;
    float D = U - 128.0f;
    float E = V - 128.0f;

    R = 1.164383f * C                   + 1.596027f * E  ;
    G = 1.164383f * C - (0.391762f * D) - (0.812968f * E) ;
    B = 1.164383f * C +  2.017232f * D                   ;
}

static float RGBCompare(float R0,float G0,float B0,float R1,float G1,float B1)
{
    return 0.25678843f*(R0-R1)*(R0-R1) +
           0.50412965f*(G0-G1)*(G0-G1) +
           0.09790586f*(B0-B1)*(B0-B1);
}
*/
#if 0
static float yfind(float U,float V,float R0,float G0,float B0)
{
    float bestdiff=10000000.0f;
    float besty=0.0f;

//    if (R0<10.0f && G0<10.0f && B0<10.0f)
//        return 16.0f;

    for (float y1=0.0f; y1<256.0f; y1+=1.0f)
    {
    	float R,G,B;

        YUVtoRGB(R,G,B,y1,U,V);

        if (R<0.0f) R=0.0f;
        if (G<0.0f) G=0.0f;
        if (B<0.0f) B=0.0f;
        if (R>255.0f) R=255.0f;
        if (G>255.0f) G=255.0f;
        if (B>255.0f) B=255.0f;

        float diff = RGBCompare(R,G,B,R0,G0,B0);
        if (diff<bestdiff)
        {
            bestdiff = diff;
            besty = y1;
        }
    }
    return besty;
}
#endif

//----------------------------------------------------------------------
// convert 2 32-bit source pixels to one 32-bit yuy2 value
void argb32toyuy2( unsigned char* pDst,
    unsigned char* pSrc0,
    unsigned char* pSrc1,
    unsigned char* pSrc2,
    unsigned char* pSrc3)
{
	float R0 = (float)(((ARGB *)pSrc0)->r);
	float G0 = (float)(((ARGB *)pSrc0)->g);
	float B0 = (float)(((ARGB *)pSrc0)->b);

	float R1 = (float)(((ARGB *)pSrc1)->r);
	float G1 = (float)(((ARGB *)pSrc1)->g);
	float B1 = (float)(((ARGB *)pSrc1)->b);

	float R2 = (float)(((ARGB *)pSrc2)->r);
	float G2 = (float)(((ARGB *)pSrc2)->g);
	float B2 = (float)(((ARGB *)pSrc2)->b);

	float R3 = (float)(((ARGB *)pSrc3)->r);
	float G3 = (float)(((ARGB *)pSrc3)->g);
	float B3 = (float)(((ARGB *)pSrc3)->b);

	float Y0,Y1,Y2,Y3;
	float U0,U1,U2,U3;
	float V0,V1,V2,V3;

    RGBtoYUV(Y0,U0,V0,R0,G0,B0);
    RGBtoYUV(Y1,U1,V1,R1,G1,B1);
    RGBtoYUV(Y2,U2,V2,R2,G2,B2);
    RGBtoYUV(Y3,U3,V3,R3,G3,B3);

        //float YC0 = 0.0f;//Y0 * 0.05f + 1.0f;
        //float YC1 = 1.0f;//Y1 * 0.50f + 1.0f;
        //float YC2 = 1.0f;//Y2 * 0.50f + 1.0f;
        //float YC3 = 0.0f;//Y3 * 0.05f + 1.0f;

    float U=(U1+U2)/2.0f;
    float V=(V1+V2)/2.0f;

//    float U=(U0*YC0 + U1*YC1 + U2*YC2 + U3*YC3)/(YC0+YC1+YC2+YC3);
//    float V=(V0*YC0 + V1*YC1 + V2*YC2 + V3*YC3)/(YC0+YC1+YC2+YC3);

    // recalculate

//    Y1 = yfind(U,V,R1,G1,B1);
//    Y2 = yfind(U,V,R2,G2,B2);

//	Y1 = ((B1 - U1/0.493f) + (R1 - V1/0.877f))/2.0f;
//	Y2 = ((B2 - U2/0.493f) + (R2 - V2/0.877f))/2.0f;

	*(pDst + 0) = ROUNDCLAMPINT(Y1);
	*(pDst + 1) = ROUNDCLAMPINT(U);
	*(pDst + 2) = ROUNDCLAMPINT(Y2);
	*(pDst + 3) = ROUNDCLAMPINT(V);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// handles YUY2 encoding and swizzling, refpack handled elsewhere.
static int SHPWWriteYUY2(const GINFO * ginfo, int rowbytes, const char * infileptr, char * outfileptr)
{
	int width  = ginfo->width;
	int height = ginfo->height;
	int drowbytes = ((width+1)&~1)*2;
	int datalen = drowbytes*height;

    bool isodd = (width&1)?true:false;
    /* encode YUY2 data */

    for (int y=0; y<height; y++)                                   // note do a square of 2 pixels at a time.
    {
    	unsigned char * d = (unsigned char *) outfileptr+y*drowbytes;
        unsigned char * s0;
        unsigned char * s1 = (unsigned char *) infileptr+y*rowbytes;
        unsigned char * s2;
        unsigned char * s3;

        for (int x=0; x<width; x+=2)                               // loop over scanline
        {
            s0 = x?s1-4:s1;
            s2 = x<(width-1)?s1+4:s1;
            s3 = x<(width-2)?s1+8:s2;

			argb32toyuy2( d, s0,s1,s2,s3 );

            if (isodd && x == width-1)
            {
			    d  += 4;  // advance by one 32-bit yuy2 pixel
                s1 += 4;  // advance by 2 32-bit source pixels
            }
            else
            {
			    d  += 4;  // advance by one 32-bit yuy2 pixel
                s1 += 8;  // advance by 2 32-bit source pixels
            }
        }
    }

    return(datalen);                                           // return the actual image size for storage purposes.
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert from RGB to YUV format
static void rgbtoyum(YUMCOORD *rgb, YUMCOORD *yum)
{
    int Red;
    int Green;
    int Blue;

    Red   = rgb->x;
    Green = rgb->y;
    Blue  = rgb->z;

    yum->x = ROUNDCLAMPINT((float)(( 0.25678843 * Red + 0.50412965 * Green + 0.09790586 * Blue) +  16.0f));
    yum->y = ROUNDCLAMPINT((float)((-0.14822295 * Red - 0.29099280 * Green + 0.43921576 * Blue) + 128.0f));
    yum->z = ROUNDCLAMPINT((float)(( 0.43921557 * Red - 0.36778826 * Green - 0.07142731 * Blue) + 128.0f));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// not currently used for much as it's crippled for consistent behaviour right now. But an interesting
// feature, possibly used later.
static int YUMQuant(int v,int q)
{
    if (v<0)
    {
        v = -((-v+q/2)/q*q);
    }
    else if (v>0)
    {
        v = (v+q/2)/q*q;
    }
    return(v);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// encode the Y channel as 8bit representation, do 4 pixels at a time.
static unsigned char *YEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3, int quant, int width)
{
    YUMCOORD rgb;
    YUMCOORD yum[4];

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyum(&rgb,&yum[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyum(&rgb,&yum[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyum(&rgb,&yum[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyum(&rgb,&yum[3]);

    d[0] = gmax(0,gmin(255,YUMQuant(yum[0].x,quant)));
    d[1] = gmax(0,gmin(255,YUMQuant(yum[1].x,quant)));

    d[width+0] = gmax(0,gmin(255,YUMQuant(yum[2].x,quant)));
    d[width+1] = gmax(0,gmin(255,YUMQuant(yum[3].x,quant)));

    return(d+2);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// encode the blue YUV data.
static unsigned char *CbEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3, int quant)
{
    YUMCOORD rgb;
    YUMCOORD yum[4];
    int uavg;
    int ucount;
    int uweight;

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyum(&rgb,&yum[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyum(&rgb,&yum[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyum(&rgb,&yum[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyum(&rgb,&yum[3]);

    uavg = ucount = 0;
    uweight = (yum[0].x+50)/64; uavg += yum[0].y*uweight; ucount += uweight;
    uweight = (yum[1].x+50)/64; uavg += yum[1].y*uweight; ucount += uweight;
    uweight = (yum[2].x+50)/64; uavg += yum[2].y*uweight; ucount += uweight;
    uweight = (yum[3].x+50)/64; uavg += yum[3].y*uweight; ucount += uweight;
    if (!ucount) ++ucount;
    uavg = uavg/ucount;

    *d++ = gmax(0,gmin(255,YUMQuant(uavg,quant)));

    return(d);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// encode the red YUV data as effective 2bit values (really one value for 4 pixels 1byte)
static unsigned char *CrEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3,int quant)
{
    YUMCOORD rgb;
    YUMCOORD yum[4];
    int uavg;
    int ucount;
    int uweight;

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyum(&rgb,&yum[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyum(&rgb,&yum[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyum(&rgb,&yum[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyum(&rgb,&yum[3]);

/*    vavg = (yum[0].z+yum[1].z+yum[2].z+yum[3].z)\4; */

    uavg = ucount = 0;
    uweight = (yum[0].x+50)/64; uavg += yum[0].z*uweight; ucount += uweight;
    uweight = (yum[1].x+50)/64; uavg += yum[1].z*uweight; ucount += uweight;
    uweight = (yum[2].x+50)/64; uavg += yum[2].z*uweight; ucount += uweight;
    uweight = (yum[3].x+50)/64; uavg += yum[3].z*uweight; ucount += uweight;
    if (!ucount) ++ucount;
    uavg = uavg/ucount;


    *d++ = gmax(0,gmin(255,YUMQuant(uavg,quant)));

    return(d);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// swizzle each plane seperatly as if it was 8bit source.
static void DoYUVSwizzle(char * outfileptr,int Width,int Height,int ShapeType,int type,int ShapeRowBytes)
{
    //static char * SwizzleAll(unsigned char * MemPtr,int Width, int Height,int Shapebpp,int Membpp,int ShapeType,int type,int ShapeRowBytes)
    unsigned char * yumbuf;
    unsigned char * buf2;
    unsigned char * buf3;

    yumbuf = (unsigned char *)SwizzleAll((unsigned char *)outfileptr,Width,Height,8,ShapeType,type,Width);
    if (yumbuf)
    {

        buf2 = (unsigned char *)SwizzleAll((unsigned char *)(outfileptr+(Width*Height)),Width/2,Height/2,8,ShapeType,type,Width/2);
        if (buf2)
        {
            buf3 = (unsigned char *)SwizzleAll((unsigned char *)(outfileptr+(Width*Height)+(Width*Height/4)),Width/2,Height/2,8,ShapeType,type,Width/2);
            if (buf3)
            {
                    memcpy(outfileptr,yumbuf,Width*Height);
                    memcpy(outfileptr+(Width*Height),buf2,Width*Height/4);
                    memcpy(outfileptr+(Width*Height)+(Width*Height/4),buf3,Width*Height/4);
                    if (yumbuf) gfree(yumbuf);
                    if (buf2)   gfree(buf2);
                    if (buf3)   gfree(buf3);
            }
            else
            {
                gfree(yumbuf);
                gfree(buf2);
                yumbuf = 0;
            }
        }
        else
        {
            gfree(yumbuf);
            yumbuf = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// handles YUV encoding and swizzling, refpack handled elsewhere.
static int SHPWWriteYUV(const GINFO * ginfo, int rowbytes, const char * infileptr, char * outfileptr,int YQuant,int CbQuant,int CrQuant,int subtype,int type,int ShapeType,int ShapeRowBytes,int rw,int rh)
{
    int width  = ginfo->width;
    int height = ginfo->height;
    int x;
    int y;
    unsigned char *s1;
    unsigned char *s2;
    unsigned char *d;
    int datalen = (int)((float)width*(float)height*3.0/2.0);
    //char * infileptr = infilep;


    if (type==TYPEGSH && subtype & SUBSWIZZLE)
    {
        datalen = (int)((float)rw*(float)rh*3.0/2.0);
    }
    d = (unsigned char *) outfileptr;

    //width =  width;
    //height = height;
    /* encode Y channel */
    for (y=0; y<height-1; y+=2)                                // note do a square of 4 pixels at a time.
    {
        s1 = (unsigned char*)infileptr+y*rowbytes;             // grab both source points
        s2 = s1+rowbytes;
        for (x=0; x<width; x+=2)                               // loop over scanline
        {
            d = YEncode(d,s1,s1+4,s2,s2+4,YQuant,width);       // encode the Y information using standard YUV conversions.
            s1 += 2*4;                                         // skip one pixle each time.
            s2 += 2*4;
        }
        if (type==TYPEGSH && subtype & SUBSWIZZLE)
            d+= rw;
        else
            d+= width;
    }

    /* encode Cb channel */

    for (y=0; y<height-1; y+=2)                                // similar to above.
    {
        s1 = (unsigned char*)infileptr+y*rowbytes;
        s2 = s1+rowbytes;
        for (x=0; x<width; x+=2)
        {
            d = CbEncode(d,s1,s1+4,s2,s2+4,CbQuant);
            s1 += 2*4;
            s2 += 2*4;
        }
    }

    /* encode Cr channel */

    for (y=0; y<height-1; y+=2)                                // similar to above.
    {
        s1 = (unsigned char*)infileptr+y*rowbytes;
        s2 = s1+rowbytes;
        for (x=0; x<width; x+=2)
        {
            d = CrEncode(d,s1,s1+4,s2,s2+4,CrQuant);
            s1 += 2*4;
            s2 += 2*4;
        }
    }
    if (subtype & SUBSWIZZLE)                                                     // handle swizzling of data.
    {
        if (type == TYPEGSH)
            DoYUVSwizzle(outfileptr,rw,rh,ShapeType,type,ShapeRowBytes);         // swizzles right into data buffer.
        else
            DoYUVSwizzle(outfileptr,width,height,ShapeType,type,ShapeRowBytes);  // swizzles right into data buffer.
    }
    return(datalen);                                                             // return the actual image size for storage purposes.
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Write S3 compressed texture. 4x4 block (can be swizzled GSH have this) effectivly 16bpp
static int SHPWWriteDXT(const GINFO *ginfo, int rowbytes, const char *source, char *dest, int type, int collapse)
{
    unsigned char *ordertemp;
    unsigned int realwidth;
    unsigned int realheight;
    int datalen;
    int options[1] = {0};
    int packlen=0;

    // if game cube then we must calculate padding for block size.
    //realwidth  = ((ginfo->width +15)&~15);   // we need a row not only a multiple of 4
    //realheight = ((ginfo->height+15)&~15);   // but with 4 blocks on it so pad the extra bits.
    realwidth  = ((ginfo->width +7)&~7);   // we need a row not only a multiple of 4
    realheight = ((ginfo->height+7)&~7);   // but with 4 blocks on it so pad the extra bits.

    if (type == TYPEGSH)
        datalen = ((ginfo->width+7)&~7) * ((ginfo->height+7)&~7) * 4;
    else
        datalen = ((ginfo->width+3)&~3) * ((ginfo->height+3)&~3) * 4;

    ordertemp = (unsigned char *) galloc(datalen+8);
    if (ordertemp)
    {
        memset(ordertemp,0,datalen+8);
        if (ginfo->alphabits > 1 && ginfo->alphabits != 3) // DXT 5 is 3565
        {
            /* DXT3 */
            options[0] = DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_EXPLICIT;
        }
        else if (ginfo->alphabits == 3)
        {
            /* DXT5 */
            options[0] = DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_INTERPOLATED;
        }
        else if (ginfo->alphabits == 1)
        {
            /* DXT1 */
            options[0] = DXT_ENCODE_RGB_ALPHA_COMPARE;
        }
        else
        {
            /* DXT1 */
            options[0] = DXT_ENCODE_RGB_FULL;
        }
        // force user to use 565 type now for PS2.
        //if (type==TYPESSH)
        //    options[0] |= DXT_ENCODE_RGB_555;

        if (type == TYPEGSH)
        {
            if (ginfo->alphabits > 0)  // include alpha information in the encode, slower but more accurate.
                /* DXT1 */
                options[0] = DXT_ENCODE_RGB_ALPHA_COMPARE| DXT_DXT1B_ENCODEOPTION;
            else
                /* DXT1 */
                options[0] = DXT_ENCODE_RGB_FULL| DXT_DXT1B_ENCODEOPTION;
//            SHPWReorderDXTGSHBlocks(realwidth,realheight,ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, (unsigned char *)ordertemp);
        }
//        else
            SHPWReorderDXTBlocks(ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, (unsigned char *)ordertemp, type);

        if (ginfo->packed==3) // S3TC (s3 code)
            packlen = SHPALIGN(S3TC_encode(dest,ordertemp,datalen,options));
        else    // DXT (ea code)
            packlen = SHPALIGN(DXT_encode(dest,ordertemp,datalen,options));

        gputm(((DXTHEADER*)dest)->size, ginfo->width*ginfo->height*sizeof(unsigned short), sizeof(((DXTHEADER*)dest)->size));
        gputm(((DXTHEADER*)dest)->reserved, ginfo->width, sizeof(((DXTHEADER*)dest)->reserved));
        gfree(ordertemp);

        if (collapse)
        {
            packlen -= 8;
            memcpy(dest,dest+8,packlen);
			memset(dest+packlen,0,8);
        }
    }
    return(packlen);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Write MG compressed texture
static int SHPWWriteMG(const GINFO *ginfo, int rowbytes, const char *source, char *dest)
{
    int flags;
    int alphatype;
    int quant;
    int mgsize;

    flags = 0;
    alphatype = 0;
    if (ginfo->alphabits>=1)
    {
        flags = IDECFLAG_HASALPHA;
        alphatype = IDEC_ALPHA4BITRAW;

        if (ginfo->alphabits>4)
            alphatype = IDEC_ALPHA8BITRAW;
    }
    quant = gmax(1,gmin(17,(int)((100-ginfo->quality)/6.25f+1.5f)));

	mgsize = IDEC_encode( (ARGB *)source, (unsigned char *)dest,ginfo->width,ginfo->height,rowbytes,flags,quant, alphatype);
    return(mgsize);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// intlog2: returns integer log to the base 2 of a number.
static int SHPWintlog2(unsigned int n)
{
    int msb=0;

    if (n & 0xffff0000)
    {
        msb += 16;
        n &= 0xffff0000;
    }
    if (n & 0xff00ff00)
    {
        msb += 8;
        n &= 0xff00ff00;
    }
    if (n & 0xf0f0f0f0)
    {
        msb += 4;
        n &= 0xf0f0f0f0;
    }
    if (n & 0xcccccccc)
    {
        msb += 2;
        n &= 0xcccccccc;
    }
    if (n & 0xaaaaaaaa)
    {
        msb += 1;
    }

    return(msb);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out encodeing type and call routine that actually encodes and stuffs the buffer with
// the formatted data.
static int SHPWSetShape(SHPHeaderStruct *HeaderPict,const GINFO *ginfo,int bpp,const char *source,int rowbytes,int noshp,int type,int isI,int isIA)
{
    int shplen;
    int packlen;
    int refpacklen;
    int realwidth;
    int realheight;
    /* int kpnpacklen; */
    void *packtemp;
    int shptype;
    int subtype=ginfo->subtype;
    int dxt = 0;
    int mg = 0;
    int collapse = !(ginfo->subtype&SUBNODXTCOLLAPSE);
    int gshembm  = type==TYPEGSH && ginfo->subtype&SUBEMBM;
    memset(HeaderPict,0,sizeof(SHPHeaderStruct));

	bool dxtenable=false;
	if (type==TYPEXSH || type==TYPEFSH || type==TYPEGSH || type==TYPEZSH || type==TYPEYSH || (subtype & SUBADVANCED))
		dxtenable = true;

    /* Special case for DXT encoding */
    if ( dxtenable && (ginfo->packed == 3 || ginfo->packed == 5) && (bpp > 8) && (ginfo->bpp > 8) && !isIA && !isI)
    {
        if (ginfo->alphabits >= 4)      // DXT3
            dxt = 3;
        else if (ginfo->alphabits > 1)  // DXT5
            dxt = 5;
        else
            dxt = 1;

        bpp = 16;
    }
    // Special case for MG encoding
	// Has to be a multiple of 16x16
    if (  (ginfo->packed == 4)    && (bpp > 8)    &&
		 ((ginfo->width+15)&~15)  == ginfo->width &&
		 ((ginfo->height+15)&~15) == ginfo->height)
    {
        bpp = 16;
        if (ginfo->redbits>5 || ginfo->greenbits>6 || ginfo->greenbits>5)  // more than 16 bit
            bpp = 24;
        if (ginfo->alphabits>1)       // has alpha channel
            bpp = 32;
        mg = 1;
    }


    if (type==TYPEPSH) /* psh */
    {
        switch (bpp)
        {
            default:
            case 4:  shptype=0x40; break;
            case 8:  shptype=0x41; break;
            case 32:
            case 15:
            case 16: shptype=0x42; break;
            case 24: shptype=0x43; break;
        }
    }
    else if (type==TYPESSH) /* ssh */
    {
        switch (bpp)
        {
            default:
            case 4:  shptype=0x01; break;
            case 8:  shptype=0x02; break;
            case 15:
            case 16: if (ginfo->alphabits > 2)   // possibly support 3 level transparency so 2 bits of accuracy here.
                         shptype=0x05;          // set to 32bpp mode if this is the case.
                     else if (ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                         shptype=0x6c;          // YUV (YUI) format
                     else
                         shptype=0x03;          // else set to 16bpp mode.
                     break;
            case 24: shptype=0x04; break;
            case 32: shptype=0x05; break;
        }
        if (bpp == 24 && (subtype & SUBADVANCED))
        {
            shptype=0x04;
        }
    }
    else if (type==TYPEASH) /* ash */
    {
        switch (bpp)
        {
            default:
            case 4:  shptype=0x01; break;
            case 8:  shptype=0x02; break;
            case 15:
            case 16: shptype=0x03; break;
        }
    }
    else if (type==TYPEGSH) /* gsh */
    {
        if ((gshembm || isIA == 8))
            shptype=0x13;     // set to IA8 format for embm maps and gsh.
        else if ((isIA == 4))
            shptype=0x12;     // set to IA4 format.
        else if ((isI == 8)&&(subtype&SUBADVANCED))
            shptype=0x11;     // set to I8  format.
        else if ((isI == 4)&&(subtype&SUBADVANCED))
            shptype=0x10;     // set to I4  format.
        else
        {
            switch (bpp)
            {
                default:
                case 4:  shptype=0x18; break;
                case 8:  shptype=0x19; break;
                case 15:
                case 16:
                    if (ginfo->alphabits>=1 && !dxt) // default to 5a3 if we have any alpha since all other formats don't keep alpha.
                        shptype=0x15;   /* 5a3 (3444) */
                    else if (ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                        shptype=0x1c;   // YUV (YUM) format.
                    else
                        shptype=0x14;   /* 0565 */
                    break;
                case 24:
                case 32: shptype=0x16;  /* 8888 */
            }
        }
    }
    else if (type==TYPEMSH) /* msh */
    {
        switch (bpp)
        {
            default:
            case 4:  shptype=0x5c; break;
            case 8:  shptype=0x5d; break;
            case 15:
            case 16:
                if (ginfo->alphabits>=4 && !dxt)
                    shptype=0x5a;   /* 4444 */
                else if (ginfo->greenbits>=6 || dxt)
                    shptype=0x58;   /* 565 */
                else
                    shptype=0x59;   /* 1555 */
                break;
            case 32: shptype=0x5b; break;
        }
    }
    else if (type==TYPEXSH || type==TYPEZSH || type==TYPEYSH) /* xsh */
    {
        switch (bpp)
        {
            default:
            case 8:  shptype=0x7b; break;
            case 15:
            case 16:
                if (ginfo->alphabits>=4 && !dxt)
                    shptype=0x6d;   /* 4444 */
                else if (ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                    shptype=0x6c;   // YUV (YUI) format.
                else if (ginfo->alphabits == 0 && ginfo->redbits == 4 && ginfo->greenbits == 8 && ginfo->bluebits == 4)
                    shptype=0x68;   // YUY2 format.
                else if (ginfo->greenbits>=6 || dxt)
                    shptype=0x78;   /* 565 */
                else if (ginfo->redbits>=6)
                    shptype=0x67;   /* 655 */
                else
                    shptype=0x7e;   /* 1555 */
                break;
            case 24: shptype=0x7f; break;
            case 32: shptype=0x7d; break;

        }

        if (isIA == 8)
            shptype=0x65;   /* IA8 */
        else if ((isI == 8) && ginfo->alphabits)
            shptype=0x64;   /* A8  */
        else if ((isI == 8) && !ginfo->alphabits)
            shptype=0x63;   /* L8  */

    }
    else    /* fsh or vsh */
    {
        switch (bpp)
        {
            default:
            case 4:  shptype=0x79; break;  // 7a is hi/lo
            case 8:  shptype=0x7b; break;
            case 15:
            case 16:
                if (ginfo->alphabits>=4 && !dxt)
                    shptype=0x6d;   /* 4444 */
                else if (ginfo->alphabits>=3 && !dxt && ginfo->subtype&SUBADVANCED)
                    shptype=0x15;   /* 5a3 (3444) */
				else if (ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                    shptype=0x6c;   // YUV (YUI) format.
                else if (ginfo->alphabits == 0 && ginfo->redbits == 4 && ginfo->greenbits == 8 && ginfo->bluebits == 4)
                    shptype=0x68;   // YUY2 format.
                else if (ginfo->greenbits>=6 || dxt)
                    shptype=0x78;   /* 565 */
                else if (ginfo->redbits>=6)
                    shptype=0x67;   /* 655 */
                else
                    shptype=0x7e;   /* 1555 */
                break;
            case 24:
                if (ginfo->alphabits>=6 && ginfo->subtype&SUBADVANCED)
                    shptype=0x66;   /* 6666 */
                else
                    shptype=0x7f;
                break;
            case 32:
                if (ginfo->alphabits<=2 && ginfo->subtype&SUBADVANCED)
                    shptype=0x6a;   /* 2101010 */
                else
                    shptype=0x7d; break;
                break;
        }
        if (isIA == 8)
            shptype=0x65;   /* IA8 */
        else if ((isI == 8) && ginfo->alphabits)
            shptype=0x64;   /* A8  */
        else if ((isI == 8) && !ginfo->alphabits)
            shptype=0x63;   /* L8  */
	}

    if (dxt && collapse)
    {
        if (type==TYPEGSH)
            shptype = 0x1e; // CMPR (DXT1)
        else if (type==TYPEXSH || type==TYPEFSH || type==TYPESSH || type==TYPEASH || type==TYPEZSH || type==TYPEYSH)
        {
            switch (dxt)
            {
                default:
                case 1:  shptype=0x60; break; // DXT1
                case 3:  shptype=0x61; break; // DXT3
                case 5:  shptype=0x62; break; // DXT5
            }
        }
        else
            collapse = 0;
    }


    {
        int width  = ginfo->width;
        int height = ginfo->height;
        int shapex = ginfo->defaultx;
        int shapey = ginfo->defaulty;
        int flags  = ELEMENT_FLAG_NEW_FORMAT; // by default mark as new shape

        if (subtype & SUBSWIZZLE)
        {
            if (type==TYPEXSH || type==TYPEFSH || type==TYPEZSH || type==TYPEYSH)
            {
                if ((width!=(1<<SHPWintlog2(width)))
                 || (height!=(1<<SHPWintlog2(height))))
                    subtype &= ~SUBSWIZZLE;
            }
            else if (type==TYPESSH)
            {
                if (!CanSwizzleSsh(width,height,bpp)
                 || (bpp>=24)   // disable swizzle on 24/32 bpp
#if SWIZZLE4BUG
                 || (bpp<=4 && width!=height)
#endif
                )   // disable swizzle on 4 bpp due to issue with mipmap collapse
				{
                    subtype &= ~SUBSWIZZLE;
				}

                // calculate for lower than 16x16 or 32x32 sized textures realwidth and realheight.
                RetSSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
            }
            else if (type==TYPEMSH && !dxt)
            {
                RetMSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
            }
            else if (type==TYPEGSH && !dxt)
            {
                RetGSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
            }
            else
            {
                subtype &= ~SUBSWIZZLE; // disable swizzle on platforms that do not support it
            }
        }
        // dxt textures for gsh are considered swizzled since their blocks are
		// swizzled by default, turn on swizzle bit to be nice to EAGL.
		if (dxt)
        {
            if (type==TYPEGSH)
    			subtype |= SUBSWIZZLE;
            else
                subtype &= ~SUBSWIZZLE;
        }

        if ((bpp==24) // disable swizzle on 24 bpp
         || (bpp==4 || shptype==0x68) && (type==TYPEXSH || type==TYPEFSH || type==TYPEZSH || type==TYPEYSH))
            subtype &= ~SUBSWIZZLE; // disable swizzle on 4 bpp & YUY2

        if (subtype & SUBTRANSPOSE)   flags  |= TEXTURE_FLAG_TRANSPOSED;
        if (subtype & SUBSWIZZLE)     flags  |= TEXTURE_FLAG_SWIZZLED;
        if (subtype & SUBOPAQUE)      flags  |= TEXTURE_FLAG_OPAQUE;
        if (subtype & SUBTRANSPARENT) flags  |= TEXTURE_FLAG_TRANSPARENT;
        if (subtype & SUBHALFALPHA)   flags  |= TEXTURE_FLAG_HALF_ALPHA;
        if (subtype & SUBEMBM)        flags  |= TEXTURE_FLAG_EMBM;
        if (subtype & SUBDOT3)        flags  |= TEXTURE_FLAG_DOT3;
        if (subtype & SUBCUBEMAP)     flags  |= TEXTURE_FLAG_CUBE_MAP;
        if (subtype & SUBMIPMONT)     flags  |= TEXTURE_FLAG_MIPMONT;

        SHPPUTFIELD(HeaderPict->type,shptype);
        SHPPUTFIELD(HeaderPict->flags,flags);
        SHPPUTFIELD(HeaderPict->width,width);
        SHPPUTFIELD(HeaderPict->height,height);
        SHPPUTFIELD(HeaderPict->shapex,shapex);
        SHPPUTFIELD(HeaderPict->shapey,shapey);
    }

    /* Special case for DXT encoding */
    if (dxt)
    {
        shplen = SHPWWriteDXT(ginfo, rowbytes, source, (char *)HeaderPict+sizeof(SHPHeaderStruct), type, collapse);
        if (!collapse)
            SHPPUTFIELD(HeaderPict->flags,(SHPGETFIELD(HeaderPict->flags)|ELEMENT_FLAG_COMPRESSED)); // mark as compressed
    }
    else if (shptype == 0x1c || shptype == 0x6c) // YUV format.
    {
        int quality = ginfo->quality;
        int YQuant  = (100-quality)*7/100+1;   /* 1..8 */
        int CbQuant = (100-quality)*12/100+2;  /* 2..14 */
        int CrQuant = (100-quality)*17/100+4;  /* 4..21 */
//        if (quality==100) // for now default to 100% every time.
        {
            YQuant  = 1;
            CbQuant = 1;
            CrQuant = 1;
        }
        shplen = SHPWWriteYUV(ginfo, rowbytes, source, (char *)HeaderPict+sizeof(SHPHeaderStruct),YQuant,CbQuant,CrQuant,subtype,type,shptype,rowbytes,realwidth,realheight);
    }
	else if (shptype == 0x68) // YUY2 format
		shplen = SHPWWriteYUY2(ginfo, rowbytes, source, (char *)HeaderPict+sizeof(SHPHeaderStruct));
    else if (mg)
    {
        shplen = SHPWWriteMG(ginfo, rowbytes, source, (char *)HeaderPict+sizeof(SHPHeaderStruct));
        SHPPUTFIELD(HeaderPict->flags,(SHPGETFIELD(HeaderPict->flags)|ELEMENT_FLAG_COMPRESSED)); // mark as compressed
    }
#ifdef __MSWINDOWS__
	else if (ginfo->packed > 1000 && MVCODEC_exists(ginfo))	// if compression greater than 1000, then it's a fourCC code
	{
		shplen = MVCODEC_encode(ginfo, rowbytes, (unsigned char *) source, HeaderPict);
		SHPPUTFIELD(HeaderPict->flags,(SHPGETFIELD(HeaderPict->flags)|ELEMENT_FLAG_COMPRESSED));

	}
#endif
	else
    {
        SHPWWriteBitmap(HeaderPict,(char *)HeaderPict+sizeof(SHPHeaderStruct),ginfo,source,rowbytes,type,subtype,realwidth,realheight);
        if ((type == TYPEGSH || type == TYPESSH || type == TYPEMSH) && (subtype & SUBSWIZZLE))   // if I am working with a swizzled GSH then I have to work with fake sizes to accomodate padding.
            shplen = SHPWCalcSizeShape(realwidth,realheight,bpp,type,dxt);
        else
            shplen = SHPWCalcSizeShape(ginfo->width,ginfo->height,bpp,type,dxt);
    }

    if (ginfo->packed == 2)
    {
        packtemp = galloc((int)(shplen*1.2+1024));
        if (packtemp)
        {
            refpacklen = SHPALIGN(REF_encode(packtemp,(char *)HeaderPict+sizeof(SHPHeaderStruct),shplen,0));
            packlen = refpacklen;

            if (packlen<shplen)
            {
                memcpy((char *)HeaderPict+sizeof(SHPHeaderStruct),packtemp,packlen);
                shplen = packlen;
                SHPPUTFIELD(HeaderPict->flags,(SHPGETFIELD(HeaderPict->flags)|ELEMENT_FLAG_COMPRESSED)); // mark as compressed
            }
            gfree(packtemp);
        }
    }

    SHPPUTFIELD(HeaderPict->DataOffset,sizeof(SHPHeaderStruct));
    SHPPUTFIELD(HeaderPict->DataSize,shplen);

    shplen = SHPALIGN(sizeof(SHPHeaderStruct)+shplen);
    return(shplen);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// encode the clut in the correct format and stuff the buffer with the clut.
static void SHPWSetShapeClut(const GINFO *ginfo,int NbColours,const ARGB *colourtbl,CLUTHeaderStruct **theClut,int *SizeClut,int noclut,int type)
{
    int  i       = 0;
    int  j       = 0;
    int  csm1    = 0;
    int  cluttype= 0;
    char *pal    = 0;
    int SizeData = 0;
    CLUTHeaderStruct *DataClut=0L;

    if (NbColours>0 && !noclut)
    {
        SizeData = NbColours;
        switch (type)
        {
            case TYPEPSH:     /* PSH */
                cluttype = 0x23;
                SizeData *= 2;      /* 16-bit palette */
                break;

            case TYPEGSH:     /* GSH */
            {
                if (ginfo->subtype&SUBADVANCED)
                    cluttype=gshadvancedcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                else
                    cluttype=gshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);

                if (cluttype==0x33)
                    SizeData = ((SizeData*2+31)&~31)+SizeData*2;  /* 32 bit palette of 2 IAs, with 32 byte alignment for 2nd clut */
                else
                    SizeData *= 2;  /* 16 bit palette */
                break;
            }

            case TYPEMSH:     /* MSH */
            {
                cluttype=mshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                if (cluttype==0x38 || /* 565  */
					cluttype==0x39 || /* 1555 */
					cluttype==0x3a)   /* 4444 */
                    SizeData *= 2;  /* 16 bit palette */
                else /* type 3b */
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPESSH:     /* SSH */
            {
                cluttype=sshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                if (ginfo->originalbpp>4)
                {
                    if (!(ginfo->subtype&SUBCSM2))
                        csm1 = 1;
                    SizeData = SHPWcsmsize(SizeData);
                }
                if (cluttype==0x20)
                    SizeData *= 2;  /* 16 bit palette */
                else /* type 21 */
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEASH:     /* ASH */
            {
                cluttype=0x20;
                SizeData *= 2;  /* 16 bit palette */
                break;
            }

            case TYPEXSH:     /* XSH */
			case TYPEZSH:	/* Xenon */
			case TYPEYSH:  /*PS3*/
            {
                cluttype=xshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                if (cluttype==0x29 || cluttype==0x2d)
                    SizeData *= 2;  /* 16 bit palette */
                else
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEFSH:     /* FSH */
            {
                if (ginfo->subtype&SUBADVANCED)
                    cluttype=fshadvancedcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                else
                    cluttype=fshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);

                if (cluttype==0x29 || cluttype==0x2d)
                    SizeData *= 2;  /* 16 bit palette */
                else if (cluttype==0x22 || cluttype==0x24 || cluttype==0x2f)
                    SizeData *= 3;  /* 24 bit palette */
                else
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEVSH:     /* VSH */
                cluttype = 0x22;
                SizeData *= 3;      /* 24-bit palette */
                break;

            case TYPENSH:     /* NSH */
            {
                cluttype = 0x27;

                /* Intensity or intensity/alpha shapes have
                   no CLUT. If all colours in the palette are
                   grey scale then assume this is the case. */

                if (NSHWIsGreyPalette(colourtbl, NbColours, ginfo->subtype&SUBNOPAL))
                {
                    SizeData = 0;
                }
                else
                {
                        /* Otherwise use 16-bit RGBA palette. */
                    SizeData *= 2;
                }
                break;
            }
        }

        if (SizeData)
        {
            SizeData = SHPALIGN(sizeof(CLUTHeaderStruct)+SizeData);
            DataClut = (CLUTHeaderStruct*) galloc(SizeData);
        }

        if (DataClut)
        {
            int flags = 1;
            if (csm1 && type==TYPESSH)
                flags |= TEXTURE_FLAG_SWIZZLED;
            memset(DataClut,0,SizeData);
            SHPPUTFIELD(DataClut->type,     cluttype);
            SHPPUTFIELD(DataClut->flags,flags); // new shape
            SHPPUTFIELD(DataClut->DataOffset,sizeof(CLUTHeaderStruct));
            SHPPUTFIELD(DataClut->DataSize,SizeData-sizeof(CLUTHeaderStruct));
            SHPPUTFIELD(DataClut->width,    NbColours);
            SHPPUTFIELD(DataClut->height,   1);
//            SHPPUTFIELD(DataClut->centerx,  NbColours);
//            SHPPUTFIELD(DataClut->centery,  0);

            pal = (char *) DataClut + sizeof(CLUTHeaderStruct);

            for (j=0; j<NbColours; ++j)
            {
                i = (csm1 && (j&31)>=8 && (j&31)<=23) ? j ^ 0x18 : j;
                switch (type)
                {
                    case TYPEPSH: /* PSH */
                        gputi(pal+i*2,PSHWpixel8To5(&colourtbl[j]),2);
                        break;

                    case TYPEFSH: /* FSH */
                    case TYPEXSH: /* XSH */
					case TYPEZSH: /* Xenon */
					case TYPEYSH: /*PS3*/
                    case TYPEGSH: /* GSH */
                    case TYPESSH: /* SSH */
                    case TYPEASH: /* ASH */
                    case TYPEMSH: /* MSH */
                        switch (cluttype)
                        {
                            case 0x20: /* SSH/ASH 15 bit palette (1555) */
                                gputi(pal+i*2,SSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x32: /* GSH 15 bpp */
                                gputm(pal+i*2,GSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x2d: /* FSH 15 bit palette (1555) */
                                gputi(pal+i*2,FSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x38: /* MSH 16 bit palette (565) */
                                gputi(pal+i*2,MSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x39: /* MSH 15 bit palette (1555) */
                                gputi(pal+i*2,SSHWpixel8To5(&colourtbl[j]),2);
                                break;

							case 0x3a: /* MSH 16 bit palette (4444) */
                                gputi(pal+i*2,MSHWpixel8To4444(&colourtbl[j]),2);
                                break;

                            case 0x31: /* GSH 565 */
                                gputm(pal+i*2,FSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x29: /* FSH 16 bit palette (565) */
                                gputi(pal+i*2,FSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x30: /* GSH IA8 */
                                gputm(pal+i*2,(int)GSHWpixel8ToIA(&colourtbl[j]),2);
                                break;

                            case 0x33: /* GSH 32 bit palette (8888) as 2 IA cluts */
                                pal[i*2+0] = colourtbl[j].a;
                                pal[i*2+1] = colourtbl[j].r;
                                pal[i*2+((NbColours*2+31)&~31)+0] = colourtbl[j].g;
                                pal[i*2+((NbColours*2+31)&~31)+1] = colourtbl[j].b;
                                break;

                            case 0x22: /* FSH 24 bit palette (666) */
                                pal[i*3+0] = colourtbl[j].r>>2;
                                pal[i*3+1] = colourtbl[j].g>>2;
                                pal[i*3+2] = colourtbl[j].b>>2;
                                break;

                            case 0x24: /* OLDFSH 24 bit palette (888) */
                                pal[i*3+0] = colourtbl[j].r;
                                pal[i*3+1] = colourtbl[j].g;
                                pal[i*3+2] = colourtbl[j].b;
								break;

                            case 0x2f: /* FSH 24 bit palette (888) */
                                pal[i*3+0] = colourtbl[j].b;
                                pal[i*3+1] = colourtbl[j].g;
                                pal[i*3+2] = colourtbl[j].r;
                                break;


                            case 0x2e: /* FSH 32 bit palette (8555) */
                                gputi(pal+i*4,FSHWpixel8To5(&colourtbl[j]),2);
                                pal[i*4+2] = 0;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x2c: /* FSH 32 bit palette (8565) */
                                gputi(pal+i*4,FSHWpixel8To565(&colourtbl[j]),2);
                                pal[i*4+2] = 0;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x2a: /* FSH 32 bit palette (8888) */
                                pal[i*4+0] = colourtbl[j].b;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].r;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x3b: /* MSH 32 bit palette (8888) */
                                pal[i*4+0] = colourtbl[j].r;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].b;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x21: /* SSH 8888 */
                                pal[i*4+0] = colourtbl[j].r;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].b;
                                pal[i*4+3] = colourtbl[j].a;
                                break;
                        }
                        break;

                    case TYPEVSH: /* VSH */
                        pal[i*3+0] = colourtbl[j].r>>2;
                        pal[i*3+1] = colourtbl[j].g>>2;
                        pal[i*3+2] = colourtbl[j].b>>2;
                        break;

                    case TYPENSH: /* NSH */
                        gputm(pal+i*2,NSHWpixel8To5(&colourtbl[j]),2);
                        break;
                }
            }
        }
    }
    *SizeClut=SizeData;
    *theClut=DataClut;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// stuff the shape hotspot buffer with requested values from the ginfo struct.
static void SHPWSetShapeHotSpot(const GINFO *ginfo,HOTHeaderStruct **theHotSpot,int *SizeHotSpot,int nohot, int type)
{
    HOTHeaderStruct  *DataHotSpot=0L;
    int SizeData=0L;
    int i;

    if ((ginfo->numhotspots>0 || ginfo->centerx || ginfo->centery) && !nohot)
    {
        SizeData=SHPALIGN(sizeof(HOTHeaderStruct)+ginfo->numhotspots*8); /* NumHotSpot*8 */
        DataHotSpot=(HOTHeaderStruct*)galloc(SizeData);
        if (DataHotSpot)
        {
            memset(DataHotSpot,0,SizeData);
            DataHotSpot->type=0x7c;
            SHPPUTFIELD(DataHotSpot->flags,1); // new shape

            SHPPUTFIELD(DataHotSpot->DataOffset,sizeof(HOTHeaderStruct));
            SHPPUTFIELD(DataHotSpot->DataSize,SizeData-sizeof(HOTHeaderStruct));

			SHPPUTFIELD(DataHotSpot->Dimension,2); // Number of values in a Hotspot (2)
			SHPPUTFIELD(DataHotSpot->NumHotSpot,ginfo->numhotspots);

            SHPPUTFIELD(DataHotSpot->centerx,ginfo->centerx);
            SHPPUTFIELD(DataHotSpot->centery,ginfo->centery);

            for (i=0; i<ginfo->numhotspots; ++i)
            {
                SHPPUTFIELD(((int *)((char *)DataHotSpot+sizeof(HOTHeaderStruct)))[i*2+0], ginfo->hotspottbl[i][0]);
                SHPPUTFIELD(((int *)((char *)DataHotSpot+sizeof(HOTHeaderStruct)))[i*2+1], ginfo->hotspottbl[i][1]);
            }
        }
    }
    *SizeHotSpot=SizeData;
    *theHotSpot=DataHotSpot;
}

// set the shape databin information. This holds opaque binary data
static void SHPWSetShapeDataBin(const GINFO *ginfo,DataHeaderStruct **theData,int *SizeDataBin, int type)
{
    int SizeData=0L;
    DataHeaderStruct *DataBin=0L;

     if (ginfo->size >= (int)sizeof(GINFOSHP) && ((GINFOSHP *)ginfo)->binsize > 0 &&
         (((ginfo->signature&0xffffff00)==GMAKEID('S','h','p',0)) ||
          ((ginfo->signature&0xffffff00)==GMAKEID('S','H','P',0))))
    {
        SizeData=sizeof(DataHeaderStruct) + ((GINFOSHP *)ginfo)->binsize;
        SizeData = SHPALIGN(SizeData);
        DataBin=(DataHeaderStruct*)galloc(SizeData);
        if (DataBin)
        {
            memset(DataBin,0,SizeData);
            DataBin->type=0x6b;
            // fill the space with something meaningful
            strncpy(((char *)DataBin)+sizeof(DataHeaderStruct),
                    "UserData Reserved Area",
                    ((GINFOSHP *)ginfo)->binsize);

            SHPPUTFIELD(DataBin->flags,1); // new shape
            {
                SHPPUTFIELD(DataBin->DataOffset,sizeof(MetalHeaderStruct));
                SHPPUTFIELD(DataBin->DataSize,((GINFOSHP*)ginfo)->binsize);
            }
        }
    }
    *SizeDataBin =SizeData;
    *theData     =DataBin;
}

// set the shape meta information. This hold information about extended types such as cubemap embm etc.
static void SHPWSetShapeMetal(const GINFO *ginfo,MetalHeaderStruct **theMetal,int *SizeMetal, int type)
{
    int SizeData=0L;
    MetalHeaderStruct *DataMetal=0L;
    int metalbinsize=64;

    if (type==TYPESSH)
        metalbinsize=240; // was 86;
    if (ginfo->subtype&SUBMETALBIN)
    {
        SizeData=sizeof(MetalHeaderStruct);
        if (type != TYPEGSH)
        {
            SizeData += metalbinsize; // eagl metalbin area
        }

        SizeData = SHPALIGN(SizeData);
        DataMetal=(MetalHeaderStruct*)galloc(SizeData);
        if (DataMetal)
        {
            memset(DataMetal,0,SizeData);
            DataMetal->type=0x69;
            if (type != TYPEGSH)
            {
                if (type==TYPESSH)
                    strcpy((char *)DataMetal+sizeof(MetalHeaderStruct),
                        "EAGL240 metal bin attachment for runtime texture management");
                else
                    strcpy((char *)DataMetal+sizeof(MetalHeaderStruct),
                        "EAGL64 metal bin attachment for runtime texture management");
            }
            SHPPUTFIELD(DataMetal->flags,1); // new shape and the metalbin flag
            if (type != TYPEGSH)
            {
                SHPPUTFIELD(DataMetal->DataOffset,sizeof(MetalHeaderStruct));
                SHPPUTFIELD(DataMetal->DataSize,metalbinsize);
            }
//            SHPPUTFIELD(DataMetal->metal,metal);
        }
    }
    *SizeMetal=SizeData;
    *theMetal=DataMetal;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// set the comment information for the current shape.
static void SHPWSetShapeComment(const GINFO *ginfo,COMMENTHeaderStruct **theComment,int *SizeComment,int nocomment, int type)
{
    COMMENTHeaderStruct *DataComment=0L;
    int len;
    int SizeData=0L;

    if (ginfo->comment[0] && !nocomment)
    {
        len = strlen(ginfo->comment);
        SizeData=SHPALIGN(sizeof(COMMENTHeaderStruct)+len+1);
        DataComment=(COMMENTHeaderStruct*)galloc(SizeData);
        if (DataComment)
        {
            memset(DataComment,0,SizeData);
            DataComment->type=0x6f;
            SHPPUTFIELD(DataComment->flags,1);  // new shape
            SHPPUTFIELD(DataComment->DataOffset,sizeof(COMMENTHeaderStruct));
            SHPPUTFIELD(DataComment->DataSize,len+1);
            strcpy((char *)DataComment+sizeof(COMMENTHeaderStruct),ginfo->comment);
        }
    }
    *SizeComment=SizeData;
    *theComment=DataComment;
}

static void SHPWSetShapeName(const GINFO *ginfo,char **theName, int framenum)
{
    int SizeData=strlen(ginfo->framename)+1;
    char *DataName = (char *) galloc(SizeData);
    if (DataName)
    {
        strcpy(DataName,ginfo->framename);
    }
    *theName = DataName;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// general function that fills all of the shape nexts with the apropriate information.
int SHPWAddShape(ShapeListPtr theShape,const GINFO *tginfo, const char *source, int rowbytes, int framenum, int type, int subtype)
{
    SHPHeaderStruct     * PictureShape = 0;
    CLUTHeaderStruct    * ClutShape    = 0;
    MetalHeaderStruct   * MetalShape   = 0;
    DataHeaderStruct    * DataBinShape = 0;
    HOTHeaderStruct     * HotSpotShape = 0;
    COMMENTHeaderStruct * CommentShape = 0;
    char *NameShape                    = 0;
    int  bpp                           = 0;
    int  i                             = 0;
    int  ok                            = 0;
    int SizeShape                      = 0;
    int SizeClut                       = 0;
    int SizeMetal                      = 0;
    int SizeDataBin                    = 0;
    int SizeHotSpot                    = 0;
    int SizeComment                    = 0;
    int isIA                           = 0;
    int isI                            = 0;
    int nopalettevar                   = 0;
    int dxt                            = 0;
    char shapeid[8];
    char clutid[8];

	// have to make a copy of the ginfo to work with. This allows me to
	// modify the clut by compression functions for later export by
	// the regular shape export functions
    GINFO * ginfo                      = (GINFO *)galloc(tginfo->size);
    memcpy(ginfo,tginfo,tginfo->size);

	// validate shape type and compression. Turn off GST compression
	// if the shape does not pass dimension and type checks.
	SHPGstValidate(ginfo,type);
    switch (type)
    {
        case TYPEPSH: /* PSH */
            bpp=4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=16;
            if (ginfo->originalbpp>16) bpp=24;
            break;

        case TYPEFSH: /* FSH */
            bpp=4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>16) bpp=24;
            if (ginfo->originalbpp>24) bpp=32;

			// A_8 or L_8 format?
            // 8000 or 0800
            else if (((ginfo->alphabits >= 1 && ginfo->redbits == 0 && ginfo->bluebits == 0 && ginfo->greenbits == 0)
 	             ||   (ginfo->alphabits == 0 && ginfo->redbits >= 1 && ginfo->bluebits == 0 && ginfo->greenbits == 0))
 	 			 && ginfo->packed < 3)
            {
                bpp  = 8;
                isI  = 8;  // just 1 channel ramp of alpha
                isIA = 0;
                nopalettevar = 1;
            }

			// check for: AL_88 FORMAT
            // 8800 8080 8008 formats
            if ((ginfo->packed < 3 ) && (
               (ginfo->redbits >= 1 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits >= 1 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo-> bluebits >= 1) ))
            {
                bpp  = 16;
                isIA = 8;
                isI  = 0;
                nopalettevar = 1;
            }
            break;

        case TYPEXSH: /* XSH */
		case TYPEZSH: /* Xenon */
		case TYPEYSH: /* PS3 */
            bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>16) bpp=32;

			// A_8 or L_8 format?
            // 8000 or 0800
            else if (((ginfo->alphabits >= 1 && ginfo->redbits == 0 && ginfo->bluebits == 0 && ginfo->greenbits == 0)
 	             ||   (ginfo->alphabits == 0 && ginfo->redbits >= 1 && ginfo->bluebits == 0 && ginfo->greenbits == 0))
 	 			 && ginfo->packed < 3)
            {
                bpp  = 8;
                isI  = 8;  // just 1 channel ramp of alpha
                isIA = 0;
                nopalettevar = 1;
            }

			// check for: AL_88 FORMAT
            // 8800 8080 8008 formats
            if ((ginfo->packed < 3 ) && (
               (ginfo->redbits >= 1 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits >= 1 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo-> bluebits >= 1) ))
            {
                bpp  = 16;
                isIA = 8;
                isI  = 0;
                nopalettevar = 1;
            }

            break;

        case TYPEVSH: /* VSH */
            bpp=8;
            break;

        case TYPEGSH: /* GSH */
            bpp = 4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>(2+15)) bpp=32;
            isI = isShapeIntensityTexture((ARGB *)source,ginfo);

            // I8 format?
            if ((ginfo->subtype&SUBADVANCED) && ginfo->alphabits > 4 && ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->packed < 3)
            {
                isI = 8;
                bpp = 8;
                nopalettevar = 1;
            }
            // I4 format?
            else if ((ginfo->subtype&SUBADVANCED) && ginfo->alphabits >= 1 && ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->packed < 3)
            {
                isI = 4;
                bpp = 4;
                nopalettevar = 1;
            }
			// IA8 format?
            // 8800 8080 8008 formats are supported GSH does a convert to grey
            // internally.
            else if ((
                 (ginfo->packed < 3 ) &&
                 (
                   (ginfo->redbits >= 1 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
                   (ginfo->redbits == 0 && ginfo->greenbits >= 1 && ginfo-> bluebits == 0) ||
                   (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo-> bluebits >= 1)
                 )
               )    ||  // let through Luminance textures if and only if advanced is not turned on.
               (
                 (ginfo->packed < 3 && !(ginfo->subtype&SUBADVANCED)) &&
                 (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->bpp >4 && ginfo->bpp <=8)
               )
              )
            {
                //IA8 CLUT format?
                if (ginfo->bpp == 8)
                {
                    isIA = 9;     // 9 is IA8 CLUT FORMAT DO NOT CHANGE... should be enum
                    //bpp  = 8; can be 4 or 8
                    isI  = 0;
                }
                // Just plain IA8 format?
                else if (ginfo->alphabits > 4 || ginfo->redbits > 4 || ginfo->greenbits > 4 || ginfo-> bluebits > 4)
                {
                    isIA = 8;
                    bpp  = 16;
                    isI  = 0;
                    nopalettevar = 1;
                }
                else // Just plain IA4 format?
				{
	                isIA = 4;
	                bpp  = 8;
	                isI  = 0;
	                nopalettevar = 1;
				}
            }
            else
            {
                isI = 0;
                isIA = 0;
                nopalettevar = 0;
            }
            break;

        case TYPEMSH: /* MSH */
            bpp=4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
			if (ginfo->originalbpp>16) bpp=32;
		break;

        case TYPESSH: /* SSH */
            bpp = 4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>16) bpp=24;
            if (ginfo->originalbpp>24) bpp=32;
            if ((bpp == 15 || bpp == 16) && ginfo->alphabits > 2)
                bpp = 32;
//            if (bpp == 24 && !(subtype&SUBADVANCED))
//                bpp=32;
            break;

        case TYPEASH: /* ASH */
            bpp = 4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            break;

        case TYPENSH: /* NSH */
            bpp=1;
            if (ginfo->originalbpp>1) bpp=4;
            if (ginfo->originalbpp>4) bpp=8;
            if (ginfo->originalbpp>8) bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>16) bpp=32;
            /* I/A special case. */
            if ((ginfo->alphabits == 8) &&
                (ginfo->redbits   == 8) &&
                (ginfo->greenbits == 0) &&
                (ginfo->bluebits  == 0))
                bpp = 16;
            break;
    }
    if (type != TYPEVSH)
    {
        if ((ginfo->alphabits == 0) &&
            (ginfo->redbits   == 2) &&
            (ginfo->greenbits == 8) &&
            (ginfo->bluebits  == 2))
            bpp = 16;

    }


	bool dxtenable=false;
	if (type==TYPEXSH || type==TYPEFSH || type==TYPEGSH || type==TYPEZSH || type==TYPEYSH || (subtype & SUBADVANCED))
		dxtenable = true;

    dxt = (dxtenable && (ginfo->packed==3 || ginfo->packed==5))?1:0;
    if (dxt && ginfo->bpp > 8)  // can't DXT compress a 8bpp image.
        bpp = 16;


    ok = 1;
	// NO SHAPE element
    if (ginfo->subtype&SUBNOSHP)
    {
        PictureShape = (SHPHeaderStruct *) 0;
        SizeShape=0;
    }
	// GST compression: note we do not GST compress mipmaps.
    else if(SHPIsGst(ginfo,type) && !SHPismipmap(ginfo->framename,strlen(ginfo->framename)))
    {
        SizeShape = SHPSetGstShape(&PictureShape,ginfo,reinterpret_cast<const unsigned char *>(source),type);
        SizeShape = SHPALIGN(SizeShape);
    }
	// Straight pixel formats
    else
    {
	  // If we were asking for GST but are not a format
      // that supports GST I need to turn it off so VfW
      // doesn't get messed up.
	  if(SHPIsGst(ginfo,TYPESSH))
		ginfo->packed = 0;

        if ((type==TYPEGSH  || type == TYPESSH  || type == TYPEMSH) && ginfo->subtype&SUBSWIZZLE && !dxt)
        {
            int realwidth  = ginfo->width;
            int realheight = ginfo->height;
            if (type == TYPEGSH)
                RetGSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            if (type == TYPESSH)
                RetSSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            if (type == TYPEMSH)
                RetMSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            SizeShape = SHPWCalcSizeShape((int)realheight,(int)realwidth,bpp,type,dxt);
        }
        else if (type==TYPESSH && ginfo->subtype&SUBSWIZZLE & !dxt)
        {
            int realwidth  = ginfo->width;
            int realheight = ginfo->height;
            RetSSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            SizeShape = SHPWCalcSizeShape((int)realheight,(int)realwidth,bpp,type,dxt);
        }
        // YUY2 HAS to be a multiple of 2 width this is VERY important! Buffer at the very least has
        // to be marginally bigger.
        else if ((type==TYPEXSH || type==TYPEFSH || type==TYPEZSH || type==TYPEYSH) && ginfo->alphabits == 0 && ginfo->redbits == 4 && ginfo->greenbits == 8 && ginfo->bluebits == 4)
        {
            int realwidth  = (ginfo->width+1)&~1;
            int realheight = ginfo->height; //(ginfo->height+1)&~1;
            SizeShape = SHPWCalcSizeShape((int)realheight,(int)realwidth,bpp,type,dxt);
        }
        else
            SizeShape = SHPWCalcSizeShape((int)(ginfo->width),(int)(ginfo->height),bpp,type,dxt);

        if (ginfo->packed==4)
            SizeShape = SizeShape*4 + 1024; // mpeg allowed space to grow
        else if (ginfo->packed>=2)
            SizeShape = (int)(SizeShape*1.2f + 1024); // otherwise, 20% is good enough

        SizeShape = SHPALIGN(sizeof(SHPHeaderStruct)+SizeShape);

        PictureShape=(SHPHeaderStruct*)galloc(SizeShape);
        if (PictureShape)
        {
            memset(PictureShape,0,SizeShape);
            SizeShape = SHPWSetShape(PictureShape,ginfo,bpp,source,rowbytes,ginfo->subtype&SUBNOSHP,type,isI,isIA);
            SizeShape = SHPALIGN(SizeShape);
        }
        else
            ok = 0;
    }


    SHPWSetShapeClut(ginfo,ginfo->numcolours,&ginfo->colourtbl[0],&ClutShape,&SizeClut,(ginfo->subtype&SUBNOPAL)||nopalettevar,type);
    SHPWSetShapeMetal(ginfo,&MetalShape,&SizeMetal,type);
    SHPWSetShapeDataBin(ginfo,&DataBinShape,&SizeDataBin,type);
    SHPWSetShapeHotSpot(ginfo,&HotSpotShape,&SizeHotSpot,ginfo->subtype&SUBNOHOT, type);
    SHPWSetShapeComment(ginfo,&CommentShape,&SizeComment,ginfo->subtype&SUBNOCOMMENT, type);

    SHPWSetShapeName(ginfo,&NameShape,framenum);

    /* get clut id */

	// fb - needs work... should base on NameShape or remove this
    clutid[0] = '!';
    for (i=1; i<4; ++i)
        clutid[i] = shapeid[i];

    for (i=0; (i<4) && ginfo->framename[i]; ++i)
        ;
    if (ginfo->framename[i])
    {
        for (i=0; (i<4) && ginfo->framename[i+4]; ++i)
            clutid[i] = ginfo->framename[i+4];

        while (i<4)
            clutid[i++] = ' ';
    }

    gputm(theShape->ListShape[framenum].shape_ClutID,ggetm(clutid,4),4);
    theShape->ListShape[framenum].shape_Data       = (char *) PictureShape;
    theShape->ListShape[framenum].shape_SizeData   = SizeShape;
    theShape->ListShape[framenum].shape_Clut       = (char *) ClutShape;
    theShape->ListShape[framenum].shape_SizeClut   = SizeClut;
    theShape->ListShape[framenum].shape_Metal      = (char *) MetalShape;
    theShape->ListShape[framenum].shape_SizeMetal  = SizeMetal;
    theShape->ListShape[framenum].shape_DataBin    = (char *) DataBinShape;
    theShape->ListShape[framenum].shape_SizeDataBin= SizeDataBin;
    theShape->ListShape[framenum].shape_HotSpot    = (char *) HotSpotShape;
    theShape->ListShape[framenum].shape_SizeHotSpot= SizeHotSpot;
    theShape->ListShape[framenum].shape_Comment    = (char *) CommentShape;
    theShape->ListShape[framenum].shape_SizeComment= SizeComment;
    theShape->ListShape[framenum].shape_Name       = NameShape;
    theShape->ListShape[framenum].subtype = ginfo->subtype;

    if (((ginfo->signature&0xffffff00)==GMAKEID('S','h','p',0))
      && ginfo->size==sizeof(GINFOSHP)
      && (((GINFOSHP *)ginfo)->align))
        theShape->ListShape[framenum].align = (((GINFOSHP *)ginfo)->align)-1;
    else
        theShape->ListShape[framenum].align = 0;

	// cleanup ginfo copy
    if(ginfo)
        gfree(ginfo);

    return(ok);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// actually write the shape file out to disk. Loops over all shapes and writes the file out.
int SHPWWriteShapeFile(GSTREAM *g,ShapeListPtr theShape,int type, int subtype, int multi,int shpsignature, int shpdataalignment, int shpclutdataalignment,int shpalignment, int shpmipmapdataalignment, int shppackeddataalignment)
{
    SHPFILEHeaderStruct HeaderFile;   /* shape file header sizeof(SHPHeaderStruct) bytes */
    ShapeTbl *TableShape=0L;    /* id/offset table */
    int  offset;
    int  paloffset;
    int  bigpad   = 0;
    int  i;            // it's a big shape and needs to go at end of file.
    int  ok;
    char *shape_Data;
    char *shape_Clut;
    char *shape_Metal;
    char *shape_DataBin;
    char *shape_HotSpot;
    char *shape_Comment;
    int  shape_SizeData;
    int  shape_SizeClut;
    int  shape_SizeMetal;
    int  shape_SizeDataBin;
    int  shape_SizeHotSpot;
    int  shape_SizeComment;
    char *shape_Name;
    int NumShapesAndPals=0;          /* number of shapes AND palettes in shape header */
    int theSize;


    /* Concatenate mipmap level data if subtype flag set. */


    if (theShape->NumShape>1)
        if (!(theShape->ListShape[0].subtype&SUBNOMIPMAPCOLLAPSE) && !(theShape->ListShape[1].subtype&SUBNOMIPMAPCOLLAPSE))
            SHPWCollapseMipmaps(theShape,type, shpalignment, shpmipmapdataalignment);

    SHPWOrderCubemaps(theShape,type,shpalignment,shpmipmapdataalignment);

    SHPWAlignPalette(theShape,type, shpclutdataalignment);

    bigpad = SHPWSetNexts(theShape,type,shppackeddataalignment,shpdataalignment);  /* Calc mNext shape */

    /* write shape file header sizeof(SHPHeaderStruct) (or 6) bytes */

    if (subtype==SHAPEFILE && multi)
    {
        theSize = sizeof(SHPFILEHeaderStruct);
        ok = (gwrite(g,(char*)&HeaderFile,theSize) == theSize);
    }
    else
        ok = 1;

    if (ok)
    {
	    int HeaderSize=0;
        const int SIZEOFVENOM=8;


        /* write id/offset table */

        if (subtype==SHAPEFILE && multi)
        {

            NumShapesAndPals = theShape->NumShape;
#if 0

            if ((theShape->ListShape[0].subtype&SUBVSHPAL) && (type==TYPEFSH || type==TYPEVSH)) /* FSH and VSH like entries for pals */
            {
                EntriesPerShape = 2;
                NumShapesAndPals *= 2;
                PutPal = 1;
            }
#endif

            for (i=0; i<theShape->NumShape; ++i)
            {
                HeaderSize += sizeof(struct ItemShapeTbl)+strlen(theShape->ListShape[i].shape_Name);

//printf("name is %s %d\n", theShape->ListShape[i].shape_Name,HeaderSize);

            }

            ok = 1;
            if (HeaderSize)
            {
                TableShape = (ShapeTbl *) galloc(HeaderSize+SIZEOFVENOM);
                if (!TableShape)
                    ok=0;

                if (ok)
                {
                    memset(TableShape,0,HeaderSize+SIZEOFVENOM);
                    ok = (gwrite(g,(char*)TableShape,HeaderSize+SIZEOFVENOM) == (HeaderSize+SIZEOFVENOM));
                }
            }
        }
        if (ok)
        {
            ShapeTbl * NextTableShape = TableShape;

            for (i=0; i<theShape->NumShape; ++i)
            {
                shape_Data        = theShape->ListShape[i].shape_Data;
                shape_SizeData    = theShape->ListShape[i].shape_SizeData;
                shape_Clut        = theShape->ListShape[i].shape_Clut;
                shape_SizeClut    = theShape->ListShape[i].shape_SizeClut;
                shape_Metal       = theShape->ListShape[i].shape_Metal;
                shape_SizeMetal   = theShape->ListShape[i].shape_SizeMetal;
                shape_DataBin     = theShape->ListShape[i].shape_DataBin;
                shape_SizeDataBin = theShape->ListShape[i].shape_SizeDataBin;
                shape_HotSpot     = theShape->ListShape[i].shape_HotSpot;
                shape_SizeHotSpot = theShape->ListShape[i].shape_SizeHotSpot;
                shape_Comment     = theShape->ListShape[i].shape_Comment;
                shape_SizeComment = theShape->ListShape[i].shape_SizeComment;
                shape_Name        = theShape->ListShape[i].shape_Name;

                offset = (int)glen(g);

                if (theShape->ListShape[i].align)
                    shpdataalignment = theShape->ListShape[i].align;

                // pad shape data for alignment
                if (subtype==SHAPEFILE && multi)
                {
                    int padlen;
                    SHPHeaderStruct *shpout = (SHPHeaderStruct *) shape_Data;
                    if (isCompressed(shpout,type))
                        padlen = ((offset+(sizeof(SHPHeaderStruct)+8)+shppackeddataalignment)&~shppackeddataalignment)-(sizeof(SHPHeaderStruct)+8)-offset;
                    else
                        padlen = ((offset+sizeof(SHPHeaderStruct)+shpdataalignment)&~shpdataalignment)-sizeof(SHPHeaderStruct)-offset;
                    if (padlen>0)
                    {
                        char *padstr=(char *)galloc(shpdataalignment+1);
                        memset(padstr,0,shpdataalignment+1); // "EASports";

#ifdef DEBUG
                        memcpy(padstr,"SellERTS",8); /* a way to spot debug lib :) */
#else
                        memcpy(padstr,"Buy ERTS",8); /* pad with a promotional message */
#endif
                        ok = (gwrite(g,padstr,padlen) == padlen);
                        gfree(padstr);
                        offset += padlen;
                    }
                }

                paloffset = offset+shape_SizeData;

                if (subtype==SHAPEFILE && multi)
                {
                    int chunksize=
                        shape_SizeData+
                        shape_SizeClut+
                        shape_SizeMetal+
                        shape_SizeHotSpot+
                        shape_SizeComment;

                    SHPFILEPUTFIELD(NextTableShape->ShapeTable[0].OffsetShape,offset);
                    SHPFILEPUTFIELD(NextTableShape->ShapeTable[0].SizeShape,chunksize);
                             strcpy(NextTableShape->ShapeTable[0].NameShape, theShape->ListShape[i].shape_Name);

                    NextTableShape = (ShapeTbl *) ((char *) NextTableShape + sizeof(struct ItemShapeTbl)+strlen(theShape->ListShape[i].shape_Name));
                }

                if (subtype==STREAMINGSHAPE && multi)
                {
                    char chunkbuf[8];
                    int chunksize=8+
                        shape_SizeData+
                        shape_SizeClut+
                        shape_SizeMetal+
                        shape_SizeHotSpot+
                        shape_SizeComment;

                    gputm(chunkbuf,shpsignature,4);
                    gputi(chunkbuf+4,chunksize,4);
                    ok = gwrite(g,chunkbuf,8);

                }

                if (ok && shape_Data)
                {
                    ok = (gwrite(g,(char*)shape_Data,shape_SizeData) == shape_SizeData);
                }
                if (ok && shape_Clut)
                {
                    ok = (gwrite(g,(char*)shape_Clut,shape_SizeClut) == shape_SizeClut);
                }
                if (ok && shape_Metal)
                {
                    ok = (gwrite(g,(char*)shape_Metal,shape_SizeMetal) == shape_SizeMetal);
                }
                if (ok && shape_DataBin)
                {
                    ok = (gwrite(g,(char*)shape_DataBin,shape_SizeDataBin) == shape_SizeDataBin);
                }
                if (ok && shape_HotSpot)
                {
                    ok = (gwrite(g,(char*)shape_HotSpot,shape_SizeHotSpot) == shape_SizeHotSpot);
                }
                if (ok && shape_Comment)
                {
                    ok = (gwrite(g,(char*)shape_Comment,shape_SizeComment) == shape_SizeComment);
                }
            }


            if (ok && subtype==SHAPEFILE && multi)
            {
                gseek(g,(GPOS)0); /* To be at the beginning of the file */

                {
                    gputm(HeaderFile.typeFile,shpsignature,4);
                    gputi(HeaderFile.LengthFile,(int)glen(g),4);
                    SHPFILEPUTFIELD(HeaderFile.NumShape,NumShapesAndPals);

                    SHPFILEPUTFIELD(HeaderFile.Creator,sizeof(struct SHPFILEHeaderStruct)+HeaderSize+SIZEOFVENOM); // Header size
                    ok = (gwrite(g,(char*)&HeaderFile,sizeof(SHPFILEHeaderStruct)) == sizeof(SHPFILEHeaderStruct));
                }

                ok = (gwrite(g,(char*)TableShape,HeaderSize) == HeaderSize);

#if VENOM
                {
                    char venom[SIZEOFVENOM];
                    memset(venom,0,SIZEOFVENOM);

                    if (theShape->ListShape[0].subtype&SUBADVANCED)
                    {
                        gputm(venom,GMAKEID(
                            'A',
                            (SHP_VERSION/100)%10+'0',
                            (SHP_VERSION/10)%10+'0',
                            (SHP_VERSION)%10+'0'),4);
                    }
                    else
                    {
                        gputm(venom,GMAKEID(
                            'G',
                            (SHP_VERSION/100)%10+'0',
                            (SHP_VERSION/10)%10+'0',
                            (SHP_VERSION)%10+'0'),4);
                    }
                    ok = (gwrite(g,venom,SIZEOFVENOM) == SIZEOFVENOM);
                }
#endif
            }
        }
    }
    if (TableShape) gfree((char*)TableShape);
    return(ok);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Delete a list of Shapes/cluts/hotspots/comments
void SHPWFreeShapes(ShapeListPtr theShape)
{
    int    i;

    for (i=0; i<theShape->NumShape; ++i)
    {
        if (theShape->ListShape[i].shape_Data)    gfree((char*)(theShape->ListShape[i].shape_Data));
        if (theShape->ListShape[i].shape_Clut)    gfree((char*)(theShape->ListShape[i].shape_Clut));
        if (theShape->ListShape[i].shape_Metal)   gfree((char*)(theShape->ListShape[i].shape_Metal));
        if (theShape->ListShape[i].shape_DataBin) gfree((char*)(theShape->ListShape[i].shape_DataBin));
        if (theShape->ListShape[i].shape_HotSpot) gfree((char*)(theShape->ListShape[i].shape_HotSpot));
        if (theShape->ListShape[i].shape_Comment) gfree((char*)(theShape->ListShape[i].shape_Comment));
        if (theShape->ListShape[i].shape_Name)    gfree((char*)(theShape->ListShape[i].shape_Name));
    }
    gfree((char*)theShape);
}

#endif

