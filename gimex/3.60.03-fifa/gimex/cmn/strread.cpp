// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       STR - MDEC Gimex                           */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 950304 FB STR MDEC based on HSI and MDEC PC tool         */
/*    1.01 950308 FB put globals in MDEC context struct             */
/*    1.02 950310 FB rowbyte proper                                 */
/*    1.03 950311 FB fdct,arrays,intel,enc2output,clearpad          */
/*    1.04 950314 FB import based on TGV (for structure)            */
/*    1.05 950319 FB decodefinal/dequantize                         */
/*    1.06 950320 FB idct, importer works (but slow)                */
/*    1.07 950320 FB fast YCbCr,Quant,getbits                       */
/*    1.08 950321 FB fast words,noupsample,decodeit                 */
/*    1.09 950322 FB fast decodefinal,Quant loop                    */
/*    1.10 950324 FB idct noclip,YUV int                            */
/*    1.11 950327 FB YUV,quant,any qfactor,export prequant,dct      */
/*    1.12 950328 FB bit maintainence in decode final               */
/*    1.13 950331 FB MDC chunk variation as conditional compile     */
/*    1.14 950409 FB YUV simple version on SGI                      */
/*    1.15 950411 FB read .xa/extension added                       */
/*    1.16 950420 FB clear sector before writting                   */
/*    1.17 950421 FB XA import/8 in reserved filled/q logic         */
/*    1.18 950529 FB ginstance signature                            */
/*    1.19 950626 FB YUV 2x2 C, asm and asm low quality             */
/*    1.20 950627 FB DCT row reoorder/asm                           */
/*    1.21 950628 FB DCT tables,reorder                             */
/*    1.22 950927 FB standalone                                     */
/*    1.23 951003 FB mdec 3 encode/decode                           */
/*    1.24 951013 FB subtype 2 for refpack instead of mdec (1=xa)   */
/*    1.25 951109 FB casting for MrC (mac)                          */
/*    1.26 970422 FB add alpha to YUVclip table                     */
/*    1.27 970602 KO Cr and Cb arrays in DCT_YUV_RGB now init [255] */
/*    1.28 970602 FB switched to C code. Set a=255                  */
/*    1.29 970618 KO support r/w DCTm (Chunk) format, seek on read  */
/*    1.30 970619 KO gis reports correctly with audio chunks        */
/*    1.31 970625 JF fix bad frame size in STR header on write      */
/*    1.32 970921 FB dotsubtype about field to make DCT easier      */
/*    1.33 971127 KO Added alpha channel support (4 Alpha blocks)   */
/*    1.34 980106 KO Alpha chunk for DCT                            */
/*    1.35 980209 FB Default quality to 8 on export (if 0 passed)   */
/*    1.36 991125 FB Default quality to 8 on export (if 0 passed)   */
/*    1.37 000214 FB MDEC 3 re-added (line 990 exporter)            */
/*    1.38 000404 FB about->movie                                   */
/*    1.39 000720 FB quality for dct based on chunksize/2048        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* Reentrant                                                        */
/* Frame buffered                                                   */
/* Files: strread.cpp strwrite.cpp strgimex.h                           */
/*                                                                  */
/* Tables are allocated and initialized once in openstr and freed   */
/* in closestr.                                                     */
/*                                                                  */
/* Quick YUV inverse table method is 3 times faster, but slightly   */
/* lower quality.  Recommend using it for player.                   */
/*                                                                  */
/* Subtypes:                                                        */
/* 0 str normal mdec 2048 byte data sectors                         */
/* 1 xa  mdec in 2352 byte raw sectors                              */
/* 2 refpack instead of mdec                                        */
/* 4 dct mdec chunk style                                           */
/* 8 store alpha channel                                            */
/* 0x14 dct mdec chunk with alpha chunk.                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* STR files are sector based MDEC files used on Sony PSX.  Each    */
/* frame is independently encoded using lossy JPEG-like compression */
/*                                                                  */
/* Sector structure is:                                             */
/*  STR_UWORD   id;          0160                                       */
/*  STR_UWORD   type;        8001                                       */
/*  STR_UWORD   secCount;    number of current sector in frame          */
/*  STR_UWORD   nSectors;    number of sectors in frame                 */
/*  STR_ULONG   frameCount;  current frame number                       */
/*  STR_ULONG   frameSize;   number of bytes in vlc (add 8 & sectors)   */
/*                                                                  */
/*  STR_UWORD   width;       frame width in pixels (must be mult of 16) */
/*  STR_UWORD   height;      frame height in pixels                     */
/*                                                                  */
/*  STR_UBYTE   reserved[12];   zero                                    */
/*  STR_ULONG   data[USRSIZE];  mdec data itself                        */
/*                                                                  */
/* DCTm chunk format is (dctweave)                                  */
/*  int    chunkid      'DCTm'                                     */
/*  int    chunksize    length of chunk in bytes                   */
/*  int    crc          crc16 for chunk                            */
/*  short   width        frame width                                */
/*  short   height       frame height                               */
/*  char    data[chunksize-16];  mdec data itself                   */
/*                                                                  */
/* MDEC data:                                                       */
/*  short   enc1_length  dct data size                              */
/*  short   signature    0x3800                                     */
/*  short   QFactor      quality factor                             */
/*  short   ver          version 2 or 3                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"

#include "codex.h"
#include "huffcodex.h"

#ifdef MDECCHUNK

#define aboutstr aboutmdc
#define isstr        ismdc
#define openstr      openmdc
#define infostr      infomdc
#define readstr      readmdc
#define closestr     closemdc

#endif

#include "strgimex.h"

#if defined(__NOWATCOMC__) && defined(CMN)
#define ASMYUV 0
//#define ASMDCT 1
#define ASMDCT 0
/*#define ASMYUVLQ 1 */
#endif

/****************************************************************/
/*  Decoder Functions                                           */
/****************************************************************/

static void initdcttbls(int *dctrowtbl);

