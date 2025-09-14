// (c) 1994-2007 Electronic Arts Inc.


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SHPCMN_H_
#define _SHPCMN_H_ 1

#include "gimex.h"
#include "shpgimex.h"

// all shapes have at least 16 byte header alignment
#define SHPALIGNMENT 15
#define SHPALIGN(x) (((x)+SHPALIGNMENT)&~SHPALIGNMENT)

#define TYPEPSH 1
#define TYPEFSH 2
#define TYPEVSH 3
#define TYPENSH 4
#define TYPEGSH 5
#define TYPESSH 6
#define TYPEMSH 7
#define TYPEXSH 8
#define TYPEASH 9
#define TYPEZSH 10 /*XENON*/
#define TYPEYSH 11 /*PS3*/
#define TYPEMAX 100
#define TYPEFSS (TYPEMAX+TYPEFSH)
#define TYPEGSS (TYPEMAX+TYPEGSH)
#define TYPESSS (TYPEMAX+TYPESSH)
#define TYPEMSS (TYPEMAX+TYPENSH)
#define TYPEXSS (TYPEMAX+TYPEXSH)
#define TYPEZSS (TYPEMAX+TYPEZSH)
#define TYPEYSS (TYPEMAX+TYPEYSH)

/* Subtype mask */

#define SUBNOPAL              0x1
#define SUBNOSHP              0x2
#define SUBNOHOT              0x4
#define SUBNOCOMMENT          0x8
//#define SUBNOANIM            0x10
//#define SUBNOTPAGE           0x20
//      reserved             0x40
//      reserved             0x80
#define SUBVSHPAL           0x100
#define SUBNOMIPMAPCOLLAPSE 0x200
#define SUBVSHSORT          0x400
#define SUBROTATE           0x800
#define SUBTRANSPOSE       0x1000
#define SUBSWIZZLE         0x2000
#define SUBCSM2            0x4000
//#define SUBNONAME          0x8000
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
#define SUBCONFIGURE    0x8000000


/* Constant shp */

#define kUnknowShape  0
#define kPictureShape 1
#define kClutShape    2
#define kHotSpotShape 3
#define kCommentShape 4
//#define kAnimShape    5
//#define kTpageShape   6
//#define kNameShape    7
#define kMetalShape   8
#define kDataBinShape 9


//------------------------------------------------------------------------
// convert FSH 16 bit entry to 32 bit ARGB entry
int FSHRpixel5To8(unsigned int aRgb5);


//------------------------------------------------------------------------
// convert FSH 16 bit entry to 32 bit ARGB entry
int FSHRpixel565To8(unsigned int aRgb5);

//------------------------------------------------------------------------
// convert FSH 655 bit entry to 32 bit ARGB entry
int FSHRpixel655To8(unsigned int aRgb5);

//------------------------------------------------------------------------
// convert PSX 16 bit entry to 32 bit ARGB entry
// with transparency and tranluciency
int PSHRpixel5To8(unsigned int aRgb5);


//------------------------------------------------------------------------
// convert SSH 1555 bit entry to 32 bit ARGB entry
int SSHRpixel5To8(unsigned int aRgb5);


//------------------------------------------------------------------------
// convert GameCube 16 bit entry to 32 bit ARGB entry
int GSHRpixel5To8(unsigned int aRgb5);


//------------------------------------------------------------------------
// convert FSH 16 bit 4444 entry to 32 bit ARGB entry
int FSHRpixel4444To8(unsigned int aRgb4444);

//------------------------------------------------------------------------
// convert FSH 24 bit 6666 entry to 32 bit ARGB entry
int FSHRpixel6666To8(unsigned int aRgb6666);

// convert FSH 32 bit 2101010 entry to 32 bit ARGB entry
int FSHRpixel2101010To8(unsigned int aRgb2101010);


//------------------------------------------------------------------------
// convert NSH 16 bit RGBA entry to 32 bit ARGB entry
int NSHRpixel5To8(unsigned int aRgb5);


//------------------------------------------------------------------------
// convert NSH 16 bit IA entry to 32 bit ARGB entry
int NSHRpixelIATo8(unsigned int ia88);


//------------------------------------------------------------------------
int GSHRpixelIATo8(unsigned int ia88);

//--------------------------------------------------------------------------------------------------------
int MSHRpixel4444To8(unsigned int aRgb4444);

//--------------------------------------------------------------------------------------------------------
int MSHRpixel565To8(unsigned int aRgb5);

//------------------------------------------------------------------------
// swizzled PS2 cluts.
int SHPRcsmsize(int colours);




