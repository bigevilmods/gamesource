// (c) 1994-2007 Electronic Arts Inc.
#include <stdio.h>


/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               AVI - Microsoft Audio Visual Interleave            */
/*                                                                  */
/*                 by Kaz Okuda and Frank Barchard, EAC             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970512 KO AVI Format losely based on MOV                 */
/*    1.01 970512 KO Added support for 'rec ' LISTs in data chunk   */
/*    1.02 970514 KO Implemented isavi. Read properly uses rowbytes */
/*    1.03 970520 KO Added chunk index support and RGB formats      */
/*    1.04 970527 KO Video for windows decompression added          */
/*    1.05 970528 KO Fixed warnings. Added VisualC++ sup. for vfw   */
/*    1.06 970602 KO openavi resets *gxptr to 0 on fail after free. */
/*    1.07 970604 KO generate index if needed to help speed.        */
/*    1.08 970812 KO packed=4cc, vfw compression, subtype, quality  */
/*    1.09 970826 KO ginfo->quality correct, uses keyframes         */
/*    1.10 990804 FB ginfo->fps import/export                       */
/*    1.11 000814 FB gabout->globalpalette=1                        */
/*    1.12 000831 KO re-read the same frame.                        */
/*    1.13 001208 FB free blocks on export empty file               */
/*    1.14 010808 ID fixed off by 8 byte error in writer.           */
/*    1.15 020913 AE change bpp,try different codecs if fails       */
/*    1.16 020915 AE 'CVID' convert to 'cvid' for windows to pickup */
/*    1.17 020919 FB q80 default                                    */
/*    1.18 020930 AE various bpp permutations for finding compressor*/
/*    1.19 021008 AE slow loading of AVI's without index fixed up	*/
/*    1.20 021028 AE De/compressor wasn't being closed properly		*/
/*    1.21 021028 AE get framesize from AVI index                   */
/*    1.22 021030 AE enforce even length movi chunks                */
/*    1.23 021104 FB 32 bpp with alpha                              */
/*    1.24 021126 AE move VFW dependence into an interface, avivfw  */
/*    1.25 021203 AE Lower 'IS' if codec not installed on system    */
/*    1.26 021204 AE Better support for finding decompressor        */
/*    1.27 030423 FB keyframe interval 25 default for now (was 5)   */
/*    1.28 030508 FB clean alpha using alpha mask                   */
/*    1.29 030603 FB write 00db for keyframes, 00dc for motion      */
/*    1.30 040108 FB export check width,height same as first frame  */
/*    1.31 050215 FB support up to 4 GB import/export AVI1 format   */
/*    1.32 051017 FB -gop, -vp6, avi2, mCompressor filled in        */
/*    1.33 060404 FB vfw datarate, default fps 30                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1,4,8,16,24,32 bit                                               */
/* Packed 8 bit                                                     */
/* Compressed                                                       */
/* Could do RLE4 if required                                        */
/*                                                                  */
/* Avi decompression and compression supported in windows requires  */
/* "vfw.h" and link with vfw32.lib (Video For Windows).             */
/* compression uses packed field to specify the fourcc of the codec */
/* 0=RGB, 1=RLE8, other=fourcc                                      */
/* Subtype says whether to use fast compression, 0=normal, 1=fast   */
/* For codecs that support it, quality is used                      */
/*                                                                  */
/* Files: aviread.cpp aviwrite.cpp avigimex.h                       */
/*                                                                  */
/* Watcom comes with some useful utilities for inspecting avis      */
/* called riffwalk.exe in the \watcom\bin95 directory.  Also        */
/* aviedit.exe (Same dir) is helpful.                               */
/*                                                                  */
/* Could add a decode cache to module so that reading in reverse    */
/* can reuse the frames decoded since the last key frame.           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Support For Microsoft RLE8                                       */
/*         and Microsoft RGB 1,4,8,16,24,32                         */
/*                                                                  */
/* Windows only:                                                    */
/*         Support for decompressing any avi's supported by         */
/*         the system using video for windows.                      */
/*                                                                  */
/* EA's FourCC codelist                                             */
/*   VP31    On2 VP3 codec (OggVorbis Video)                        */
/*   MV41    Midivid4 (BlackBox)                                    */
/*   CVID    Cinepak 8,16,32                                        */
/*   IV50    Indeo 24                                               */
/*   MSVC    MS Video 1 8,16                                        */
/*   MP42    MPeg 8,24,32                                           */
/*   MRLE    MS RLE 8  -> this codec only supports 8bpp.            */
/*   MJPG    Motion JPeg 24 -> www.jpg.com                          */
/*   MJ2C    Motion JPeg2000                                        */

