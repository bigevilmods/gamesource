// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"


#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  SHP Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL SHP_about(void);
int      GCALL SHP_is(GSTREAM *g);

/* Import Functions */

int      GCALL SHP_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL SHP_info(GINSTANCE *gx, int framenum);
int      GCALL SHP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL SHP_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL SHP_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL SHP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL SHP_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef __SHPGIMEX
#define __SHPGIMEX 1

#define SHP_VERSION 427

#define SHAPEFILE 0
#define STREAMINGSHAPE 1

#include <string.h>

/* Shape File entries */

typedef struct ItemShapeTbl
{
    char  OffsetShape[4];   /*  The Offset for this item */
    char  SizeShape[4];     /*  The Offset for this item */
    char  NameShape[1];       /*  The Name for this item - NULL + strlen*/

} ItemShapeTbl;

typedef struct ShapeTbl
{
    ItemShapeTbl    ShapeTable[1];      /*  The table that describe the file */

} ShapeTbl;

typedef struct SHPFILEHeaderStruct
{
    /* SHP SHAPE FILE HEADER structure 16 bytes */

    char typeFile[4];        /*  'ShpS' for Sony */
    char LengthFile[4];      /*  The length of the file in byte */
    char NumShape[4];        /*  The Number of Shape in this file */
    char Creator[4];         /*  The creator */

} SHPFILEHeaderStruct;

// flags field for shape elements.  straight from realshape

enum ElementFlags
{
    ELEMENT_FLAG_NEW_FORMAT   = 0x000001,
    ELEMENT_FLAG_COMPRESSED   = 0x000002,
    ELEMENT_FLAG_OWNED        = 0x000004,
    ELEMENT_FLAG_OWNED_DATA   = 0x000008
//  ELEMENT_FLAG_RESERVED     = 0x0000F0
};

enum TextureFlags
{
    TEXTURE_FLAG_CUBE_MAP     = 0x000100,
    TEXTURE_FLAG_DOT3         = 0x000200,
    TEXTURE_FLAG_EMBM         = 0x000400,
    TEXTURE_FLAG_HALF_ALPHA   = 0x000800,
    TEXTURE_FLAG_TRANSPARENT  = 0x001000,
    TEXTURE_FLAG_OPAQUE       = 0x002000,
    TEXTURE_FLAG_SWIZZLED     = 0x004000,
    TEXTURE_FLAG_TRANSPOSED   = 0x008000,
	TEXTURE_FLAG_MIPMONT      = 0x010000,
//  TEXTURE_FLAG_RESERVED     = 0x0E0000,
    TEXTURE_FLAG_NUM_MIP_MAPS = 0xF00000
};

enum compress_type
{
    COMPRESS_NON              = 0,   // must be zero
    COMPRESS_RLE              = 1,
    COMPRESS_REF              = 2,
    COMPRESS_DXT              = 3,
    COMPRESS_MPG              = 4,
	COMPRESS_MV				  = 5,
    COMPRESS_GST              = 6
};

/* shape */

typedef struct SHPHeaderStruct
{
    char type;               /*  The type for this shape */
    char flags[3];           /*  Flags for this shape */
    char mNext[4];           /*  The mNext to the mNext shape */
    char DataOffset[4];      /*  Offset to data of this shape */
    char DataSize[4];        /*  Size of data of this shape */

    char shapex[4];          /*  position of the Shape */
    char shapey[4];          /*  position of the Shape */
    char width[4];           /*  width of the Shape */
    char height[4];          /*  height of the Shape */

} SHPHeaderStruct;

/* clut */

typedef struct CLUTHeaderStruct
{
    char type;               /*  The type for this shape */
    char flags[3];           /*  Flags for this shape */
    char mNext[4];           /*  The mNext to the mNext shape */
    char DataOffset[4];      /*  Offset to data of this shape */
    char DataSize[4];        /*  Size of data of this shape */

    char shapex[4];          /*  position of the Shape */
    char shapey[4];          /*  position of the Shape */
    char width[4];           /*  width of the Shape */
    char height[4];          /*  height of the Shape */
} CLUTHeaderStruct;

/* hot spot */

typedef struct HOTHeaderStruct
{
    char type;               /*  The type for this shape */
    char flags[3];           /*  Flags for this shape */
    char mNext[4];           /*  The mNext to the mNext shape */
    char DataOffset[4];      /*  Offset to data of this shape */
    char DataSize[4];        /*  Size of data of this shape */

    char centerx[4];         /*  center point of the Shape */
    char centery[4];         /*  center point of the Shape */
    char Dimension[4];       /*  Number of values in a Hotspot (2) */
    char NumHotSpot[4];      /*  The number of HotSpot */

} HOTHeaderStruct;

