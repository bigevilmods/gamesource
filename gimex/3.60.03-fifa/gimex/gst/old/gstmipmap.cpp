// (c) 2004-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                       Gst Mipmap collapser
// ------------------------------------------------------------

#include "cremap.h"
#include "crapi.h"
#include "gimex.h"
#include "codex.h"
#include "refcodex.h"
#include "gst.h"
#include "oldshpgimex.h"
#include <stdio.h>
#include <string.h>

namespace Gst
{
    // ------------------------------------------------------------
    typedef struct
    {
        char	shape_ID[4];        /* The signature of this shape */
        char *	shape_Data;         /* The basic shape found on the file */
        int		shape_SizeData;     /* The size of this shape (write) */
        char	shape_ClutID[4];    /* The signature of this clut */
        char *	shape_Clut;         /* The Clut link to this shape */
        int		shape_SizeClut;     /* The size of this Clut (write) */
        char *	shape_HotSpot;      /* The HotSpot link to this shape */
        int		shape_SizeHotSpot;  /* The size of this HotSpot (write) */
        char *	shape_Comment;      /* The Comment link to this shape */
        int		shape_SizeComment;  /* The size of this Comment (write) */
        char *	shape_Name;         /* The Name link to this shape */
        int		shape_SizeName;     /* The size of this Name (write) */
        char *	shape_Anim;         /* The Anim link to this shape */
        int		shape_SizeAnim;     /* The size of this Anim (write) */
        char *	shape_Tpage;        /* The Tpage link to this shape */
        int		shape_SizeTpage;    /* The size of this Tpage (write) */
        char *	shape_Metal;        /* The Metal link to this shape */
        int		shape_SizeMetal;    /* The size of this Metal (write) */
        int		subtype;            /* Subtype for shape */
        int		packed;             /* Packed method (0=none,2=refpack,3=dxt,4=mpeg) (read) */
        int		framesize;          /* Total data size (read) */
        int		align;              /* Alignment to write shape data to (write) */
        GINFO * GST_StoredGinfo;
        int     shape_SizeGstMipmaps;
        char  * shape_GstMipmaps;
    } OLDgshapeInfo;

    // ------------------------------------------------------------
    typedef struct
    {
        int        NuoLdmshape;           /* The Number of Shape in this file */
        OLDgshapeInfo ListShape[1];       /* The list of shape for this composite file */
    } OLDShapeList, *OLDShapeListPtr;    
}

    // We assume that this is to be compiled into a shp library
    // in which case these two functions will exist and provide
    // the necessary behaviour:
    void OLDSHPWWriteBitmap(OLDSHPHeaderStruct *HeaderPict,char *ShapePtr,const GINFO *ginfo,const char *SourceMemPtr,int MemRowBytes,int type, int subtype,int rw,int rh);
    int OLDSHPRReadBitmap(OLDSHPHeaderStruct *HeaderPict,char *MemPtr,int MemRowBytes,int type);


namespace Gst
{




    // ------------------------------------------------------------
    #define OLDSHPTYPESSH              6
    #define OLDSHPALIGNMENT            15
    #define SUBSWIZZLE                 0x2000
    #define OLDSHPGETFIELD(field)      ggeti((void *)&field, sizeof(field))
    #define OLDSHPPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))
    #define OLDSHPMIPMAPALIGN(x)       (((x)+oldshpmipmapdataalignment)&~oldshpmipmapdataalignment)
    #define OLDSHPALIGN(x)             (((x)+OLDSHPALIGNMENT)&~OLDSHPALIGNMENT)


    bool IsGst(OLDgshapeInfo * sinfo);
	bool IsGstFourCC(unsigned int packed);
    
