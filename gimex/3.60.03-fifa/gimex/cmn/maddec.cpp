// (c) 1997-2007 Electronic Arts Inc.

#include "madgimex.h"
#include "maddec.h"

#define fixedmult(a,b) ((int)((double)(a) * (double)(b) / 65536.0))


/* Encoding table used by the MADCOW encoder. */
/* This table contains all the MPEG bit patterns with less */
/* than 14 bits except for the escape code (000001). */
/* Entries are bit count, value to encode, unused, bit pattern. */
static unsigned int encodetbl1[95*4] =
{
    0x02, 0xfe00, 0x00, 0x8000,    /* end of block */
    0x03, 0x0001, 0x00, 0xc000,
    0x03, 0x03ff, 0x00, 0xe000,
    0x04, 0x0401, 0x00, 0x6000,
    0x04, 0x07ff, 0x00, 0x7000,
    0x05, 0x0002, 0x00, 0x4000,
    0x05, 0x03fe, 0x00, 0x4800,
    0x05, 0x0801, 0x00, 0x5000,
    0x05, 0x0bff, 0x00, 0x5800,
    0x06, 0x0003, 0x00, 0x2800,
    0x06, 0x03fd, 0x00, 0x2c00,
    0x06, 0x0c01, 0x00, 0x3800,
    0x06, 0x0fff, 0x00, 0x3c00,
    0x06, 0x1001, 0x00, 0x3000,
    0x06, 0x13ff, 0x00, 0x3400,
    0x07, 0x0402, 0x00, 0x1800,
    0x07, 0x07fe, 0x00, 0x1a00,
    0x07, 0x1401, 0x00, 0x1c00,
    0x07, 0x17ff, 0x00, 0x1e00,
    0x07, 0x1801, 0x00, 0x1400,
    0x07, 0x1bff, 0x00, 0x1600,
    0x07, 0x1c01, 0x00, 0x1000,
    0x07, 0x1fff, 0x00, 0x1200,
    0x08, 0x0004, 0x00, 0x0c00,
    0x08, 0x03fc, 0x00, 0x0d00,
    0x08, 0x0802, 0x00, 0x0800,
    0x08, 0x0bfe, 0x00, 0x0900,
    0x08, 0x2001, 0x00, 0x0e00,
    0x08, 0x23ff, 0x00, 0x0f00,
    0x08, 0x2401, 0x00, 0x0a00,
    0x08, 0x27ff, 0x00, 0x0b00,
    0x09, 0x0005, 0x00, 0x2600,
    0x09, 0x03fb, 0x00, 0x2680,
    0x09, 0x0006, 0x00, 0x2100,
    0x09, 0x03fa, 0x00, 0x2180,
    0x09, 0x0403, 0x00, 0x2500,
    0x09, 0x07fd, 0x00, 0x2580,
    0x09, 0x0c02, 0x00, 0x2400,
    0x09, 0x0ffe, 0x00, 0x2480,
    0x09, 0x2801, 0x00, 0x2700,
    0x09, 0x2bff, 0x00, 0x2780,
    0x09, 0x2c01, 0x00, 0x2300,
    0x09, 0x2fff, 0x00, 0x2380,
    0x09, 0x3001, 0x00, 0x2200,
    0x09, 0x33ff, 0x00, 0x2280,
    0x09, 0x3401, 0x00, 0x2000,
    0x09, 0x37ff, 0x00, 0x2080,
    0x0b, 0x0007, 0x00, 0x0280,
    0x0b, 0x03f9, 0x00, 0x02a0,
    0x0b, 0x0404, 0x00, 0x0300,
    0x0b, 0x07fc, 0x00, 0x0320,
    0x0b, 0x0803, 0x00, 0x02c0,
    0x0b, 0x0bfd, 0x00, 0x02e0,
    0x0b, 0x1002, 0x00, 0x03c0,
    0x0b, 0x13fe, 0x00, 0x03e0,
    0x0b, 0x1402, 0x00, 0x0240,
    0x0b, 0x17fe, 0x00, 0x0260,
    0x0b, 0x3801, 0x00, 0x0380,
    0x0b, 0x3bff, 0x00, 0x03a0,
    0x0b, 0x3c01, 0x00, 0x0340,
    0x0b, 0x3fff, 0x00, 0x0360,
    0x0b, 0x4001, 0x00, 0x0200,
    0x0b, 0x43ff, 0x00, 0x0220,
    0x0d, 0x0008, 0x00, 0x01d0,
    0x0d, 0x03f8, 0x00, 0x01d8,
    0x0d, 0x0009, 0x00, 0x0180,
    0x0d, 0x03f7, 0x00, 0x0188,
    0x0d, 0x000a, 0x00, 0x0130,
    0x0d, 0x03f6, 0x00, 0x0138,
    0x0d, 0x000b, 0x00, 0x0100,
    0x0d, 0x03f5, 0x00, 0x0108,
    0x0d, 0x0405, 0x00, 0x01b0,
    0x0d, 0x07fb, 0x00, 0x01b8,
    0x0d, 0x0804, 0x00, 0x0140,
    0x0d, 0x0bfc, 0x00, 0x0148,
    0x0d, 0x0c03, 0x00, 0x01c0,
    0x0d, 0x0ffd, 0x00, 0x01c8,
    0x0d, 0x1003, 0x00, 0x0120,
    0x0d, 0x13fd, 0x00, 0x0128,
    0x0d, 0x1802, 0x00, 0x01e0,
    0x0d, 0x1bfe, 0x00, 0x01e8,
    0x0d, 0x1c02, 0x00, 0x0150,
    0x0d, 0x1ffe, 0x00, 0x0158,
    0x0d, 0x2002, 0x00, 0x0110,
    0x0d, 0x23fe, 0x00, 0x0118,
    0x0d, 0x4401, 0x00, 0x01f0,
    0x0d, 0x47ff, 0x00, 0x01f8,
    0x0d, 0x4801, 0x00, 0x01a0,
    0x0d, 0x4bff, 0x00, 0x01a8,
    0x0d, 0x4c01, 0x00, 0x0190,
    0x0d, 0x4fff, 0x00, 0x0198,
    0x0d, 0x5001, 0x00, 0x0170,
    0x0d, 0x53ff, 0x00, 0x0178,
    0x0d, 0x5401, 0x00, 0x0160,
    0x0d, 0x57ff, 0x00, 0x0168
};

