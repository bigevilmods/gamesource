// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "gifgimex.h"


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

static const int ALPHATHRESHOLD=64;
#define HT_SIZE                 8192       /* 12bits = 4096 or twice as big! */
#define HT_KEY_MASK             0x1FFF                        /* 13bits keys */
#define HT_KEY_NUM_BITS         13                            /* 13bits keys */
#define HT_MAX_KEY              8191    /* 13bits - 1, maximal code possible */
#define HT_MAX_CODE             4095    /* Biggest code possible in 12 bits. */

/* The 32 bits of the int are divided into two parts for the key & code:   */
/* 1. The code is 12 bits as our compression algorithm is limited to 12bits */
/* 2. The key is 12 bits Prefix code + 8 bit new char or 20 bits.           */

#define HT_GET_KEY(l)   (l >> 12)
#define HT_GET_CODE(l)  (l & 0x0FFF)
#define HT_PUT_KEY(l)   (l << 12)
#define HT_PUT_CODE(l)  (l & 0x0FFF)

typedef unsigned int GifHashTableType;

GifHashTableType *_InitHashTable(void);
void _ClearHashTable(GifHashTableType *HashTable);
void _InsertHashTable(GifHashTableType *HashTable, unsigned int Key, int Code);
int _ExistsHashTable(GifHashTableType *HashTable, unsigned int Key);

/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.                                     *
*                                                                            *
* Written by:  Gershon Elber                    IBM PC Ver 0.1, Jun. 1989    *
******************************************************************************
* Module to support the following operations:                                *
*                                                                            *
* 1. InitHashTable - initialize hash table.                                  *
* 2. ClearHashTable - clear the hash table to an empty state.                *
* 2. InsertHashTable - insert one item into data structure.                  *
* 3. ExistsHashTable - test if item exists in data structure.                *
*                                                                            *
* This module is used to hash the GIF codes during encoding.                 *
******************************************************************************
* History:                                                                   *
* 14 Jun 89 - Version 1.0 by Gershon Elber.                                  *
*****************************************************************************/

#define PROGRAM_NAME    "GIF_LIBRARY"
#define VERSION         "á Version 1.0, "

/* #define  DEBUG_HIT_RATE */

/* Debug number of misses per hash Insert/Exists */

#ifdef  DEBUG_HIT_RATE
/*static int NumberOfTests = 0, */
/*          NumberOfMisses = 0; */
#endif

static int KeyItem(unsigned int Item);

/******************************************************************************
* Initialize HashTable - allocate the memory needed and clear it.             *
******************************************************************************/

GifHashTableType *_InitHashTable(void)
{
    GifHashTableType *HashTable;

    if ((HashTable = (GifHashTableType *) galloc(sizeof(unsigned int)*HT_SIZE))
        == NULL)
        return NULL;

    _ClearHashTable(HashTable);

    return HashTable;
}

/******************************************************************************
* Routine to clear the HashTable to an empty state.                           *
* This part is a little machine depended. Use the commented part otherwise.   *
******************************************************************************/
void _ClearHashTable(GifHashTableType *HashTable)
{
    memset(HashTable, 0xFF, HT_SIZE * sizeof(int));
}

/******************************************************************************
* Routine to insert a new Item into the HashTable. The data is assumed to be  *
* new one.                                                                    *
******************************************************************************/
void _InsertHashTable(GifHashTableType *HashTable, unsigned int Key, int Code)
{
    int HKey = KeyItem(Key);
    unsigned int *HTable = HashTable;

#   ifdef DEBUG_HIT_RATE
        NumberOfTests++;
        NumberOfMisses++;
#   endif

    while (HT_GET_KEY(HTable[HKey]) != 0xFFFFFL) {
#       ifdef DEBUG_HIT_RATE
            NumberOfMisses++;
#       endif
        HKey = (HKey + 1) & HT_KEY_MASK;
    }
    HTable[HKey] = HT_PUT_KEY(Key) | HT_PUT_CODE(Code);
}

/******************************************************************************
* Routine to test if given Key exists in HashTable and if so returns its code *
* Returns the Code if key was found, -1 if not.                               *
******************************************************************************/
int _ExistsHashTable(GifHashTableType *HashTable, unsigned int Key)
{
    int HKey = KeyItem(Key);
    unsigned int *HTable = HashTable, HTKey;

#   ifdef DEBUG_HIT_RATE
        NumberOfTests++;
        NumberOfMisses++;
#   endif

    while ((HTKey = HT_GET_KEY(HTable[HKey])) != 0xFFFFFL) {
#       ifdef DEBUG_HIT_RATE
            NumberOfMisses++;
#       endif
        if (Key == HTKey) return HT_GET_CODE(HTable[HKey]);
        HKey = (HKey + 1) & HT_KEY_MASK;
    }

    return -1;
}