/*                                                                  */
/* FourCC code reference                                            */
/* http://www.eu.microsoft.com/hwdev/archive/devdes/fourcc.asp      */
/*   * is common                                                    */
/*   Compressor Code  Description                                   */
/*   ANIM  Intel - RDX                                              */
/*   AUR2  AuraVision - Aura 2 Codec - YUV 422                      */
/*   AURA  AuraVision - Aura 1 Codec - YUV 411                      */
/*   BT20  Brooktree - MediaStream codec                            */
/*   BTCV  Brooktree - Composite Video codec                        */
/*   CC12  Intel - YUV12 codec                                      */
/*   CDVC  Canopus - DV codec                                       */
/*   CHAM  Winnov, Inc. - MM_WINNOV_CAVIARA_CHAMPAGNE               */
/*   CPLA  Weitek - 4:2:0 YUV Planar                                */
/*  *CVID  Supermac - Cinepak                                       */
/*   CWLT  reserved                                                 */
/*   DUCK  Duck Corp. - TrueMotion 1.0                              */
/*   DVE2  InSoft - DVE-2 Videoconferencing codec                   */
/*   DXT1  reserved                                                 */
/*   DXT2  reserved                                                 */
/*   DXT3  reserved                                                 */
/*   DXT4  reserved                                                 */
/*   DXT5  reserved                                                 */
/*   DXTC  DirectX Texture Compression                              */
/*   FLJP  D-Vision - Field Encoded Motion JPEG With LSI Bitstream  */
/*   GWLT  reserved                                                 */
/*   H260  Intel - Conferencing codec                               */
/*   H261  Intel - Conferencing codec                               */
/*   H262  Intel - Conferencing codec                               */
/*   H263  Intel - Conferencing codec                               */
/*   H264  Intel - Conferencing codec                               */
/*   H265  Intel - Conferencing codec                               */
/*   H266  Intel - Conferencing codec                               */
/*   H267  Intel - Conferencing codec                               */
/*   H268  Intel - Conferencing codec                               */
/*   H269  Intel - Conferencing codec                               */
/*   I263  Intel - I263                                             */
/*   I420  Intel - Indeo 4 codec                                    */
/*   IAN  Intel - RDX                                               */
/*   ICLB  InSoft - CellB Videoconferencing codec                   */
/*   ILVC  Intel - Layered Video                                    */
/*   ILVR  ITU-T - H.263+ compression standard                      */
/*   IRAW  Intel - YUV uncompressed                                 */
/*   IV30  Intel - Indeo Video 3 codec                              */
/*  *IV31  Intel - Indeo Video 3.1 codec                            */
/*   IV32  Intel - Indeo Video 3 codec                              */
/*   IV33  Intel - Indeo Video 3 codec                              */
/*   IV34  Intel - Indeo Video 3 codec                              */
/*   IV35  Intel - Indeo Video 3 codec                              */
/*   IV36  Intel - Indeo Video 3 codec                              */
/*   IV37  Intel - Indeo Video 3 codec                              */
/*   IV38  Intel - Indeo Video 3 codec                              */
/*   IV39  Intel - Indeo Video 3 codec                              */
/*   IV40  Intel - Indeo Video 4 codec                              */
/*   IV41  Intel - Indeo Video 4 codec                              */
/*   IV42  Intel - Indeo Video 4 codec                              */
/*   IV43  Intel - Indeo Video 4 codec                              */
/*   IV44  Intel - Indeo Video 4 codec                              */
/*   IV45  Intel - Indeo Video 4 codec                              */
/*   IV46  Intel - Indeo Video 4 codec                              */
/*   IV47  Intel - Indeo Video 4 codec                              */
/*   IV48  Intel - Indeo Video 4 codec                              */
/*   IV49  Intel - Indeo Video 4 codec                              */
/*   IV50  Intel - Indeo 5.0                                        */
/*   IV51  Intel - Indeo 5.1                                        */
/*   MP42  Microsoft - MPEG-4 Video Codec V2                        */
/*   MPEG  Chromatic - MPEG 1 Video I Frame                         */
/*   MRCA  FAST Multimedia - Mrcodec                                */
/*  *MRLE  Microsoft - Run Length Encoding                          */
/*  *MSVC  Microsoft - Video 1                                      */
/*   NTN1  Nogatech - Video Compression 1                           */
/*   qpeq  Q-Team - QPEG 1.1 Format video codec                     */
/*   RGBT  Computer Concepts - 32 bit support                       */
/*   RT21  Intel - Indeo 2.1 codec                                  */
/*   RVX  Intel - RDX                                               */
/*   SDCC  Sun Communications - Digital Camera Codec                */
/*   SFMC  Crystal Net - SFM Codec                                  */
/*   SMSC  Radius - proprietary                                     */
/*   SMSD  Radius - proprietary                                     */
/*   SPLC  Splash Studios - ACM audio codec                         */
/*   SQZ2  Microsoft - VXtreme Video Codec V2                       */
/*   SV10  Sorenson - Video R1                                      */
/*   TLMS  TeraLogic - Motion Intraframe Codec                      */
/*   TLST  TeraLogic - Motion Intraframe Codec                      */
/*   TM20  Duck Corp. - TrueMotion 2.0                              */
/*   TMIC  TeraLogic - Motion Intraframe Codec                      */
/*   TMOT  Horizons Technology - TrueMotion Video Compression Algori*/
/*   TR20  Duck Corp. - TrueMotion RT 2.0                           */
/*   V422  Vitec Multimedia - 24 bit YUV 4:2:2 format (CCIR 601).   */
/*   V655  Vitec Multimedia - 16 bit YUV 4:2:2 format.              */
/*   VCR1  ATI - VCR 1.0                                            */
/*   VIVO  Vivo - H.263 Video Codec                                 */
/*   VIXL  Miro Computer Products AG - for use with the Miro line of*/
/*   VLV1  Videologic - VLCAP.DRV                                   */
/*   WBVC  Winbond Electronics - W9960                              */
/*   XLV0  NetXL, Inc. - XL Video Decoder                           */
/*   YC12  Intel - YUV12 codec                                      */
/*   YUV8  Winnov, Inc. - MM_WINNOV_CAVIAR_YUV8                     */
/*   YUV9  Intel - YUV9                                             */
/*   YUYV  Canopus - YUYV compressor                                */
/*   ZPEG  Metheus - Video Zipper                                   */
/*                                                                  */
/*                                                                  */
/*   The following list shows the FOURCC values for DIB compression.*/
/*                                                                  */
/*   Compressor Code  Description                                   */
/*   CYUV  Creative Labs, Inc - Creative Labs YUV                   */
/*   FVF1  Iterated Systems, Inc. - Fractal Video Frame             */
/*   IF09  Intel - Intel Intermediate YUV9                          */
/*   JPEG  Microsoft - Still Image JPEG DIB                         */
/*   MJPG  Microsoft - Motion JPEG DIB Format                       */
/*   PHMO  IBM - Photomotion                                        */
/*   ULTI  IBM - Ultimotion                                         */
/*   VDCT  Vitec Multimedia - Video Maker Pro DIB                   */
/*   VIDS  Vitec Multimedia - YUV 4:2:2 CCIR 601 for V422           */
/*   YU92  Intel - YUV                                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "avigimex.h"

#if defined(_MSC_VER) && !defined(_XBOX)
#  define __MSWINDOWS__
#endif

#ifdef __MSWINDOWS__
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <vfw.h>
#include <avivfw.h>
#endif

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int AVIreadaviheader(GSTREAM *g, unsigned int size, AVIAVIHStruct *header)
{
    int ok=0;
    char buf[56];

    if (size < 56)
    {
        /* Size must be at least 56 bytes */
        return(0);
    }

    ok = gread(g,buf,56);
    size -= 56;

    header->TimeBetweenFrames = ggeti(buf, 4);
    header->MaxBps = ggeti(buf+4, 4);
    header->PaddingGranularity = ggeti(buf+8, 4);
    header->Flags = ggeti(buf+12, 4);
    header->TotalFrames = ggeti(buf+16, 4);
    header->InitFrames = ggeti(buf+20, 4);
    header->Streams = ggeti(buf+24, 4);
    header->SuggestedBufferSize = ggeti(buf+28, 4);
    header->Width = ggeti(buf+32, 4);
    header->Height = ggeti(buf+36, 4);
    header->TimeScale = ggeti(buf+40, 4);
    header->DataRate = ggeti(buf+44, 4);
    header->StartTime = ggeti(buf+48, 4);
    header->DataLength = ggeti(buf+52, 4);

    /* Ignore extra data */
    while(size > 0)
    {
        ok = gread(g,buf,1);
        size--;
    }

    return(ok);
}

