// (c) 2000-2007 Electronic Arts Inc.
//
//
//		PSX2/PC IDEC Encoder/Decoder
//
//		Martin Griffiths, Bullfrog Productions Ltd.
//
//
//		Last Updated : December 6th 1999
//
//		V1.01
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "idec_encode.h"

#ifndef PI
#define PI (3.14159265358979323846f)
#endif

#if 0
#define FLOAT_TO_INT(_i,_f){float _ff = _f; __asm fld _ff __asm fistp dword ptr _i}
#else
#define FLOAT_TO_INT(_i,_f){_i = (int) (_f+0.5f);}
#endif

/* picture coding type */
#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4
/* macroblock type */
#define MB_INTRA    1
#define MB_PATTERN  2
#define MB_BACKWARD 4
#define MB_FORWARD  8
#define MB_QUANT    16

typedef struct
{
  unsigned char code; 
  char len;
} VLCtable;

typedef struct
{
  unsigned short code; 
  char len;
} sVLCtable;

static int dc_dct_pred[3];

static float         work1[6][8][8]; // input data
static int           work2[6][8][8]; 

static unsigned char *cdataBuffer;      // compressed data
static unsigned int    shiftReg;       // shift register
static unsigned int             bitCount;       // bit count in shift register
static unsigned int            byteCount;      // bytes output from shift register

static float            a1,a2,a3,a4;
static float            prescale[8][8];

// Zigzag matrix used to extract DCT coefficients for compression.

static unsigned char  zigzag[64] =
{
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
    19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
    42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

/* default intra quantization matrix */
static unsigned char default_intra_quantizer_matrix[64]
=
{
   8, 16, 19, 22, 26, 27, 29, 34,
  16, 16, 22, 24, 27, 29, 34, 37,
  19, 22, 26, 27, 29, 34, 34, 38,
  22, 22, 26, 27, 29, 34, 37, 40,
  22, 26, 27, 29, 32, 35, 40, 48,
  26, 27, 29, 32, 35, 40, 48, 58,
  26, 27, 29, 34, 38, 46, 56, 69,
  27, 29, 35, 38, 46, 56, 69, 83
};




static VLCtable cbptable[64]=
{
  {0x01,9}, {0x0b,5}, {0x09,5}, {0x0d,6}, 
  {0x0d,4}, {0x17,7}, {0x13,7}, {0x1f,8}, 
  {0x0c,4}, {0x16,7}, {0x12,7}, {0x1e,8}, 
  {0x13,5}, {0x1b,8}, {0x17,8}, {0x13,8}, 
  {0x0b,4}, {0x15,7}, {0x11,7}, {0x1d,8}, 
  {0x11,5}, {0x19,8}, {0x15,8}, {0x11,8}, 
  {0x0f,6}, {0x0f,8}, {0x0d,8}, {0x03,9}, 
  {0x0f,5}, {0x0b,8}, {0x07,8}, {0x07,9}, 
  {0x0a,4}, {0x14,7}, {0x10,7}, {0x1c,8}, 
  {0x0e,6}, {0x0e,8}, {0x0c,8}, {0x02,9}, 
  {0x10,5}, {0x18,8}, {0x14,8}, {0x10,8}, 
  {0x0e,5}, {0x0a,8}, {0x06,8}, {0x06,9}, 
  {0x12,5}, {0x1a,8}, {0x16,8}, {0x12,8}, 
  {0x0d,5}, {0x09,8}, {0x05,8}, {0x05,9}, 
  {0x0c,5}, {0x08,8}, {0x04,8}, {0x04,9},
  {0x07,3}, {0x0a,5}, {0x08,5}, {0x0c,6}
};

/* Table B-11, variable length codes for dmvector
 *
 * treated elsewhere
 */

/* Table B-12, variable length codes for dct_dc_size_luminance
 *
 * indexed by [dct_dc_size_luminance]
 */

static sVLCtable DClumtab[12]=
{
  {0x0004,3}, {0x0000,2}, {0x0001,2}, {0x0005,3}, {0x0006,3}, {0x000e,4},
  {0x001e,5}, {0x003e,6}, {0x007e,7}, {0x00fe,8}, {0x01fe,9}, {0x01ff,9}
};


/* Table B-13, variable length codes for dct_dc_size_chrominance
 *
 * indexed by [dct_dc_size_chrominance]
 */

static sVLCtable DCchromtab[12]=
{
  {0x0000,2}, {0x0001,2}, {0x0002,2}, {0x0006,3}, {0x000e,4}, {0x001e,5},
  {0x003e,6}, {0x007e,7}, {0x00fe,8}, {0x01fe,9}, {0x03fe,10},{0x03ff,10}
};


/* Table B-14, DCT coefficients table zero
 *
 * indexed by [run][level-1]
 * split into two tables (dct_code_tab1, dct_code_tab2) to reduce size
 * 'first DCT coefficient' condition and 'End of Block' are treated elsewhere
 * codes do not include s (sign bit)
 */

static VLCtable dct_code_tab1[2][40]=
{
 /* run = 0, level = 1...40 */
 {
  {0x03, 2}, {0x04, 4}, {0x05, 5}, {0x06, 7},
  {0x26, 8}, {0x21, 8}, {0x0a,10}, {0x1d,12},
  {0x18,12}, {0x13,12}, {0x10,12}, {0x1a,13},
  {0x19,13}, {0x18,13}, {0x17,13}, {0x1f,14},
  {0x1e,14}, {0x1d,14}, {0x1c,14}, {0x1b,14},
  {0x1a,14}, {0x19,14}, {0x18,14}, {0x17,14},
  {0x16,14}, {0x15,14}, {0x14,14}, {0x13,14},
  {0x12,14}, {0x11,14}, {0x10,14}, {0x18,15},
  {0x17,15}, {0x16,15}, {0x15,15}, {0x14,15},
  {0x13,15}, {0x12,15}, {0x11,15}, {0x10,15}
 },
 /* run = 1, level = 1...18 */
 {
  {0x03, 3}, {0x06, 6}, {0x25, 8}, {0x0c,10},
  {0x1b,12}, {0x16,13}, {0x15,13}, {0x1f,15},
  {0x1e,15}, {0x1d,15}, {0x1c,15}, {0x1b,15},
  {0x1a,15}, {0x19,15}, {0x13,16}, {0x12,16},
  {0x11,16}, {0x10,16}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}
 }
};

