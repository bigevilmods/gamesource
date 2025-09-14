// Copyright (C) Electronic Arts Canada Inc. 1994-2002.  All rights reserved.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   GIF - Compuserve GIF Gimex                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*               based on GIF lib by Gershon Elber                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.90 890614 GE Gif lib for PC                                 */
/*    1.00 940923 FB Gimex based on GIF Lib, plugin, and TGA Gimex  */
/*    1.01 940924 FB gifwrite                                       */
/*    1.02 941001 FB SGI tweeks for casting, slashes etc            */
/*    1.03 950529 FB ginstance signature                            */
/*    1.04 950612 FB export transparent colour extension            */
/*    1.05 951025 FB import interlaced gif                          */
/*    1.06 951102 FB export allow multiple transparent colours      */
/*    1.07 951112 FB import transparency                            */
/*    1.08 960513 FB opengif allow gif89a                           */
/*    1.09 970424 FB set alpha bits and transparent on import       */
/*    1.10 970529 KO Multiframe support. Fixed offsets+Transparency */
/*    1.11 970604 KO Removed warnings.                              */
/*    1.12 970820 KO Stop decoding at EOL code (No Error)           */
/*    1.13 980519 KO Loop clearing image to respect neg. rowbytes   */
/*    1.14 980728 FB fps export, sign extend and clamp top          */
/*    1.15 990804 FB fps import                                     */
/*    1.16 991222 FB opaque gifs                                    */
/*    1.17 000330 FB about maxalphabits 1                           */
/*    1.18 000404 FB about->movie                                   */
/*    1.19 000718 FB output transparent index only if >=0           */
/*    1.20 020215 FB -fps forces a transparent index                */
/*    1.21 070207 FB unknown disposal treated as no disposal        */
/*    1.22 070228 FB multilayer support import as 32 bit            */
/*    1.23 070314 FB interlace support                              */
/*    1.24 070716 FC palette null fix (for 1.23)                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit import                                                     */
/* decompresses                                                     */
/* Reentrant                                                        */
/* Files: gifread.cpp gifwrite.cpp gifgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* GIF files support 8 bit only                                     */
/* LZW compression on rows                                          */
/* Intended for serial communications                               */
/*                                                                  */
/* Follows some Netscapeisms in order to use to and from web sites  */
/*  - Output now contains a Netscape application block to declare   */
/*    the frame rate for playback (currently set to 10fps)          */
/*  - If transparency exists and clear flag is set to GIF_BACKGROUND*/
/*    then rather than clearing to the background colour it clears  */
/*    to the transparent colour.                                    */
/*  - Also, only clears the area used by that image not necessarily */
/*    the whole framebuffer.                                        */
/*  - Transparent values are not written to the buffer.             */
/*  - Image buffer is cleared to the background colour (Transparent */
/*    if a transparent colours exists) before any images are loaded.*/
/*                                                                  */
/* Some notes on GIF patent                                         */
/* http://www.mindworkshop.com/alchemy/lzw.html                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <stdio.h>

#include <string.h>
#include "gifgimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

/******************************************************************************
*   Update a new gif file, given its file handle.                             *
*   Returns GifFileType pointer dynamically allocated which serves as the gif *
* info record.                                                                *
******************************************************************************/

static GifFileType *DGifOpenFileHandle(GSTREAM *f)
{
    char Buf[GIF_STAMP_LEN+1];
    GifFileType *GifFile;
    GifFilePrivateType *Private;

    if ((GifFile = (GifFileType *) galloc(sizeof(GifFileType))) == NULL)
    {
        return NULL;
    }

    if ((Private = (GifFilePrivateType *) galloc(sizeof(GifFilePrivateType))) == NULL)
    {
        gfree((char *) GifFile);
        return NULL;
    }
    GifFile -> Private = (void *) Private;
    GifFile -> SColorMap = GifFile -> IColorMap = NULL;
    Private -> FileHandle = 0;/*FileHandle; */
    Private -> File = f;
    Private -> FileState = 0; /* Make sure bit 0 = 0 (File opened for read) */
    Private -> CodeMasks[0]  = 0x0000;
    Private -> CodeMasks[1]  = 0x0001;
    Private -> CodeMasks[2]  = 0x0003;
    Private -> CodeMasks[3]  = 0x0007;
    Private -> CodeMasks[4]  = 0x000f;
    Private -> CodeMasks[5]  = 0x001f;
    Private -> CodeMasks[6]  = 0x003f;
    Private -> CodeMasks[7]  = 0x007f;
    Private -> CodeMasks[8]  = 0x00ff;
    Private -> CodeMasks[9]  = 0x01ff;
    Private -> CodeMasks[10] = 0x03ff;
    Private -> CodeMasks[11] = 0x07ff;
    Private -> CodeMasks[12] = 0x0fff;

    /* Lets see if this is GIF file: */
    if (!gread(Private -> File,Buf, GIF_STAMP_LEN))
    {
        gfree((char *) Private);
        gfree((char *) GifFile);
        return NULL;
    }

    Buf[GIF_STAMP_LEN] = 0;
    if (!(!strcmp(GIF_STAMP87, Buf) || !strcmp(GIF_STAMP89, Buf)))
    {
        gfree((char *) Private);
        gfree((char *) GifFile);
        return NULL;
    }

    if (DGifGetScreenDesc(GifFile) == ERROR)
    {
        gfree((char *) Private);
        gfree((char *) GifFile);
        return NULL;
    }


    return GifFile;
}