/* Encoding table used by the MADCOW encoder. */
/* This table contains all the MPEG bit patterns with 14 or more bits. */
/* The first eight bits of each pattern are zeros and are not counted. */
/* Entries are bit count, value to encode, unused, bit pattern. */
static unsigned int encodetbl2[128*4] =
{
    0x06, 0x000c, 0x00, 0xd000,
    0x06, 0x03f4, 0x00, 0xd400,
    0x06, 0x000d, 0x00, 0xc800,
    0x06, 0x03f3, 0x00, 0xcc00,
    0x06, 0x000e, 0x00, 0xc000,
    0x06, 0x03f2, 0x00, 0xc400,
    0x06, 0x000f, 0x00, 0xb800,
    0x06, 0x03f1, 0x00, 0xbc00,
    0x06, 0x0406, 0x00, 0xb000,
    0x06, 0x07fa, 0x00, 0xb400,
    0x06, 0x0407, 0x00, 0xa800,
    0x06, 0x07f9, 0x00, 0xac00,
    0x06, 0x0805, 0x00, 0xa000,
    0x06, 0x0bfb, 0x00, 0xa400,
    0x06, 0x0c04, 0x00, 0x9800,
    0x06, 0x0ffc, 0x00, 0x9c00,
    0x06, 0x1403, 0x00, 0x9000,
    0x06, 0x17fd, 0x00, 0x9400,
    0x06, 0x2402, 0x00, 0x8800,
    0x06, 0x27fe, 0x00, 0x8c00,
    0x06, 0x2802, 0x00, 0x8000,
    0x06, 0x2bfe, 0x00, 0x8400,
    0x06, 0x5801, 0x00, 0xf800,
    0x06, 0x5bff, 0x00, 0xfc00,
    0x06, 0x5c01, 0x00, 0xf000,
    0x06, 0x5fff, 0x00, 0xf400,
    0x06, 0x6001, 0x00, 0xe800,
    0x06, 0x63ff, 0x00, 0xec00,
    0x06, 0x6401, 0x00, 0xe000,
    0x06, 0x67ff, 0x00, 0xe400,
    0x06, 0x6801, 0x00, 0xd800,
    0x06, 0x6bff, 0x00, 0xdc00,
    0x07, 0x0010, 0x00, 0x7c00,
    0x07, 0x03f0, 0x00, 0x7e00,
    0x07, 0x0011, 0x00, 0x7800,
    0x07, 0x03ef, 0x00, 0x7a00,
    0x07, 0x0012, 0x00, 0x7400,
    0x07, 0x03ee, 0x00, 0x7600,
    0x07, 0x0013, 0x00, 0x7000,
    0x07, 0x03ed, 0x00, 0x7200,
    0x07, 0x0014, 0x00, 0x6c00,
    0x07, 0x03ec, 0x00, 0x6e00,
    0x07, 0x0015, 0x00, 0x6800,
    0x07, 0x03eb, 0x00, 0x6a00,
    0x07, 0x0016, 0x00, 0x6400,
    0x07, 0x03ea, 0x00, 0x6600,
    0x07, 0x0017, 0x00, 0x6000,
    0x07, 0x03e9, 0x00, 0x6200,
    0x07, 0x0018, 0x00, 0x5c00,
    0x07, 0x03e8, 0x00, 0x5e00,
    0x07, 0x0019, 0x00, 0x5800,
    0x07, 0x03e7, 0x00, 0x5a00,
    0x07, 0x001a, 0x00, 0x5400,
    0x07, 0x03e6, 0x00, 0x5600,
    0x07, 0x001b, 0x00, 0x5000,
    0x07, 0x03e5, 0x00, 0x5200,
    0x07, 0x001c, 0x00, 0x4c00,
    0x07, 0x03e4, 0x00, 0x4e00,
    0x07, 0x001d, 0x00, 0x4800,
    0x07, 0x03e3, 0x00, 0x4a00,
    0x07, 0x001e, 0x00, 0x4400,
    0x07, 0x03e2, 0x00, 0x4600,
    0x07, 0x001f, 0x00, 0x4000,
    0x07, 0x03e1, 0x00, 0x4200,
    0x08, 0x0020, 0x00, 0x3000,
    0x08, 0x03e0, 0x00, 0x3100,
    0x08, 0x0021, 0x00, 0x2e00,
    0x08, 0x03df, 0x00, 0x2f00,
    0x08, 0x0022, 0x00, 0x2c00,
    0x08, 0x03de, 0x00, 0x2d00,
    0x08, 0x0023, 0x00, 0x2a00,
    0x08, 0x03dd, 0x00, 0x2b00,
    0x08, 0x0024, 0x00, 0x2800,
    0x08, 0x03dc, 0x00, 0x2900,
    0x08, 0x0025, 0x00, 0x2600,
    0x08, 0x03db, 0x00, 0x2700,
    0x08, 0x0026, 0x00, 0x2400,
    0x08, 0x03da, 0x00, 0x2500,
    0x08, 0x0027, 0x00, 0x2200,
    0x08, 0x03d9, 0x00, 0x2300,
    0x08, 0x0028, 0x00, 0x2000,
    0x08, 0x03d8, 0x00, 0x2100,
    0x08, 0x0408, 0x00, 0x3e00,
    0x08, 0x07f8, 0x00, 0x3f00,
    0x08, 0x0409, 0x00, 0x3c00,
    0x08, 0x07f7, 0x00, 0x3d00,
    0x08, 0x040a, 0x00, 0x3a00,
    0x08, 0x07f6, 0x00, 0x3b00,
    0x08, 0x040b, 0x00, 0x3800,
    0x08, 0x07f5, 0x00, 0x3900,
    0x08, 0x040c, 0x00, 0x3600,
    0x08, 0x07f4, 0x00, 0x3700,
    0x08, 0x040d, 0x00, 0x3400,
    0x08, 0x07f3, 0x00, 0x3500,
    0x08, 0x040e, 0x00, 0x3200,
    0x08, 0x07f2, 0x00, 0x3300,
    0x09, 0x040f, 0x00, 0x1300,
    0x09, 0x07f1, 0x00, 0x1380,
    0x09, 0x0410, 0x00, 0x1200,
    0x09, 0x07f0, 0x00, 0x1280,
    0x09, 0x0411, 0x00, 0x1100,
    0x09, 0x07ef, 0x00, 0x1180,
    0x09, 0x0412, 0x00, 0x1000,
    0x09, 0x07ee, 0x00, 0x1080,
    0x09, 0x1803, 0x00, 0x1400,
    0x09, 0x1bfd, 0x00, 0x1480,
    0x09, 0x2c02, 0x00, 0x1a00,
    0x09, 0x2ffe, 0x00, 0x1a80,
    0x09, 0x3002, 0x00, 0x1900,
    0x09, 0x33fe, 0x00, 0x1980,
    0x09, 0x3402, 0x00, 0x1800,
    0x09, 0x37fe, 0x00, 0x1880,
    0x09, 0x3802, 0x00, 0x1700,
    0x09, 0x3bfe, 0x00, 0x1780,
    0x09, 0x3c02, 0x00, 0x1600,
    0x09, 0x3ffe, 0x00, 0x1680,
    0x09, 0x4002, 0x00, 0x1500,
    0x09, 0x43fe, 0x00, 0x1580,
    0x09, 0x6c01, 0x00, 0x1f00,
    0x09, 0x6fff, 0x00, 0x1f80,
    0x09, 0x7001, 0x00, 0x1e00,
    0x09, 0x73ff, 0x00, 0x1e80,
    0x09, 0x7401, 0x00, 0x1d00,
    0x09, 0x77ff, 0x00, 0x1d80,
    0x09, 0x7801, 0x00, 0x1c00,
    0x09, 0x7bff, 0x00, 0x1c80,
    0x09, 0x7c01, 0x00, 0x1b00,
    0x09, 0x7fff, 0x00, 0x1b80
};