static VLCtable dct_code_tab2[30][5]=
{
  /* run = 2...31, level = 1...5 */
  {{0x05, 4}, {0x04, 7}, {0x0b,10}, {0x14,12}, {0x14,13}},
  {{0x07, 5}, {0x24, 8}, {0x1c,12}, {0x13,13}, {0x00, 0}},
  {{0x06, 5}, {0x0f,10}, {0x12,12}, {0x00, 0}, {0x00, 0}},
  {{0x07, 6}, {0x09,10}, {0x12,13}, {0x00, 0}, {0x00, 0}},
  {{0x05, 6}, {0x1e,12}, {0x14,16}, {0x00, 0}, {0x00, 0}},
  {{0x04, 6}, {0x15,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x07, 7}, {0x11,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x05, 7}, {0x11,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x27, 8}, {0x10,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x23, 8}, {0x1a,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x22, 8}, {0x19,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x20, 8}, {0x18,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x0e,10}, {0x17,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x0d,10}, {0x16,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x08,10}, {0x15,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1a,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x19,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x17,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x16,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}}
};


/* Table B-15, DCT coefficients table one
 *
 * indexed by [run][level-1]
 * split into two tables (dct_code_tab1a, dct_code_tab2a) to reduce size
 * 'End of Block' is treated elsewhere
 * codes do not include s (sign bit)
 */

