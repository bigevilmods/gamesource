// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  GTF - Graphics Texture Format                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 061205 FB based on xpr                                   */
/*    1.10 061207 FB added u555, f32x4                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: gtfread.cpp gtfwrite.cpp gtfgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* GTF files are generated with dds2gtf from the ps3 sdk            */
/* Structure of file                                                */
/*   gtf header 12 bytes. version, size, numtexture                 */
/*   texture attributes are 9 dwords (36 bytes) per entry.          */
/*      id           4                                              */
/*      offsettotex  4                                              */
/*      texturesize  4                                              */
/*      CellGcmTexture 6*4=24                                       */
/*                                                                  */
/* CellGcmTexture                                                   */
/*     format        1                                              */
/*     mipmap        1                                              */
/*     dimension     1                                              */
/*     cubemap       1                                              */
/*     remap         4                                              */
/*     width         2                                              */
/*     height        2                                              */
/*     depth         2                                              */
/*     location      1                                              */
/*     padding       1                                              */
/*     pitch         4                                              */
/*     offset        4                                              */
/*                                                                  */
/* texels begin add offsettotex which is on 128 boundary            */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "gtfgimex.h"
#include "codex.h"
#include "dxtcodex.h"


typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
uint32_t half_to_float( uint16_t h );


// Branch-free implementation of half-precision (16 bit) floating point
// Copyright 2006 Mike Acton <macton@gmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE
//
// Half-precision floating point format
// ------------------------------------
//
//   | Field    | Last | First | Note
//   |----------|------|-------|----------
//   | Sign     | 15   | 15    |
//   | Exponent | 14   | 10    | Bias = 15
//   | Mantissa | 9    | 0     |
//
// Compiling
// ---------
//
//  Preferred compile flags for GCC: 
//     -O3 -fstrict-aliasing -std=c99 -pedantic -Wall -Wstrict-aliasing
//
//     This file is a C99 source file, intended to be compiled with a C99 
//     compliant compiler. However, for the moment it remains combatible
//     with C++98. Therefore if you are using a compiler that poorly implements
//     C standards (e.g. MSVC), it may be compiled as C++. This is not
//     guaranteed for future versions. 
//
// Features
// --------
//
//  * QNaN + <x>  = QNaN
//  * <x>  + +INF = +INF
//  * <x>  - -INF = -INF
//  * INF  - INF  = SNaN
//  * Denormalized values
//  * Difference of ZEROs is always +ZERO
//  * Sum round with guard + round + sticky bit (grs)
//  * And of course... no branching
// 
// Precision of Sum
// ----------------
//
//  (SUM)        uint16_t z = half_add( x, y );
//  (DIFFERENCE) uint16_t z = half_add( x, -y );
//
//     Will have exactly (0 ulps difference) the same result as:
//     (For 32 bit IEEE 784 floating point and same rounding mode)
//
//     union FLOAT_32
//     {
//       float    f32;
//       uint32_t u32;
//     };
//
//     union FLOAT_32 fx = { .u32 = half_to_float( x ) };
//     union FLOAT_32 fy = { .u32 = half_to_float( y ) };
//     union FLOAT_32 fz = { .f32 = fx.f32 + fy.f32    };
//     uint16_t       z  = float_to_half( fz );
//

// Load immediate
static inline uint32_t _uint32_li( uint32_t a )
{
  return (a);
}

// Decrement
static inline uint32_t _uint32_dec( uint32_t a )
{
  return (a - 1);
}

// Complement
static inline uint32_t _uint32_not( uint32_t a )
{
  return (~a);
}

// Negate
static inline uint32_t _uint32_neg( uint32_t a )
{
  return (uint32_t)(-(int)a);
}

// Extend sign
static inline uint32_t _uint32_ext( uint32_t a )
{
  return (((int32_t)a)>>31);
}

// And
static inline uint32_t _uint32_and( uint32_t a, uint32_t b )
{
  return (a & b);
}

// And with Complement
static inline uint32_t _uint32_andc( uint32_t a, uint32_t b )
{
  return (a & ~b);
}

// Or
static inline uint32_t _uint32_or( uint32_t a, uint32_t b )
{
  return (a | b);
}

// Shift Right Logical
static inline uint32_t _uint32_srl( uint32_t a, int sa )
{
  return (a >> sa);
}

// Shift Left Logical
static inline uint32_t _uint32_sll( uint32_t a, int sa )
{
  return (a << sa);
}

// Add
static inline uint32_t _uint32_add( uint32_t a, uint32_t b )
{
  return (a + b);
}

// Subtract
static inline uint32_t _uint32_sub( uint32_t a, uint32_t b )
{
  return (a - b);
}