/******************************************************************************
* Routine to generate an HKey for the hashtable out of the given unique key.  *
* The given Key is assumed to be 20 bits as follows: lower 8 bits are the     *
* new postfix character, while the upper 12 bits are the prefix code.         *
* Because the average hit ratio is only 2 (2 hash references per entry),      *
* evaluating more complex keys (such as twin prime keys) does not worth it!   *
******************************************************************************/

static int KeyItem(unsigned int Item)
{
    return ((Item >> 12) ^ Item) & HT_KEY_MASK;
}

#ifdef  DEBUG_HIT_RATE
/******************************************************************************
* Debugging routine to print the hit ratio - number of times the hash table   *
* was tested per operation. This routine was used to test the KeyItem routine *
******************************************************************************/
void HashTablePrintHitRatio(void)
{
    printf("Hash Table Hit Ratio is %ld/%ld = %ld%%\n",
        NumberOfMisses, NumberOfTests,
        NumberOfMisses * 100 / NumberOfTests);
}
#endif



/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.                                     *
*                                                                            *
* Written by:  Gershon Elber                    IBM PC Ver 0.1, Jun. 1989    *
******************************************************************************
* The kernel of the GIF Encoding process can be found here.                  *
******************************************************************************
* History:                                                                   *
* 14 Jun 89 - Version 1.0 by Gershon Elber.                                  *
*****************************************************************************/

#define PROGRAM_NAME    "GIF_LIBRARY"
#define VERSION         "á Version 1.0, "

#define GIF_STAMP87     "GIF87a"          /* First chars in file - GIF stamp */
#define ZL_MAX_CODE     4095            /* Biggest code possible in 12 bits. */

#define GSTREAM_STATE_WRITE     0x01/* 1 write, 0 read - DGIF_LIB compatible */
#define GSTREAM_STATE_SCREEN    0x02
#define GSTREAM_STATE_IMAGE     0x04

#define FLUSH_OUTPUT            4096     /* Impossible code, to signal flush */
#define FIRST_CODE              4097     /* Impossible code, to signal first */

#define IS_WRITEABLE(Private)   (Private -> FileState & GSTREAM_STATE_WRITE)

#if 0
#define DEBUG_NO_PREFIX                 /* Dump only compressed data */
#endif

typedef struct XGifFilePrivateType {
    int FileState,
        FileHandle,                          /* Where old this data goes to! */
        BitsPerPixel,        /* Bits per pixel (Codes uses at list this + 1) */
        ClearCode,                                      /* The CLEAR LZ code */
        EOFCode,                                          /* The EOF LZ code */
        RunningCode,                 /* The next code algorithm can generate */
        RunningBits, /* The number of bits required to represent RunningCode */
        MaxCode1,/* 1 bigger than maximum possible code, in RunningBits bits */
        CrntCode,                                  /* Current algorithm code */
        CrntShiftState;                  /* Number of bits in CrntShiftDWord */
    unsigned int CrntShiftDWord;      /* For bytes decomposition into codes */
    int          PixelCount;
    GSTREAM *File;                                                 /* File as stream */
    unsigned char Buf[256];                    /* Compressed output is buffered here */
    GifHashTableType *HashTable;
    unsigned char CodeMask[9];
} XGifFilePrivateType;

/* Masks given codes to BitsPerPixel, to make sure all codes are in range: */

static int EGifPutWord(int Word, GSTREAM *File);
static int EGifSetupCompress(GifFileType *XGifFile);
static int EGifCompressLine(GifFileType *XGifFile, unsigned char *Line, int LineLen);
static int EGifCompressOutput(XGifFilePrivateType *Private, int Code);
static int EGifBufferedOutput(GSTREAM *File, unsigned char *Buf, int c);