static VLCtable dct_code_tab1a[2][40]=
{
 /* run = 0, level = 1...40 */
 {
  {0x02, 2}, {0x06, 3}, {0x07, 4}, {0x1c, 5},
  {0x1d, 5}, {0x05, 6}, {0x04, 6}, {0x7b, 7},
  {0x7c, 7}, {0x23, 8}, {0x22, 8}, {0xfa, 8},
  {0xfb, 8}, {0xfe, 8}, {0xff, 8}, {0x1f,14},
  {0x1e,14}, {0x1d,14}, {0x1c,14}, {0x1b,14},
  {0x1a,14}, {0x19,14}, {0x18,14}, {0x17,14},
  {0x16,14}, {0x15,14}, {0x14,14}, {0x13,14},
  {0x12,14}, {0x11,14}, {0x10,14}, {0x18,15},
  {0x17,15}, {0x16,15}, {0x15,15}, {0x14,15},
  {0x13,15}, {0x12,15}, {0x11,15}, {0x10,15}
 },
 /* run = 1, level = 1...18 */
 {
  {0x02, 3}, {0x06, 5}, {0x79, 7}, {0x27, 8},
  {0x20, 8}, {0x16,13}, {0x15,13}, {0x1f,15},
  {0x1e,15}, {0x1d,15}, {0x1c,15}, {0x1b,15},
  {0x1a,15}, {0x19,15}, {0x13,16}, {0x12,16},
  {0x11,16}, {0x10,16}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}
 }
};

static VLCtable dct_code_tab2a[30][5]=
{
  /* run = 2...31, level = 1...5 */
  {{0x05, 5}, {0x07, 7}, {0xfc, 8}, {0x0c,10}, {0x14,13}},
  {{0x07, 5}, {0x26, 8}, {0x1c,12}, {0x13,13}, {0x00, 0}},
  {{0x06, 6}, {0xfd, 8}, {0x12,12}, {0x00, 0}, {0x00, 0}},
  {{0x07, 6}, {0x04, 9}, {0x12,13}, {0x00, 0}, {0x00, 0}},
  {{0x06, 7}, {0x1e,12}, {0x14,16}, {0x00, 0}, {0x00, 0}},
  {{0x04, 7}, {0x15,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x05, 7}, {0x11,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x78, 7}, {0x11,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x7a, 7}, {0x10,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x21, 8}, {0x1a,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x25, 8}, {0x19,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x24, 8}, {0x18,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x05, 9}, {0x17,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x07, 9}, {0x16,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x0d,10}, {0x15,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1a,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x19,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x17,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x16,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}}
};




/* data from ISO/IEC 13818-2 DIS, Annex B, variable length code tables */

/* Table B-1, variable length codes for macroblock_address_increment
 *
 * indexed by [macroblock_address_increment-1]
 * 'macroblock_escape' is treated elsewhere
 */

static VLCtable addrinctab[33]=
{
  {0x01,1},  {0x03,3},  {0x02,3},  {0x03,4},
  {0x02,4},  {0x03,5},  {0x02,5},  {0x07,7},
  {0x06,7},  {0x0b,8},  {0x0a,8},  {0x09,8},
  {0x08,8},  {0x07,8},  {0x06,8},  {0x17,10},
  {0x16,10}, {0x15,10}, {0x14,10}, {0x13,10},
  {0x12,10}, {0x23,11}, {0x22,11}, {0x21,11},
  {0x20,11}, {0x1f,11}, {0x1e,11}, {0x1d,11},
  {0x1c,11}, {0x1b,11}, {0x1a,11}, {0x19,11},
  {0x18,11}
};


/* Table B-2, B-3, B-4 variable length codes for macroblock_type
 *
 * indexed by [macroblock_type]
 */