/* MPEG quantization matrix. */
static int quanttbl[64] =
{
    0x08, 0x10, 0x13, 0x16, 0x1a, 0x1b, 0x1d, 0x22,
    0x10, 0x10, 0x16, 0x18, 0x1b, 0x1d, 0x22, 0x25,
    0x13, 0x16, 0x1a, 0x1b, 0x1d, 0x22, 0x22, 0x26,
    0x16, 0x16, 0x1a, 0x1b, 0x1d, 0x22, 0x25, 0x28,
    0x16, 0x1a, 0x1b, 0x1d, 0x20, 0x23, 0x28, 0x30,
    0x1a, 0x1b, 0x1d, 0x20, 0x23, 0x28, 0x30, 0x3a,
    0x1a, 0x1b, 0x1d, 0x22, 0x26, 0x2e, 0x38, 0x45,
    0x1b, 0x1d, 0x23, 0x26, 0x2e, 0x38, 0x45, 0x53,
};

extern "C"
{
/* Shift register for MADCOW bitstream. */
unsigned short  *maddataptr;
unsigned int   madshiftreg;
int             madbitcount;

/* Decoding tables. */
char            madclipbiastbl[256];
unsigned int   madvlctbl1[512];
unsigned int   madvlctbl2[256];
unsigned int   madvlctbl3[256];
unsigned int   madvlctbl4[64];
int            madquant[64];
}