/******************************************************************************
*   Update a new gif file, given its file handle, which must be opened for    *
* write in binary mode.                                                       *
*   Returns GifFileType pointer dynamically allocated which serves as the gif *
* info record. _GifError is cleared if succesfull.                            *
******************************************************************************/
GifFileType *EGifOpenFileHandle(GSTREAM *f)
{
    GifFileType *XGifFile;
    XGifFilePrivateType *Private;

    if ((XGifFile = (GifFileType *) galloc(sizeof(GifFileType))) == NULL) {
        return NULL;
    }

    XGifFile -> SWidth = XGifFile -> SHeight =
    XGifFile -> SColorResolution = XGifFile -> SBitsPerPixel =
    XGifFile -> SBackGroundColor =
    XGifFile -> ILeft = XGifFile -> ITop = XGifFile -> IWidth = XGifFile -> IHeight =
    XGifFile -> IInterlace =
    XGifFile -> IBitsPerPixel = 0;

    XGifFile -> SColorMap = XGifFile -> IColorMap = NULL;

#   ifndef DEBUG_NO_PREFIX
    if (!gwrite(f,GIF_STAMP87, strlen(GIF_STAMP87))) {
        gfree((char *) XGifFile);
        return NULL;
    }
#   endif

    if ((Private = (XGifFilePrivateType *) galloc(sizeof(XGifFilePrivateType)))
        == NULL) {
        return NULL;
    }

    XGifFile -> Private = (void *) Private;
    Private -> FileHandle = 0;
    Private -> File = f;
    Private -> FileState = GSTREAM_STATE_WRITE;
    Private -> CodeMask[0] = 0x00;
    Private -> CodeMask[1] = 0x01;
    Private -> CodeMask[2] = 0x03;
    Private -> CodeMask[3] = 0x07;
    Private -> CodeMask[4] = 0x0f;
    Private -> CodeMask[5] = 0x1f;
    Private -> CodeMask[6] = 0x3f;
    Private -> CodeMask[7] = 0x7f;
    Private -> CodeMask[8] = 0xff;

    if ((Private -> HashTable = _InitHashTable()) == NULL) {
        return NULL;
    }

    return XGifFile;
}


/******************************************************************************
*   Puts the Netscape looping block into the file. Without this the animation *
* will only play once.                                                        *
******************************************************************************/
int GifPutNetscapeBlock(GSTREAM *g)
{
    char buf[5];
    int ok;

    gputi(&buf[0], 0x21, 1);    /* Extension Code */
    gputi(&buf[1], 0xFF, 1);    /* Application Extension Block */
    gputi(&buf[2], 0x0B, 1);    /* Length of application block */
    ok = gwrite(g, buf, 3);
    ok = gwrite(g, "NETSCAPE2.0", 11);
    gputi(&buf[0], 0x03, 1);    /* Size of Block */
    gputi(&buf[1], 0x01, 1);    /* Unknown */
    gputi(&buf[2], 0x0000, 2);  /* Number of loops in animation 0=infinite */
    gputi(&buf[4], 0x00, 1);    /* End block */
    ok = gwrite(g, buf, 5);

    return (ok?OK:ERROR);
}


/******************************************************************************
*   This routine should be called before any other EGif calls, immediately    *
* follows the GIF file openning.                                              *
******************************************************************************/
int EGifPutScreenDesc(GifFileType *XGifFile,
        int Width, int Height, int ColorRes, int BackGround,
        int BitsPerPixel, GifColorType *ColorMap)
{
    int i, Size;
    unsigned char Buf[3];
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (Private -> FileState & GSTREAM_STATE_SCREEN) {
        /* If already has screen descriptor - something is wrong! */
        return ERROR;
    }
    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }

    XGifFile -> SWidth = Width;
    XGifFile -> SHeight = Height;
    XGifFile -> SColorResolution = ColorRes;
    XGifFile -> SBitsPerPixel = BitsPerPixel;
    XGifFile -> SBackGroundColor = BackGround;
    if (ColorMap) {
        Size = sizeof(GifColorType) * (1 << BitsPerPixel);
        XGifFile -> SColorMap = (GifColorType *) galloc(Size);
        memcpy(XGifFile -> SColorMap, ColorMap, Size);
    }

    /* Put the screen descriptor into the file: */
    EGifPutWord(Width, Private -> File);
    EGifPutWord(Height, Private -> File);
    Buf[0] = (unsigned char)((ColorMap ? 0x80 : 0x00) |
             ((ColorRes - 1) << 4) |
             (BitsPerPixel - 1));
    Buf[1] = (unsigned char)BackGround;
    Buf[2] = 0;
#   ifndef DEBUG_NO_PREFIX
    gwrite(Private -> File,Buf, 3);
#   endif

    /* If we have Global color map - dump it also: */
#   ifndef DEBUG_NO_PREFIX
    if (ColorMap != NULL)
        for (i=0; i<(1 << BitsPerPixel); i++) {/* Put the ColorMap out also: */
            Buf[0] = ColorMap[i].Red;
            Buf[1] = ColorMap[i].Green;
            Buf[2] = ColorMap[i].Blue;
            if (!gwrite(Private -> File,Buf, 3)) {
                return ERROR;
            }
        }