    // ------------------------------------------------------------
    bool IsMipmap(char * shape_ID,int len)
    {
            // if the name is the old one.
        if((shape_ID[0] == 'm') &&
           (shape_ID[1] == 'm') &&
           (shape_ID[2] >= '0') && (shape_ID[2] <= '9'))
            return true;
            // if the name is the new one.
        if(len == 0 || len > 4)
        {
            int    i = 0;
            char * s = shape_ID;

            if(len == 0)
                len = strlen(shape_ID) - 3;

            for(i = 0; i < len; i++)
            {
                if((s[0] == '_') &&
                   (s[1] == 'm') &&
                   (s[2] == 'm') &&
                   (s[3] >= '0') && (s[3] <= '9'))
                    return true;
                s++;
            }

        }
            // otherwise not a mipmap.
        return false;
    }

    // ------------------------------------------------------------
    int GetSSHBpp(int type)
    {
        int bpp = 32;
        switch(type)
        {
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
        }
        return bpp;
    }

    // ------------------------------------------------------------
    void SetMipmapLevels(OLDSHPHeaderStruct *theShape, int levels)
    {
        int shapey;

        if (levels==0)
            levels = 1;
        if (theShape)
        {
            shapey = OLDSHPGETFIELD(theShape->shapey) & 0x0fff;
            OLDSHPPUTFIELD(theShape->shapey, shapey + ((levels-1) << 12));
        }
    }

    // ------------------------------------------------------------
    int CalcSizeShape(int width,int height,int bpp)
    {
        return height*(((width*bpp+15)&~15)/8);
    }

    // ------------------------------------------------------------
    void SwizzleWidthHeight(int *rw,int *rh,int w,int h)
    {
        *rw = (w + 15)&~15;
        *rh = (h + 15)&~15;
    }
    
    // ------------------------------------------------------------
    int GetSize(int swizzled, int w,int h,int bpp)
    {    
        if(swizzled)  // is swizzled OLDSSH?
        {
            int realwidth  = 0;
            int realheight = 0;
            SwizzleWidthHeight(&realwidth,&realheight,w,h);
            return CalcSizeShape(realwidth, realheight, bpp);
        }
        else
            return CalcSizeShape(w, h, bpp);
    }

    // ------------------------------------------------------------
    char * GetMipmapBuffer(int w,int h,int bpp,int swizzled,int * retnewsize,OLDSHPHeaderStruct * oldshp,int oldshpmipmapdataalignment)
    {
        int thissize   = 0;
        int newsize    = 0;
        char * newdata = 0;
            // don't allocate for first level since this is the GST texture
            //thissize = newsize = GetSize(swizzled,w,h,bpp);

        // loop over all the expected mipmaps and build a buffer big enough
        // to hold them all
        while ((w > 1) || (h > 1))
        {

            if (w > 1)  w >>= 1;
            if (h > 1)  h >>= 1;
            int tempsize = GetSize(swizzled,w,h,bpp);
            newsize     += OLDSHPMIPMAPALIGN(tempsize);
        }
        
        newsize += sizeof(OLDSHPHeaderStruct);
		newsize =  OLDSHPALIGN(newsize);
        newdata = (char *)galloc(newsize); 
        memset(newdata,0,newsize);
        memcpy(newdata, oldshp, sizeof(OLDSHPHeaderStruct));
        // set my counter to only containing a header
        *retnewsize = sizeof(OLDSHPHeaderStruct);
        return newdata;
    }
    
    // ------------------------------------------------------------
    void CopyMipmap(char * newdata,int newsize,int thissize,OLDSHPHeaderStruct * oldshp)
    {
        char * sourceData = (char *)oldshp + sizeof(OLDSHPHeaderStruct);
        int ispacked      = oldshp->type&0x80;
        int isrefpacked   = ispacked ? REF_is(sourceData) : 0;

        // If this is a refpacked texture I need to decode it here and then I will
        // recode the entire buffer later when I have all my mipmaps nicely collapsed
        if (ispacked && isrefpacked)
            REF_decode(newdata + newsize, sourceData, 0);
        else
            memcpy(newdata + newsize, sourceData, thissize);
    }