static int STRinittables(signed char **QuickLen1ptr,
                         unsigned int **QuickCode1ptr,
                         signed char **QuickLen2ptr,
                         unsigned int **QuickCode2ptr,
                         signed char **QuickLen3ptr,
                         unsigned int **QuickCode3ptr,
                         signed char **QuickLen4ptr,
                         unsigned int **QuickCode4ptr,
                         int **QuickYUVptr,
                         int **DCTTabptr)
{
    int i;
    int ok=0;

    unsigned int EnTab1[96*4] =
    {
        0x02,0xFE00,0x01,0x8000,
        0x03,0x0001,0x01,0xC000,
        0x03,0x03FF,0x00,0xE000,
        0x04,0x0401,0x01,0x6000,
        0x04,0x07FF,0x00,0x7000,
        0x05,0x0002,0x01,0x4000,
        0x05,0x03FE,0x00,0x4800,
        0x05,0x0801,0x00,0x5000,
        0x05,0x0BFF,0x00,0x5800,
        0x06,0x0003,0x01,0x2800,
        0x06,0x03FD,0x00,0x2C00,
        0x06,0x0C01,0x00,0x3800,
        0x06,0x0FFF,0x00,0x3C00,
        0x06,0x1001,0x00,0x3000,
        0x06,0x13FF,0x00,0x3400,
        0x07,0x0402,0x01,0x1800,
        0x07,0x07FE,0x00,0x1A00,
        0x07,0x1401,0x00,0x1C00,
        0x07,0x17FF,0x00,0x1E00,
        0x07,0x1801,0x00,0x1400,
        0x07,0x1BFF,0x00,0x1600,
        0x07,0x1C01,0x00,0x1000,
        0x07,0x1FFF,0x00,0x1200,
        0x06,0x7C1F,0x01,0x0400,  /* clue for 16 bit lit */
        0x08,0x0004,0x01,0x0C00,
        0x08,0x03FC,0x00,0x0D00,
        0x08,0x0802,0x00,0x0800,
        0x08,0x0BFE,0x00,0x0900,
        0x08,0x2001,0x00,0x0E00,
        0x08,0x23FF,0x00,0x0F00,
        0x08,0x2401,0x00,0x0A00,
        0x08,0x27FF,0x00,0x0B00,
        0x09,0x0005,0x01,0x2600,
        0x09,0x03FB,0x00,0x2680,
        0x09,0x0006,0x00,0x2100,
        0x09,0x03FA,0x00,0x2180,
        0x09,0x0403,0x00,0x2500,
        0x09,0x07FD,0x00,0x2580,
        0x09,0x0C02,0x00,0x2400,
        0x09,0x0FFE,0x00,0x2480,
        0x09,0x2801,0x00,0x2700,
        0x09,0x2BFF,0x00,0x2780,
        0x09,0x2C01,0x00,0x2300,
        0x09,0x2FFF,0x00,0x2380,
        0x09,0x3001,0x00,0x2200,
        0x09,0x33FF,0x00,0x2280,
        0x09,0x3401,0x00,0x2000,
        0x09,0x37FF,0x00,0x2080,
        0x0B,0x0007,0x01,0x0280,
        0x0B,0x03F9,0x00,0x02A0,
        0x0B,0x0404,0x00,0x0300,
        0x0B,0x07FC,0x00,0x0320,
        0x0B,0x0803,0x00,0x02C0,
        0x0B,0x0BFD,0x00,0x02E0,
        0x0B,0x1002,0x00,0x03C0,
        0x0B,0x13FE,0x00,0x03E0,
        0x0B,0x1402,0x00,0x0240,
        0x0B,0x17FE,0x00,0x0260,
        0x0B,0x3801,0x00,0x0380,
        0x0B,0x3BFF,0x00,0x03A0,
        0x0B,0x3C01,0x00,0x0340,
        0x0B,0x3FFF,0x00,0x0360,
        0x0B,0x4001,0x00,0x0200,
        0x0B,0x43FF,0x00,0x0220,
        0x0D,0x0008,0x01,0x01D0,
        0x0D,0x03F8,0x00,0x01D8,
        0x0D,0x0009,0x00,0x0180,
        0x0D,0x03F7,0x00,0x0188,
        0x0D,0x000A,0x00,0x0130,
        0x0D,0x03F6,0x00,0x0138,
        0x0D,0x000B,0x00,0x0100,
        0x0D,0x03F5,0x00,0x0108,
        0x0D,0x0405,0x00,0x01B0,
        0x0D,0x07FB,0x00,0x01B8,
        0x0D,0x0804,0x00,0x0140,
        0x0D,0x0BFC,0x00,0x0148,
        0x0D,0x0C03,0x00,0x01C0,
        0x0D,0x0FFD,0x00,0x01C8,
        0x0D,0x1003,0x00,0x0120,
        0x0D,0x13FD,0x00,0x0128,
        0x0D,0x1802,0x00,0x01E0,
        0x0D,0x1BFE,0x00,0x01E8,
        0x0D,0x1C02,0x00,0x0150,
        0x0D,0x1FFE,0x00,0x0158,
        0x0D,0x2002,0x00,0x0110,
        0x0D,0x23FE,0x00,0x0118,
        0x0D,0x4401,0x00,0x01F0,
        0x0D,0x47FF,0x00,0x01F8,
        0x0D,0x4801,0x00,0x01A0,
        0x0D,0x4BFF,0x00,0x01A8,
        0x0D,0x4C01,0x00,0x0190,
        0x0D,0x4FFF,0x00,0x0198,
        0x0D,0x5001,0x00,0x0170,
        0x0D,0x53FF,0x00,0x0178,
        0x0D,0x5401,0x00,0x0160,
        0x0D,0x57FF,0x00,0x0168
    };

    unsigned int  EnTab2[128*4] =
    {
        0x06,0x000C,0x01,0xD000,
        0x06,0x03F4,0x00,0xD400,
        0x06,0x000D,0x00,0xC800,
        0x06,0x03F3,0x00,0xCC00,
        0x06,0x000E,0x00,0xC000,
        0x06,0x03F2,0x00,0xC400,
        0x06,0x000F,0x00,0xB800,
        0x06,0x03F1,0x00,0xBC00,
        0x06,0x0406,0x00,0xB000,
        0x06,0x07FA,0x00,0xB400,
        0x06,0x0407,0x00,0xA800,
        0x06,0x07F9,0x00,0xAC00,
        0x06,0x0805,0x00,0xA000,
        0x06,0x0BFB,0x00,0xA400,
        0x06,0x0C04,0x00,0x9800,
        0x06,0x0FFC,0x00,0x9C00,
        0x06,0x1403,0x00,0x9000,
        0x06,0x17FD,0x00,0x9400,
        0x06,0x2402,0x00,0x8800,
        0x06,0x27FE,0x00,0x8C00,
        0x06,0x2802,0x00,0x8000,
        0x06,0x2BFE,0x00,0x8400,
        0x06,0x5801,0x00,0xF800,
        0x06,0x5BFF,0x00,0xFC00,
        0x06,0x5C01,0x00,0xF000,
        0x06,0x5FFF,0x00,0xF400,
        0x06,0x6001,0x00,0xE800,
        0x06,0x63FF,0x00,0xEC00,
        0x06,0x6401,0x00,0xE000,
        0x06,0x67FF,0x00,0xE400,
        0x06,0x6801,0x00,0xD800,
        0x06,0x6BFF,0x00,0xDC00,
        0x07,0x0010,0x01,0x7C00,
        0x07,0x03F0,0x00,0x7E00,
        0x07,0x0011,0x00,0x7800,
        0x07,0x03EF,0x00,0x7A00,
        0x07,0x0012,0x00,0x7400,
        0x07,0x03EE,0x00,0x7600,
        0x07,0x0013,0x00,0x7000,
        0x07,0x03ED,0x00,0x7200,
        0x07,0x0014,0x00,0x6C00,
        0x07,0x03EC,0x00,0x6E00,
        0x07,0x0015,0x00,0x6800,
        0x07,0x03EB,0x00,0x6A00,
        0x07,0x0016,0x00,0x6400,
        0x07,0x03EA,0x00,0x6600,
        0x07,0x0017,0x00,0x6000,
        0x07,0x03E9,0x00,0x6200,
        0x07,0x0018,0x00,0x5C00,
        0x07,0x03E8,0x00,0x5E00,
        0x07,0x0019,0x00,0x5800,
        0x07,0x03E7,0x00,0x5A00,
        0x07,0x001A,0x00,0x5400,
        0x07,0x03E6,0x00,0x5600,
        0x07,0x001B,0x00,0x5000,
        0x07,0x03E5,0x00,0x5200,
        0x07,0x001C,0x00,0x4C00,
        0x07,0x03E4,0x00,0x4E00,
        0x07,0x001D,0x00,0x4800,
        0x07,0x03E3,0x00,0x4A00,
        0x07,0x001E,0x00,0x4400,
        0x07,0x03E2,0x00,0x4600,
        0x07,0x001F,0x00,0x4000,
        0x07,0x03E1,0x00,0x4200,
        0x08,0x0020,0x01,0x3000,
        0x08,0x03E0,0x00,0x3100,
        0x08,0x0021,0x00,0x2E00,
        0x08,0x03DF,0x00,0x2F00,
        0x08,0x0022,0x00,0x2C00,
        0x08,0x03DE,0x00,0x2D00,
        0x08,0x0023,0x00,0x2A00,
        0x08,0x03DD,0x00,0x2B00,
        0x08,0x0024,0x00,0x2800,
        0x08,0x03DC,0x00,0x2900,
        0x08,0x0025,0x00,0x2600,
        0x08,0x03DB,0x00,0x2700,
        0x08,0x0026,0x00,0x2400,
        0x08,0x03DA,0x00,0x2500,
        0x08,0x0027,0x00,0x2200,
        0x08,0x03D9,0x00,0x2300,
        0x08,0x0028,0x00,0x2000,
        0x08,0x03D8,0x00,0x2100,
        0x08,0x0408,0x00,0x3E00,
        0x08,0x07F8,0x00,0x3F00,
        0x08,0x0409,0x00,0x3C00,
        0x08,0x07F7,0x00,0x3D00,
        0x08,0x040A,0x00,0x3A00,
        0x08,0x07F6,0x00,0x3B00,
        0x08,0x040B,0x00,0x3800,
        0x08,0x07F5,0x00,0x3900,
        0x08,0x040C,0x00,0x3600,
        0x08,0x07F4,0x00,0x3700,
        0x08,0x040D,0x00,0x3400,
        0x08,0x07F3,0x00,0x3500,
        0x08,0x040E,0x00,0x3200,
        0x08,0x07F2,0x00,0x3300,
        0x09,0x040F,0x01,0x1300,
        0x09,0x07F1,0x00,0x1380,
        0x09,0x0410,0x00,0x1200,
        0x09,0x07F0,0x00,0x1280,
        0x09,0x0411,0x00,0x1100,
        0x09,0x07EF,0x00,0x1180,
        0x09,0x0412,0x00,0x1000,
        0x09,0x07EE,0x00,0x1080,
        0x09,0x1803,0x00,0x1400,
        0x09,0x1BFD,0x00,0x1480,
        0x09,0x2C02,0x00,0x1A00,
        0x09,0x2FFE,0x00,0x1A80,
        0x09,0x3002,0x00,0x1900,
        0x09,0x33FE,0x00,0x1980,
        0x09,0x3402,0x00,0x1800,
        0x09,0x37FE,0x00,0x1880,
        0x09,0x3802,0x00,0x1700,
        0x09,0x3BFE,0x00,0x1780,
        0x09,0x3C02,0x00,0x1600,
        0x09,0x3FFE,0x00,0x1680,
        0x09,0x4002,0x00,0x1500,
        0x09,0x43FE,0x00,0x1580,
        0x09,0x6C01,0x00,0x1F00,
        0x09,0x6FFF,0x00,0x1F80,
        0x09,0x7001,0x00,0x1E00,
        0x09,0x73FF,0x00,0x1E80,
        0x09,0x7401,0x00,0x1D00,
        0x09,0x77FF,0x00,0x1D80,
        0x09,0x7801,0x00,0x1C00,
        0x09,0x7BFF,0x00,0x1C80,
        0x09,0x7C01,0x00,0x1B00,
        0x09,0x7FFF,0x00,0x1B80
    };

    STR_UWORD NEnTab1[9*4] =
    {
        0x02,0x0001,0x00,0x0000,
        0x02,0x0002,0x00,0x4000,
        0x03,0x0000,0x00,0x8000,
        0x03,0x0003,0x00,0xa000,
        0x03,0x0004,0x00,0xc000,
        0x04,0x0005,0x00,0xe000,
        0x05,0x0006,0x00,0xf000,
        0x06,0x0007,0x00,0xf800,
        0x07,0x0008,0x00,0xfc00,
    };

    STR_UWORD NEnTab2[9*4] =
    {
        0x02,0x0000,0x00,0x0000,
        0x02,0x0001,0x00,0x4000,
        0x02,0x0002,0x00,0x8000,
        0x03,0x0003,0x00,0xc000,
        0x04,0x0004,0x00,0xe000,
        0x05,0x0005,0x00,0xf000,
        0x06,0x0006,0x00,0xf800,
        0x07,0x0007,0x00,0xfc00,
        0x08,0x0008,0x00,0xfe00,
    };

    signed char *QuickLen1;
    unsigned int *QuickCode1;
    signed char *QuickLen2;
    unsigned int *QuickCode2;
    signed char *QuickLen3;
    unsigned int *QuickCode3;
    signed char *QuickLen4;
    unsigned int *QuickCode4;
    int *QuickYUV;
    int *DCTTab;

    /* initialize QuickLen1 */
    /*  n table 1 index */
    /* 64 fe00 end of block */
    /* 65 table 2 */
    /* 66 16 bit literal */

    QuickLen1 =   (signed char *) galloc(8192);
    QuickCode1 = (unsigned int *) galloc(8192*sizeof(int));
    QuickLen2 =   (signed char *) galloc(512);
    QuickCode2 = (unsigned int *) galloc(512*sizeof(int));
    QuickLen3 =   (signed char *) galloc(256);
    QuickCode3 = (unsigned int *) galloc(256*sizeof(int));
    QuickLen4 =   (signed char *) galloc(256);
    QuickCode4 = (unsigned int *) galloc(256*sizeof(int));
    QuickYUV =            (int *) galloc(4);/*65536*4); */

#ifdef ASMDCT
    {
        extern int DctTable[12*2048];


        DCTTab = DctTable;
    }

#else
    DCTTab =              (int *) galloc(12*2048*4);
#endif

    if (QuickLen1 && QuickCode1
     && QuickLen2 && QuickCode2
     && QuickLen3 && QuickCode3
     && QuickLen4 && QuickCode4
     && QuickYUV && DCTTab)
    {
        int bits;
        int bitscount;
        int j;
        int start;
        int v;
        int v2;

        for (i=96-1; i>=0; --i)
        {
            bits = EnTab1[i*4];

            bitscount = 1 << (13-bits);
            start = EnTab1[i*4+3]>>3;

            v = bits;
            if (i==23)
                v = 66;
            if (i==0)
                v = 64;
            v2 = EnTab1[i*4+1];

            for (j=0; j<bitscount; ++j)
            {
                QuickLen1[start+j] = v;
                QuickCode1[start+j] = v2;
            }
        }

        for (i=0; i<32; ++i)
            QuickLen1[i] = 65;

        for (i=0; i<128; ++i)
        {
            bits = EnTab2[i*4];

            bitscount = 1 << (9-bits);
            start = EnTab2[i*4+3]>>7;

            v = bits;
            v2 = EnTab2[i*4+1];

            for (j=0; j<bitscount; ++j)
            {
                QuickLen2[start+j] = v;
                QuickCode2[start+j] = v2;
            }
        }

        /* MDEC 3 DC decode tables */
        for (i=0; i<9; ++i)
        {
            bits = NEnTab1[i*4];

            bitscount = 1 << (8-bits);
            start = NEnTab1[i*4+3]>>8;

            v = bits;
            v2 = NEnTab1[i*4+1];

            for (j=0; j<bitscount; ++j)
            {
                QuickLen3[start+j] = v;
                QuickCode3[start+j] = v2;
            }
        }

        for (i=0; i<9; ++i)
        {
            bits = NEnTab2[i*4];

            bitscount = 1 << (8-bits);
            start = NEnTab2[i*4+3]>>8;

            v = bits;
            v2 = NEnTab2[i*4+1];

            for (j=0; j<bitscount; ++j)
            {
                QuickLen4[start+j] = v;
                QuickCode4[start+j] = v2;
            }
        }

#if 0
        {
            int y,cb,cr;
            int r,g,b;
            int i;

            unsigned char uclip[1024];
            int  CrP1p4020[256];    /*  1.4020*Cr */
            int  CrN0p7141[256];    /* -0.7141*Cr */
            int  CbN0p3441[256];    /* -0.3441*Cb */
            int  CbP1p7720[256];    /*  1.7720*Cb */

            for (i=-512; i<512; ++i)
                uclip[i+512] = 0xff000000|gmax(0,gmin(255,i));

            /* KO 97/06/02 - Could be a problem in the future. 'i' never gets to 127 */
            /* Don't want to change now as I don't kow enough about it and can't test */
            for (i=-128; i<127; ++i)
            {
                CrP1p4020[i+128] = (int) (((double)  1.4020 * (double) i));
                CrN0p7141[i+128] = (int) (((double) -0.7141 * (double) i));
                CbN0p3441[i+128] = (int) (((double) -0.3441 * (double) i));
                CbP1p7720[i+128] = (int) (((double)  1.7720 * (double) i));
            }


            i = 0;
            for (y=0; y<64; ++y)
            {
                int Y=y*4;
                for (cb=0; cb<32; ++cb)
                {
                    int Cb=(cb<<3)-128;

                    b = (uclip+512)[Y + CbP1p7720[Cb+128]];
                    for (cr=0; cr<32; ++cr)
                    {
                        int Cr=(cr<<3)-128;

                        r = (uclip+512)[Y + CrP1p4020[Cr+128]];
                        g = (uclip+512)[Y + CrN0p7141[Cr+128] + CbN0p3441[Cb+128]];
                        QuickYUV[i++] = 0xff000000|(r<<16)|(g<<8)|(b);
                    }
                }
            }
        }
#endif

        initdcttbls(DCTTab);
        ok = 1;
    }
    *QuickLen1ptr  = QuickLen1;
    *QuickCode1ptr = QuickCode1;
    *QuickLen2ptr  = QuickLen2;
    *QuickCode2ptr = QuickCode2;
    *QuickLen3ptr  = QuickLen3;
    *QuickCode3ptr = QuickCode3;
    *QuickLen4ptr  = QuickLen4;
    *QuickCode4ptr = QuickCode4;
    *QuickYUVptr = QuickYUV;
    *DCTTabptr = DCTTab;
    return(ok);
}

