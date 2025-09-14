// (c) 2000-2007 Electronic Arts Inc.
//
//
//		TQI444 Encoder
//
//		Martin Griffiths, Bullfrog Productions Ltd.
//
//
//		Last Updated : 18th May 1999


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tqi444encode.h"

#define REFERENCE_DCT (0)
#define VERIFY_ENCODE (0)

#ifndef PI
#define PI (3.14159265358979323846)
#endif

//#define FLOAT_TO_INT(_i,_f){float _ff = _f; __asm fld _ff __asm fistp dword ptr _i}
#define FLOAT_TO_INT(_i,_f) (_i = (int)(((float)(_f))+((((float)(_f))>=0.0f)?0.5f:-0.5f)))

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


static float         work1[4][8][8]; // input data
static int           work2[4][8][8]; 

static signed int      dctBuffer[64];  // DCT results to compress

static unsigned char *cdataBuffer;      // compressed data
static unsigned int    shiftReg;       // shift register
static unsigned int             bitCount;       // bit count in shift register
static unsigned int            byteCount;      // bytes output from shift register

static float            a1,a2,a3,a4;



#if REFERENCE_DCT
/* private data */
static double c[8][8]; /* transform coefficients */

#else

static float            prescale[8][8];

#endif
// Zigzag matrix used to extract DCT coefficients for compression.

