// (c) 1997-2007 Electronic Arts Inc.

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gimex.h"
#include "madlab.h"


#define PI      3.141592654

/* Access squares table, x must be in range -256..255. */
#define SQR(x)  sqrtbl[(x) & 0x1ff]


/* MPEG quantization matrix. */
static int quanttbl[8][8] =
{
    {   0x08, 0x10, 0x13, 0x16, 0x1a, 0x1b, 0x1d, 0x22  },
    {   0x10, 0x10, 0x16, 0x18, 0x1b, 0x1d, 0x22, 0x25  },
    {   0x13, 0x16, 0x1a, 0x1b, 0x1d, 0x22, 0x22, 0x26  },
    {   0x16, 0x16, 0x1a, 0x1b, 0x1d, 0x22, 0x25, 0x28  },
    {   0x16, 0x1a, 0x1b, 0x1d, 0x20, 0x23, 0x28, 0x30  },
    {   0x1a, 0x1b, 0x1d, 0x20, 0x23, 0x28, 0x30, 0x3a  },
    {   0x1a, 0x1b, 0x1d, 0x22, 0x26, 0x2e, 0x38, 0x45  },
    {   0x1b, 0x1d, 0x23, 0x26, 0x2e, 0x38, 0x45, 0x53  }
};

/* Zigzag table used to read out the contents of the DCT results matrix. */
static int zigzagtbl[64] =
{
     0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

/* This table contains all the MPEG bit patterns with less */
/* than 14 bits except for the escape code (000001). */
/* Entries are bit count, value to encode, unused, bit pattern. */
static unsigned int encodetbl1[95*4] =
{
    0x02, 0xfe00, 0x00, 0x8000,
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


/* Variables used for DCT\IDCT. */
static float   a1,a2,a3,a4;
static float   prescale[8][8];
static float   quant[8][8];

/* Variables used for motion compensation. */
static signed char *motionsrc;
static signed char *motionref;
static int         motionwidth;
static int         motionheight;
static int         motionthreshold;
static int         sqrtbl[512];

/* Variables used for bitstream encoding. */
static unsigned char   *dataptr;
static unsigned int   shiftreg;
static int             bitcount;


/*
;
; Initializes the specified YUV frame buffer to the specified width
; and height and allocates the required amount of memory. The buffer
; is organized as a single plane of signed bytes (all values in the
; range -128..127) with the chrominance channels subsampled 4:1 and
; layed-out as follows:
;
;     +---------+
;     |         |
;     |    Y    |
;     |         |
;     +----+----+
;     | Cb | Cr |
;     +----+----+
;
*/

int madallocateframe(FRAMETYPE *frame, int width, int height)
{
    frame->width = width;
    frame->height = height;
    frame->data = (signed char *) galloc(3*width*height/2);

    if (frame->data == NULL) return(1);
    return(0);
}

/*
;
; Releases the memory allocated to the specified frame buffer.
;
*/

void madreleaseframe(FRAMETYPE *frame)
{
    if (frame->data != NULL) gfree(frame->data);
    frame->width = 0;
    frame->height = 0;
    frame->data = NULL;
}

/*
;
; Copies the contents of the source frame buffer to the destination
; frame buffer. Both buffers must be the same size.
;
*/

void madcopyframe(FRAMETYPE *src, FRAMETYPE *dest)
{
    memcpy(dest->data, src->data, 3*dest->width*dest->height/2);
}

/*
;
; Rounds to the nearest integer.
;
*/

static int madround(float x)
{
    if (x >= 0.0) return((int) (x+0.5));
    return((int) (x-0.5));
}

/*
;
; Rounds a floating point R, G or B value to the nearest integer
; and clamps it to the range 0..255.
;
*/

#if 0
static int cliprgb(float x)
{
    if (x > 255.0f) return(255);
    if (x < 0.0f) return(0);
    return((int) (x+0.5f));
}
#endif

static signed char ROUNDBIASINT(float f)
{
    int i = (int) (f+0.5f);
    return (signed char) i-128;
}

/*
;
; Rounds a floating point Y, Cb or Cr value to the nearest integer
; and clamps it to the range -128..127.
;
*/

#if 0
static int clipyuv(float x)
{
    if (x > 127.0) return(127);
    if (x < -128.0) return(-128);
    return(madround(x));
}
#endif

/*
;
; Returns the Y component of a given ARGB pixel.
;
*/

static inline float gety(ARGB pix)
{
    return (float)(( 0.25678843 * pix.r + 0.50412965 * pix.g + 0.09790586 * pix.b) +  16.0f);
//    return(clipyuv(0.2990f*pix.r + 0.5870f*pix.g + 0.1140f*pix.b - 128.0f));
}

/*
;
; Returns the Cb component of a given ARGB pixel.
;
*/

static inline float getcb(ARGB pix)
{
    return (float)((-0.14822295 * pix.r - 0.29099280 * pix.g + 0.43921576 * pix.b) + 128.0f);
//    return(clipyuv(-0.1687f*pix.r - 0.3313f*pix.g + 0.5000f*pix.b));
}

/*
;
; Returns the Cr component of a given ARGB pixel.
;
*/

static inline float getcr(ARGB pix)
{
    return (float)(( 0.43921557 * pix.r - 0.36778826 * pix.g - 0.07142731 * pix.b) + 128.0f);
//    return(clipyuv(0.5000f*pix.r - 0.4187f*pix.g - 0.0813f*pix.b));
}


/*
;
; Converts a given ARGB frame buffer to YUV. A rectangle with
; the dimensions of the destination frame buffer is extracted
; from the source frame buffer.
;
*/

void madrgbtoyuv(ARGB *src, int rowbytes, FRAMETYPE *dest)
{
    signed char *data;
    int         width;
    int         height;
    int         stride;
    int         i,j;

    width = dest->width;
    height = dest->height;
    data = dest->data;
    stride = rowbytes/4;

    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            data[i*width+j] = ROUNDBIASINT(gety(src[i*stride+j]));
        }
    }

    for (i = 0; i < (height/2); ++i)
    {
        for(j = 0; j < (width/2); ++j)
        {
            data[(height+i)*width+j] = ROUNDBIASINT(
                (getcb(src[(2*i+0)*stride+(2*j+0)]) +
                 getcb(src[(2*i+0)*stride+(2*j+1)]) +
                 getcb(src[(2*i+1)*stride+(2*j+0)]) +
                 getcb(src[(2*i+1)*stride+(2*j+1)])) / 4.0f);

            data[(height+i)*width+j+width/2] = ROUNDBIASINT(
                (getcr(src[(2*i+0)*stride+(2*j+0)]) +
                 getcr(src[(2*i+0)*stride+(2*j+1)]) +
                 getcr(src[(2*i+1)*stride+(2*j+0)]) +
                 getcr(src[(2*i+1)*stride+(2*j+1)])) / 4.0f);
        }
    }
}