/* Work variables. */
static int  initflag;
static int  motionframe;
static int luma[16*16];
static int chroma[2*8*8];

/*
;
; Initializes the tables used by the MADCOW decoder.
;
; The clip-bias table is used to convert IDCT results to YUVPAIR entries.
; It clips a value in the range -128..127 to -64..63 and adds 64 to it.
;
; The VLC decoding tables are used to extract values from the MADCOW
; bitstream. Each table entry contains a 10 bit signed value in bits
; 22..31, a 6 bit run length value in bits 16..21 and a bit count in
; bits 0..7. The bit count specifies the number of bits to shift out.
; The following escape values are defined for the bit count:
;
;   00001111 - first 9 bits are zero: shift them out and use madvlctbl2
;   00011111 - first 6 bits are zero: shift them out and use madvlctbl3
;   00101111 - escape: shift out 6 bits and read the 16 bit value
;   00111111 - EOB: shift out 2 bits
;
; The first three tables are used to decode AC coefficients using MPEG
; bit encoding. The primary table examines the first 9 bits of the shift
; register. Depending on the number of leading zeros, one of the other
; two tables will be used to examine the remaining bits.
;
; The fourth table is used to extract delta values. A delta value is a
; number in the range -16..16 and is encoded as follows:
;
;   0       - 0
;   1snnnn  - (n+1) if s=0, (n-16) if s=1
;
; Delta values are used for motion vectors and DC corrections. The run
; length field is not used with delta values.
;
*/