static int AVIreaddmlheader(GSTREAM *g, unsigned int size, AVIDMLHStruct *header)
{
    int ok=0;
    char buf[248];

    if (size < 248)
    {
        /* Size must be at least 248 bytes */
        return(0);
    }

    ok = gread(g,buf,248);
    size -= 248;

    header->GrandFrames = ggeti(buf, 4); // fetch GrandFrames field

    /* Ignore extra data */
    while(size > 0)
    {
        ok = gread(g,buf,1);
        size--;
    }

    return(ok);
}


static int AVIreadstreamheader(GSTREAM *g, unsigned int size, AVISTRHStruct *header)
{
    int ok=0;
    char buf[48];

    if (size < 48)
    {
        /* Size must be at least 48 bytes */
        return(0);
    }

    ok = gread(g,buf,48);
    size -= 48;

    header->DataType = ggetm(buf, 4);
    header->DataHandler = ggetm(buf+4, 4);
    header->Flags = ggeti(buf+8, 4);
    header->Priority = ggeti(buf+12, 4);
    header->InitialFrames = ggeti(buf+16, 4);
    header->TimeScale = ggeti(buf+20, 4);
    header->DataRate = ggeti(buf+24, 4);
    header->StartTime = ggeti(buf+28, 4);
    header->DataLength = ggeti(buf+32, 4);
    header->SuggestedBufferSize = ggeti(buf+36, 4);
    header->Quality = ggeti(buf+40, 4);
    header->SampleSize = ggeti(buf+44, 4);

    /* Ignore any extra bytes at the end of the stream header */
    while(size > 0)
    {
        ok = gread(g,buf,1);
        size--;
    }

    return(ok);
}


static int AVIreadbitmapinfo(GSTREAM *g, AVIINFO *avi, unsigned int size)
{
    AVIRGBStruct *cptr;
    AVIBMIStruct *header;
    unsigned int i;
    int ok=0;
    char buf[40];

    if (size < 40)
    {
        /* Size must be at least 40 bytes */
        return(0);
    }

    ok = gread(g,buf,40);   // the stream format can be bigger than 40 bytes, so we need to get the size
                            // ie. compressors and decompressors can have private data

    int sizeBmiHeader = ggeti(buf,4);

    avi->BitmapHeader = (AVIVIDSStruct *) galloc(sizeof(AVIVIDSStruct));
    avi->BitmapHeader->bmiHeader = (AVIBMIStruct *) galloc(sizeBmiHeader);

    if (avi->BitmapHeader && avi->BitmapHeader->bmiHeader)
    {
        memset( (char*)avi->BitmapHeader+4, 0, sizeof(AVIVIDSStruct)-4);
        memset(avi->BitmapHeader->bmiHeader, 0, sizeBmiHeader);
        header = ((avi->BitmapHeader)->bmiHeader);

        //ok = gread(g,buf,40);
        header->Size = ggeti(buf,4);
        header->Width = ggeti(buf+4,4);
        header->Height = ggeti(buf+8,4);
        header->Planes = (unsigned short)ggeti(buf+12,2);
        header->BitCount = (unsigned short)ggeti(buf+14,2);
        header->Compression = ggeti(buf+16,4);
        header->ImageSize = ggeti(buf+20,4);
        header->XpelsMeter = ggeti(buf+24,4);
        header->YpelsMeter = ggeti(buf+28,4);
        header->NumberOfColours = ggeti(buf+32,4);
        header->ImportantColours = ggeti(buf+36,4);
        size -= 40;

        if (sizeBmiHeader > 40) // there is proprietary information, need to copy it over
        {
            char* extra = (char *) galloc(sizeBmiHeader-40);
            ok = gread(g,extra,sizeBmiHeader-40);
            memcpy( (unsigned char *)header + 40, extra, (sizeBmiHeader-40) );
            gfree(extra);
            size -= sizeBmiHeader-40;
        }

        avi->Width = header->Width;
        avi->Height = header->Height;
        avi->Bpp = header->BitCount;
        avi->CurrentFrame = galloc(avi->Height*((avi->Width*avi->Bpp+31)&~31)/8);
        avi->Compression = header->Compression;

        avi->AlphaMask = 255; // initialize for safety

        // kludge VP31/VP50/VP60/VP61

        if (avi->Compression==GMAKEID('1','3','P','V')
         || avi->Compression==GMAKEID('0','5','P','V')
         || avi->Compression==GMAKEID('0','6','P','V')
         || avi->Compression==GMAKEID('1','6','P','V'))
            avi->AlphaMask = 0;

        /* Get the colour palette if there is one */
        if (header->BitCount <= 8)
        {
            if ((header->NumberOfColours)*4 > size)
            {
                ok = 0;
            }
            else
            {
                cptr = (avi->BitmapHeader)->bmiColours;
                memset(cptr, 0, 256 * sizeof(AVIRGBStruct));

                for(i=0; i < header->NumberOfColours; i++)
                {
                    ok = gread(g,buf,4);
                    size -= 4;
                    avi->ColourTable[i].b = *buf;
                    avi->ColourTable[i].g = *(buf+1);
                    avi->ColourTable[i].r = *(buf+2);
                    avi->ColourTable[i].a = 255;  /* Not transparent */
                    cptr->rgbBlue = *buf;
                    cptr->rgbGreen = *(buf+1);
                    cptr->rgbRed = *(buf+2);
                    cptr->rgbReserved = *(buf+3);
                    cptr++;
                    if ((size <= 0) || !ok) break;
                }
            }
        }
        else
        {
            if (size>=4)
            {
                ok = gread(g,buf,4);
                avi->AlphaMask = (int)buf[3];  // ordering is bgra - 3=alpha mask
                size -= 4;
            }
        }

        /* Read the rest of the header */
        while (size > 0)
        {
            ok = gread(g,buf,1);
            size--;
        }

        /* Read the rest of the header */
        while (size > 0)
        {
            ok = gread(g,buf,1);
            size--;
        }

        if (!ok)
        {
            gfree(avi->BitmapHeader);
            avi->BitmapHeader = 0;
        }
    }
    return(ok);
}