static int zigzag[64] =
{
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
    19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
    42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
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
; Performs an 8 element Inverse Discrete Cosine Transform (IDCT) on
; one column of an 8x8 matrix.
;
; Uses the fast 29 add and 5 multiply algorithm by Arai, Agui and
; Nakajima (described in "JPEG Still Image Data Compression Standard"
; by Pennebaker and Mitchell).
;
*/

static void idct_column( float* src, float* dest )
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

/*
;
; Performs an 8 element Inverse Discrete Cosine Transform (IDCT) on
; one row of an 8x8 matrix.
;
; Uses the fast 29 add and 5 multiply algorithm by Arai, Agui and
; Nakajima (described in "JPEG Still Image Data Compression Standard"
; by Pennebaker and Mitchell).
;
*/

static void idct_row( float* src, float* dest )
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



/*
;
; Performs an 8x8 Discrete Cosine Transform (DCT) on the specified
; matrix of work1 and stores the result in work2. The routine uses the
; fast AAN algorithm, which requires scaling of the coefficients
; following the transform.
;
*/

static void dct( float in[8][8],int out[8][8] )
{	
#if REFERENCE_DCT
	int i, j;
	
	float tmp[8][8];

	for (i=0; i<8; i++)
	{		for (j=0; j<8; j++)
			{	float s =  c[j][0] * in[i][0];
				s += c[j][1] * in[i][1];
				s += c[j][2] * in[i][2];
				s += c[j][3] * in[i][3];
				s += c[j][4] * in[i][4];
				s += c[j][5] * in[i][5];
				s += c[j][6] * in[i][6];
				s += c[j][7] * in[i][7];
				tmp[i][j] = (float) s;
			}
	}

	for (j=0; j<8; j++)
	{		for (i=0; i<8; i++)
			{	int s2;
				float s =  c[i][0] * tmp[0][j];
				s += c[i][1] * tmp[1][j];
				s += c[i][2] * tmp[2][j];
				s += c[i][3] * tmp[3][j];
				s += c[i][4] * tmp[4][j];
				s += c[i][5] * tmp[5][j];
				s += c[i][6] * tmp[6][j];
				s += c[i][7] * tmp[7][j];
				s = (float) s/8.0;    
				FLOAT_TO_INT(s2,(float)s);
				out[i][j] = s2;
			}
	}
#else

	float   temp1[8][8];
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
#endif

}




static void rgb_to_workrgb( ARGB* data, int stride,int width,int height)
{   int i,j;
	for(  i = 0 ; i < 8 ; i++ )
	{	if ( i < height) 
		{
			for ( j = 0 ; j < 8 ; j++)
			{	if (j < width)
				{	work1[0][i][j] =(float)data[j].r;  
					work1[1][i][j] =(float)data[j].g; 
					work1[2][i][j] =(float)data[j].b; 
					work1[3][i][j] =(float)data[j].a; 
				} else
				{	work1[0][i][j] =(float)0.0f;  
					work1[1][i][j] =(float)0.0f; 
					work1[2][i][j] =(float)0.0f; 
					work1[3][i][j] =(float)0.0f; 
				}
			}	
		} else
		{	work1[0][i][0] =0.0f;  
			work1[1][i][0] =0.0f; 
			work1[2][i][0] =0.0f; 
			work1[3][i][0] =0.0f; 
			work1[0][i][1] =0.0f; 
			work1[1][i][1] =0.0f; 
			work1[2][i][1] =0.0f; 
			work1[3][i][1] =0.0f; 
			work1[0][i][2] =0.0f;  
			work1[1][i][2] =0.0f; 
			work1[2][i][2] =0.0f; 
			work1[3][i][2] =0.0f; 
			work1[0][i][3] =0.0f; 
			work1[1][i][3] =0.0f; 
			work1[2][i][3] =0.0f; 
			work1[3][i][3] =0.0f; 
			work1[0][i][4] =0.0f;
			work1[1][i][4] =0.0f;
			work1[2][i][4] =0.0f;
			work1[3][i][4] =0.0f;
			work1[0][i][5] =0.0f;
			work1[1][i][5] =0.0f;
			work1[2][i][5] =0.0f;
			work1[3][i][5] =0.0f;
			work1[0][i][6] =0.0f;
			work1[1][i][6] =0.0f;
			work1[2][i][6] =0.0f;
			work1[3][i][6] =0.0f;
			work1[0][i][7] =0.0f;
			work1[1][i][7] =0.0f;
			work1[2][i][7] =0.0f;
			work1[3][i][7] =0.0f;
		}

		data += stride;
	}


}


static void rgb_to_workyuv( ARGB* data, int stride,int width,int height)
{   int i,j;
	float cr = 0.299f;
 	float cg = 0.587f;
 	float cb = 0.114f;
	float cu = 0.5f/(1.0f-cb);
	float cv = 0.5f/(1.0f-cr);

	for(  i = 0 ; i < 8 ; i++ )
	{	if ( i < height) 
		{
			for ( j = 0 ; j < 8 ; j++)
			{	if (j < width)
				{	float y,u,v;
					ARGB pixel;
					pixel = data[j];

					y = cr*(float)pixel.r + cg*(float)pixel.g + cb*(float)pixel.b;
					u = cu*((float) pixel.b-y);
					v = cv*((float) pixel.r-y);
				
					work1[0][i][j] =(float) (219.0f/256.0f)*y + 16.0f;  /* nominal range: 16..235 */
					work1[1][i][j] =(float) (224.0f/256.0f)*u + 128.0f; /* 16..240 */  
					work1[2][i][j] =(float) (224.0f/256.0f)*v + 128.0f; /* 16..240 */
									
					//work1[0][i][j] =(float) y; 
					//work1[1][i][j] =(float) u; 
					//work1[2][i][j] =(float) v; 

					work1[3][i][j] =(float)data[j].a; 
				} else
				{	work1[0][i][j] =(float)0.0f;  
					work1[1][i][j] =(float)0.0f; 
					work1[2][i][j] =(float)0.0f; 
					work1[3][i][j] =(float)0.0f; 
				}
			}	
		} else
		{	work1[0][i][0] =0.0f;  
			work1[1][i][0] =0.0f; 
			work1[2][i][0] =0.0f; 
			work1[3][i][0] =0.0f; 
			work1[0][i][1] =0.0f; 
			work1[1][i][1] =0.0f; 
			work1[2][i][1] =0.0f; 
			work1[3][i][1] =0.0f; 
			work1[0][i][2] =0.0f;  
			work1[1][i][2] =0.0f; 
			work1[2][i][2] =0.0f; 
			work1[3][i][2] =0.0f; 
			work1[0][i][3] =0.0f; 
			work1[1][i][3] =0.0f; 
			work1[2][i][3] =0.0f; 
			work1[3][i][3] =0.0f; 
			work1[0][i][4] =0.0f;
			work1[1][i][4] =0.0f;
			work1[2][i][4] =0.0f;
			work1[3][i][4] =0.0f;
			work1[0][i][5] =0.0f;
			work1[1][i][5] =0.0f;
			work1[2][i][5] =0.0f;
			work1[3][i][5] =0.0f;
			work1[0][i][6] =0.0f;
			work1[1][i][6] =0.0f;
			work1[2][i][6] =0.0f;
			work1[3][i][6] =0.0f;
			work1[0][i][7] =0.0f;
			work1[1][i][7] =0.0f;
			work1[2][i][7] =0.0f;
			work1[3][i][7] =0.0f;
		}

		data += stride;
	}

}

#if VERIFY_ENCODE

static unsigned char * verify_encode_cdata_baseptr;

#endif



static void write_bits_start(unsigned char *dest)
{   cdataBuffer = dest;
#if VERIFY_ENCODE
	verify_encode_cdata_baseptr=dest;
#endif
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
		if (bitCount==32) /* 8 bit buffer full */
		{	cdataBuffer[byteCount++] = (unsigned char) (shiftReg    );
			cdataBuffer[byteCount++] = (unsigned char) (shiftReg>>8 );
			cdataBuffer[byteCount++] = (unsigned char) (shiftReg>>16);
			cdataBuffer[byteCount++] = (unsigned char) (shiftReg>>24);
			bitCount = 0;
    
		}
	}	
}