/*
;
; Converts a given YUV frame buffer to ARGB.  A rectangle with
; the dimensions of the source frame buffer is copied to the
; destination frame buffer. The alpha channel is set to 255.
;
; This function clears the LSB of the Y, Cb and Cr values before
; converting to simulate the accuracy of the MADCOW decoder.
;
*/

#if 0
See MADyuvtorgb
void madyuvtorgb(FRAMETYPE *src, ARGB *dest, int rowbytes)
{
    signed char *data;
    int         y,cb,cr;
    int         width;
    int         height;
    int         stride;
    int         i,j;

    width = src->width;
    height = src->height;
    data = src->data;
    stride = rowbytes/4;

    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            y  = (data[i*width+j] + 128 - 16);
            cb = (data[(height+i/2)*width+j/2]);
            cr = (data[(height+i/2)*width+j/2+width/2]);

            float R = 1.164383f * y                     + 1.596027f * cr  ;
            float G = 1.164383f * y - (0.391762f * cb) - (0.812968f * cr) ;
            float B = 1.164383f * y +  2.017232f * cb                     ;

            dest[i*stride+j].r = cliprgb(R); //y + 1.4020f*cr);
            dest[i*stride+j].g = cliprgb(G); //y - 0.3441f*cb - 0.7141f*cr);
            dest[i*stride+j].b = cliprgb(B); //y + 1.7720f*cb);
            dest[i*stride+j].a = 255;
        }
    }
}
#endif

/*
;
; Initializes the DCT/IDCT functions.
; This function must be called before a frame can be
; compressed or uncompressed.
;
*/

void madinitdct(void)
{
    int     i,j;

    a1 = (float) (cos(2.0*(PI/8.0)));
    a2 = (float) (cos(PI/8.0) - cos(3.0*(PI/8.0)));
    a3 = (float) (cos(PI/8.0) + cos(3.0*(PI/8.0)));
    a4 = (float) (cos(3.0*(PI/8.0)));

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            prescale[i][j] = 1.0;

            if (i == 0)
            {
                prescale[i][j] /= (float) sqrt(8.0);
            }
            else
            {
                prescale[i][j] /= 4.0f * (float) cos(i*(PI/16.0));
            }

            if (j == 0)
            {
                prescale[i][j] /= (float) sqrt(8.0);
            }
            else
            {
                prescale[i][j] /= 4.0f * (float) cos(j*(PI/16.0));
            }
        }
    }
}

/*
;
; Set the quality factor for compression/decompression.
; The highest quality is factor 1 and the quality degrades
; as the factor increases.
;
*/

void madsetquality(int quality)
{
    int     i,j;

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            quant[i][j] = (quanttbl[i][j]*quality)/8.0f;
        }
    }

    quant[0][0] = (float)(quanttbl[0][0]);
}

/*
;
; Performs a fast column DCT using the 5 multiply algorithm by Arai,
; Agui and Nakajima (described in "JPEG Still Image Data Compression
; Standard" by Pennebaker and Mitchell).
;
; Parameters src and dest are the topmost entries of the intendeded
; columns within the 8x8 source and destination matrices.
;
*/

static void dctcolumn(float* src, float* dest)
{
    float t1,t2,t3,t4,t5,t6;

    t1 = src[0*8] + src[7*8];
    t2 = src[1*8] + src[6*8];
    t3 = src[2*8] + src[5*8];
    t4 = src[3*8] + src[4*8];

    t5 = t1 + t4;
    t6 = t2 + t3;
    t2 -= t3;
    t1 -= t4;

    t3 = a1*(t1 + t2);

    dest[0*8] = t5 + t6;
    dest[4*8] = t5 - t6;
    dest[2*8] = t1 + t3;
    dest[6*8] = t1 - t3;

    t1 = src[3*8] - src[4*8];
    t2 = src[2*8] - src[5*8];
    t3 = src[1*8] - src[6*8];
    t4 = src[0*8] - src[7*8];

    t1 += t2;
    t2 += t3;
    t3 += t4;

    t5 = a4*(t1 - t3);
    t1 = a2*t1 + t5;
    t2 *= a1;
    t3 = a3*t3 + t5;

    t5 = t2 + t4;
    t4 -= t2;

    dest[5*8] = t4 + t1;
    dest[1*8] = t5 + t3;
    dest[7*8] = t5 - t3;
    dest[3*8] = t4 - t1;
}