/******************************************************************************
*   This routine should be called before any other DGif calls. Note that      *
* this routine is called automatically from DGif file open routines.          *
******************************************************************************/

static int DGifGetScreenDesc(GifFileType *GifFile)
{
    int Size, i;
    unsigned char Buf[3];
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    /* Put the screen descriptor into the file: */
    if (DGifGetWord(Private -> File, &GifFile -> SWidth) == ERROR ||
    DGifGetWord(Private -> File, &GifFile -> SHeight) == ERROR)
    return ERROR;

    if (!gread(Private -> File,Buf, 3))
    {
        return ERROR;
    }

    GifFile -> SColorResolution = (((Buf[0] & 0x70) + 1) >> 4) + 1;
    GifFile -> SBitsPerPixel = (Buf[0] & 0x07) + 1;
    GifFile -> SBackGroundColor = Buf[1];

    if (Buf[0] & 0x80)
    {
        /* Do we have global color map? */

        Size = (1 << GifFile -> SBitsPerPixel);

        GifFile -> SColorMap = (GifColorType *) galloc(sizeof(GifColorType) * Size);

        for (i=0; i<Size; i++)
        {
            /* Get the global color map: */
            if (!gread(Private -> File,Buf, 3))
            {
                return ERROR;
            }
            GifFile -> SColorMap[i].Red = Buf[0];
            GifFile -> SColorMap[i].Green = Buf[1];
            GifFile -> SColorMap[i].Blue = Buf[2];
        }
    }

    return OK;
}

/******************************************************************************
*   This routine should be called before any attemp to read an image.         *
******************************************************************************/

static int DGifGetRecordType(GifFileType *GifFile, GifRecordType *Type)
{
    unsigned char Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (!gread(Private -> File,&Buf, 1))
    {
        return ERROR;
    }

    switch (Buf)
    {
        case ',': *Type = IMAGE_DESC_RECORD_TYPE; break;
        case '!': *Type = EXTENSION_RECORD_TYPE;  break;
        case ';': *Type = TERMINATE_RECORD_TYPE;  break;
        default:  *Type = UNDEFINED_RECORD_TYPE;  return(ERROR);
    }

    return(OK);
}

/******************************************************************************
*   This routine should be called before any attemp to read an image.         *
*   Note it is assumed the Image desc. header (',') has been read.        *
******************************************************************************/

static int DGifGetImageDesc(GifFileType *GifFile)
{
    int Size, i;
    unsigned char Buf[3];
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (DGifGetWord(Private -> File, &GifFile -> ILeft) == ERROR ||
    DGifGetWord(Private -> File, &GifFile -> ITop) == ERROR ||
    DGifGetWord(Private -> File, &GifFile -> IWidth) == ERROR ||
    DGifGetWord(Private -> File, &GifFile -> IHeight) == ERROR)
    return ERROR;
    if (!gread(Private -> File,Buf, 1))
    {
        return ERROR;
    }
    GifFile -> IBitsPerPixel = (Buf[0] & 0x07) + 1;
    GifFile -> IInterlace = (Buf[0] & 0x40);
    if (Buf[0] & 0x80)
    {
        /* Does this image have local color map? */
        Size = (1 << GifFile -> IBitsPerPixel);
        if (GifFile -> IColorMap)
            gfree((char *) GifFile -> IColorMap);

        GifFile -> IColorMap = (GifColorType *) galloc(sizeof(GifColorType) * Size);

        for (i=0; i<Size; i++)
        {
            /* Get the image local color map: */

            if (!gread(Private -> File,Buf, 3))
            {
                return ERROR;
            }
            GifFile -> IColorMap[i].Red = Buf[0];
            GifFile -> IColorMap[i].Green = Buf[1];
            GifFile -> IColorMap[i].Blue = Buf[2];
        }
    }

    Private -> PixelCount = (int) GifFile -> IWidth *
                (int) GifFile -> IHeight;

    DGifSetupDecompress(GifFile);   /* Reset decompress algorithm parameters */

    return OK;
}

/******************************************************************************
*  Get one full scanned line (Line) of length LineLen from GIF file.          *
******************************************************************************/

static int DGifGetLine(GifFileType *GifFile, unsigned char *Line, int LineLen)
{
    unsigned char *Dummy;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (!LineLen) LineLen = GifFile -> IWidth;

/* Not possible right now (PixelCount = LineLen * GifFile->Height) */
#if 0

    if ((Private -> PixelCount -= LineLen) < 0)
    {
        return ERROR;
    }
#else
    Private -> PixelCount -= LineLen;
#endif

    if (DGifDecompressLine(GifFile, Line, LineLen) == OK)
    {
        if (Private -> PixelCount == 0)
        {
            /* We probably would not be called any more, so lets clean       */
            /* everything before we return: need to flush out all rest of    */
            /* image until empty block (size 0) detected. We use GetCodeNext */

            do
            {
                if (DGifGetCodeNext(GifFile, &Dummy) == ERROR)
                    return ERROR;
            }
            while (Dummy != NULL);
        }
        return OK;
    }
    else
        return ERROR;
}

