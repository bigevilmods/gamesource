// (c) 1996-2007 Electronic Arts Inc.
/* puthdr.c, generation of headers                                          */


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
#include <math.h>
#include "mpeg2enc.h"

/* private prototypes */
static int frametotc _ANSI_ARGS_((int frame));

/* generate sequence header (6.2.2.1, 6.3.3)
 *
 * matrix download not implemented
 */
void putseqhdr()
{
  int i;

  alignbits();
  putbits(SEQ_START_CODE,32); /* sequence_header_code */
  putbits(MPEhorizontal_size,12); /* horizontal_size_value */
  putbits(MPEvertical_size,12); /* vertical_size_value */
  putbits(MPEaspectratio,4); /* aspect_ratio_information */
  putbits(MPEframe_rate_code,4); /* frame_rate_code */
  putbits((int)ceil(MPEbit_rate/400.0),18); /* bit_rate_value */
  putbits(1,1); /* marker_bit */
  putbits(MPEvbv_buffer_size,10); /* vbv_buffer_size_value */
  putbits(MPEconstrparms,1); /* constrained_parameters_flag */

  putbits(MPEload_iquant,1); /* load_intra_quantizer_matrix */
  if (MPEload_iquant)
    for (i=0; i<64; i++)  /* matrices are always downloaded in zig-zag order */
      putbits(MPEintra_q[MPEzig_zag_scan[i]],8); /* intra_quantizer_matrix */

  putbits(MPEload_niquant,1); /* load_non_intra_quantizer_matrix */
  if (MPEload_niquant)
    for (i=0; i<64; i++)
      putbits(MPEinter_q[MPEzig_zag_scan[i]],8); /* non_intra_quantizer_matrix */
}

/* generate sequence extension (6.2.2.3, 6.3.5) header (MPEG-2 only) */
void putseqext()
{
  alignbits();
  putbits(EXT_START_CODE,32); /* extension_start_code */
  putbits(SEQ_ID,4); /* extension_start_code_identifier */
  putbits((MPEprofile<<4)|MPElevel,8); /* profile_and_level_indication */
  putbits(MPEprog_seq,1); /* progressive sequence */
  putbits(MPEchroma_format,2); /* chroma_format */
  putbits(MPEhorizontal_size>>12,2); /* horizontal_size_extension */
  putbits(MPEvertical_size>>12,2); /* vertical_size_extension */
  putbits(((int)ceil(MPEbit_rate/400.0))>>18,12); /* bit_rate_extension */
  putbits(1,1); /* marker_bit */
  putbits(MPEvbv_buffer_size>>10,8); /* vbv_buffer_size_extension */
  putbits(0,1); /* low_delay  -- currently not implemented */
  putbits(0,2); /* frame_rate_extension_n */
  putbits(MPEframe_rate_extension_d,5); /* frame_rate_extension_d */
}

/* generate sequence display extension (6.2.2.4, 6.3.6)
 *
 * content not yet user setable
 */
void putseqdispext()
{
  alignbits();
  putbits(EXT_START_CODE,32); /* extension_start_code */
  putbits(DISP_ID,4); /* extension_start_code_identifier */
  putbits(MPEvideo_format,3); /* video_format */
  putbits(1,1); /* colour_description */
  putbits(MPEcolor_primaries,8); /* colour_primaries */
  putbits(MPEtransfer_characteristics,8); /* transfer_characteristics */
  putbits(MPEmatrix_coefficients,8); /* matrix_coefficients */
  putbits(MPEdisplay_horizontal_size,14); /* display_horizontal_size */
  putbits(1,1); /* marker_bit */
  putbits(MPEdisplay_vertical_size,14); /* display_vertical_size */
}

/* output a zero terminated string as user data (6.2.2.2.2, 6.3.4.1)
 *
 * string must not emulate start codes
 */
void putuserdata(userdata)
char *userdata;
{
  alignbits();
  putbits(USER_START_CODE,32); /* user_data_start_code */
  while (*userdata)
    putbits(*userdata++,8);
}

