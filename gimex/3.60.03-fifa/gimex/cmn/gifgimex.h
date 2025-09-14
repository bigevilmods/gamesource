// (c) 1994-2007 Electronic Arts Inc.

#ifndef __GIFGIMEX
#define __GIFGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  GIF Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL GIF_about(void);
int      GCALL GIF_is(GSTREAM *g);

/* Import Functions */

int      GCALL GIF_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL GIF_info(GINSTANCE *gx, int framenum);
int      GCALL GIF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL GIF_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL GIF_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL GIF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL GIF_wclose(GINSTANCE *gx);


/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef GIF_LIB_H
#define GIF_LIB_H

#define ERROR           0
#define OK              1

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

#define FILE_BUFFER_SIZE 16384       /* Files uses bigger buffers than usual */

#define MESSAGE(s)      fprintf(stderr, "\n%s: %s\n", __FILE__, s)
#define EXIT(s)         { MESSAGE(s); exit(-3); }

typedef struct GifColorType
{
    unsigned char Red, Green, Blue;
} GifColorType;

typedef enum
{
    GIF_UNKNOWN = 0x00,
    GIF_NO_DISPOSE = 0x04,
    GIF_BACKGROUND = 0x08,
    GIF_PREVIOUS = 0x0C
} GifDisposalType;

/* Note entries prefixed with S are of Screen information, while entries     */
/* prefixed with I are of the current defined Image.                         */

typedef struct GifFileType
{
    int SWidth, SHeight,                       /* Screen dimensions */
        SColorResolution, SBitsPerPixel;        /* How many colors can we generate? */
    int SBackGroundColor,                      /* I hope you understand this one... */
        ILeft, ITop, IWidth, IHeight,           /* Current image dimensions */
        IInterlace,                             /* Sequential/Interlaced lines */
        IBitsPerPixel;                          /* How many colors this image has? */
    int TransparentIndex;                      /* Which colour is transparent */
    int DelayTime;                             /* Speed of movie */
    GifDisposalType IDisposal;                  /* Image disposal flag */
    unsigned int SFilePosition;              /* Position of last file write */
    GifColorType *SColorMap, *IColorMap;        /* NULL if not exists */
    int SFrames;                               /* Number of frames/images in file */
    void *Private;                             /* The regular user should not mess with this one! */
} GifFileType;

typedef struct GifInternal                     // Internal Gif multiframe managment strucutre, used with gx->gref
{
    int HeaderPos;                             // location of the LSD Header in file.
    int LogicalScreenWidth;                    // Current LSW this is needed as future frames may have larger LSW and LSD needs to be updated
    int LogicalScreenHeight;
    GifFileType * GFT;                         // the CurrentLocation in Gif file for writing.
}GifInternal;

typedef enum
{
    UNDEFINED_RECORD_TYPE,
    SCREEN_DESC_RECORD_TYPE,
    IMAGE_DESC_RECORD_TYPE,                                /* Begin with ',' */
    EXTENSION_RECORD_TYPE,                                 /* Begin with '!' */
    TERMINATE_RECORD_TYPE                                  /* Begin with ';' */
} GifRecordType;

static GifFileType *DGifOpenFileHandle(GSTREAM *f);
static int DGifGetScreenDesc(GifFileType *GifFile);
static int DGifGetRecordType(GifFileType *GifFile, GifRecordType *GifType);
static int DGifGetImageDesc(GifFileType *GifFile);
static int DGifGetLine(GifFileType *GifFile, unsigned char *GifLine, int GifLineLen);
static int DGifGetPixel(GifFileType *GifFile, unsigned char GifPixel);
static int DGifGetComment(GifFileType *GifFile, char *GifComment);
static int DGifGetExtension(GifFileType *GifFile, int *GifExtCode,
                                                unsigned char **GifExtension);
static int DGifGetExtensionNext(GifFileType *GifFile, unsigned char **GifExtension);
static int DGifGetCode(GifFileType *GifFile, int *GifCodeSize,
                                                unsigned char **GifCodeBlock);
static int DGifGetCodeNext(GifFileType *GifFile, unsigned char **GifCodeBlock);
static int DGifGetLZCodes(GifFileType *GifFile, int *GifCode);
static int DGifCloseFile(GifFileType *GifFile);