/******************************************************************************
* Put one pixel (Pixel) into GIF file.                        *
******************************************************************************/

#if 0

static int DGifGetPixel(GifFileType *GifFile, unsigned char Pixel)
{
    unsigned char *Dummy;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (--Private -> PixelCount < 0)
    {
    return ERROR;
    }

    if (DGifDecompressLine(GifFile, &Pixel, 1) == OK)
    {
        if (Private -> PixelCount == 0) {
            /* We probably would not be called any more, so lets clean       */
            /* everything before we return: need to flush out all rest of    */
            /* image until empty block (size 0) detected. We use GetCodeNext */
            do if (DGifGetCodeNext(GifFile, &Dummy) == ERROR) return ERROR;
            while (Dummy != NULL);
        }
        return OK;
    }
    else return ERROR;
}

#endif

/******************************************************************************
*   Get an extension block (see GIF manual) from gif file. This routine only  *
* returns the first data block, and DGifGetExtensionNext shouldbe called      *
* after this one until NULL extension is returned.                *
*   The Extension should NOT be gfreed by the user (not dynamically allocated).*
*   Note it is assumed the Extension desc. header ('!') has been read.        *
******************************************************************************/

static int DGifGetExtension(GifFileType *GifFile, int *ExtCode, unsigned char **Extension)
{
    unsigned char Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (!gread(Private -> File,&Buf, 1))
    {
        return ERROR;
    }
    *ExtCode = Buf;

    return DGifGetExtensionNext(GifFile, Extension);
}

/******************************************************************************
*   Get a following extension block (see GIF manual) from gif file. This      *
* routine sould be called until NULL Extension is returned.           *
*   The Extension should NOT be gfreed by the user (not dynamically allocated).*
******************************************************************************/

static int DGifGetExtensionNext(GifFileType *GifFile, unsigned char **Extension)
{
    unsigned char Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (!gread( Private -> File,&Buf, 1))
    {
        return ERROR;
    }
    if (Buf > 0)
    {
        *Extension = Private -> Buf;            /* Use private unused buffer */
        (*Extension)[0] = Buf; /* Pascal strings notation (pos. 0 is length) */
        if (!gread( Private -> File,&((*Extension)[1]), Buf))
        {
            return ERROR;
        }
    }
    else *Extension = NULL;

    return OK;
}

/******************************************************************************
*   This routine should be called last, to close GIF file.            *
******************************************************************************/

static int DGifCloseFile(GifFileType *GifFile)
{
    GifFilePrivateType *Private;
    GSTREAM *File;

    if (GifFile == NULL) return ERROR;

    Private = (GifFilePrivateType *) GifFile -> Private;

    File = Private -> File;

    if (GifFile -> IColorMap) gfree((char *) GifFile -> IColorMap);
    if (GifFile -> SColorMap) gfree((char *) GifFile -> SColorMap);
    if (Private) gfree((char *) Private);
    gfree(GifFile);

    return OK;
}

/******************************************************************************
*   Get 2 bytes (word) from the given file:                   *
******************************************************************************/

#define SIGNEXTEND(v,bits) (((int)(v)<<(sizeof(int)*8-(bits)))>>(sizeof(int)*8-(bits)))

static int DGifGetWord(GSTREAM *File, int *Word)
{
    unsigned char c[2];

    if (!gread( File,c, 2))
    {
        return ERROR;
    }

    *Word = SIGNEXTEND(ggeti(c,2),16);
    return OK;
}

#undef SIGNEXTEND

/******************************************************************************
*   Get the image code in compressed form. his routine can be called if the   *
* information needed to be piped out as is. Obviously this is much faster     *
* than decoding and encoding again. This routine should be followed by calls  *
* to DGifGetCodeNext, until NULL block is returned.               *
*   The block should NOT be gfreed by the user (not dynamically allocated).    *
******************************************************************************/

#if 0

static int DGifGetCode(GifFileType *GifFile, int *CodeSize, unsigned char **CodeBlock)
{
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    *CodeSize = Private -> BitsPerPixel;

    return DGifGetCodeNext(GifFile, CodeBlock);
}

#endif

/******************************************************************************
*   Continue to get the image code in compressed form. This routine should be *
* called until NULL block is returned.                        *
*   The block should NOT be gfreed by the user (not dynamically allocated).    *
******************************************************************************/