// Select on Sign bit
static inline uint32_t _uint32_sels( uint32_t test, uint32_t a, uint32_t b )
{
  const uint32_t mask   = _uint32_ext( test );
  const uint32_t sel_a  = _uint32_and(  a,     mask  );
  const uint32_t sel_b  = _uint32_andc( b,     mask  );
  const uint32_t result = _uint32_or(   sel_a, sel_b );

  return (result);
}

// Load Immediate
static inline uint16_t _uint16_li( uint16_t a )
{
  return (a);
}

// Extend sign
static inline uint16_t _uint16_ext( uint16_t a )
{
  return (((int16_t)a)>>15);
}

// Negate
static inline uint16_t _uint16_neg( uint16_t a )
{
  return (-a);
}

// Complement
static inline uint16_t _uint16_not( uint16_t a )
{
  return (~a);
}

// Decrement
static inline uint16_t _uint16_dec( uint16_t a )
{
  return (a - 1);
}

// Shift Left Logical
static inline uint16_t _uint16_sll( uint16_t a, int sa )
{
  return (a << sa);
}

// Shift Right Logical
static inline uint16_t _uint16_srl( uint16_t a, int sa )
{
  return (a >> sa);
}

// Add
static inline uint16_t _uint16_add( uint16_t a, uint16_t b )
{
  return (a + b);
}

// Subtract
static inline uint16_t _uint16_sub( uint16_t a, uint16_t b )
{
  return (a - b);
}

// And
static inline uint16_t _uint16_and( uint16_t a, uint16_t b )
{
  return (a & b);
}

// Or
static inline uint16_t _uint16_or( uint16_t a, uint16_t b )
{
  return (a | b);
}

// Exclusive Or
static inline uint16_t _uint16_xor( uint16_t a, uint16_t b )
{
  return (a ^ b);
}

// And with Complement
static inline uint16_t _uint16_andc( uint16_t a, uint16_t b )
{
  return (a & ~b);
}

// And then Shift Right Logical
static inline uint16_t _uint16_andsrl( uint16_t a, uint16_t b, int sa )
{
  return ((a & b) >> sa);
}

// Shift Right Logical then Mask
static inline uint16_t _uint16_srlm( uint16_t a, int sa, uint16_t mask )
{
  return ((a >> sa) & mask);
}

// Add then Mask
static inline uint16_t _uint16_addm( uint16_t a, uint16_t b, uint16_t mask )
{
  return ((a + b) & mask);
}


// Select on Sign bit
static inline uint16_t _uint16_sels( uint16_t test, uint16_t a, uint16_t b )
{
  const uint16_t mask   = _uint16_ext( test );
  const uint16_t sel_a  = _uint16_and(  a,     mask  );
  const uint16_t sel_b  = _uint16_andc( b,     mask  );
  const uint16_t result = _uint16_or(   sel_a, sel_b );

  return (result);
}

// Count Leading Zeros
static inline uint32_t _uint32_cntlz( uint32_t x )
{
#ifdef __GNUC__
  /* On PowerPC, this will map to insn: cntlzw */
  /* On Pentium, this will map to insn: clz    */
  uint32_t nlz = __builtin_clz( x );
  return (nlz);
#else
  const uint32_t x0  = _uint32_srl(  x,  1 );
  const uint32_t x1  = _uint32_or(   x,  x0 );
  const uint32_t x2  = _uint32_srl(  x1, 2 );
  const uint32_t x3  = _uint32_or(   x1, x2 );
  const uint32_t x4  = _uint32_srl(  x3, 4 );
  const uint32_t x5  = _uint32_or(   x3, x4 );
  const uint32_t x6  = _uint32_srl(  x5, 8 );
  const uint32_t x7  = _uint32_or(   x5, x6 );
  const uint32_t x8  = _uint32_srl(  x7, 16 );
  const uint32_t x9  = _uint32_or(   x7, x8 );
  const uint32_t xA  = _uint32_not(  x9 );
  const uint32_t xB  = _uint32_srl(  xA, 1 );
  const uint32_t xC  = _uint32_and(  xB, 0x55555555 );
  const uint32_t xD  = _uint32_sub(  xA, xC );
  const uint32_t xE  = _uint32_and(  xD, 0x33333333 );
  const uint32_t xF  = _uint32_srl(  xD, 2 );
  const uint32_t x10 = _uint32_and(  xF, 0x33333333 );
  const uint32_t x11 = _uint32_add(  xE, x10 );
  const uint32_t x12 = _uint32_srl(  x11, 4 );
  const uint32_t x13 = _uint32_add(  x11, x12 );
  const uint32_t x14 = _uint32_and(  x13, 0x0f0f0f0f );
  const uint32_t x15 = _uint32_srl(  x14, 8 );
  const uint32_t x16 = _uint32_add(  x14, x15 );
  const uint32_t x17 = _uint32_srl(  x16, 16 );
  const uint32_t x18 = _uint32_add(  x16, x17 );
  const uint32_t x19 = _uint32_and(  x18, 0x0000003f );
  return ( x19 );
#endif
}