typedef struct CommentHeaderStruct
{
    char type;               /*  The type for this shape */
    char flags[3];           /*  Flags for this shape */
    char mNext[4];           /*  The mNext to the mNext shape */
    char DataOffset[4];      /*  Offset to data of this shape */
    char DataSize[4];        /*  Size of data of this shape */

} COMMENTHeaderStruct;


typedef struct MetalHeaderStruct
{
    char type;               /*  The type for this shape */
    char flags[3];           /*  Flags for this shape */
    char mNext[4];           /*  The mNext to the mNext shape */
    char DataOffset[4];      /*  Offset to data of this shape */
    char DataSize[4];        /*  Size of data of this shape */

} MetalHeaderStruct;

typedef struct DataHeaderStruct
{
    char type;               /*  The type for the dataBin */
    char flags[3];           /*  Flags for dataBin, currently only 1 */
    char mNext[4];           /*  The mNext to the mNext shape */
    char DataOffset[4];      /*  Offset to data of this shape */
    char DataSize[4];        /*  Size of data of this shape */
} DataHeaderStruct;


/* internal shape list */

typedef struct
{
    char *	shape_Data;         /* The basic shape found on the file */
    int		shape_SizeData;     /* The size of this shape (write) */
    char	shape_ClutID[4];    /* The signature of this clut */
    char *	shape_Clut;         /* The Clut mNext to this shape */
    int		shape_SizeClut;     /* The size of this Clut (write) */
    char *	shape_HotSpot;      /* The HotSpot mNext to this shape */
    int		shape_SizeHotSpot;  /* The size of this HotSpot (write) */
    char *	shape_Comment;      /* The Comment mNext to this shape */
    int		shape_SizeComment;  /* The size of this Comment (write) */
    char *	shape_Metal;        /* The Metal mNext to this shape */
    int		shape_SizeMetal;    /* The size of this Metal (write) */\
    char *	shape_DataBin;      /* The DataBin mNext to this shape */
    int		shape_SizeDataBin;  /* The size of this DataBin (write) */
    char *	shape_Name;         /* The name of of this shape */
    int		subtype;            /* Subtype for shape */
    int		packed;             /* Packed method (0=none,2=refpack,3=dxt,4=mpeg) (read) */
    int		framesize;          /* Total data size (read) */
    int		align;              /* Alignment to write shape data to (write) */
	int		binsize;			/* Bindata size */
} GShapeInfo;

typedef struct
{
    int        NumShape;           /* The Number of Shape in this file */
    GShapeInfo ListShape[1];       /* The list of shape for this composite file */
} ShapeList, *ShapeListPtr;

// NOTE GINFOSHP used by OLDSHPGIMEX.h as well
typedef struct
{
    GINFO   ginfo;              /* standard ginfo */
    int     align;              /* alignment of texture (ie 128) */
    int     creator;            /* 4 char code of creator (app/version ie 'G231') */
	int		binsize;			/* Bindata size */

//    int     clutx;              /* clut position x */
//    int     cluty;              /* clut position y */
//    int     tpage;
//    char    clutid;
//    char    code;
//    char    pad[2];             /* pad to 4 bytes */

//    int     numanims;           /* number of palette anim ranges */
//    PALANIM palanim[MAXPALANIMS]; /* palette animation ranges */


} GINFOSHP;

typedef struct yumcoorddef
{
    int x;
    int y;
    int z;
} YUMCOORD;


#include "shpcmn.h"

#endif

#define CleanField(Field,type)  Field=ggeti((void*)&(Field),sizeof(type))
#define SetIField(Field,type)   gputi((void*)&(Field),(int)Field,sizeof(type))

#define SHPGETFIELD(field) ggetx((void *)&field, sizeof(field))
#define SHPPUTFIELD(field,data) gputx((void *)&field,(unsigned int)(data),sizeof(field))

/* cleanup macros */

#ifndef SHPMAKECODE

#undef SHP_about
#undef SHP_is
#undef SHP_open
#undef SHP_info
#undef SHP_read
#undef SHP_close
#undef SHP_wopen
#undef SHP_write
#undef SHP_wclose

#undef SHPSIGNATURE
#undef FNTSIGNATURE
#undef SHPMULTIFRAME
#undef SHPEXTENSION0
#undef SHPEXTENSION1
#undef SHPEXTENSION2
#undef SHPEXTENSION3
#undef SHPMACTYPE0
#undef SHPMACTYPE1
#undef SHPMACTYPE2
#undef SHPMACTYPE3
#undef SHPSHORTTYPE
#undef SHPWORDTYPE
#undef SHPLONGTYPE
#undef SHPDATAALIGNMENT
#undef SHPMIPMAPDATAALIGNMENT
#undef SHPCLUTDATAALIGNMENT
#undef SHPPACKEDDATAALIGNMENT

#undef ggetx
#undef gputx
#undef SHPGETFIELD
#undef SHPPUTFIELD

#endif

#ifdef __cplusplus
}
#endif