#   endif

    /* For animation */
    if (!GifPutNetscapeBlock(Private->File))
        return ERROR;

    /* Mark this file as has screen descriptor, and no pixel written yet: */
    Private -> FileState |= GSTREAM_STATE_SCREEN;

    return OK;
}

/******************************************************************************
*   This routine should be called before any attemp to dump an image - any    *
* call to any of the pixel dump routines.                                     *
******************************************************************************/
int EGifPutImageDesc(GifFileType *XGifFile,
        int Left, int Top, int Width, int Height, int Interlace,
        int BitsPerPixel, GifColorType *ColorMap)
{
    int i, Size;
    unsigned char Buf[3];
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (Private -> FileState & GSTREAM_STATE_IMAGE &&
        Private -> PixelCount > 0) {
        /* If already has active image descriptor - something is wrong! */
        return ERROR;
    }
    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }
    XGifFile -> ILeft = gmax(0,Left);
    XGifFile -> ITop = gmax(0,Top);
    XGifFile -> IWidth = Width;
    XGifFile -> IHeight = Height;
    XGifFile -> IBitsPerPixel = BitsPerPixel;
    XGifFile -> IInterlace = Interlace;
    if (ColorMap) {
        Size = sizeof(GifColorType) * (1 << BitsPerPixel);
        if (XGifFile -> IColorMap) gfree((char *) XGifFile -> IColorMap);
        XGifFile -> IColorMap = (GifColorType *) galloc(Size);
        memcpy(XGifFile -> IColorMap, ColorMap, Size);
    }

    /* Put the image descriptor into the file: */
    Buf[0] = ',';                               /* Image seperator character */
#   ifndef DEBUG_NO_PREFIX
    gwrite(Private -> File,Buf, 1);
#   endif
    EGifPutWord(Left, Private -> File);
    EGifPutWord(Top, Private -> File);
    EGifPutWord(Width, Private -> File);
    EGifPutWord(Height, Private -> File);
    Buf[0] = (unsigned char)((ColorMap ? 0x80 : 0x00) |
          (Interlace ? 0x40 : 0x00) |
          (BitsPerPixel - 1));
#   ifndef DEBUG_NO_PREFIX
    gwrite(Private -> File,Buf, 1);
#   endif

    /* If we have Global color map - dump it also: */
#   ifndef DEBUG_NO_PREFIX
    if (ColorMap != NULL)
        for (i=0; i<(1 << BitsPerPixel); i++) {/* Put the ColorMap out also: */
            Buf[0] = ColorMap[i].Red;
            Buf[1] = ColorMap[i].Green;
            Buf[2] = ColorMap[i].Blue;
            if (!gwrite(Private -> File,Buf, 3)) {
                return ERROR;
            }
        }
#   endif
    if (XGifFile -> SColorMap == NULL && XGifFile -> IColorMap == NULL)
    {
        return ERROR;
    }

    /* Mark this file as has screen descriptor: */
    Private -> FileState |= GSTREAM_STATE_IMAGE;
    Private -> PixelCount = (int) Width * (int) Height;

    EGifSetupCompress(XGifFile);       /* Reset compress algorithm parameters */

    return OK;
}

/******************************************************************************
*  Put one full scanned line (Line) of length LineLen into GIF file.          *
******************************************************************************/
int EGifPutLine(GifFileType *XGifFile, unsigned char *Line, int LineLen)
{
    int i;
    unsigned char Mask;
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }

    if (!LineLen) LineLen = XGifFile -> IWidth;
    if ((Private -> PixelCount -= LineLen) < 0) {
        return ERROR;
    }

    /* Make sure the codes are not out of bit range, as we might generate    */
    /* wrong code (because of overflow when we combine them) in this case:   */
    Mask = Private->CodeMask[Private -> BitsPerPixel];
    for (i=0; i<LineLen; i++) Line[i] &= Mask;

    return EGifCompressLine(XGifFile, Line, LineLen);
}

/******************************************************************************
* Put one pixel (Pixel) into GIF file.                                        *
******************************************************************************/
int EGifPutPixel(GifFileType *XGifFile, unsigned char Pixel)
{
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }

    if (--Private -> PixelCount < 0)
    {
        return ERROR;
    }

    /* Make sure the code is not out of bit range, as we might generate      */
    /* wrong code (because of overflow when we combine them) in this case:   */
    Pixel &= Private->CodeMask[Private -> BitsPerPixel];

    return EGifCompressLine(XGifFile, &Pixel, 1);
}

