// (c) 1994-2007 Electronic Arts Inc.

#ifndef __AVIGIMEX
#define __AVIGIMEX 1

#include "gimex.h"


#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  AVI Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL AVI_about(void);
int      GCALL AVI_is(GSTREAM *g);

/* Import Functions */

int      GCALL AVI_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL AVI_info(GINSTANCE *gx, int framenum);
int      GCALL AVI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL AVI_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL AVI_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL AVI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL AVI_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct
{
    /* AVIH AVI Header structure */
    int TimeBetweenFrames;
    int MaxBps;
    int PaddingGranularity;
    int Flags;
    int TotalFrames;
    int InitFrames;
    int Streams;
    int SuggestedBufferSize;
    int Width;
    int Height;
    int TimeScale;
    int DataRate;
    int StartTime;
    unsigned int DataLength;

} AVIAVIHStruct;

// http://dpi.ufv.br/downloads/C++IntroductoryEdition/VCB600ENU1/VC98/INCLUDE/AVIRIFF.H
typedef struct
{
    /* DMLH AVI Header structure */
    int GrandFrames;
    char Future[248-4];

} AVIDMLHStruct;

/* AVIAVIHStruct Flags */
        /* Has an index Chunk */
#define AVIHASINDEX            0x00000010
        /* Must use index Chunk */
#define AVIUSEINDEX            0x00000020
        /* File is interleaved */
#define AVIISINTERLEAVED       0x00000100
        /* Optimized for live video capture */
#define AVIWASCAPTUREFILE      0x00010000
        /* contains copyrighted data */
#define AVICOPYRIGHTED         0x00020000
        /* Use CKType to find key frames? ie, the 2CC code 'db' in '00db'*/
#define AVITRUSTCKTYPE         0x00000800
// subtype for the codec configure box
#define SUBCONFIGURE  0x8000000


typedef struct
{
    /* STRH Stream header chunk */
    int DataType;       /* vids */
    int DataHandler;    /* msvc, rle */
    int Flags;
    int Priority;
    int InitialFrames;
    int TimeScale;
    int DataRate;
    int StartTime;
    unsigned int DataLength;
    int SuggestedBufferSize;
    int Quality;
    int SampleSize;

} AVISTRHStruct;


/*
 * Used in AVIVIDSStruct.Compression
 * Also used for AVIINFO.Compression
 */
#define AVI_BI_RGB  0x00000000
#define AVI_RGB      0x20424752
#define AVI_BI_RLE8 0x00000001
#define AVI_RLE8     0x38454c52

/* Windows types */
typedef unsigned short AVIWORD;
typedef unsigned int  AVIDWORD;
typedef int           AVILONG;
typedef unsigned char  AVIBYTE;

typedef struct /* BITMAPINFOHEADER */
{
    AVIDWORD Size;
    AVILONG Width;
    AVILONG Height;
    AVIWORD Planes;
    AVIWORD BitCount;
    AVIDWORD Compression;
    AVIDWORD ImageSize;
    AVILONG XpelsMeter;
    AVILONG YpelsMeter;
    AVIDWORD NumberOfColours;
    AVIDWORD ImportantColours;

} AVIBMIStruct;

typedef struct /* RGBQUAD */
{
    AVIBYTE    rgbBlue;
    AVIBYTE    rgbGreen;
    AVIBYTE    rgbRed;
    AVIBYTE    rgbReserved;
} AVIRGBStruct;

typedef struct /* BITMAPINFO */
{
    AVIBMIStruct*    bmiHeader;
    AVIRGBStruct     bmiColours[256];
} AVIVIDSStruct;


typedef struct
{
    int Identifier;
    int Flags;
    GPOS Offset;
    unsigned int Length;
} AVIINDXStruct;

/*
 * AVIINDXStruct Flag values
 * From lpdwFlags parameter of ICCompress in Video For Windows Lib
 */
#define AVI_KEYFRAME 0x00000010L
#define AVI_TWOCC    0x00000002L

/* See GFF 554 */

struct AVIVfwCodecInfo
{
    int width;
    int height;
    int srcBpp;
    int fourCC;
    int nColours;       // if palette atached (bpp < 8 only)
    int destBpp;
    int quality;
    int framesize;
    int flags;
    int frameFlags;
    void * extraBihInfo;     // pointer to extra data written into the BITMAPINFOHEADER section of file
    int sizeExtraBihInfo;
    ARGB* palette;
    unsigned char* lastFrame;
};


typedef struct __AVIINFO
{
    int   Width;
    int   Height;
    int   Bpp;
    int   AlphaMask;
    float Fps;
    GPOS  MaxFrameSize;
    int   Compression;
    int   Speed;                /* 0=normal, 1=faster */
    GPOS  DataPosition;
    GPOS  CurrentPosition;      /* Position in the file of last read/write */
    int   Flags;                /* AVI header flags */
    void *CurrentFrame;         /* The last read frame */
    int   Framenum;
    int   Quality;
    int   FrameSize;
    unsigned int   FrameFlag;   /* Frames Index flag */
    void *hic;                  /* For windows only decompressors */
    AVIVfwCodecInfo format;     /* For windows only decompressors */
    AVIVIDSStruct *BitmapHeader;
    AVIINDXStruct *ChunkOffsetTbl;
    ARGB  ColourTable[256];
    int  (*Decoder)(const void *source, void *dest, unsigned int size, struct __AVIINFO *avi, int rowbutes);
    int  (*Encoder)(GINSTANCE *gx, const void *source, int rowbutes);
    int  Configure : 1;
} AVIINFO;


#define AVIGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define AVIPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

