// (c) 2007 Electronic Arts Inc.
#include <string.h>

#include "gimex.h"
#include "oldshpgimex.h"
#ifdef DEBUG
#include <stdio.h>
#endif

#include "gst.h"
#include "gstglue.h"
#include "oldshpgstglue.h"
using Gst::OLDSHPIsGst;
using Gst::OLDSHPSetGstShape;
using Gst::OLDSHPGstValidate;

#define SWIZZLE4BUG 0

void OLDgputx(void *memptr, unsigned int val, int numbytes, int type);
unsigned int OLDggetx(const void *memptr, int numbytes, int type);
#define OLDSHPGETFIELD(field) OLDggetx((void *)&field, sizeof(field), type)
#define OLDSHPPUTFIELD(field,data) OLDgputx((void *)&field,(unsigned int)(data),sizeof(field), type)

#define OLDSHPMIPMAPALIGN(x) (((x)+oldshpmipmapdataalignment)&~oldshpmipmapdataalignment)


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

#ifndef __OLDSHPWRITE
#define __OLDSHPWRITE 1
#include "codex.h"
#include "refcodex.h"
#include "dxtcodex.h"
#include "s3tccodex.h"
#include "idec_encode.h"
#include "oldshpcmn.h"

#include "oldshpwrite.h"

#define BIGSHAPESIZE 8388608


