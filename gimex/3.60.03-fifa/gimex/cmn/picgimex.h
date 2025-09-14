// (c) 1994-2007 Electronic Arts Inc.

#ifndef __PICTGIMEX
#define __PICTGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  PICT Gimex                                                  */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL PIC_about(void);
int      GCALL PIC_is(GSTREAM *g);

/* Import Functions */

int      GCALL PIC_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL PIC_info(GINSTANCE *gx, int framenum);
int      GCALL PIC_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL PIC_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL PIC_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL PIC_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL PIC_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

#define SHIFT5TO8(x) (((x)<<3)|(x)>>2)

#define PICTHEADERSIZE  512

#ifdef applec

/* Mac specific */

#include "memory.h"
#include "QDOffscreen.h"
#include "PictUtil.h"

CTabHandle PICBuildColorTable(GINFO* ginfo, int colorCount);

#endif

/* All other machines do PICT the hard way... */

typedef signed char Int8;
typedef unsigned char UInt8;
typedef short Int16;
typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char *BytePtr;
#ifndef applec
/* Mac already defines this */
typedef unsigned char Byte;
#endif
typedef int BoolEnum;

#define BOOL_FALSE 0
#define BOOL_TRUE 1

typedef struct _PICTRECT
{
    Int16  left;
    Int16  top;
    Int16  right;
    Int16  bottom;
} PICT_RECT;

typedef struct _PICTHDR1
{
    UInt16    FileSize1;
    PICT_RECT Frame;
    UInt16    VerOpcode;
    UInt16    VerNum;
} PICTHDR1;

/* Header 2 for Version 2 only */

typedef struct _PICTHDR2
{
    UInt16    HdrOpcode;       /* Version 2 Header opcode */
    Int16     HdrOpcodeData;
    UInt16    Reserved1;
    UInt32    HorzPixPerInch;
    UInt32    VertPixPerInch;
    PICT_RECT Frame;
    UInt32    Reserved2;
} PICTHDR2;

typedef struct _PICTPIXMAP
{
    UInt32            BaseAddr;
    UInt16            RowBytes;
    PICT_RECT         Bounds;
    UInt16            Version;
    UInt16            PkngType;
    UInt32            PkdSize;
    UInt32            SrcXRes; /* fixed point */
    UInt32            SrcYRes; /* fixed point */
    UInt16            PixType;
    UInt16            PixSize;
    UInt16            NumPlanes;
    UInt16            CompSize;
    UInt16            pmTable;
    UInt16            pmReserved;
} PICT_PIXMAP;

typedef struct _PICTRGB16
{
    UInt16 red;
    UInt16 green;
    UInt16 blue;
} PICT_RGB16;

typedef struct
{
    GINFO *ginfo;
    char  *dest;
    int   rowbytes;
    int   frames;
} PICT;

/* PICT Version No. */

/* Use logical AND   (VerNum & PICT_VERS_NUM_1) */

#define PICT_VERS_OPCODE_1 0x11
#define PICT_VERS_OPCODE_2 0x0011
#define PICT_VERS_NUM_1 0x01
#define PICT_VERS_NUM_2 0x02FF
#define PICT_VERS2_HDR_OPCODE 0x0C00

/* PICT Version 1 Opcode defines */

#define PICTOP_NOP 0x0000
#define PICTOP_CLIPREGION 0x0001
#define PICTOP_BKGPATTERN 0x0002
#define PICTOP_TEXTFONT 0x0003
#define PICTOP_TEXTTYPEFACE 0x0004
#define PICTOP_TEXTMODE 0x0005
#define PICTOP_TEXTEXTRASPACE 0x0006
#define PICTOP_PENSIZE 0x0007
#define PICTOP_PENMODE 0x0008
#define PICTOP_PENPATTERN 0x0009
#define PICTOP_FILLPATTERN 0x000A
#define PICTOP_OVALSIZE 0x000B
#define PICTOP_ORIGINPOINT 0x000C
#define PICTOP_TEXTSIZE 0x000D
#define PICTOP_FRGNDCOLOR 0x000E
#define PICTOP_BKGNDCOLOR 0x000F
#define PICTOP_TEXTRATIO 0x0010
#define PICTOP_PICTVERSION 0x0011
#define PICTOP_LINE 0x0020
#define PICTOP_LINEFROM 0x0021
#define PICTOP_SHORTLINE 0x0022
#define PICTOP_SHORTLINEFROM 0x0023
#define PICTOP_LONGTEXT 0x0028
#define PICTOP_DHTEXT 0x0029
#define PICTOP_DVTEXT 0x002A
#define PICTOP_DHDV 0x002B

#define PICTOP_FRAMERECT 0x0030
#define PICTOP_PAINTRECT 0x0031
#define PICTOP_ERASERECT 0x0032
#define PICTOP_INVERTRECT 0x0033
#define PICTOP_FILLRECT 0x0034

#define PICTOP_FRAMESAMERECT 0x0038
#define PICTOP_PAINTSAMERECT 0x0039
#define PICTOP_ERASESAMERECT 0x003A
#define PICTOP_INVERTSAMERECT 0x003B
#define PICTOP_FILLSAMERECT 0x003C