static int AVIdecoderle8(const void *source, void *dest, unsigned int size, AVIINFO *avi, int rowbytes)
{
    int ok=1;
    int width;
    int height;
    unsigned int  pos=0;
    unsigned char *buf;
    unsigned char *output;
    unsigned char mode;
    unsigned char opcode;
    int x;
    int y;
    int i;

    width = avi->Width;
    height = avi->Height;
    x = 0;
    y = height-1;
    buf = (unsigned char *)source;
    output = (unsigned char *)avi->CurrentFrame;

    while ((pos < size) && (y >= 0) && ok)
    {
        mode = *buf;
        opcode = *(buf+1);
        buf+=2;
        pos+=2;

        /* Wrap lines */
        while(x > avi->Width)
        {
            x-=width;
            y--;
        }

        if (mode == 0x00)               /* Escape */
        {
            if (opcode == 0x00)         /* EOL */
            {
                x = 0;
                y--;
            }
            else if (opcode == 0x01)    /* EOI */
            {
                y = -1;
                x = width;
                pos = size;
            }
            else if (opcode == 0x02)    /* SKIP */
            {
                x += *buf;
                y -= *(buf+1);
                buf+=2;
                pos+=2;
            }
            else                        /* Absolute mode */
            {
                for(i=0;i<opcode;i++)
                {
                    *(output+width*y+x) = *buf;
                    buf++;
                    pos++;
                    x++;
                }
                if (opcode & 0x01)      /* Must word align */
                {
                    buf++;
                    pos++;
                }
            }
        }
        else                            /* Encoded */
        {
            for(i=0;i<mode;i++)
            {
                *(output+width*y+x) = opcode;
                x++;
            }
        }
    }

    for (i=0;i<height;i++)
    {
        memcpy((char *)dest+rowbytes*i, output+width*i, width);
    }

    return(ok);
}


static int AVIdecodergb(const void *source, void *dest, unsigned int size, AVIINFO *avi, int rowbytes)
{
    int ok=1;
    int i;
    int j;
    int bpl;
    unsigned char *dstp;
    unsigned char *srcp;

    if (avi)
    {
        if (rowbytes < avi->Width) return(0);

        /* Move srcp to the end of the data */
        dstp = (unsigned char *)dest+((avi->Height-1)*rowbytes);

        bpl = ((avi->Width*avi->Bpp+31)&~31)/8;

        for (i = 0; i < avi->Height; i++)
        {
            srcp = (unsigned char *)(source)+i*bpl;
            switch(avi->Bpp)
            {
                case 1:
                    for(j = 0; j < avi->Width; j++)
                    {
                        *dstp++ = (unsigned char)((*srcp>>(7-(j&7)))&1);
                        if ((j&7)==7)
                            srcp++;
                    }
                    break;
                case 4:
                    for(j = 0; j < avi->Width; j++)
                    {
                        if (j&1)
                        {
                            *dstp = (unsigned char)(*srcp&15);
                            srcp++;
                        }
                        else
                        {
                            *dstp = (unsigned char)((*srcp>>4)&15);
                        }
                        dstp++;
                    }
                    break;
                case 8:
                    for(j = 0; j < avi->Width; j++)
                    {
                        *dstp = (unsigned char)ggeti(srcp,1);
                        dstp++;
                        srcp++;
                    }
                    break;
                case 16:
                    for(j = 0; j < avi->Width; j++)
                    {
                        /* 555 */
                        ((ARGB *)dstp)->b = (unsigned char)((((*srcp)&31) * 255 + 16)/31);
                        ((ARGB *)dstp)->g = (unsigned char)((((((*(srcp+1))&3)<<3) | (((*srcp)>>5)&7)) * 255 + 16)/31);
                        ((ARGB *)dstp)->r = (unsigned char)(((((*(srcp+1))>>2)&31) * 255 + 16)/31);
                        ((ARGB *)dstp)->a = (unsigned char)255;
                        dstp+=4;
                        srcp+=2;
                    }
                    break;
                case 24:
                    for(j = 0; j < avi->Width; j++)
                    {
                        ((ARGB *)dstp)->b = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->g = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->r = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->a = (unsigned char)255;
                        dstp+=4;
                    }
                    break;
                case 32:
                    for(j = 0; j < avi->Width; j++)
                    {
                        ((ARGB *)dstp)->b = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->g = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->r = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->a = (unsigned char)ggeti(srcp++,1);
                        dstp+=4;
                    }
                    break;
                default:
                    ok = 0;
                    break;
            }
            dstp-=2*rowbytes;
        }
    }
    return(ok);
}


#ifdef __MSWINDOWS__ /* The following 4 functions are for windows use only */

    static int AVIcreateuncompressedheader(AVIVIDSStruct *to, AVIVIDSStruct *from)
    {
        memcpy(to, from, sizeof(AVIVIDSStruct) + (from->bmiHeader->Size-40) );
        to->bmiHeader->Compression = 0;
        return(1);
    }

    static int AVIdecompressbegin(AVIINFO *avi)
    {
                VfwCodecInfo info;
                memset(&info,0,sizeof(VfwCodecInfo));
                info.width = avi->BitmapHeader->bmiHeader->Width;
                info.height = avi->BitmapHeader->bmiHeader->Height;
                info.srcBpp = avi->BitmapHeader->bmiHeader->BitCount;
                info.fourCC = avi->BitmapHeader->bmiHeader->Compression;
                info.nColours = avi->BitmapHeader->bmiHeader->NumberOfColours;
                info.flags   = 0;

                info.extraBihInfo = (void *) ((char *) avi->BitmapHeader->bmiHeader + 40);
                info.sizeExtraBihInfo = avi->BitmapHeader->bmiHeader->Size - 40;


                avi->hic = (void *) VfwBeginDecompress(&info);

                if(avi->hic)
                {
                    memcpy(&avi->format,&info,sizeof(VfwCodecInfo));
                    return 1;
                }
                else
                    return 0;
    }


    static int AVIdecompressend(AVIINFO *avi)
    {
        return VfwEndDecompress( (VFWHANDLE *) avi->hic, (VfwCodecInfo * ) &avi->format);
    }


    static int AVIdecodeall(const void *source, void *dest, unsigned int size, AVIINFO *avi, int rowbytes)
    {
        int ok=0;

        if (size == 0)
        {
            /* Found a file with a 0 byte frame so I added this test, KO */
            /* If there is no data in the current frame then we will just */
            /* use the last decoded frame. */
            ok = AVIdecodergb(avi->CurrentFrame, dest, size, avi, rowbytes);
            return (ok);
        }

        if(avi->BitmapHeader->bmiHeader->NumberOfColours == 0) // no palette
        {
            ok = VfwDecompress32((unsigned char *) dest, (unsigned char *) source, (VFWHANDLE*) avi->hic,(VfwCodecInfo*) &avi->format, size);
        }
        else
        {
            ok = VfwDecompress8( (unsigned char *) dest, (unsigned char *) source, (VFWHANDLE*) avi->hic,(VfwCodecInfo*) &avi->format, size);
        }

        return ok;

    }

#else /* Not windows */

    static int AVIdecompressbegin(AVIINFO *avi)
    {
        return(0);
    }

    static int AVIdecompressend(AVIINFO *avi)
    {
        return(0);
    }

    static int AVIdecodeall(const void *source, void *dest, unsigned int size, AVIINFO *avi, int rowbytes)
    {
        return(0);
    }

#endif


static int AVIcodecquery(AVIINFO *avi)
{

    int ok=0;

    switch(avi->Compression)
    {
        case AVI_BI_RGB:
        case AVI_RGB:
            switch(avi->Bpp)
            {
                case 1:
                case 4:
                case 8:
                case 16:
                case 24:
                case 32:
                    avi->Decoder = AVIdecodergb;
                    avi->Compression = AVI_BI_RGB;
                    ok = 1;
                    break;
                default:
                    ok = 0;
                    break;
            }
            break;
        case AVI_BI_RLE8:
        case AVI_RLE8:
            if (avi->Bpp == 8)
            {
                avi->Decoder = AVIdecoderle8;
                avi->Compression = AVI_BI_RLE8;
                ok = 1;
            }
            break;

        default:
            avi->Decoder = AVIdecodeall;

            ok = AVIdecompressbegin(avi);
            break;
    }

    return(ok);
}


static int AVIreadindex(GSTREAM *g, GINSTANCE *gx, unsigned int clen)
{
    GPOS offset=0;
    int ok=0;
    int index=0;
    char buf[16];
    AVIINFO *avi;
    int firstchunk = 1;
    GPOS offsetdiff = 0;

    avi = (AVIINFO *) gx->gref;
    if (avi->ChunkOffsetTbl)
    {
        ok = 1;
        while (ok && (offset < clen) && (index < gx->frames))
        {
            ok = gread(g,buf,16);
            offset += 16;

            avi->ChunkOffsetTbl[index].Identifier = ggetm(buf,4);
            avi->ChunkOffsetTbl[index].Flags = ggeti(buf+4,4);
            avi->ChunkOffsetTbl[index].Offset = ggeti(buf+8,4) - offsetdiff;
            avi->ChunkOffsetTbl[index].Length = ggeti(buf+12,4);

            if (firstchunk)
            {
                firstchunk = 0;

                /* If offset not 4 then is offset from start of file so reduce rest of offsets */
                if (avi->ChunkOffsetTbl[index].Offset > 4)
                {
                    offsetdiff = avi->ChunkOffsetTbl[index].Offset - 4;
                    avi->ChunkOffsetTbl[index].Offset = 4;
                }
            }

            /* Ignore non-video indices */
            if ((((avi->ChunkOffsetTbl[index].Identifier)>>16)&0xFFFF) == 0x3030)
            {
                /*
                 * Replaced checks for specific codes like '00dc' or '00iv' since there
                 * are too many options as I think the only restriction is that it starts with '00'
                 * Just it case, these are the codes I have seen until now.
                 *
                 *  case GMAKEID('0','0','4','1'):
                 *  case GMAKEID('0','0','i','v'):
                 *  case GMAKEID('0','0','d','b'):
                 *  case GMAKEID('0','0','d','c'):
                 */
                index++;
            }
        }

        /* Ignore the rest of the chunk */
        while(offset < clen)
        {
            gread(g,buf,1);
            offset++;
        }
    }
    return(ok);
}

static int AVIcreateindex(GSTREAM *g, GINSTANCE *gx, GPOS size)
{
    int ok=0;
    int id;
    unsigned int clen;
    int index=0;
    char buf[8];
    AVIINFO *avi;

    avi = (AVIINFO *) gx->gref;
    if (avi->ChunkOffsetTbl)
    {
        ok = 1;
        while (ok && (gtell(g) < size) && (index < gx->frames))
        {
            /* Get the Chunk id and size */
            ok = gread(g,buf,8);
            id = ggetm(buf,4);
            clen = ggeti(buf+4,4);
            if (clen & 0x01) clen++; /* Must be even */

            /* Ignore non-video indices */
            if (((id>>16)&0xFFFF) == 0x3030)
            {
                avi->ChunkOffsetTbl[index].Identifier = id;
                avi->ChunkOffsetTbl[index].Flags = 0;
                avi->ChunkOffsetTbl[index].Offset = gtell(g) - avi->DataPosition - 4;
                avi->ChunkOffsetTbl[index].Length = clen;
                index++;
            }
            else if (id == GMAKEID('L','I','S','T') || id == GMAKEID('R','I','F','F'))
            {
                clen = 4;            /* Go into the list */
            }

            gseek(g, gtell(g)+clen);
        }
    }
    return(ok);
}