static void madinit(void)
{
    int         bits;
    int         val;
    int         vlc;
    int         prefix;
    int         count;
    int         i,j;

    /* Initialize the clip-bias table. */
    for (i = -128; i < 127; ++i)
    {
        val = i;
        if (val < -64) val = -64;
        else if (val > 63) val = 63;

        madclipbiastbl[i & 0xff] = val + 64;
    }

    /* Add the escape sequences to the primary decoding table. */
    madvlctbl1[0x000] = 0x0f;                               /* 000000000 (table 2) */
    for (i = 1; i < 8 ; ++i) madvlctbl1[0x000+i] = 0x1f;    /* 000000xxx (table 3) */
    for (i = 0; i < 8 ; ++i) madvlctbl1[0x008+i] = 0x2f;    /* 000001xxx (escape) */
    for (i = 0; i < 128 ; ++i) madvlctbl1[0x100+i] = 0x3f;  /* 01xxxxxxx (EOB) */

    /* Add the values from encodetbl1 (except EOB) to the decoding tables. */
    for (i = 1; i < 95; ++i)
    {
        bits = encodetbl1[4*i+0];
        val  = encodetbl1[4*i+1];
        vlc  = encodetbl1[4*i+3];

        if ((vlc & 0xfc00) != 0)
        {
            prefix = vlc >> (16-9);
            count = 1 << (9-bits);
            val = ((val & 0x3ff) << (32-10)) | ((val & 0xfc00) << 6) | bits;
            for (j = 0; j < count; ++j) madvlctbl1[prefix+j] = val;
        }
        else
        {
            prefix = vlc >> ((16-6)-8);
            count = 1 << (8-(bits-6));
            val = ((val & 0x3ff) << (32-10)) | ((val & 0xfc00) << 6) | (bits-6);
            for (j = 0; j < count; ++j) madvlctbl3[prefix+j] = val;
        }
    }

    /* Add the values from encodetbl2 to the decoding tables. */
    for (i = 0; i < 128; ++i)
    {
        bits = encodetbl2[4*i+0] + 8;
        val  = encodetbl2[4*i+1];
        vlc  = encodetbl2[4*i+3];

        if ((vlc & 0x8000) == 0)
        {
            prefix = vlc >> ((24-9)-8);
            count = 1 << (8-(bits-9));
            val = ((val & 0x3ff) << (32-10)) | ((val & 0xfc00) << 6) | (bits-9);
            for (j = 0; j < count; ++j) madvlctbl2[prefix+j] = val;
        }
        else
        {
            prefix = vlc >> ((24-6)-8);
            count = 1 << (8-(bits-6));
            val = ((val & 0x3ff) << (32-10)) | ((val & 0xfc00) << 6) | (bits-6);
            for (j = 0; j < count; ++j) madvlctbl3[prefix+j] = val;
        }
    }

    /* Initialize the delta decoding table. */
    for (i = 0; i < 32 ; ++i) madvlctbl4[0x00+i] = 1;
    for (i = 0; i < 16 ; ++i) madvlctbl4[0x20+i] = ((i+1) << (32-10)) | 6;
    for (i = 0; i < 16 ; ++i) madvlctbl4[0x30+i] = ((i-16) << (32-10)) | 6;

    /* Don't initialize again. */
    initflag = 1;
}

/*
;
; Discards a specified number of bits from the shift register.
;
*/

static void discardbits(int bits)
{
    madshiftreg <<= bits;
    madbitcount -= bits;

    if (madbitcount < 16)
    {
        madshiftreg |= *maddataptr << (16-madbitcount);
        madbitcount += 16;
        ++maddataptr;
    }
}

/*
;
; Reads a delta value (-16..16) from the bitstream.
;
*/