/*
;
; Performs a fast row DCT using the 5 multiply algorithm by Arai,
; Agui and Nakajima (described in "JPEG Still Image Data Compression
; Standard" by Pennebaker and Mitchell).
;
; Parameters src and dest are the leftmost entries of the intendeded
; rows within the 8x8 source and destination matrices.
;
*/

static void dctrow(float* src, float* dest)
{
    float t1,t2,t3,t4,t5,t6;

    t1 = src[0] + src[7];
    t2 = src[1] + src[6];
    t3 = src[2] + src[5];
    t4 = src[3] + src[4];

    t5 = t1 + t4;
    t6 = t2 + t3;
    t2 -= t3;
    t1 -= t4;

    t3 = a1*(t1 + t2);

    dest[0] = t5 + t6;
    dest[4] = t5 - t6;
    dest[2] = t1 + t3;
    dest[6] = t1 - t3;

    t1 = src[3] - src[4];
    t2 = src[2] - src[5];
    t3 = src[1] - src[6];
    t4 = src[0] - src[7];

    t1 += t2;
    t2 += t3;
    t3 += t4;

    t5 = a4*(t1 - t3);
    t1 = a2*t1 + t5;
    t2 *= a1;
    t3 = a3*t3 + t5;

    t5 = t2 + t4;
    t4 -= t2;

    dest[5] = t4 + t1;
    dest[1] = t5 + t3;
    dest[7] = t5 - t3;
    dest[3] = t4 - t1;
}

/*
;
; Performs a fast column IDCT using the 5 multiply algorithm by Arai,
; Agui and Nakajima (described in "JPEG Still Image Data Compression
; Standard" by Pennebaker and Mitchell).
;
; Parameters src and dest are the topmost entries of the intendeded
; columns within the 8x8 source and destination matrices.
;
*/

#if 0
static void idctcolumn(float* src, float* dest)
{
    float t1,t2,t3,t4,t5,t6,t7,t8,t9;

    t1 = src[5*8] - src[3*8];
    t2 = src[1*8] + src[7*8];
    t3 = src[1*8] - src[7*8];
    t4 = src[5*8] + src[3*8];

    t5 = t2 - t4;
    t4 += t2;

    t2 = a4*(t1 + t3);
    t1 = a2*t1 + t2;
    t5 = a1*t5;
    t3 = a3*t3 - t2;

    t4 += t3;
    t3 += t5;
    t5 += t1;

    t6 = src[0*8] + src[4*8];
    t7 = src[0*8] - src[4*8];
    t8 = a1*(src[2*8] - src[6*8]);
    t9 = src[2*8] + src[6*8] + t8;

    t2 = t6 - t9;
    t6 += t9;
    t9 = t7 - t8;
    t7 += t8;

    dest[0*8] = t6 + t4;
    dest[1*8] = t7 + t3;
    dest[2*8] = t9 + t5;
    dest[3*8] = t2 + t1;
    dest[4*8] = t2 - t1;
    dest[5*8] = t9 - t5;
    dest[6*8] = t7 - t3;
    dest[7*8] = t6 - t4;
}
#endif

/*
;
; Performs a fast row IDCT using the 5 multiply algorithm by Arai,
; Agui and Nakajima (described in "JPEG Still Image Data Compression
; Standard" by Pennebaker and Mitchell).
;
; Parameters src and dest are the leftmost entries of the intendeded
; rows within the 8x8 source and destination matrices.
;
*/

#if 0
static void idctrow(float* src, float* dest)
{
    float t1,t2,t3,t4,t5,t6,t7,t8,t9;

    t1 = src[5] - src[3];
    t2 = src[1] + src[7];
    t3 = src[1] - src[7];
    t4 = src[5] + src[3];

    t5 = t2 - t4;
    t4 += t2;

    t2 = a4*(t1 + t3);
    t1 = a2*t1 + t2;
    t5 = a1*t5;
    t3 = a3*t3 - t2;

    t4 += t3;
    t3 += t5;
    t5 += t1;

    t6 = src[0] + src[4];
    t7 = src[0] - src[4];
    t8 = a1*(src[2] - src[6]);
    t9 = src[2] + src[6] + t8;

    t2 = t6 - t9;
    t6 += t9;
    t9 = t7 - t8;
    t7 += t8;

    dest[0] = t6 + t4;
    dest[1] = t7 + t3;
    dest[2] = t9 + t5;
    dest[3] = t2 + t1;
    dest[4] = t2 - t1;
    dest[5] = t9 - t5;
    dest[6] = t7 - t3;
    dest[7] = t6 - t4;
}
#endif

/*
;
; Compresses a specified 8x8 block of Y, Cb or Cr values.
; This routine performs a DCT on the data, quantizes it and outputs
; it in zigzag order using run length encoding.
;
; Each run length code contains a signed 10 bit value in bits 0..9
; and a run length count in bits 10..15. The first code contains the
; DC coefficient. Subsequent codes contain the non-zero AC coefficients
; with their runs of leading zeros. The last code output is the EOB
; code (0xfe00).
;
; Returns the number of run length codes output. The destination buffer
; must be large enough to hold all these values (potentially up to 65).
;
*/

static int compressblock(signed char *src, int stride, unsigned short *dest)
{
    float   temp1[8][8];
    float   temp2[8][8];
    int     result[64];
    int     count;
    int     zeros;
    int     val;
    int     i,j;

    /* Get the data. */
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            temp1[i][j] = src[i*stride+j];
        }
    }

    /* Do the column DCTs. */
    for (i = 0; i < 8; ++i)
    {
        dctcolumn( &temp1[0][i], &temp2[0][i] );
    }

    /* Do the row DCTs. */
    for (i = 0; i < 8; ++i)
    {
        dctrow( &temp2[i][0], &temp1[i][0] );
    }

    /* Scale and quantize the results. */
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            result[8*i+j] = madround(temp1[i][j]*prescale[i][j]/quant[i][j]);
        }
    }

    /* Output the run length codes. */
    dest[0] = result[0] & 0x3ff;
    count = 1;
    zeros = 0;

    for (i = 1; i < 64; ++i)
    {
        val = result[zigzagtbl[i]];

        if (val == 0)
        {
            ++zeros;
        }
        else
        {
            dest[count] = (zeros << 10) | (val & 0x3ff);
            ++count;
            zeros = 0;
        }
    }

    dest[count] = 0xfe00;
    ++count;

    return(count);
}

/*
;
; Uncompresses an 8x8 block of Y, Cb or Cr values.
; Reverses the effect of compressblock().
;
; Returns the number of run length codes read so that the caller
; can locate the start of the next block.
;
*/

#if 0
static int uncompressblock(unsigned short *src, signed char *dest, int stride)
{
    float   temp1[8][8];
    float   temp2[8][8];
    int     result[64];
    int     index;
    int     count;
    int     i,j;

    /* Read the run length codes and initialize the matrix. */
    result[0] = (src[0] << 22) >> 22;
    for (i = 1; i < 64; ++i) result[i] = 0;

    index = 1;
    count = 1;
    while (src[count] != 0xfe00)
    {
        index += (src[count] >> 10) & 0x3f;
        result[zigzagtbl[index]] = (src[count] << 22) >> 22;
        ++index;
        ++count;
    }
    ++count;

    /* Dequantize and scale the results. */
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            temp1[i][j] = result[8*i+j]*quant[i][j]*prescale[i][j];
        }
    }

    /* Do the column IDCTs. */
    for (i = 0; i < 8; ++i)
    {
        idctcolumn( &temp1[0][i], &temp2[0][i] );
    }

    /* Do the row IDCTs. */
    for (i = 0; i < 8; ++i)
    {
        idctrow( &temp2[i][0], &temp1[i][0] );
    }

    /* Clip and output the results. */
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            dest[i*stride+j] = clipyuv(temp1[i][j]);
        }
    }

    return(count);
}
#endif

/*
;
; Compresses the specified frame buffer to a series of run length codes.
;
; The frame is broken down into a series of 16x16 pixel macroblocks and
; each macroblock is in turn broken down into six 8x8 blocks in the
; order Y1, Y2, Y3, Y4, Cb, Cr. The macroblocks are processed from left
; to right and from top to bottom. Run length codes for each block are
; output in sequence followed by a final EOF code (0xfe00).
;
; Returns the number of run length codes output unless this number is
; close to parameter limit (within 400), in which case the value limit
; is returned to indicate failure.
;
*/

int madcompressframe(FRAMETYPE *src, unsigned short *dest, int limit)
{
    signed char *data;
    int         width;
    int         height;
    int         count;
    int         i,j;

    width = src->width;
    height = src->height;
    data = src->data;

    count = 0;

    for (i = 0; i < height; i += 16)
    {
        for (j = 0; j < width; j += 16)
        {
            if (count > (limit-400)) return(limit);

            count += compressblock(&data[(i+0)*width+(j+0)], width, &dest[count]);
            count += compressblock(&data[(i+0)*width+(j+8)], width, &dest[count]);
            count += compressblock(&data[(i+8)*width+(j+0)], width, &dest[count]);
            count += compressblock(&data[(i+8)*width+(j+8)], width, &dest[count]);
            count += compressblock(&data[(height+i/2)*width+j/2], width, &dest[count]);
            count += compressblock(&data[(height+i/2)*width+j/2+width/2], width, &dest[count]);
        }
    }

    dest[count] = 0xfe00;
    ++count;

    return(count);
}

/*
;
; Uncompresses a series of run length codes and stores the result
; in the specified frame buffer.
; Reverses the effect of madcompressframe().
;
*/

#if 0
void maduncompressframe(unsigned short *src, FRAMETYPE *dest)
{
    signed char *data;
    int         width;
    int         height;
    int         count;
    int         i,j;

    width = dest->width;
    height = dest->height;
    data = dest->data;

    count = 0;

    for (i = 0; i < height; i += 16)
    {
        for (j = 0; j < width; j += 16)
        {
            count += uncompressblock(&src[count], &data[(i+0)*width+(j+0)], width);
            count += uncompressblock(&src[count], &data[(i+0)*width+(j+8)], width);
            count += uncompressblock(&src[count], &data[(i+8)*width+(j+0)], width);
            count += uncompressblock(&src[count], &data[(i+8)*width+(j+8)], width);
            count += uncompressblock(&src[count], &data[(height+i/2)*width+j/2], width);
            count += uncompressblock(&src[count], &data[(height+i/2)*width+j/2+width/2], width);
        }
    }
}
#endif

/*
;
; Initializes the motion compensation functions.
; This function must be called before motion analysis
; can be performed.
;
*/

void madinitmotion(void)
{
    int     i;

    for (i = -256; i < 256; ++i) SQR(i) = i*i;
}

/*
;
; Sets the motion threshold used for motion analysis.
; The motion threshold is the limit on the sum of the square of errors
; between a source and a reference block after DC correction.
;
*/

void madsetmotionthreshold(int threshold)
{
    motionthreshold = threshold;
}

/*
;
; Compares a given 8x8 source block to a given reference block and returns
; an error term (the sum of the squares of pixel differences) as well as the
; DC correction (applied to each pixel of the source block) that was used
; to minimize this error.
;
; If x[i], i=1..64 is the per pixel error, then we want to find the value
; dc that minimizes err = sum((x[i]-dc)^2). This value is dc = sum(x[i])/64,
; the average error.
;
; It can be shown that sum((x[i]-sum(x[i])/64)^2) = sum(x[i]^2)-sum(x[i])^2/64.
; This allows us to compute both the dc correction and the error term in a
; single pass by accumulating the per pixel errors and their squares.
; Furthermore, if err(n) is the error term for the first n pixels, it can
; be shown that err(n+1) >= err(n). This allows us to stop as soon as we
; exceed the error threshold. If this occurs, the error term is set to the
; threshold and the DC correction is not valid.
;
*/

static void matchblock(signed char *src, signed char *ref, int *err, int *dc)
{
    int     diff;
    int     sum,sum2;
    int     i;

    sum = 0;
    sum2 = 0;
    for (i = 0; i < 8; ++i)
    {
        diff = src[0] - ref[0];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[1] - ref[1];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[2] - ref[2];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[3] - ref[3];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[4] - ref[4];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[5] - ref[5];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[6] - ref[6];
        sum += diff;
        sum2 += SQR(diff);

        diff = src[7] - ref[7];
        sum += diff;
        sum2 += SQR(diff);

        /* Check (without dividing) if we have exceeded the error threshold. */
        /* The -1 term accounts for any rounding effects. */
        if ((8*(i+1))*(sum2-motionthreshold-1) >= (sum*sum))
        {
            *err = motionthreshold;
            return;
        }

        src += motionwidth;
        ref += motionwidth;
    }

    *err = sum2 - ((sum*sum+32) >> 6);
    *dc = (sum+32) >> 6;
}

/*
;
; Compares the 16x16 pixel macroblock at position x1,y1 of the source frame
; with the macroblock at position x2,y2 of the reference frame and returns
; the number of blocks that are within the error threshold, as well as the
; sum of the error terms for these blocks. Blocks with a DC correction greater
; than 33 are excluded.
;
; Parameter countmax is the highest block match count so far for the source
; macroblock. This allows us to exit as soon as we discover that we can not
; possibly reach this count.
;
; Note that the motion vector within the chrominance data if found by
; dividing the motion vector for the macroblock by 2 and truncating the
; result.
;
*/

static void matchmacroblock(int x1, int y1, int x2, int y2, int countmax, int *count, int *err)
{
    signed char *src;
    signed char *ref;
    int         error;
    int         dc;

    *count = 0;
    *err = 0;

    src = motionsrc + y1*motionwidth + x1;
    ref = motionref + y2*motionwidth + x2;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        ++*count;
        *err += error;
    }
    if (*count < countmax-5) return;

    src += 8;
    ref += 8;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        ++*count;
        *err += error;
    }
    if (*count < countmax-4) return;

    src += 8*motionwidth - 8;
    ref += 8*motionwidth - 8;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        ++*count;
        *err += error;
    }
    if (*count < countmax-3) return;

    src += 8;
    ref += 8;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        ++*count;
        *err += error;
    }
    if (*count < countmax-2) return;

    src = motionsrc + (motionheight+y1/2)*motionwidth + (x1/2);
    ref = motionref + (motionheight+y2/2)*motionwidth + (x2/2);

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        ++*count;
        *err += error;
    }
    if (*count < countmax-1) return;

    src += motionwidth/2;
    ref += motionwidth/2;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        ++*count;
        *err += error;
    }
}

/*
;
; Searches the reference frame to find the best match for the 16x16 pixel
; macroblock at position x,y of the source frame, with an offset of no more
; than 16 pixels horizontally or vertically. The position of the best match
; is returned in x0,y0 and the function return value gives the number of
; blocks that were matched at that position.
;
; The best match is the one that yields the highest block match count. If
; there is a tie, the one with the lowest sum of error terms wins.
;
*/

static int bestmatch(int x, int y, int *x0, int *y0)
{
    int xmin,xmax;
    int ymin,ymax;
    int count;
    int countmax;
    int err;
    int errmin;
    int imin,jmin;
    int i,j;

    /* Determine the search region. */
    xmin = x - 16;
    xmax = x + 16;
    ymin = y - 16;
    ymax = y + 16;

    /* Do not search outside the reference frame. */
    if (xmin < 0) xmin = 0;
    if (ymin < 0) ymin = 0;
    if (xmax > (motionwidth-16)) xmax = motionwidth-16;
    if (ymax > (motionheight-16)) ymax = motionheight-16;

    /* Check the reference macroblock at offset 0,0. */
    imin = y;
    jmin = x;
    matchmacroblock(x, y, x, y, 0, &countmax, &errmin);

    /* If we don't have a perfect match, scan the entire region. */
    if ((countmax < 6) || (errmin > 0))
    {
        for (i = ymin; i <= ymax; ++i)
        {
            for (j = xmin; j <= xmax; ++j)
            {
                matchmacroblock(x, y, j, i, countmax, &count, &err);
                if ((count > countmax) || ((count == countmax) && (err < errmin)))
                {
                    imin = i;
                    jmin = j;
                    countmax = count;
                    errmin = err;
                }
            }
        }
    }

    *x0 = jmin;
    *y0 = imin;

    return(countmax);
}

/*
;
; Compares the 16x16 pixel macroblock at position x1,y1 of the source frame
; to the macroblock at position x2,y2 of the reference frame and stores the
; results to the specified motion information structure.
;
; The motion information structure contains the motion vector for the
; macroblock as well as a motion flag and DC correction that is set for
; every block that is within the motion threshold and has a DC correction
; less than 34. The DC correction is divided by 2 before being stored.
;
*/

static void setmotioninfo(int x1, int y1, int x2, int y2, MOTIONINFOTYPE *motion)
{
    signed char *src;
    signed char *ref;
    int         error;
    int         dc;

    motion->flags = 0;
    motion->vectorx = x2-x1;
    motion->vectory = y2-y1;

    src = motionsrc + y1*motionwidth + x1;
    ref = motionref + y2*motionwidth + x2;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        motion->flags |= 0x01;
        motion->dccorrect[0] = dc/2;
    }

    src += 8;
    ref += 8;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        motion->flags |= 0x02;
        motion->dccorrect[1] = dc/2;
    }

    src += 8*motionwidth - 8;
    ref += 8*motionwidth - 8;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        motion->flags |= 0x04;
        motion->dccorrect[2] = dc/2;
    }

    src += 8;
    ref += 8;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        motion->flags |= 0x08;
        motion->dccorrect[3] = dc/2;
    }

    src = motionsrc + (motionheight+y1/2)*motionwidth + (x1/2);
    ref = motionref + (motionheight+y2/2)*motionwidth + (x2/2);

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        motion->flags |= 0x10;
        motion->dccorrect[4] = dc/2;
    }

    src += motionwidth/2;
    ref += motionwidth/2;

    matchblock(src, ref, &error, &dc);
    if ((error < motionthreshold) && (abs(dc) < 34))
    {
        motion->flags |= 0x020;
        motion->dccorrect[5] = dc/2;
    }
}

/*
;
; Performs motion analysis on the specified source frame compared to
; the specified reference frame, using the motion threshold set with
; madsetmotionthreshold(). The results are stored to the specified array
; of motion information structures containing one entry per macroblock.
; The function returns the total number of motion blocks found within
; the source frame.
;
*/

int madanalyzemotion(FRAMETYPE *src, FRAMETYPE *ref, MOTIONINFOTYPE *motion)
{
    int     motionblocks;
    int     x0,y0;
    int     i,j;

    motionsrc = src->data;
    motionref = ref->data;
    motionwidth = src->width;
    motionheight = src->height;

    motionblocks = 0;

    for (i = 0; i < motionheight; i += 16)
    {
        for (j = 0; j < motionwidth; j += 16)
        {
            motionblocks += bestmatch(j, i, &x0, &y0);
            setmotioninfo(j, i, x0, y0, motion);
            ++motion;
        }
    }

    return(motionblocks);
}

/*
;
; Moves an 8x8 block of values from the specified position in the source
; frame to the specified position in the destination frame, applying the
; specified DC correction to each one. The values are clamped to the
; range -128..127.
;
*/

static void moveblock(signed char *src, signed char *dest, int dc)
{
    int     val;
    int     i;
    int     j;

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            val = src[j] + dc;
            if (val > 127) val = 127;
            else if (val < -128) val = -128;
            dest[j] = val;
        }

        src += motionwidth;
        dest += motionwidth;
    }
}

/*
;
; Applies the motion compensation described in the specified array of
; motion information structures to the specified destination frame,
; using the specified source frame as reference.
;
*/

void madapplymotion(FRAMETYPE *src, FRAMETYPE *dest, MOTIONINFOTYPE *motion)
{
    int     flags;
    int     x0,y0;
    int     i,j;

    motionwidth = src->width;
    motionheight = src->height;

    for (i = 0; i < motionheight; i += 16)
    {
        for (j = 0; j < motionwidth; j += 16)
        {
            flags = motion->flags;
            x0 = j + motion->vectorx;
            y0 = i + motion->vectory;

            if ((flags & 0x01) != 0) moveblock(src->data + y0*motionwidth + x0,
                                               dest->data + i*motionwidth + j,
                                               2*motion->dccorrect[0]);

            if ((flags & 0x02) != 0) moveblock(src->data + y0*motionwidth + (x0+8),
                                               dest->data + i*motionwidth + (j+8),
                                               2*motion->dccorrect[1]);

            if ((flags & 0x04) != 0) moveblock(src->data + (y0+8)*motionwidth + x0,
                                               dest->data + (i+8)*motionwidth + j,
                                               2*motion->dccorrect[2]);

            if ((flags & 0x08) != 0) moveblock(src->data + (y0+8)*motionwidth + (x0+8),
                                               dest->data + (i+8)*motionwidth + (j+8),
                                               2*motion->dccorrect[3]);

            if ((flags & 0x10) != 0) moveblock(src->data + (motionheight+y0/2)*motionwidth + x0/2,
                                               dest->data + (motionheight+i/2)*motionwidth + j/2,
                                               2*motion->dccorrect[4]);

            if ((flags & 0x20) != 0) moveblock(src->data + (motionheight+y0/2)*motionwidth + x0/2 + motionwidth/2,
                                               dest->data + (motionheight+i/2)*motionwidth + j/2 + motionwidth/2,
                                               2*motion->dccorrect[5]);

            ++motion;
        }
    }
}

/*
;
; Computes the number of bits required to VLC encode one block of data
; from the specified array of run length codes. The result is returned
; via parameter bitcount.
;
; The function returns the number of run length codes read so that the
; caller can locate the start of the next block in the array.
;
*/

static int sizeblock(unsigned short *rledata, int *bitcount)
{
    int     bits;
    int     count;
    int     val;
    int     found;
    int     i;

    /* DC coefficient. */
    bits = 8;
    count = 1;

    /* AC coefficients. */
    while (rledata[count] != 0xfe00)
    {
        val = rledata[count];
        found = 0;

        /* Check table 1. */
        for (i = 0; i < 95; ++i)
        {
            if (val == (int) encodetbl1[4*i+1])
            {
                bits += encodetbl1[4*i];
                found = 1;
                break;
            }
        }

        if (!found)
        {
            /* Check table 2. */
            for (i = 0; i < 128; ++i)
            {
                if (val == (int) encodetbl2[4*i+1])
                {
                    bits += encodetbl2[4*i] + 8;
                    found = 1;
                    break;
                }
            }
        }

        if (!found)
        {
            /* Code as escape sequence. */
            bits += 22;
        }

        ++count;
    }

    /* End Of Block. */
    bits += 2;
    ++count;

    *bitcount = bits;
    return(count);
}

/*
;
; Computes the number of bits required to VLC encode one macroblock of
; data from the specified array of run length codes. Parameter key returns
; the number of bits required if the macroblock is in a key frame.
; Parameter motion returns the number of bits required if the macroblock
; is in a motion frame, using the motion information in parameter
; motiondata.
;
; The function returns the number of run length codes read so that the
; caller can locate the start of the next macroblock in the array.
;
*/

static int sizemacroblock(unsigned short *rledata, MOTIONINFOTYPE *motiondata, int *key, int *motion)
{
    int     keybits;
    int     motionbits;
    int     bitcount;
    int     count;
    int     flags;
    int     i;

    count = 0;
    flags = motiondata->flags;
    keybits = 0;

    /* Bits to encode motion flags and motion vectors. */
    if (flags == 0)
    {
        motionbits = 2;
    }
    else
    {
        if (flags == 0x3f) motionbits = 1;
        else motionbits = 8;

        if (motiondata->vectorx == 0) motionbits += 1;
        else motionbits += 6;

        if (motiondata->vectory == 0) motionbits += 1;
        else motionbits += 6;
    }

    /* Bits to encode each block. */
    for (i = 0; i < 6; ++i)
    {
        count += sizeblock(&rledata[count], &bitcount);

        keybits += bitcount;

        if ((flags & 1) == 0)
        {
            motionbits += bitcount;
        }
        else
        {
            if (motiondata->dccorrect[i] == 0) motionbits += 1;
            else motionbits += 6;
        }

        flags >>= 1;
    }

    *key = keybits;
    *motion = motionbits;
    return(count);
}

/*
;
; Computes the number of bits required to VLC encode the frame of data
; in the specified array of run length codes. Parameter key returns
; the number of bits required to encode it as a key frame. Parameter
; motion returns the number of bits required to encode it as a motion
; frame, using the motion information array specified by parameter
; motiondata. Parameter motiondata can be NULL, in which case only
; the number of bits required to code the frame as a key frame will
; be computed.
;
*/

void madsizeframe(unsigned short *rledata, MOTIONINFOTYPE *motiondata, int *key, int *motion)
{
    MOTIONINFOTYPE  dummy;
    int             keybits;
    int             motionbits;
    int             count;

    *key = 0;
    count = 0;

    if (motiondata == NULL)
    {
        dummy.flags = 0;

        while (rledata[count] != 0xfe00)
        {
            count += sizemacroblock(&rledata[count], &dummy, &keybits, &motionbits);

            *key += keybits;
        }
    }
    else
    {
        *motion = 0;

        while (rledata[count] != 0xfe00)
        {
            count += sizemacroblock(&rledata[count], motiondata, &keybits, &motionbits);
            ++motiondata;

            *key += keybits;

            /* Check if it's cheaper to encode the macroblock with */
            /* or without motion information. */
            if ((keybits+2) < motionbits) *motion += keybits+2;
            else *motion += motionbits;
        }
    }
}

/*
;
; Outputs a specified number of bits of data to the current bitstream.
; The bits are output from parameter data starting at bit 15 and working
; towards bit 0.
;
*/

static void emitbits(unsigned int data, int bits)
{
    shiftreg |= data << (16-bitcount);
    bitcount += bits;

    if (bitcount > 16)
    {
        dataptr[0] = shiftreg >> 16;
        dataptr[1] = shiftreg >> 24;
        shiftreg <<= 16;
        bitcount -= 16;
        dataptr += 2;
    }
}

/*
;
; Outputs a specified delta value to the current bitstream.
; A delta value is a number in the range -16..16 and is encoded
; as follows:
;
;   0       - 0
;   1..16   - 10bbbb where bbbb = n-1
;   -1..-16 - 11bbbb where bbbb = n+16
;
*/

static void emitdelta(int delta)
{
    if (delta == 0) emitbits(0, 1);
    else if (delta > 0) emitbits(((delta-1) | 0x20) << 10, 6);
    else emitbits(((delta+16) | 0x30) << 10, 6);
}

/*
;
; VLC encodes one block of data from the specified array of run length
; codes and outputs it to the current bitstream.
;
; The function returns the number of run length codes read so that the
; caller can locate the start of the next block in the array.
;
*/

