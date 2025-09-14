// (c) 1994-2007 Electronic Arts Inc.


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _OLDSHPCMN_H_
#define _OLDSHPCMN_H_ 1


#include "gimex.h"
#include "oldshpgimex.h"

// all shapes have at least 16 byte header alignment
#define OLDSHPALIGNMENT 15
#define OLDSHPALIGN(x) (((x)+OLDSHPALIGNMENT)&~OLDSHPALIGNMENT)

#define TYPEOLDPSH 1
#define TYPEOLDFSH 2
#define TYPEOLDVSH 3
#define TYPEOLDNSH 4
#define TYPEOLDGSH 5
#define TYPEOLDSSH 6
#define TYPEOLDMSH 7
#define TYPEOLDXSH 8
#define TYPEOLDASH 9
#define TYPEOLDMAX 100
#define TYPEOLDFSS 102
#define TYPEOLDGSS 105
#define TYPEOLDSSS 106
#define TYPEOLDMSS 107
#define TYPEOLDXSS 108


/* Subtype mask */

#define SUBNOPAL              0x1
#define SUBNOOLDSHP           0x2
#define SUBNOHOT              0x4
#define SUBNOCOMMENT          0x8
#define SUBNOANIM            0x10
#define SUBNOTPAGE           0x20
//      reserved             0x40
//      reserved             0x80
#define SUBVOLDSHPAL           0x100
#define SUBNOMIPMAPCOLLAPSE 0x200
#define SUBOLDVSHSORT          0x400
#define SUBROTATE           0x800
#define SUBTRANSPOSE       0x1000
#define SUBSWIZZLE         0x2000
#define SUBCSM2            0x4000
#define SUBNONAME          0x8000
#define SUBDOT3           0x10000
#define SUBEMBM           0x20000
#define SUBCUBEMAP        0x40000
#define SUBMIPMONT        0x80000
#define SUBNODXTCOLLAPSE 0x100000
#define SUBMETALBIN      0x200000
#define SUBADVANCED      0x400000
//      reserved         0x800000
#define SUBOPAQUE       0x1000000
#define SUBTRANSPARENT  0x2000000
#define SUBHALFALPHA    0x4000000


/* Constant oldshp */

#define kUnknowShape  0
#define kPictureShape 1
#define kClutShape    2
#define kHotSpotShape 3
#define kCommentShape 4
#define kAnimShape    5
#define kTpageShape   6
#define kNameShape    7
#define kMetalShape   8



//--------------------------------------------------------------------------------------------------------
// convert OLDFSH 16 bit entry to 32 bit ARGB entry
int OLDFSHRpixel5To8(unsigned int aRgb5);


//--------------------------------------------------------------------------------------------------------
// convert OLDFSH 16 bit entry to 32 bit ARGB entry
int OLDFSHRpixel565To8(unsigned int aRgb5);

//--------------------------------------------------------------------------------------------------------
// convert OLDFSH 655 bit entry to 32 bit ARGB entry
int OLDFSHRpixel655To8(unsigned int aRgb5);

//--------------------------------------------------------------------------------------------------------
// convert PSX 16 bit entry to 32 bit ARGB entry
// with transparency and tranluciency
int OLDPSHRpixel5To8(unsigned int aRgb5);


//--------------------------------------------------------------------------------------------------------
// convert OLDSSH 1555 bit entry to 32 bit ARGB entry
int OLDSSHRpixel5To8(unsigned int aRgb5);


//--------------------------------------------------------------------------------------------------------
// convert GameCube 16 bit entry to 32 bit ARGB entry
int OLDGSHRpixel5To8(unsigned int aRgb5);


//--------------------------------------------------------------------------------------------------------
// convert OLDFSH 16 bit 4444 entry to 32 bit ARGB entry
int OLDFSHRpixel4444To8(unsigned int aRgb4444);

//--------------------------------------------------------------------------------------------------------
// convert OLDFSH 24 bit 6666 entry to 32 bit ARGB entry
int OLDFSHRpixel6666To8(unsigned int aRgb6666);

// convert OLDFSH 32 bit 2101010 entry to 32 bit ARGB entry
int OLDFSHRpixel2101010To8(unsigned int aRgb2101010);


//--------------------------------------------------------------------------------------------------------
// convert OLDNSH 16 bit RGBA entry to 32 bit ARGB entry
int OLDNSHRpixel5To8(unsigned int aRgb5);


//--------------------------------------------------------------------------------------------------------
// convert OLDNSH 16 bit IA entry to 32 bit ARGB entry
int OLDNSHRpixelIATo8(unsigned int ia88);


