// (c) 1996-2007 Electronic Arts Inc.

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

#ifdef __cplusplus
extern "C" {
#endif

/* choose between declaration (GLOBAL undefined)
 * and definition (GLOBAL defined)
 * GLOBAL is defined in exactly one file mpeg2dec.c)
 */

#ifndef EXTERN
#ifndef GLOBAL
#define EXTERN extern
#else
#define EXTERN
#endif
#endif

/* define NON_ANSI_COMPILER for compilers without function prototyping */
/* #define NON_ANSI_COMPILER */

#ifdef NON_ANSI_COMPILER
#define _ANSI_ARGS_(x) ()
#else
#define _ANSI_ARGS_(x) x
#endif


/* constants */

#define ERROR (-1)

#define PICTURE_START_CODE      0x100
#define SLICE_START_CODE_MIN    0x101
#define SLICE_START_CODE_MAX    0x1AF
#define USER_DATA_START_CODE    0x1B2
#define SEQUENCE_HEADER_CODE    0x1B3
#define SEQUENCE_ERROR_CODE     0x1B4
#define EXTENSION_START_CODE    0x1B5
#define SEQUENCE_END_CODE       0x1B7
#define GROUP_START_CODE        0x1B8
#define SYSTEM_START_CODE_MIN   0x1B9
#define SYSTEM_START_CODE_MAX   0x1FF

#define ISO_END_CODE            0x1B9
#define PACK_START_CODE         0x1BA
#define SYSTEM_START_CODE       0x1BB

#define VIDEO_ELEMENTARY_STREAM 0x1e0

/* scalable_mode */
#define SC_NONE 0
#define SC_DP   1
#define SC_SPAT 2
#define SC_SNR  3
#define SC_TEMP 4

/* picture coding type */
#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4

/* picture structure */
#define TOP_FIELD     1
#define BOTTOM_FIELD  2
#define FRAME_PICTURE 3

/* macroblock type */
#define MACROBLOCK_INTRA                        1
#define MACROBLOCK_PATTERN                      2
#define MACROBLOCK_MOTION_BACKWARD              4
#define MACROBLOCK_MOTION_FORWARD               8
#define MACROBLOCK_QUANT                        16
#define SPATIAL_TEMPORAL_WEIGHT_CODE_FLAG       32
#define PERMITTED_SPATIAL_TEMPORAL_WEIGHT_CLASS 64

/* motion_type */
#define MC_FIELD 1
#define MC_FRAME 2
#define MC_16X8  2
#define MC_DMV   3

/* mv_format */
#define MV_FIELD 0
#define MV_FRAME 1

/* chroma_format */
#define CHROMA420 1
#define CHROMA422 2
#define CHROMA444 3

/* extension start code IDs */

#define SEQUENCE_EXTENSION_ID                    1
#define SEQUENCE_DISPLAY_EXTENSION_ID            2
#define QUANT_MATRIX_EXTENSION_ID                3
#define COPYRIGHT_EXTENSION_ID                   4
#define SEQUENCE_SCALABLE_EXTENSION_ID           5
#define PICTURE_DISPLAY_EXTENSION_ID             7
#define PICTURE_CODING_EXTENSION_ID              8
#define PICTURE_SPATIAL_SCALABLE_EXTENSION_ID    9
#define PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID  10

#define ZIG_ZAG                                  0

#define PROFILE_422                             (128+5)
#define MAIN_LEVEL                              8

/* Layers: used by Verbose_Flag, Verifier_Flag, Stats_Flag, and Trace_Flag */
#define NO_LAYER                                0
#define SEQUENCE_LAYER                          1
#define PICTURE_LAYER                           2
#define SLICE_LAYER                             3    
#define MACROBLOCK_LAYER                        4    
#define BLOCK_LAYER                             5
#define EVENT_LAYER                             6
#define ALL_LAYERS                              7

#define MB_WEIGHT                  32
#define MB_CLASS4                  64


/* type definitions */

typedef void *MPDALLOC(unsigned int size);
typedef void  MPDFREE(void *ptr);


/* prototypes of global functions */

/* getbits.c */
void Initialize_Buffer _ANSI_ARGS_((unsigned char *bitstream));
unsigned int Show_Bits _ANSI_ARGS_((int n));
unsigned int Get_Bits1 _ANSI_ARGS_((void));
void Flush_Buffer _ANSI_ARGS_((int n));
unsigned int Get_Bits _ANSI_ARGS_((int n));
void Flush_Buffer32 _ANSI_ARGS_((void));
unsigned int Get_Bits32 _ANSI_ARGS_((void));

/* getblk.c */
void Decode_MPEG1_Intra_Block _ANSI_ARGS_((int comp, int dc_dct_pred[]));
void Decode_MPEG1_Non_Intra_Block _ANSI_ARGS_((int comp));
void Decode_MPEG2_Intra_Block _ANSI_ARGS_((int comp, int dc_dct_pred[]));
void Decode_MPEG2_Non_Intra_Block _ANSI_ARGS_((int comp));

/* gethdr.c */
int Get_Hdr _ANSI_ARGS_((void));
void next_start_code _ANSI_ARGS_((void));
int slice_header _ANSI_ARGS_((void));
void marker_bit _ANSI_ARGS_((char *text));

/* getpic.c */
void Decode_Picture _ANSI_ARGS_((int bitstream_framenum, 
  int sequence_framenum));

/* getvlc.c */
int Get_macroblock_type _ANSI_ARGS_((void));
int Get_motion_code _ANSI_ARGS_((void));
int Get_dmvector _ANSI_ARGS_((void));
int Get_coded_block_pattern _ANSI_ARGS_((void));
int Get_macroblock_address_increment _ANSI_ARGS_((void));
int Get_Luma_DC_dct_diff _ANSI_ARGS_((void));
int Get_Chroma_DC_dct_diff _ANSI_ARGS_((void));

/* idct.c */
void Fast_IDCT _ANSI_ARGS_((short *block));
void Initialize_Fast_IDCT _ANSI_ARGS_((void));

/* motion.c */
void motion_vectors _ANSI_ARGS_((int PMV[2][2][2], int dmvector[2],
  int motion_vertical_field_select[2][2], int s, int motion_vector_count, 
  int mv_format, int h_r_size, int v_r_size, int dmv, int mvscale));
void motion_vector _ANSI_ARGS_((int *PMV, int *dmvector,
  int h_r_size, int v_r_size, int dmv, int mvscale, int full_pel_vector));
void Dual_Prime_Arithmetic _ANSI_ARGS_((int DMV[][2], int *dmvector, int mvx, int mvy));

/* mpeg2dec.c */
int  MPD_initdecoder(MPDALLOC *alloc, MPDFREE *free);
int  MPD_decodeframe(int num, unsigned char *bitstream);
int  MPD_restoredecoder(void);
void Error _ANSI_ARGS_((char *text));
void Print_Bits _ANSI_ARGS_((int code, int bits, int len));

/* recon.c */
void form_predictions _ANSI_ARGS_((int bx, int by, int macroblock_type, 
  int motion_type, int PMV[2][2][2], int motion_vertical_field_select[2][2], 
  int dmvector[2], int stwtype));

/* verify.c */
#ifdef VERIFY
void Check_Headers _ANSI_ARGS_((int Bitstream_Framenum, int Sequence_Framenum));
void Clear_Verify_Headers _ANSI_ARGS_((void));
#endif /* VERIFY */


/* global variables */

/* zig-zag and alternate scan patterns */
EXTERN unsigned char MPDscan[2][64]
#ifdef GLOBAL
=
{
  { /* Zig-Zag scan pattern  */
    0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
    12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
    35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
  },
  { /* Alternate scan pattern */
    0,8,16,24,1,9,2,10,17,25,32,40,48,56,57,49,
    41,33,26,18,3,11,4,12,19,27,34,42,50,58,35,43,
    51,59,20,28,5,13,6,14,21,29,36,44,52,60,37,45,
    53,61,22,30,7,15,23,31,38,46,54,62,39,47,55,63
  }
}
#endif
;

/* default intra quantization matrix */
EXTERN unsigned char MPDdefault_intra_quantizer_matrix[64]
#ifdef GLOBAL
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
}
#endif
;