/* Returns the last keyframe before "frame". */
static int AVIfindlastkeyframe(AVIINFO *avi, int frame)
{
#   define AVIKEYFRAME 0x10

    while(!(avi->ChunkOffsetTbl[frame].Flags & AVIKEYFRAME))
    {
        frame--;
        if (frame == 0)
            break;
    }

    return(frame);

#   undef AVIKEYFRAME

}

static int AVI_packtype(int Compression)
{
    if (Compression>255)
        return ggetm(&Compression,4);
    return Compression;
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL AVI_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','A','V','I');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */

#       ifdef __MSWINDOWS__
            info->external        = 1;    /* module uses external resources */
#       else
            info->external        = 0;    /* module uses external resources */
#       endif

        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 0;    /* lie - infact avi is one palette per file, but enabling this rare feature hurts video compression */
        info->maxframename    = 0;    /* max frame name character */
        info->dotsubtype      = 1;    /* subtype based on extension */
        info->movie           = 1;    /* movie file format */
        info->defaultquality  = 80;   /* default pack quality */
        info->mactype[0]      = GMAKEID('.','A','V','I');   /* mac file system types used */
        strcpy(info->extensions[0], ".avi");                /* extensions */
        strcpy(info->extensions[1], ".vp6");
        strcpy(info->authorstr,     "FBarchard & Kaz Okuda");           /* author */
        strcpy(info->versionstr,    "1.33");                /* version # */
        strcpy(info->shorttypestr,  "AVI");                 /* type */
        strcpy(info->wordtypestr,   "AVI");                 /* wordtype */
        strcpy(info->longtypestr,   "Audio Visual Interleave");  /* longtype */
    }
    return(info);
}

static bool AVI_BadCodec(int Compression)
{
#if 0
    switch (AVI_packtype(Compression))
    {
        // kludge VP31
        case 'VP31':
            return true;

        default:
            break;
    }
#endif
    return false;
}

int GCALL AVI_is(GSTREAM *g)
{
    char buf[40];
    int ok=0;

    /* Get the initial chunk id and sub id */
    gseek(g, 0);
    gread(g,buf,12);

    /* If it's an AVI and the main chunk is not larger than the file */
    if ((ggetm(buf,4) == GMAKEID('R','I','F','F')) &&
        (ggetm(buf+8,4) == GMAKEID('A','V','I',' ')))
    {

#ifdef __MSWINDOWS__
        gseek(g,0xA4);  // constant offset to stream format
        gread(g,buf,4);

        if(ggetm(buf,4) == GMAKEID('s','t','r','f'))
        {
            gseek(g,0xA4+8);
            gread(g,buf,40);    // read in strf

            AVIBMIStruct header;
            memset(&header,0,40);

            header.Size = ggeti(buf,4);
            header.Width = ggeti(buf+4,4);
            header.Height = ggeti(buf+8,4);
            header.Planes = (unsigned short)ggeti(buf+12,2);
            header.BitCount = (unsigned short)ggeti(buf+14,2);
            header.Compression = ggeti(buf+16,4);
            header.ImageSize = ggeti(buf+20,4);
            header.XpelsMeter = ggeti(buf+24,4);
            header.YpelsMeter = ggeti(buf+28,4);
            header.NumberOfColours = ggeti(buf+32,4);
            header.ImportantColours = ggeti(buf+36,4);


            VfwCodecInfo info;
            memset(&info,0,sizeof(VfwCodecInfo));

            info.width = header.Width;
            info.height = header.Height;
            info.srcBpp = header.BitCount;
            info.fourCC = header.Compression;
            info.nColours = header.NumberOfColours;
            info.flags   = 0;

            void* hic = NULL;
            hic = (void *) VfwBeginDecompress(&info);
            VfwEndDecompress(hic, &info);

            if (hic)
            {
                if (AVI_BadCodec(header.Compression))   // Perhaps quicktime will have more luck
                    ok = 30;
                else
                    ok = 75;    // give other modules a chance to decompress this AVI, maybe without going through Video for Windows

            }
            else
                ok = 10;    // got upto strf

        }
        else
            ok = 5;     // got the first bit right.

        gseek(g,0);
#else

        ok = 100;
#endif

    }

    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/


int GCALL AVI_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    AVIINFO *avi;
    AVIAVIHStruct aviheader;
    AVIDMLHStruct dmlheader;
    AVISTRHStruct streamheader={0,};
    int  ok=0;
    unsigned char buf[12];
    unsigned int clen;
    unsigned int id;
    unsigned int subid;
    int streamnumber=-1;   /* Ignore all but the first video stream */

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;

        gx->signature=GMAKEID('.','A','V','I');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = 0;  /* keep track of which frame were pointing at */

        gseek(g,(GPOS)0);

        gx->gref = galloc(sizeof(AVIINFO));
        if (gx->gref)
        {
            avi = (AVIINFO *)gx->gref;
            memset((void *) avi,0,sizeof(AVIINFO));

            /* Get the initial chunk size (file size - 8) */
            ok = gread(g,buf,12);
            id = ggetm(buf,4);
            subid = ggetm(buf+8,4);

            if ((id == GMAKEID('R','I','F','F')) &&
                (subid == GMAKEID('A','V','I',' ')))
            {
                while((gtell(g) < glen(g)) && ok)
                {
                    /* Get the Chunk id and size */
                    ok = gread(g,buf,8);

                    id = ggetm(buf,4);
                    clen = ggeti(buf+4,4);
                    if (clen & 0x01) clen++; /* Must be even */

                    switch(id)
                    {
                        case GMAKEID('L','I','S','T'):
                            /* Get the subid of the LIST */
                            ok = gread(g,buf,4);
                            subid = ggetm(buf,4);
                            if (subid == GMAKEID('m','o','v','i'))
                            {
                                avi->DataPosition = (unsigned int)gtell(g);

                                 ok = AVIcreateindex(g, gx, glen(g));

//                                if (!(avi->Flags & AVIHASINDEX)  )//|| (avi->Flags & AVITRUSTCKTYPE))
//                                {
//                                    ok = AVIcreateindex(g, gx, clen-4);
//                                }
//                                else
//                                {
//                                    gseek(g, gtell(g)+clen-4);
//                                }
                            }
                            else
                            {
                                /* Ignore it */
                            }
                            break;
                        case GMAKEID('a','v','i','h'):
                            /* Read in the AVI header */
                            ok = AVIreadaviheader(g, clen, &aviheader);

                            gx->frames = aviheader.TotalFrames;
                            avi->Flags = aviheader.Flags;
                            avi->Fps = 30.0f;
                            if (aviheader.TimeBetweenFrames)
                                avi->Fps = 1000000.0f/aviheader.TimeBetweenFrames;

                            /* Allocate the offset table */
                            avi->ChunkOffsetTbl = (AVIINDXStruct *) galloc(gx->frames*sizeof(AVIINDXStruct));
                            break;

                        case GMAKEID('d','m','l','h'):
                            /* Read in the AVI2 header */
                            ok = AVIreaddmlheader(g, clen, &dmlheader);

                            gx->frames = dmlheader.GrandFrames;

                            /* ReAllocate the offset table */
                            if (avi->ChunkOffsetTbl)
                            {
                                gfree(avi->ChunkOffsetTbl);
                                avi->ChunkOffsetTbl = NULL;
                            }
                            avi->ChunkOffsetTbl = (AVIINDXStruct *) galloc(gx->frames*sizeof(AVIINDXStruct));

                            break;

                        case GMAKEID('s','t','r','h'):

                            /* Advance to the next stream */
                            streamnumber++;

                            /* Ignore all but the first stream*/
                            if (streamnumber == 0)
                            {
                                ok = AVIreadstreamheader(g, clen, &streamheader);
                                avi->Quality = streamheader.Quality/100; /* Max 10000 */
                            }
                            else
                            {
                                ok = gseek(g,gtell(g)+clen);
                            }
                            break;
                        case GMAKEID('s','t','r','f'):
                            /* Only parse video stream header */
                            if ((streamheader.DataType == GMAKEID('v','i','d','s')) &&
                                (streamnumber == 0))
                            {
                                ok = AVIreadbitmapinfo(g, avi, clen);
                            }
                            else
                            {
                                ok = gseek(g,gtell(g)+clen);
                            }
                            break;
                        case GMAKEID('s','t','r','d'):
                            /* Ignore for now */
                            ok = gseek(g,gtell(g)+clen);
                            break;
                        case GMAKEID('s','t','r','n'):
                            /* Ignore for now */
                            ok = gseek(g,gtell(g)+clen);
                            break;
                        case GMAKEID('i','d','x','1'):
                            ok = AVIreadindex(g,gx,clen);
                            break;
                        default:
                            /* Ignore anything I don't recognize */

                            /* Don't go beyond the end of the file. */
                            ok = gseek(g, gmin(gtell(g)+clen, glen(g)));

                            break;
                    }
                }

                /* Set the decoder function */
                if (ok) ok = AVIcodecquery(avi);

            }
        }
        if (ok) ok = gx->frames;
    }

    if (!ok)
    {
        if (*gxptr)
        {
            if ((*gxptr)->gref)
            {
                avi = (AVIINFO *)(*gxptr)->gref;
                if (avi->ChunkOffsetTbl) gfree(avi->ChunkOffsetTbl);
                if (avi->CurrentFrame) gfree(avi->CurrentFrame);
                if (avi->BitmapHeader->bmiHeader) gfree(avi->BitmapHeader->bmiHeader);
                if (avi->BitmapHeader) gfree(avi->BitmapHeader);
                if (avi->hic)
                    AVIdecompressend(avi);
                gfree(avi);
            }
            gfree(*gxptr);
            *gxptr = 0;
        }
    }

    return(ok);
}