/* generate group of pictures header (6.2.2.6, 6.3.9)
 *
 * uses tc0 (timecode of first frame) and frame0 (number of first frame)
 */
void putgophdr(frame,closed_gop)
int frame,closed_gop;
{
  int tc;

  alignbits();
  putbits(GOP_START_CODE,32); /* group_start_code */
  tc = frametotc(MPEtc0+frame);
  putbits(tc,25); /* time_code */
  putbits(closed_gop,1); /* closed_gop */
  putbits(0,1); /* broken_link */
}

/* convert frame number to time_code
 *
 * drop_frame not implemented
 */
static int frametotc(frame)
int frame;
{
  int fps, pict, sec, minute, hour, tc;

  fps = (int)(MPEframe_rate+0.5);
  pict = frame%fps;
  frame = (frame-pict)/fps;
  sec = frame%60;
  frame = (frame-sec)/60;
  minute = frame%60;
  frame = (frame-minute)/60;
  hour = frame%24;
  tc = (hour<<19) | (minute<<13) | (1<<12) | (sec<<6) | pict;

  return tc;
}

/* generate picture header (6.2.3, 6.3.10) */
void putpicthdr()
{
  alignbits();
  putbits(PICTURE_START_CODE,32); /* picture_start_code */
  /* PRATTO: vbv_delay calc doesn't work so don't bother */
  /* calc_vbv_delay(); */
  MPEvbv_delay = 0xffff;
  putbits(MPEtemp_ref,10); /* temporal_reference */
  putbits(MPEpict_type,3); /* picture_coding_type */
  putbits(MPEvbv_delay,16); /* vbv_delay */

  if (MPEpict_type==P_TYPE || MPEpict_type==B_TYPE)
  {
    putbits(0,1); /* full_pel_forward_vector */
    if (MPEmpeg1)
      putbits(MPEforw_hor_f_code,3);
    else
      putbits(7,3); /* forward_f_code */
  }

  if (MPEpict_type==B_TYPE)
  {
    putbits(0,1); /* full_pel_backward_vector */
    if (MPEmpeg1)
      putbits(MPEback_hor_f_code,3);
    else
      putbits(7,3); /* backward_f_code */
  }

  putbits(0,1); /* extra_bit_picture */
}

/* generate picture coding extension (6.2.3.1, 6.3.11)
 *
 * composite display information (v_axis etc.) not implemented
 */
void putpictcodext()
{
  alignbits();
  putbits(EXT_START_CODE,32); /* extension_start_code */
  putbits(CODING_ID,4); /* extension_start_code_identifier */
  putbits(MPEforw_hor_f_code,4); /* forward_horizontal_f_code */
  putbits(MPEforw_vert_f_code,4); /* forward_vertical_f_code */
  putbits(MPEback_hor_f_code,4); /* backward_horizontal_f_code */
  putbits(MPEback_vert_f_code,4); /* backward_vertical_f_code */
  putbits(MPEdc_prec,2); /* intra_dc_precision */
  putbits(MPEpict_struct,2); /* picture_structure */
  putbits((MPEpict_struct==FRAME_PICTURE)?MPEtopfirst:0,1); /* top_field_first */
  putbits(MPEframe_pred_dct,1); /* frame_pred_frame_dct */
  putbits(0,1); /* concealment_motion_vectors  -- currently not implemented */
  putbits(MPEq_scale_type,1); /* q_scale_type */
  putbits(MPEintravlc,1); /* intra_vlc_format */
  putbits(MPEaltscan,1); /* alternate_scan */
  putbits(MPErepeatfirst,1); /* repeat_first_field */
  putbits(MPEprog_frame,1); /* chroma_420_type */
  putbits(MPEprog_frame,1); /* progressive_frame */
  putbits(0,1); /* composite_display_flag */
}

/* generate sequence_end_code (6.2.2) */
void putseqend()
{
  alignbits();
  putbits(SEQ_END_CODE,32);
}