#define PICTOP_FAMEROUNDEDRECT 0x0040
#define PICTOP_PAINTRNDRECT 0x0041
#define PICTOP_ERASERNDRECT 0x0042
#define PICTOP_INVERTRNDRECT 0x0043
#define PICTOP_FILLRNDRECT  0x0044

#define PICTOP_FRAMESAMERNDRECT 0x0048
#define PICTOP_PAINTSAMERNDRECT 0x0049
#define PICTOP_ERASESAMERNDRECT 0x004A
#define PICTOP_INVERTSAMERNDRECT 0x004B
#define PICTOP_FILLSAMERNDRECT 0x004C

#define PICTOP_FRAMEOVAL 0x0050
#define PICTOP_PAINTOVAL 0x0051
#define PICTOP_ERASEOVAL 0x0052
#define PICTOP_INVERTOVAL 0x0053
#define PICTOP_FILLOVAL  0x0054

#define PICTOP_FRAMESAMEOVAL 0x0058
#define PICTOP_PAINTSAMEOVAL 0x0059
#define PICTOP_ERASESAMEOVAL 0x005A
#define PICTOP_INVERTSAMEOVAL 0x005B
#define PICTOP_FILLSAMEOVAL 0x005C

#define PICTOP_FRAMEARC 0x0060
#define PICTOP_PAINTARC 0x0061
#define PICTOP_ERASEARC 0x0062
#define PICTOP_INVERTARC 0x0063
#define PICTOP_FILLARC 0x0064

#define PICTOP_FRAMESAMEARC 0x0068
#define PICTOP_PAINTSAMEARC 0x0069
#define PICTOP_ERASESAMEARC 0x006A
#define PICTOP_INVERTSAMEARC 0x006B
#define PICTOP_FILLSAMEARC 0x006C

#define PICTOP_FRAMEPOLYGON 0x0070
#define PICTOP_PAINTPOLYGON 0x0071
#define PICTOP_ERASEPOLYGON 0x0072
#define PICTOP_INVERTPOLYGON 0x0073
#define PICTOP_FILLPOLYGON 0x0074

#define PICTOP_FRAMESAMEPOLYGON 0x0078
#define PICTOP_PAINTSAMEPOLYGON 0x0079
#define PICTOP_ERASESAMEPOLYGON 0x007A
#define PICTOP_INVERTSAMEPOLYGON 0x007B
#define PICTOP_FILLSAMEPOLYGON 0x007C

#define PICTOP_FRAMEREGION 0x0080
#define PICTOP_PAINTREGION 0x0081
#define PICTOP_ERASEREGION 0x0082
#define PICTOP_INVERTREGION 0x0083
#define PICTOP_FILLREGION 0x0084

#define PICTOP_FRAMESAMEREGION 0x0088
#define PICTOP_PAINTSAMEREGION 0x0089
#define PICTOP_ERASESAMEREGION 0x008A
#define PICTOP_INVERTSAMEREGION 0x008B
#define PICTOP_FILLSAMEREGION 0x008C

#define PICTOP_BITSRECT 0x0090
#define PICTOP_BITSREGION 0x0091
#define PICTOP_PKBITSRECT 0x0098
#define PICTOP_PKBITSREGION 0x0099
#define PICTOP_SHORTCOMMENT 0x00A0
#define PICTOP_LONGCOMMENT 0x00A1
#define PICTOP_ENDOFPICTURE 0x00FF


/* PICTOP Version 2.0 Opcodes */
#define PICTOP_COLORBKGNDPATTERN 0x0012
#define PICTOP_COLORPENPATTERN 0x0013
#define PICTOP_COLORFILLPATTERN 0x0014
#define PICTOP_FRXNLPENPOS 0x0015
#define PICTOP_EXTRASPACEPERCHAR 0x0016
#define PICTOP_RGBFRGNDCOLOR 0x001A
#define PICTOP_RGBBKGNDCOLOR 0x001B
#define PICTOP_HILIGHTMODE  0x001C
#define PICTOP_HILIGHTCOLOR 0x001D
#define PICTOP_USEHILIGHTCOLORDEF 0x001E
#define PICTOP_RGBOPCOLOR 0x001F
#define PICTOP_PIXELRECT 0x0090
#define PICTOP_PIXELREGION 0x0091
#define PICTOP_PKPIXELRECT 0x0098
#define PICTOP_PKPIXELREGION 0x0099
#define PICTOP_HEADER 0x0C00

/* V2.0 Extension */
#define PICTOP_DIRECTBITSRECT 0x009A
#define PICTOP_DIRECTBITSREGION 0x009B
#define PICTOP_COMPRESSEDQUICKTIME 0x8200
#define PICTOP_UNCOMPRESSEDQUICKTIME 0x8201


typedef struct _PICTPT
{
    Int16 dh;
    Int16 dv;
} PICT_PT;

#define PICT_USEPIXMAP     0x8000


#define PICGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define PICPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif

