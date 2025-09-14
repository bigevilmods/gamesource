// (c) 1996-2007 Electronic Arts Inc.
/* conform.c, conformance checks                                            */


/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mpeg2enc.h"

/* check for (level independent) parameter limits */
void range_checks()
{
  int i;

  /* range and value checks */

  if (MPEhorizontal_size<1 || MPEhorizontal_size>16383)
    error("MPEhorizontal_size must be between 1 and 16383");
  if (MPEmpeg1 && MPEhorizontal_size>4095)
    error("MPEhorizontal_size must be less than 4096 (MPEG-1)");
  if ((MPEhorizontal_size&4095)==0)
    error("MPEhorizontal_size must not be a multiple of 4096");
  if (MPEchroma_format!=CHROMA444 && MPEhorizontal_size%2 != 0)
    error("MPEhorizontal_size must be a even (4:2:0 / 4:2:2)");

  if (MPEvertical_size<1 || MPEvertical_size>16383)
    error("MPEvertical_size must be between 1 and 16383");
  if (MPEmpeg1 && MPEvertical_size>4095)
    error("MPEvertical_size must be less than 4096 (MPEG-1)");
  if ((MPEvertical_size&4095)==0)
    error("MPEvertical_size must not be a multiple of 4096");
  if (MPEchroma_format==CHROMA420 && MPEvertical_size%2 != 0)
    error("MPEvertical_size must be a even (4:2:0)");
  if(MPEfieldpic)
  {
    if (MPEvertical_size%2 != 0)
      error("MPEvertical_size must be a even (field pictures)");
    if (MPEchroma_format==CHROMA420 && MPEvertical_size%4 != 0)
      error("MPEvertical_size must be a multiple of 4 (4:2:0 field pictures)");
  }

  if (MPEmpeg1)
  {
    if (MPEaspectratio<1 || MPEaspectratio>14)
      error("MPEaspectratio must be between 1 and 14 (MPEG-1)");
  }
  else
  {
    if (MPEaspectratio<1 || MPEaspectratio>4)
      error("MPEaspectratio must be 1, 2, 3 or 4");
  }

  if (MPEframe_rate_code<1 || MPEframe_rate_code>8)
    error("MPEframe_rate code must be between 1 and 8");

  if (MPEbit_rate<=0.0)
    error("MPEbit_rate must be positive");
  if (MPEbit_rate > ((1<<30)-1)*400.0)
    error("MPEbit_rate must be less than 429 Gbit/s");
  if (MPEmpeg1 && MPEbit_rate > ((1<<18)-1)*400.0)
    error("MPEbit_rate must be less than 104 Mbit/s (MPEG-1)");

  if (MPEvbv_buffer_size<1 || MPEvbv_buffer_size>0x3ffff)
    error("MPEvbv_buffer_size must be in range 1..(2^18-1)");
  if (MPEmpeg1 && MPEvbv_buffer_size>=1024)
    error("MPEvbv_buffer_size must be less than 1024 (MPEG-1)");

  if (MPEchroma_format<CHROMA420 || MPEchroma_format>CHROMA444)
    error("MPEchroma_format must be in range 1...3");

  if (MPEvideo_format<0 || MPEvideo_format>4)
    error("MPEvideo_format must be in range 0...4");

  if (MPEcolor_primaries<1 || MPEcolor_primaries>7 || MPEcolor_primaries==3)
    error("MPEcolor_primaries must be in range 1...2 or 4...7");

  if (MPEtransfer_characteristics<1 || MPEtransfer_characteristics>7
      || MPEtransfer_characteristics==3)
    error("MPEtransfer_characteristics must be in range 1...2 or 4...7");

  if (MPEmatrix_coefficients<1 || MPEmatrix_coefficients>7 || MPEmatrix_coefficients==3)
    error("MPEmatrix_coefficients must be in range 1...2 or 4...7");

  if (MPEdisplay_horizontal_size<0 || MPEdisplay_horizontal_size>16383)
    error("MPEdisplay_horizontal_size must be in range 0...16383");
  if (MPEdisplay_vertical_size<0 || MPEdisplay_vertical_size>16383)
    error("MPEdisplay_vertical_size must be in range 0...16383");

  if (MPEdc_prec<0 || MPEdc_prec>3)
    error("MPEdc_prec must be in range 0...3");

  for (i=0; i<MPEm; i++)
  {
    if (MPEmotion_data[i].forw_hor_f_code<1 || MPEmotion_data[i].forw_hor_f_code>9)
      error("f_code must be between 1 and 9");
    if (MPEmotion_data[i].forw_vert_f_code<1 || MPEmotion_data[i].forw_vert_f_code>9)
      error("f_code must be between 1 and 9");
    if (MPEmpeg1 && MPEmotion_data[i].forw_hor_f_code>7)
      error("f_code must be le less than 8");
    if (MPEmpeg1 && MPEmotion_data[i].forw_vert_f_code>7)
      error("f_code must be le less than 8");
    if (MPEmotion_data[i].sxf<=0)
      error("search window must be positive"); /* doesn't belong here */
    if (MPEmotion_data[i].syf<=0)
      error("search window must be positive");
    if (i!=0)
    {
      if (MPEmotion_data[i].back_hor_f_code<1 || MPEmotion_data[i].back_hor_f_code>9)
        error("f_code must be between 1 and 9");
      if (MPEmotion_data[i].back_vert_f_code<1 || MPEmotion_data[i].back_vert_f_code>9)
        error("f_code must be between 1 and 9");
      if (MPEmpeg1 && MPEmotion_data[i].back_hor_f_code>7)
        error("f_code must be le less than 8");
      if (MPEmpeg1 && MPEmotion_data[i].back_vert_f_code>7)
        error("f_code must be le less than 8");
      if (MPEmotion_data[i].sxb<=0)
        error("search window must be positive");
      if (MPEmotion_data[i].syb<=0)
        error("search window must be positive");
    }
  }
}

/* identifies valid profile / level combinations */
static char profile_level_defined[5][4] =
{
/* HL   H-14 ML   LL  */
  {1,   1,   1,   0},  /* HP   */
  {0,   1,   0,   0},  /* Spat */
  {0,   0,   1,   1},  /* SNR  */
  {1,   1,   1,   1},  /* MP   */
  {0,   0,   1,   0}   /* SP   */
};

static struct level_limits {
  int hor_f_code;
  int vert_f_code;
  int hor_size;
  int vert_size;
  int sample_rate;
  int bit_rate; /* Mbit/s */
  int vbv_buffer_size; /* 16384 bit steps */
} maxval_tab[4] =
{
  {9, 5, 1920, 1152, 62668800, 80, 597}, /* HL */
  {9, 5, 1440, 1152, 47001600, 60, 448}, /* H-14 */
  {8, 5,  720,  576, 10368000, 15, 112}, /* ML */
  {7, 4,  352,  288,  3041280,  4,  29}  /* LL */
};

#define SP   5
#define MP   4
#define SNR  3
#define SPAT 2
#define HP   1

#define LL  10
#define ML   8
#define H14  6
#define HL   4

void profile_and_level_checks()
{
  int i;
  struct level_limits *maxval;

  if (MPEprofile<0 || MPEprofile>15)
    error("MPEprofile must be between 0 and 15");

  if (MPElevel<0 || MPElevel>15)
    error("MPElevel must be between 0 and 15");

  if (MPEprofile>=8)
  {
    if (!MPEquiet)
      fprintf(stderr,"Warning: MPEprofile uses a reserved value, conformance checks skipped\n");
    return;
  }

  if (MPEprofile<HP || MPEprofile>SP)
    error("undefined Profile");

  if (MPEprofile==SNR || MPEprofile==SPAT)
    error("This encoder currently generates no scalable bitstreams");

  if (MPElevel<HL || MPElevel>LL || MPElevel&1)
    error("undefined Level");

  maxval = &maxval_tab[(MPElevel-4) >> 1];

  /* check profile@level combination */
  if(!profile_level_defined[MPEprofile-1][(MPElevel-4) >> 1])
    error("undefined profile@level combination");
  

  /* profile (syntax) constraints */

  if (MPEprofile==SP && MPEm!=1)
    error("Simple Profile does not allow B pictures");

  if (MPEprofile!=HP && MPEchroma_format!=CHROMA420)
    error("chroma format must be 4:2:0 in specified Profile");

  if (MPEprofile==HP && MPEchroma_format==CHROMA444)
    error("chroma format must be 4:2:0 or 4:2:2 in High Profile");

  if (MPEprofile>=MP) /* SP, MP: constrained repeat_first_field */
  {
    if (MPEframe_rate_code<=2 && MPErepeatfirst)
      error("MPErepeatfirst must be zero");
    if (MPEframe_rate_code<=6 && MPEprog_seq && MPErepeatfirst)
      error("MPErepeatfirst must be zero");
  }

  if (MPEprofile!=HP && MPEdc_prec==3)
    error("11 bit DC precision only allowed in High Profile");


  /* level (parameter value) constraints */

  /* Table 8-8 */
  if (MPEframe_rate_code>5 && MPElevel>=ML)
    error("Picture rate greater than permitted in specified Level");

  for (i=0; i<MPEm; i++)
  {
    if (MPEmotion_data[i].forw_hor_f_code > maxval->hor_f_code)
      error("forward horizontal f_code greater than permitted in specified Level");

    if (MPEmotion_data[i].forw_vert_f_code > maxval->vert_f_code)
      error("forward vertical f_code greater than permitted in specified Level");

    if (i!=0)
    {
      if (MPEmotion_data[i].back_hor_f_code > maxval->hor_f_code)
        error("backward horizontal f_code greater than permitted in specified Level");
  
      if (MPEmotion_data[i].back_vert_f_code > maxval->vert_f_code)
        error("backward vertical f_code greater than permitted in specified Level");
    }
  }

  /* Table 8-10 */
  if (MPEhorizontal_size > maxval->hor_size)
    error("Horizontal size is greater than permitted in specified Level");

  if (MPEvertical_size > maxval->vert_size)
    error("Horizontal size is greater than permitted in specified Level");

  /* Table 8-11 */
  if (MPEhorizontal_size*MPEvertical_size*MPEframe_rate > maxval->sample_rate)
    error("Sample rate is greater than permitted in specified Level");

  /* Table 8-12 */
  if (MPEbit_rate> 1.0e6 * maxval->bit_rate)
    error("Bit rate is greater than permitted in specified Level");

  /* Table 8-13 */
  if (MPEvbv_buffer_size > maxval->vbv_buffer_size)
    error("MPEvbv_buffer_size exceeds High Level limit");
}