    // ------------------------------------------------------------
    void CleanupMipmap(OLDgshapeInfo * sinfo)
    {
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

        sinfo->shape_SizeData    = 0;
        sinfo->shape_SizeClut    = 0;
        sinfo->shape_SizeMetal   = 0;
        sinfo->shape_SizeHotSpot = 0;
        sinfo->shape_SizeAnim    = 0;
        sinfo->shape_SizeName    = 0;
        sinfo->shape_SizeComment = 0;
        sinfo->shape_SizeTpage   = 0;
    }
    
    // ------------------------------------------------------------
    void RemapMipmap(GINFO * palette,int width,int height,OLDgshapeInfo * sinfo)
    {
        int size                 = width*height;
        char * dest              = (char *)galloc(size);
        OLDSHPHeaderStruct * shp = (OLDSHPHeaderStruct *)(sinfo->shape_Data);
        char * realSource        = ((char *)shp) + sizeof(OLDSHPHeaderStruct);
        int realsourceBpp        = GetSSHBpp(shp->type);
        int sourceBpp            = realsourceBpp >= 8?32:8;
        char * source            = (char *)galloc(height*width*sourceBpp/8);
		palette->width           = width;
		palette->height          = height;
		
		// read the shape
        OLDSHPRReadBitmap((::OLDSHPHeaderStruct *)shp,source,width*sourceBpp/8,OLDSHPTYPESSH);
        
		// remap the read shape
		remap(dest, palette,width,source,width*sourceBpp/8,sourceBpp,NULL,0);
		
		// write the shape as a 8bpp image
		shp->type = 0x02;
        OLDSHPWWriteBitmap((::OLDSHPHeaderStruct *)shp,realSource,palette,dest,width,OLDSHPTYPESSH,palette->subtype,width,height);
        
		// cleanup
		gfree(source);
        gfree(dest);
    }
    