static void __inline place_current_bits(void)
{	unsigned int sreg = shiftReg << (32-bitCount);
	cdataBuffer[byteCount] = (unsigned char) (sreg    );
	cdataBuffer[byteCount+1] = (unsigned char) (sreg >>8 );
	cdataBuffer[byteCount+2] = (unsigned char) (sreg >>16);
	cdataBuffer[byteCount+3] = (unsigned char) (sreg>>24);


}


static void write_bits_end(void)
{	if (bitCount!=0)
		write_bits(0,32-bitCount);
 
}

static int bits_written(void)
{	return (8*byteCount) + bitCount;
}

#if VERIFY_ENCODE

extern "C"
{
	extern int tqi444_DecompressTest(unsigned char *data,int);
	extern int tqi444_work_verify[64];
};

static void Verify_Block(int block[64],int bitpos)
{	tqi444_DecompressTest(verify_encode_cdata_baseptr,bitpos);
	for (int i = 0 ; i < 8 ; i++)
	{	for (int j = 0 ; j < 8 ; j++)
		{	if (block[i*8+j] != tqi444_work_verify[j*8+i])
			{	printf("decode error (%d,%d) val = %d decoded = %d\n",i,j,block[i*8+j],tqi444_work_verify[i*8+j]);
			}
		}
	}

}

#endif






/* generate variable length code for DC coefficient (7.2.1) */