static int getdelta(void)
{
    int     val;

    val = madvlctbl4[madshiftreg >> (32-6)];
    discardbits(val & 0xff);
    return(val >> (32-10));
}

/*
;
; Performs an IDCT on a block that is known not to have any AC coefficients.
; Simply copies the DC coefficient to all elements of the output matrix.
;
*/

static void dcblock(int *dest, int stride)
{
    int     i;

    for (i = 0; i < 8; ++i)
    {
        dest[0] = idctinput[0];
        dest[1] = idctinput[0];
        dest[2] = idctinput[0];
        dest[3] = idctinput[0];
        dest[4] = idctinput[0];
        dest[5] = idctinput[0];
        dest[6] = idctinput[0];
        dest[7] = idctinput[0];

        dest += stride;
    }
}

/*
;
; Moves an 8x8 block of luminance values from a YUV frame buffer to a
; section of the luminance matrix, applying a fixed correction to each
; entry. Parameter src specifies the YUVPAIR containing the upper left
; luminance value. Parameter srcodd specifies whether to start with the
; even or odd luminance value within the pair. Parameter stride contains
; the width of the YUV buffer in macropixels. Parameter dest specifies
; the position within the luminance matrix to store the upper left value.
;
; The corrected luminance values are written to bits 16..23 of each
; entry within the luminance matrix. The remaining bits do not need to
; be updated since they are not read when converting back to YUVPAIRs.
; Note that the correction value includes the DC correction from the
; bitstream as well as a bias of -64 to compensate for the bias
; applied when converting to a YUVPAIR.
;
*/

static void moveluma(YUVPAIR *src, int srcodd, int stride, int *dest, int correction)
{
    unsigned char   *bytesrc;
    signed char     *bytedest;
    int             i;

    bytesrc = (unsigned char*) src;
    bytedest = (signed char*) dest;
    stride *= 4;

    if (srcodd)
    {
        for (i = 0; i < 8; ++i)
        {
            bytedest[0*4+2] = (int) bytesrc[0*4+2] + correction;
            bytedest[1*4+2] = (int) bytesrc[1*4+3] + correction;
            bytedest[2*4+2] = (int) bytesrc[1*4+2] + correction;
            bytedest[3*4+2] = (int) bytesrc[2*4+3] + correction;
            bytedest[4*4+2] = (int) bytesrc[2*4+2] + correction;
            bytedest[5*4+2] = (int) bytesrc[3*4+3] + correction;
            bytedest[6*4+2] = (int) bytesrc[3*4+2] + correction;
            bytedest[7*4+2] = (int) bytesrc[4*4+3] + correction;

            bytesrc += stride;
            bytedest += 4*16;
        }
    }
    else
    {
        for (i = 0; i < 8; ++i)
        {
            bytedest[0*4+2] = (int) bytesrc[0*4+3] + correction;
            bytedest[1*4+2] = (int) bytesrc[0*4+2] + correction;
            bytedest[2*4+2] = (int) bytesrc[1*4+3] + correction;
            bytedest[3*4+2] = (int) bytesrc[1*4+2] + correction;
            bytedest[4*4+2] = (int) bytesrc[2*4+3] + correction;
            bytedest[5*4+2] = (int) bytesrc[2*4+2] + correction;
            bytedest[6*4+2] = (int) bytesrc[3*4+3] + correction;
            bytedest[7*4+2] = (int) bytesrc[3*4+2] + correction;

            bytesrc += stride;
            bytedest += 4*16;
        }
    }
}

/*
;
; Moves an 8x8 block of chrominance values from a YUV frame buffer to
; one of the chrominance matrices, applying a fixed correction to each
; entry. Parameter src specifies the YUVPAIR containing the upper left
; chrominance value. Parameter offset specifies which chrominance byte
; to use within the pair (0=Cr, 1=Cb). Parameter stride contains the
; width of the YUV buffer in macropixels. Parameter dest specifies the
; upper left entry of the chrominance matrix to use.
;
; The corrected chrominance values are written to bits 16..23 of each
; entry within the chrominance matrix. The remaining bits do not need to
; be updated since they are not read when converting back to YUVPAIRs.
; Note that the correction value includes the DC correction from the
; bitstream as well as a bias of -64 to compensate for the bias
; applied when converting to a YUVPAIR.
;
*/

