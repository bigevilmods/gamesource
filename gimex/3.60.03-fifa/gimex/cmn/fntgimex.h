// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"


#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  FNT Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL FNT_about(void);
int      GCALL FNT_is(GSTREAM *g);

/* Import Functions */

int      GCALL FNT_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL FNT_info(GINSTANCE *gx, int framenum);
int      GCALL FNT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL FNT_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL FNT_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL FNT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL FNT_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef __FNTGIMEX
#define __FNTGIMEX 1

#define FNT_VERSION 414

/******************************************************************/
/* Font build codes, these are used to determine which platform   */
/* the internal functions in fntread, fntwrite are being compiled */
/* for.  See fntgimex.doc for more information.                   */
/******************************************************************/


#define FNTTYPE_FFN     1
#define FNTTYPE_PFN     2
#define FNTTYPE_SFN     3
#define FNTTYPE_3FN     4
#define FNTTYPE_NFN     5
#define FNTTYPE_GFN     6
#define FNTTYPE_MFN     7
#define FNTTYPE_XFN     8
#define FNTTYPE_AFN     9  /* gba */
#define FNTTYPE_ZFN     10 /* XENON */
#define FNTTYPE_YFN     11 /* PS3 */

#include <string.h>

enum FONTTextureFlags
{
    FONT_TEXTURE_FLAG_NEW          = 0x000001,
	FONT_TEXTURE_FLAG_SWIZZLED     = 0x004000
};


struct SHAPE
{
    // Shape2 texture
    char mtype;               /*  The type for this shape */
    char mFlags[3];           /*  Flags for this shape */
    char mNext[4];            /*  The mNext to the mNext shape */
    char mDataOffset[4];      /*  Offset to data of this shape */
    char mDataSize[4];        /*  Size of data of this shape */

    char mshapex[4];          /*  position of the Shape */
    char mshapey[4];          /*  position of the Shape */
    char mwidth[4];           /*  width of the Shape */
    char mheight[4];          /*  height of the Shape */

    // Shape1

    unsigned char   type;           /* type for this shape */
    char            link[3];        /* link to next shape */
    char            width[2];       /* width of shape bitmap */
    char            height[2];      /* height of shape bitmap */
    char            centerx[2];     /* center point for shape */
    char            centery[2];     /* center point for shape */
    char            shapex[2];      /* position of shape */
    char            shapey[2];      /* position of shape */
};

/* clut */
struct CLUTRHeaderStruct
{
    // Shape2 clut
    char mtype;               /*  The type for this shape */
    char mFlags[3];           /*  Flags for this shape */
    char mNext[4];            /*  The mNext to the mNext shape */
    char mDataOffset[4];      /*  Offset to data of this shape */
    char mDataSize[4];        /*  Size of data of this shape */

    char mshapex[4];          /*  position of the Shape */
    char mshapey[4];          /*  position of the Shape */
    char mwidth[4];           /*  width of the Shape */
    char mheight[4];          /*  height of the Shape */

    // Shape1 clut

    char type;               /*  The type for this shape */
    char LinkShape[3];       /*  The Link to the next shape */
    char width[2];           /*  width of the Shape */
    char height[2];          /*  height of the Shape */
    char centerx[2];         /*  center point of the Shape */
    char centery[2];         /*  center point of the Shape */
    char shapex[2];          /*  position of the Shape */
    char shapey[2];          /*  position of the Shape */
    char Entries[1][2];      /*  The colours */
};


struct FONTMetalHeaderStruct
{
    // Shape2
    char mtype;               /*  The type for this shape */
    char mFlags[3];           /*  Flags for this shape */
    char mNext[4];            /*  The mNext to the mNext shape */
    char mDataOffset[4];      /*  Offset to data of this shape */
    char mDataSize[4];        /*  Size of data of this shape */

    // Shape1
    char type;               /*  The type for this shape */
    char link[3];            /*  Flags for this shape */
    char metalsize[2];
    char metalbits[2];
    char pad[8];
};

static const float FONT_ZNEAR = 0.0f;
static const float FONT_WNEAR = 1.0f;