static void STRfreetables(signed char **QuickLen1ptr,
                         unsigned int **QuickCode1ptr,
                         signed char **QuickLen2ptr,
                         unsigned int **QuickCode2ptr,
                         signed char **QuickLen3ptr,
                         unsigned int **QuickCode3ptr,
                         signed char **QuickLen4ptr,
                         unsigned int **QuickCode4ptr,
                         int **QuickYUVptr,
                         int **DCTTabptr)
{
    if (*QuickLen1ptr)
    {
        gfree(*QuickLen1ptr);
        *QuickLen1ptr = 0;
    }
    if (*QuickCode1ptr)
    {
        gfree(*QuickCode1ptr);
        *QuickCode1ptr = 0;
    }
    if (*QuickLen2ptr)
    {
        gfree(*QuickLen2ptr);
        *QuickLen2ptr = 0;
    }
    if (*QuickCode2ptr)
    {
        gfree(*QuickCode2ptr);
        *QuickCode2ptr = 0;
    }
    if (*QuickLen3ptr)
    {
        gfree(*QuickLen3ptr);
        *QuickLen3ptr = 0;
    }
    if (*QuickCode3ptr)
    {
        gfree(*QuickCode3ptr);
        *QuickCode3ptr = 0;
    }
    if (*QuickLen4ptr)
    {
        gfree(*QuickLen4ptr);
        *QuickLen4ptr = 0;
    }
    if (*QuickCode4ptr)
    {
        gfree(*QuickCode4ptr);
        *QuickCode4ptr = 0;
    }
    if (*QuickYUVptr)
    {
        gfree(*QuickYUVptr);
        *QuickYUVptr = 0;
    }
    if (*DCTTabptr)
    {
#ifndef ASMDCT
        gfree(*DCTTabptr);
#endif
        *DCTTabptr = 0;
    }
}


/* optimized but doesnt work on sample.str */

/****************************************************************/
/*  Inverse Discrete Cosine Transform                           */
/****************************************************************/