    // ------------------------------------------------------------
    void CollapseGstMipmaps(char * theShapes, int oldshpalignment, int oldshpmipmapdataalignment)
    {
        OLDgshapeInfo      *sinfo , *sinfoout;
        OLDSHPHeaderStruct *oldshp, *oldshpout;
        OLDShapeListPtr theShape = (OLDShapeListPtr)theShapes;
        int levels     = 0;
        char * newdata = 0;
        int newsize    = 0;
        int collapse   = 0;
        int notmipmap  = 0;

        // Start at 1, since 0th frame has nothing to be appended to.
        int out        = 0;
        int in         = 1;
        
        while (in < theShape->NuoLdmshape)
        {
            sinfoout     = &theShape->ListShape[out];
            sinfo        = &theShape->ListShape[in];
            oldshp       = (OLDSHPHeaderStruct *)sinfo->shape_Data;
            oldshpout    = (OLDSHPHeaderStruct *)sinfoout->shape_Data;
            int swizzled = OLDSHPGETFIELD(oldshpout->shapex)&0x2000;
            
            // Identify mipmaps that belong to Gst compressed textures
            notmipmap = 0;
            if (IsMipmap(sinfo->shape_ID,4) && IsGst(sinfoout))
            {
                int thissize = 0;
                int width    = OLDSHPGETFIELD(oldshp->width);
                int height   = OLDSHPGETFIELD(oldshp->height);
                int bpp      = 8;

				// remap the current image to use the GST master palette.
				RemapMipmap(sinfoout->GST_StoredGinfo,width,height,sinfo);

                // On first level we allocate space for combined data;
                // assuming that there may be mipmaps all the way down
                // to 1x1.
                if (!levels)
                {
                    int w         = OLDSHPGETFIELD(oldshpout->width);
                    int h         = OLDSHPGETFIELD(oldshpout->height);
                    newdata       = GetMipmapBuffer(w,h,bpp,swizzled,&newsize,oldshp,oldshpmipmapdataalignment);
                    levels        = 1;
                }
                
                // append each subsequent mipmap into one continuous buffer.
                newsize = OLDSHPMIPMAPALIGN(newsize-16)+16;

                // get the size of the current mipmap and append
                thissize = GetSize(swizzled,width,height,bpp);
                CopyMipmap(newdata,newsize,thissize,oldshp);
                

                // Free this shape and mark as such. (Note that cluts and
                // hot spots get lost for all but top level of mipmap).
                CleanupMipmap(sinfo);

                // Add size for next level of mipmap
                newsize += thissize;
                ++levels;
            } // END if isMipmap
            else
                notmipmap = 1; // set this because we can't check oldshp if it's memory gets deleted later.

            
            // If not a mipmap level or we're at the last shape
            // then store out the results
            if ((in == theShape->NuoLdmshape - 1) || notmipmap)
            {
                // If we just finished combining mipmaps then save
                // the combined data in the output shape
                if (newdata)
                {
                    newsize = OLDSHPALIGN(newsize);

                    // Check whether to pack the new data
                    if ((oldshpout->type&0x80) && REF_is((char *)oldshpout+sizeof(OLDSHPHeaderStruct)))
                    {
                        char *newdatapack = (char *)galloc(newsize*3/2+1024);

                        // Pack data into newly allocated space. This is wasteful of memory, oh well
                        memcpy(newdatapack, newdata, sizeof(OLDSHPHeaderStruct));
                        newsize = (REF_encode(newdatapack + sizeof(OLDSHPHeaderStruct),
                                              newdata + sizeof(OLDSHPHeaderStruct),
                                              newsize - sizeof(OLDSHPHeaderStruct),
                                              0));
                        newsize += sizeof(OLDSHPHeaderStruct);
                        gfree(newdata);
                        newdata = newdatapack;
                    }

                    sinfoout->shape_GstMipmaps     = (char *) newdata;
                    sinfoout->shape_SizeGstMipmaps = newsize;
                    SetMipmapLevels(oldshpout, levels);
					SetMipmapLevels((OLDSHPHeaderStruct *)newdata,levels);
                }

                if(sinfoout->GST_StoredGinfo)
                {
                    gfree(sinfoout->GST_StoredGinfo);
                    sinfoout->GST_StoredGinfo = NULL;
                }
                // Next output shape will be this shape
                out     = in;
                levels  = 0;
                newdata = 0;
                newsize = 0;
            }
            ++in;
        } // END while NumShape

        // Second pass through shape list removes all the unused entries
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
        // Reset total number of shapes to the number remaining after mipmap collapsing
        theShape->NuoLdmshape = out;
    }

    // ------------------------------------------------------------
    int GetMipmapsLevels(OLDgshapeInfo * sinfo,OLDSHPHeaderStruct * theShape)
    {
        if(!theShape)
            return 0;
        
        if(IsGstFourCC(sinfo->packed))
        {   
            int shapey = OLDSHPGETFIELD(theShape->shapey);
            int levels = shapey >> 12;

            if ( (levels == 0) | (levels == 15) )
                levels = 1;
            else
                levels += 1;
            return levels;
        }
        return 1;
    }
    
    // ------------------------------------------------------------
    int GetTotalFrameCount(OLDShapeListPtr OldshpList)
    {
        int count = 0;
        for (int in = 0; in < OldshpList->NuoLdmshape; ++in)
        {
            OLDgshapeInfo      * sinfo    = &OldshpList->ListShape[in];
            OLDSHPHeaderStruct * theShape = (OLDSHPHeaderStruct *)sinfo->shape_Data;
            count += GetMipmapsLevels(sinfo,theShape);
        }
        return count;
    }