static void movechroma(YUVPAIR *src, int offset, int stride, int *dest, int correction)
{
    unsigned char   *bytesrc;
    signed char     *bytedest;
    int             i;

    bytesrc = ((unsigned char*) src) + offset;
    bytedest = (signed char*) dest;
    stride *= 2*4;

    for (i = 0; i < 8; ++i)
    {
        bytedest[0*4+2] = (int) bytesrc[0*4] + correction;
        bytedest[1*4+2] = (int) bytesrc[1*4] + correction;
        bytedest[2*4+2] = (int) bytesrc[2*4] + correction;
        bytedest[3*4+2] = (int) bytesrc[3*4] + correction;
        bytedest[4*4+2] = (int) bytesrc[4*4] + correction;
        bytedest[5*4+2] = (int) bytesrc[5*4] + correction;
        bytedest[6*4+2] = (int) bytesrc[6*4] + correction;
        bytedest[7*4+2] = (int) bytesrc[7*4] + correction;

        bytesrc += stride;
        bytedest += 4*8;
    }
}

/*
;
; UNABSTRACT
;
; madinitdecode - initialize decoding of a MADCOW bitstream
;
;
; SUMMARY
;
; #include "eacmad.h"
;
; void madinitdecode(src, motion, quality)
; unsigned short *src;  bitstream
; int motion;           0=key frame, 1=motion/extra frame
; int quality;          quality factor
;
;
; DESCRIPTION
;
; Initializes decoding of the specified MADCOW bitstream. Parameter
; motion specifies whether the frame is encoded as a key frame or a
; motion frame (extra frames are coded like motion frames). The quality
; factor must be the same value used when encoding the frame.
;
;
; SEE ALSO
;
; maddecodemacroblock
;
;
; END ABSTRACT
;
*/

void madinitdecode(unsigned short *src, int motion, int quality)
{
    int     i;

    if (!initflag) madinit();

    madshiftreg = (src[0] << 16) | src[1];
    madbitcount = 32;
    maddataptr = src+2;

    motionframe = motion;

    /* DC quantization value = quanttbl[0] */
    /* We also fold in the idct prescale value and divide by 2 to scale */
    /* the output to -64..64. */
    madquant[0] = fixedmult(quanttbl[0] << 15, idctprescale[0]);

    for (i = 1; i < 64; ++i)
    {
        /* AC quantization value = quality*quanttbl[i]\8 */
        /* We also fold in the idct prescale value and divide by 2 to scale */
        /* the output to -64..64. */
        madquant[i] = fixedmult((quality*quanttbl[i]) << 12, idctprescale[i]);
    }
}

/*
;
; UNABSTRACT
;
; maddecodemacroblock - decode one macroblock of MADCOW data
;
;
; SUMMARY
;
; #include "eacmad.h"
;
; void maddecodemacroblock(YUVPAIR *src, YUVPAIR *dest, int width)
; YUVPAIR *src;     upper left corner of macroblock in source YUV buffer
; YUVPAIR *dest;    upper left corner of macroblock in destination YUV buffer
; int width;        width of YUV buffers in pixels
;
;
; DESCRIPTION
;
; Decodes one macroblock worth of data from the MADCOW bitstream passed
; to madinitdecode() and outputs the resulting 16x16 pixel macroblock
; to the specified YUV buffer. Macroblocks run from left to right and
; from top to bottom in the MADCOW format. For motion frames, parameter
; src specifies the macroblock in the motion reference frame that
; occupies to same position as the destination macroblock.
;
;
; SEE ALSO
;
; madinitdecode
;
;
; END ABSTRACT
;
*/