// states
static const int FONT_COLOUR      = 0;  /*!< \i (int)   current font colour as ARGB (0xffffffff) */
static const int FONT_HEIGHT      = 1;  /*!< \i (int)   height to advance Y by */
static const int FONT_Z           = 2;  /*!< \i (float) z value ZNEAR */
static const int FONT_W           = 3;  /*!< \i (float) rhw value WNEAR */
static const int FONT_SHAPE       = 4;  /*!< \i (int)   offset to Shape header from start of file */
static const int FONT_UNDEF       = 5;  /*!< \i (int)   unicode character code to draw for undefined characters (default 0x007f) */
static const int FONT_SCALE_X     = 6;  /*!< \i (float) scale x factor (1.0f) */
static const int FONT_SCALE_Y     = 7;  /*!< \i (float) scale y factor (1.0f) */
static const int FONT_SCALE_U     = 8;  /*!< \i (float) reciprocal of font shape width to scale UV by, except for XBox linear where it is 1.0 */
static const int FONT_SCALE_V     = 9;  /*!< \i (float) reciprocal of font shape height to scale UV by, except for XBox linear where it is 1.0 */
static const int FONT_FILTER      = 10; /*!< \i (int)   point or bilinear filtering */
static const int FONT_DEPTHWRITE  = 11; /*!< \i (int)   enable/disable writes to the depth buffer */
static const int FONT_DRAWLISTMAX = 12; /*!< \i (int)   maximum number of characters in a drawlist (default 128) */
static const int FONT_ANGLE       = 13; /*!< \i (float) angle to draw rotated font at in radians */
static const int FONT_DANGLE      = 14; /*!< \i (float) change in angle per pixel advance in radians (for arcs) */
static const int FONT_BLEND       = 15; /*!< \i (int)   alpha blend mode (0=none, 1=alpha blend, 2=alpha test) */


#define FONT_RW 23
#define FONT_RH 22

typedef struct
{
    char            signature[4];   /*  0 signature of font (ie 'FNTX') */
    char            size[4];        /*  4 size of entire file in bytes */
    char            version[2];     /*  8 format vesion (~100 = 11 byte, 200 = 12byte index, >210 = 128 byte font header, >300 char12,kern4, >350 char16 */
    char            num[2];         /* 10 number of characters defined in font */
    char            flags[4];       /* 12 font flags */
    signed char     centerx;        /* 16 xoffset delta for all characters */
    signed char     centery;        /* 17 yoffset delta for all characters */
    signed char     ascent;         /* 18 distance from baseline to top of font */
    signed char     descent;        /* 19 distance from baseline to bottom of font */
    char            chartbl[4];     /* 20 offset to first character info struct from start of file in bytes */
    char            kerntbl[4];     /* 24 offset to kern table from start of file in bytes */
    char            shape[4];       /* 28 offset to shape header from start of file in bytes */
    char            states[24][4];  /* 32..127 driver state area (24 ints) */
} FONTHEADER;

typedef struct CHARACTER11
{
    unsigned char   index[2];
    unsigned char   width;
    unsigned char   height;
    unsigned char   u[2];
    unsigned char   v[2];
    signed char     advance;  // advance x
    signed char     xoffset;
    signed char     yoffset;
} CHARACTER11;

/* sizeof(CHARACTER11) will return 12 */
#define CHARACTERSIZE11 11

typedef struct CHARACTER12
{
    unsigned char   index[2];
    unsigned char   width;
    unsigned char   height;
    unsigned char   u[2];
    unsigned char   v[2];
    signed char     advance;  // advance x (note change in behavior)
    signed char     xoffset;
    signed char     yoffset;
    unsigned char   numkern;
} CHARACTER12;

#define CHARACTERSIZE12 sizeof(CHARACTER12)

typedef struct CHARACTER16
{
    unsigned char   index[2];       /*    unicode value of character */
    unsigned char   width;          /* -> width in pixels */
    unsigned char   height;         /* -> height in pixels */
    unsigned char   u[2];           /* -> u in pixels */
    unsigned char   v[2];           /* -> v in pixels */
    signed char     advancey;       /*    Y increment in pixels */
    signed char     xoffset;        /* -> X offset to draw character in pixels */
    signed char     yoffset;        /* -> Y offset to draw character in pixels */
    unsigned char   numkern;        /*    number of kerning entries */
    unsigned char	kernindex[2];   /*    index into kern table */
    signed char     advancex[2];    /*    X increment in pixels */

} CHARACTER16;

#define CHARACTERSIZE16 sizeof(CHARACTER16)