#define W1 5681 /* 4096*sqrt(2)*cos(1*pi/16) */
#define W2 5352 /* 4096*sqrt(2)*cos(2*pi/16) */
#define W3 4816 /* 4096*sqrt(2)*cos(3*pi/16) */
#define W5 3218 /* 4096*sqrt(2)*cos(5*pi/16) */
#define W6 2217 /* 4096*sqrt(2)*cos(6*pi/16) */
#define W7 1130 /* 4096*sqrt(2)*cos(7*pi/16) */

#define RTW7    (dctrowtbl+ 0*2048+2048)
#define RTW1mW7 (dctrowtbl+ 2*2048+1024)
#define RTW1pW7 (dctrowtbl+ 3*2048+1024)
#define RTW3    (dctrowtbl+ 4*2048+2048)
#define RTW3mW5 (dctrowtbl+ 6*2048+1024)
#define RTW3pW5 (dctrowtbl+ 7*2048+1024)
#define RTW6    (dctrowtbl+ 8*2048+2048)
#define RTW2pW6 (dctrowtbl+10*2048+1024)
#define RTW2mW6 (dctrowtbl+11*2048+1024)

#define initdcttbl(size,tbl,coef) \
    for (i=-size; i<size; ++i) \
        (tbl)[i] = (coef)*i; \

static void initdcttbls(int *dctrowtbl)
{
    int i;

    initdcttbl(2048,RTW7,   W7);
    initdcttbl(1024,RTW1mW7,W1-W7);
    initdcttbl(1024,RTW1pW7,W1+W7);
    initdcttbl(2048,RTW3,   W3);
    initdcttbl(1024,RTW3mW5,W3-W5);
    initdcttbl(1024,RTW3pW5,W3+W5);
    initdcttbl(2048,RTW6,   W6);
    initdcttbl(1024,RTW2pW6,W2+W6);
    initdcttbl(1024,RTW2mW6,W2-W6);
}

#ifdef ASMDCT

extern void cdecl STR_idctrow(int *src,int *dst);

extern void cdecl STR_idctrowpragma(int *src,int *dst);

#pragma aux STR_idctrowpragma =         \
    "call        STR_idctrow"               \
    parm [esi] [edi]                    \
    modify [eax ebx ecx edx esi edi];

#define STR_idctrow(a,b,c) STR_idctrowpragma(b,c)

#else

static void STR_idctrow(int *dctrowtbl, int *src,int *dst)
{
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;
    int t;
    int s1;
    int s2;

    /* shortcut */
    if (!src[0]
     && !src[4]
     && !src[7]
     && !src[2]
     && !src[6]
     && !src[1]
     && !src[3]
     && !src[5])
    {
        dst[0] = dst[1] = dst[2] = dst[3] =
        dst[4] = dst[5] = dst[6] = dst[7] = 0;
        return;
    }

    /* first stage */

    s1 = src[1];
    s2 = src[7];
    t = RTW7[(s1+s2)];
    x4 = t + (RTW1mW7)[s1];
    x5 = t - (RTW1pW7)[s2];

    s1 = src[5];
    s2 = src[3];
    t = RTW3[(s1+s2)];
    x6 = t - (RTW3mW5)[s1];
    x7 = t - (RTW3pW5)[s2];

    s1 = src[0];
    s2 = src[4];
    x8 = ((s1+s2)<<12) + 16; /* +16 for proper rounding in the fourth stage */
    x0 = ((s1-s2)<<12) + 16;

    s1 = src[2];
    s2 = src[6];
    t = RTW6[(s1+s2)];
    x3 = t + (RTW2mW6)[s1];
    x2 = t - (RTW2pW6)[s2];

    /* second stage (no source) */

    x1 = x4 + x6;
    x4 = x4 - x6;
    x6 = x5 + x7;
    x5 = x5 - x7;

    x7 = x8 + x3;
    dst[0] = (x7+x1)>>5;
    dst[7] = (x7-x1)>>5;

    x8 = x8 - x3;
    dst[3] = (x8+x6)>>5;
    dst[4] = (x8-x6)>>5;

    x3 = x0 + x2;
    x0 = x0 - x2;
    x2 = (181*(x4+x5)+128)>>8;
    dst[1] = (x3+x2)>>5;
    dst[6] = (x3-x2)>>5;

    x4 = (181*(x4-x5)+128)>>8;
    dst[2] = (x0+x4)>>5;
    dst[5] = (x0-x4)>>5;
}

#endif

static void STR_idctcol(int *src,int *dst)
{
    int x0, x1, x2, x3, x4, x5, x6, x7, x8;

    /* first stage */
    x8 = W7*(src[8*1]+src[8*7]) + (2048);
    x4 = (x8+(W1-W7)*src[8*1])>>12;
    x5 = (x8-(W1+W7)*src[8*7])>>12;
    x8 = W3*(src[8*5]+src[8*3]) + (2048);
    x6 = (x8-(W3-W5)*src[8*5])>>12;
    x7 = (x8-(W3+W5)*src[8*3])>>12;

    /* second stage */
    x8 = src[8*0] + src[8*4];
    x0 = src[8*0] - src[8*4];
    x1 = W6*(src[8*2]+src[8*6]) + (2048);
    x2 = (x1-(W2+W6)*src[8*6])>>12;
    x3 = (x1+(W2-W6)*src[8*2])>>12;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;

    /* third stage */
    x7 = x8 + x3 + (512);
    x8 += -x3 + (512);
    x3 = x0 + x2 + (512);
    x0 += -x2 + (512);
    x2 = (181*(x4+x5)+(128))>>8;
    x4 = (181*(x4-x5)+(128))>>8;

    /* fourth stage */
    dst[8*0] = (int)(x7+x1)>>10;
    dst[8*7] = (int)(x7-x1)>>10;
    dst[8*1] = (int)(x3+x2)>>10;
    dst[8*6] = (int)(x3-x2)>>10;
    dst[8*2] = (int)(x0+x4)>>10;
    dst[8*5] = (int)(x0-x4)>>10;
    dst[8*3] = (int)(x8+x6)>>10;
    dst[8*4] = (int)(x8-x6)>>10;
}

/* two dimensional inverse discrete cosine transform */

static void STR_idct(int *dctrowtbl, STR_LONG *dctin, STR_LONG *dctout)
{
    int temp[64];     /* intermediate storage for 1D transform */

    STR_idctrow(dctrowtbl,dctin+8*0, temp+8*0);
    STR_idctrow(dctrowtbl,dctin+8*1, temp+8*1);
    STR_idctrow(dctrowtbl,dctin+8*2, temp+8*2);
    STR_idctrow(dctrowtbl,dctin+8*3, temp+8*3);
    STR_idctrow(dctrowtbl,dctin+8*4, temp+8*4);
    STR_idctrow(dctrowtbl,dctin+8*5, temp+8*5);
    STR_idctrow(dctrowtbl,dctin+8*6, temp+8*6);
    STR_idctrow(dctrowtbl,dctin+8*7, temp+8*7);

    STR_idctcol(temp+0, dctout+0);
    STR_idctcol(temp+1, dctout+1);
    STR_idctcol(temp+2, dctout+2);
    STR_idctcol(temp+3, dctout+3);
    STR_idctcol(temp+4, dctout+4);
    STR_idctcol(temp+5, dctout+5);
    STR_idctcol(temp+6, dctout+6);
    STR_idctcol(temp+7, dctout+7);
}




#ifdef MSDOS

#define STRRGB(uclip,CrP1p4020,CrN0p7141,CbN0p3441,CbP1p7720,Y,Cb,Cr,red,green,blue) \
{ \
    *red =   (uclip)[(Y +                      CrP1p4020[Cr+128])]; \
    *green = (uclip)[(Y + CbN0p3441[Cb+128]  + CrN0p7141[Cr+128])]; \
    *blue =  (uclip)[(Y + CbP1p7720[Cb+128])]; \
}

#else

#define STRRGB(uclip,CrP1p4020,CrN0p7141,CbN0p3441,CbP1p7720,Y,Cb,Cr,red,green,blue) \
{ \
    *red =   (uclip)[(int)(Y +                1.4020*Cr)]; \
    *green = (uclip)[(int)(Y + -0.3441*Cb  + -0.7141*Cr)]; \
    *blue =  (uclip)[(int)(Y +  1.7720*Cb)]; \
}

#endif

#if 0

#define STRRGB(uclip,CrP1p4020,CrN0p7141,CbN0p3441,CbP1p7720,Y,Cb,Cr,red,green,blue) \
{ \
    *(int *) blue = *(int *)((char *)QuickYUV + ((YIndex+512)[Y] | (CbIndex+512)[Cb] | (CrIndex+512)[Cr])); \
}
#endif


/* Convert RGB frame to Y Cr Cb and perform DCT on blocks           */