    // ------------------------------------------------------------
    // copy the shape name pointer for later reinsertion into the shapename.
    void StoreBaseName(char ** rname,OLDgshapeInfo * sinfo)
    {
        char * name      = 0;
        char * shapeName = (char *)((OLDnameheaderStruct*)(sinfo->shape_Name))->ListName;
        
        if(*rname)
            gfree(*rname); *rname = 0;

        if(sinfo->shape_Name)
        {
            name = (char *)galloc(strlen(shapeName)+1);
            strcpy(name,shapeName);
        }
        else
        {
            name = (char *)galloc(5);
            memcpy(name,sinfo->shape_ID,4);
            name[4] = '\0';
        }
        *rname = name;
    }

    // ------------------------------------------------------------
    void CreateIDAndNameFields(char * name,OLDgshapeInfo * sinfoout,int type,int l)
    {
        // set the ID to mm<level>
        sinfoout->shape_ID[0] = 'm';
        sinfoout->shape_ID[1] = 'm';
        sinfoout->shape_ID[2] = (char)('0'+(l/10));
        sinfoout->shape_ID[3] = (char)('0'+(l%10));

        // build a new name element for this new shape
        int nameSize             = (sizeof(OLDnameheaderStruct) + strlen(name)+10);
        sinfoout->shape_SizeName = nameSize;
        sinfoout->shape_Name     = (char *)galloc(nameSize);
        memset(sinfoout->shape_Name,0,nameSize);

        // set the name field to BaseName_mm<level>
        ((OLDnameheaderStruct*)(sinfoout->shape_Name))->type = type;
        sprintf((char *)((OLDnameheaderStruct*)(sinfoout->shape_Name))->ListName,"%s_mm%02d",name,l);
    }

    // ------------------------------------------------------------
    void CopyClut(OLDgshapeInfo * sinfo,OLDgshapeInfo * sinfoout)
    {
        // Copy clut. (We could use the same clut, but then it
        //  would be freed twice when the shape is deleted.)
        if (sinfo->shape_Clut && sinfo->shape_SizeClut)
        {
            sinfoout->shape_Clut = (char *)galloc(sinfo->shape_SizeClut);
            memcpy(sinfoout->shape_Clut, sinfo->shape_Clut, sinfo->shape_SizeClut);
        }
    }

    // ------------------------------------------------------------
    void ClearNonMipmapAttachementFields(OLDgshapeInfo * sinfoout)
    {
        // Toss other attachments.
        sinfoout->shape_HotSpot     = 0;
        sinfoout->shape_SizeHotSpot = 0;
        sinfoout->shape_Comment     = 0;
        sinfoout->shape_SizeComment = 0;
        sinfoout->shape_Tpage       = 0;
        sinfoout->shape_SizeTpage   = 0;
        sinfoout->shape_Metal       = 0;
        sinfoout->shape_SizeMetal   = 0;
        sinfoout->shape_Anim        = 0;
        sinfoout->shape_SizeAnim    = 0;
		sinfoout->shape_GstMipmaps  = 0;
    }

    // ------------------------------------------------------------
    OLDgshapeInfo * CopyHeader(OLDShapeListPtr OldshpListOut, OLDgshapeInfo * sinfo,int out)
    {
        OLDgshapeInfo * sinfoout  = &OldshpListOut->ListShape[out];
            // Copy the header.
        memcpy(sinfoout, sinfo, sizeof(OLDgshapeInfo));
        return sinfoout;
    }

    // ------------------------------------------------------------
    int AllocateNewBufferAndCopyData(OLDSHPHeaderStruct * oldshpin,OLDSHPHeaderStruct ** oldshpout,OLDgshapeInfo * sinfoout,int width, int height,char * datain)
    {
        // Allocate new shape data.
        int size                 = GetSize(OLDSHPGETFIELD(oldshpin->shapex)&0x2000,width,height,8);
        *oldshpout               = (OLDSHPHeaderStruct *)galloc(sizeof(OLDSHPHeaderStruct) + size);
        sinfoout->shape_Data     = (char *)(*oldshpout);
        sinfoout->shape_SizeData = sizeof(OLDSHPHeaderStruct) + size;

        // Set shape header fields for this level.
        memcpy(*oldshpout, oldshpin, sizeof(OLDSHPHeaderStruct));
        OLDSHPPUTFIELD((*oldshpout)->width,  width);
        OLDSHPPUTFIELD((*oldshpout)->height, height);
        memcpy((char *)(*oldshpout) + sizeof(OLDSHPHeaderStruct), datain, size);

        return size;
    }
	