// old KERN8 format for version 200..299
typedef struct KERN8
{
    unsigned char previouscode[2];   /* character preceding about to be kerned character (in "He" this would be 'H') */
    unsigned char code[2];           /* character about to be kerned (in "He" this would be 'e') */
    signed   char kernvalue[1];      /* signed amount to kern by, - kerns to the left, + kerns to the right */
    char          pad[3];            /* pad structure to 8 bytes to prevent surprises when structure padding is turned on */
} KERN8;

// KERN format for version 300

typedef struct KERN
{
    unsigned char previouscode[2];   /* character preceding about to be kerned character (in "He" this would be 'H') */
    signed   char kernvalue[1];      /* signed amount to kern by, - kerns to the left, + kerns to the right */
    unsigned char code[1];           /* pad structure to 4 bytes with low bits of code */
} KERN;

/* sign extend byte */
#define SXB(b) (((int)((unsigned int)(b)<<24))>>24)

typedef struct
{
    /* stored in gref field in the current GINSTANCE */

    FONTHEADER  *header;    /* pointer to font header */
    CHARACTER12 *chartbl;   /* pointer to list of character info structures, Note: only one of chartbl and chartbl16 have data at once */
	CHARACTER16 *chartbl16; /* pointer to list of character16 info structures */
    SHAPE*      shape;      /* pointer to shape for font */
    int         bpp;        /* depth of font */

	int* kerntbl;
	int  kernpairs;
    CLUTRHeaderStruct * clut;
    int  numcolours;            /* store the number of colours in the clut */
    int  hasclut;

} FONTREADINFO;

typedef struct
{
    /* stored in gref field in the current GINSTANCE */

    FONTHEADER  *header;        /* pointer to font header */
    CHARACTER12 *chartbl;       /* pointer to list of character info structures */
    CHARACTER16 *chartbl16;		/* pointer to list of character info structures 16 bytes each*/
	char**      frameList;      /* array of pointers to frame buffers for each character */
    int*        rowBytesList;   /* array of pointers to rowbytes for each character */
    int         bpp;            /* depth of font */
    int         fontheight;     /* height of font (advancy - normally ascent + descent) */
    int         fontascent;     /* distance from top to baseline */
    int         fontdescent;    /* distance from baseline to bottom */

	int* kerntbl;
	int  kernpairs;
    ARGB * clut;                /* store the clut here and write out if necessary */
    ARGB * originalclut;        /* keep a copy of clut for alpha mapping purposes */
	int  numcolours;            /* store the number of colours in the clut */
    int  alphabits;
    int  redbits;
    int  bluebits;
    int  greenbits;
    int  subtype;
    int  originalbpp;
	int  packed;				/* type of compression (ie. DXT) */
	int  characterinfosize;		/* size of the character info structs (ie. 12 oe 16 bytes) */
} FONTWRITEINFO;


//---------------------------------------------------------------------------------------------

// table copied from getoffset.cpp in REAL ( \real\shape\xbox)

// swizzled offsets are found by interleaving the x and y bits of
// the linear texel coordinate.
// this table expands an 8-bit value into 16-bits for interleaving
// eg b00001101 becomes     b0000000001010001

static const unsigned short ExpandTableXbox[256] = {

0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555,
};

//---------------------------------------------------------------------------------------------


#endif




/****************************************************************/
/* cleanup macros                                               */
/****************************************************************/

#ifndef FNTTYPE

#undef FNT_about
#undef FNT_is
#undef FNT_open
#undef FNT_info
#undef FNT_read
#undef FNT_close
#undef FNT_wopen
#undef FNT_write
#undef FNT_wclose

#undef SIGNATURE
#undef OLDSIGNATURE
#undef EXTENSION0
#undef EXTENSION1
#undef EXTENSION2
#undef EXTENSION3
#undef MACTYPE0
#undef MACTYPE1
#undef MACTYPE2
#undef MACTYPE3
#undef SHORTTYPE
#undef WORDTYPE
#undef LONGTYPE

#undef SHAPECODE4
#undef SHAPECODE8
#undef SHAPECODE16
#undef SHAPECODE32

#undef FNTGETMEM
#undef FNTPUTMEM
#undef FNTGETFIELD
#undef FNTPUTFIELD

#endif

#ifdef __cplusplus
}

#endif