/******************************************************************************
*   Put an extension block (see GIF manual) into gif file.                    *
******************************************************************************/
int EGifPutExtension(GifFileType *XGifFile, int ExtCode, int ExtLen, void *Extension)
{
    unsigned char Buf[3];
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }

    Buf[0] = '!';
    Buf[1] = (unsigned char)(ExtCode);
    Buf[2] = (unsigned char)(ExtLen);
    gwrite(Private -> File,Buf, 3);
    gwrite(Private -> File,Extension, ExtLen);
    Buf[0] = 0;
    gwrite(Private -> File,Buf, 1);

    return OK;
}

/******************************************************************************
* Put a comment into GIF file using extension block.                          *
******************************************************************************/

int EGifPutComment(GifFileType *XGifFile, char *Comment)
{
    return EGifPutExtension(XGifFile, COMMENT_EXT_FUNC_CODE, strlen(Comment),
                                                                Comment);
}

/******************************************************************************
* FPS, transparent index                                                      *
******************************************************************************/

int EGifPutGraphicsControlExtension(GifFileType *XGifFile, int Packed, int DelayTime, int ColorIndex)
{
    char buf[4];

    gputi(buf+0,Packed,1);
    gputi(buf+1,DelayTime,2);
    gputi(buf+3,ColorIndex,1);
    return EGifPutExtension(XGifFile, GCONTROL_EXT_FUNC_CODE, 4, buf);
}


/******************************************************************************
*   Put the image code in compressed form. This routine can be called if the  *
* information needed to be piped out as is. Obviously this is much faster     *
* than decoding and encoding again. This routine should be followed by calls  *
* to EGifPutCodeNext, until NULL block is given.                              *
*   The block should NOT be gfreed by the user (not dynamically allocated).    *
******************************************************************************/
int EGifPutCode(GifFileType *XGifFile, int CodeSize, unsigned char *CodeBlock)
{
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }

    /* No need to dump code size as Compression set up does any for us: */
    /*
    Buf = CodeSize;
    if (!gwrite(Private -> File,&Buf, 1)) {
        return ERROR;
    }
    */

    return EGifPutCodeNext(XGifFile, CodeBlock);
}

/******************************************************************************
*   Continue to put the image code in compressed form. This routine should be *
* called with blocks of code as read via DGifGetCode/DGifGetCodeNext. If      *
* given buffer pointer is NULL, empty block is written to mark end of code.   *
******************************************************************************/
int EGifPutCodeNext(GifFileType *XGifFile, unsigned char *CodeBlock)
{
    unsigned char Buf;
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    if (CodeBlock != NULL) {
        if (!gwrite(Private -> File,CodeBlock, CodeBlock[0] + 1)) {
            return ERROR;
        }
    }
    else {
        Buf = 0;
        if (!gwrite(Private -> File,&Buf, 1)) {
            return ERROR;
        }
        Private -> PixelCount = 0;    /* And local info. indicate image read */
    }

    return OK;
}

/******************************************************************************
*   This routine should be called last, to close GIF file.                    *
******************************************************************************/
int EGifCloseFile(GifFileType *XGifFile)
{
    unsigned char Buf;
    XGifFilePrivateType *Private;
    GSTREAM *File;

    if (XGifFile == NULL) return ERROR;

    Private = (XGifFilePrivateType *) XGifFile -> Private;
    if (!IS_WRITEABLE(Private)) {
        /* This file was NOT open for writing: */
        return ERROR;
    }

    File = Private -> File;

    Buf = ';';
    gwrite(Private -> File,&Buf, 1);

    if (XGifFile -> IColorMap) gfree((char *) XGifFile -> IColorMap);
    if (XGifFile -> SColorMap) gfree((char *) XGifFile -> SColorMap);
    if (Private) {
        if (Private -> HashTable) gfree((char *) Private -> HashTable);
        gfree((char *) Private);
    }
    gfree(XGifFile);

    return OK;
}

/******************************************************************************
*   Put 2 bytes (word) into the given file:                                   *
******************************************************************************/
static int EGifPutWord(int Word, GSTREAM *File)
{
    char c[2];

    c[0] = (char)(Word & 0xff);
    c[1] = (char)((Word >> 8) & 0xff);
#   ifndef DEBUG_NO_PREFIX
    if (gwrite(File,c, 2))
         return OK;
    else return ERROR;
#   else
    return OK;
#   endif
}