//*************************************************************************

//------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 15 bit entry
// with transparency
unsigned short FSHWpixel8To5(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 16 bit entry
// with transparency
unsigned short FSHWpixel8To565(const ARGB *aRgb8);

//------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 655 bit entry
// with transparency
unsigned short FSHWpixel8To655(const ARGB *aRgb8);

//------------------------------------------------------------------------
// convert 32 bit ARGB entry to PSX 16 bit entry
// with transparency and tranluciency
unsigned short PSHWpixel8To5(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 32 bit ARGB entry to NSH 16 bit IA (GREY8 macro borrowed from fgrey.c)
unsigned short NSHWpixel8ToIA(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 8bit data to Intensity alpha information for GSH.
unsigned short GSHWpixel8ToIA(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 8bit data to 8bit Intensity alpha  information for GSH.
unsigned char GSHWpixel4ToIA(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 8bit ARGB data to 8bit Intensity information for GSH.
unsigned char GSHWpixel8ToI(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 32 bit ARGB entry to NSH 16 bit RGBA
// with transparency
unsigned short NSHWpixel8To5(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 32 bit ARGB entry to SSH 15 bit entry
// with transparency
unsigned short SSHWpixel8To5(const ARGB *aRgb8);


//------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 16 bit ARGB 4444 entry
// with transparency
unsigned short FSHWpixel8To4444(const ARGB *aRgb8);

//------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 24 bit ARGB 6666 entry
// with transparency
unsigned int   FSHWpixel8To6666(const ARGB *aRgb8);

//------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 32 bit ARGB 2101010 entry
// with transparency
unsigned int   FSHWpixel8To2101010(const ARGB *aRgb8);


//------------------------------------------------------------------------
// bit depth conversion function for GSH.
unsigned short GSHWpixel8To5(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to MSH 16 bit entry
unsigned short MSHWpixel8To565(const ARGB *aRgb8);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to MSH 16 bit ARGB 4444 entry
// with transparency
unsigned short MSHWpixel8To4444(const ARGB *aRgb8);


//------------------------------------------------------------------------
// check if it's A8 or greyscale.
int NSHWIsGreyPalette(const ARGB *colourtbl, int NbColours, int nopal);


//------------------------------------------------------------------------
// CLUT for PS2
int SHPWcsmsize(int colours);


//------------------------------------------------------------------------
// figure out the clut type based on channel depth.
int fshcluttype(int a, int r, int g, int b);
int fshadvancedcluttype(int a, int r, int g, int b);


//------------------------------------------------------------------------
// figure out clut type based on channel depth.
int xshcluttype(int a, int r, int g, int b);


//------------------------------------------------------------------------
// figure out clut type based on channel depth.
int gshcluttype(int a, int r, int g, int b);
int gshadvancedcluttype(int a, int r, int g, int b);


//------------------------------------------------------------------------
// figure out clut type based on channel depth.
int sshcluttype(int a, int r, int g, int b);


//------------------------------------------------------------------------
// figure out clut type based on channel depth.
int mshcluttype(int a, int r, int g, int b);


//*************************************************************************


//------------------------------------------------------------------------
// calculate the correct width and height for any swizzled GSH.
void RetGSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp);


//------------------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
void RetSSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled MSH.
void RetMSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp);


//------------------------------------------------------------------------
// return true when the width and height are the propper dimentions for swizzleing as 32
bool CanSwizzleSsh(int w,int h,int bpp);

//------------------------------------------------------------------------
// switching function for GameCube specific pixel offset lookups.
int SHPWswizzleGC(int x,int y,int width,int bpp);


//------------------------------------------------------------------------
// look up the pixel offset for a 4bpp GameCube Swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset4( int x, int y, int width);


//------------------------------------------------------------------------
// Look up the pixel offset for a 8bpp swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset8( int x, int y, int width);


//------------------------------------------------------------------------
// loop up the pixel offset for a 16bpp swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset16( int x, int y, int width);


//------------------------------------------------------------------------
// look up the pixel offset for 32bpp swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset32( int x, int y, int width);


//------------------------------------------------------------------------
// Look up the pixel offset for a XBox AND PC Swizzled shape,
// the XSH should really be XSH and FSH.
int SHPWswizzleXSH(int x,int y,int width,int height);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a PSP Swizzled shape,
int SHPWswizzleMSH(int x,int y,int width,int height,int bpp);

//------------------------------------------------------------------------
// PS2 Swizzleing code for 4bpp images, swizzle 4 and then unswizzle 32 so then when I upload as
// 32 and ask for a swizzle I get swizzle 4. Faster than uploading as 4bpp.
int SHAPE_swizzle4(unsigned char *p_input, unsigned char *p_output,int width, int height);


//------------------------------------------------------------------------
// PS2 Swizzleing code for 16bpp images, swizzle 16 and then unswizzle 32 so then when I upload as
// 32 and ask for a swizzle I get swizzle 16. Faster than uploading as 16bpp.
int SHAPE_swizzle16(unsigned char *src, unsigned char *dest, int width, int height);

//------------------------------------------------------------------------
// support function for SHAPE_swizzle4 PS2 swizzle function. swizzles the pages inside texture
// memory.
int PageConv4to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//------------------------------------------------------------------------
// support function for SHAPE_swizzle4() swizzles a block inside a page.
// swizzles first as 4 then unswizzles as 32.
int BlockConv4to32(unsigned char *p_input, unsigned char *p_output);


//------------------------------------------------------------------------
// PS2 swizzle function, swizzles as 8 then unswizzles as 32, for faster upload times. See
// comment on SHAPE_swizzle4 for more information.
int SHAPE_swizzle8(unsigned char *p_input, unsigned char *p_output,int width,int height);


//------------------------------------------------------------------------
// support function for SHAPE_swizzle8 PS2 swizzling function. Swizzles a single page within
// the texture space.
int PageConv8to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//------------------------------------------------------------------------
// support function for SHAPE_swizzle8() swizzles a single block withing a single page
// swizzles first as 8bpp then unswizzles as 16bpp.
int BlockConv8to32(unsigned char *p_input, unsigned char *p_output);

//SWIZZLE READ FUNCTIONS//

//------------------------------------------------------------------------
// switching function for GameCube swizzling.
int SHPRswizzleGC(int x,int y,int width,int bpp);


//------------------------------------------------------------------------
// pixel lookup function for BOTH XBox and PC Shapes. The name is a bit misleading,
// it should have both XSH and FSH in the name.
int SHPRswizzleXSH(int x,int y,int width,int height);


//------------------------------------------------------------------------
// unswizzle function for 4bpp PS2 shapes. swizzles 32 and then unswizzles 4. We usually
// swizzle 4 then unswizzle 32 so when we upload as 32 we get swizzled 4, this is faster
// than uploading 4.
int SHAPE_unswizzle4(unsigned char *p_input, unsigned char *p_output,int width, int height);


//------------------------------------------------------------------------
// unswizzle function for 16bpp PS2 shapes. swizzles 32 and then unswizzles 16. We usually
// swizzle 16 then unswizzle 32 so when we upload as 32 we get swizzled 16, this is faster
// than uploading 16.
int SHAPE_unswizzle16(unsigned char *p_input, unsigned char *p_output,int width,int height);


//------------------------------------------------------------------------
// helper function for SHAPE_unswizzle4() unswizzles a single page.
int PageConv4to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//------------------------------------------------------------------------
// helper function for SHAPE_unswizzle4() Unswizzles a single block of data.
int BlockConv4to32(unsigned char *p_input, unsigned char *p_output);


//------------------------------------------------------------------------
// PS2 swizzle function, swizzles as 8 then unswizzle 32bpp. See 4bpp verions for more info.
int SHAPE_unswizzle8(unsigned char *p_input, unsigned char *p_output,int width,int height);


//------------------------------------------------------------------------
// helper function for SHAPE_unswizzle8(), handles the unswizzleing of a single page.
int PageConv8to32(int width, int height, unsigned char *p_input, unsigned char *p_output);


//------------------------------------------------------------------------
// helper function for SHAPE_unswizzle8() swizzles 8 then unswizzles 32. A little confusing
// but it works. Look up table was generate from Sony's code.
int BlockConv8to32(unsigned char *p_input, unsigned char *p_output);


// MISC functions
//*************************************************************************

//------------------------------------------------------------------------
// scan over the entire image and determine if the image is greyscale or not.
// return non zero if true.
bool isShapeIntensityTexture(ARGB *data, const GINFO * ginfo);


//------------------------------------------------------------------------
// returns true if the name is a valid mipmap name. Ie it is either mm##
// (old format) or basetexturename_mm## (new format) otherwise returns false
// NOTE: len can be zero specifying SHPismipmap should do a strlen on the
// ID.
bool SHPismipmap(char * shape_ID,int len);


#endif

#ifdef __cplusplus
}
#endif