typedef struct
{
    STR_LONG    Y1[64];
    STR_LONG    Y2[64];
    STR_LONG    Y3[64];
    STR_LONG    Y4[64];
    STR_LONG    Cb[64];
    STR_LONG    Cr[64];
    unsigned char  ucliptbl[1024]; /* clipping table */

    int  CrP1p4020[256];    /*  1.4020*Cr */
    int  CrN0p7141[256];    /* -0.7141*Cr */
    int  CbN0p3441[256];    /* -0.3441*Cb */
    int  CbP1p7720[256];    /*  1.7720*Cb */
} YUVTBL;


#ifdef ASMYUV

#define yuv8x2(yuv,YPtr,CbPtr,p,rowbytes) yuv8x2pcc(YPtr,CbPtr,p,rowbytes)

void yuv8x2pcc(STR_LONG *YPtr,STR_LONG *CbPtr,ARGB *p, int rowbytes);

void __cdecl yuv8x2pc(STR_LONG *YPtr,STR_LONG *CbPtr,ARGB *p, int rowbytes);
void __cdecl yuv8x2lq(STR_LONG *YPtr,STR_LONG *CbPtr,ARGB *p, int rowbytes);

#ifdef ASMYUVLQ

#pragma aux yuv8x2pcc =                 \
    "call        yuv8x2lq"                  \
    parm [esi] [eax] [edi] [edx]        \
    modify [eax ebx ecx edx esi edi];

#else

#pragma aux yuv8x2pcc =                 \
    "call        yuv8x2pc"                  \
    parm [esi] [eax] [edi] [edx]        \
    modify [eax ebx ecx edx esi edi];

#endif

#else

static void yuv8x2(YUVTBL *yuv, STR_LONG *Yptr,STR_LONG *Cbptr,STR_LONG *Aptr,ARGB *p, int rowbytes)
{
    int rc;
    int gc;
    int bc;
    int Cr;
    int Cb;
    int Y;
    unsigned char *uclip=yuv->ucliptbl+512;

    /* 2x2 0 */

    Cb = gmax(-128,gmin(127,*(Cbptr+0)));
    Cr = gmax(-128,gmin(127,*(Cbptr+64)));
    rc =                           yuv->CrP1p4020[Cr+128];
    gc = yuv->CbN0p3441[Cb+128]  + yuv->CrN0p7141[Cr+128];
    bc = yuv->CbP1p7720[Cb+128];

    Y = *(Yptr+0);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+0)] );

    Y = *(Yptr+1);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+1)] );

    p = (ARGB *) ((char *)p + rowbytes);

    Y = *(Yptr+8);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+8)] );

    Y = *(Yptr+9);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+9)] );

    p = (ARGB *) ((char *)p - rowbytes+8);

    /* 2x2 1 */

    Cb = gmax(-128,gmin(127,*(Cbptr+1)));
    Cr = gmax(-128,gmin(127,*(Cbptr+65)));
    rc =                           yuv->CrP1p4020[Cr+128];
    gc = yuv->CbN0p3441[Cb+128]  + yuv->CrN0p7141[Cr+128];
    bc = yuv->CbP1p7720[Cb+128];

    Y = *(Yptr+2);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+2)] );

    Y = *(Yptr+3);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+3)] );

    p = (ARGB *) ((char *)p + rowbytes);

    Y = *(Yptr+10);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+10)] );

    Y = *(Yptr+11);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+11)] );

    p = (ARGB *) ((char *)p - rowbytes+8);

    /* 2x2 2 */

    Cb = gmax(-128,gmin(127,*(Cbptr+2)));
    Cr = gmax(-128,gmin(127,*(Cbptr+66)));
    rc =                           yuv->CrP1p4020[Cr+128];
    gc = yuv->CbN0p3441[Cb+128]  + yuv->CrN0p7141[Cr+128];
    bc = yuv->CbP1p7720[Cb+128];

    Y = *(Yptr+4);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+4)] );

    Y = *(Yptr+5);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+5)] );

    p = (ARGB *) ((char *)p + rowbytes);

    Y = *(Yptr+12);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+12)] );

    Y = *(Yptr+13);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+13)] );

    p = (ARGB *) ((char *)p - rowbytes+8);


    /* 2x2 3 */

    Cb = gmax(-128,gmin(127,*(Cbptr+3)));
    Cr = gmax(-128,gmin(127,*(Cbptr+67)));
    rc =                           yuv->CrP1p4020[Cr+128];
    gc = yuv->CbN0p3441[Cb+128]  + yuv->CrN0p7141[Cr+128];
    bc = yuv->CbP1p7720[Cb+128];

    Y = *(Yptr+6);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+6)] );

    Y = *(Yptr+7);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+7)] );

    p = (ARGB *) ((char *)p + rowbytes);

    Y = *(Yptr+14);
    p[0].r = uclip[(Y + rc)];
    p[0].g = uclip[(Y + gc)];
    p[0].b = uclip[(Y + bc)];
    p[0].a = gmax( gmax(gmax(p[0].r, p[0].g),p[0].b), uclip[*(Aptr+14)] );

    Y = *(Yptr+15);
    p[1].r = uclip[(Y + rc)];
    p[1].g = uclip[(Y + gc)];
    p[1].b = uclip[(Y + bc)];
    p[1].a = gmax( gmax(gmax(p[1].r, p[1].g),p[1].b), uclip[*(Aptr+15)] );
}

#endif

static void DCT_YUV_RGB(int width,int height,char *Frame,int rowbytes,STR_LONG *DCT_Output, int *QuickYUV, int *dctrowtbl, int alpha)
{
    int     i;
    int     row;
    int     column;
    ARGB    *p;
    YUVTBL  yuv;
    STR_LONG    A1[64];
    STR_LONG    A2[64];
    STR_LONG    A3[64];
    STR_LONG    A4[64];


#if 0
    int  YIndex[1024];
    int  CbIndex[1024];
    int  CrIndex[1024];


    for (i=-512; i<512; ++i)
    {
        int  j;

        j = (i+128+2)/4;
        j = gmax(0,gmin(63,j));
        YIndex[i+512] = j<<12;
        j = (i+128+4)/8;
        j = gmax(0,gmin(31,j));
        CbIndex[i+512] = j<<7;
        j = (i+128+4)/8;
        j = gmax(0,gmin(31,j));
        CrIndex[i+512] = j<<2;
    }
#endif


#ifdef ASMYUV

    {
        extern unsigned int  YUVclip[1024]; /* clipping table */

        extern int CrP1p4020[256];    /*  1.4020*Cr */
        extern int CrN0p7141[256];    /* -0.7141*Cr */
        extern int CbN0p3441[256];    /* -0.3441*Cb */
        extern int CbP1p7720[256];    /*  1.7720*Cb */

        for (i=0; i<256; ++i)
        {
            CrP1p4020[i] = (int) ( 1.4020 * (i-128))*4;
            CrN0p7141[i] = (int) (-0.7141 * (i-128))*4;
            CbN0p3441[i] = (int) (-0.3441 * (i-128))*4;
            CbP1p7720[i] = (int) ( 1.7720 * (i-128))*4;
        }

        for (i= -512; i<512; i++)
        {
            YUVclip[i+512] = (((i<-128) ? 0 : ((i>127) ? 255 : i+128))<<16)|0xff000000;
        }
    }

#else

    for (i=0; i<256; ++i)
    {
        yuv.CrP1p4020[i] = (int) ( 1.4020 * (i-128));
        yuv.CrN0p7141[i] = (int) (-0.7141 * (i-128));
        yuv.CbN0p3441[i] = (int) (-0.3441 * (i-128));
        yuv.CbP1p7720[i] = (int) ( 1.7720 * (i-128));
    }

/*#define uclip ucliptbl+512 */

    for (i= -512; i<512; i++)
        yuv.ucliptbl[i+512] = (i<-128) ? 0 : ((i>127) ? 255 : i+128);

#endif

    /* Clear the A? buffers to 255 */
    for (i=0;i<64;i++)
    {
        A1[i] = 255;
        A2[i] = 255;
        A3[i] = 255;
        A4[i] = 255;
    }

    for (column=0;column<(width>>4);++column)
    {
        for (row=0;row<(height>>4);++row)
        {
            /* iDCT the 8x8 blocks - cr cb y1 y2 y3 y4 order */

            STR_idct(dctrowtbl,DCT_Output+64*0,&yuv.Cr[0]);
            STR_idct(dctrowtbl,DCT_Output+64*1,&yuv.Cb[0]);
            STR_idct(dctrowtbl,DCT_Output+64*2,&yuv.Y1[0]);
            STR_idct(dctrowtbl,DCT_Output+64*3,&yuv.Y2[0]);
            STR_idct(dctrowtbl,DCT_Output+64*4,&yuv.Y3[0]);
            STR_idct(dctrowtbl,DCT_Output+64*5,&yuv.Y4[0]);
            DCT_Output+=64*6;

            /* iDCT the alpha block if they exist - a1 a2 a3 a4 */
            if (alpha)
            {
                STR_idct(dctrowtbl,DCT_Output+64*0,&A1[0]);
                STR_idct(dctrowtbl,DCT_Output+64*1,&A2[0]);
                STR_idct(dctrowtbl,DCT_Output+64*2,&A3[0]);
                STR_idct(dctrowtbl,DCT_Output+64*3,&A4[0]);
                DCT_Output+=64*4;
            }

            p = (ARGB *)(Frame + ((row*16)*rowbytes)) + column*16;
            yuv8x2(&yuv, &yuv.Y1[0],  &yuv.Cb[0+0], &A1[0],  p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y1[16], &yuv.Cb[8+0], &A1[16],  p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y1[32], &yuv.Cb[16+0], &A1[32], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y1[48], &yuv.Cb[24+0], &A1[48], p, rowbytes);

            p = (ARGB *)(Frame + ((row*16)*rowbytes)) + column*16+8;
            yuv8x2(&yuv, &yuv.Y2[0],  &yuv.Cb[0+4], &A2[0],  p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y2[16], &yuv.Cb[8+4], &A2[16],  p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y2[32], &yuv.Cb[16+4], &A2[32], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y2[48], &yuv.Cb[24+4], &A2[48], p, rowbytes);

            p = (ARGB *)(Frame + ((row*16+8)*rowbytes)) + column*16;
            yuv8x2(&yuv, &yuv.Y3[0],  &yuv.Cb[32+0], &A3[0], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y3[16], &yuv.Cb[40+0], &A3[16], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y3[32], &yuv.Cb[48+0], &A3[32], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y3[48], &yuv.Cb[56+0], &A3[48], p, rowbytes);

            p = (ARGB *)(Frame + ((row*16+8)*rowbytes)) + column*16+8;
            yuv8x2(&yuv, &yuv.Y4[0],  &yuv.Cb[32+4], &A4[0], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y4[16], &yuv.Cb[40+4], &A4[16], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y4[32], &yuv.Cb[48+4], &A4[32], p, rowbytes); p = (ARGB *) ((char *) p + (rowbytes<<1));
            yuv8x2(&yuv, &yuv.Y4[48], &yuv.Cb[56+4], &A4[48], p, rowbytes);
        }
    }
}



