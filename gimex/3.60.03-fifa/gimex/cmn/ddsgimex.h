// (c) 1994-2007 Electronic Arts Inc.

#ifndef __DDSGIMEX
#define __DDSGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  S3 Texture Gimex                                            */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL DDS_about(void);
int      GCALL DDS_is(GSTREAM *g);

/* Import Functions */

int      GCALL DDS_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL DDS_info(GINSTANCE *gx, int framenum);
int      GCALL DDS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL DDS_close(GINSTANCE *gx);

int      GCALL DDS_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL DDS_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL DDS_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#if 0
/*
C:\tools\Gimex>xd texture1.dds -l256
00000: 44445320 7c000000 07100800 00010000  DDS ............
00010: 00010000 00800000 00000000 00000000  ................
00020: 00000000 00000000 00000000 00000000  ................
00030: 00000000 00000000 00000000 00000000  ................
00040: 00000000 00000000 00000000 20000000  ............ ...
00050: 04000000 44585431 00000000 00000000  ....DXT1........
00060: 00000000 00000000 00000000 00100000  ................
00070: 00000000 00000000 00000000 00000000  ................
00080: 0000ffff ffffffff 0000ffff ffffffff  ................
00090: 0000ffff ffffffff 0000ffff ffffffff  ................
000a0: 0000ffff ffffffff 0000ffff ffffffff  ................
000b0: 0000ffff ffffffff 0000ffff ffffffff  ................
*/
#endif

typedef struct
{
    /* BIT MAP FILE HEADER structure 0x70 bytes */
    char dwVersion[4];            /* 0x91910000 */
    char dwSize[4];               /* size of surface header */
    char dwFlags[4];              /* 0x00021007 */
    char dwHeight[4];             /* height of image */
    char dwWidth[4];              /* width of image */
    char dwPitchOrLinearSize[4];  /* The pitch or number of bytes per scan line in an uncompressed texture */
    char dwDepth[4];              /* Depth of a volume texture (in pixels), otherwise unused. */
    
	char dwMipMapCount[4];        /* mip map levels */
	char dwReserved1[11 * 4];     /* Unused */


	struct
	{
		char dwSize[4];             /* sizeof pixel format */
		char dwFlags[4];            /* Values which indicate what type of data is in the surface. */
		char dwFourCC[4];           /* Four-character codes for specifying compressed or custom formats. DXT1, DXT2,  */
		char dwRGBBitCount[4];      /* Number of bits in an RGB (possibly including alpha) format. */
		char dwRBitMask[4];         /* Red (or lumiannce or Y) mask for reading color data.  */
		char dwGBitMask[4];         /* Green (or U) mask for reading color data. */
		char dwBBitMask[4];         /* Blue (or V) mask for reading color data. */
		char dwRGBAlphaBitMask[4];  /* Alpha mask for reading alpha data. dwFlags must include DDPF_ALPHAPIXELS or DDPF_ALPHA.  */
	} ddsPixelFormat;               /* Pixel format */

    char dwCaps[4];                 /* Specifies the complexity of the surfaces stored. DDSCAPS_TEXTURE | [DDSCAPS_COMPLEX|DDSCAPS_MIPMAP] */
	char dwCaps2[4];                /* Cubemap/Volume  cubmap face info */
	char dwCaps3[4];				/* Unused */
	char dwCaps4[4];                /* Unused */
	char dwReserved2[4];            /* Unused */

} DDSHeaderStruct;

/*
typedef struct {
	DXGI_FORMAT              dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	UINT                     miscFlag;
	UINT                     arraySize;
	UINT                     miscFlags2;
} DDS_HEADER_DXT10;
*/


#define DDSGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define DDSPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#define GDDSMAKEFOURCC(ch1,ch2,ch3,ch4) ((int)(ch4) | (int)(ch3 << 8) | (int)(ch2 << 16) | (int)(ch1 << 24))
#define GDDS_SUBTYPE_CUBESIMPLE  42    // DDS Cubemap subtype.
#define GDDS_SUBTYPE_CUBECOMPlEX 262144// DDS Cubemap with all 6 images in file.
#define GDDS_SUBTYPE_VOLUMETEX   256   // DDS Volume Texture Subtype.