// Count Leading Zeros
static inline uint16_t _uint16_cntlz( uint16_t x )
{
#ifdef __GNUC__
  /* On PowerPC, this will map to insn: cntlzw */
  /* On Pentium, this will map to insn: clz    */
  uint32_t x32   = _uint32_sll( x, 16 );
  uint16_t nlz   = (uint16_t)__builtin_clz( x32 );
  return (nlz);
#else
  const uint16_t x0  = _uint16_srl(  x,  1 );
  const uint16_t x1  = _uint16_or(   x,  x0 );
  const uint16_t x2  = _uint16_srl(  x1, 2 );
  const uint16_t x3  = _uint16_or(   x1, x2 );
  const uint16_t x4  = _uint16_srl(  x3, 4 );
  const uint16_t x5  = _uint16_or(   x3, x4 );
  const uint16_t x6  = _uint16_srl(  x5, 8 );
  const uint16_t x7  = _uint16_or(   x5, x6 );
  const uint16_t x8  = _uint16_not(  x7 );
  const uint16_t x9  = _uint16_srlm( x8, 1, 0x5555 );
  const uint16_t xA  = _uint16_sub(  x8, x9 );
  const uint16_t xB  = _uint16_and(  xA, 0x3333 );
  const uint16_t xC  = _uint16_srlm( xA, 2, 0x3333 );
  const uint16_t xD  = _uint16_add(  xB, xC );
  const uint16_t xE  = _uint16_srl(  xD, 4 );
  const uint16_t xF  = _uint16_addm( xD, xE, 0x0f0f );
  const uint16_t x10 = _uint16_srl(  xF, 8 );
  const uint16_t x11 = _uint16_addm( xF, x10, 0x001f );
  return ( x11 );
#endif
}

uint16_t
half_from_float( uint32_t f )
{
  const uint32_t one                        = _uint32_li( 0x00000001 );
  const uint32_t f_e_mask                   = _uint32_li( 0x7f800000 );
  const uint32_t f_m_mask                   = _uint32_li( 0x007fffff );
  const uint32_t f_s_mask                   = _uint32_li( 0x80000000 );
  const uint32_t h_e_mask                   = _uint32_li( 0x00007c00 );
  const uint32_t f_e_pos                    = _uint32_li( 0x00000017 );
  const uint32_t f_m_round_bit              = _uint32_li( 0x00001000 );
  const uint32_t h_nan_em_min               = _uint32_li( 0x00007c01 );
  const uint32_t f_h_s_pos_offset           = _uint32_li( 0x00000010 );
  const uint32_t f_m_hidden_bit             = _uint32_li( 0x00800000 );
  const uint32_t f_h_m_pos_offset           = _uint32_li( 0x0000000d );
  const uint32_t f_h_bias_offset            = _uint32_li( 0x38000000 );
  const uint32_t f_m_snan_mask              = _uint32_li( 0x003fffff );
  const uint16_t h_snan_mask                = _uint32_li( 0x00007e00 );
  const uint32_t f_e                        = _uint32_and( f, f_e_mask  );
  const uint32_t f_m                        = _uint32_and( f, f_m_mask  );
  const uint32_t f_s                        = _uint32_and( f, f_s_mask  );
  const uint32_t f_e_h_bias                 = _uint32_sub( f_e,               f_h_bias_offset );
  const uint32_t f_e_h_bias_amount          = _uint32_srl( f_e_h_bias,        f_e_pos         );
  const uint32_t f_m_round_mask             = _uint32_and( f_m,               f_m_round_bit     );
  const uint32_t f_m_round_offset           = _uint32_sll( f_m_round_mask,    one               );
  const uint32_t f_m_rounded                = _uint32_add( f_m,               f_m_round_offset  );
  const uint32_t f_m_rounded_overflow       = _uint32_and( f_m_rounded,       f_m_hidden_bit    );
  const uint32_t f_m_denorm_sa              = _uint32_sub( one,               f_e_h_bias_amount );
  const uint32_t f_m_with_hidden            = _uint32_or(  f_m_rounded,       f_m_hidden_bit    );
  const uint32_t f_m_denorm                 = _uint32_srl( f_m_with_hidden,   f_m_denorm_sa     );
  const uint32_t f_em_norm_packed           = _uint32_or(  f_e_h_bias,        f_m_rounded       );
  const uint32_t f_e_overflow               = _uint32_add( f_e_h_bias,        f_m_hidden_bit    );
  const uint32_t h_s                        = _uint32_srl( f_s,               f_h_s_pos_offset );
  const uint32_t h_m_nan                    = _uint32_srl( f_m,               f_h_m_pos_offset );
  const uint32_t h_m_denorm                 = _uint32_srl( f_m_denorm,        f_h_m_pos_offset );
  const uint32_t h_em_norm                  = _uint32_srl( f_em_norm_packed,  f_h_m_pos_offset );
  const uint32_t h_em_overflow              = _uint32_srl( f_e_overflow,      f_h_m_pos_offset );
  const uint32_t is_e_eqz_msb               = _uint32_dec(  f_e     );
  const uint32_t is_m_nez_msb               = _uint32_neg(  f_m     );
  const uint32_t is_h_m_nan_nez_msb         = _uint32_neg(  h_m_nan );
  const uint32_t is_e_nflagged_msb          = _uint32_sub(  f_e,                 f_e_mask          );
  const uint32_t is_ninf_msb                = _uint32_or(   is_e_nflagged_msb,   is_m_nez_msb      );
  const uint32_t is_underflow_msb           = _uint32_sub(  is_e_eqz_msb,        f_h_bias_offset   );
  const uint32_t is_nan_nunderflow_msb      = _uint32_or(   is_h_m_nan_nez_msb,  is_e_nflagged_msb );
  const uint32_t is_m_snan_msb              = _uint32_sub(  f_m_snan_mask,       f_m               );
  const uint32_t is_snan_msb                = _uint32_andc( is_m_snan_msb,       is_e_nflagged_msb );
  const uint32_t is_overflow_msb            = _uint32_neg(  f_m_rounded_overflow );
  const uint32_t h_nan_underflow_result     = _uint32_sels( is_nan_nunderflow_msb, h_em_norm,                h_nan_em_min       );
  const uint32_t h_inf_result               = _uint32_sels( is_ninf_msb,           h_nan_underflow_result,   h_e_mask           );
  const uint32_t h_underflow_result         = _uint32_sels( is_underflow_msb,      h_m_denorm,               h_inf_result       );
  const uint32_t h_overflow_result          = _uint32_sels( is_overflow_msb,       h_em_overflow,            h_underflow_result );
  const uint32_t h_em_result                = _uint32_sels( is_snan_msb,           h_snan_mask,              h_overflow_result  );
  const uint32_t h_result                   = _uint32_or( h_em_result, h_s );

  return (h_result);
}

