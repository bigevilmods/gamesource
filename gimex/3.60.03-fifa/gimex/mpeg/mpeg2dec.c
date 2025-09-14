// (c) 1996-2007 Electronic Arts Inc.

/* mpeg2dec.c, main(), initialization, option processing                    */


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

#define GLOBAL
#include "mpeg2dec.h"

/* private prototypes */
static int  Headers _ANSI_ARGS_((void));
static void Initialize_Sequence _ANSI_ARGS_((void));
static void Initialize_Decoder _ANSI_ARGS_((void));
static void Deinitialize_Sequence _ANSI_ARGS_((void));
static void Clear_Options();


/* memory allocation functions */
static MPDALLOC *MPDalloc;
static MPDFREE  *MPDfree;


int MPD_initdecoder(MPDALLOC *alloc, MPDFREE *free)
{
  MPDalloc = alloc;
  MPDfree = free;
  Clear_Options();
  MPDld = &MPDbase; /* select base layer context */
  Initialize_Decoder();

  return(1);
}


int MPD_decodeframe(int num, unsigned char *bitstream)
{
  Initialize_Buffer(bitstream);
  Headers();
  if (MPDbackward_reference_frame[0] == NULL) Initialize_Sequence();
  Decode_Picture(num, num);

  return(1);
}


int MPD_restoredecoder(void)
{
  Deinitialize_Sequence();
  MPDbackward_reference_frame[0] = NULL;
  MPDclip -= 384;
  MPDfree(MPDclip);

  return(1);
}


/* IMPLEMENTAION specific routines */
static void Initialize_Decoder()
{
  int i;

  /* Clip table */
  if (!(MPDclip=(unsigned char *)MPDalloc(1024)))
    Error("MPDclip[] malloc failed\n");

  MPDclip += 384;

  for (i=-384; i<640; i++)
    MPDclip[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  /* IDCT */
  Initialize_Fast_IDCT();
}

/* mostly IMPLEMENTAION specific rouintes */
static void Initialize_Sequence()
{
  int cc, size;
  static int Table_6_20[3] = {6,8,12};

  /* check scalability mode of enhancement layer */
  if (MPDtwo_streams && (MPDenhan.scalable_mode!=SC_SNR) && (MPDbase.scalable_mode!=SC_DP))
    Error("unsupported scalability mode\n");

  /* force MPEG-1 parameters for proper decoder behavior */
  /* see ISO/IEC 13818-2 section D.9.14 */
  if (!MPDbase.MPEG2_Flag)
  {
    MPDprogressive_sequence = 1;
    MPDprogressive_frame = 1;
    MPDpicture_structure = FRAME_PICTURE;
    MPDframe_pred_frame_dct = 1;
    MPDchroma_format = CHROMA420;
    MPDmatrix_coefficients = 5;
  }

  /* round to nearest multiple of coded macroblocks */
  /* ISO/IEC 13818-2 section 6.3.3 sequence_header() */
  MPDmb_width = (MPDhorizontal_size+15)/16;
  MPDmb_height = (MPDbase.MPEG2_Flag && !MPDprogressive_sequence) ? 2*((MPDvertical_size+31)/32)
                                        : (MPDvertical_size+15)/16;

  MPDcoded_picture_width = 16*MPDmb_width;
  MPDcoded_picture_height = 16*MPDmb_height;

  /* ISO/IEC 13818-2 sections 6.1.1.8, 6.1.1.9, and 6.1.1.10 */
  MPDchroma_width = (MPDchroma_format==CHROMA444) ? MPDcoded_picture_width
                                           : MPDcoded_picture_width>>1;
  MPDchroma_height = (MPDchroma_format!=CHROMA420) ? MPDcoded_picture_height
                                            : MPDcoded_picture_height>>1;
  
  /* derived based on Table 6-20 in ISO/IEC 13818-2 section 6.3.17 */
  MPDblock_count = Table_6_20[MPDchroma_format-1];

  for (cc=0; cc<3; cc++)
  {
    if (cc==0)
      size = MPDcoded_picture_width*MPDcoded_picture_height;
    else
      size = MPDchroma_width*MPDchroma_height;

    if (!(MPDbackward_reference_frame[cc] = (unsigned char *)MPDalloc(size)))
      Error("MPDbackward_reference_frame[] malloc failed\n");

    if (!(MPDforward_reference_frame[cc] = (unsigned char *)MPDalloc(size)))
      Error("MPDforward_reference_frame[] malloc failed\n");

    if (!(MPDauxframe[cc] = (unsigned char *)MPDalloc(size)))
      Error("MPDauxframe[] malloc failed\n");
  }
}


void Error(text)
char *text;
{
#ifdef DEBUG
    printf(text);
#endif
}


/* Trace_Flag output */
void Print_Bits(code,bits,len)
int code,bits,len;
{
  int i;
  for (i=0; i<len; i++)
    printf("%d",(code>>(bits-1-i))&1);
}


static int Headers()
{
  int ret;

  MPDld = &MPDbase;
  

  /* return when end of sequence (0) or picture
     header has been parsed (1) */

  ret = Get_Hdr();


  if (MPDtwo_streams)
  {
    MPDld = &MPDenhan;
    if (Get_Hdr()!=ret && !MPDquiet_flag)
      printf("streams out of sync\n");
    MPDld = &MPDbase;
  }

  return ret;
}


static void Deinitialize_Sequence()
{
  int i;

  /* clear flags */
  MPDbase.MPEG2_Flag=0;

  for(i=0;i<3;i++)
  {
    MPDfree(MPDbackward_reference_frame[i]);
    MPDfree(MPDforward_reference_frame[i]);
    MPDfree(MPDauxframe[i]);
  }
}


static void Clear_Options()
{
  MPDverbose_flag = 0;
  MPDtrace_flag = 0;
  MPDquiet_flag = 0;
  MPDtwo_streams = 0;
}