static int DGifGetCodeNext(GifFileType *GifFile, unsigned char **CodeBlock)
{
    unsigned char Buf;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (!gread( Private -> File,&Buf, 1))
    {
        return ERROR;
    }

    if (Buf > 0)
    {
        *CodeBlock = Private -> Buf;        /* Use private unused buffer */
        (*CodeBlock)[0] = Buf; /* Pascal strings notation (pos. 0 is length) */
        if (!gread( Private -> File,&((*CodeBlock)[1]), Buf))
        {
            return ERROR;
        }
    }
    else
    {
        *CodeBlock = NULL;
        Private -> Buf[0] = 0;         /* Make sure the buffer is empty! */
        Private -> PixelCount = 0;    /* And local info. indicate image read */
    }

    return OK;
}

/******************************************************************************
*   Setup the LZ decompression for this image:                    *
******************************************************************************/

static int DGifSetupDecompress(GifFileType *GifFile)
{
    int i, BitsPerPixel;
    unsigned char CodeSize;
    unsigned int *Prefix;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    gread( Private -> File,&CodeSize, 1);
    BitsPerPixel = CodeSize;

    Private -> Buf[0] = 0;                 /* Input Buffer empty */
    Private -> BitsPerPixel = BitsPerPixel;
    Private -> ClearCode = (1 << BitsPerPixel);
    Private -> EOFCode = Private -> ClearCode + 1;
    Private -> RunningCode = Private -> EOFCode + 1;
    Private -> RunningBits = BitsPerPixel + 1;    /* Number of bits per code */
    Private -> MaxCode1 = 1 << Private -> RunningBits;      /* Max. code + 1 */
    Private -> StackPtr = 0;             /* No pixels on the pixel stack */
    Private -> LastCode = NO_SUCH_CODE;
    Private -> CrntShiftState = 0;   /* No information in CrntShiftDWord */
    Private -> CrntShiftDWord = 0;

    Prefix = Private -> Prefix;
    for (i=0; i<LZ_MAX_CODE; i++)
        Prefix[i] = NO_SUCH_CODE;

    return OK;
}


/******************************************************************************
*   The LZ decompression routine:                         *
*   This version decompress the given gif file into Line of length LineLen.   *
*   This routine can be called few times (one per scan line, for example), in *
* order the complete the whole image.                         *
******************************************************************************/

static int DGifDecompressLine(GifFileType *GifFile, unsigned char *Line, int LineLen)
{
    int i = 0, j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr;
    unsigned char *Stack, *Suffix;
    unsigned int *Prefix;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    StackPtr = Private -> StackPtr;
    Prefix = Private -> Prefix;
    Suffix = Private -> Suffix;
    Stack = Private -> Stack;
    EOFCode = Private -> EOFCode;
    ClearCode = Private -> ClearCode;
    LastCode = Private -> LastCode;

    if (StackPtr != 0)
    {
        /* Let pop the stack off before continueing to read the gif file: */
        while (StackPtr != 0 && i < LineLen)
            /* Only if not transparent */
            if (Stack[--StackPtr] != GifFile->TransparentIndex)
                Line[i++] = Stack[StackPtr];
            else
                i++;
    }

    while (i < LineLen)
    {
        /* Decode LineLen items. */

        if (DGifDecompressInput(Private, &CrntCode) == ERROR)
            return ERROR;

        if (CrntCode == EOFCode)
        {
            /* Note however that usually we will not be here as we will stop */
            /* decoding as soon as we got all the pixel, or EOF code will    */
            /* not be read at all, and DGifGetLine/Pixel clean everything.   */
            if (i != LineLen - 1 || Private -> PixelCount != 0)
            {
                /* return ERROR; */
                /* Rather than return an error let's just stop decoding */
                i = LineLen - 1;
            }
            i++;
        }
        else
        {
            if (CrntCode == ClearCode)
            {
                /* We need to start over again: */
                for (j=0; j<LZ_MAX_CODE; j++)
                    Prefix[j] = NO_SUCH_CODE;
                Private -> RunningCode = Private -> EOFCode + 1;
                Private -> RunningBits = Private -> BitsPerPixel + 1;
                Private -> MaxCode1 = 1 << Private -> RunningBits;
                LastCode = Private -> LastCode = NO_SUCH_CODE;
            }
            else
            {
                /* Its regular code - if in pixel range simply add it to output  */
                /* stream, otherwise trace to codes linked list until the prefix */
                /* is in pixel range:                        */

                if (CrntCode < ClearCode)
                {
                    /* This is simple - its pixel scalar, so add it to output:   */
                    /* Only if it not transparent */
                    if (CrntCode != GifFile->TransparentIndex)
                        Line[i++] = CrntCode;
                    else
                        i++;
                }
                else
                {
                    /* Its a code to needed to be traced: trace the linked list  */
                    /* until the prefix is a pixel, while pushing the suffix     */
                    /* pixels on our stack. If we done, pop the stack in reverse */
                    /* (thats what stack is good for!) order to output.      */
                    if (Prefix[CrntCode] == NO_SUCH_CODE)
                    {
                        /* Only allowed if CrntCode is exactly the running code: */
                        /* In that case CrntCode = XXXCode, CrntCode or the      */
                        /* prefix code is last code and the suffix char is       */
                        /* exactly the prefix of last code!              */

                        if (CrntCode == Private -> RunningCode - 2)
                        {
                            CrntPrefix = LastCode;
                            Suffix[Private -> RunningCode - 2] =
                                Stack[StackPtr++] = DGifGetPrefixChar(Prefix,
                                    LastCode, ClearCode);
                        }
                        else
                        {
                            return ERROR;
                        }
                    }
                    else
                        CrntPrefix = CrntCode;

                    /* Now (if image is O.K.) we should not get and NO_SUCH_CODE */
                    /* During the trace. As we might loop forever, in case of    */
                    /* defective image, we count the number of loops we trace    */
                    /* and stop if we got LZ_MAX_CODE. obviously we can not      */
                    /* loop more than that.                      */

                    j = 0;
                    while (j++ <= LZ_MAX_CODE &&
                       CrntPrefix > ClearCode &&
                       CrntPrefix <= LZ_MAX_CODE)
                    {
                        Stack[StackPtr++] = Suffix[CrntPrefix];
                        CrntPrefix = Prefix[CrntPrefix];
                    }

                    if (j >= LZ_MAX_CODE || CrntPrefix > LZ_MAX_CODE)
                    {
                        return ERROR;
                    }

                    /* Push the last character on stack: */
                    Stack[StackPtr++] = CrntPrefix;

                    /* Now lets pop all the stack into output: */
                    while (StackPtr != 0 && i < LineLen)
                        /* Only if not transparent */
                        if (Stack[--StackPtr] != GifFile->TransparentIndex)
                            Line[i++] = Stack[StackPtr];
                        else
                            i++;
                }

                if (LastCode != NO_SUCH_CODE)
                {
                    Prefix[Private -> RunningCode - 2] = LastCode;

                    if (CrntCode == Private -> RunningCode - 2)
                    {

                        /* Only allowed if CrntCode is exactly the running code: */
                        /* In that case CrntCode = XXXCode, CrntCode or the      */
                        /* prefix code is last code and the suffix char is       */
                        /* exactly the prefix of last code!              */

                        Suffix[Private -> RunningCode - 2] =
                            DGifGetPrefixChar(Prefix, LastCode, ClearCode);
                    }
                    else
                    {
                        Suffix[Private -> RunningCode - 2] =
                        DGifGetPrefixChar(Prefix, CrntCode, ClearCode);
                    }
                }

                LastCode = CrntCode;
            }
        }
    }

    Private -> LastCode = LastCode;
    Private -> StackPtr = StackPtr;

    return OK;
}