uint32_t 
half_to_float( uint16_t h )
{
  const uint32_t h_e_mask              = _uint32_li( 0x00007c00 );
  const uint32_t h_m_mask              = _uint32_li( 0x000003ff );
  const uint32_t h_s_mask              = _uint32_li( 0x00008000 );
  const uint32_t h_f_s_pos_offset      = _uint32_li( 0x00000010 );
  const uint32_t h_f_e_pos_offset      = _uint32_li( 0x0000000d );
  const uint32_t h_f_bias_offset       = _uint32_li( 0x0001c000 );
  const uint32_t f_e_mask              = _uint32_li( 0x7f800000 );
  const uint32_t f_m_mask              = _uint32_li( 0x007fffff );
  const uint32_t h_f_e_denorm_bias     = _uint32_li( 0x0000007e );
  const uint32_t h_f_m_denorm_sa_bias  = _uint32_li( 0x00000008 );
  const uint32_t f_e_pos               = _uint32_li( 0x00000017 );
  const uint32_t h_e_mask_minus_one    = _uint32_li( 0x00007bff );
  const uint32_t h_e                   = _uint32_and( h, h_e_mask );
  const uint32_t h_m                   = _uint32_and( h, h_m_mask );
  const uint32_t h_s                   = _uint32_and( h, h_s_mask );
  const uint32_t h_e_f_bias            = _uint32_add( h_e, h_f_bias_offset );
  const uint32_t h_m_nlz               = _uint32_cntlz( h_m );
  const uint32_t f_s                   = _uint32_sll( h_s,        h_f_s_pos_offset );
  const uint32_t f_e                   = _uint32_sll( h_e_f_bias, h_f_e_pos_offset );
  const uint32_t f_m                   = _uint32_sll( h_m,        h_f_e_pos_offset );
  const uint32_t f_em                  = _uint32_or(  f_e,        f_m              );
  const uint32_t h_f_m_sa              = _uint32_sub( h_m_nlz,             h_f_m_denorm_sa_bias );
  const uint32_t f_e_denorm_unpacked   = _uint32_sub( h_f_e_denorm_bias,   h_f_m_sa             );
  const uint32_t h_f_m                 = _uint32_sll( h_m,                 h_f_m_sa             );
  const uint32_t f_m_denorm            = _uint32_and( h_f_m,               f_m_mask             );
  const uint32_t f_e_denorm            = _uint32_sll( f_e_denorm_unpacked, f_e_pos              );
  const uint32_t f_em_denorm           = _uint32_or(  f_e_denorm,          f_m_denorm           );
  const uint32_t f_em_nan              = _uint32_or(  f_e_mask,            f_m                  );
  const uint32_t is_e_eqz_msb          = _uint32_dec(  h_e );
  const uint32_t is_m_nez_msb          = _uint32_neg(  h_m );
  const uint32_t is_e_flagged_msb      = _uint32_sub(  h_e_mask_minus_one, h_e );
  const uint32_t is_zero_msb           = _uint32_andc( is_e_eqz_msb,       is_m_nez_msb );
  const uint32_t is_inf_msb            = _uint32_andc( is_e_flagged_msb,   is_m_nez_msb );
  const uint32_t is_denorm_msb         = _uint32_and(  is_m_nez_msb,       is_e_eqz_msb );
  const uint32_t is_nan_msb            = _uint32_and(  is_e_flagged_msb,   is_m_nez_msb ); 
  const uint32_t is_zero               = _uint32_ext(  is_zero_msb );
  const uint32_t f_zero_result         = _uint32_andc( f_em, is_zero );
  const uint32_t f_denorm_result       = _uint32_sels( is_denorm_msb, f_em_denorm, f_zero_result );
  const uint32_t f_inf_result          = _uint32_sels( is_inf_msb,    f_e_mask,    f_denorm_result );
  const uint32_t f_nan_result          = _uint32_sels( is_nan_msb,    f_em_nan,    f_inf_result    );
  const uint32_t f_result              = _uint32_or( f_s, f_nan_result );
 
  return (f_result);
}