void maddecodemacroblock(YUVPAIR *src, YUVPAIR *dest, int width)
{
    int     flags;
    int     dx,dy;
    int     srcodd=0;
    int     index;
    int     correction;

    width >>= 1;

    /* Get the motion flags and motion vector. */
    if (!motionframe)
    {
        /* Motion flags are implied cleared for key frames. */
        flags = 0;
    }
    else
    {
        if ((madshiftreg & 0xc0000000) == 0)
        {
            /* Bit pattern 00 indicates all flags cleared. */
            flags = 0;
            discardbits(2);
        }
        else
        {
            if ((madshiftreg & 0x80000000) != 0)
            {
                /* Bit pattern 1 indicates all flags set. */
                flags = 0x3ff;
                discardbits(1);
            }
            else
            {
                /* Bit pattern 01ffffff explicitly codes the flags. */
                flags = madshiftreg >> (32-8);
                discardbits(8);
            }

            /* Get the motion vector and adjust the source address. */
            dx = getdelta();
            dy = getdelta();
            src += dy*width + (dx >> 1);
            srcodd = dx & 1;
        }
    }

    /* Decode Y1. */
    if ((flags & 0x01) == 0)
    {
        index = madvlcdecode();
        if (index == 1) dcblock(luma, 16);
        else idctcompute(luma, 16);
    }
    else
    {
        correction = getdelta() - 64;
        moveluma(src, srcodd, width, luma, correction);
    }

    /* Decode Y2. */
    if ((flags & 0x02) == 0)
    {
        index = madvlcdecode();
        if (index == 1) dcblock(luma+8, 16);
        else idctcompute(luma+8, 16);
    }
    else
    {
        correction = getdelta() - 64;
        moveluma(src+4, srcodd, width, luma+8, correction);
    }

    /* Decode Y3. */
    if ((flags & 0x04) == 0)
    {
        index = madvlcdecode();
        if (index == 1) dcblock(luma+8*16, 16);
        else idctcompute(luma+8*16, 16);
    }
    else
    {
        correction = getdelta() - 64;
        moveluma(src+8*width, srcodd, width, luma+8*16, correction);
    }

    /* Decode Y4. */
    if ((flags & 0x08) == 0)
    {
        index = madvlcdecode();
        if (index == 1) dcblock(luma+8*16+8, 16);
        else idctcompute(luma+8*16+8, 16);
    }
    else
    {
        correction = getdelta() - 64;
        moveluma(src+8*width+4, srcodd, width, luma+8*16+8, correction);
    }

    /* Decode Cb. */
    if ((flags & 0x10) == 0)
    {
        index = madvlcdecode();
        if (index == 1) dcblock(chroma, 8);
        else idctcompute(chroma, 8);
    }
    else
    {
        correction = getdelta() - 64;
        movechroma(src, 1, width, chroma, correction);
    }

    /* Decode Cr. */
    if ((flags & 0x20) == 0)
    {
        index = madvlcdecode();
        if (index == 1) dcblock(chroma+64, 8);
        else idctcompute(chroma+64, 8);
    }
    else
    {
        correction = getdelta() - 64;
        movechroma(src, 0, width, chroma+64, correction);
    }

    /* Convert the luminance and chrominance values to macropixels */
    /* and output to the YUV buffer. */
    madoutputrow(luma+ 0*16, chroma+0*8, dest);
    dest += width;
    madoutputrow(luma+ 1*16, chroma+0*8, dest);
    dest += width;
    madoutputrow(luma+ 2*16, chroma+1*8, dest);
    dest += width;
    madoutputrow(luma+ 3*16, chroma+1*8, dest);
    dest += width;
    madoutputrow(luma+ 4*16, chroma+2*8, dest);
    dest += width;
    madoutputrow(luma+ 5*16, chroma+2*8, dest);
    dest += width;
    madoutputrow(luma+ 6*16, chroma+3*8, dest);
    dest += width;
    madoutputrow(luma+ 7*16, chroma+3*8, dest);
    dest += width;
    madoutputrow(luma+ 8*16, chroma+4*8, dest);
    dest += width;
    madoutputrow(luma+ 9*16, chroma+4*8, dest);
    dest += width;
    madoutputrow(luma+10*16, chroma+5*8, dest);
    dest += width;
    madoutputrow(luma+11*16, chroma+5*8, dest);
    dest += width;
    madoutputrow(luma+12*16, chroma+6*8, dest);
    dest += width;
    madoutputrow(luma+13*16, chroma+6*8, dest);
    dest += width;
    madoutputrow(luma+14*16, chroma+7*8, dest);
    dest += width;
    madoutputrow(luma+15*16, chroma+7*8, dest);
}

#undef fixedmult