/* non-linear quantization coefficient table */
EXTERN unsigned char MPDnon_linear_quantizer_scale[32]
#ifdef GLOBAL
=
{
   0, 1, 2, 3, 4, 5, 6, 7,
   8,10,12,14,16,18,20,22,
  24,28,32,36,40,44,48,52,
  56,64,72,80,88,96,104,112
}
#endif
;

/* color space conversion coefficients
 * for YCbCr -> RGB mapping
 *
 * entries are {crv,cbu,cgu,cgv}
 *
 * crv=(255/224)*65536*(1-cr)/0.5
 * cbu=(255/224)*65536*(1-cb)/0.5
 * cgu=(255/224)*65536*(cb/cg)*(1-cb)/0.5
 * cgv=(255/224)*65536*(cr/cg)*(1-cr)/0.5
 *
 * where Y=cr*R+cg*G+cb*B (cr+cg+cb=1)
 */

/* ISO/IEC 13818-2 section 6.3.6 sequence_display_extension() */

EXTERN int MPDinverse_table_6_9[8][4]
#ifdef GLOBAL
=
{
  {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
  {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
  {104597, 132201, 25675, 53279}, /* unspecified */
  {104597, 132201, 25675, 53279}, /* reserved */
  {104448, 132798, 24759, 53109}, /* FCC */
  {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
  {104597, 132201, 25675, 53279}, /* SMPTE 170M */
  {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
}
#endif
;

/* decoder operation control flags */
EXTERN int MPDquiet_flag;
EXTERN int MPDtrace_flag;
EXTERN int MPDfault_flag;
EXTERN int MPDverbose_flag;
EXTERN int MPDtwo_streams;

/* buffers for multiuse purposes */
EXTERN unsigned char *MPDclip;

/* pointers to generic picture buffers */
EXTERN unsigned char *MPDbackward_reference_frame[3];
EXTERN unsigned char *MPDforward_reference_frame[3];
EXTERN unsigned char *MPDauxframe[3];
EXTERN unsigned char *MPDcurrent_frame[3];

/* non-normative variables derived from normative elements */
EXTERN int MPDcoded_picture_width;
EXTERN int MPDcoded_picture_height;
EXTERN int MPDchroma_width;
EXTERN int MPDchroma_height;
EXTERN int MPDblock_count;
EXTERN int MPDsecond_field;
EXTERN int MPDprofile, MPDlevel;

/* normative derived variables (as per ISO/IEC 13818-2) */
EXTERN int MPDhorizontal_size;
EXTERN int MPDvertical_size;
EXTERN int MPDmb_width;
EXTERN int MPDmb_height;
EXTERN double MPDbit_rate;
EXTERN double MPDframe_rate; 

/* headers */

/* ISO/IEC 13818-2 section 6.2.2.1:  sequence_header() */
EXTERN int MPDaspect_ratio_information;
EXTERN int MPDframe_rate_code; 
EXTERN int MPDbit_rate_value; 
EXTERN int MPDvbv_buffer_size;
EXTERN int MPDconstrained_parameters_flag;

/* ISO/IEC 13818-2 section 6.2.2.3:  sequence_extension() */
EXTERN int MPDprofile_and_level_indication;
EXTERN int MPDprogressive_sequence;
EXTERN int MPDchroma_format;
EXTERN int MPDlow_delay;
EXTERN int MPDframe_rate_extension_n;
EXTERN int MPDframe_rate_extension_d;

/* ISO/IEC 13818-2 section 6.2.2.4:  sequence_display_extension() */
EXTERN int MPDvideo_format;  
EXTERN int MPDcolor_description;
EXTERN int MPDcolor_primaries;
EXTERN int MPDtransfer_characteristics;
EXTERN int MPDmatrix_coefficients;
EXTERN int MPDdisplay_horizontal_size;
EXTERN int MPDdisplay_vertical_size;

/* ISO/IEC 13818-2 section 6.2.3: picture_header() */
EXTERN int MPDtemporal_reference;
EXTERN int MPDpicture_coding_type;
EXTERN int MPDvbv_delay;
EXTERN int MPDfull_pel_forward_vector;
EXTERN int MPDforward_f_code;
EXTERN int MPDfull_pel_backward_vector;
EXTERN int MPDbackward_f_code;

/* ISO/IEC 13818-2 section 6.2.3.1: picture_coding_extension() header */
EXTERN int MPDf_code[2][2];
EXTERN int MPDintra_dc_precision;
EXTERN int MPDpicture_structure;
EXTERN int MPDtop_field_first;
EXTERN int MPDframe_pred_frame_dct;
EXTERN int MPDconcealment_motion_vectors;

EXTERN int MPDintra_vlc_format;

EXTERN int MPDrepeat_first_field;

EXTERN int MPDchroma_420_type;
EXTERN int MPDprogressive_frame;
EXTERN int MPDcomposite_display_flag;
EXTERN int MPDv_axis;
EXTERN int MPDfield_sequence;
EXTERN int MPDsub_carrier;
EXTERN int MPDburst_amplitude;
EXTERN int MPDsub_carrier_phase;

/* ISO/IEC 13818-2 section 6.2.3.3: picture_display_extension() header */
EXTERN int MPDframe_center_horizontal_offset[3];
EXTERN int MPDframe_center_vertical_offset[3];

/* ISO/IEC 13818-2 section 6.2.2.5: sequence_scalable_extension() header */
EXTERN int MPDlayer_id;
EXTERN int MPDlower_layer_prediction_horizontal_size;
EXTERN int MPDlower_layer_prediction_vertical_size;
EXTERN int MPDhorizontal_subsampling_factor_m;
EXTERN int MPDhorizontal_subsampling_factor_n;
EXTERN int MPDvertical_subsampling_factor_m;
EXTERN int MPDvertical_subsampling_factor_n;

/* ISO/IEC 13818-2 section 6.2.3.5: picture_spatial_scalable_extension() header */
EXTERN int MPDlower_layer_temporal_reference;
EXTERN int MPDlower_layer_horizontal_offset;
EXTERN int MPDlower_layer_vertical_offset;
EXTERN int MPDspatial_temporal_weight_code_table_index;
EXTERN int MPDlower_layer_progressive_frame;
EXTERN int MPDlower_layer_deinterlaced_field_select;

/* ISO/IEC 13818-2 section 6.2.3.6: copyright_extension() header */
EXTERN int MPDcopyright_flag;
EXTERN int MPDcopyright_identifier;
EXTERN int MPDoriginal_or_copy;
EXTERN int MPDcopyright_number_1;
EXTERN int MPDcopyright_number_2;
EXTERN int MPDcopyright_number_3;

/* ISO/IEC 13818-2 section 6.2.2.6: group_of_pictures_header()  */
EXTERN int MPDdrop_flag;
EXTERN int MPDhour;
EXTERN int MPDminute;
EXTERN int MPDsec;
EXTERN int MPDframe;
EXTERN int MPDclosed_gop;
EXTERN int MPDbroken_link;

/* layer specific variables (needed for SNR and DP scalability) */
EXTERN struct layer_data {
  /* bit input */
  unsigned char *Rdptr;
  unsigned int Bfr;
  int Incnt;
  int Bitcnt;
  /* sequence header and quant_matrix_extension() */
  int intra_quantizer_matrix[64];
  int non_intra_quantizer_matrix[64];
  int chroma_intra_quantizer_matrix[64];
  int chroma_non_intra_quantizer_matrix[64];
  
  int load_intra_quantizer_matrix;
  int load_non_intra_quantizer_matrix;
  int load_chroma_intra_quantizer_matrix;
  int load_chroma_non_intra_quantizer_matrix;

  int MPEG2_Flag;
  /* sequence scalable extension */
  int scalable_mode;
  /* picture coding extension */
  int q_scale_type;
  int alternate_scan;
  /* picture spatial scalable extension */
  int pict_scal;
  /* slice/macroblock */
  int priority_breakpoint;
  int quantizer_scale;
  int intra_slice;
  double force_8_byte_alignment;
  short block[12][64];
} MPDbase, MPDenhan, *MPDld;

/* Enables MMX code on Intel platforms when set */
EXTERN int MPDusemmx;

#ifdef VERIFY
EXTERN int MPDverify_sequence_header;
EXTERN int MPDverify_group_of_pictures_header;
EXTERN int MPDverify_picture_header;
EXTERN int MPDverify_slice_header;
EXTERN int MPDverify_sequence_extension;
EXTERN int MPDverify_sequence_display_extension;
EXTERN int MPDverify_quant_matrix_extension;
EXTERN int MPDverify_sequence_scalable_extension;
EXTERN int MPDverify_picture_display_extension;
EXTERN int MPDverify_picture_coding_extension;
EXTERN int MPDverify_picture_spatial_scalable_extension;
EXTERN int MPDverify_picture_temporal_scalable_extension;
EXTERN int MPDverify_copyright_extension;
#endif /* VERIFY */

EXTERN int MPDglobal_mba;
EXTERN int MPDglobal_pic;
EXTERN int MPDtrue_framenum;


/* Inline functions */

static void __inline Inline_Flush_Buffer(int N)
{
  int Incnt;

  MPDbase.Bfr <<= N;

  Incnt = MPDbase.Incnt - N;

  while (Incnt <= 24)
  {
    MPDbase.Bfr |= *MPDbase.Rdptr++ << (24 - Incnt);
    Incnt += 8;
  }

  MPDbase.Incnt = Incnt;
}


#ifdef __cplusplus
}
#endif
