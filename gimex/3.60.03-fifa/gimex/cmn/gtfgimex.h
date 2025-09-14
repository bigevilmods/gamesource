// (c) 1994-2007 Electronic Arts Inc.

#ifndef __GTFGIMEX
#define __GTFGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GTF_MAXFRAMENAME 0

typedef enum {

	// SetSurface
	CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5	= (1),
	CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5	= (2),
	CELL_GCM_SURFACE_R5G6B5				= (3),
	CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8	= (4),
	CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8	= (5),
	CELL_GCM_SURFACE_A8R8G8B8			= (8),
	CELL_GCM_SURFACE_B8					= (9),
	CELL_GCM_SURFACE_G8B8				= (10),
	CELL_GCM_SURFACE_F_W16Z16Y16X16		= (11),
	CELL_GCM_SURFACE_F_W32Z32Y32X32		= (12),
	CELL_GCM_SURFACE_F_X32				= (13),
	CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8	= (14),
	CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8	= (15),
	CELL_GCM_SURFACE_A8B8G8R8			= (16),

	// SetTexture
	CELL_GCM_TEXTURE_B8						= (0x81),
	CELL_GCM_TEXTURE_A1R5G5B5				= (0x82),
	CELL_GCM_TEXTURE_A4R4G4B4				= (0x83),
	CELL_GCM_TEXTURE_R5G6B5					= (0x84),
	CELL_GCM_TEXTURE_A8R8G8B8				= (0x85),
	CELL_GCM_TEXTURE_COMPRESSED_DXT1		= (0x86),
	CELL_GCM_TEXTURE_COMPRESSED_DXT23		= (0x87),
	CELL_GCM_TEXTURE_COMPRESSED_DXT45		= (0x88),
	CELL_GCM_TEXTURE_G8B8					= (0x8B),
	CELL_GCM_TEXTURE_R6G5B5					= (0x8F),
	CELL_GCM_TEXTURE_DEPTH24_D8				= (0x90),
	CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT		= (0x91),
	CELL_GCM_TEXTURE_DEPTH16				= (0x92),
	CELL_GCM_TEXTURE_DEPTH16_FLOAT			= (0x93),
	CELL_GCM_TEXTURE_X16					= (0x94),
	CELL_GCM_TEXTURE_Y16_X16				= (0x95),
	CELL_GCM_TEXTURE_R5G5B5A1				= (0x97),
	CELL_GCM_TEXTURE_COMPRESSED_HILO8		= (0x98),
	CELL_GCM_TEXTURE_COMPRESSED_HILO_S8		= (0x99),
	CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT	= (0x9A),
	CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT	= (0x9B),
	CELL_GCM_TEXTURE_X32_FLOAT				= (0x9C),
	CELL_GCM_TEXTURE_D1R5G5B5				= (0x9D),
	CELL_GCM_TEXTURE_D8R8G8B8				= (0x9E),
	CELL_GCM_TEXTURE_Y16_X16_FLOAT			= (0x9F),
	CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8	= (0xAD),
	CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8	= (0xAE),

	CELL_GCM_TEXTURE_SZ	= (0x00),
	CELL_GCM_TEXTURE_LN	= (0x20),
	CELL_GCM_TEXTURE_NR	= (0x00),
	CELL_GCM_TEXTURE_UN	= (0x40),

} CellGcmEnum;


/****************************************************************/
/*  GTF Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL GTF_about(void);
int      GCALL GTF_is(GSTREAM *g);

/* Import Functions */

int      GCALL GTF_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL GTF_info(GINSTANCE *gx, int framenum);
int      GCALL GTF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL GTF_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL GTF_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL GTF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL GTF_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