/******************************************************************************
*   Update the LSD block with new logical width and height stay at current    *
*   Position in file                                                          *
******************************************************************************/
static int EGifUpdateLSD(int Width, int Height, GSTREAM *File,int CurrentPos, int HeaderPos)
{
    gseek(File,HeaderPos);
    if(EGifPutWord(Width,File) == ERROR)
        return ERROR;
    if(EGifPutWord(Height,File) == ERROR)
        return ERROR;
    gseek(File,CurrentPos);
    return OK;
}



/******************************************************************************
*   Setup the LZ compression for this image:                                  *
******************************************************************************/
static int EGifSetupCompress(GifFileType *XGifFile)
{
    int BitsPerPixel;
    unsigned char Buf;
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    /* Test and see what color map to use, and from it # bits per pixel: */
    if (XGifFile -> IColorMap)
         BitsPerPixel = XGifFile -> IBitsPerPixel;
    else
    if (XGifFile -> SColorMap)
         BitsPerPixel = XGifFile -> SBitsPerPixel;
    else {
        return ERROR;
    }

    Buf = (unsigned char)(BitsPerPixel = (BitsPerPixel < 2 ? 2 : BitsPerPixel));
    gwrite(Private -> File,&Buf, 1);      /* Write the Code size to file */

    Private -> Buf[0] = 0;                         /* Nothing was output yet */
    Private -> BitsPerPixel = BitsPerPixel;
    Private -> ClearCode = (1 << BitsPerPixel);
    Private -> EOFCode = Private -> ClearCode + 1;
    Private -> RunningCode = Private -> EOFCode + 1;
    Private -> RunningBits = BitsPerPixel + 1;    /* Number of bits per code */
    Private -> MaxCode1 = 1 << Private -> RunningBits;      /* Max. code + 1 */
    Private -> CrntCode = FIRST_CODE;      /* Signal that this is first one! */
    Private -> CrntShiftState = 0;       /* No information in CrntShiftDWord */
    Private -> CrntShiftDWord = 0;

    /* Clear hash table and send Clear to make sure the decoder do the same  */
    _ClearHashTable(Private -> HashTable);
    if (EGifCompressOutput(Private, Private -> ClearCode) == ERROR) {
        return ERROR;
    }
    return OK;
}

/******************************************************************************
*   The LZ compression routine:                                               *
*   This version compress the given buffer Line of length LineLen.            *
*   This routine can be called few times (one per scan line, for example), in *
* order the complete the whole image.                                         *
******************************************************************************/
static int EGifCompressLine(GifFileType *XGifFile, unsigned char *Line, int LineLen)
{
    int i = 0, CrntCode, NewCode;
    unsigned int NewKey;
    unsigned char Pixel;
    GifHashTableType *HashTable;
    XGifFilePrivateType *Private = (XGifFilePrivateType *) XGifFile -> Private;

    HashTable = Private -> HashTable;

    if (Private -> CrntCode == FIRST_CODE)                /* Its first time! */
         CrntCode = Line[i++];
    else CrntCode = Private -> CrntCode;     /* Get last code in compression */

    while (i < LineLen) {                           /* Decode LineLen items. */
        Pixel = Line[i++];                     /* Get next pixel from stream */
        /* Form a new unique key to search hash table for the code combines  */
        /* CrntCode as Prefix string with Pixel as postfix char.             */
        NewKey = (((unsigned int) CrntCode) << 8) + Pixel;
        if ((NewCode = _ExistsHashTable(HashTable, NewKey)) >= 0) {
            /* This Key is already there, or the string is old one, so       */
            /* simple take new code as our CrntCode:                         */
            CrntCode = NewCode;
        }
        else {
            /* Put it in hash table, output the prefix code, and make our    */
            /* CrntCode equal to Pixel.                                      */
            if (EGifCompressOutput(Private, CrntCode)
                == ERROR) {
                return ERROR;
            }
            CrntCode = Pixel;

            /* If however the HashTable if full, we send a clear first and   */
            /* Clear the hash table.                                         */
            if (Private -> RunningCode >= ZL_MAX_CODE) {
                /* Time to do some clearance: */
                if (EGifCompressOutput(Private, Private -> ClearCode)
                    == ERROR) {
                    return ERROR;
                }
                Private -> RunningCode = Private -> EOFCode + 1;
                Private -> RunningBits = Private -> BitsPerPixel + 1;
                Private -> MaxCode1 = 1 << Private -> RunningBits;
                _ClearHashTable(HashTable);
            }
            else {
                /* Put this unique key with its relative Code in hash table: */
                _InsertHashTable(HashTable, NewKey, Private -> RunningCode++);
            }
        }
    }

    /* Preserve the current state of the compression algorithm: */
    Private -> CrntCode = CrntCode;

    if (Private -> PixelCount == 0)
    {
        /* We are done - output last Code and flush output buffers: */
        if (EGifCompressOutput(Private, CrntCode)
            == ERROR) {
            return ERROR;
        }
        if (EGifCompressOutput(Private, Private -> EOFCode)
            == ERROR) {
            return ERROR;
        }
        if (EGifCompressOutput(Private, FLUSH_OUTPUT) == ERROR) {
            return ERROR;
        }
    }

    return OK;
}