static void putDC(sVLCtable *tab,int val)
{	int absval, size;

	absval = (val<0) ? -val : val; /* abs(val) */

	if (absval>255)
	{	/* should never happen */
		printf("warning:: DC value out of range (%d)\n",val);
		absval = 255;
		//exit(1);
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

void __inline putDCchrom(int val)
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
		printf("AC value out of range (run=%d, signed_level=%d)\n",run,signed_level);
		exit(1);

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

static void compress( int *dctBuffer )
{	int n,  signed_level;
	int run = 0;

#if VERIFY_ENCODE
	int bitpos = bits_written();
#endif

	if ( (dctBuffer[0] > 255) ||  (dctBuffer[0] < 0) )
	{	printf("warning! DC out of range :%d\n",dctBuffer[0]);
	}

	write_bits(dctBuffer[0],8);

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

#if VERIFY_ENCODE
	place_current_bits();
	Verify_Block(dctBuffer,bitpos);
#endif

}


static void tqi_initencoder( void )
{
  int i, j;
#if REFERENCE_DCT
  double s;
  for (i=0; i<8; i++)
  {
    s = (i==0) ? sqrt(0.125) : 0.5;

    for (j=0; j<8; j++)
      c[i][j] = s * cos((PI/8.0)*i*(j+0.5));
  }
#else
    a1 = (float) (cos(2.0*(PI/8.0)));
    a2 = (float) (cos(PI/8.0) - cos(3.0*(PI/8.0)));
    a3 = (float) (cos(PI/8.0) + cos(3.0*(PI/8.0)));
    a4 = (float) (cos(3.0*(PI/8.0)));

    for( i = 0 ; i < 8 ; ++i )
    {
        for( j = 0 ; j < 8 ; ++j )
        {
            prescale[i][j] = 1.0/8.0;

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

#endif

}



static int encodeblock( int has_alpha)
{

	dct( work1[0],work2[0] );
	compress( &work2[0][0][0] );


	dct( work1[1],work2[1] );
	compress( &work2[1][0][0] );


	dct( work1[2],work2[2] );
	compress( &work2[2][0][0] );

	if (has_alpha)
	{	dct( work1[3],work2[3] );
	    compress( &work2[3][0][0] );
	}

	return 0;
}


int tqi444_encode( ARGB *image,unsigned char *output,int width,int height, int pitch,ARGB ColourKey,int flags )
{	static int init = 0;	
    int i,j;
	int bytes;
	int real_width,real_height;
	int tbwidth,tbheight;
	
	if (!init)
	{	tqi_initencoder();
		init = 1;
	}

	ColourKey.r = 255;
	ColourKey.g = 0;
	ColourKey.b = 255;
	ColourKey.a = 0;

	output[0] = width % 256;
	output[1] = width / 256;
	output[2] = height % 256;
	output[3] = height / 256;
	output[4] = 100 | (((flags & TQI444_ALPHA) ? 1 : 0) << 7) | ((flags & TQI444_COLOURKEY) ? 0 : 1) | (((flags & TQI444_YUV) ? 1 : 0) << 4);	// crap code alert

	if (flags & TQI444_COLOURKEY)
	{	output[5] = ColourKey.r;
		output[6] = ColourKey.g;
		output[7] = ColourKey.b;
	} else
	{	output[5] = 0;
		output[6] = 0;
		output[7] = 0;
	}

	real_width = width;
	real_height = height;

	width =  ((width)+7)   & 0xfffffff8;
	height = ((height)+7) & 0xfffffff8;


	write_bits_start( output+8);

    for( i = 0 ; i < height ; i += 8 )
    {
		if ((i+8) > real_height)
			tbheight = real_height - i;
		else
			tbheight = 8;

        for( j = 0 ; j < width ; j += 8 )
        {	int x,y;					
			ARGB*     src= &image[i*(pitch>>2)+j];
	
			if ((j+8) > real_width)
				tbwidth = real_width - j;
			else
				tbwidth = 8;
			if (flags & TQI444_YUV)
				rgb_to_workyuv( src, pitch>>2 ,tbwidth,tbheight);
			else
				rgb_to_workrgb( src, pitch>>2 ,tbwidth,tbheight);

			encodeblock(flags & TQI444_ALPHA );

			if (flags & TQI444_COLOURKEY)						
			{	unsigned int bitmask[8];
				unsigned int totmask=0;
				for (y = 0 ; y < tbheight ; y++)
				{	bitmask[y] =0;
					for (x = 0 ; x < tbwidth ; x++)
					{	ARGB col = *(image+((pitch>>2)*(i+y)+(x+j)));
						bitmask[y] <<=1;
						if ((col.r == ColourKey.r) && (col.g == ColourKey.g) && (col.b == ColourKey.b))
							bitmask[y] |=1;	

					}
					totmask|=bitmask[y];
				
				}
				if (totmask)
				{	write_bits(1,1);
					for (y = 0 ; y < 8 ; y++)
					{	write_bits(bitmask[y],8);
					
					}
				} else
				{	write_bits(0,1);
				}
			}
			
        }

    }
	write_bits_end();

	bytes = bits_written()/8;

	return (bytes+8);

}