static VLCtable mbtypetab[3][32]=
{
 /* I */
 {
  {0,0}, {1,1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {1,2}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
 },
 /* P */
 {
  {0,0}, {3,5}, {1,2}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {1,3}, {0,0}, {1,1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {1,6}, {1,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {2,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
 },
 /* B */
 {
  {0,0}, {3,5}, {0,0}, {0,0}, {2,3}, {0,0}, {3,3}, {0,0},
  {2,4}, {0,0}, {3,4}, {0,0}, {2,2}, {0,0}, {3,2}, {0,0},
  {0,0}, {1,6}, {0,0}, {0,0}, {0,0}, {0,0}, {2,6}, {0,0},
  {0,0}, {0,0}, {3,6}, {0,0}, {0,0}, {0,0}, {2,5}, {0,0}
 }
};




/*
;
; Performs an 8 element Discrete Cosine Transform (DCT) on
; one column of an 8x8 matrix.
;
; Uses the fast 29 add and 5 multiply algorithm by Arai, Agui and
; Nakajima (described in "JPEG Still Image Data Compression Standard"
; by Pennebaker and Mitchell).
;
*/

static void __inline dct_column( float* src, float* dest )
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
; Performs an 8 element Discrete Cosine Transform (DCT) on
; one row of an 8x8 matrix.
;
; Uses the fast 29 add and 5 multiply algorithm by Arai, Agui and
; Nakajima (described in "JPEG Still Image Data Compression Standard"
; by Pennebaker and Mitchell).
;
*/

static void __inline dct_row( float* src, float* dest )
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
; Performs an 8x8 Discrete Cosine Transform (DCT) on the specified
; matrix of work1 and stores the result in work2. The routine uses the
; fast AAN algorithm, which requires scaling of the coefficients
; following the transform.
;
*/

static void dct( float in[8][8],int out[8][8] )
{	float   temp1[8][8];
    float   temp2[8][8];
    int     i;

    dct_column( &in[0][0], &temp1[0][0] );
    dct_column( &in[0][1], &temp1[0][1] );
    dct_column( &in[0][2], &temp1[0][2] );
    dct_column( &in[0][3], &temp1[0][3] );
    dct_column( &in[0][4], &temp1[0][4] );
    dct_column( &in[0][5], &temp1[0][5] );
    dct_column( &in[0][6], &temp1[0][6] );
    dct_column( &in[0][7], &temp1[0][7] );

    dct_row( &temp1[0][0], &temp2[0][0] );
    dct_row( &temp1[1][0], &temp2[1][0] );
    dct_row( &temp1[2][0], &temp2[2][0] );
    dct_row( &temp1[3][0], &temp2[3][0] );
    dct_row( &temp1[4][0], &temp2[4][0] );
    dct_row( &temp1[5][0], &temp2[5][0] );
    dct_row( &temp1[6][0], &temp2[6][0] );
    dct_row( &temp1[7][0], &temp2[7][0] );

    for( i = 0 ; i < 8 ; ++i )
    {	int s2;
		FLOAT_TO_INT(s2,(temp2[i][0] * prescale[i][0]));
		out[i][0] = s2;
		FLOAT_TO_INT(s2,(temp2[i][1] * prescale[i][1]));
		out[i][1] = s2;
		FLOAT_TO_INT(s2,(temp2[i][2] * prescale[i][2]));
		out[i][2] = s2;
		FLOAT_TO_INT(s2,(temp2[i][3] * prescale[i][3]));
		out[i][3] = s2;
		FLOAT_TO_INT(s2,(temp2[i][4] * prescale[i][4]));
		out[i][4] = s2;
		FLOAT_TO_INT(s2,(temp2[i][5] * prescale[i][5]));
		out[i][5] = s2;
		FLOAT_TO_INT(s2,(temp2[i][6] * prescale[i][6]));
		out[i][6] = s2;
		FLOAT_TO_INT(s2,(temp2[i][7] * prescale[i][7]));
		out[i][7] = s2;
	}
}


static void rgb_to_yuv( const ARGB* data, int stride, int index, int n, int m )
{   ARGB    pixel;
    float   cbArray[8][8];
    float   crArray[8][8];
    int     i,j;
	float cr = 0.299f;
 	float cg = 0.587f;
 	float cb = 0.114f;
	float cu = 0.5f/(1.0f-cb);
	float cv = 0.5f/(1.0f-cr);
	
	for( i = 0 ; i < 8 ; ++i )
	{
		for( j = 0 ; j < 8 ; ++j )
		{  
			float y,u,v;
			int yi,ui,vi;
			pixel = data[j];

			y = cr*(float)pixel.r + cg*(float)pixel.g + cb*(float)pixel.b;
			u = cu*((float) pixel.b-y);
			v = cv*((float) pixel.r-y);

			//work1[index][i][j] =  (float) floor( (219.0f/256.0f)*y + 16.5f)-128.0f;  
		    //cbArray[i][j] = (float) floor( (224.0f/256.0f)*u + .5f);    
			//crArray[i][j] = (float) floor( (224.0f/256.0f)*v + .5f); 
			FLOAT_TO_INT(yi,( (219.0f/256.0f)*y + 16.5f)-128.0f);
			FLOAT_TO_INT(ui,( (224.0f/256.0f)*u));
			FLOAT_TO_INT(vi,( (224.0f/256.0f)*v));
			work1[index][i][j] =  (float) yi;
			cbArray[i][j] = (float) ui;
			crArray[i][j] = (float) vi;


		}

		data += stride;
	}
    for( i = 0 ; i < 4 ; ++i )
    {
        for( j = 0 ; j < 4 ; ++j )
        {	float   cb,cr;   
            cb = (cbArray[2*i][2*j] +
                  cbArray[2*i][2*j+1] +
                  cbArray[2*i+1][2*j] +
                  cbArray[2*i+1][2*j+1]) / 4.0f;

            cr = (crArray[2*i][2*j] +
                  crArray[2*i][2*j+1] +
                  crArray[2*i+1][2*j] +
                  crArray[2*i+1][2*j+1]) / 4.0f;
			
            work1[4][n+i][m+j] = cb;
            work1[5][n+i][m+j] = cr;
        }
    }
}


static void alpha_to_yuv( ARGB* data, int stride, int index, int n, int m )
{  
    int     i,j;
	float cr = 0.299f;
 	float cg = 0.587f;
 	float cb = 0.114f;
	//float cu = 0.5f/(1.0f-cb);
	//float cv = 0.5f/(1.0f-cr);
	
	for( i = 0 ; i < 8 ; ++i )
	{
		for( j = 0 ; j < 8 ; ++j )
		{
			//pixel.r = data[j].a;
			//pixel.g = data[j].a;
			//pixel.b = data[j].a;
			//y = cr*(float)pixel.r + cg*(float)pixel.g + cb*(float)pixel.b;
			//FLOAT_TO_INT(yi,( (219.0f/256.0f)*y + 16.5f)-128.0f);

			work1[index][i][j] =  ((float) data[j].a);

			//work1[index][i][j] =  ((float) data[j].a);
		}

		data += stride;
	}
    for( i = 0 ; i < 4 ; ++i )
    {   for( j = 0 ; j < 4 ; ++j )
        {   work1[4][n+i][m+j] = 0.0f;
            work1[5][n+i][m+j] = 0.0f;
        }
    }
}


/* Test Model 5 quantization
 *
 * this quantizer has a bias of 1/8 stepsize towards zero
 * (except for the DC coefficient)
 */



static void quantize(int *src,int *dst,int dc_prec,unsigned char*quant_mat,int mquant)
{	int i,x, y, d;

	x = src[0];
	d = 8>>dc_prec; 
	dst[0] = (x>=0) ? (x+(d>>1))/d : -((-x+(d>>1))/d); /* round(x/d) */

	for (i=1; i<64; i++)
	{
		x = src[i];
		d = quant_mat[i];
		y = (32*(x>=0 ? x : -x) + (d>>1))/d; /* round(32*x/quant_mat) */
		d = (3*mquant+2)>>2;
		y = (y+d)/(2*mquant); /* (y+0.75*mquant) / (2*mquant) */

		/* clip to syntax limits */
		if (y > 255)
		{	if (1)			// if (mpeg1)
				y = 255;
			else if (y > 2047)
				y = 2047;
		}

		dst[i] = (x>=0) ? y : -y;

	}
	return;
}




static void write_bits_start(unsigned char *dest)
{   cdataBuffer = dest;
    shiftReg = 0;
    bitCount = 0;
    byteCount = 0;
}

static void __inline write_bits( unsigned int code, int bits )
{	int i;
	unsigned int mask = 1 << (bits-1); /* selects first (leftmost) bit */
	for (i=0; i<bits; i++)
	{	shiftReg <<= 1;
		if (code & mask)
			shiftReg|= 1;
		mask >>= 1; /* select next bit */
		bitCount++;
		if (bitCount==8) 
		{	cdataBuffer[byteCount++] = (unsigned char) (shiftReg );		
			bitCount = 0;
    
		}
	}	
}



static void write_bits_align(void)
{	if (bitCount!=0)
		write_bits(0,8-bitCount);
 
}



static int bits_written(void)
{	return (8*byteCount) + bitCount;
}





/* generate variable length code for DC coefficient (7.2.1) */

static void putDC(sVLCtable *tab,int val)
{	int absval, size;

	absval = (val<0) ? -val : val; /* abs(val) */

	if (absval>255)
	{	/* should never happen */
		printf("warning:: DC value out of range (%d)\n",val);
		absval = 255;	
	}

	/* compute dct_dc_size */
	size = 0;

	while (absval)
	{	absval >>= 1;
		size++;
	}

	/* generate VLC for dct_dc_size (Table B-12 or B-13) */
	write_bits(tab[size].code,tab[size].len);

	/* append fixed length code (dc_dct_differential) */
	if (size!=0)
	{
		if (val>=0)
			absval = val;
		else
			absval = val + (1<<size) - 1; /* val + (2 ^ size) - 1 */
		write_bits(absval,size);
	}
}


/* generate variable length code for luminance DC coefficient */

static void __inline putDClum(int val)
{	putDC(DClumtab,val);
}

/* generate variable length code for chrominance DC coefficient */

static void __inline putDCchrom(int val)
{	putDC(DCchromtab,val);
}


/*
;
; Compresses the contents of dctBuffer. The coefficients are removed
; in zigzag order, compressed and moved to the output buffer through
; a shift register. The DC coefficient is always stored directly.
; Subsequent coefficients are encoded as follows:
;
*/


static void __inline putAC(int run,int signed_level)

{	int level, len;
	VLCtable *ptab;

	level = (signed_level<0) ? -signed_level : signed_level; /* abs(signed_level) */

	/* make sure run and level are valid */

	if (run<0 || run>63 || level==0 ||level>255)
	{
		printf("warning:: AC value out of range (run=%d, signed_level=%d)\n",run,signed_level);
		level = 255;

	}

	len = 0;

	if (run<2 && level<41)
	{
		/* vlcformat selects either of Table B-14 / B-15 */
		ptab = &dct_code_tab1[run][level-1];

		len = ptab->len;
	}
	else if (run<32 && level<6)
	{
		/* vlcformat selects either of Table B-14 / B-15 */
	
			ptab = &dct_code_tab2[run-2][level-1];

		len = ptab->len;
	}

	if (len!=0) /* a VLC code exists */
	{	write_bits(ptab->code,len);
		write_bits(signed_level<0,1); /* sign */
	}
	else
	{   /* no VLC for this (run, level) combination: use escape coding  */
		write_bits(1l,6); /* Escape */
		write_bits(run,6); /* 6 bit code for run */
		/* ISO/IEC 11172-2 uses a 8 or 16 bit code */
		if (signed_level>127)
			write_bits(0,8);
		if (signed_level<-127)
			write_bits(128,8);
		write_bits(signed_level,8);

	}
}

static void compress( int *dctBuffer,int index )
{	int n,  signed_level;
	int run = 0;
	int dct_diff;
	
	if ((index <=3))
	{	dct_diff = dctBuffer[0] - dc_dct_pred[0]; 
		dc_dct_pred[0] = dctBuffer[0];	
		putDClum(dct_diff);
	} else
	{	dct_diff = dctBuffer[0] - dc_dct_pred[(index&1)+1]; 
		dc_dct_pred[(index&1)+1] = dctBuffer[0];	
		putDCchrom(dct_diff);
	}

	/* AC coefficients  */
	
	for (n=1; n<64; n++)
	{	/* use appropriate entropy scanning pattern */
		signed_level = dctBuffer[zigzag[n]];
		if (signed_level!=0)
		{	putAC(run,signed_level);
			run = 0;
		}	else
			run++; /* count zero coefficients */
	}

    write_bits(2,2); /* 10 (Table B-14) End of Block -- normative block punctuation */
}

/* generate variable length code for first coefficient
 * of a non-intra block (7.2.2.2) */
void putACfirst(int run,int val)
{	if (run==0 && (val==1 || val==-1)) /* these are treated differently */
		write_bits(2|((val<0) ? 1 : 0),2); /* generate '1s' (s=sign), (Table B-14, line 2) */
	else
		putAC(run,val); /* no difference for all others */
}

/* generate variable length codes for a non-intra-coded block (6.2.6, 6.3.17) */

void compress_non_intra(int *blk)
{	int n, run, signed_level, first;
	run = 0;
	first = 1;

	for (n=0; n<64; n++)
	{	/* use appropriate entropy scanning pattern */
		signed_level = blk[zigzag[n]];

		if (signed_level!=0)
		{
			if (first)
			{	/* first coefficient in non-intra block */
				putACfirst(run,signed_level);
				first = 0;
			} else
				putAC(run,signed_level);

			run = 0;
		} else
			run++; /* count zero coefficients */
	}

	/* End of Block -- normative block punctuation  */
	write_bits(2,2);
}


/* 
 * generate variable length code for macroblock_address_increment (6.3.16) 
 */

static void putaddrinc(int addrinc)
{	while (addrinc>33)
	{
		write_bits(0x08,11); /* macroblock_escape */
		addrinc-= 33;
	}

	write_bits(addrinctab[addrinc-1].code,addrinctab[addrinc-1].len);
}

void putcbp(int cbp)
{
	write_bits(cbptable[cbp].code,cbptable[cbp].len);
}

/* 
 * generate variable length code for macroblock_type (6.3.16.1) 
 */

static void putmbtype(int pict_type,int mb_type)
{	write_bits(mbtypetab[pict_type-1][mb_type].code,
          mbtypetab[pict_type-1][mb_type].len);
}


static void idec_initencoder( void )
{	int i, j;

    a1 = (float) (cos(2.0*(PI/8.0)));
    a2 = (float) (cos(PI/8.0) - cos(3.0*(PI/8.0)));
    a3 = (float) (cos(PI/8.0) + cos(3.0*(PI/8.0)));
    a4 = (float) (cos(3.0*(PI/8.0)));

    for( i = 0 ; i < 8 ; ++i )
    {
        for( j = 0 ; j < 8 ; ++j )
        {
            prescale[i][j] = 1.0f;

            if( i == 0 )
            {
                prescale[i][j] /= (float) sqrt(8.0);
            }
            else
            {
                prescale[i][j] /= (float)(4.0*cos(i*(PI/16.0)));
            }

            if( j == 0 )
            {
                prescale[i][j] /= (float)sqrt(8.0);
            }
            else
            {
                prescale[i][j] /= (float)(4.0*cos(j*(PI/16.0)));
            }
        }
    }

}




static void idec_encodeblock( int firstblock,int mquant )
{
    int     i;
	if (firstblock)
		putmbtype(I_TYPE,MB_INTRA|MB_QUANT); // macroblock type
	else
		putmbtype(I_TYPE,MB_INTRA);

	//write_bits(1,0);						// DCT type

	if (firstblock)
	{	write_bits(mquant,5);			// QSC
	} else
		putaddrinc(1);				 // macroblock_address_increment 


    for( i = 0 ; i < 6 ; ++i )
    {	dct( work1[i],work2[i] );
		quantize(&work2[i][0][0],&work2[i][0][0],0,default_intra_quantizer_matrix,mquant<<1);
	
	    compress( &work2[i][0][0], i);
    }

	
}

static void bdec_encodeblock( int firstblock,int mquant )
{
    int     i;

    for( i = 0 ; i < 6 ; ++i )
    {	dct( work1[i],work2[i] );
		quantize(&work2[i][0][0],&work2[i][0][0],0,default_intra_quantizer_matrix,mquant<<1);

		compress( &work2[i][0][0], i);


    }

}

static void alpha_outputraw( ARGB* data, int stride,int alpha8bit)
{	int     i,j;

	for( i = 0 ; i < 16 ; ++i )
	{
		for( j = 0 ; j < 16 ; j+=2 )
		{	if (alpha8bit)
			{	write_bits ((unsigned int)data[j].a>>1,8);
				write_bits ((unsigned int)data[j+1].a>>1,8);

			}
			else
			{	write_bits ((unsigned int)data[j+1].a>>1,4);
				write_bits ((unsigned int)data[j].a>>1,4);

			}
		}

		data += stride;
	}

}






int CDECL IDEC_encode( const ARGB *image,unsigned char *output,int width,int height, int pitch,int flags,int mquant,int alphatype )
{	static int init = 0;	
	IDEC_HEADER *header;
    int i,j;
	int bytes;
	int real_width,real_height;
	int macroblock_count;
	int firstblock;
	if (!init)
	{	idec_initencoder();
		init = 1;
	}

	real_width = width;
	real_height = height;

	width =  ((width)+15)   & 0xfffffff0;
	height = ((height)+15) & 0xfffffff0;

	write_bits_start( output);
	// write dummy header
	write_bits(0,32);
	write_bits(0,32);

	// Encode RGB Image

	macroblock_count = (width>>4) * (height>>4);
	dc_dct_pred[0] = dc_dct_pred[1] = dc_dct_pred[2] = 0;
	firstblock = 1;

	for( j = 0 ; j < width ; j += 16 )
	{	for( i = 0 ; i < height ; i += 16 )
	    {
			const ARGB*     src= &image[i*(pitch>>2)+j];

			rgb_to_yuv( src,           width, 0, 0, 0 );
			rgb_to_yuv( src+8,         width, 1, 0, 4 );
			rgb_to_yuv( src+8*width,   width, 2, 4, 0 );
			rgb_to_yuv( src+8*width+8, width, 3, 4, 4 );
			idec_encodeblock( firstblock,mquant );

			//	else if (alphatype == IDEC_ALPHA_BDEC)
			//	{	alpha_to_yuv( src,           width, 0, 0, 0 );
			//		alpha_to_yuv( src+8,         width, 1, 0, 4 );
			//		alpha_to_yuv( src+8*width,   width, 2, 4, 0 );
			//		alpha_to_yuv( src+8*width+8, width, 3, 4, 4 );
			//		bdec_encodeblock( firstblock,mquant  );


			firstblock = 0;
			macroblock_count--;
	    }
	}

	write_bits_align();
	write_bits(0x00000134,32);

	bytes = bits_written()/8;
	while (bytes & 15) { write_bits(0,8); bytes++;}

	if (flags & IDECFLAG_HASALPHA)
	{
		{	for( i = 0 ; i < height ; i ++ )
			{	const ARGB*     data= &image[i*(pitch>>2)];
				for( j = 0 ; j < width ; j ++)
				{
					if (alphatype == IDEC_ALPHA8BITRAW)
					{
						write_bits ((unsigned int)data[j].a,8);
					}
					else if (alphatype == IDEC_ALPHA4BITRAW)
					{
						write_bits ((unsigned int)data[j].a>>4,4);
					}
				}
			}
		}
	}

	write_bits(0,32);   // pad with 128 zero bits - fb dev 5, 2000
	write_bits(0,32);
	write_bits(0,32);
	write_bits(0,32);



	bytes = (bits_written()+7)/8;

	header = (IDEC_HEADER *) output;
	header->id = IDEC_HEADER_ID;
	header->size = bytes;
	header->flags = flags;
	header->alpha_type = alphatype;
	header->alpha_quant = mquant;
	header->width = width>>4;
	header->height = height>>4;

	return (bytes);

}