	void AdjustWidthHeight(int * width,int * height)
	{
        if ((*width) > 1)
            (*width) >>= 1;
        if ((*height) > 1)
            (*height) >>= 1;
	}
    
    
    // ------------------------------------------------------------
    //   Search through list of shapes for collapsed mipmaps
    //   and expand into multiple shapes if present.
    char * ExpandGstMipmaps(char * inList, int type, int oldshpmipmapdataalignment)
    {
        OLDShapeListPtr    OldshpList       = (OLDShapeListPtr)inList;
        OLDShapeListPtr    OldshpListOut    = 0;
        int                in               = 0;
        int                out              = 0;
        OLDgshapeInfo      *sinfo           = 0;
        OLDgshapeInfo      *sinfoout        = 0;
        OLDSHPHeaderStruct *oldshpin        = 0;
        OLDSHPHeaderStruct *oldshpout       = 0;
        char               *datain          = 0;
        char               *name            = 0;
        int                levels           = 0;
        int                l                = 0;
        int                width            = 0;
        int                height           = 0;
        int                bpp              = 0;
        int                size             = 0;
        int                totalFrameCount  = 0;
		int                alignedsize      = 0;
        
        totalFrameCount = GetTotalFrameCount(OldshpList);
        if(totalFrameCount == OldshpList->NuoLdmshape)
            return (char *)OldshpList;

        
        // allocate and fill the new shape list based on totalFrameCount
        OldshpListOut = (OLDShapeListPtr)galloc(sizeof(OLDgshapeInfo)*totalFrameCount+sizeof(int));
        if (!OldshpListOut)
            return (char *)OldshpList;

        OldshpListOut->NuoLdmshape = totalFrameCount;

        // iterate and expand all GST mipmaps
        for (in = 0; in < OldshpList->NuoLdmshape; ++in)
        {
            sinfo     = &OldshpList->ListShape[in];
            oldshpin  = (OLDSHPHeaderStruct *)sinfo->shape_Data;
            levels    = GetMipmapsLevels(sinfo,oldshpin);

            // early out if this is not a GST item or if it doesn't have any
            // mipmaps
            if(!IsGstFourCC(sinfo->packed) || levels <= 1)
            {
                CopyHeader(OldshpListOut,sinfo,out);
                out++;
                continue;
            }

            datain    = sinfo->shape_GstMipmaps;
            width     = OLDSHPGETFIELD(((OLDSHPHeaderStruct *)datain)->width);
            height    = OLDSHPGETFIELD(((OLDSHPHeaderStruct *)datain)->height);
            bpp       = GetSSHBpp(oldshpin->type);

			// have to nuke the record of mipmap levels so the normal mipmap
			// extractor will not try to expand this one.
			if(levels > 1)
				SetMipmapLevels(oldshpin,1);

            // Store the base texture name so I can apply it to all the mipmaps
            StoreBaseName(&name,sinfo);

            // point datain to the GstMipmaps element data and begin the unpacking
            if(levels > 1 && datain)
				alignedsize = OLDSHPMIPMAPALIGN(sizeof(OLDSHPHeaderStruct)-16)+16; 
            
            // Extract all levels.
            for (l = 0; l < levels; ++l)
            {
                sinfoout = CopyHeader(OldshpListOut,sinfo,out);

                // first element we only have to copy the header
                if(l == 0)
                {
                    out++;
                //    AdjustWidthHeight(&width,&height);
                    continue;
                }
                
                // If not top level then signature is "mm<level>",
                // clut is copied from top level, hot spots get lost.
                if (l > 0)
                {
                    CreateIDAndNameFields(name,sinfoout,type,l);
                    CopyClut(sinfo,sinfoout);
                    ClearNonMipmapAttachementFields(sinfoout);
                }

                // create a new shape with the shpin header and copy over the current mipmap
                size = AllocateNewBufferAndCopyData(oldshpin,&oldshpout,sinfoout,width,height,datain+alignedsize);


                // Update for next level.
                //datain += size;
				alignedsize += size;
				alignedsize = OLDSHPMIPMAPALIGN(alignedsize-16)+16;
				AdjustWidthHeight(&width,&height);
                ++out;
            }
        }
        if(name)
        {
            gfree(name); name = 0;
        }

        // Free old list and return new one.
        gfree(OldshpList);
        return (char *)OldshpListOut;
    }

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
    bool IsGstFourCC(unsigned int packed)
    {
        if((packed&0xFFFFFF00) == 0x47535400) // if the fourCC == 'GST '
            return true;
        return false;
    }

    
    // ------------------------------------------------------------
    bool IsGst(const GINFO * ginfo,int type)
    {
        if(type == OLDSHPTYPESSH)
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
    bool IsGst(OLDgshapeInfo * sinfo)
    {
        OLDSHPHeaderStruct * temp = (OLDSHPHeaderStruct *)OLDSHPGETFIELD(sinfo->shape_Data);
        if(temp->type >= 0x08 && temp->type <= 0x0f)
            return true;
        return false;
    }
    
    // ------------------------------------------------------------
    unsigned int OLDSHPWSetGstShape(OLDSHPHeaderStruct ** image,GINFO * ginfo,const unsigned char * source, int type)
    {
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
        int blockWidth       = 1 << (((unsigned char)(ginfo->packed))&3);
        int blockHeight      = 2;
        int detailBpp        = 1 << ((((unsigned char)(ginfo->packed))&4)>>2);
        int sizeofTexture    = 0;
        int sizeofClut       = 0;
        unsigned char * clut = 0;
        int fastness         = ginfo->quality > 75 ? 0 : ginfo->quality >= 50 ? 1 : ginfo->quality >= 25 ? 2 : 3;
        int swizzleTexture   = ginfo->subtype&SUBSWIZZLE?1:0;

    
    
            // first run compressor, I'm returned a shape but I have to put the clut back into the ginfo
            // for the rest of the shape code to work correctly
        *image = (OLDSHPHeaderStruct *)GSTCompressImage(source        ,  ginfo->width*ginfo->bpp/8,  ginfo->width,  ginfo->height,
                                                        &sizeofTexture, &sizeofClut               ,  &clut       ,  blockWidth   ,
                                                        blockHeight   , detailBpp                 ,  8           ,  32           ,
                                                        swizzleTexture, 0                         ,  fastness    ,  ginfo->bpp   ,
                                                        (char *)ginfo->colourtbl);

            // have to copy the clut over so I can then later recreate this clut using the regular code and
            // not franks GST code. Not order is swapped (abgr) so unscramble when I put into ginfo.
        int numcolours    = OLDSHPGETFIELD(((OLDSHPHeaderStruct *)clut)->width);
        ARGB * temp       = (ARGB *)(clut+sizeof(OLDSHPHeaderStruct));
        for(int i = 0; i < numcolours; i++)
        {
            ginfo->colourtbl[i].a = temp[i].a;
            ginfo->colourtbl[i].r = temp[i].b;
            ginfo->colourtbl[i].g = temp[i].g;
            ginfo->colourtbl[i].b = temp[i].r;
        }
        ginfo->numcolours = numcolours;
        ginfo->bpp        = 8;

        gfree(clut);
        return sizeofTexture;
                              
    }
    
}