static int encodeblock(unsigned short *rledata)
{
    int     count;
    int     val;
    int     found;
    int     i;

    /* DC coefficient. */
    emitbits((rledata[0] & 0xff) << 8, 8);
    count = 1;

    /* AC coefficients. */
    while (rledata[count] != 0xfe00)
    {
        val = rledata[count];
        found = 0;

        /* Check table 1. */
        for (i = 0; i < 95; ++i)
        {
            if (val == (int) encodetbl1[4*i+1])
            {
                emitbits(encodetbl1[4*i+3], encodetbl1[4*i]);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            /* Check table 2. */
            for (i = 0; i < 128; ++i)
            {
                if (val == (int) encodetbl2[4*i+1])
                {
                    emitbits(0, 8);
                    emitbits(encodetbl2[4*i+3], encodetbl2[4*i]);
                    found = 1;
                    break;
                }
            }
        }

        if (!found)
        {
            /* Code as escape sequence (esc, val, run). */
            emitbits(0x0400, 6);
            emitbits((val & 0x3ff) << 6, 10);
            emitbits((val & 0xfc00), 6);
        }

        ++count;
    }

    /* End Of Block. */
    emitbits(encodetbl1[4*0+3], encodetbl1[4*0]);
    ++count;

    return(count);
}

/*
;
; VLC encodes one macroblock of data from the specified array of run length
; codes and outputs it to the current bitstream. If any of the motion flags
; are set in the specified motion information structure, the macroblock is
; encoded for a motion frame. If all the flags are clear, the macroblock is
; encoded for a key frame (note that this requires the caller to output the
; initial 00 bit sequence if the macroblock is actually in a motion frame).
;
; The function returns the number of run length codes read so that the
; caller can locate the start of the next macroblock in the array.
;
*/

static int encodemacroblock(unsigned short *rledata, MOTIONINFOTYPE *motiondata)
{
    int     bitcount;
    int     count;
    int     flags;
    int     i;

    count = 0;
    flags = motiondata->flags;

    if (flags != 0)
    {
        /* Emit motion flags. */
        if (flags == 0x3f) emitbits(1 << 15, 1);
        else emitbits((flags | 0x40) << 8, 8);

        /* Emit motion vector. */
        emitdelta(motiondata->vectorx);
        emitdelta(motiondata->vectory);
    }

    for (i = 0; i < 6; ++i)
    {
        if ((flags & 1) == 0)
        {
            /* Encode block explicitly. */
            count += encodeblock(&rledata[count]);
        }
        else
        {
            /* Motion block: emit DC correction and skip RLE data. */
            emitdelta(motiondata->dccorrect[i]);
            count += sizeblock(&rledata[count], &bitcount);
        }

        flags >>= 1;
    }

    return(count);
}

/*
;
; VLC encodes the frame of data in the specified array of run length codes.
; If parameter motiondata is NULL, the frame is encoded as a key frame. If
; not, the frame is encoded as a motion frame using the array of motion
; information structures supplied. The output bitstream is written to the
; array specified by parameter dest and the last 16 bit word is padded with
; zeros. The array must be large enough to hold the entire frame.
;
; Returns the number of bytes written to the destination buffer.
;
; MADCOW encodes a frame by breaking it down into a series of 16x16 pixel
; macroblocks that are scanned from left to right and from top to bottom.
; Each macroblock is converted to YUV and the chroma channels subsampled to
; yield six 8x8 data blocks. The macroblocks are then encoded as follows:
;
; Motion flags (all flags clear is implied for key frames):
;   1        - all flags set
;   00       - all flags clear
;   01ffffff - other (order is Cr, Cb, Y4, Y3, Y2, Y1)
;
; Motion vector X,Y (only if there is a motion flag set):
;   0       - 0
;   1snnnn  - (n+1) if s=0, (n-16) if s=1
;
; Data blocks Y1, Y2, Y3, Y4, Cb, Cr:
;
;   Zigzag encoding if motion flag clear:
;     DC,{(run,AC)},EOB with 8 bit DC, 6 bit run, 10 bit AC.
;     Uses MPEG1 VLC/quantization, escape sequence is Esc,AC,run (22 bits).
;
;   DC correction if motion flag set:
;     0       - 0
;     1snnnn  - 2*(n+1) if s=0, 2*(n-16) if s=1
;
; A MADCOW bitstream consists of a series of 16 bit words output using
; Intel byte ordering and interpreted from the high bit towards the low bit.
;
*/

int madencodeframe(unsigned short *rledata, MOTIONINFOTYPE *motiondata, unsigned char *dest)
{
    MOTIONINFOTYPE  dummy;
    int             keybits;
    int             motionbits;
    int             count;

    /* Initialize the bitstream. */
    dataptr = dest;
    shiftreg = 0;
    bitcount = 0;

    dummy.flags = 0;
    count = 0;

    if (motiondata == NULL)
    {
        while (rledata[count] != 0xfe00)
        {
            count += encodemacroblock(&rledata[count], &dummy);
        }
    }
    else
    {
        while (rledata[count] != 0xfe00)
        {
            /* Check if it's cheaper to encode the macroblock with */
            /* or without motion information. */
            sizemacroblock(&rledata[count], motiondata, &keybits, &motionbits);

            if ((keybits+2) < motionbits)
            {
                emitbits(0, 2);
                count += encodemacroblock(&rledata[count], &dummy);
            }
            else
            {
                if (motiondata->flags == 0) emitbits(0, 2);
                count += encodemacroblock(&rledata[count], motiondata);
            }

            ++motiondata;
        }
    }

    /* Pad the last word with zeros and force it into the buffer. */
    if (bitcount > 0) emitbits(0, 16);

    return(dataptr - dest);
}