// for dxt
static void GTFunorderblocks(int width, int height, int rowbytes, const void *source, void *dest)
{
    int x,y,bx,by;
    const unsigned char *s = (const unsigned char *)source;

    for (y=0; y<height; y+=4)
    {
        for (x=0; x<width; x+=4)
        {
            for (by=0; by<4; ++by) // rows of block
            {
                for (bx=0; bx<4; ++bx) // columns of block
                {
                    if ((y+by)<height && (x+bx)<width)
                        *(int *) ((char *)dest + (y+by)*rowbytes + (x+bx)*4) = *(int *) (s + by*16 + bx*4);
                }
            }
            s += 64;
        }
    }
}


static void GTFuntile(int width, int height, int destrowbytes, const void *source, void *dest)
{
    const unsigned char *s = (const unsigned char *)source;
    destrowbytes = width*4; // fudge

    for (int ty=0; ty<height; ty+=32)       // 32x32 tiles
    {
        for (int tx=0; tx<width; tx+=32)
        {
            for (int by=0; by<32; by+=2)    // 8x2 blocks
            {
                for (int bx=0; bx<32; bx+=8)
                {
                    int bx2=bx^((by&8)/8)*16; // 16x8 swap

                    for (int y=0; y<2; y+=1)    // 4x1
                    {
                        for (int x=0; x<8; x+=1)
                        {
                            int y2=x/4;
                            int x2=(x&3)+y*4;
                            *(int *) ((char *)dest + (ty+by+y2)*destrowbytes + (tx+bx2+x2)*4) = *(int *) s;
                            s += 4;
                        }
                    }
                }
            }
        }
    }
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL GTF_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or attrib fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external attribs */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name */
        info->defaultquality  = 70;   /* default pack quality (for dxt) */
        info->movie           = 0;    /* movie file format */
        info->file64          = 0;    /* this format supports 64 bit file sizes */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.00");                /* version # */
		info->signature       = GMAKEID('.','G','T','F');
		info->mactype[0]      = GMAKEID('.','G','T','F');   /* mac file system types used */
        strcpy(info->extensions[0], ".gtf");                /* extensions */
        strcpy(info->shorttypestr,  "GTF");                 /* type */
        strcpy(info->wordtypestr,   "GTF");                 /* wordtype */
        strcpy(info->longtypestr,   "PS3 Graphics Texture Format");/* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* has 4 byte version */

// C:\env\egami\gx\dev>xd john.gtf -h
// 00000: 00090701 00040000 00000001 00000000  ................
// 00010: 00000080 00040000 a5010200 0000aae4  ........%.....*d
// 00020: 01000100 00010000 00000400 00000000  ................
// 00030: 50505050 50505050 50505050 50505050  PPPPPPPPPPPPPPPP
// 00040: 50505050 50505050 50505050 50505050  PPPPPPPPPPPPPPPP
// 00050: 50505050 50505050 50505050 50505050  PPPPPPPPPPPPPPPP
// 00060: 50505050 50505050 50505050 50505050  PPPPPPPPPPPPPPPP
// 00070: 50505050 50505050 50505050 50505050  PPPPPPPPPPPPPPPP
// 00080: ff9c9c9c ff9c9c9c ff9c9c9c ff9c9c9c  ................
// 00090: ff9c9c9c ff9c9c9c ff9c9c9c ff9c9c9c  ................
// 000a0: ff9c9c9c ff9c9c9c ff9c9c9c ffa5a5a5  .............%%%
 
int GCALL GTF_is(GSTREAM *g)
{
    unsigned char gtfheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,gtfheader,4))
    {
		if (ggetm(gtfheader,4)==0x00090701)
            ok = 100;
		else if (ggetm(gtfheader,4)==0x00090000)
            ok = 90;
		else if (ggetm(gtfheader,4)==0x00080501)
            ok = 80;
		else if (ggetm(gtfheader,4)==0x00080301)
            ok = 70;
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL GTF_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    unsigned char gtfheader[16];

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
		gx->signature=GMAKEID('.','G','T','F');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

        gseek(g,(GPOS)0);
        if (gread(g,gtfheader,12))
        {
            gx->frames = ggetm(gtfheader+8,4);
            int headersize = 12+gx->frames*36;

            gx->gref = galloc(headersize);
            if (gx->gref)
            {
                gseek(gx->gstream,(GPOS)0);
                if (gread(gx->gstream,gx->gref,headersize))
                {
                    ok = gx->frames;
                }
            }
        }
        if (gx->gref && !ok)
            gfree(gx->gref);
    }
    return(ok);
}