//-------------DXT Compression information------------------
#define GDXT1FORMAT    GDDSMAKEFOURCC('D','X','T','1')
#define GDXT2FORMAT    GDDSMAKEFOURCC('D','X','T','2')
#define GDXT3FORMAT    GDDSMAKEFOURCC('D','X','T','3')
#define GDXT4FORMAT    GDDSMAKEFOURCC('D','X','T','4')
#define GDXT5FORMAT    GDDSMAKEFOURCC('D','X','T','5')
//-------------RGB Colour Mask information------------------
#define GR4444BITMASK  0xF000
#define GG4444BITMASK  0x00F0  //0x0F00
#define GB4444BITMASK  0x000F
#define GA4444BITMASK  0x0F00
#define GA555BITMASK   0x8000
#define GR555BITMASK   0x7C00
#define GG555BITMASK   0x03E0
#define GB555BITMASK   0x001F
#define GR565BITMASK   0xF800
#define GG565BITMASK   0x07E0
#define GB565BITMASK   0x001F
//------------DD Pixel Format Flags-------------------------
#define GDDPF_ALPHAPIXELS                        0x00000001l
#define GDDPF_RGB                                0x00000040l  // this is an uncompressed image.
#define GDDPF_FOURCC                             0x00000004l  // this is a fourCC image.
#define GDDPF_RGBA                               GDDPF_RGB | GDDPF_ALPHAPIXELS
//------------DDSurfaceDesc Flags---------------------------
#define GDDSD_HEIGHT             0x00000002l  // height info is valid.
#define GDDSD_WIDTH              0x00000004l  // width info is valid
#define GDDSD_PIXELFORMAT        0x00001000l  // pixel format struct is valid
#define GDDSD_MIPMAPCOUNT        0x00020000l  // mipmapcount field is valid
#define GDDSD_LINEARSIZE         0x00080000l  // linear size field is valid
#define GDDSD_CAPS               0x00000001l  // dwCaps is valid
//#define GDDSD_VOLUMETEXTURE      0x00800000l  // not fully supported Volume texture mode. only RGBA
//------------DDCapabilities Flags--------------------------
//#define GDDCAPS_ALPHA            0x00000002l  // surface supports alpha: todo yury: deprecated
#define GDDCAPS_TEXTURE          0x00001000l  // indicates surface CAN be used as a texture.
#define GDDSCAPS_COMPLEX         0x00000008l  // indicates surface has more than one layer.
#define GDDSCAPS_MIPMAP          0x00400000l  // indicates surface has mipmaps.
#define GDDSCAPS2_CUBEMAP        0x00000200l  // indicates surface is one face of a cubemap.
#define GDDSCAPS2_VOLUME         0x00200000l  // surface is a volume texture
        ///////CUBEMAP SPECIFIC DEFINES////////
#define GDDSCAPS_CUBEMAP_POSITIVEX               0x00000400l // indicates which face of the cube
#define GDDSCAPS_CUBEMAP_NEGATIVEX               0x00000800l // the current picture is set to.
#define GDDSCAPS_CUBEMAP_POSITIVEY               0x00001000l
#define GDDSCAPS_CUBEMAP_NEGATIVEY               0x00002000l
#define GDDSCAPS_CUBEMAP_POSITIVEZ               0x00004000l
#define GDDSCAPS_CUBEMAP_NEGATIVEZ               0x00008000l
#define GDDSCAPS_CUBEMAP_ALL_FACES   GDDSCAPS_CUBEMAP_POSITIVEX | GDDSCAPS_CUBEMAP_NEGATIVEX | GDDSCAPS_CUBEMAP_POSITIVEY | GDDSCAPS_CUBEMAP_NEGATIVEY | GDDSCAPS_CUBEMAP_POSITIVEZ | GDDSCAPS_CUBEMAP_NEGATIVEZ 


#ifdef __cplusplus
}
#endif
#endif