//--------------------------------------------------------------------------------------------------------
int OLDGSHRpixelIATo8(unsigned int ia88);

//--------------------------------------------------------------------------------------------------------
int OLDMSHRpixel4444To8(unsigned int aRgb4444);

//--------------------------------------------------------------------------------------------------------
int OLDMSHRpixel565To8(unsigned int aRgb5);

//--------------------------------------------------------------------------------------------------------
// swizzled PS2 cluts.
int OLDSHPRcsmsize(int colours);




//**************************************************************************************************************************************************************

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDFSH 15 bit entry
// with transparency
unsigned short OLDFSHWpixel8To5(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDFSH 16 bit entry
// with transparency
unsigned short OLDFSHWpixel8To565(const ARGB *aRgb8);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDFSH 655 bit entry
// with transparency
unsigned short OLDFSHWpixel8To655(const ARGB *aRgb8);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to PSX 16 bit entry
// with transparency and tranluciency
unsigned short OLDPSHWpixel8To5(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDNSH 16 bit IA (GREY8 macro borrowed from fgrey.c)
unsigned short OLDNSHWpixel8ToIA(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 8bit data to Intensity alpha information for OLDGSH.
unsigned short OLDGSHWpixel8ToIA(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 8bit data to 8bit Intensity alpha  information for OLDGSH.
unsigned char OLDGSHWpixel4ToIA(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 8bit ARGB data to 8bit Intensity information for OLDGSH.
unsigned char OLDGSHWpixel8ToI(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDNSH 16 bit RGBA
// with transparency
unsigned short OLDNSHWpixel8To5(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDSSH 15 bit entry
// with transparency
unsigned short OLDSSHWpixel8To5(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDFSH 16 bit ARGB 4444 entry
// with transparency
unsigned short OLDFSHWpixel8To4444(const ARGB *aRgb8);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDFSH 24 bit ARGB 6666 entry
// with transparency
unsigned int   OLDFSHWpixel8To6666(const ARGB *aRgb8);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDFSH 32 bit ARGB 2101010 entry
// with transparency
unsigned int   OLDFSHWpixel8To2101010(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// bit depth conversion function for OLDGSH.
unsigned short OLDGSHWpixel8To5(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDMSH 16 bit entry
unsigned short OLDMSHWpixel8To565(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to OLDMSH 16 bit ARGB 4444 entry
// with transparency
unsigned short OLDMSHWpixel8To4444(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// check if it's A8 or greyscale.
int OLDNSHWIsGreyPalette(const ARGB *colourtbl, int NbColours, int nopal);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// CLUT for PS2
int OLDSHPWcsmsize(int colours);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out the clut type based on channel depth.
int oldfshcluttype(int a, int r, int g, int b);
int oldfshadvancedcluttype(int a, int r, int g, int b);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int oldxshcluttype(int a, int r, int g, int b);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int oldgshcluttype(int a, int r, int g, int b);
int oldgshadvancedcluttype(int a, int r, int g, int b);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int oldsshcluttype(int a, int r, int g, int b);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int oldmshcluttype(int a, int r, int g, int b);


//**************************************************************************************************************************************************************


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled OLDGSH.
void RetOLDGSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled OLDSSH.
void RetOLDSSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled OLDMSH.
void RetOLDMSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// return true when the width and height are the propper dimentions for swizzleing as 32
bool OLDCanSwizzleSsh(int w,int h,int bpp);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// switching function for GameCube specific pixel offset lookups.
int OLDSHPWswizzleGC(int x,int y,int width,int bpp);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// look up the pixel offset for a 4bpp GameCube Swizzled shape.
int GC_OLDSHPCMN_SwizzleGetPixelOffset4( int x, int y, int width);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a 8bpp swizzled shape.
int GC_OLDSHPCMN_SwizzleGetPixelOffset8( int x, int y, int width);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// loop up the pixel offset for a 16bpp swizzled shape.
int GC_OLDSHPCMN_SwizzleGetPixelOffset16( int x, int y, int width);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// look up the pixel offset for 32bpp swizzled shape.
int GC_OLDSHPCMN_SwizzleGetPixelOffset32( int x, int y, int width);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a XBox AND PC Swizzled shape,
// the OLDXSH should really be OLDXSH and OLDFSH.
int OLDSHPWswizzleOLDXSH(int x,int y,int width,int height);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a PSP Swizzled shape,
int OLDSHPWswizzleOLDMSH(int x,int y,int width,int height,int bpp);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// PS2 Swizzleing code for 4bpp images, swizzle 4 and then unswizzle 32 so then when I upload as
// 32 and ask for a swizzle I get swizzle 4. Faster than uploading as 4bpp.
int OLDSHAPE_swizzle4(unsigned char *p_input, unsigned char *p_output,int width, int height);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// PS2 Swizzleing code for 16bpp images, swizzle 16 and then unswizzle 32 so then when I upload as
// 32 and ask for a swizzle I get swizzle 16. Faster than uploading as 16bpp.
int OLDSHAPE_swizzle16(unsigned char *src, unsigned char *dest, int width, int height);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for OLDSHAPE_swizzle4 PS2 swizzle function. swizzles the pages inside texture
// memory.
int PageConv4to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for OLDSHAPE_swizzle4() swizzles a block inside a page.
// swizzles first as 4 then unswizzles as 32.
int BlockConv4to32(unsigned char *p_input, unsigned char *p_output);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// PS2 swizzle function, swizzles as 8 then unswizzles as 32, for faster upload times. See
// comment on OLDSHAPE_swizzle4 for more information.
int OLDSHAPE_swizzle8(unsigned char *p_input, unsigned char *p_output,int width,int height);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for OLDSHAPE_swizzle8 PS2 swizzling function. Swizzles a single page within
// the texture space.
int PageConv8to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for OLDSHAPE_swizzle8() swizzles a single block withing a single page
// swizzles first as 8bpp then unswizzles as 16bpp.
int BlockConv8to32(unsigned char *p_input, unsigned char *p_output);

////SWIZZLE READ FUNCTIONS////

//--------------------------------------------------------------------------------------------------------
// switching function for GameCube swizzling.
int OLDSHPRswizzleGC(int x,int y,int width,int bpp);


//--------------------------------------------------------------------------------------------------------
// pixel lookup function for BOTH XBox and PC Shapes. The name is a bit misleading,
// it should have both OLDXSH and OLDFSH in the name.
int OLDSHPRswizzleOLDXSH(int x,int y,int width,int height);


//--------------------------------------------------------------------------------------------------------
// unswizzle function for 4bpp PS2 shapes. swizzles 32 and then unswizzles 4. We usually
// swizzle 4 then unswizzle 32 so when we upload as 32 we get swizzled 4, this is faster
// than uploading 4.
int OLDSHAPE_unswizzle4(unsigned char *p_input, unsigned char *p_output,int width, int height);


//--------------------------------------------------------------------------------------------------------
// unswizzle function for 16bpp PS2 shapes. swizzles 32 and then unswizzles 16. We usually
// swizzle 16 then unswizzle 32 so when we upload as 32 we get swizzled 16, this is faster
// than uploading 16.
int OLDSHAPE_unswizzle16(unsigned char *p_input, unsigned char *p_output,int width,int height);


//--------------------------------------------------------------------------------------------------------
// helper function for OLDSHAPE_unswizzle4() unswizzles a single page.
int PageConv4to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//--------------------------------------------------------------------------------------------------------
// helper function for OLDSHAPE_unswizzle4() Unswizzles a single block of data.
int BlockConv4to32(unsigned char *p_input, unsigned char *p_output);


//--------------------------------------------------------------------------------------------------------
// PS2 swizzle function, swizzles as 8 then unswizzle 32bpp. See 4bpp verions for more info.
int OLDSHAPE_unswizzle8(unsigned char *p_input, unsigned char *p_output,int width,int height);


//--------------------------------------------------------------------------------------------------------
// helper function for OLDSHAPE_unswizzle8(), handles the unswizzleing of a single page.
int PageConv8to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//--------------------------------------------------------------------------------------------------------
// helper function for OLDSHAPE_unswizzle8() swizzles 8 then unswizzles 32. A little confusing
// but it works. Look up table was generate from Sony's code.
int BlockConv8to32(unsigned char *p_input, unsigned char *p_output);


// MISC functions
//**************************************************************************************************************************************************************

//--------------------------------------------------------------------------------------------------------
// scan over the entire image and determine if the image is greyscale or not.
// return non zero if true.
bool ioLdsshapeIntensityTexture(ARGB *data, const GINFO * ginfo);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// returns true if the name is a valid mipmap name. Ie it is either mm## (old format) or
// basetexturename_mm## (new format) otherwise returns false
bool OLDSHPismipmap(char * shape_ID,int len);

#endif

#ifdef __cplusplus
}
#endif