static void OLDSHPWOrderCubemaps(OLDShapeListPtr theShape,int type, int oldshpalignment, int oldshpmipmapdataalignment);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static void OLDSHPWSetLink(char **prevptr, int *prevlen, char *newptr, int newlen, int type)
{
    if (newptr)
    {
        if (*prevptr)
        {
            int linkoffset = *prevlen;
            if (linkoffset>0x7fffff)
                linkoffset = 0;
            OLDSHPPUTFIELD((((OLDSHPHeaderStruct*)*prevptr)->LinkShape),linkoffset);
        }

        OLDSHPPUTFIELD((((OLDSHPHeaderStruct*)newptr)->LinkShape),0);
        *prevptr = newptr;
        *prevlen = newlen;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the rowbyes of the shape taking into account the alignment issues.

static int OLDSHPWCalcRowBytes(int width, int bpp, int type)
{
    int SizeShape=0L;

    if (bpp == 15)
        bpp = 16;

    switch (type)
    {
        case TYPEOLDPSH:
        case TYPEOLDASH:
        case TYPEOLDSSH: SizeShape=((width*bpp+15)&~15)/8; break;
        case TYPEOLDMSH: SizeShape=((width*bpp+127)&~127)/8; break; // PSP shape rows must be 16-byte aligned.
        default:         SizeShape=((width*bpp+7)&~7)/8; break;
    }

    return(SizeShape);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// note this has been a constant source of aggravation with alignment watch out
// when playing with shape alignment.

static int OLDSHPWCalcSizeShape(int width,int height,int bpp, int type, int dxt)
{
    int    SizeShape=0L;

    if (dxt)
    {
        width = (width+3)&~3;
        height = (height+3)&~3;
    }
    SizeShape = OLDSHPWCalcRowBytes(width, bpp, type);
    SizeShape *= height;

    return(SizeShape);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// unltimate source of information about shapetypes. Looks up based on
// shapetype the bpp of the Shape.
static int OLDSHPWGetOldshpbpp(OLDSHPHeaderStruct *theShape,int type)
{
    int     bpp=0;

    if (type==TYPEOLDVSH)
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
            case 0x23: bpp=16; break;   /* OLDPSH types */
            case 0x40: bpp=4;  break;
            case 0x41: bpp=8;  break;
            case 0x42: bpp=16; break;
            case 0x43: bpp=24; break;
            case 0x44: bpp=1;  break;

            case 0x30: bpp=16; break;   /* OLDGSH types */
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

            case 0x38: bpp=16; break;   /* OLDMSH types */
            case 0x39: bpp=15; break;
            case 0x3a: bpp=16; break;
            case 0x3b: bpp=32; break;
            case 0x58: bpp=16; break;
            case 0x59: bpp=15; break;
            case 0x5a: bpp=16; break;
            case 0x5b: bpp=32; break;
            case 0x5c: bpp=4;  break;
            case 0x5d: bpp=8;  break;

            case 0x20: bpp=15; break;   /* OLDSSH types */
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

            case 0x22: bpp=24; break;   /* OLDFSH OLDXSH OLDVSH types */
            case 0x24: bpp=24; break;
            case 0x29: bpp=16; break;
            case 0x2a: bpp=32; break;
            case 0x2c: bpp=32; break;
            case 0x2d: bpp=16; break;
            case 0x2e: bpp=32; break;
            case 0x60: bpp=4;  break;   /* DXT1 */
            case 0x61: bpp=8;  break;   /* DXT3 */
            case 0x62: bpp=8;  break;   /* DXT5 */
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

                                        /* OLDNSH */
            case 0x27: bpp=16; break;   /* RGBA clut */
            case 0x28: bpp=16; break;   /* IA clut */
//            case 0x60: bpp=1;  break;   /* CI */
//            case 0x61: bpp=4;  break;   /* I */
//            case 0x62: bpp=4;  break;   /* IA */
            case 0x63: bpp=4;  break;   /* CI */
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



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Put number of mipmap levels into shape header struct
//   in the upper 4 bits of the shapey field.
static void OLDSHPWSetMipmapLevels(OLDSHPHeaderStruct *theShape, int levels, int type)
{
    int shapey;

    if (levels==0)
        levels = 1;
    if (theShape)
    {
        DXTHEADER *dxt = (DXTHEADER *)((char *)theShape+sizeof(OLDSHPHeaderStruct));

        shapey = OLDSHPGETFIELD(theShape->shapey) & 0x0fff;
        OLDSHPPUTFIELD(theShape->shapey, shapey + ((levels-1) << 12));

        if ((theShape->type&0x80) && DXT_is((char *)dxt))
        {
            dxt->mode[0] = (dxt->mode[0] & 0x0f) | ((levels-1) << 4);
        }
    }
}

//--------------------------------------------------------------------------------------------------------
//   Mipmap levels are stored in upper 4 bits of
//   shapey. If 0 then shape has no mipmap levels;
//   non-zero records total number of levels,
//   including level 0.
static int  OLDSHPWGetMipmapLevels(OLDSHPHeaderStruct *theShape,int type)
{
    int shapey;
    int levels;
    int width;

    if (!theShape)
        return 0;
    width  = OLDSHPGETFIELD(theShape->width);
    shapey = OLDSHPGETFIELD(theShape->shapey);
    levels = shapey >> 12;
    if ( (levels == 0) | (levels == 15) )
        levels = 1;
    else
        levels += 1;

    if((1<<(levels-1)) > width) // simple error checking for mipmap level information.
        levels = 1;
    return(levels);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set the relative bit depth if oldshpout is DXT compressed.
static void GetDXTBpp(OLDSHPHeaderStruct *oldshpout,OLDSHPHeaderStruct *oldshp,int collapse,int * bpp,int *dxt,int type)
{
    // if the shape is packed, it's a DXT shape or DXT pack type then we will check further.
    if ((oldshpout->type&0x80) && DXT_is((char *)oldshpout+sizeof(OLDSHPHeaderStruct)) || collapse)
    {
        if (DXT3_is((const char *)oldshpout+sizeof(OLDSHPHeaderStruct)) || OLDSHPGETFIELD((oldshp->type)) == 0x61)
        {
            *bpp = 8; // DXT3 treated as 8 bpp
            *dxt = 3;
        }
        else if (DXT5_is((const char *)oldshpout+sizeof(OLDSHPHeaderStruct))|| OLDSHPGETFIELD(oldshp->type) == 0x62)
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
static void OLDSHPWCollapseMipmaps(OLDShapeListPtr theShape,int type, int oldshpalignment, int oldshpmipmapdataalignment)
{
    int             in, out;
    OLDgshapeInfo      *sinfo, *sinfoout;
    OLDSHPHeaderStruct *oldshp, *oldshpout;
    int levels = 0;
    char * newdata = 0;
    int newsize    = 0;
    int collapse   = 0;
    int notmipmap  = 0;
    /* Start at 1, since 0th frame has nothing to be appended to. */

    out = 0;
    in = 1;
    while (in < theShape->NuoLdmshape)
    {
        sinfoout = &theShape->ListShape[out];
        sinfo = &theShape->ListShape[in];
        oldshp = (OLDSHPHeaderStruct *)sinfo->shape_Data;
        oldshpout = (OLDSHPHeaderStruct *)sinfoout->shape_Data;

        /* Mipmaps identified by signature "mm<digit>". Add all such
           consecutive shapes to the current "output" shape. */
        notmipmap = 0;
        if (OLDSHPismipmap(sinfo->shape_ID,4) && (OLDSHPWGetOldshpbpp(oldshp, type) == OLDSHPWGetOldshpbpp(oldshpout, type)))
        {
            int width, height, bpp, thissize, dxt=0;

            width = OLDSHPGETFIELD(oldshp->width);
            height = OLDSHPGETFIELD(oldshp->height);
            bpp = OLDSHPWGetOldshpbpp(oldshp, type);
            // figure out if this is a DXT shape type or a packtype dxt shape.. either way set bpp correctly.
            collapse = ((OLDSHPGETFIELD(oldshpout->type) >= 0x60) && (OLDSHPGETFIELD(oldshpout->type) <= 0x62) || (OLDSHPGETFIELD(oldshpout->type) == 0x1e)); // 1e oldgsh comparable to dxt1.
            GetDXTBpp(oldshpout,oldshp,collapse,&bpp,&dxt,type);


            /* On first level we allocate space for combined data;
               assuming that there may be mipmaps all the way down
               to 1x1. */

            if (!levels)
            {
                int w = OLDSHPGETFIELD(oldshpout->width);
                int h = OLDSHPGETFIELD(oldshpout->height);

//              thissize = OLDSHPMIPMAPALIGN(OLDSHPWCalcSizeShape(w, h, bpp, type, dxt));
                if(dxt && type==TYPEOLDGSH)
                {
                    w = (w+7)&~7;
                    h = (h+7)&~7;
                }


                if((type == TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH) && OLDSHPGETFIELD(oldshpout->shapex)&0x2000 && !dxt)  // is swizzled OLDGSH?
                {
                    int realwidth  = 0;
                    int realheight = 0;
                    if(type == TYPEOLDGSH)
                        RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                    if(type == TYPEOLDSSH)
                        RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
					if(type == TYPEOLDMSH)
						RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                    thissize = newsize = OLDSHPWCalcSizeShape(realwidth, realheight, bpp, type, dxt);
                }
                else
                    thissize = newsize = OLDSHPWCalcSizeShape(w, h, bpp, type, dxt);

                while ((w > 1) || (h > 1))
                {
//                    int tw, th;
                    if (w > 1)  w >>= 1;
                    if (h > 1)  h >>= 1;
                    if((type == TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH) && OLDSHPGETFIELD(oldshpout->shapex)&0x2000 && !dxt)  // is swizzled OLDGSH?
                    {
                        int realwidth  = 0;
                        int realheight = 0;
                        if(type == TYPEOLDGSH)
                            RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                        if(type == TYPEOLDSSH)
                            RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                        if(type == TYPEOLDMSH)
                            RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,w,h,bpp);
                        newsize += OLDSHPMIPMAPALIGN(thissize) + OLDSHPWCalcSizeShape(realwidth, realheight, bpp, type, dxt);
                    }
                    else if(dxt && type==TYPEOLDGSH)
                    {
                        //newsize += OLDSHPMIPMAPALIGN(thissize) + OLDSHPWCalcSizeShape(((w+15)&~15), ((h+15)&~15), bpp, type, dxt);
                        newsize += OLDSHPMIPMAPALIGN(thissize) + OLDSHPWCalcSizeShape(((w+7)&~7), ((h+7)&~7), bpp, type, dxt);
                    }
                    else
                        newsize += OLDSHPMIPMAPALIGN(thissize) + OLDSHPWCalcSizeShape(w, h, bpp, type, dxt);
                }
                newsize += sizeof(OLDSHPHeaderStruct);
                if (dxt && !collapse) // dont remove dxt header
                    newsize += 8;

                newdata = (char *)galloc(newsize*2+1024); // 2+1024 is a kludge for dxt3 corruption issue
                memset(newdata,0,newsize*2+1024);

                memcpy(newdata, oldshpout, sizeof(OLDSHPHeaderStruct));

                newsize = sizeof(OLDSHPHeaderStruct);

                /* Add top level of mipmap. */
                if ((oldshpout->type&0x80) && REF_is((char *)oldshpout+sizeof(OLDSHPHeaderStruct)))
                    REF_decode(newdata + newsize, (char*)oldshpout + sizeof(OLDSHPHeaderStruct), 0);
                else if (dxt && !collapse) // dont remove dxt header
                {
                    thissize += 8;  // copy header on first one
                    memcpy(newdata + newsize, (char*)oldshpout + sizeof(OLDSHPHeaderStruct), thissize);
                }
                else
                    memcpy(newdata + newsize, (char*)oldshpout + sizeof(OLDSHPHeaderStruct), thissize);
                newsize += thissize;
                levels = 1;
            }

            /* Append data for this shape to newdata for "out" shape. */

            if (dxt && !collapse) // dont remove dxt header
                newsize = OLDSHPMIPMAPALIGN(newsize-24)+24;
            else
                newsize = OLDSHPMIPMAPALIGN(newsize-16)+16;

            if((type == TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH) && OLDSHPGETFIELD(oldshpout->shapex)&0x2000 && !dxt)  // is swizzled OLDGSH?
            {
                int realwidth  = 0;
                int realheight = 0;
                if(type == TYPEOLDGSH)
                    RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                if(type == TYPEOLDSSH)
                    RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                if(type == TYPEOLDMSH)
                    RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                thissize = OLDSHPWCalcSizeShape(realwidth, realheight, bpp, type, dxt);
            }
            else if(dxt && type==TYPEOLDGSH)
            {
                width  = (width+7)&~7;
                height = (height+7)&~7;
                //width  = (width+15)&~15;
                //height = (height+15)&~15;
                thissize = OLDSHPWCalcSizeShape(width, height, bpp, type, dxt);
            }
            else
                thissize = OLDSHPWCalcSizeShape(width, height, bpp, type, dxt);

            //thissize = OLDSHPWCalcSizeShape(width, height, bpp, type, dxt);

            if ((oldshp->type&0x80) && REF_is((char *)oldshp+sizeof(OLDSHPHeaderStruct)))
                REF_decode(newdata + newsize,(char *)oldshp + sizeof(OLDSHPHeaderStruct), 0);
            else if (dxt && !collapse)
                memcpy(newdata + newsize, (char *)oldshp + sizeof(OLDSHPHeaderStruct) + 8, thissize);
            else
                memcpy(newdata + newsize, (char *)oldshp + sizeof(OLDSHPHeaderStruct), thissize);

            /* Free this shape and mark as such. (Note that cluts and
               hot spots get lost for all but top level of mipmap). */

            if (sinfo->shape_Data)
                gfree(sinfo->shape_Data);
            if (sinfo->shape_Clut)
                gfree(sinfo->shape_Clut);
            if (sinfo->shape_Metal)
                gfree(sinfo->shape_Metal);
            if (sinfo->shape_HotSpot)
                gfree(sinfo->shape_HotSpot);
            if (sinfo->shape_Anim)
                gfree(sinfo->shape_Anim);
            if (sinfo->shape_Name)
                gfree(sinfo->shape_Name);
            if (sinfo->shape_Comment)
                gfree(sinfo->shape_Comment);
            if (sinfo->shape_Tpage)
                gfree(sinfo->shape_Tpage);

            sinfo->shape_SizeData = 0;
            sinfo->shape_SizeClut = 0;
            sinfo->shape_SizeMetal = 0;
            sinfo->shape_SizeHotSpot = 0;
            sinfo->shape_SizeAnim = 0;
            sinfo->shape_SizeName = 0;
            sinfo->shape_SizeComment = 0;
            sinfo->shape_SizeTpage = 0;

            /* Add size for next level of mipmap. */

            newsize += thissize;
            ++levels;
        } // END if SHPismipmap
        else
            notmipmap = 1;            // set this because we can't check oldshp if it's memory gets deleted later.

        /* If not a mipmap level or we're at the last shape
           then store out the results. */

        if (
              (in == theShape->NuoLdmshape - 1) ||
              notmipmap
           )
        {
            /* If we just finished combining mipmaps then save
               the combined data in the output shape. */

            if (newdata)
            {
                newsize = OLDSHPALIGN(newsize);

                /* Check whether to pack the new data. */

                if ((oldshpout->type&0x80) && REF_is((char *)oldshpout+sizeof(OLDSHPHeaderStruct)))
                {
                    char *newdatapack = (char *)galloc(newsize*3/2+1024);

                    /* Pack data into newly allocated space. This is wasteful of memory, oh well. */

                    memcpy(newdatapack, newdata, sizeof(OLDSHPHeaderStruct));
                    newsize = (REF_encode(newdatapack + sizeof(OLDSHPHeaderStruct),
                                          newdata + sizeof(OLDSHPHeaderStruct),
                                          newsize - sizeof(OLDSHPHeaderStruct),
                                          0));
                    newsize += sizeof(OLDSHPHeaderStruct);
                    gfree(newdata);
                    newdata = newdatapack;
                }

                gfree(sinfoout->shape_Data);
                sinfoout->shape_Data = newdata;
                sinfoout->shape_SizeData = newsize;
                oldshpout = (OLDSHPHeaderStruct *)newdata;

                OLDSHPWSetMipmapLevels(oldshpout, levels, type);
            }

            /* Next output shape will be this shape. */
            out = in;
            levels = 0;
            newdata = 0;
            newsize = 0;
        }

        ++in;
    }

    /* Second pass through shape list removes all the unused entries. */

    out = 0;
    for (in = 0; in < theShape->NuoLdmshape; ++in)
    {
        if (theShape->ListShape[in].shape_SizeData ||
            theShape->ListShape[in].shape_SizeClut ||
            theShape->ListShape[in].shape_SizeMetal ||
            theShape->ListShape[in].shape_SizeHotSpot ||
            theShape->ListShape[in].shape_SizeAnim ||
            theShape->ListShape[in].shape_SizeName ||
            theShape->ListShape[in].shape_SizeComment ||
            theShape->ListShape[in].shape_SizeTpage)
        {
            if (in != out)
            {
                memcpy(&theShape->ListShape[out],
                       &theShape->ListShape[in],
                       sizeof(OLDgshapeInfo));
            }

            ++out;
        }
    }

    /* Reset total number of shapes to the number remaining after mipmap collapsing. */

    theShape->NuoLdmshape = out;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// includes header size.
static char * CreateCubemapBuffer(int width,int height,int bpp,int * newsize, int levels,int oldshpmipmapdataalignment,int oldshpalignment)
{
    int csize = sizeof(OLDSHPHeaderStruct);
    // assume width is right and that height is divided by 6 to get real height.
    int rsize = height/6;
    int onesize = 0;
    int i = 0;
    int j = 0;
    int w = width;
    int h = rsize;
    char * newbuf = 0;
    // for each mipmaplevel.
    for(i = 0; i < levels; i++)
    {
        onesize = (int)(OLDSHPMIPMAPALIGN(onesize) + ((float)bpp/8.0)*w*h);
        w = w>>1;
        h = h>>1;
        h = h<1?1:h;
        w = w<1?1:w;
    }
    // align that face.
    //onesize += OLDSHPFACEALIGN(onesize); we will need to align to the face alignment in the future.
    onesize = onesize*6;
    csize += onesize;
    OLDSHPALIGN(csize);

    *newsize = csize;
    newbuf   = (char *)galloc(csize);
    return (newbuf);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static int getCubeOffset(int l,int s,int oldshpalignment,int oldshpmipmapdataalignment,int levels,int * lsize,int width,int height,int bpp)
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
        if(i != (levels-1))
            levelsize = OLDSHPMIPMAPALIGN(levelsize);
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
static int locNextCube(int l,int current,int oldshpalignment,int oldshpmipmapdataalignment,int levels,int lsize)
{
    // add the written size to the current position.
    current = (current + lsize);
    // align, in the future we will have to do the face alignment but currently we can't as
    // we do not know what to align to,
    if(l < (levels-1))
        current = OLDSHPMIPMAPALIGN(current);
    //else
    //    current = OLDSHPFACEALIGN(current);
    return(current);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Rewrite mipmapped shapes into single shape. The shape
//   header has the width and height of the top level; data
//   for following levels is concatenated to top level data.
static void OLDSHPWOrderCubemaps(OLDShapeListPtr theShape,int type, int oldshpalignment, int oldshpmipmapdataalignment)
{
    int             in        = 0;
    OLDgshapeInfo      *sinfo    = 0;
    OLDSHPHeaderStruct *oldshp      = 0;
    OLDgshapeInfo      *sinfoout = 0;
    OLDSHPHeaderStruct *oldshpout   = 0;
    int levels                = 0;
    int collapse              = 0;
    int size                  = 0;

    for(in = 0; in < theShape->NuoLdmshape;in++)
    {
        sinfo    = &theShape->ListShape[in];
        oldshp      = (OLDSHPHeaderStruct *)sinfo->shape_Data;
        sinfoout = sinfo;             // just to make relevant code look like the collapse mipmaps code.
        oldshpout   = oldshp;
        levels   = OLDSHPWGetMipmapLevels(oldshp,type);
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
            unsigned int current = sizeof(OLDSHPHeaderStruct);
            char * tempbuf       = 0;
            char * data          = (char *)oldshp + sizeof(OLDSHPHeaderStruct);

            width  = OLDSHPGETFIELD(oldshp->width);
            height = OLDSHPGETFIELD(oldshp->height);
            bpp    = OLDSHPWGetOldshpbpp(oldshp, type);
            // figure out if this is a DXT shape type or a packtype dxt shape.. either way set bpp correctly.
            collapse = ((OLDSHPGETFIELD(oldshpout->type) >= 0x60) && (OLDSHPGETFIELD(oldshpout->type) <= 0x62) || (OLDSHPGETFIELD(oldshpout->type) == 0x1e)); // 1e oldgsh comparable to dxt1.
            GetDXTBpp(oldshpout,oldshp,collapse,&bpp,&dxt,type);
            // get size of new buffer and create.
            tempbuf = CreateCubemapBuffer(width,height,bpp,&size,levels,oldshpmipmapdataalignment,oldshpalignment);
            memset(tempbuf,0,size);
            memcpy(tempbuf,oldshp,sizeof(OLDSHPHeaderStruct));
            // for side of the cube.
            for(s = 0; s < 6; s++)
            {
                // for mipmap level.
                for(l = 0; l < levels;l++)
                {
                    // get offset of level from top.
                    // get size of level.
                    offset = getCubeOffset(l,s,oldshpalignment,oldshpmipmapdataalignment,levels,&lsize,width,height,bpp);
                    // copy over bitmap.
                    memcpy(tempbuf+current,data+offset,lsize);
                    // increment current position.
                    current = locNextCube(l,current,oldshpalignment,oldshpmipmapdataalignment,levels,lsize);
                }
            }
            // cleanup
            if(sinfo->shape_Data)
                gfree(sinfo->shape_Data);
            sinfo->shape_Data     = tempbuf;
            sinfo->shape_SizeData = size;
        }

    }  // for (in) over nuoldmshapes.
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// adjust size of collapsed shape to account for palette alignment
static void OLDSHPWAlignPalette(OLDShapeListPtr theShape,int type, int oldshpclutdataalignment)
{
    int in;

    for (in = 0; in < theShape->NuoLdmshape; ++in)
    {
        if (theShape->ListShape[in].shape_Data &&
            theShape->ListShape[in].shape_SizeData &&
            theShape->ListShape[in].shape_SizeClut)
        {
            int padlen;
            int offset=theShape->ListShape[in].shape_SizeData;
            void *newdata;

            padlen = ((offset-16+oldshpclutdataalignment+16)&~oldshpclutdataalignment)-offset;
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
// Link the clut, hotspot, name, comment and anim to the shape
static int OLDSHPWSetLinks(OLDShapeListPtr theShape,int type,int oldshppackeddataalignment,int oldshpdataalignment)
{
    int i;
    int tlink;
    char *oldvshheader;
    int padlen          = 0;

    for (i=0; i<theShape->NuoLdmshape; ++i)
    {
        char *prevptr=0;
        int  prevlen=0;
        int  accume = 0;
        if(theShape->ListShape[i].shape_SizeData < BIGSHAPESIZE)
        {
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Data,theShape->ListShape[i].shape_SizeData, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Clut,theShape->ListShape[i].shape_SizeClut, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Metal,theShape->ListShape[i].shape_SizeMetal, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_HotSpot,theShape->ListShape[i].shape_SizeHotSpot, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Anim,theShape->ListShape[i].shape_SizeAnim, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Name,theShape->ListShape[i].shape_SizeName, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Comment,theShape->ListShape[i].shape_SizeComment, type);
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Tpage,theShape->ListShape[i].shape_SizeTpage, type);
        }
        else
        {
			// the accumulator stuff is to figure out how much I need to pad to make the
			// the data aligned correctly.
			// The amounts I am working with in the links is exactly what gets written
			// out. (see the file writing function) so this should be correct.
			// Notice that the clut DATA is the item that is aligned correctly
			// so when I calculate my padlength I would have added 16 for the picture
			// header and then subtracted it again for the clut header.
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Clut,theShape->ListShape[i].shape_SizeClut, type);
            if(prevlen == theShape->ListShape[i].shape_SizeClut)
			    accume += prevlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Metal,theShape->ListShape[i].shape_SizeMetal, type);
            if(prevlen == theShape->ListShape[i].shape_SizeMetal)
			    accume += prevlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_HotSpot,theShape->ListShape[i].shape_SizeHotSpot, type);
            if(prevlen == theShape->ListShape[i].shape_SizeHotSpot)
			    accume += prevlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Anim,theShape->ListShape[i].shape_SizeAnim, type);
            if(prevlen == theShape->ListShape[i].shape_SizeAnim)
			    accume += prevlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Name,theShape->ListShape[i].shape_SizeName, type);
            if(prevlen == theShape->ListShape[i].shape_SizeName)
			    accume += prevlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Comment,theShape->ListShape[i].shape_SizeComment, type);
            if(prevlen == theShape->ListShape[i].shape_SizeComment)
			    accume += prevlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Tpage,theShape->ListShape[i].shape_SizeTpage, type);
            if(prevlen == theShape->ListShape[i].shape_SizeTpage)
			    accume += prevlen;

            if (theShape->ListShape[i].align)
				oldshpdataalignment = theShape->ListShape[i].align;
            {
                OLDSHPHeaderStruct *oldshpout = (OLDSHPHeaderStruct *) theShape->ListShape[i].shape_Data;
                if ((oldshpout) && (oldshpout->type&0x80))
                    padlen = ((accume+8+oldshppackeddataalignment)&~oldshppackeddataalignment)-8-accume;    // deal with extra 8 byte
                else                                                                                  // codex header.
                    padlen = ((accume+oldshpdataalignment)&~oldshpdataalignment)-accume;
            }
            prevlen += padlen;
            OLDSHPWSetLink(&prevptr,&prevlen,theShape->ListShape[i].shape_Data,theShape->ListShape[i].shape_SizeData, type);

        }

        /* fix OLDVSH header now */

        if (type==TYPEOLDVSH)
        {
            oldvshheader = (char *) theShape->ListShape[i].shape_Data;
            if (oldvshheader)
            {
                tlink = ggetm(oldvshheader,4);
                memcpy(oldvshheader,oldvshheader+4,12);
                gputm(oldvshheader+12,tlink,4);
            }
            else
            {
                oldvshheader = (char *) theShape->ListShape[i].shape_Clut;
                if (oldvshheader)
                {
                    tlink = ggetm(oldvshheader,4);
                    memcpy(oldvshheader,oldvshheader+4,12);
                    gputm(oldvshheader+12,tlink,4);
                }
            }
        }
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
    char * ShapePtr = (char *)galloc(ShapeRowBytes*Height+sizeof(OLDSHPHeaderStruct));         // create a temp buffer to swizzle into.

    if(ShapePtr)
    {
        // clear the buffer for oldfsh 4bpp
        memset(ShapePtr,0,ShapeRowBytes*Height+sizeof(OLDSHPHeaderStruct));

        // special case stuff for OLDSSH  Functions Swizzle 8 or 4 and then unswizzle 32 for speed increase.
        if(type == TYPEOLDSSH)
        {
            if(Shapebpp == 8)
                OLDSHAPE_swizzle8((unsigned char *)MemPtr,(unsigned char *)ShapePtr,Width,Height);

            else if(Shapebpp == 4)
                OLDSHAPE_swizzle4((unsigned char *)MemPtr,(unsigned char *)ShapePtr,Width,Height);

            else if(Shapebpp == 16 || Shapebpp == 15)
                OLDSHAPE_swizzle16((unsigned char *)MemPtr,(unsigned char *)ShapePtr,Width,Height);

            else if(Shapebpp == 32)
                memcpy(ShapePtr,MemPtr,ShapeRowBytes*Height);
            else if(Shapebpp == 24)   // not supported
            {
                if(ShapePtr) gfree(ShapePtr);
                return (NULL);
            }

            return(ShapePtr);
        }

        // other formats.
        if(type == TYPEOLDGSH || type == TYPEOLDFSH || type == TYPEOLDXSH || type==TYPEOLDMSH)
        {
            // have to set the byte per pixel amount. Most of the 4bpp formats can
            // be treated the same as the 8bpp format in the unswizzle, only
            // oldfsh and oldpsh need special care.
            if(Shapebpp <= 8)       bypp = 1;
            else if(Shapebpp <= 16) bypp = 2;
            else if(Shapebpp <= 24) bypp = 3;
            else if(Shapebpp <= 32) bypp = 4;

            for (y=0;y<Height;++y)
            {
                MemOffset=MemRowbase;
                ShapeOffset=ShapeRowbase;

                for (x=0; x<Width; ++x)
                {
                    if (type==TYPEOLDGSH)
                    {
                        ShapeOffset = OLDSHPWswizzleGC(x,y,Width,Shapebpp);// this swizzles the dest not the src like PS2 swizzle
                    }
                    else if (type==TYPEOLDFSH || type==TYPEOLDXSH)
                    {
                        int PixOffset;
                        int tx,ty;

                        PixOffset = OLDSHPWswizzleOLDXSH(x,y,Width,Height);

                        ty = PixOffset/Width;
                        tx = PixOffset%Width;

                        // 4 bit kludge
                        if(ShapeType == 0x40 || ShapeType == 0x79 || ShapeType == 0x7a)
                            MemOffset = ty*ShapeRowBytes+tx/2;
                        else
                            MemOffset = ty*ShapeRowBytes+tx*bypp;            // do a regular pixel lookup
                    }
                    else if (type==TYPEOLDMSH)
                    {
                        int PixOffset;
                        int tx,ty;

                        PixOffset = OLDSHPWswizzleOLDMSH(x,y,Width,Height,Shapebpp);

                        ty = PixOffset/Width;
                        tx = PixOffset%Width;

                        // 4 bit kludge
                        if(ShapeType == 0x5c)
                            ShapeOffset = ty*ShapeRowBytes+tx/2;
                        else
                            ShapeOffset = ty*ShapeRowBytes+tx*bypp;            // do a regular pixel lookup
					}

                    // OLDGSH 32 bpp and
                    // OLDGSH uncompressed DXT have a funny swizzle type for greater cache locality.
                    if(type == TYPEOLDGSH && (ShapeType == 0x16 || ShapeType == 0x1e))
                    {
                        (*(ShapePtr+ShapeOffset+0))  = *(MemPtr+MemOffset+0);
                        (*(ShapePtr+ShapeOffset+1))  = *(MemPtr+MemOffset+1);
                        (*(ShapePtr+ShapeOffset+32)) = *(MemPtr+MemOffset+2);
                        (*(ShapePtr+ShapeOffset+33)) = *(MemPtr+MemOffset+3);
                    }

                    else if(ShapeType == 0x40 || ShapeType == 0x79 || ShapeType == 0x7a || ShapeType == 0x5c)                // oldfsh's (and oldpsh) have to have each nibble extracted
                    {                                                              // and reinserted. Rather ugly but it's the way it
                        Value=MemPtr[MemOffset];                                   // has to be done.
                        if (x&1)
                            ShapePtr[ShapeOffset] |= (char)(Value&0xF0);
                        else
                            ShapePtr[ShapeOffset] |= (char)(Value&0x0F);
                    }

                    else                                                           // all other formats can just use a memcpy.
                        memcpy(ShapePtr+ShapeOffset,MemPtr+MemOffset,bypp);

                    if(Shapebpp != 4 || (Shapebpp == 4 && x&1))                    // only incerment every second pass for the bpp types otherwise always increment.
                    {
                        MemOffset     += bypp;
                        ShapeOffset   += bypp;
                    }

                }
                ShapeRowbase += ShapeRowBytes;
                MemRowbase   += ShapeRowBytes;

            }
        }// if type == OLDGSH,OLDXSH,OLDFSH,OLDASH
        return(ShapePtr);
    }// if ShapePtr
    return(NULL);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// extract the details of writing a pixel at a given memory location into one function.
// this should hopefully allow for easier code maintainability.
inline static void OLDSHPWWritePixel(const GINFO * ginfo,unsigned char * MemPtr,unsigned char * ShapePtr,int *pMemOffset,int * oLdpshapeOffset,int type,int *px, int *py,int Width,int Height)
{
    int Value       = 0;
    int i           = 0;
    int x           = *px;
    int y           = *py;
    int MemOffset   = *pMemOffset;
    int ShapeOffset = *oLdpshapeOffset;
    switch (type)
    {
        case 0x44:      /* OLDPSH 1 bpp */
        case 0x60:      /* OLDNSH 1 bpp */
            Value = 0;
            for (i = 7; (i >= 0) && (x < Width); --i,++x)
            {
                Value |= (MemPtr[MemOffset++] & 0x01) << i;
            }
            ShapePtr[ShapeOffset++] = (char)Value;
            --x;
            break;

        case 0x01:      /* OLDSSH */
        case 0x18:      /* OLDGSH */
        case 0x5c:      /* OLDMSH */
        case 0x40:      /* OLDPSH */
        case 0x62:      /* OLDNSH 4-bit intensity/alpha */
        case 0x63:      /* OLDNSH 4-bit colour-indexed */
        case 0x79:
        case 0x7a:
            if (type==0x01  /* OLDSSH 4 bit */
             || type==0x40  /* OLDPSH 4 bit */
			 || type==0x5c  /* OLDMSH 4 bit */
             || type==0x79) /* OLDFSH 4 bit */
            {
                Value=MemPtr[MemOffset++]&15;
                if (++x<Width) Value |= (MemPtr[MemOffset++]&15)<<4;
            }
            else        /* OLDGSH/OLDNSH 4 bit */
            {
                Value=MemPtr[MemOffset++]<<4;
                if (++x<Width) Value |= (MemPtr[MemOffset++]&15);
            }

            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x10:      /* OLDGSH 4-bit intensity--look up grey scale. */
            if(ginfo->bpp == 8)
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
                Value=(int)OLDGSHWpixel8ToI((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
                Value <<= 4;
                ++x;
                if (x < Width)
                {
                    int v2;
                    v2=(int)OLDGSHWpixel8ToI((const ARGB *)(MemPtr+MemOffset));
                    Value |= (v2/16) & 0x0f;
                    MemOffset+=4;
                }
            }
            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x61:      /* OLDNSH 4-bit intensity--look up grey scale. */
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

        case 0x02:      /* OLDSSH 8 bit */
        case 0x19:      /* OLDGSH 8 bit */
        case 0x5d:      /* OLDMSH 8 bit */
        case 0x41:      /* OLDPSH 8 bit */
//        case 0x65:      /* OLDNSH 8 bit intensity/alpha */
//        case 0x66:      /* OLDNSH 8 bit colour-indexed */
            ShapePtr[ShapeOffset++] = MemPtr[MemOffset++];
            break;

        case 0x64:      /* OLDFSH/OLDXSH 8 bit alpha */
        if(ginfo->bpp == 8)
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


//                case 0x64:      /* OLDNSH 8 bit intensity--look up grey level. */
//                    Value = MemPtr[MemOffset++];
//                    ShapePtr[ShapeOffset++] = ginfo->colourtbl[Value].r;
//                    break;

        case 0x7b:      /* OLDFSH/OLDVSH 8 bit */
            Value = MemPtr[MemOffset++];
//                    if (ginfo->alphabits<=2 && ginfo->colourtbl[Value].a<25*255/100)   /* transparent with 24 bit clut */
//                        Value = 255;
            ShapePtr[ShapeOffset++] = (char) (unsigned char) Value;
            break;

        case 0x11:      /* OLDGSH 8-bit intensity format */
            if(ginfo->bpp == 8)
        {
                Value=(int)OLDGSHWpixel8ToI((const ARGB *)&(ginfo->colourtbl[*((unsigned char *)(MemPtr+MemOffset))]));
                MemOffset+=1;
        }
        else
        {
                Value=(int)OLDGSHWpixel8ToI((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
        }
            gputm(ShapePtr+ShapeOffset,Value,1);
            ShapeOffset+=1;
            break;

        case 0x12:      /* OLDGSH 8-bit intensity/alpha format*/
            if(ginfo->bpp == 8)
            {
                Value=(int)OLDGSHWpixel4ToIA((const ARGB *)&(ginfo->colourtbl[*((unsigned char *)(MemPtr+MemOffset))]));
                MemOffset+=1;
            }
            else
            {
                Value=(int)OLDGSHWpixel4ToIA((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
            }
            gputm(ShapePtr+ShapeOffset,Value,1);
            ShapeOffset+=1;
            break;
		case 0x6c: /* OLDFSH 16 bpp YUV and OLDFSH 16 bpp YUY2 formats not supported here */
		case 0x68:
			break;


        case 0x15:      /* OLDGSH 15 bit */
            Value=(int)OLDGSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputm(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x7e:      /* OLDFSH 15 bit */
            Value=(int)OLDFSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x03:      /* OLDSSH 15 bit */
        case 0x59:      /* OLDMSH 15 bit */
            Value=(int)OLDSSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

//        case 0x68:      /* OLDNSH 15-bit RGBA */
//            Value=(int)OLDNSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
//            gputm(ShapePtr+ShapeOffset,Value,2);
//            ShapeOffset+=2;
//            MemOffset+=4;
//            break;

        case 0x14:      /* OLDGSH 16 bit */
            Value=(int)OLDFSHWpixel8To565((const ARGB *)(MemPtr+MemOffset));
            gputm(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x58:      /* OLDMSH 16 bit */
            Value=(int)OLDMSHWpixel8To565((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x78:      /* OLDFSH 16 bit */
            Value=(int)OLDFSHWpixel8To565((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x65:      /* OLDXSH AL_88 */
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
                if(ginfo->redbits > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ((const ARGB *)(MemPtr+MemOffset))->r;
                else if(ginfo->greenbits > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ((const ARGB *)(MemPtr+MemOffset))->g;
                else if(ginfo->bluebits  > 1)
                    ((char *)(ShapePtr+ShapeOffset))[0] = ((const ARGB *)(MemPtr+MemOffset))->b;
                ((char *)(ShapePtr+ShapeOffset))[1] = ((const ARGB *)(MemPtr+MemOffset))->a;
                MemOffset+=4;
            }
            ShapeOffset+=2;
            break;

        case 0x67:      /* OLDXSH 655 bit */
            Value=(int)OLDFSHWpixel8To655((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x13:      /* OLDGSH 16-bit intensity/alpha embm format*/
            if(ginfo->bpp <= 8)
            {
                Value=(int)OLDGSHWpixel8ToIA((const ARGB *)&(ginfo->colourtbl[*((unsigned char *)(MemPtr+MemOffset))]));
                MemOffset+=1;
            }
            else
            {
                Value=(int)OLDGSHWpixel8ToIA((const ARGB *)(MemPtr+MemOffset));
                MemOffset+=4;
            }
            gputm(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            break;

        case 0x42:      /* OLDPSH 16 bit */
            Value=(int)OLDPSHWpixel8To5((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x5a:      /* OLDMSH 4444 */
            Value=(int)OLDMSHWpixel8To4444((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x6d:      /* OLDFSH 16 bit argb (4444) */
            Value=(int)OLDFSHWpixel8To4444((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,2);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x66:      /* OLDFSH 24 bit argb (6666) */
            Value=(int)OLDFSHWpixel8To6666((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,3);
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x6a:      /* OLDFSH 32 bit argb (2101010) */
            Value=(int)OLDFSHWpixel8To2101010((const ARGB *)(MemPtr+MemOffset));
            gputi(ShapePtr+ShapeOffset,Value,4);
            ShapeOffset+=4;
            MemOffset+=4;
            break;


        case 0x04:      /* OLDSSH 24 bit */
        case 0x43:      /* OLDPSH 24 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->b;
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x7f:      /* OLDFSH 24 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->b;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->r;
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x7d:      /* OLDFSH 32 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->b;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->a;
            ShapeOffset+=4;
            MemOffset+=4;
            break;

        case 0x05:      /* OLDSSH 32 bit */
        case 0x5b:      /* OLDMSH 32 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->b;
            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->a;
            ShapeOffset+=4;
            MemOffset+=4;
            break;

        case 0x16:      /* OLDGSH 32 bit */
            *(ShapePtr+ShapeOffset+0) = ((const ARGB *)(MemPtr+MemOffset))->a;
            *(ShapePtr+ShapeOffset+1) = ((const ARGB *)(MemPtr+MemOffset))->r;
            *(ShapePtr+ShapeOffset+2) = ((const ARGB *)(MemPtr+MemOffset))->g;
            *(ShapePtr+ShapeOffset+3) = ((const ARGB *)(MemPtr+MemOffset))->b;
            ShapeOffset+=4;
            MemOffset+=4;
            break;

//        case 0x6a:      /* OLDNSH 32 bit RGBA */
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
    *px           = x;                     // reset these values for next loop.
    *py           = y;
    *pMemOffset   = MemOffset;
    *oLdpshapeOffset = ShapeOffset;
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Write the actual Bitmap, deals with swizzling and different shapetypes.
void OLDSHPWWriteBitmap(OLDSHPHeaderStruct *HeaderPict,char *ShapePtr,const GINFO *ginfo,const char *SourceMemPtr,int MemRowBytes,int type, int subtype,int rw,int rh)
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
    Shapebpp=OLDSHPWGetOldshpbpp(HeaderPict,type);
    Width = ginfo->width;
    Height = ginfo->height;
    ShapeRowBytes=OLDSHPWCalcRowBytes(Width,Shapebpp,type);


    if(subtype & SUBSWIZZLE && (type == TYPEOLDGSH || type ==  TYPEOLDSSH || type ==  TYPEOLDMSH))        // do we have a logically bigger shape?
        ShapeRowBytes=OLDSHPWCalcRowBytes(rw,Shapebpp,type);


    Membpp = Shapebpp<=8?1:4;



    // be good and clear new shape buffer.
    if(subtype & SUBSWIZZLE && (type == TYPEOLDGSH || type ==  TYPEOLDSSH|| type ==  TYPEOLDMSH))    // have to worry about logically bigger shapes!
        memset(ShapePtr,0,ShapeRowBytes*rh);                             // OLDGSH has funny requirement of having larger shape in file
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
            OLDSHPWWritePixel(ginfo,MemPtr,(unsigned char *)ShapePtr,&MemOffset,&ShapeOffset,HeaderPict->type,&x, &y,Width,Height);

        ShapeRowbase += ShapeRowBytes; // Increment the pointer with a line align on 16 bits
        MemRowbase   += MemRowBytes;
    }



    // handle any swizzling that is necessary.
    if(subtype&SUBSWIZZLE)
    {
        if(type == TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH)   // logically bigger shape in some cases!
            temp = (unsigned char *)SwizzleAll((unsigned char *)ShapePtr,rw,rh,Shapebpp,HeaderPict->type,type,ShapeRowBytes);
        else
            temp = (unsigned char *)SwizzleAll((unsigned char *)ShapePtr,Width,Height,Shapebpp,HeaderPict->type,type,ShapeRowBytes);
        if(temp)                       // just in case I cannot swizzle it save the data and clean up.
        {
            if(type == TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH)
                memcpy(ShapePtr,temp,ShapeRowBytes*rh);
            else
                memcpy(ShapePtr,temp,ShapeRowBytes*Height);
            gfree(temp);
        }
    }


}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static void OLDSHPWReorderDXTBlocks(int width, int height, int rowbytes, const unsigned char *source, unsigned char *dest)
{
    int i;
    int j;
    int k;
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


//-------------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------------
// game cube specific function for reordering swizzled blocks... blocks have to
// be in 12 56
//       34 78  order across screen (Note 4x4 block ordering.
static void OLDSHPWReorderDXTOLDGSHBlocks(int realwidth, int realheight,int width, int height, int rowbytes, const unsigned char *source, unsigned char *dest)
{
    int i = 0;
    int j = 0;
    int k = 0;
    const unsigned char *s = 0;
    unsigned char *d = dest;
    unsigned char * temp = 0;         // destination swizzled pointer.
    int copywidth   = 0;
    int clearwidth  = 0;
    int copyheight  = 0;
    int clearheight = 0;

    for (i=0; i<realheight; i+=4)
    {

        /* If height is not a multiple of 4 and we're on the last row of blocks */
        if (i+4 > height)                          // figure out how much block padding I need.
        {
            copyheight = height-i;                 // how much of input to copy into block
            if(copyheight > 4)                     // clamp value to valid block range.
                copyheight = 4;
            if(copyheight < 0)
                copyheight = 0;
            clearheight = 4-copyheight;            // how much should I clear?
        }
        else                                       // copy the entire block.
        {
            copyheight = 4;
            clearheight = 0;
        }

        s = source + i*rowbytes;                   // setup my source pointer based on current height.
        for (j=0; j<realwidth; j+=4)               // loop over entire realwidth.
        {
            /* If width is not a multiple of 4 and we are on the last block of the line */
            if (j+4 > width)                       // figure out any width padding.
            {
                copywidth = width-j;               // calculate the padding.
                if(copywidth > 4)                  // clamp to valid block width.
                    copywidth = 4;
                if(copywidth < 0)
                    copywidth = 0;
                clearwidth = 4-copywidth;          // calculate the amount to clear of my current block.
            }
            else                                   // nope going to copy the entire block.
            {
                copywidth = 4;
                clearwidth = 0;
            }

            temp = (&d[(SwizzleDXT1GetIndex(j>>2,i>>2,(realwidth>>2))*64)]);        // find the offset of this swizzled block.
            /* Copy only what exists in the source */
            for (k=0; k<4; k++)                   // loop over my block
            {
                if(k < copyheight)                // if I have data here at this point and not padding.
                {

                    /* Copy only what exists in the source */
                    memcpy(temp, s + k*rowbytes, copywidth * 4);

                    /* Clear the rest */
                    if(clearwidth == 4)
                        memset(temp, 0, 4 * 4);
                    if (clearwidth)               // I might have padding here so fill in with pixel in upper right hand corner.
                    {
                        memset((temp + copywidth * 4), (int)(*s), clearwidth * 4);
                    }
                }
                else if(copyheight)               // otherwise if partially filled just fill in with single colour. NOTE I don't fill in transparent.
                    memset(temp, (int)(*s), 4 * 4);
                else
                    memset(temp, 0, 4 * 4);       // otherwise this is a pad block can totally be filled with 0's

                temp += 16;                       // go to next scan line on my current block.
            }
			// I don't necessarily want to move to the next block since it could be
            // entirely empty. If it is then I want to stay pointing at this block
            if(clearwidth == 0 && !(8-width+j >= 4))  // if the next block is not empty
            	s += 4 * 4;                           // move in my buffer to next tile pos.
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

static void YUVtoRGB(float &R, float &G, float &B, float Y, float U, float V)
{
    float C = Y - 16.0f;
    float D = U - 128.0f;
    float E = V - 128.0f;

    R = 1.164383f * C                   + 1.596027f * E  ;
    G = 1.164383f * C - (0.391762f * D) - (0.812968f * E) ;
    B = 1.164383f * C +  2.017232f * D                   ;
}

static float OLDRGBCompare(float R0,float G0,float B0,float R1,float G1,float B1)
{
    return 0.25678843f*(R0-R1)*(R0-R1) +
           0.50412965f*(G0-G1)*(G0-G1) +
           0.09790586f*(B0-B1)*(B0-B1);
}

#if 0
static float OLDf(float U,float V,float R0,float G0,float B0)
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

        float diff = OLDRGBCompare(R,G,B,R0,G0,B0);
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
static void OLDargb32toyuy2( unsigned char* pDst,
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

	float YC0 = 0.0f;//Y0 * 0.05f + 1.0f;
	float YC1 = 1.0f;//Y1 * 0.50f + 1.0f;
	float YC2 = 1.0f;//Y2 * 0.50f + 1.0f;
	float YC3 = 0.0f;//Y3 * 0.05f + 1.0f;

    float U=(U1+U2)/2.0f;
    float V=(V1+V2)/2.0f;

//    float U=(U0*YC0 + U1*YC1 + U2*YC2 + U3*YC3)/(YC0+YC1+YC2+YC3);
//    float V=(V0*YC0 + V1*YC1 + V2*YC2 + V3*YC3)/(YC0+YC1+YC2+YC3);

    // recalculate

//    Y1 = OLDf(U,V,R1,G1,B1);
//    Y2 = OLDf(U,V,R2,G2,B2);

//	Y1 = ((B1 - U1/0.493f) + (R1 - V1/0.877f))/2.0f;
//	Y2 = ((B2 - U2/0.493f) + (R2 - V2/0.877f))/2.0f;

	*(pDst + 0) = ROUNDCLAMPINT(Y1);
	*(pDst + 1) = ROUNDCLAMPINT(U);
	*(pDst + 2) = ROUNDCLAMPINT(Y2);
	*(pDst + 3) = ROUNDCLAMPINT(V);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// handles YUY2 encoding and swizzling, refpack handled elsewhere.
static int OLDSHPWWriteYUY2(const GINFO * ginfo, int rowbytes, const char * infileptr, char * outfileptr)
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

			OLDargb32toyuy2( d, s0,s1,s2,s3 );

            if(isodd && x == width-1)
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
static void rgbtoyum(YUMOLDCOORD *rgb, YUMOLDCOORD *yum)
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
    YUMOLDCOORD rgb;
    YUMOLDCOORD yum[4];

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
    YUMOLDCOORD rgb;
    YUMOLDCOORD yum[4];
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
    YUMOLDCOORD rgb;
    YUMOLDCOORD yum[4];
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
    if(yumbuf)
    {

        buf2 = (unsigned char *)SwizzleAll((unsigned char *)(outfileptr+(Width*Height)),Width/2,Height/2,8,ShapeType,type,Width/2);
        if(buf2)
        {
            buf3 = (unsigned char *)SwizzleAll((unsigned char *)(outfileptr+(Width*Height)+(Width*Height/4)),Width/2,Height/2,8,ShapeType,type,Width/2);
            if(buf3)
            {
                    memcpy(outfileptr,yumbuf,Width*Height);
                    memcpy(outfileptr+(Width*Height),buf2,Width*Height/4);
                    memcpy(outfileptr+(Width*Height)+(Width*Height/4),buf3,Width*Height/4);
                    if(yumbuf) gfree(yumbuf);
                    if(buf2)   gfree(buf2);
                    if(buf3)   gfree(buf3);
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
static int OLDSHPWWriteYUV(const GINFO * ginfo, int rowbytes, const char * infileptr, char * outfileptr,int YQuant,int CbQuant,int CrQuant,int subtype,int type,int ShapeType,int ShapeRowBytes,int rw,int rh)
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


    if(type==TYPEOLDGSH && subtype & SUBSWIZZLE)
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
        if(type==TYPEOLDGSH && subtype & SUBSWIZZLE)
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
    if(subtype & SUBSWIZZLE)                                                     // handle swizzling of data.
    {
        if(type == TYPEOLDGSH)
            DoYUVSwizzle(outfileptr,rw,rh,ShapeType,type,ShapeRowBytes);         // swizzles right into data buffer.
        else
            DoYUVSwizzle(outfileptr,width,height,ShapeType,type,ShapeRowBytes);  // swizzles right into data buffer.
    }
    return(datalen);                                                             // return the actual image size for storage purposes.
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Write S3 compressed texture. 4x4 block (can be swizzled OLDGSH have this) effectivly 16bpp
static int OLDSHPWWriteDXT(const GINFO *ginfo, int rowbytes, const char *source, char *dest, int type, int collapse)
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

    if(type == TYPEOLDGSH)
        datalen = (realwidth * realheight * 4);
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
        else if(ginfo->alphabits == 3)
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
        //if (type==TYPEOLDSSH)
        //    options[0] |= DXT_ENCODE_RGB_555;

        if(type == TYPEOLDGSH)
        {
            if (ginfo->alphabits > 0)  // include alpha information in the encode, slower but more accurate.
                /* DXT1 */
                options[0] = DXT_ENCODE_RGB_ALPHA_COMPARE| DXT_DXT1B_ENCODEOPTION;
            else
                /* DXT1 */
                options[0] = DXT_ENCODE_RGB_FULL| DXT_DXT1B_ENCODEOPTION;
            OLDSHPWReorderDXTOLDGSHBlocks(realwidth,realheight,ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, (unsigned char *)ordertemp);
        }
        else
            OLDSHPWReorderDXTBlocks(ginfo->width, ginfo->height, rowbytes, (unsigned char *)source, (unsigned char *)ordertemp);

        if (ginfo->packed==3) // S3TC (s3 code)
            packlen = OLDSHPALIGN(S3TC_encode(dest,ordertemp,datalen,options));
        else    // DXT (ea code)
            packlen = OLDSHPALIGN(DXT_encode(dest,ordertemp,datalen,options));

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
static int OLDSHPWWriteMG(const GINFO *ginfo, int rowbytes, const char *source, char *dest)
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
static int OLDSHPWintlog2(unsigned int n)
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
static int OLDSHPWSetShape(OLDSHPHeaderStruct *HeaderPict,const GINFO *ginfo,int bpp,const char *source,int rowbytes,int nooldshp,int type,int isI,int isIA)
{
    int oldshplen;
    int packlen;
    int refpacklen;
    int realwidth;
    int realheight;
    /* int kpnpacklen; */
    void *packtemp;
    int oldshptype;
    int subtype=ginfo->subtype;
    int dxt = 0;
    int mg = 0;
    int collapse = !(ginfo->subtype&SUBNODXTCOLLAPSE);
    int oldgshembm  = type==TYPEOLDGSH && ginfo->subtype&SUBEMBM;
    memset(HeaderPict,0,sizeof(OLDSHPHeaderStruct));

	bool dxtenable=false;
	if (type==TYPEOLDXSH || type==TYPEOLDFSH || type==TYPEOLDGSH || (subtype & SUBADVANCED))
		dxtenable = true;

    /* Special case for DXT encoding */
    if ( dxtenable && (ginfo->packed == 3 || ginfo->packed == 5) && (bpp > 8) && (ginfo->bpp > 8) && !isIA && !isI)
    {
        if (ginfo->alphabits >= 4)      // DXT3
        {
            bpp = 16;
            dxt = 3;
        }
        else if (ginfo->alphabits > 1)  // DXT5
        {
            bpp = 16;
            dxt = 5;
        }
        else
        {
            bpp = 16;
            dxt = 1;
        }
    }
    /* Special case for MG encoding */
    if ( (ginfo->packed == 4) && (bpp > 8) )
    {
        bpp = 16;
        if (ginfo->redbits>5 || ginfo->greenbits>6 || ginfo->greenbits>5)  // more than 16 bit
            bpp = 24;
        if (ginfo->alphabits>1)       // has alpha channel
            bpp = 32;
        mg = 1;
    }


    if (type==TYPEOLDPSH) /* oldpsh */
    {
        switch (bpp)
        {
            default:
            case 4:  oldshptype=0x40; break;
            case 8:  oldshptype=0x41; break;
            case 32:
            case 15:
            case 16: oldshptype=0x42; break;
            case 24: oldshptype=0x43; break;
        }
    }
    else if (type==TYPEOLDSSH) /* oldssh */
    {
        switch (bpp)
        {
            default:
            case 4:  oldshptype=0x01; break;
            case 8:  oldshptype=0x02; break;
            case 15:
            case 16: if(ginfo->alphabits > 2)   // possibly support 3 level transparency so 2 bits of accuracy here.
                         oldshptype=0x05;          // set to 32bpp mode if this is the case.
                     else if(ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                         oldshptype=0x6c;          // YUV (YUI) format
                     else
                         oldshptype=0x03;          // else set to 16bpp mode.
                     break;
            case 24: oldshptype=0x04; break;
            case 32: oldshptype=0x05; break;
        }
        if(bpp == 24 && (subtype & SUBADVANCED))
        {
            oldshptype=0x04;
        }
    }
    else if (type==TYPEOLDASH) /* oldash */
    {
        switch (bpp)
        {
            default:
            case 4:  oldshptype=0x01; break;
            case 8:  oldshptype=0x02; break;
            case 15:
            case 16: oldshptype=0x03; break;
        }
    }
    else if (type==TYPEOLDGSH) /* oldgsh */
    {
        if ((oldgshembm || isIA == 8))
            oldshptype=0x13;     // set to IA8 format for embm maps and oldgsh.
        else if ((isIA == 4))
            oldshptype=0x12;     // set to IA4 format.
        else if((isI == 8)&&(subtype&SUBADVANCED))
            oldshptype=0x11;     // set to I8  format.
        else if((isI == 4)&&(subtype&SUBADVANCED))
            oldshptype=0x10;     // set to I4  format.
        else
        {
            switch (bpp)
            {
                default:
                case 4:  oldshptype=0x18; break;
                case 8:  oldshptype=0x19; break;
                case 15:
                case 16:
                    if (ginfo->alphabits>=1 && !dxt) // default to 5a3 if we have any alpha since all other formats don't keep alpha.
                        oldshptype=0x15;   /* 5a3 (3444) */
                    else if(ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                        oldshptype=0x1c;   // YUV (YUM) format.
                    else
                        oldshptype=0x14;   /* 0565 */
                    break;
                case 24:
                case 32: oldshptype=0x16;  /* 8888 */
            }
        }
    }
    else if (type==TYPEOLDMSH) /* oldmsh */
    {
        switch (bpp)
        {
            default:
            case 4:  oldshptype=0x5c; break;
            case 8:  oldshptype=0x5d; break;
            case 15:
            case 16:
                if (ginfo->alphabits>=4 && !dxt)
                    oldshptype=0x5a;   /* 4444 */
                else if (ginfo->greenbits>=6 || dxt)
                    oldshptype=0x58;   /* 565 */
                else
                    oldshptype=0x59;   /* 1555 */
                break;
            case 32: oldshptype=0x5b; break;
        }
    }
    else if (type==TYPEOLDXSH) /* oldxsh */
    {
        switch (bpp)
        {
            default:
            case 8:  oldshptype=0x7b; break;
            case 15:
            case 16:
                if(ginfo->alphabits>=4 && !dxt)
                    oldshptype=0x6d;   /* 4444 */
                else if(ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                    oldshptype=0x6c;   // YUV (YUI) format.
                else if(ginfo->alphabits == 0 && ginfo->redbits == 4 && ginfo->greenbits == 8 && ginfo->bluebits == 4)
                    oldshptype=0x68;   // YUY2 format.
                else if (ginfo->greenbits>=6 || dxt)
                    oldshptype=0x78;   /* 565 */
                else if (ginfo->redbits>=6)
                    oldshptype=0x67;   /* 655 */
                else
                    oldshptype=0x7e;   /* 1555 */
                break;
            case 24: oldshptype=0x7f; break;
            case 32: oldshptype=0x7d; break;

        }
        if (isIA == 8)
            oldshptype=0x65;   /* IA8 */
        else if ((isI == 8))
            oldshptype=0x64;   /* A8  */

    }
    else    /* oldfsh or oldvsh */
    {
        switch (bpp)
        {
            default:
            case 4:  oldshptype=0x79; break;  // 7a is hi/lo
            case 8:  oldshptype=0x7b; break;
            case 15:
            case 16:
                if (ginfo->alphabits>=4 && !dxt)
                    oldshptype=0x6d;   /* 4444 */
                else if(ginfo->alphabits == 0 && ginfo->redbits == 2 && ginfo->greenbits == 8 && ginfo->bluebits == 2)
                    oldshptype=0x6c;   // YUV (YUI) format.
                else if(ginfo->alphabits == 0 && ginfo->redbits == 4 && ginfo->greenbits == 8 && ginfo->bluebits == 4)
                    oldshptype=0x68;   // YUY2 format.
                else if (ginfo->greenbits>=6 || dxt)
                    oldshptype=0x78;   /* 565 */
                else if (ginfo->redbits>=6)
                    oldshptype=0x67;   /* 655 */
                else
                    oldshptype=0x7e;   /* 1555 */
                break;
            case 24:
                if (ginfo->alphabits>=6 && ginfo->subtype&SUBADVANCED)
                    oldshptype=0x66;   /* 6666 */
                else
                    oldshptype=0x7f;
                break;
            case 32:
                if (ginfo->alphabits<=2 && ginfo->subtype&SUBADVANCED)
                    oldshptype=0x6a;   /* 2101010 */
                else
                    oldshptype=0x7d; break;
                break;
        }
        if (isIA == 8)
            oldshptype=0x65;   /* IA8 */
        else if ((isI == 8))
            oldshptype=0x64;   /* A8  */
    }

    if (dxt && collapse)
    {
        if (type==TYPEOLDGSH)
            oldshptype = 0x1e; // CMPR (DXT1)
        else if (type==TYPEOLDXSH || type==TYPEOLDFSH || type==TYPEOLDSSH || type==TYPEOLDASH)
        {
            switch (dxt)
            {
                default:
                case 1:  oldshptype=0x60; break; // DXT1
                case 3:  oldshptype=0x61; break; // DXT3
                case 5:  oldshptype=0x62; break; // DXT5
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

        if (subtype & SUBSWIZZLE)
        {
            if (type==TYPEOLDXSH || type==TYPEOLDFSH)
            {
                if ((width!=(1<<OLDSHPWintlog2(width)))
                 || (height!=(1<<OLDSHPWintlog2(height))))
                    subtype &= ~SUBSWIZZLE;
            }
            else if (type==TYPEOLDSSH)
            {

                if (!OLDCanSwizzleSsh(width,height,bpp)
                 || (bpp>=24)   // disable swizzle on 24/32 bpp
#if SWIZZLE4BUG
                 || (bpp<=4 && width!=height)
#endif
                )   // disable swizzle on 4 bpp due to issue with mipmap collapse
				{
                    subtype &= ~SUBSWIZZLE;
				}

                // calculate for lower than 16x16 or 32x32 sized textures realwidth and realheight.
                RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
            }
            else if (type==TYPEOLDMSH && !dxt)
            {
                RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
            }
            else if (type==TYPEOLDGSH && !dxt)
            {
                RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
            }
            else
            {
                subtype &= ~SUBSWIZZLE; // disable swizzle on platforms that do not support it
            }
        }
        // dxt textures for oldgsh are considered swizzled since their blocks are
		// swizzled by default, turn on swizzle bit to be nice to EAGL.
		if (dxt)
        {
            if (type==TYPEOLDGSH)
    			subtype |= SUBSWIZZLE;
            else
                subtype &= ~SUBSWIZZLE;
        }

        if ((bpp==24) // disable swizzle on 24 bpp
         || (bpp==4 || oldshptype==0x68) && (type==TYPEOLDXSH || type==TYPEOLDFSH))
            subtype &= ~SUBSWIZZLE; // disable swizzle on 4 bpp & YUY2

        shapex &= 0x0fff;
        if (subtype & SUBROTATE)    shapex |= 0x8000;
        if (subtype & SUBTRANSPOSE) shapex |= 0x4000;
        if (subtype & SUBSWIZZLE)   shapex |= 0x2000;
        shapey &= 0x0fff; // upper 4 bits for mipmap count


        OLDSHPPUTFIELD(HeaderPict->type,oldshptype);
        OLDSHPPUTFIELD(HeaderPict->width,width);
        OLDSHPPUTFIELD(HeaderPict->height,height);
        OLDSHPPUTFIELD(HeaderPict->centerx,ginfo->centerx);
        OLDSHPPUTFIELD(HeaderPict->centery,ginfo->centery);
        OLDSHPPUTFIELD(HeaderPict->shapex,shapex);
        OLDSHPPUTFIELD(HeaderPict->shapey,shapey);
    }

    /* Special case for DXT encoding */
    if (dxt)
    {
        oldshplen = OLDSHPWWriteDXT(ginfo, rowbytes, source, (char *)HeaderPict+16, type, collapse);
        if (!collapse)
            HeaderPict->type |= 0x80;   /* mark as packed */
    }
    else if(oldshptype == 0x1c || oldshptype == 0x6c) // YUV format.
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
        oldshplen = OLDSHPWWriteYUV(ginfo, rowbytes, source, (char *)HeaderPict+16,YQuant,CbQuant,CrQuant,subtype,type,oldshptype,rowbytes,realwidth,realheight);
    }
	else if(oldshptype == 0x68) // YUY2 format
		oldshplen = OLDSHPWWriteYUY2(ginfo, rowbytes, source, (char *)HeaderPict+16);
    else if (mg)
    {
        oldshplen = OLDSHPWWriteMG(ginfo, rowbytes, source, (char *)HeaderPict+16);
        HeaderPict->type |= 0x80;   /* mark as packed */
    }
    else
    {
        OLDSHPWWriteBitmap(HeaderPict,(char *)HeaderPict+16,ginfo,source,rowbytes,type,subtype,realwidth,realheight);
        if((type == TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH) && (subtype & SUBSWIZZLE))   // if I am working with a swizzled OLDGSH then I have to work with fake sizes to accomodate padding.
            oldshplen = OLDSHPWCalcSizeShape(realwidth,realheight,bpp,type,dxt);
        else
            oldshplen = OLDSHPWCalcSizeShape(ginfo->width,ginfo->height,bpp,type,dxt);
    }

    if (ginfo->packed == 2)
    {
        packtemp = galloc((int)(oldshplen*1.2+1024));
        if (packtemp)
        {
            refpacklen = OLDSHPALIGN(REF_encode(packtemp,(char *)HeaderPict+16,oldshplen,0));
            packlen = refpacklen;

            if (packlen<oldshplen)
            {
                memcpy((char *)HeaderPict+16,packtemp,packlen);
                oldshplen = packlen;
                HeaderPict->type |= 0x80;   /* mark as packed */
            }
            gfree(packtemp);
        }
    }

    oldshplen = OLDSHPALIGN(16+oldshplen);
    return(oldshplen);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// encode the clut in the correct format and stuff the buffer with the clut.
static void OLDSHPWSetShapeClut(const GINFO *ginfo,int NbColours,const ARGB *colourtbl,OLDClutHeaderStruct **theClut,int *SizeClut,int noclut,int type)
{
    int  i;
    int  j;
    int  csm1=0;
    int  clutx;
    int  cluty;
    int  cluttype;
    char *pal;
    int SizeData=0;
    OLDClutHeaderStruct *DataClut=0L;

    if (NbColours>0 && !noclut)
    {
        SizeData = NbColours;
        switch (type)
        {
            case TYPEOLDPSH:     /* OLDPSH */
                cluttype = 0x23;
                SizeData *= 2;      /* 16-bit palette */
                break;

            case TYPEOLDGSH:     /* OLDGSH */
            {
                if (ginfo->subtype&SUBADVANCED)
                    cluttype=oldgshadvancedcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                else
                    cluttype=oldgshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);

                if (cluttype==0x33)
                    SizeData = ((SizeData*2+31)&~31)+SizeData*2;  /* 32 bit palette of 2 IAs, with 32 byte alignment for 2nd clut */
                else
                    SizeData *= 2;  /* 16 bit palette */
                break;
            }

            case TYPEOLDMSH:     /* OLDMSH */
            {
                cluttype=oldmshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                if (cluttype==0x38 || /* 565  */
					cluttype==0x39 || /* 1555 */
					cluttype==0x3a)   /* 4444 */
                    SizeData *= 2;  /* 16 bit palette */
                else /* type 3b */
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEOLDSSH:     /* OLDSSH */
            {
                cluttype=oldsshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                if (ginfo->originalbpp>4)
                {
                    if (!(ginfo->subtype&SUBCSM2))
                        csm1 = 1;
                    SizeData = OLDSHPWcsmsize(SizeData);
                }
                if (cluttype==0x20)
                    SizeData *= 2;  /* 16 bit palette */
                else /* type 21 */
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEOLDASH:     /* OLDASH */
            {
                cluttype=0x20;
                SizeData *= 2;  /* 16 bit palette */
                break;
            }

            case TYPEOLDXSH:     /* OLDXSH */
            {
                cluttype=oldxshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                if (cluttype==0x29 || cluttype==0x2d)
                    SizeData *= 2;  /* 16 bit palette */
                else
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEOLDFSH:     /* OLDFSH */
            {
                if (ginfo->subtype&SUBADVANCED)
                    cluttype=oldfshadvancedcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
                else
                    cluttype=oldfshcluttype(ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);

                if (cluttype==0x29 || cluttype==0x2d)
                    SizeData *= 2;  /* 16 bit palette */
                else if (cluttype==0x22 || cluttype==0x24)
                    SizeData *= 3;  /* 24 bit palette */
                else
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case TYPEOLDVSH:     /* OLDVSH */
                cluttype = 0x22;
                SizeData *= 3;      /* 24-bit palette */
                break;

            case TYPEOLDNSH:     /* OLDNSH */
            {
                cluttype = 0x27;

                /* Intensity or intensity/alpha shapes have
                   no CLUT. If all colours in the palette are
                   grey scale then assume this is the case. */

                if (OLDNSHWIsGreyPalette(colourtbl, NbColours, ginfo->subtype&SUBNOPAL))
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
            SizeData = OLDSHPALIGN(sizeof(OLDSHPHeaderStruct)+SizeData);
            DataClut = (OLDClutHeaderStruct*) galloc(SizeData);
        }

        if (DataClut)
        {
            clutx = 0;
            cluty = 0;
            if (type==TYPEOLDPSH)
                cluty = 240;
            if ((ginfo->signature==GMAKEID('.','T','I','M') || ginfo->signature==GMAKEID('S','H','P','P'))
              && ginfo->size==sizeof(GINFOOLDSHP))
            {
                clutx = ((GINFOOLDSHP *) ginfo)->clutx;
                cluty = ((GINFOOLDSHP *) ginfo)->cluty;
            }
            if (csm1 || cluttype==0x33)
                clutx |= 0x2000;    // swizzled clut

            memset(DataClut,0,SizeData);

            OLDSHPPUTFIELD(DataClut->type,     cluttype);
            OLDSHPPUTFIELD(DataClut->width,    NbColours);
            OLDSHPPUTFIELD(DataClut->height,   1);
            OLDSHPPUTFIELD(DataClut->centerx,  NbColours);
            OLDSHPPUTFIELD(DataClut->centery,  0);
            OLDSHPPUTFIELD(DataClut->shapex,   clutx);
            OLDSHPPUTFIELD(DataClut->shapey,   cluty);

            pal = (char *) &DataClut->Entries[0];

            for (j=0; j<NbColours; ++j)
            {
                i = (csm1 && (j&31)>=8 && (j&31)<=23) ? j ^ 0x18 : j;
                switch (type)
                {
                    case TYPEOLDPSH: /* OLDPSH */
                        gputi(pal+i*2,OLDPSHWpixel8To5(&colourtbl[j]),2);
                        break;

                    case TYPEOLDFSH: /* OLDFSH */
                    case TYPEOLDXSH: /* OLDXSH */
                    case TYPEOLDGSH: /* OLDGSH */
                    case TYPEOLDSSH: /* OLDSSH */
                    case TYPEOLDASH: /* OLDASH */
                    case TYPEOLDMSH: /* OLDMSH */
                        switch (cluttype)
                        {
                            case 0x20: /* OLDSSH/OLDASH 15 bit palette (1555) */
                                gputi(pal+i*2,OLDSSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x32: /* OLDGSH 15 bpp */
                                gputm(pal+i*2,OLDGSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x2d: /* OLDFSH 15 bit palette (1555) */
                                gputi(pal+i*2,OLDFSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x38: /* OLDMSH 16 bit palette (565) */
                                gputi(pal+i*2,OLDMSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x39: /* OLDMSH 15 bit palette (1555) */
                                gputi(pal+i*2,OLDSSHWpixel8To5(&colourtbl[j]),2);
                                break;

							case 0x3a: /* OLDMSH 16 bit palette (4444) */
                                gputi(pal+i*2,OLDMSHWpixel8To4444(&colourtbl[j]),2);
                                break;

                            case 0x31: /* OLDGSH 565 */
                                gputm(pal+i*2,OLDFSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x29: /* OLDFSH 16 bit palette (565) */
                                gputi(pal+i*2,OLDFSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x30: /* OLDGSH IA8 */
                                gputm(pal+i*2,(int)OLDGSHWpixel8ToIA(&colourtbl[j]),2);
                                break;

                            case 0x33: /* OLDGSH 32 bit palette (8888) as 2 IA cluts */
                                pal[i*2+0] = colourtbl[j].a;
                                pal[i*2+1] = colourtbl[j].r;
                                pal[i*2+((NbColours*2+31)&~31)+0] = colourtbl[j].g;
                                pal[i*2+((NbColours*2+31)&~31)+1] = colourtbl[j].b;
                                break;

                            case 0x22: /* OLDFSH 24 bit palette (666) */
                                pal[i*3+0] = colourtbl[j].r>>2;
                                pal[i*3+1] = colourtbl[j].g>>2;
                                pal[i*3+2] = colourtbl[j].b>>2;
                                break;

                            case 0x24: /* OLDFSH 24 bit palette (888) */
                                pal[i*3+0] = colourtbl[j].r;
                                pal[i*3+1] = colourtbl[j].g;
                                pal[i*3+2] = colourtbl[j].b;
                                break;

                            case 0x2e: /* OLDFSH 32 bit palette (8555) */
                                gputi(pal+i*4,OLDFSHWpixel8To5(&colourtbl[j]),2);
                                pal[i*4+2] = 0;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x2c: /* OLDFSH 32 bit palette (8565) */
                                gputi(pal+i*4,OLDFSHWpixel8To565(&colourtbl[j]),2);
                                pal[i*4+2] = 0;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x2a: /* OLDFSH 32 bit palette (8888) */
                                pal[i*4+0] = colourtbl[j].b;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].r;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x3b: /* OLDMSH 32 bit palette (8888) */
                                pal[i*4+0] = colourtbl[j].r;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].b;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x21: /* OLDSSH 8888 */
                                pal[i*4+0] = colourtbl[j].r;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].b;
                                pal[i*4+3] = colourtbl[j].a;
                                break;
                        }
                        break;

                    case TYPEOLDVSH: /* OLDVSH */
                        pal[i*3+0] = colourtbl[j].r>>2;
                        pal[i*3+1] = colourtbl[j].g>>2;
                        pal[i*3+2] = colourtbl[j].b>>2;
                        break;

                    case TYPEOLDNSH: /* OLDNSH */
                        gputm(pal+i*2,OLDNSHWpixel8To5(&colourtbl[j]),2);
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
static void OLDSHPWSetShapeHotSpot(const GINFO *ginfo,OLDHOTHeaderStruct **theHotSpot,int *SizeHotSpot,int nohot, int type)
{
    OLDHOTHeaderStruct*    DataHotSpot=0L;
    int                SizeData=0L;
    int                i;

    if (ginfo->numhotspots>0 && !nohot)
    {
        SizeData=OLDSHPALIGN(8+ginfo->numhotspots*8); /* NumHotSpot*8 */
        DataHotSpot=(OLDHOTHeaderStruct*)galloc(SizeData);
        if (DataHotSpot)
        {
            memset(DataHotSpot,0,SizeData);
            DataHotSpot->type=0x7c;
            OLDSHPPUTFIELD(DataHotSpot->NumHotSpot,ginfo->numhotspots);

            for (i=0; i<ginfo->numhotspots; ++i)
            {
                OLDSHPPUTFIELD(DataHotSpot->ListHotSpot[i*2],    ginfo->hotspottbl[i][0]);
                OLDSHPPUTFIELD(DataHotSpot->ListHotSpot[(i*2)+1],ginfo->hotspottbl[i][1]);
            }
        }
    }
    *SizeHotSpot=SizeData;
    *theHotSpot=DataHotSpot;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// write palette animation information.
static void OLDSHPWSetShapeAnim(const GINFO *ginfo,OLDAnimHeaderStruct **theAnim,int *SizeAnim,int noanim, int type)
{
    int i;
    int SizeData=0L;
    OLDAnimHeaderStruct *DataAnim=0L;
    GINFOOLDSHP *ginfooldshp = (GINFOOLDSHP *)ginfo;

    if ((ginfo->signature==GMAKEID('I','L','B','M') || ((ginfo->signature&~255)==GMAKEID('S','H','P',0)))
      && ginfo->size==sizeof(GINFOOLDSHP))
    {
        if (ginfooldshp->numanims>0 && !noanim)
        {
            SizeData=OLDSHPALIGN(8+ginfooldshp->numanims*8);
            DataAnim=(OLDAnimHeaderStruct*)galloc(SizeData);
            if (DataAnim)
            {
                memset(DataAnim,0,SizeData);
                DataAnim->type=0x6e;
                OLDSHPPUTFIELD(DataAnim->NumAnim,ginfooldshp->numanims);

                for (i=0; i<ginfooldshp->numanims; ++i)
                {
                      ((signed char *)(DataAnim->ListAnim+i))[0] = ginfooldshp->palanim[i].dir;
                      ((signed char *)(DataAnim->ListAnim+i))[1] = ginfooldshp->palanim[i].speed;
                    ((unsigned char *)(DataAnim->ListAnim+i))[2] = ginfooldshp->palanim[i].start;
                    ((unsigned char *)(DataAnim->ListAnim+i))[3] = ginfooldshp->palanim[i].end;
                }
            }
        }
    }
    *SizeAnim=SizeData;
    *theAnim=DataAnim;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// set the shape texture page information.
static void OLDSHPWSetShapeTpage(const GINFO *ginfo,OLDTpageHeaderStruct **theTpage,int *SizeTpage,int notpage, char type)
{
    int SizeData=0L;
    OLDTpageHeaderStruct *DataTpage=0L;
    GINFOOLDSHP *ginfooldshp = (GINFOOLDSHP *)ginfo;

    if ((ginfo->signature==GMAKEID('I','L','B','M') || ((ginfo->signature&~255)==GMAKEID('S','H','P',0)))
      && ginfo->size==sizeof(GINFOOLDSHP))
    {
        if ( !notpage && !((ginfooldshp->tpage == 0) && (ginfooldshp->code == 0)) )
        {
            SizeData=OLDSHPALIGN(sizeof(OLDTpageHeaderStruct));
            DataTpage=(OLDTpageHeaderStruct*)galloc(SizeData);
            if (DataTpage)
            {
                memset(DataTpage,0,SizeData);
                DataTpage->type=0x6b;
                OLDSHPPUTFIELD(DataTpage->code,ginfooldshp->code);
                OLDSHPPUTFIELD(DataTpage->clutid,(ginfooldshp->cluty<<6)|((ginfooldshp->clutx>>4)&0x3f));
                OLDSHPPUTFIELD(DataTpage->tpage,(((int)type&0x3)<<7) | (ginfooldshp->tpage&0x0060) | ((ginfo->defaulty&0x100)>>4) | ((ginfo->defaultx&0x3ff)>>6) | ((ginfo->defaulty&0x200)<<2));
            }
        }
    }
    *SizeTpage=SizeData;
    *theTpage=DataTpage;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// set the shape meta information. This hold information about extended types such as cubemap embm etc.
static void OLDSHPWSetShapeMetal(const GINFO *ginfo,OLDMetalHeaderStruct **theMetal,int *SizeMetal, int type)
{
    int SizeData=0L;
    OLDMetalHeaderStruct *DataMetal=0L;
    int metalbinsize=64;

    if (type==TYPEOLDSSH)
        metalbinsize=240; // was 86;

    if (ginfo->subtype&(SUBDOT3|SUBEMBM|SUBCUBEMAP|SUBMIPMONT|SUBMETALBIN|SUBOPAQUE|SUBTRANSPARENT|SUBHALFALPHA))
    {
        SizeData=sizeof(OLDMetalHeaderStruct);
        if (ginfo->subtype&SUBMETALBIN && type != TYPEOLDGSH)
        {
            SizeData += metalbinsize; // eagl metalbin area
        }

        SizeData = OLDSHPALIGN(SizeData);
        DataMetal=(OLDMetalHeaderStruct*)galloc(SizeData);
        if (DataMetal)
        {
            unsigned int metal=0;
            memset(DataMetal,0,SizeData);
            DataMetal->type=0x69;
            if (ginfo->subtype&SUBDOT3)
                metal |= 1;
            if (ginfo->subtype&SUBEMBM)
                metal |= 2;
            if (ginfo->subtype&SUBCUBEMAP)
                metal |= 4;
            if (ginfo->subtype&SUBMIPMONT)
                metal |= 8;
            if (ginfo->subtype&SUBMETALBIN && type != TYPEOLDGSH)
            {
                metal |= 16;
                if (type==TYPEOLDSSH)
                    strcpy((char *)DataMetal+sizeof(OLDMetalHeaderStruct),
                        "EAGL240 metal bin attachment for runtime texture management");
                else
                    strcpy((char *)DataMetal+sizeof(OLDMetalHeaderStruct),
                        "EAGL64 metal bin attachment for runtime texture management");

// was "PS2 Metal Bin is an 86 byte attachment for a runtime structure. See OLDSHAPE_metalbin");

            }
            if (ginfo->subtype&SUBOPAQUE)
                metal |= 32;
            if (ginfo->subtype&SUBTRANSPARENT)
                metal |= 64;
            if (ginfo->subtype&SUBHALFALPHA)
                metal |= 128;

            if (ginfo->subtype&SUBMETALBIN && type != TYPEOLDGSH)
                OLDSHPPUTFIELD(DataMetal->metalbinsize,metalbinsize);
            OLDSHPPUTFIELD(DataMetal->metal,metal);
        }
    }

    *SizeMetal=SizeData;
    *theMetal=DataMetal;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// set the comment information for the current shape.
static void OLDSHPWSetShapeComment(const GINFO *ginfo,OLDCOMMEntheaderStruct **theComment,int *SizeComment,int nocomment, int type)
{
    OLDCOMMEntheaderStruct *DataComment=0L;
    int len;
    int SizeData=0L;

    if (ginfo->comment[0] && !nocomment)
    {
        len = strlen(ginfo->comment);
        SizeData=OLDSHPALIGN(8+len+1);
        DataComment=(OLDCOMMEntheaderStruct*)galloc(SizeData);
        if (DataComment)
        {
            memset(DataComment,0,SizeData);
            DataComment->type=0x6f;
            OLDSHPPUTFIELD(DataComment->SizeComment,len+1);
            strcpy((char *)DataComment->ListComment,ginfo->comment);
        }
    }
    *SizeComment=SizeData;
    *theComment=DataComment;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// name usually is the filename of the file converted to shape, or some other description.
static void OLDSHPWSetShapeName(const GINFO *ginfo,OLDnameheaderStruct **theName,int *SizeName,int noname)
{
    OLDnameheaderStruct *DataName=0L;
    int len;
    int SizeData=0L;

    if (strlen(ginfo->framename)>0 && !noname) // eagl and streaming shapes really want names all the time
    {
        len = strlen(ginfo->framename);
        SizeData=OLDSHPALIGN(4+len+1);
        DataName=(OLDnameheaderStruct*)galloc(SizeData);
        if (DataName)
        {
            memset(DataName,0,SizeData);
            DataName->type=0x70;
            strcpy((char *)DataName->ListName,ginfo->framename);
        }
    }
    *SizeName=SizeData;
    *theName=DataName;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// general function that fills all of the shape links with the apropriate information.
int OLDSHPWAddShape(OLDShapeListPtr theShape,const GINFO * tginfo, const char *source, int rowbytes, int framenum, int type, int subtype)
{
    OLDSHPHeaderStruct *   PictureShape=0;
    OLDClutHeaderStruct *  ClutShape=0;
    OLDMetalHeaderStruct * MetalShape=0;
    OLDHOTHeaderStruct *   HotSpotShape=0;
    OLDAnimHeaderStruct *  AnimShape=0;
    OLDCOMMEntheaderStruct * CommentShape=0;
    OLDnameheaderStruct *  NameShape=0;
    OLDTpageHeaderStruct * TpageShape=0;
    GINFO * ginfo    = (GINFO *)galloc(tginfo->size);
    memcpy(ginfo,tginfo,tginfo->size);
    int  bpp         = 0;
    int  i           = 0;
    int  ok          = 0;
    int SizeShape    = 0;
    int SizeClut     = 0;
    int SizeMetal    = 0;
    int SizeHotSpot  = 0;
    int SizeAnim     = 0;
    int SizeComment  = 0;
    int SizeName     = 0;
    int SizeTpage    = 0;
    int isIA         = 0;
    int isI          = 0;
    int nopalettevar = 0;
    int dxt          = 0;
    char shapeid[8];
    char clutid[8];

	OLDSHPGstValidate(ginfo,type);
    switch (type)
    {
        case TYPEOLDPSH: /* OLDPSH */
            bpp=4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=16;
            if (ginfo->originalbpp>16) bpp=24;
            break;

        case TYPEOLDFSH: /* OLDFSH */
            bpp=4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>16) bpp=24;
            if (ginfo->originalbpp>24) bpp=32;
            // for now do a check if 8888 OR if 8000. We don't support 8000 yet at the
            // command line but hopefully soon we will!

            // check for: AL_88 FORMAT
            // 8800 8080 8008 formats
            if((ginfo->alphabits > 4 && ginfo->packed < 3 ) && (
               (ginfo->redbits > 4 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits > 4 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo-> bluebits > 4) ))
            {
                bpp  = 16;
                isIA = 8;
                isI  = 0;
                nopalettevar = 1;
            }
            // A_8 format?
            // 8000
            else if((ginfo->alphabits > 4  && ginfo->redbits == 0 && ginfo->bluebits == 0 && ginfo->greenbits == 0)
                   && ginfo->packed < 3)
            {
                isI  = 8;
                bpp  = 8;
                isIA = 0;
                nopalettevar = 1;
            }

            break;

        case TYPEOLDXSH: /* OLDXSH */
            bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>16) bpp=32;

            // check for: AL_88 FORMAT
            // 8800 8080 8008 formats
            if((ginfo->alphabits > 4 && ginfo->packed < 3 ) && (
               (ginfo->redbits > 4 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits > 4 && ginfo-> bluebits == 0) ||
               (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo-> bluebits > 4) ))
            {
                bpp  = 16;
                isIA = 8;
                isI  = 0;
                nopalettevar = 1;
            }
            // A_8 format?
            // 8000
            else if((ginfo->alphabits > 4  && ginfo->redbits == 0 && ginfo->bluebits == 0 && ginfo->greenbits == 0)
                   && ginfo->packed < 3)
            {
                isI  = 8;
                bpp  = 8;
                isIA = 0;
                nopalettevar = 1;
            }
            break;

        case TYPEOLDVSH: /* OLDVSH */
            bpp=8;
            break;

        case TYPEOLDGSH: /* OLDGSH */
            bpp = 4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>(4+15)) bpp=32;
            isI = ioLdsshapeIntensityTexture((ARGB *)source,ginfo);

            // IA8 format?
            // 8800 8080 8008 formats are supported OLDGSH does a convert to grey
            // internally.
            if((
                 (ginfo->alphabits > 4 && ginfo->packed < 3 ) &&
                 (
                   (ginfo->redbits > 4 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
                   (ginfo->redbits == 0 && ginfo->greenbits > 4 && ginfo-> bluebits == 0) ||
                   (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo-> bluebits > 4)
                 )
               )    ||  // let through Luminance textures if and only if advanced is not turned on.
               (
                 (ginfo->alphabits > 4 && ginfo->packed < 3 && !(ginfo->subtype&SUBADVANCED)) &&
                 (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->bpp >4 && ginfo->bpp <=8)
               )
              )
            {
                //IA8 CLUT format?
                if(ginfo->bpp == 8)
                {
                    isIA = 9;
                    //bpp  = 8; can be 4 or 8
                    isI  = 0;
                }
                // Just plain IA8 format?
                else
                {
                    isIA = 8;
                    bpp  = 16;
                    isI  = 0;
                    nopalettevar = 1;
                }
            }
            // IA4 format?
            // bits between 2-4
//            else if((ginfo->alphabits > 2 && ginfo->alphabits <= 4 && ginfo->packed < 3 ) && (
//                   (ginfo->redbits   > 2 && ginfo->redbits   <= 4 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
//                    (ginfo->redbits  == 0 && ginfo->greenbits > 2  && ginfo->greenbits <= 4 && ginfo-> bluebits == 0) ||
//                    (ginfo->redbits  == 0 && ginfo->greenbits == 0 && ginfo-> bluebits  > 2 && ginfo-> bluebits <= 4) ))
            else if
              ((
                 (ginfo->alphabits > 2 && ginfo->alphabits <= 4 && ginfo->packed < 3 ) &&
                 (
                    (ginfo->redbits   > 2 && ginfo->redbits   <= 4 && ginfo->greenbits == 0 && ginfo-> bluebits == 0) ||
                    (ginfo->redbits  == 0 && ginfo->greenbits > 2  && ginfo->greenbits <= 4 && ginfo-> bluebits == 0) ||
                    (ginfo->redbits  == 0 && ginfo->greenbits == 0 && ginfo-> bluebits  > 2 && ginfo-> bluebits <= 4)
                 )
               )    ||  // let through other bpp's and Luminance textures if and only if advanced is not turned on.
               (
                 (ginfo->alphabits > 2 && ginfo->alphabits <= 4 && ginfo->packed < 3 && !(ginfo->subtype&SUBADVANCED)) &&
                 (ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->bpp > 2 && ginfo->bpp <=4)
               )
              )
            {
                isIA = 4;
                bpp  = 8;
                isI  = 0;
                nopalettevar = 1;
            }
            // I8 format?
            else if((ginfo->subtype&SUBADVANCED) && ginfo->alphabits == 8 && ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->packed < 3)
            {
                isI = 8;
                bpp = 8;
                nopalettevar = 1;
            }
            // I4 format?
            else if((ginfo->subtype&SUBADVANCED) && ginfo->alphabits == 8 && ginfo->redbits == 0 && ginfo->greenbits == 0 && ginfo->bluebits == 0 && ginfo->packed < 3)
            {
                isI = 4;
                bpp = 4;
                nopalettevar = 1;
            }
            else
            {
                isI = 0;
                isIA = 0;
                nopalettevar = 0;
            }
            break;

        case TYPEOLDMSH: /* OLDMSH */
            bpp=4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>15) bpp=16;
            if (ginfo->originalbpp>16) bpp=32;
            break;

        case TYPEOLDSSH: /* OLDSSH */
            bpp = 4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            if (ginfo->originalbpp>16) bpp=24;
            if (ginfo->originalbpp>24) bpp=32;
            if ((bpp == 15 || bpp == 16) && ginfo->alphabits > 2)
                bpp = 32;
//            if(bpp == 24 && !(subtype&SUBADVANCED))
//                bpp=32;
            break;

        case TYPEOLDASH: /* OLDASH */
            bpp = 4;
            if (ginfo->originalbpp>4)  bpp=8;
            if (ginfo->originalbpp>8)  bpp=15;
            break;

        case TYPEOLDNSH: /* OLDNSH */
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
    if(type != TYPEOLDVSH)
    {
        if ((ginfo->alphabits == 0) &&
            (ginfo->redbits   == 2) &&
            (ginfo->greenbits == 8) &&
            (ginfo->bluebits  == 2))
            bpp = 16;

    }


	bool dxtenable=false;
	if (type==TYPEOLDXSH || type==TYPEOLDFSH || type==TYPEOLDGSH || (subtype & SUBADVANCED))
		dxtenable = true;

	dxt = (dxtenable && (ginfo->packed==3 || ginfo->packed==5))?1:0;
    if (dxt && ginfo->bpp > 8)  // can't DXT compress a 8bpp image.
        bpp = 16;


    ok = 1;
    if (ginfo->subtype&SUBNOOLDSHP)
    {
        PictureShape = (OLDSHPHeaderStruct *) 0;
        SizeShape=0;
    }
    else if(OLDSHPIsGst(ginfo,type) && !OLDSHPismipmap(ginfo->framename,strlen(ginfo->framename)))  // note we do not GST compress mipmaps.
    {
        SizeShape = OLDSHPSetGstShape(&PictureShape,ginfo,reinterpret_cast<const unsigned char *>(source),type);
        SizeShape = OLDSHPALIGN(SizeShape);
    }
    else
    {
        if((type==TYPEOLDGSH  || type == TYPEOLDSSH  || type == TYPEOLDMSH) && ginfo->subtype&SUBSWIZZLE && !dxt)
        {
            int realwidth  = ginfo->width;
            int realheight = ginfo->height;
            if(type == TYPEOLDGSH)
                RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            if(type == TYPEOLDSSH)
                RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            if(type == TYPEOLDMSH)
                RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            SizeShape = OLDSHPWCalcSizeShape((int)realheight,(int)realwidth,bpp,type,dxt);
        }
        else if(type==TYPEOLDSSH && ginfo->subtype&SUBSWIZZLE & !dxt)
        {
            int realwidth  = ginfo->width;
            int realheight = ginfo->height;
            RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,ginfo->width,ginfo->height,bpp);
            SizeShape = OLDSHPWCalcSizeShape((int)realheight,(int)realwidth,bpp,type,dxt);
        }
        // YUY2 HAS to be a multiple of 2 width this is VERY important! Buffer at the very least has
        // to be marginally bigger.
        else if((type==TYPEOLDXSH || type==TYPEOLDFSH) && ginfo->alphabits == 0 && ginfo->redbits == 4 && ginfo->greenbits == 8 && ginfo->bluebits == 4)
        {
            int realwidth  = (ginfo->width+1)&~1;
            int realheight = ginfo->height; //(ginfo->height+1)&~1;
            SizeShape = OLDSHPWCalcSizeShape((int)realheight,(int)realwidth,bpp,type,dxt);
        }
        else
            SizeShape = OLDSHPWCalcSizeShape((int)(ginfo->width),(int)(ginfo->height),bpp,type,dxt);

        if (ginfo->packed==4)
            SizeShape = SizeShape*4 + 1024; // mpeg allowed space to grow
        else if (ginfo->packed>=2)
            SizeShape = (int)(SizeShape*1.2f + 1024); // otherwise, 20% is good enough

        SizeShape = OLDSHPALIGN(sizeof(OLDSHPHeaderStruct)+SizeShape);

        PictureShape=(OLDSHPHeaderStruct*)galloc(SizeShape);
        if (PictureShape)
        {
            memset(PictureShape,0,SizeShape);
            SizeShape = OLDSHPWSetShape(PictureShape,ginfo,bpp,source,rowbytes,ginfo->subtype&SUBNOOLDSHP,type,isI,isIA);
            SizeShape = OLDSHPALIGN(SizeShape);
        }
        else
            ok = 0;
    }


    OLDSHPWSetShapeClut(ginfo,ginfo->numcolours,&ginfo->colourtbl[0],&ClutShape,&SizeClut,(ginfo->subtype&SUBNOPAL)||nopalettevar,type);
    OLDSHPWSetShapeMetal(ginfo,&MetalShape,&SizeMetal,type);
    OLDSHPWSetShapeHotSpot(ginfo,&HotSpotShape,&SizeHotSpot,ginfo->subtype&SUBNOHOT, type);
    OLDSHPWSetShapeAnim(ginfo,&AnimShape,&SizeAnim,ginfo->subtype&SUBNOANIM, type);
    OLDSHPWSetShapeName(ginfo,&NameShape,&SizeName,ginfo->subtype&SUBNONAME);
    OLDSHPWSetShapeComment(ginfo,&CommentShape,&SizeComment,ginfo->subtype&SUBNOCOMMENT, type);
    if (PictureShape && type==TYPEOLDPSH)
        OLDSHPWSetShapeTpage(ginfo,&TpageShape,&SizeTpage,ginfo->subtype&SUBNOTPAGE, PictureShape->type);

    /* get shape id */

    if (!ginfo->framename[0])
    {
        shapeid[0] = ((framenum/1000)%10)+'0';
        shapeid[1] = ((framenum/100)%10)+'0';
        shapeid[2] = ((framenum/10)%10)+'0';
        shapeid[3] = ((framenum)%10)+'0';
    }
    else
    {
        if(!OLDSHPismipmap((char *)ginfo->framename,0))
        {
            for (i=0; (i<4) && ginfo->framename[i]; ++i)
                shapeid[i] = ginfo->framename[i];
            while (i<4)
                shapeid[i++] = ' ';
        }
        else
        // if the name is the new one.
        {
            int  len = strlen(ginfo->framename);
            int    id = 0;
            char * s = (char *)ginfo->framename;
            len -= 3;                // check up to 4 before the end last char.
            for(id = 0; id < len; id++)
            {
                if((s[0] == 'm') &&
                   (s[1] == 'm') &&
                   (s[2] >= '0') && (s[2] <= '9'))
                        break;
                s++;
            }
            for (i=0; (i<4) && *s; ++i,++id)
                shapeid[i] = *s++;
            while (i<4)
                shapeid[i++] = ' ';

        }
    }

    /* get clut id */

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


    gputm(theShape->ListShape[framenum].shape_ID,ggetm(shapeid,4),4);
    gputm(theShape->ListShape[framenum].shape_ClutID,ggetm(clutid,4),4);
    theShape->ListShape[framenum].shape_Data       = (char *) PictureShape;
    theShape->ListShape[framenum].shape_SizeData   = SizeShape;
    theShape->ListShape[framenum].shape_Clut       = (char *) ClutShape;
    theShape->ListShape[framenum].shape_SizeClut   = SizeClut;
    theShape->ListShape[framenum].shape_Metal      = (char *) MetalShape;
    theShape->ListShape[framenum].shape_SizeMetal  = SizeMetal;
    theShape->ListShape[framenum].shape_HotSpot    = (char *) HotSpotShape;
    theShape->ListShape[framenum].shape_SizeHotSpot= SizeHotSpot;
    theShape->ListShape[framenum].shape_Anim       = (char *) AnimShape;
    theShape->ListShape[framenum].shape_SizeAnim   = SizeAnim;
    theShape->ListShape[framenum].shape_Name       = (char *) NameShape;
    theShape->ListShape[framenum].shape_SizeName   = SizeName;
    theShape->ListShape[framenum].shape_Comment    = (char *) CommentShape;
    theShape->ListShape[framenum].shape_SizeComment= SizeComment;
    theShape->ListShape[framenum].shape_Tpage      = (char *) TpageShape;
    theShape->ListShape[framenum].shape_SizeTpage  = SizeTpage;
    theShape->ListShape[framenum].subtype          = ginfo->subtype;

    if (((ginfo->signature&0xffffff00)==GMAKEID('S','H','P',0))
      && ginfo->size==sizeof(GINFOOLDSHP)
      && (((GINFOOLDSHP *)ginfo)->align))
        theShape->ListShape[framenum].align = (((GINFOOLDSHP *)ginfo)->align)-1;
    else
        theShape->ListShape[framenum].align = 0;

    if(ginfo)
        gfree(ginfo);
    return(ok);
}

//-------------------------------------------------------------
// actually write the shape file out to disk. Loops over all shapes and writes the file out.
int OLDSHPWWriteShapeFile(GSTREAM *g,OLDShapeListPtr theShape,int type, int subtype, int multi,int oldshpsignature, int oldshpdataalignment, int oldshpclutdataalignment,int oldshpalignment, int oldshpmipmapdataalignment, int oldshppackeddataalignment)
{
    OLDSHPFILEHeaderStruct HeaderFile;   /* shape file header 16 bytes */
    OLDShapeTbl *TableShape=0L;    /* id/offset table */
    int  theSize;
    int  offset;
    int  paloffset;
    int  bigpad   = 0;
    int  bioldgshape = 0; // if we have a shape larger than 2^24 bytes then
    int  i;            // it's a big shape and needs to go at end of file.
    int  ok;
    char *shape_Data;
    char *shape_Clut;
    char *shape_Metal;
    char *shape_HotSpot;
    char *shape_Anim;
    char *shape_Name;
    char *shape_Comment;
    char *shape_Tpage;
    int  shape_SizeData;
    int  shape_SizeClut;
    int  shape_SizeMetal;
    int  shape_SizeHotSpot;
    int  shape_SizeAnim;
    int  shape_SizeName;
    int  shape_SizeComment;
    int  shape_SizeTpage;
    char oldvshheader[6];
    int EntriesPerShape=1;         /* put a palette entry with shape? */
    int PutPal=0;                  /* put a palette entry with shape? */
    int NuoLdmshapesAndPals;          /* number of shapes AND palettes in shape header */


    // Concatenate mipmap level data if subtype flag set.
    if (theShape->NuoLdmshape>1)
    {
        if (!(theShape->ListShape[0].subtype&SUBNOMIPMAPCOLLAPSE) && !(theShape->ListShape[1].subtype&SUBNOMIPMAPCOLLAPSE))
            OLDSHPWCollapseMipmaps(theShape,type, oldshpalignment, oldshpmipmapdataalignment);
    }

    OLDSHPWOrderCubemaps(theShape,type,oldshpalignment,oldshpmipmapdataalignment);

    OLDSHPWAlignPalette(theShape,type, oldshpclutdataalignment);

    bigpad = OLDSHPWSetLinks(theShape,type,oldshppackeddataalignment,oldshpdataalignment);  /* Calc link shape */

    /* write shape file header 16 (or 6) bytes */

    if (subtype==SHAPEFILE && multi)
    {
        theSize = (type==TYPEOLDVSH)?6:sizeof(OLDSHPFILEHeaderStruct);
        ok = (gwrite(g,(char*)&HeaderFile,theSize) == theSize);
    }
    else
        ok = 1;

    if (ok)
    {
        /* write id/offset table */

        if (subtype==SHAPEFILE && multi)
        {
            NuoLdmshapesAndPals = theShape->NuoLdmshape;

            if ((theShape->ListShape[0].subtype&SUBVOLDSHPAL) && (type==TYPEOLDFSH || type==TYPEOLDVSH)) /* OLDFSH and OLDVSH like entries for pals */
            {
                EntriesPerShape = 2;
                NuoLdmshapesAndPals *= 2;
                PutPal = 1;
            }

            theSize=sizeof(IteoLdmshapeTbl)*NuoLdmshapesAndPals;
            ok = 1;
            if (theSize)
            {
                TableShape = (OLDShapeTbl *) galloc(theSize);
                if (!TableShape)
                    ok=0;

                if (ok)
                {
                    ok = (gwrite(g,(char*)TableShape,theSize) == theSize);
                }
            }
        }
        if (ok)
        {
            for (i=0; i<theShape->NuoLdmshape; ++i)
            {

                shape_Data           = theShape->ListShape[i].shape_Data;
                shape_SizeData       = theShape->ListShape[i].shape_SizeData;
                shape_Clut           = theShape->ListShape[i].shape_Clut;
                shape_SizeClut       = theShape->ListShape[i].shape_SizeClut;
                shape_Metal          = theShape->ListShape[i].shape_Metal;
                shape_SizeMetal      = theShape->ListShape[i].shape_SizeMetal;
                shape_HotSpot        = theShape->ListShape[i].shape_HotSpot;
                shape_SizeHotSpot    = theShape->ListShape[i].shape_SizeHotSpot;
                shape_Anim           = theShape->ListShape[i].shape_Anim;
                shape_SizeAnim       = theShape->ListShape[i].shape_SizeAnim;
                shape_Name           = theShape->ListShape[i].shape_Name;
                shape_SizeName       = theShape->ListShape[i].shape_SizeName;
                shape_Comment        = theShape->ListShape[i].shape_Comment;
                shape_SizeComment    = theShape->ListShape[i].shape_SizeComment;
                shape_Tpage          = theShape->ListShape[i].shape_Tpage;
                shape_SizeTpage      = theShape->ListShape[i].shape_SizeTpage;

                bioldgshape = (shape_SizeData > BIGSHAPESIZE)?1:0;

                offset = (int)glen(g);

                if (theShape->ListShape[i].align)
                    oldshpdataalignment = theShape->ListShape[i].align;

                // pad shape data for alignment
                if (subtype==SHAPEFILE && multi)
                {
                    int padlen;
                    OLDSHPHeaderStruct *oldshpout = (OLDSHPHeaderStruct *) shape_Data;
                    if ((oldshpout) && (oldshpout->type&0x80))
                        padlen = ((offset+24+oldshppackeddataalignment)&~oldshppackeddataalignment)-24-offset;
                    else
                        padlen = ((offset+16+oldshpdataalignment)&~oldshpdataalignment)-16-offset;
                    if (padlen>0)
                    {
                        char *padstr=(char *)galloc(oldshpdataalignment+1);
                        memset(padstr,0,oldshpdataalignment+1); // "EASports";

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
                /* OLDVSH offset relative to end of table */
                if (type==TYPEOLDVSH)
                {
                    offset -= (6+NuoLdmshapesAndPals*8);
                    paloffset -= (6+NuoLdmshapesAndPals*8);
                }

                if (subtype==SHAPEFILE && multi)
                {
                    if (type==TYPEOLDVSH)
                    {
                        if (PutPal)
                        {
                            gputm((char *) TableShape->ShapeTable + i*4*EntriesPerShape,ggetm(theShape->ListShape[i].shape_ClutID,4),4); /* !pal */
                            gputi((char *) TableShape->ShapeTable + NuoLdmshapesAndPals*4+i*4*EntriesPerShape,paloffset,4);
                            gputm((char *) TableShape->ShapeTable + i*4*EntriesPerShape+4,ggetm(theShape->ListShape[i].shape_ID,4),4);  /* oldshp */
                            gputi((char *) TableShape->ShapeTable + NuoLdmshapesAndPals*4+i*4*EntriesPerShape+4,offset,4);
                        }
                        else
                        {
                            gputm((char *) TableShape->ShapeTable + i*4*EntriesPerShape,ggetm(theShape->ListShape[i].shape_ID,4),4);
                            gputi((char *) TableShape->ShapeTable + NuoLdmshapesAndPals*4+i*4*EntriesPerShape,offset,4);
                        }
                    }
                    else
                    {
                        if (PutPal)
                        {
                            gputm(TableShape->ShapeTable[i*EntriesPerShape].IDShape,ggetm(theShape->ListShape[i].shape_ClutID,4),4);  /* !pal */
                            OLDSHPPUTFIELD(TableShape->ShapeTable[i*EntriesPerShape].OffsetShape,paloffset);
                            gputm(TableShape->ShapeTable[i*EntriesPerShape+1].IDShape,ggetm(theShape->ListShape[i].shape_ID,4),4);  /* oldshp */
                            OLDSHPPUTFIELD(TableShape->ShapeTable[i*EntriesPerShape+1].OffsetShape,offset);
                        }
                        else
                        {
                            gputm(TableShape->ShapeTable[i].IDShape,ggetm(theShape->ListShape[i].shape_ID,4),4);
                            OLDSHPPUTFIELD(TableShape->ShapeTable[i].OffsetShape,offset);
                        }
                    }
                }

                if (subtype==STREAMINOLDGSHAPE && multi)
                {
                    char chunkbuf[8];
                    int chunksize=8+
                        shape_SizeData+
                        shape_SizeClut+
                        shape_SizeMetal+
                        shape_SizeHotSpot+
                        shape_SizeAnim+
                        shape_SizeName+
                        shape_SizeComment+
                        shape_SizeTpage;

                    gputm(chunkbuf,oldshpsignature,4);
                    gputi(chunkbuf+4,chunksize,4);
                    ok = gwrite(g,chunkbuf,8);

                }

                if(!bioldgshape)
                {
                        if (ok && shape_Data)
                        {
                            ok = (gwrite(g,(char*)shape_Data,shape_SizeData) == shape_SizeData);
                        }
                }
                if (ok && shape_Clut)
                {
                    ok = (gwrite(g,(char*)shape_Clut,shape_SizeClut) == shape_SizeClut);
                }
                if (ok && shape_Metal)
                {
                    ok = (gwrite(g,(char*)shape_Metal,shape_SizeMetal) == shape_SizeMetal);
                }
                if (ok && shape_HotSpot)
                {
                    ok = (gwrite(g,(char*)shape_HotSpot,shape_SizeHotSpot) == shape_SizeHotSpot);
                }
                if (ok && shape_Anim)
                {
                    ok = (gwrite(g,(char*)shape_Anim,shape_SizeAnim) == shape_SizeAnim);
                }
                if (ok && shape_Name)
                {
                    ok = (gwrite(g,(char*)shape_Name,shape_SizeName) == shape_SizeName);
                }
                if (ok && shape_Comment)
                {
                    ok = (gwrite(g,(char*)shape_Comment,shape_SizeComment) == shape_SizeComment);
                }
                if (ok && shape_Tpage)
                {
                    ok = (gwrite(g,(char*)shape_Tpage,shape_SizeTpage) == shape_SizeTpage);
                }
                if(bioldgshape)
                {
                    // write out a pad for alignment.
                    // hope that other items have not been padded from link
                    // set time till now.
                    if (bigpad>0)
                    {
                        char *padstr=(char *)galloc(oldshpdataalignment+1);
                        memset(padstr,0,oldshpdataalignment+1); // "EASports";
                        memcpy(padstr,"EASports",8);
                        ok = (gwrite(g,padstr,bigpad) == bigpad);
                        gfree(padstr);
                        offset += bigpad;
                    }
                    if (ok && shape_Data)
                    {
                        ok = (gwrite(g,(char*)shape_Data,shape_SizeData) == shape_SizeData);
                    }
                }

            }


            /* OLDVSH sort header */

            if (ok && type==TYPEOLDVSH && !(theShape->ListShape[0].subtype&SUBOLDVSHSORT))
            {
                for (i=0; i<NuoLdmshapesAndPals; ++i)
                {
                    int j;
                    int id1;
                    int id2;
                    int off1;
                    int off2;
                    char *head;
                    head = (char *) TableShape->ShapeTable;

                    for (j=0; j<NuoLdmshapesAndPals-1; ++j)
                    {
                        id1  = ggetm(head+j*4+0,4);
                        id2  = ggetm(head+j*4+4,4);
                        off1 = ggeti(head+(NuoLdmshapesAndPals+j)*4+0,4);
                        off2 = ggeti(head+(NuoLdmshapesAndPals+j)*4+4,4);
                        if (id1>id2)
                        {
                            gputm(head+j*4+0,id2,4);
                            gputm(head+j*4+4,id1,4);
                            gputi(head+(NuoLdmshapesAndPals+j)*4+0,off2,4);
                            gputi(head+(NuoLdmshapesAndPals+j)*4+4,off1,4);
                        }
                    }
                }
            }

            if (ok && subtype==SHAPEFILE && multi)
            {
                gseek(g,(GPOS)0); /* To be at the beginning of the file */

                if (type==TYPEOLDVSH)
                {
                    gputi(oldvshheader,((int)glen(g)),4);
                    gputi(oldvshheader+4,NuoLdmshapesAndPals,2);
                    ok = (gwrite(g,oldvshheader,6) == 6);
                }
                else
                {
                    gputm(HeaderFile.typeFile,oldshpsignature,4);
                    gputi(HeaderFile.LengthFile,(int)glen(g),4);
                    OLDSHPPUTFIELD(HeaderFile.NuoLdmshape,NuoLdmshapesAndPals);

                    if (theShape->ListShape[0].subtype&SUBADVANCED)
                    {
                        gputm(HeaderFile.Creator,GMAKEID(
                            'A',
                            (OLDSHP_VERSION/100)%10+'0',
                            (OLDSHP_VERSION/10)%10+'0',
                            (OLDSHP_VERSION)%10+'0'),4);
                    }
                    else
                    {
                        gputm(HeaderFile.Creator,GMAKEID(
                            'G',
                            (OLDSHP_VERSION/100)%10+'0',
                            (OLDSHP_VERSION/10)%10+'0',
                            (OLDSHP_VERSION)%10+'0'),4);
                    }

                    ok = (gwrite(g,(char*)&HeaderFile,sizeof(OLDSHPFILEHeaderStruct)) == sizeof(OLDSHPFILEHeaderStruct));
                }

                theSize=sizeof(IteoLdmshapeTbl)*NuoLdmshapesAndPals;
                ok = (gwrite(g,(char*)TableShape,theSize) == theSize);
            }
        }
    }
    if (TableShape) gfree((char*)TableShape);
    return(ok);
}

//-------------------------------------------------------------
// Delete a list of Shapes/cluts/anim/hotspots/name/comments
void OLDSHPWFreeShapes(OLDShapeListPtr theShape)
{
    int    i;

    for (i=0; i<theShape->NuoLdmshape; ++i)
    {
        if (theShape->ListShape[i].shape_Data)         gfree((char*)(theShape->ListShape[i].shape_Data));
        if (theShape->ListShape[i].shape_Clut)         gfree((char*)(theShape->ListShape[i].shape_Clut));
        if (theShape->ListShape[i].shape_Metal)        gfree((char*)(theShape->ListShape[i].shape_Metal));
        if (theShape->ListShape[i].shape_HotSpot)      gfree((char*)(theShape->ListShape[i].shape_HotSpot));
        if (theShape->ListShape[i].shape_Anim)         gfree((char*)(theShape->ListShape[i].shape_Anim));
        if (theShape->ListShape[i].shape_Name)         gfree((char*)(theShape->ListShape[i].shape_Name));
        if (theShape->ListShape[i].shape_Comment)      gfree((char*)(theShape->ListShape[i].shape_Comment));
        if (theShape->ListShape[i].shape_Tpage)        gfree((char*)(theShape->ListShape[i].shape_Tpage));

    }
    gfree((char*)theShape);
}



#endif