/* Used by DeQuantizeEncode()                                                */
/* Decode 16bits value into 10bits and place 6bits of data into upper bits */

#define DecodeIt(value) ((value&0x200)?((value&0x3ff)-0x0400):(value & 0x3FF))

/* DeQuantize picture using quality factor                 */

static STR_ULONG DeQuantizeEncode(STR_LONG *DCT_Output,int QFactor,STR_UWORD *Enc1_Output,STR_ULONG length)
{
    unsigned int   zi;

    STR_ULONG r_length=0;
    STR_ULONG blockoffset=0;
    STR_ULONG loop;
    unsigned int ncode;
    int   offset=0;

    unsigned char ZigZag[64] =
    {
      0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
      19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
      42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
      58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
    };

    unsigned char Quantum1[64] =
    {
      0x10,0x10,0x13,0x16,0x1a,0x1b,0x1d,0x22,
      0x10,0x10,0x16,0x18,0x1b,0x1d,0x22,0x25,
      0x13,0x16,0x1a,0x1b,0x1d,0x22,0x22,0x26,
      0x16,0x16,0x1a,0x1b,0x1d,0x22,0x25,0x28,
      0x16,0x1a,0x1b,0x1d,0x20,0x23,0x28,0x30,
      0x1a,0x1b,0x1d,0x20,0x23,0x28,0x30,0x3a,
      0x1a,0x1b,0x1d,0x22,0x26,0x2e,0x38,0x45,
      0x1b,0x1d,0x23,0x26,0x2e,0x38,0x45,0x53,
    };

    int Quantum2[64];
    int i;

    for (i=0; i<64; ++i)
        Quantum2[i] = (int) Quantum1[i] * QFactor;

    memset(DCT_Output+blockoffset,0,length);/*64*sizeof(int)); */

    for (loop=0;loop<length;loop+=256)   /* 64*4=256 bytes per block */
    {

        ncode = Enc1_Output[r_length++];
        if (ncode==0xfe00)  /* end of frame */
            break;

        /* Step one of encoding */

        DCT_Output[blockoffset] = DecodeIt(ncode) *Quantum1[0]/16*2;

        /* force lower right corner to be zero */

        offset = 1;
        ncode = Enc1_Output[r_length++];
        if (ncode!=0xfe00)
        {
            do
            {
                offset += (ncode>>10);  /* zero count */
                zi = (unsigned int) ZigZag[offset];

                /* ncode is signed 10 bit value */

                DCT_Output[blockoffset+zi] = (((int)(short)(ncode<<6)) * Quantum2[zi]) >> 9;

                ++offset;
                ncode = Enc1_Output[r_length++];

            } while (ncode!=0xfe00); /* End of block marker */
        }
        blockoffset+=64;
    }

    return(r_length*2);
}


/* Used by DecodeFinal() */

#ifdef MSDOS

#define ggetmdeclong(p) \
   (((unsigned int)*((unsigned short *) ((char *)(p)+0))<<16) \
  | ((unsigned int)*((unsigned short *) ((char *)(p)+2))<<0 ))

#else

#define ggetmdeclong(p) \
   (((unsigned int)*((unsigned char *)(p)+1)<<24) \
  | ((unsigned int)*((unsigned char *)(p)+0)<<16) \
  | ((unsigned int)*((unsigned char *)(p)+3)<<8 ) \
  | ((unsigned int)*((unsigned char *)(p)+2)<<0 ))

#endif


/* bitstream goes like this: */
/* 8..15  0..7  24..31  16..23 */
/* 7  0   7  0   7  0    7  0 */

/* Add 'n' upper bits of value to output stream                           */
/* Assumes output buffer is initially set to zero                         */

/* Enc2_Output - point to final output buffer                             */
/* value       - data to add to buffer - starts with most significant bit */
/* bitcount    - number of bits to add to output stream                   */

/*#define DecodeComp(Enc2_Output,bitcount,bitpos) ((STR_UWORD)(((ggetmdeclong((char *)Enc2_Output + ((bitpos)>>4)*2)) >> (32-(((bitpos)&15)+(bitcount)))) << (16-(bitcount)))) */


/* Perform final encoding step.  Looks for values in      */
/* standard encoding tables and encodes a shorter version */
/* into the output stream (Huffman coding scheme          */

/* Enc1_Output - pointer to encoded data from step one    */
/* Enc2_Output - pointer to final buffer output           */

#define MDECflushbits(n) \
    next16 <<= (n); \
    bitsleft -= (n); \
    if (bitsleft<0) \
    { \
        next16 = ggetmdeclong(s) << (unsigned char)(-bitsleft); \
        bitsleft += 16; \
        s += 2; \
    }

#define MDECflushngo(n,lab) \
    next16 <<= (n); \
    bitsleft -= (n); \
    if (bitsleft>0) goto lab; \
    next16 = ggetmdeclong(s) << (unsigned char)(-bitsleft); \
    bitsleft += 16; \
    s += 2; \
    goto lab;


static void DecodeFinal(STR_ULONG *Enc2_Output,STR_UWORD *Enc1_Output,
                         signed char *QuickLen1,unsigned int *QuickCode1,
                         signed char *QuickLen2,unsigned int *QuickCode2,
                         signed char *QuickLen3,unsigned int *QuickCode3,
                         signed char *QuickLen4,unsigned int *QuickCode4,
                         int ver,
                         int alpha)
{
    signed char bitsleft;
    signed char i;
    unsigned int next16;
#if MDEC3
    STR_LONG prev_y,prev_cr,prev_cb,block,blocks;
#endif

    unsigned char *s=(unsigned char *)Enc2_Output;

    next16 = ggetmdeclong(Enc2_Output);
    bitsleft = 16;
    s += 2;

#if MDEC3
    prev_y=prev_cr=prev_cb=block=blocks=0;
#endif

nextblock:
    {
        unsigned int value;

        if (ver==3)
        {
            value = (next16>>(32-10));
            if (value==0x3ff)
                goto done;

            /* VLC on DC */
            if (!(block==0||block==1))
            {
                i     =  QuickLen3[next16>>(32-8)];
                value = QuickCode3[next16>>(32-8)];
            }
            else
            {
                i     =  QuickLen4[next16>>(32-8)];
                value = QuickCode4[next16>>(32-8)];
            }
            MDECflushbits(i);
            i = (signed char)value;
            if (i)
            {
                value = (next16>>(32-i));
                if (!(value&(1<<(i-1))))    /* negative (high bit 0) */
                    value = (unsigned int)-((1<<i)-(int)value-1);
            }
            else
                value = 0;

            MDECflushbits(i);

            if (block==0)
            {
                value = (int)(value+prev_cr)<<24>>24;
                prev_cr = value;
            }
            else if (block==1)
            {
                value = (int)(value+prev_cb)<<24>>24;
                prev_cb = value;
            }
            else
            {
                value = (int)(value+prev_y)<<24>>24;
                prev_y = value;
            }

            value = (value<<2)&0x3ff;
            alpha ? (block = (block+1)%YCCA_NUM_BLOCKS) : (block = (block+1)%YCC_NUM_BLOCKS);
            ++blocks;
        }
        else
        {
            value = (next16>>(32-10));

            MDECflushbits(10);
            if (value==0x1ff)
                goto done;
        }

        /* First 10bits - actual value output */
        *Enc1_Output++ = (STR_UWORD) value;

    }
    goto nextcode;


fetchstore:

    *Enc1_Output++ = QuickCode1[next16>>(32-13)];
    next16 = ggetmdeclong(s) << (16-bitsleft);
    s += 2;

nextcode:
    i = QuickLen1[next16>>(32-13)];
    bitsleft -= i;

    if (bitsleft<0)    /* Table 1 */
        goto maybelong;


storeshort:

    *Enc1_Output++ = QuickCode1[next16>>(32-13)];
    next16 <<= (unsigned char) i;

    i = QuickLen1[next16>>(32-13)];
    bitsleft -= i;

    if (bitsleft>=0)    /* Table 1 */
        goto storeshort;

maybelong:

    bitsleft += 16;

    if (bitsleft>=0)
        goto fetchstore;


    bitsleft = bitsleft+i-16;

    if (i==65)  /* Table 2 */
    {
        unsigned int value;

        MDECflushbits(8);

        *Enc1_Output++ = QuickCode2[next16>>(32-9)];
        value = QuickLen2[next16>>(32-9)];
        MDECflushngo((int)value,nextcode);
    }

    if (i==64)     /* End of Block */
    {
        *Enc1_Output++ = 0xfe00;
        MDECflushngo(2,nextblock);
    }

    /* 16 bit literal */

    MDECflushbits(6);
    *Enc1_Output++ = (next16>>16);
    MDECflushngo(16,nextcode);

done:
    /* End of frame marker */

    *Enc1_Output++ = 0xfe00;

    return;
}