/******************************************************************************
*   The LZ compression output routine:                                        *
*   This routine is responsable for the compression of the bit stream into    *
* 8 bits (bytes) packets.                                                     *
*   Returns OK if written succesfully.                                        *
******************************************************************************/
static int EGifCompressOutput(XGifFilePrivateType *Private, int Code)
{
    int retval = OK;

    if (Code == FLUSH_OUTPUT) {
        while (Private -> CrntShiftState > 0) {
            /* Get Rid of what is left in DWord, and flush it */
            if (EGifBufferedOutput(Private -> File, Private -> Buf,
                Private -> CrntShiftDWord & 0xff) == ERROR) retval = ERROR;
            Private -> CrntShiftDWord >>= 8;
            Private -> CrntShiftState -= 8;
        }
        Private -> CrntShiftState = 0;                      /* For next time */
        if (EGifBufferedOutput(Private -> File, Private -> Buf,
            FLUSH_OUTPUT) == ERROR) retval = ERROR;
    }
    else {
        Private -> CrntShiftDWord |= ((int) Code) << Private -> CrntShiftState;
        Private -> CrntShiftState += Private -> RunningBits;
        while (Private -> CrntShiftState >= 8) {
            /* Dump out full bytes: */
            if (EGifBufferedOutput(Private -> File, Private -> Buf,
                Private -> CrntShiftDWord & 0xff) == ERROR) retval = ERROR;
            Private -> CrntShiftDWord >>= 8;
            Private -> CrntShiftState -= 8;
        }
    }

    /* If code cannt fit into RunningBits bits, must raise its size. Note */
    /* however that codes above 4095 are used for special signaling.      */
    if (Private -> RunningCode >= Private -> MaxCode1 && Code <= 4095) {
        Private -> MaxCode1 = 1 << ++Private -> RunningBits;
    }

    return retval;
}

/******************************************************************************
*   This routines buffers the given characters until 255 characters are ready *
* to be output. If Code is equal to -1 the buffer is flushed (EOF).           *
*   The buffer is Dumped with first byte as its size, as GIF format requires. *
*   Returns OK if written succesfully.                                        *
******************************************************************************/
static int EGifBufferedOutput(GSTREAM *File, unsigned char *Buf, int c)
{
    if (c == FLUSH_OUTPUT) {
        /* Flush everything out */
        if (Buf[0] != 0 && (!gwrite(File,Buf, Buf[0]+1)))
        {
            return ERROR;
        }
        /* Mark end of compressed data, by an empty block (see GIF doc): */
        Buf[0] = 0;
        if (!gwrite(File,Buf, 1))
        {
            return ERROR;
        }
    }
    else {
        if (Buf[0] == 255) {
            /* Dump out this buffer - it is full: */
            if (!gwrite(File,Buf, Buf[0] + 1))
            {
                return ERROR;
            }
            Buf[0] = 0;
        }
        Buf[++Buf[0]] = (unsigned char)c;
    }

    return OK;
}


/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL GIF_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = -1;
        *gxptr = gx;
        ok = 1;
        // create an internal memory buffer for multiframe managment.
        gx->gref = (void *)galloc(sizeof(GifInternal));
        ((GifInternal *)(gx->gref))->GFT = EGifOpenFileHandle(gx->gstream);  // this writes out the GIF87a header already.

        //((GifFileType*)(gx->gref))->SFilePosition = gtell(gx->gstream);

        // setup the internal buffers to defaults
        ((GifInternal *)(gx->gref))->GFT->SFilePosition  = (unsigned int)gtell(gx->gstream);
        ((GifInternal *)(gx->gref))->HeaderPos           = (int)gtell(gx->gstream);         // location of LSD header.
        ((GifInternal *)(gx->gref))->LogicalScreenWidth  = 0;
        ((GifInternal *)(gx->gref))->LogicalScreenHeight = 0;
    }
    return(ok);
}