GINFO * GCALL GTF_info(GINSTANCE *gx, int framenum)
{
    unsigned char *gtfheader;
    GINFO *ginfo=0;

    gtfheader = (unsigned char*)gx->gref;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

		ginfo->signature        = GMAKEID('.','G','T','F');   /* Gimex type */
	    ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
        ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = 8;    /* Number of bits in alpha channel */
        ginfo->redbits          = 8;    /* Number of bits in red channel */
        ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
        ginfo->bluebits         = 8;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */

        unsigned char *attrib = gtfheader+12+framenum*36;
        ginfo->width =  ggetm(attrib+20,2);
        ginfo->height = ggetm(attrib+22,2);

        ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = 32;   /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */

        unsigned int DataFormat = ggetm(attrib+12,1) & ~(CELL_GCM_TEXTURE_UN|CELL_GCM_TEXTURE_LN);
        switch (DataFormat)
        {
            case CELL_GCM_TEXTURE_R5G6B5:
                ginfo->alphabits = 0;
                ginfo->redbits = 5;
                ginfo->greenbits = 6;
                ginfo->bluebits = 5;
                ginfo->originalbpp = 16;
                break;

            case CELL_GCM_TEXTURE_A1R5G5B5:
                ginfo->alphabits = 1;
                ginfo->redbits = 5;
                ginfo->greenbits = 5;
                ginfo->bluebits = 5;
                ginfo->originalbpp = 16;
                break;

            case CELL_GCM_TEXTURE_D1R5G5B5:
                ginfo->alphabits = 0;
                ginfo->redbits = 5;
                ginfo->greenbits = 5;
                ginfo->bluebits = 5;
                ginfo->originalbpp = 16;
                break;

            case CELL_GCM_TEXTURE_A8R8G8B8:
                ginfo->alphabits = 8;
                ginfo->redbits = 8;
                ginfo->greenbits = 8;
                ginfo->bluebits = 8;
                ginfo->originalbpp = 32;
                break;

            case CELL_GCM_TEXTURE_B8: // A8
                ginfo->alphabits = 8;
                ginfo->redbits = 0;
                ginfo->greenbits = 0;
                ginfo->bluebits = 0;
                ginfo->originalbpp = 8;
                break;


            case CELL_GCM_TEXTURE_G8B8: // AL88
                ginfo->alphabits = 8;
                ginfo->redbits = 8;
                ginfo->greenbits = 0;
                ginfo->bluebits = 0;
                ginfo->originalbpp = 16;
                break;

            case CELL_GCM_TEXTURE_A4R4G4B4:
                ginfo->alphabits = 4;
                ginfo->redbits = 4;
                ginfo->greenbits = 4;
                ginfo->bluebits = 4;
                ginfo->originalbpp = 16;
                break;

            case CELL_GCM_TEXTURE_COMPRESSED_DXT1:
                ginfo->alphabits = 1;
                ginfo->redbits = 5;
                ginfo->greenbits = 6;
                ginfo->bluebits = 5;
                ginfo->originalbpp = 17;
                ginfo->packed = 3;             // 3 is DXT
                break;

            case CELL_GCM_TEXTURE_COMPRESSED_DXT23:
                ginfo->alphabits = 4;
                ginfo->redbits = 5;
                ginfo->greenbits = 6;
                ginfo->bluebits = 5;
                ginfo->originalbpp = 20;
                ginfo->packed = 3;             // 3 is DXT
                break;

            case CELL_GCM_TEXTURE_COMPRESSED_DXT45:
                ginfo->alphabits = 3;
                ginfo->redbits = 5;
                ginfo->greenbits = 6;
                ginfo->bluebits = 5;
                ginfo->originalbpp = 19;
                ginfo->packed = 3;             // 3 is DXT
                break;

            case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT: // half float rgba
                ginfo->alphabits = 16;
                ginfo->redbits = 16;
                ginfo->greenbits = 16;
                ginfo->bluebits = 16;
                ginfo->originalbpp = 64;
                break;

            case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT: // float rgba
                ginfo->alphabits = 32;
                ginfo->redbits = 32;
                ginfo->greenbits = 32;
                ginfo->bluebits = 32;
                ginfo->originalbpp = 128;
                break;
				
        }
    }

    return(ginfo);
}