/******************************************************************************
* Routine to trace the Prefixes linked list until we get a prefix which is    *
* not code, but a pixel value (less than ClearCode). Returns that pixel value.*
* If image is defective, we might loop here forever, so we limit the loops to *
* the maximum possible if image O.k. - LZ_MAX_CODE times.             *
******************************************************************************/

static int DGifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode)
{
    int i = 0;

    while (Code > ClearCode && i++ <= LZ_MAX_CODE) Code = Prefix[Code];
    return Code;
}

/******************************************************************************
*   Interface for accessing the LZ codes directly. Set Code to the real code  *
* (12bits), or to -1 if EOF code is returned.                     *
******************************************************************************/

#if 0

static int DGifGetLZCodes(GifFileType *GifFile, int *Code)
{
    unsigned char *CodeBlock;
    GifFilePrivateType *Private = (GifFilePrivateType *) GifFile -> Private;

    if (DGifDecompressInput(Private, Code) == ERROR) return ERROR;

    if (*Code == Private -> EOFCode) {
    /* Skip rest of codes (hopefully only NULL terminating block): */
    do if (DGifGetCodeNext(GifFile, &CodeBlock) == ERROR) return ERROR;
    while (CodeBlock != NULL);

    *Code = -1;
    }
    else
    if (*Code == Private -> ClearCode) {
    /* We need to start over again: */
    Private -> RunningCode = Private -> EOFCode + 1;
    Private -> RunningBits = Private -> BitsPerPixel + 1;
    Private -> MaxCode1 = 1 << Private -> RunningBits;
    }

    return OK;
}

#endif

/******************************************************************************
*   The LZ decompression input routine:                       *
*   This routine is responsable for the decompression of the bit stream from  *
* 8 bits (bytes) packets, into the real codes.                    *
*   Returns OK if read succesfully.                       *
******************************************************************************/

static int DGifDecompressInput(GifFilePrivateType *Private, int *Code)
{
    unsigned char NextByte;

    while (Private -> CrntShiftState < Private -> RunningBits)
    {
        /* Needs to get more bytes from input stream for next code: */

        if (DGifBufferedInput(Private -> File, Private -> Buf, &NextByte) == ERROR)
        {
            return ERROR;
        }

        Private -> CrntShiftDWord |=
            ((unsigned int) NextByte) << Private -> CrntShiftState;
        Private -> CrntShiftState += 8;
    }

    *Code = Private -> CrntShiftDWord & Private->CodeMasks[Private -> RunningBits];

    Private -> CrntShiftDWord >>= Private -> RunningBits;
    Private -> CrntShiftState -= Private -> RunningBits;

    /* If code cannt fit into RunningBits bits, must raise its size. Note */
    /* however that codes above 4095 are used for special signaling.      */
    if (++Private -> RunningCode > Private -> MaxCode1 &&
    Private -> RunningBits < LZ_BITS)
    {
        Private -> MaxCode1 <<= 1;
        Private -> RunningBits++;
    }
    return OK;
}

