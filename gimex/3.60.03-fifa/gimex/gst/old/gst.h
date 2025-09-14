// (c) 2004-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                       GST Main Header File
//                      ----------------------
// Adapted from frank's GST command line utility and patrick
// ratto's p3 the GST code is mean to work directly with the
// shape file format.
// ------------------------------------------------------------

#ifndef GST_H
#define GST_H

namespace Gst
{
#define DOALPHA    1
#define DUMPBLOCKS 1
#define SHAPE2     0
#define SWIZZLE    1
static const int DIFFPRECISSION = 0;

// included here merely to make compiling the gst.lib easier 
typedef struct OLDSHPHeaderStruct
{
     char type;               /*  The type for this shape */
     char LinkShape[3];       /*  The Link to the next shape */
     char width[2];           /*  width of the Shape */
     char height[2];          /*  height of the Shape */
     char centerx[2];         /*  center point of the Shape */
     char centery[2];         /*  center point of the Shape */
     char shapex[2];          /*  position of the Shape */
     char shapey[2];          /*  position of the Shape */

} OLDSHPHeaderStruct;

unsigned char *  GSTCompressImage(const unsigned char *image , int rowbytes      , int width            , int height          , 
	                              int *sizeofTexture         , int *sizeofPalette, unsigned char ** clut, int BlockWidth=4    ,
					              int BlockHeight=2          , int DetailBpp=2   , int BaseBpp=8        , int ClutBpp=32      , 
					              int SwizzleTexture=1       , int SwizzleClut=1 , int Fastness=1       , int bpp=8           ,
								  char * colourtbl=0         ); 
 
unsigned char *  GSTDecompressImage(unsigned char * inshape);

void             CollapseGstMipmaps(char * theShapes, int oldshpalignment, int oldshpmipmapdataalignment);

char *           ExpandGstMipmaps(char * inList, int type, int oldshpmipmapdataalignment);
    
unsigned int     OLDSHPWSetGstShape(OLDSHPHeaderStruct ** image,GINFO * ginfo,const unsigned char * source, int type);
 
bool             IsGst(const GINFO * ginfo,int type);

unsigned int     GetGstFourCC(unsigned char stype);

bool             IsGstFourCC(unsigned int packed);
 
}

#endif