int GCALL GIF_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int i;
    int ok=0;
    int x;
    int y;
    int flag;
    int DelayTime;
    int background;
    unsigned char p;
    unsigned char *rowbuf;
    GifFileType *gifinfo;
    GifColorType ColorMap[256];
    GifInternal * InternalStruct= (GifInternal *)gx->gref;

    if (ginfo->bpp<=8)
    {
        //gifinfo = (GifFileType *)gx->gref;
        gifinfo = InternalStruct->GFT;
        if (gifinfo)
        {
            rowbuf = (unsigned char *) galloc(ginfo->width);
            if (rowbuf)
            {
                ok = 1;

                if (gx->framenum == -1)
                {
                    gifinfo->TransparentIndex = -1;
                    gseek(gx->gstream,gifinfo->SFilePosition);

                    memset(ColorMap,0,sizeof(ColorMap));
                    for (i=0; i<ginfo->numcolours; ++i)
                    {
                        ColorMap[i].Red =   ginfo->colourtbl[i].r;
                        ColorMap[i].Green = ginfo->colourtbl[i].g;
                        ColorMap[i].Blue =  ginfo->colourtbl[i].b;
                        if (ginfo->alphabits && (ginfo->colourtbl[i].a<ALPHATHRESHOLD) && gifinfo->TransparentIndex<0)
                            gifinfo->TransparentIndex = i;
                    }
                    background = (gifinfo->TransparentIndex>=0)?gifinfo->TransparentIndex:0;

                    if (EGifPutScreenDesc(gifinfo,ginfo->width,ginfo->height,ginfo->originalbpp,background,ginfo->originalbpp,ColorMap) == ERROR)
                        ok = 0;
                    InternalStruct->LogicalScreenHeight = ginfo->height;
                    InternalStruct->LogicalScreenWidth  = ginfo->width;
                    gifinfo->SFilePosition              = (unsigned int)gtell(gx->gstream);
                }

                /* Seek to end of file */
                gseek(gx->gstream, gifinfo->SFilePosition);
                flag = ((gifinfo->TransparentIndex>=0)?0x01:0x00) | GIF_BACKGROUND;

                DelayTime = 10;
                if (ginfo->fps)
                {
                    DelayTime = gmax(0,gmin(6000,(int)(100.0f/ginfo->fps+0.5f)));
                }

                if (gifinfo->TransparentIndex>=0 || ginfo->fps)
                {
                    if (EGifPutGraphicsControlExtension(gifinfo, flag, DelayTime /* 100/10 fps */, gifinfo->TransparentIndex) == ERROR)
                        ok = 0;
                }

                // update my internal logical screen descriptor block if this image is bigger then thee
                // total screen width and height.
                if(ginfo->width > InternalStruct->LogicalScreenWidth || ginfo->height > InternalStruct->LogicalScreenHeight)
                {
                    // function should seek back to current position so I can continue as before.
                    if(EGifUpdateLSD(ginfo->width,ginfo->height,gx->gstream,(int)gtell(gx->gstream),InternalStruct->HeaderPos) == ERROR)
                        ok = 0;

                    // Update my internal records
                    InternalStruct->LogicalScreenWidth  = ginfo->width;
                    InternalStruct->LogicalScreenHeight = ginfo->height;
                }

                if (EGifPutImageDesc(gifinfo,ginfo->defaultx,ginfo->defaulty,
                    ginfo->width,ginfo->height,0,ginfo->originalbpp,0) == ERROR)
                    ok = 0;

                for (y=0; y<ginfo->height && ok; ++y)
                {
                    for (x=0; x<ginfo->width; ++x)
                    {
                        p = (unsigned char) source[x];

                        rowbuf[x] = (unsigned char)p;

                        if (ginfo->colourtbl[(int)p].a<ALPHATHRESHOLD && gifinfo->TransparentIndex>=0)
                            rowbuf[x] = (unsigned char)(gifinfo->TransparentIndex);
                    }

                    if (EGifPutLine(gifinfo, rowbuf, ginfo->width) == ERROR)
                        ok = 0;
                    source += rowbytes;
                }
                gfree(rowbuf);
                gifinfo->SFilePosition = (unsigned int)gtell(gx->gstream);
                gx->framenum++;
            }
        }
    }
    return(ok);
}

int GCALL GIF_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if (gx->gref)  // if internal structure exists.
        {
            // clean up any gif file handle infor.
            if (EGifCloseFile(((GifInternal *)gx->gref)->GFT) == ERROR)
                ok = 0;
            // free the internal structure.
            gfree(gx->gref);
        }
        ok = gfree((void *) gx);
    }
    return(ok);
}