/******************************************************************************
*   This routines read one gif data block at a time and buffers it internally *
* so that the decompression routine could access it.                  *
*   The routine returns the next byte from its internal buffer (or read next  *
* block in if buffer empty) and returns OK if succesful.              *
******************************************************************************/

static int DGifBufferedInput(GSTREAM *File, unsigned char *Buf, unsigned char *NextByte)
{
    if (Buf[0] == 0)
    {
        /* Needs to read the next buffer - this one is empty: */

        if (!gread( File,Buf, 1))
        {
            return ERROR;
        }
        if (!gread( File,&Buf[1], Buf[0]))
        {
            return ERROR;
        }
        *NextByte = Buf[1];
        Buf[1] = 2;    /* We use now the second place as last char read! */
        Buf[0]--;
    }
    else
    {
        *NextByte = Buf[Buf[1]++];
        Buf[0]--;
    }

    return OK;
}

static int LoadImage(GifFileType *GifFile, char *dest,int rowbytes)
{
    int i, j;
    int width;
    int height;
    int row;
    int col;
    unsigned char *d;

    int lace=8;
    int startingRow=0;

    row = GifFile->ITop;
    col = GifFile->ILeft;
    width  = GifFile->IWidth;
    height = GifFile->IHeight;

    // left clip
    if (col<0)
    {
        width -= (-col);
        col = 0;
    }
    // right clip
    if (width+col>GifFile->SWidth)
    {
        width -= ((width+col)-GifFile->SWidth);
    }
    // top clip
    if (row<0)
    {
        height -= (-row);
        row = 0;
    }
    // bottom clip
    if (height+row>GifFile->SHeight)
    {
        height -= ((height+row)-GifFile->SHeight);
    }

    if ((row + GifFile->IHeight) > GifFile->SHeight)
        row = GifFile->SHeight-GifFile->IHeight;

    if ((col + GifFile->IWidth) > GifFile->SWidth)
        col = GifFile->SWidth-GifFile->IWidth;

    if ( (row < 0) || (col < 0) )
    {
        return ERROR;
    }

    GifColorType* colorMap = GifFile->SColorMap;
    if (GifFile->IColorMap != NULL)
    {
        colorMap = GifFile->IColorMap;
    }

    d = (unsigned char*)galloc(GifFile->IWidth*sizeof(unsigned char));

    for (i=0; i<GifFile->IHeight; i++)
    {
        memset(d, (char)GifFile->TransparentIndex, GifFile->IWidth);

        if (DGifGetLine(GifFile, d, GifFile->IWidth) == ERROR)
        {
            gfree(d);
            return ERROR;
        }

        for (j=0; j<GifFile->IWidth; j++)
        {
            if (d[j] != GifFile->TransparentIndex)
            {
                ( (ARGB*)(dest+row*rowbytes+4*(col+j)) )->a = 255;
                ( (ARGB*)(dest+row*rowbytes+4*(col+j)) )->r = colorMap[ d[j] ].Red;
                ( (ARGB*)(dest+row*rowbytes+4*(col+j)) )->g = colorMap[ d[j] ].Green;
                ( (ARGB*)(dest+row*rowbytes+4*(col+j)) )->b = colorMap[ d[j] ].Blue;
            }
        }

        // If not the last row, update row, lace and startingRow variables.
        if (i < GifFile->IHeight-1)
        {
            if (GifFile->IInterlace)
            {
                row += lace;
                if (row>=GifFile->ITop + GifFile->IHeight)
                {
                    switch (startingRow)
                    {
                    case 0:
                        lace = 8;
                        startingRow = 4;
                        break;

                    case 4:
                        lace = 4;
                        startingRow = 2;
                        break;

                    case 2:
                        lace = 2;
                        startingRow = 1;
                        break;

                    default:
                        // We shouldn't end up in this case statement.  Clean up and return error code.
                        gfree(d);
                        return ERROR;
                    }

                    row = GifFile->ITop + startingRow;
                }
            }
            else
                ++row;
        }
    }

    gfree(d);

    return OK;
}


/*******************************************
 * Skip over the current image in the file *
 *******************************************/
static int GIFSkipImage(GSTREAM *g)
{
    unsigned char buf[1];

    do
    {
        if (!gread(g,buf, 1))
        {
            return ERROR;
        }
        if (!gseek(g,gtell(g)+buf[0]))
        {
            return ERROR;
        }
    }
    while(buf[0] != 0);

    return OK;
}