//uint32_t half_to_float( uint16_t h )
// Half-precision floating point format
// ------------------------------------
//
//   | Field    | Last | First | Note
//   |----------|------|-------|----------
//   | Sign     | 15   | 15    |
//   | Exponent | 14   | 10    | Bias = 15
//   | Mantissa | 9    | 0     |
//

#if 0
unsigned char F16to8(unsigned int f16)
{
	unsigned int m=f16&0x03ff;
	unsigned int e=(((f16)>>10)&31);
	unsigned int s=(f16&0x8000);
	int v=(int)((m<<e)>>15);
	if (s)
		v = -v;
	if (v<0) v=0;
	if (v>255) v=255;
	return (unsigned char)v;
}
#endif

unsigned char F16to8(unsigned int f16)
{
	float f;
	*(uint32_t *) &f = half_to_float( (uint16_t) f16 );

	int v = (int) (f*255);
	if (v<0) v=0;
	if (v>255) v=255;
	return (unsigned char)v;
}

unsigned char F32to8(unsigned int f32)
{
	float f;
	*(uint32_t *) &f = f32;

	int v = (int) (f*255);
	if (v<0) v=0;
	if (v>255) v=255;
	return (unsigned char)v;
}

int GCALL GTF_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;

    int framenum=ginfo->framenum;
    unsigned char *gtfheader = (unsigned char*)gx->gref;

	unsigned char *attrib = gtfheader+12+framenum*36;
	unsigned int DataFormat = ggetm(attrib+12,1) & ~(CELL_GCM_TEXTURE_UN|CELL_GCM_TEXTURE_LN);

    unsigned int Pitch = ggetm(attrib+28,4); // rowbytes
    unsigned int BaseAddress = ggetm(attrib+4,4); // offset to texture

	if (!Pitch)
	{
		if (DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT1)
		{
			Pitch = ginfo->width/2; // dxt1 is 4 bpp
		}
		else if (DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT23 || DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT45)
		{
			Pitch = ginfo->width; // dxt3/5 is 8 bpp
		}
		else if (DataFormat==CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT)
		{
			Pitch = ginfo->width*8; // 64 bits per pixel
		}
		else if (DataFormat==CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT)
		{
			Pitch = ginfo->width*16; // 128 bits per pixel
		}
		else if (ginfo->originalbpp==16)
		{
			Pitch = ginfo->width*2; // dxt3/5 is 8 bpp
		}
		else
			Pitch = ginfo->width*4; // force 32
	}