GifFileType *EGifOpenFileHandle(GSTREAM *f);
int GifPutNetscapeBlock(GSTREAM *g);
int EGifPutScreenDesc(GifFileType *XGifFile,int Width, int Height, int ColorRes, int BackGround,int BitsPerPixel, GifColorType *ColorMap);
int EGifPutImageDesc(GifFileType *XGifFile,int Left, int Top, int Width, int Height, int Interlace,int BitsPerPixel, GifColorType *ColorMap);
int EGifPutLine(GifFileType *XGifFile, unsigned char *Line, int LineLen);
int EGifPutPixel(GifFileType *XGifFile, unsigned char Pixel);
int EGifPutExtension(GifFileType *XGifFile, int ExtCode, int ExtLen, void *Extension);
int EGifPutComment(GifFileType *XGifFile, char *Comment);
int EGifPutGraphicsControlExtension(GifFileType *XGifFile, int Packed, int DelayTime, int ColorIndex);
int EGifPutCode(GifFileType *XGifFile, int CodeSize, unsigned char *CodeBlock);
int EGifPutCodeNext(GifFileType *XGifFile, unsigned char *CodeBlock);
int EGifCloseFile(GifFileType *XGifFile);


#define D_GIF_ERR_OpenFailed    101              /* And DGif possible errors */
#define D_GIF_ERR_ReadFailed    102
#define D_GIF_ERR_NotGifFile    103
#define D_GIF_ERR_NoScrnDscr    104
#define D_GIF_ERR_NoImagDscr    105
#define D_GIF_ERR_NoColorMap    106
#define D_GIF_ERR_WrongRecord   107
#define D_GIF_ERR_DataTooBig    108
#define D_GIF_ERR_NotEnoughMem  109
#define D_GIF_ERR_CloseFailed   110
#define D_GIF_ERR_NotReadable   111
#define D_GIF_ERR_ImageDefect   112
#define D_GIF_ERR_EOFTooSoon    113


#define GIF_STAMP87 "GIF87a" /* First chars in file - GIF stamp */
#define GIF_STAMP89 "GIF89a" /* First chars in file - GIF stamp */
#define GIF_STAMP_LEN   sizeof(GIF_STAMP87) - 1
#define COMMENT_EXT_FUNC_CODE   'C'   /* Extension function code for comment */
#define GCONTROL_EXT_FUNC_CODE  0xf9  /* Extension function code for graphics control (transparent colour) */

#define LZ_MAX_CODE     4095            /* Biggest code possible in 12 bits. */
#define LZ_BITS         12

#define NO_SUCH_CODE            4098     /* Impossible code, to signal empty */

#define IS_READABLE(Private) 1

typedef struct GifFilePrivateType
{
    int FileState,
        FileHandle,                          /* Where all this data goes to! */
        BitsPerPixel,        /* Bits per pixel (Codes uses at list this + 1) */
        ClearCode,                                      /* The CLEAR LZ code */
        EOFCode,                                          /* The EOF LZ code */
        RunningCode,                 /* The next code algorithm can generate */
        RunningBits, /* The number of bits required to represent RunningCode */
        MaxCode1,/* 1 bigger than maximum possible code, in RunningBits bits */
        LastCode,                        /* The code before the current code */
        CrntCode,                                  /* Current algorithm code */
        StackPtr,                         /* For character stack (see below) */
        CrntShiftState;                  /* Number of bits in CrntShiftDWord */
    unsigned int CrntShiftDWord,      /* For bytes decomposition into codes */
                  PixelCount;                   /* Number of pixels in image */
    GSTREAM *File;                                                 /* File as stream */
    unsigned int CodeMasks[13];
    unsigned char Buf[256];                     /* Compressed input is buffered here */
    unsigned char Stack[LZ_MAX_CODE];     /* Decoded pixels are stacked here */
    unsigned char Suffix[LZ_MAX_CODE+1];                /* So we can trace the codes */
    unsigned int Prefix[LZ_MAX_CODE+1];
} GifFilePrivateType;

static int DGifGetWord(GSTREAM *File, int *Word);
static int DGifSetupDecompress(GifFileType *GifFile);
static int DGifDecompressLine(GifFileType *GifFile, unsigned char *Line, int LineLen);
static int DGifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode);
static int DGifDecompressInput(GifFilePrivateType *Private, int *Code);
static int DGifBufferedInput(GSTREAM *File, unsigned char *Buf, unsigned char *NextByte);


#endif


#define GIFGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define GIFPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