static GifFileType *GIFgetinfo(GSTREAM *g,int framenum, char *dest,int rowbytes)
{
    GifFileType *gifinfo;
    GifRecordType RecordType;
    int ok=1;
    int ExtCode;
    unsigned char *Extension;
    int frame=-1;
    int istransparent;
    int i, j;
    int backcolour;
    GifColorType* colorMap;

    gseek(g,(GPOS)0);
    gifinfo = DGifOpenFileHandle(g);
    if (gifinfo)
    {
        gifinfo->TransparentIndex = -1;
        gifinfo->IDisposal = GIF_UNKNOWN;

        /* Scan the content of the GIF file and load the image(s) in: */

        do
        {
            if (DGifGetRecordType(gifinfo, &RecordType) == ERROR)
                ok = 0;

            switch (RecordType)
            {
                case IMAGE_DESC_RECORD_TYPE:
                    if (DGifGetImageDesc(gifinfo) == ERROR)
                        ok = 0;

                    colorMap = gifinfo->SColorMap;
                    if (gifinfo->IColorMap != NULL)
                    {
                        colorMap = gifinfo->IColorMap;
                    }

                    frame++;
                    if (dest)
                    {
                         /* Netscape does this so I will too for compatibility */
                        if (gifinfo->TransparentIndex >= 0)
                            backcolour = gifinfo->TransparentIndex;
                        else
                            backcolour = gifinfo->SBackGroundColor;

                        if (frame == 0)
                        {
                            GifColorType* globalColorMap = gifinfo->SColorMap;
                            if (globalColorMap == NULL)
                            {
                                globalColorMap = gifinfo->IColorMap;
                            }

                            if (globalColorMap)
                            {
                                /* Clear the buffer to the background colour */
                                /* Loop here to respect rowbytes and only clear the image area */
                                for(i=0; i<gifinfo->SHeight; i++)
                                {
                                    for (j=0; j<gifinfo->SWidth; j++)
                                    {
                                        ( (ARGB*)(dest+i*rowbytes+4*j) )->a = (backcolour == gifinfo->TransparentIndex? 0 : 255);
                                        ( (ARGB*)(dest+i*rowbytes+4*j) )->r = globalColorMap[backcolour].Red;
                                        ( (ARGB*)(dest+i*rowbytes+4*j) )->g = globalColorMap[backcolour].Green;
                                        ( (ARGB*)(dest+i*rowbytes+4*j) )->b = globalColorMap[backcolour].Blue;
                                    }
                                }
                            }
                            else
                            {
                                ok = 0;
                            }
                        }

                        if (frame < framenum)
                        {
                            switch(gifinfo->IDisposal)
                            {
                                case GIF_UNKNOWN:
                                case GIF_NO_DISPOSE:
                                    if (LoadImage(gifinfo, dest, rowbytes) == ERROR)
                                        ok = 0;
                                    break;
                                case GIF_BACKGROUND:
                                    if (colorMap)
                                    {
                                        /* Overwrite the image area with the background colour */
                                        for (i=gifinfo->ITop;i<gifinfo->ITop+gifinfo->IHeight;i++)
                                        {
                                            if (i>=0 && i<gifinfo->SHeight)
                                            {
                                                int ILeft=gifinfo->ILeft;
                                                int IWidth=gifinfo->IWidth;
                                                if (ILeft<0)
                                                {
                                                    IWidth -= (0-ILeft);
                                                    ILeft = 0;
                                                }
                                                if (ILeft+IWidth>gifinfo->SWidth)
                                                {
                                                    IWidth -= ((ILeft+IWidth)-gifinfo->SWidth);
                                                }

                                                for (j=0; j<IWidth; j++)
                                                {
                                                    ( (ARGB*)(dest+i*rowbytes+4*(ILeft+j)) )->a = (backcolour == gifinfo->TransparentIndex? 0 : 255);
                                                    ( (ARGB*)(dest+i*rowbytes+4*(ILeft+j)) )->r = colorMap[backcolour].Red;
                                                    ( (ARGB*)(dest+i*rowbytes+4*(ILeft+j)) )->g = colorMap[backcolour].Green;
                                                    ( (ARGB*)(dest+i*rowbytes+4*(ILeft+j)) )->b = colorMap[backcolour].Blue;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        ok = 0;
                                    }
                                case GIF_PREVIOUS:
                                default:
                                    if (GIFSkipImage(g) == ERROR)
                                        ok = 0;
                                    /* Unknown value */
                                    break;
                            }
                        }
                        else
                        {
                            if (LoadImage(gifinfo, dest, rowbytes) == ERROR)
                                ok = 0;
                        }
                    }
                    else
                    {
                        if (GIFSkipImage(g) == ERROR)
                            ok = 0;
                    }

                    break;

                case EXTENSION_RECORD_TYPE:

                    if (DGifGetExtension(gifinfo, &ExtCode, &Extension) == ERROR)
                        ok = 0;

                    if ((ExtCode==GCONTROL_EXT_FUNC_CODE) && (frame < framenum))
                    {
                        gifinfo->IDisposal = (GifDisposalType) (Extension[1]&0x1C);

                        gifinfo->DelayTime = (int) ggeti(Extension+2,2);

                        istransparent = Extension[1]&0x01;
                        if (istransparent)
                        {
                            gifinfo->TransparentIndex = Extension[4];
                        }
                    }

                    /* No support to more than one extension blocks, so discard: */

                    while (Extension != NULL)
                    {
                        if (DGifGetExtensionNext(gifinfo, &Extension) == ERROR)
                            ok = 0;
                    }
                    break;

                case TERMINATE_RECORD_TYPE:
                case SCREEN_DESC_RECORD_TYPE:
                case UNDEFINED_RECORD_TYPE:
                default:             /* Should be traps by DGifGetRecordType */
                    break;
            }
        }
        while (ok && (RecordType!=TERMINATE_RECORD_TYPE) && (dest?(frame < framenum):1));

        if (!ok)
        {
            DGifCloseFile(gifinfo);
            gifinfo = (GifFileType *) 0;
        }
        else
        {
            if (!dest)
                gifinfo->SFrames = frame + 1;
        }
    }
    return(gifinfo);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL GIF_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','G','I','F');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 1;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->movie           = 1;    /* movie file format */
        info->mactype[0]      = GMAKEID('G','I','F','f');   /* mac file system types used */
        strcpy(info->extensions[0], ".gif");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.24");                /* version # */
        strcpy(info->shorttypestr,  "GIF");                 /* type */
        strcpy(info->wordtypestr,   "GIF");                 /* wordtype */
        strcpy(info->longtypestr,   "Compuserve GIF");      /* longtype */
        info->maxalphabits    = 1;
        info->maxredbits      = 8;
        info->maxgreenbits    = 8;
        info->maxbluebits     = 8;
    }
    return(info);
}

/* check for reasonable header: */
/* GIF87a or GIF89a string for header */

int GCALL GIF_is(GSTREAM *g)
{
    char gifbuf[6]; /* Signature and Version */
    int ok=0;

    gseek(g, 0);
    if (gread(g, gifbuf, 6))
    {
        if (!strncmp(gifbuf,"GIF87a",6)
         || !strncmp(gifbuf,"GIF89a",6))
        {
            ok = 100;    /* it's a GIF. */
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL GIF_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    GifFileType *gifinfo;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 0;
        gx->framenum = -1;
        gx->signature=GMAKEID('.','G','I','F');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        gifinfo = GIFgetinfo(g,0,(char *)0,0);
        if (gifinfo)
        {
            gx->frames = ok = gifinfo->SFrames;
            if (DGifCloseFile(gifinfo) == ERROR)
                ok = 0;
        }
        else
            ok = 0;
    }
    return(ok);
}

GINFO * GCALL GIF_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int bpp;
    int colours;
    int i;
    int width;
    int height;
    int ok=0;
    int defaultx;
    int defaulty;
    int trans;
    float fps;
    GifFileType *gifinfo;

    gifinfo = GIFgetinfo(gx->gstream,framenum,(char *)0,0);

    if (gifinfo)
    {
        width       = gifinfo->SWidth;
        height      = gifinfo->SHeight;
        bpp         = gifinfo->SBitsPerPixel;                 /* Colour bits per pixel */
        defaultx    = 0; /* All frames are output positioned on fullsize buffers */
        defaulty    = 0; /* so no further positioning required */
        trans       = gifinfo->TransparentIndex;
        fps = 15.0f;
        if (gifinfo->DelayTime)
            fps = 100.0f/gifinfo->DelayTime;

        if (!bpp) bpp = 8;

        colours = (1 << bpp);
        if (colours>256) colours = 256;

        if (framenum < gx->frames)
            ginfo = (GINFO *) galloc(sizeof(GINFO));

        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','G','I','F');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 /* Current frame num */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = gifinfo->IInterlace?1:0;  /* subtype */
            ginfo->packed           = 1;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 1;            /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = defaultx;     /* Default Coordinate Point */
            ginfo->defaulty         = defaulty;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
            ginfo->bpp              = 32;            /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours       = colours;      /* Number of colours in original indexed palette */
            ginfo->fps              = fps;          /* Framerate */

            if (gifinfo->SColorMap)
            {
                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[i].a = 255;
                    ginfo->colourtbl[i].r = gifinfo->SColorMap[i].Red;
                    ginfo->colourtbl[i].g = gifinfo->SColorMap[i].Green;
                    ginfo->colourtbl[i].b = gifinfo->SColorMap[i].Blue;
                }
            }
            else    /* grey scale */
            {
                for (i=0; i<256; ++i)
                {
                    ginfo->colourtbl[i].a = 255;
                    ginfo->colourtbl[i].r = i;
                    ginfo->colourtbl[i].g = i;
                    ginfo->colourtbl[i].b = i;
                }
            }
            ok = 1;
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }

        if (DGifCloseFile(gifinfo) == ERROR)
            ok = 0;
    }

    return(ginfo);
}

int GCALL GIF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    GifFileType *gifinfo;
    int ok=0;

    gifinfo = GIFgetinfo(gx->gstream, ginfo->framenum, dest,rowbytes);
    if (gifinfo)
    {
        ok = 1;
        if (DGifCloseFile(gifinfo) == ERROR)
            ok = 0;

    }
    return(ok);
}

int GCALL GIF_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