static void DeCompressMDEC(char *compBuff,char *dest,int width,int height,int rowbytes,
                           signed char *QuickLen1,unsigned int *QuickCode1,
                           signed char *QuickLen2,unsigned int *QuickCode2,
                           signed char *QuickLen3,unsigned int *QuickCode3,
                           signed char *QuickLen4,unsigned int *QuickCode4,
                           int *QuickYUV, int *DCTTab)
{
    STR_LONG *DCT_Buffer;
    STR_ULONG dct_length;
    STR_ULONG *Enc2_Output;
    STR_UWORD *Enc1_Output;
    int QFactor;
    int ver;
    unsigned int enc1_length;
    int alpha;

    enc1_length = ggeti(compBuff+0,2)*4;
    QFactor =     ggeti(compBuff+4,2);
    ver =         ggeti(compBuff+6,2);  /* version 2 or 3 */

    if (ver & 0x8000) /* Alpha */
    {
        ver &= ~0x8000;
        alpha = 1;
        dct_length=((width>>4)*(height>>4)*YCCA_NUM_BLOCKS*64*4);
    }
    else
    {
        alpha = 0;
        dct_length=((width>>4)*(height>>4)*YCC_NUM_BLOCKS*64*4);
    }

    DCT_Buffer= (STR_LONG *)  galloc(dct_length);
    Enc1_Output=(STR_UWORD *) galloc(dct_length);
    Enc2_Output=(STR_ULONG *) (compBuff+8);

    DecodeFinal(Enc2_Output,Enc1_Output,
                QuickLen1,QuickCode1,
                QuickLen2,QuickCode2,
                QuickLen3,QuickCode3,
                QuickLen4,QuickCode4,
                ver, alpha);

    DeQuantizeEncode(DCT_Buffer,QFactor,Enc1_Output,dct_length);

    DCT_YUV_RGB(width,height,dest,rowbytes,DCT_Buffer,QuickYUV,DCTTab, alpha);

    gfree(Enc1_Output);
    gfree(DCT_Buffer);
}


int DeCompressALPHA(char *alpha, char *dest, int width, int height, int rowbytes)
{

    int qsize;
    unsigned char *alphabuff;
    unsigned char *s;
    ARGB *d;
    int y;
    int x;
    int ok = 0;

    alphabuff = (unsigned char *) galloc(width*height);
    if (alphabuff)
    {

        HUFF_decode(alphabuff, alpha, &qsize);

        s = alphabuff;

        for (y=0; y<height; ++y)
        {
            d = (ARGB*)(dest + y*rowbytes);

            for (x=0; x<width; ++x)
            {
                unsigned char a;

#if 0
                d->a = *s;
                s++;
#else

                /* increase from 4 bit alpha */
                if (x&1)
                {
                    a = *s & 0xF;
                    s++;
                }
                else
                {
                    a = (*s>>4) & 0xF;
                }

                d->a = (unsigned char)(((unsigned int)a*255+8)/0xF);

                /* Make sure to attenuate */
                d->r = gmin(d->r, d->a);
                d->g = gmin(d->g, d->a);
                d->b = gmin(d->b, d->a);

#endif

                d++;
            }
        }
        ok = 1;

        gfree(alphabuff);
    }

    return (ok);
}


int STRreadsector(GINSTANCE *gx, CDSECTOR *d)
{
    STRINFO *str;
    char buf[8];
    int scan4vxa;
    int ok=1;

    str = (STRINFO *) gx->gref;

    if (str->subtype == STR_XA)
    {
        do
        {
            scan4vxa = 0;
            ok = gread(gx->gstream,buf,8);
            if (ok)
            {
                if (ggetm(buf,4)!=0x00014280)
                {
                    scan4vxa = 1;
                    ok = gseek(gx->gstream,gtell(gx->gstream)-8+2336);   /* skip form2\type2 sector */
                }
            }
        } while (scan4vxa && ok);
    }

    if (ok)
        ok = gread(gx->gstream,d,sizeof(CDSECTOR));

    if (str->subtype == STR_XA)
        if (ok)
            ok = gseek(gx->gstream,gtell(gx->gstream)+2336-2048-8); /* skip error correction */
    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL STR_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->dotsubtype      = 1;    /* subtype based on extension */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 8;    /* default pack quality */
        info->movie           = 1;    /* movie file format */
        info->obsolete        = 1;    /* obsolete format */
        strcpy(info->versionstr,    "1.39");                /* version # */
#ifdef MDECCHUNK
        info->signature       = GMAKEID('M','D','E','C');
        info->mactype[0]      = GMAKEID('.','M','D','C');   /* mac file system types used */
        strcpy(info->extensions[0], ".mdc");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->shorttypestr,  "MDC");                 /* type */
        strcpy(info->wordtypestr,   "MDEC");                /* wordtype */
        strcpy(info->longtypestr,   "Sony MDEC Chunk");     /* longtype */
#else
        info->signature       = GMAKEID('.','S','T','R');
        info->mactype[0]      = GMAKEID('.','S','T','R');   /* mac file system types used */
        strcpy(info->extensions[0], ".str");                /* str sector */
        strcpy(info->extensions[1], ".xa");                 /* xa sector */
        strcpy(info->extensions[2], ".xar");                /* refpack */
        strcpy(info->extensions[3], ".dct");                /* dct chunk */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->shorttypestr,  "MDEC");                /* type */
        strcpy(info->wordtypestr,   "MDEC");                /* wordtype */
        strcpy(info->longtypestr,   "MDEC Movie");          /* longtype */
#endif
    }
    return(info);
}

/* check for reasonable header: */
/* id 160 */
/* type 8001 */
/* xa has 8 byte header before id/type */
/* dct has 4 byte "DCTm" code */