GINFO * GCALL AVI_info(GINSTANCE *gx, int framenum)
{
    AVIINFO *avi;
    GINFO *ginfo=0;
    int alphabits;
    int redbits;
    int greenbits;
    int bluebits;

    avi = (AVIINFO *)gx->gref;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        int bpp=avi->Bpp;
        alphabits=8;
        if (bpp>24 && avi->AlphaMask==0)
        {
            bpp = 24;
            alphabits = 0;
        }
        redbits = greenbits = bluebits = avi->Bpp==16?5:8;

        /* template for ginfo */

        ginfo->signature        = GMAKEID('.','A','V','I');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);             /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->width            = avi->Width;  /* Width of bitmap in pixels */
        ginfo->height           = avi->Height; /* Height of bitmap in pixels */
        ginfo->subtype          = 0;           /* Internal format sub-type 0-default */
        ginfo->packed           = AVI_packtype(avi->Compression); /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = avi->Quality;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = alphabits;   /* Number of bits in alpha channel */
        ginfo->redbits          = redbits;     /* Number of bits in red channel */
        ginfo->greenbits        = greenbits;   /* Number of bits in grreen channel */
        ginfo->bluebits         = bluebits;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
        ginfo->originalbpp      = bpp; /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours       = (avi->Bpp==8)?256:(avi->Bpp==4)?16:(avi->Bpp==1)?2:0;    /* Number of colours in original indexed palette */
        ginfo->fps              = avi->Fps;     /* Frame rate */

        if(avi->ChunkOffsetTbl)
        {
            ginfo->framesize    = avi->ChunkOffsetTbl[framenum].Length;
        }
        else
        {
            ginfo->framesize    = avi->BitmapHeader->bmiHeader->ImageSize;
        }


        if (avi->Bpp > 8)
        {
            ginfo->bpp          = 32; /* Bits per pixel (8 or 32) */
        }
        else
        {
            ginfo->bpp          = 8;
            memcpy(ginfo->colourtbl, avi->ColourTable, 256*sizeof(ARGB));
        }
    }
    return(ginfo);
}