//    unsigned int Tiled = ggetm(gputexture+0,4)>>31;
//    if (!Tiled) Pitch = (Pitch+3)&~3;   // ps3 make a multiple of 4

	unsigned int framesize = Pitch*ginfo->height;

    unsigned char *framebuf = (unsigned char *) galloc(framesize+sizeof(DXTHEADER));

	int maxa = 0;
    if (framebuf)
    {
        gseek(gx->gstream,BaseAddress);

        if (DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT1 || DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT23 || DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT45)
        {
            ok = gread(gx->gstream,framebuf+sizeof(DXTHEADER),framesize); // read dxt source
            DXTHEADER *dxtheader = (DXTHEADER *)framebuf;

            unsigned int code=0;
            if (DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT1)
                code |= DXT_DXT1_CODE;
            else if (DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT23)
                code |= DXT_DXT3_CODE;
            else// if (DataFormat==CELL_GCM_TEXTURE_COMPRESSED_DXT45)
                code |= DXT_DXT5_CODE;

            gputm(dxtheader->code, code, sizeof(dxtheader->code));
            gputm(dxtheader->size, ((ginfo->width+3)/4) * ((ginfo->height+3)/4) * 64, sizeof(dxtheader->size));
            gputm(dxtheader->mode, 0, sizeof(dxtheader->mode));

            // Decode and reorder the blocks for the destination image
            char *temp = (char *) galloc(DXT_size(dxtheader));
            if (temp)
            {
                int compressedsize;
                memset(temp, 0, DXT_size(dxtheader));
                DXT_decode((void*)temp,dxtheader, &compressedsize);
                GTFunorderblocks(ginfo->width, ginfo->height, rowbytes, (void*)(temp), dest);
                gfree(temp);
                ok = 1;
            }
        }
        else // not dxt
        {
            ok = gread(gx->gstream,framebuf,framesize);

            for (int y=0; y<ginfo->height && ok; ++y)
            {
                ARGB *d = (ARGB *) (dest+rowbytes*y);
                unsigned char *lineptr = framebuf+Pitch*y;

                switch (DataFormat)
                {
                    case CELL_GCM_TEXTURE_A4R4G4B4:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = (((ggetm(lineptr,2)>>12)&15)*255+8)/15;
                            d->r = (((ggetm(lineptr,2)>> 8)&15)*255+8)/15;
                            d->g = (((ggetm(lineptr,2)>> 4)&15)*255+8)/15;
                            d->b = (((ggetm(lineptr,2)    )&15)*255+8)/15;
                            ++d;
                            lineptr += 2;
                        }
                        break;

                    case CELL_GCM_TEXTURE_A1R5G5B5:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = (((ggetm(lineptr,2)>>15)&1)*255)/1;
                            d->r = (((ggetm(lineptr,2)>>10)&31)*255+16)/31;
                            d->g = (((ggetm(lineptr,2)>> 5)&31)*255+16)/31;
                            d->b = (((ggetm(lineptr,2)    )&31)*255+16)/31;
                            ++d;
                            lineptr += 2;
                        }
                        break;

                    case CELL_GCM_TEXTURE_D1R5G5B5:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = 255;
                            d->r = (((ggetm(lineptr,2)>>10)&31)*255+16)/31;
                            d->g = (((ggetm(lineptr,2)>> 5)&31)*255+16)/31;
                            d->b = (((ggetm(lineptr,2)    )&31)*255+16)/31;
                            ++d;
                            lineptr += 2;
                        }
                        break;

                    case CELL_GCM_TEXTURE_R5G6B5:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = 255;
                            d->r = (((ggetm(lineptr,2)>>11)&31)*255+16)/31;
                            d->g = (((ggetm(lineptr,2)>> 5)&63)*255+32)/63;
                            d->b = (((ggetm(lineptr,2)    )&31)*255+16)/31;
                            ++d;
                            lineptr += 2;
                        }
                        break;

                    case CELL_GCM_TEXTURE_A8R8G8B8: // note dds2gtf does not support DRGB properly, filling in alpha with 0 but remap to pass thru asis.
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = lineptr[0];
                            d->r = lineptr[1];
                            d->g = lineptr[2];
                            d->b = lineptr[3];
							maxa = gmax(maxa,d->a);
                            ++d;
                            lineptr += 4;
                        }
                        break;

                    case CELL_GCM_TEXTURE_B8:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = lineptr[0];
                            d->r = 255;
                            d->g = 255;
                            d->b = 255;
                            ++d;
                            lineptr += 1;
                        }
                        break;

                    case CELL_GCM_TEXTURE_G8B8:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->a = lineptr[0];
                            d->r = lineptr[1];
                            d->g = lineptr[1];
                            d->b = lineptr[1];
                            ++d;
                            lineptr += 2;
                        }
                        break;

                    case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->r = F16to8(ggetm(lineptr+0,2));
                            d->g = F16to8(ggetm(lineptr+2,2));
                            d->b = F16to8(ggetm(lineptr+4,2));
                            d->a = F16to8(ggetm(lineptr+6,2));
                            ++d;
                            lineptr += 8;
                        }
                        break;

					case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT:
                        for (int x=0; x<ginfo->width; ++x)
                        {
                            d->r = F32to8(ggetm(lineptr+0,4));
                            d->g = F32to8(ggetm(lineptr+4,4));
                            d->b = F32to8(ggetm(lineptr+8,4));
                            d->a = F32to8(ggetm(lineptr+12,4));
                            ++d;
                            lineptr += 16;
                        }
                        break;

				}
            }

			// fixedup for XRGB bug in dds2gtf
			if (maxa==0)
			{
				ginfo->alphabits = 0; // change alpha bits to 0 to tell caller that the reader has decided to change the alpha bits
				for (int y=0; y<ginfo->height && ok; ++y)
				{
					ARGB *d = (ARGB *) (dest+rowbytes*y);
					for (int x=0; x<ginfo->width; ++x, ++d)
						d->a = 255;
				}
			}

#if 0
            if (Tiled)
            {
                if (DataFormat==CELL_GCM_TEXTURE_A8R8G8B8)
                {
                    void *temp = galloc(ginfo->width*ginfo->height*4);
                    memcpy(temp,dest,ginfo->width*ginfo->height*4);

                    GTFuntile(ginfo->width, ginfo->height, rowbytes, temp, dest);
                }
                else// if (DataFormat==CELL_GCM_TEXTURE_A1R5G5B5)
                {
                    void *temp = galloc(ginfo->width*ginfo->height*4);
                    memcpy(temp,dest,ginfo->width*ginfo->height*4);

                    GTFuntile(ginfo->width, ginfo->height, rowbytes, temp, dest);
                }
            }
#endif

        }


        gfree(framebuf);
    }
    return(ok);
}

int GCALL GTF_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if (gx->gref)
            gfree(gx->gref);

        ok = gfree((void *) gx);
    }
    return(ok);
}