int GCALL STR_is(GSTREAM *g)
{
    unsigned char strheader[12];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,strheader,12))
    {
        if (ggetm(strheader,4)==0x60010180   /* str */
         || ggetm(strheader+8,4)==0x60010180 /* xa */
         || ggetm(strheader,4)==GMAKEID('m','T','C','D'))
            ok = 100;
        else if (ggetm(strheader,4)==GMAKEID('S','C','H','l'))
        {
            ok = gseek(g, gtell(g)+ggeti(strheader+4,4)-12);
            if (ok)
            {
                ok = gread(g, strheader, 8);
                if ((ok==8) && ggetm(strheader,4)==GMAKEID('m','T','C','D'))
                    ok = 100;
                else
                    ok = 0;
            }
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

/* openstr seeks to last frame to determine number of frames */

int GCALL STR_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    STRINFO *str;
    CDSECTOR strheader;
    DCTCHUNK dctheader;
    char buf[8];
    int  ok=0;
    int  i;
    int  frames;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('.','S','T','R');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = 0;  /* keep track of which frame were pointing at */

        gseek(g,(GPOS)0);

        gx->gref = galloc(sizeof(STRINFO));
        if (gx->gref)
        {
            str = (STRINFO *) gx->gref;
            memset((void *) str,0,sizeof(STRINFO));


            ok = STRinittables(&str->QuickLen1,&str->QuickCode1,
                               &str->QuickLen2,&str->QuickCode2,
                               &str->QuickLen3,&str->QuickCode3,
                               &str->QuickLen4,&str->QuickCode4,
                               &str->QuickYUV,&str->DCTTab);

            if (ok)
            {

                /* ok = STRreadsector(gx, &strheader); */
                ok = gread(g, buf, 4);
                if (ok) ok = gseek(g,(GPOS)0);

                if (ggetm(buf,4) == GMAKEID('S','C','H','l'))
                {
                    /* Ignore the audio chunk */
                    ok = gread(g, buf, 8);
                    while(ok && (ggetm(buf,4) == GMAKEID('S','C','H','l')))
                    {
                        ok = gseek(g, gtell(g)+ggeti(buf+4,4)-8);
                        if (ok) ok = gread(g, buf, 8);
                    }
                    if (ok) gseek(g, gtell(g)-8);
                }

                switch(ggetm(buf,4))
                {
                    case (GMAKEID('m','T','C','D')):
                        str->subtype = STR_DCT;
                        ok = gread(g, &dctheader, sizeof(DCTCHUNK));
                        str->width           = STRGETFIELD(dctheader.width);
                        str->height          = STRGETFIELD(dctheader.height);
                        str->sectorsPerFrame = (STRGETFIELD(dctheader.frameSize)+2047)/2048;

                        frames = 1;
                        gseek(g,(GPOS)0);
                        gread(g, buf, 8);
                        while(ok)
                        {
                            ok = gseek(g, gtell(g)+ggeti(buf+4,4)-8);
                            if (ok) ok = gread(g, buf, 8);
                            if (ok)
                            {
                                if (ggetm(buf,4) == GMAKEID('m','T','C','D'))
                                    frames++;
                                if (ggetm(buf,4) == GMAKEID('a','T','C','D'))
                                    str->subtype |= STR_DCTA;
                            }
                        }
                        ok = 1;
                        break;

                    case (0x60010180):
                        str->subtype = STR_STR;
                        ok = STRreadsector(gx, &strheader);
                        str->width           = STRGETFIELD(strheader.width);
                        str->height          = STRGETFIELD(strheader.height);
                        str->sectorsPerFrame = STRGETFIELD(strheader.nSectors);

                        frames = (int)glen(g)/(str->sectorsPerFrame*sizeof(CDSECTOR));
                        break;

                    default:
                        str->subtype = STR_XA;
                        ok = STRreadsector(gx, &strheader);
                        str->width           = STRGETFIELD(strheader.width);
                        str->height          = STRGETFIELD(strheader.height);
                        str->sectorsPerFrame = STRGETFIELD(strheader.nSectors);

                        frames = 0;
                        gseek(g,(GPOS)0);
                        while (ok)
                        {
                            for (i=0; (unsigned int)i<str->sectorsPerFrame && ok; ++i)
                                ok = STRreadsector(gx, &strheader);
                            if (ok)
                                ++frames;
                        }
                        ok = 1;

                        break;
                }

                if (ok)
                {
                    str->index = (STR_ULONG *) galloc(frames*sizeof(STR_ULONG));
                    if (str->index)
                        ok = gx->frames = frames;
                    else
                        ok = 0;
                }

                gseek(g,(GPOS)0);
                gx->framenum = -1;
                str->indexmax = -1;
            }
        }
    }
    return(ok);
}

GINFO * GCALL STR_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    STRINFO *str;

    str = (STRINFO *) gx->gref;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('.','S','T','R');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;                 /* Current frame */
        ginfo->width            = str->width;               /* Width of bitmap in pixels */
        ginfo->height           = str->height;              /* Height of bitmap in pixels */
        ginfo->subtype          = str->subtype;             /* Internal format */
        ginfo->packed           = 1;                        /* Type of packing */
        ginfo->quality          = str->sectorsPerFrame;     /* quality measured in sectors per frame ie 8 */
        ginfo->framesize        = str->sectorsPerFrame*2048;/* quality measured in bytes per frame ie 16384 */
        ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
        ginfo->redbits          = 8;    /* Number of bits in red channel */
        ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
        ginfo->bluebits         = 8;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
        ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours  = 0;    /* Number of colours in original indexed palette */
    }

    return(ginfo);
}

int GCALL STR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    STRINFO *str;
    CDSECTOR strheader;
    DCTCHUNK dctheader;
    char *mdecdata=0;
    char *alphadata=0;
    char buf[8];
    int  i;
    int  frame;
    int  ok=0;

    str = (STRINFO *) gx->gref;

    /* Seek to the frame if weve been there before */
    if (str->indexmax >= ginfo->framenum)
    {
        gseek(gx->gstream, str->index[ginfo->framenum]);
        frame = ginfo->framenum-1;
    }
    else if (str->indexmax == -1)
    {
        gseek(gx->gstream, 0);
        frame = -1;
    }
    else
    {
        gseek(gx->gstream, str->index[str->indexmax]);
        frame = str->indexmax-1;
    }


    if (str->subtype & STR_DCT)
    {
        ok = gread(gx->gstream, buf, 8);

        if (ggetm(buf, 4) == GMAKEID('m','T','C','D'))
        {
            frame++;
            str->index[frame] = (unsigned int)gtell(gx->gstream)-8;
            str->indexmax = frame;
        }

        while((frame<ginfo->framenum) && ok)
        {
            ok = gseek(gx->gstream, gtell(gx->gstream)+ggeti(buf+4,4)-8);
            if (ok)
            {
                ok = gread(gx->gstream, buf, 8);

                if (ggetm(buf,4) == GMAKEID('m','T','C','D'))
                {
                    frame++;
                    str->index[frame] = (unsigned int)gtell(gx->gstream)-8;
                    str->indexmax = frame;
                }
            }
        }

        if (ok)
        {
            ok = gseek(gx->gstream, gtell(gx->gstream)-8);
            ok = gread(gx->gstream, &dctheader, sizeof(DCTCHUNK));
            mdecdata = (char *) galloc(STRGETFIELD(dctheader.frameSize)-sizeof(DCTCHUNK));
            ok = gread(gx->gstream, mdecdata, STRGETFIELD(dctheader.frameSize)-sizeof(DCTCHUNK));

            /* Begin reading alpha chunk if it exists */
            if (str->subtype & STR_DCTA)
            {
                ok = gread(gx->gstream, buf, 8);
                if ( (ggetm(buf,4) == GMAKEID('a','T','C','D')) && ok)
                {
                    alphadata = (char *) galloc(ggeti(buf+4, 4));
                    ok = gread(gx->gstream, alphadata, ggeti(buf+4, 4)-8);
                }
            }

            gx->framenum = ginfo->framenum+1;
        }
    }
    else
    {
        mdecdata = (char *) galloc(str->sectorsPerFrame*USRSIZE*4);
        if (mdecdata)
        {
            ok = 1;

            if (str->subtype == STR_XA)
            {
                frame++;
                str->index[frame] = (unsigned int)gtell(gx->gstream);
                str->indexmax = frame;
                while ((frame < ginfo->framenum) && ok)
                {
                    for (i=0; (unsigned int)i<str->sectorsPerFrame && ok; ++i)
                        ok = STRreadsector(gx, &strheader);
                    frame++;
                    str->index[frame] = (unsigned int)gtell(gx->gstream);
                    str->indexmax = frame;
                }
            }
            else
                ok = gseek(gx->gstream, (ginfo->framenum*str->sectorsPerFrame)*sizeof(CDSECTOR));

            for (i=0; (unsigned int)i<str->sectorsPerFrame && ok; ++i)
            {
                ok = STRreadsector(gx, &strheader);
                memcpy(mdecdata+i*USRSIZE*4,&strheader.data,USRSIZE*4);
            }
            gx->framenum = ginfo->framenum+1;
        }
    }


    if (ok && mdecdata)
    {
        DeCompressMDEC(mdecdata,dest,ginfo->width,ginfo->height,rowbytes,
                       str->QuickLen1,str->QuickCode1,
                       str->QuickLen2,str->QuickCode2,
                       str->QuickLen3,str->QuickCode3,
                       str->QuickLen4,str->QuickCode4,
                       str->QuickYUV,str->DCTTab);

        if (alphadata)
        {
            ok = DeCompressALPHA(alphadata, dest, ginfo->width, ginfo->height, rowbytes);
        }
    }

    if (mdecdata) gfree(mdecdata);
    if (alphadata) gfree(alphadata);

    return(ok);
}

int GCALL STR_close(GINSTANCE *gx)
{
    STRINFO *str;
    int ok=0;

    if (gx)
    {
        if (gx->gref)
        {
            str = (STRINFO *) gx->gref;
            STRfreetables(&str->QuickLen1,&str->QuickCode1,
                          &str->QuickLen2,&str->QuickCode2,
                          &str->QuickLen3,&str->QuickCode3,
                          &str->QuickLen4,&str->QuickCode4,
                          &str->QuickYUV,&str->DCTTab);
            if (str->index) gfree(str->index);
            gfree((void *)gx->gref);
            gx->gref = 0;
        }
        ok = gfree((void *) gx);
    }

    return(ok);
}

#ifdef MDECCHUNK

#undef aboutstr
#undef isstr
#undef openstr
#undef infostr
#undef readstr
#undef closestr

#endif