int GCALL AVI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
/*
 * NOTE: This code will accept there being no index table even though one should
 *       have been created.  This is just in case anything changes in the future
 *       or somehow we get this far without and index table.  It did work before
 *       making the index manditory, although it is very slow.
 */
    int bpp;
    int bpl;
    int ok=0;
    int diffs;
    unsigned int datalength;
    GPOS curpos=0;
    int id;
    int subid;
    unsigned int clen;
    int frame=0;
    int fileframe=0;
    char buf[8];
    void *source;
    AVIINFO *avi;
    int keyframe;

    avi = (AVIINFO *)gx->gref;
    bpp = avi->Bpp;
    bpl = avi->Width*bpp/8;

    switch(avi->Compression)
    {
        case AVI_BI_RGB:
        case AVI_RGB:
            diffs = 0;
            break;
        default:
            diffs = 1;
    }


    /* Jump to the start of the data */
    gseek(gx->gstream,avi->DataPosition-8);

    /* Read the length of the chunk and the 'movi' id */
    ok = gread(gx->gstream,buf,8);
    datalength = ggeti(buf,4);
    if (datalength & 0x01) datalength++;  /* Make even */

    /* Find an earlier keyframe */
    keyframe = AVIfindlastkeyframe(avi, ginfo->framenum) - 1;

    /* If the best keyframe is after the currently decoded frame, */
    /* or the next frame is before the currently decoded frame, */
    /* then start from scratch at that keyframe. */
    if ( (keyframe > gx->framenum) || (ginfo->framenum < gx->framenum) )
    {
        gx->framenum = keyframe;
    }

    if (diffs)
    {
        if (gx->framenum == ginfo->framenum)
        {
            /* If we are rereading the same frame, go back to previous keyfframe then do just that. */
            frame = gmax(0,keyframe);//gx->framenum;
        }
        else
        {
            /* We are advancing so start on the next frame. */
            frame = gx->framenum+1;
        }
    }
    else
    {
        frame = ginfo->framenum;
    }

    /* Use offsets if they exist to jump past the last frame */
    if (avi->ChunkOffsetTbl)
    {
        fileframe = frame;
    }

    /* By this point... */
    /* "frame" is the point in the file where we should start reading. */
    /* "fileframe" is the point in the file we are decoding from. */
    /* "ginfo->framenum" is the frame we are trying to get. */

    /* "fileframe" keeps stepping through the file frames. */
    /* When "fileframe" gets to "frame" we start decoding. */
    /* When "frame" gets to "ginfo->framenum" we stop.     */

    /* If the frame we are getting is not ahead of the last one we got then */
    /* frame is set to the last one we have, otherwise it is the key frame */
    /* before the import frame.*/

    /* If there is an index table then fileframe is inialized to the same as frame. */
    /* If not, then fileframe must start at the beginning of the file (0). */

    /* Parse each frame of data */
    while ((frame <= ginfo->framenum) && ok) // && (curpos < datalength))
    {
        /* Seek to the "fileframe" if we know where it is.  */
        /* Otherwise it is assumed that we are already there. */
        if (avi->ChunkOffsetTbl)
        {
            ok = gseek(gx->gstream, avi->DataPosition-4 + avi->ChunkOffsetTbl[fileframe].Offset);
            curpos = avi->ChunkOffsetTbl[fileframe].Offset-4;
        }

        /* Get the Chunk id and size */
        ok = gread(gx->gstream,buf,8);
        id = ggetm(buf,4);
        clen = ggeti(buf+4,4);
        if (clen & 0x01) clen++;        /* Make even */
        curpos += 8;

        if (id == GMAKEID('L','I','S','T'))
        {
            ok = gread(gx->gstream, buf, 4);
            subid = ggetm(buf,4);
            curpos += 4;
            switch(subid)
            {
                case GMAKEID('r','e','c',' '):
                    break;
                default:
                    /* Unknown subid */
                    ok = 0;
            }
        }
        else if (((id>>16)&0xFFFF) == 0x3030) /* Check if id start with '00' */
        {
        /*
         * Replaced checks for specific codes like '00dc' or '00iv' since there
         * are too many options as I think the only restriction is that it starts with '00'
         * Just it case, these are the codes I have seen until now
         *
         *  case GMAKEID('0','0','4','1'):
         *  case GMAKEID('0','0','i','v'):
         *  case GMAKEID('0','0','d','b'):
         *  case GMAKEID('0','0','d','c'):
         */
            /* If we have caught up to the current frame in the file */
            if (((fileframe >= frame) && diffs) || (fileframe == ginfo->framenum))
            {
                if (avi->Decoder)
                {
                    if (clen > 0)
                        source = galloc(clen);
                    else
                        source = 0;

                    gread(gx->gstream,source,clen);
                    ok = avi->Decoder((char*)source, dest, clen, avi, rowbytes);

                    // kludge for vp3/vp5/vp6
                    if (avi->Bpp>24 && avi->AlphaMask==0)   // will require stamp alpha with 255
                    {
                        for (int y=0; y<ginfo->height; ++y)
                        {
                            ARGB *d = (ARGB *) ((char *)dest+y*rowbytes);
                            for (int x=0; x<ginfo->width; ++x)
                            {
                                d->a = 255;
                                ++d;
                            }
                        }
                    }

                    if (source)
                        gfree(source);

                    frame++;
                    fileframe++;
                }
                else
                {
                    ok = 0;
                }
            }
            else
            {
                ok = gseek(gx->gstream, gtell(gx->gstream)+clen);
                fileframe++;
            }
            curpos += clen;
        }
        else /* Unknown chunk id */
        {
            /* Ignore it for now */
            ok = gseek(gx->gstream, gtell(gx->gstream)+clen);
            curpos += clen;
        }
    }
    gx->framenum = ginfo->framenum;
    return(ok);
}


int GCALL AVI_close(GINSTANCE *gx)
{
    int ok=0;
    AVIINFO *avi;

    if (gx)
    {
        avi = (AVIINFO *)gx->gref;
        if (avi)
        {
            if (avi->ChunkOffsetTbl) gfree(avi->ChunkOffsetTbl);
            if (avi->CurrentFrame) gfree(avi->CurrentFrame);
            if (avi->BitmapHeader->bmiHeader) gfree(avi->BitmapHeader->bmiHeader);
            if (avi->BitmapHeader) gfree(avi->BitmapHeader);
            if (avi->hic)
                AVIdecompressend(avi);
            gfree(avi);
        }
        ok = gfree((void *) gx);
    }
    return(ok);
}

