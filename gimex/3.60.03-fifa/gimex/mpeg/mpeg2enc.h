// (c) 1996-2007 Electronic Arts Inc.
/* mpg2enc.h, defines and types                                             */


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

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif


/* choose between declaration (GLOBAL undefined)
 * and definition (GLOBAL defined)
 * GLOBAL is defined in exactly one file (mpeg2enc.c)
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

/* control whether statistics file is generated or not */

#ifdef STATFILE
#define STATPRINT   fprintf
#else
#define STATPRINT   1 ? (int)0 : fprintf
#endif


/* constants */

#define PICTURE_START_CODE 0x100
#define SLICE_MIN_START    0x101
#define SLICE_MAX_START    0x1AF
#define USER_START_CODE    0x1B2
#define SEQ_START_CODE     0x1B3
#define EXT_START_CODE     0x1B5
#define SEQ_END_CODE       0x1B7
#define GOP_START_CODE     0x1B8
#define ISO_END_CODE       0x1B9
#define PACK_START_CODE    0x1BA
#define SYSTEM_START_CODE  0x1BB

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
#define MB_INTRA    1
#define MB_PATTERN  2
#define MB_BACKWARD 4
#define MB_FORWARD  8
#define MB_QUANT    16

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

#define SEQ_ID       1
#define DISP_ID      2
#define QUANT_ID     3
#define SEQSCAL_ID   5
#define PANSCAN_ID   7
#define CODING_ID    8
#define SPATSCAL_ID  9
#define TEMPSCAL_ID 10

/* inputtype */
#define T_Y_U_V 0
#define T_YUV   1
#define T_PPM   2

/* macroblock information */
struct MPEmbinfo {
  int mb_type; /* intra/forward/backward/interpolated */
  int motion_type; /* frame/field/16x8/dual_prime */
  int dct_type; /* field/frame DCT */
  int mquant; /* quantization parameter */
  int cbp; /* coded block pattern */
  int skipped; /* skipped macroblock */
  int MV[2][2][2]; /* motion vectors */
  int mv_field_sel[2][2]; /* motion vertical field select */
  int dmvector[2]; /* dual prime vectors */
  double act; /* activity measure */
  int var; /* for debugging */
};

/* motion data */
struct MPEmotion_data {
  int forw_hor_f_code,forw_vert_f_code; /* vector range */
  int sxf,syf; /* search range */
  int back_hor_f_code,back_vert_f_code;
  int sxb,syb;
};

/* prototypes of global functions */

/* conform.c */
void range_checks _ANSI_ARGS_((void));
void profile_and_level_checks _ANSI_ARGS_(());

/* fdctref.c */
void init_fdctref _ANSI_ARGS_((void));
void fdctref _ANSI_ARGS_((short *block));

/* idct.c */
void Fast_IDCT _ANSI_ARGS_((short *block));
void Initialize_Fast_IDCT _ANSI_ARGS_((void));

/* motion.c */
void motion_estimation _ANSI_ARGS_((unsigned char *oldorg, unsigned char *neworg,
  unsigned char *oldref, unsigned char *newref, unsigned char *cur,
  unsigned char *curref, int sxf, int syf, int sxb, int syb,
  struct MPEmbinfo *mbi, int secondfield, int ipflag));

/* mpgwrite.c */
void error _ANSI_ARGS_((char *text));

/* predict.c */
void predict _ANSI_ARGS_((unsigned char *reff[], unsigned char *refb[],
  unsigned char *cur[3], int secondfield, struct MPEmbinfo *mbi));

/* putbits.c */
void initbits _ANSI_ARGS_((void));
void putbits _ANSI_ARGS_((int val, int n));
void alignbits _ANSI_ARGS_((void));
int bitcount _ANSI_ARGS_((void));

/* puthdr.c */
void putseqhdr _ANSI_ARGS_((void));
void putseqext _ANSI_ARGS_((void));
void putseqdispext _ANSI_ARGS_((void));
void putuserdata _ANSI_ARGS_((char *userdata));
void putgophdr _ANSI_ARGS_((int frame, int closed_gop));
void putpicthdr _ANSI_ARGS_((void));
void putpictcodext _ANSI_ARGS_((void));
void putseqend _ANSI_ARGS_((void));

/* putmpg.c */
void putintrablk _ANSI_ARGS_((short *blk, int cc));
void putnonintrablk _ANSI_ARGS_((short *blk));
void putmv _ANSI_ARGS_((int dmv, int f_code));

/* putpic.c */
void putpict _ANSI_ARGS_((unsigned char *frame));

/* putvlc.c */
void putDClum _ANSI_ARGS_((int val));
void putDCchrom _ANSI_ARGS_((int val));
void putACfirst _ANSI_ARGS_((int run, int val));
void putAC _ANSI_ARGS_((int run, int signed_level, int vlcformat));
void putaddrinc _ANSI_ARGS_((int addrinc));
void putmbtype _ANSI_ARGS_((int MPEpict_type, int mb_type));
void putmotioncode _ANSI_ARGS_((int motion_code));
void putdmv _ANSI_ARGS_((int dmv));
void putcbp _ANSI_ARGS_((int cbp));

/* quantize.c */
int quant_intra _ANSI_ARGS_((short *src, short *dst, int MPEdc_prec,
  unsigned char *quant_mat, int mquant));
int quant_non_intra _ANSI_ARGS_((short *src, short *dst,
  unsigned char *quant_mat, int mquant));
void iquant_intra _ANSI_ARGS_((short *src, short *dst, int MPEdc_prec,
  unsigned char *quant_mat, int mquant));
void iquant_non_intra _ANSI_ARGS_((short *src, short *dst,
  unsigned char *quant_mat, int mquant));

/* ratectl.c */
void rc_init_seq _ANSI_ARGS_((void));
void rc_init_GOP _ANSI_ARGS_((int np, int nb));
void rc_init_pict _ANSI_ARGS_((unsigned char *frame));
void rc_update_pict _ANSI_ARGS_((void));
int rc_start_mb _ANSI_ARGS_((void));
int rc_calc_mquant _ANSI_ARGS_((int j));
void vbv_end_of_picture _ANSI_ARGS_((void));
void calc_vbv_delay _ANSI_ARGS_((void));

/* readpic.c */
void initreadframe _ANSI_ARGS_((void));
void readframe _ANSI_ARGS_((char *source, int rowbytes, unsigned char *frame[]));
void restorereadframe _ANSI_ARGS_((void));

/* stats.c */
void calcSNR _ANSI_ARGS_((unsigned char *org[3], unsigned char *rec[3]));
void stats _ANSI_ARGS_((void));

/* transfrm.c */
void transform _ANSI_ARGS_((unsigned char *pred[], unsigned char *cur[],
  struct MPEmbinfo *mbi, short MPEblocks[][64]));
void itransform _ANSI_ARGS_((unsigned char *pred[], unsigned char *cur[],
  struct MPEmbinfo *mbi, short MPEblocks[][64]));
void dct_type_estimation _ANSI_ARGS_((unsigned char *pred, unsigned char *cur,
  struct MPEmbinfo *mbi));


/* global variables */

/* zig-zag scan */
EXTERN unsigned char MPEzig_zag_scan[64]
#ifdef GLOBAL
=
{
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
  12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
  35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
  58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
}
#endif
;

/* alternate scan */
EXTERN unsigned char MPEalternate_scan[64]
#ifdef GLOBAL
=
{
  0,8,16,24,1,9,2,10,17,25,32,40,48,56,57,49,
  41,33,26,18,3,11,4,12,19,27,34,42,50,58,35,43,
  51,59,20,28,5,13,6,14,21,29,36,44,52,60,37,45,
  53,61,22,30,7,15,23,31,38,46,54,62,39,47,55,63
}
#endif
;

/* default intra quantization matrix */
EXTERN unsigned char MPEdefault_intra_quantizer_matrix[64]
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
EXTERN unsigned char MPEnon_linear_mquant_table[32]
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

/* non-linear mquant table for mapping from scale to code
 * since reconstruction levels are not bijective with the index map,
 * it is up to the designer to determine most of the quantization levels
 */

EXTERN unsigned char MPEmap_non_linear_mquant[113] 
#ifdef GLOBAL
=
{
0,1,2,3,4,5,6,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,
16,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,22,22,
22,22,23,23,23,23,24,24,24,24,24,24,24,25,25,25,25,25,25,25,26,26,
26,26,26,26,26,26,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,29,
29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,31,31,31,31,31
}
#endif
;

/* picture data arrays */

/* reconstructed frames */
EXTERN unsigned char *MPEnewrefframe[3], *MPEoldrefframe[3], *MPEauxframe[3];
/* original frames */
EXTERN unsigned char *MPEneworgframe[3], *MPEoldorgframe[3], *MPEauxorgframe[3];
/* prediction of current frame */
EXTERN unsigned char *MPEpredframe[3];
/* 8*8 block data */
EXTERN short (*MPEblocks)[64];
/* intra / non_intra quantization matrices */
EXTERN unsigned char MPEintra_q[64], MPEinter_q[64];
/* prediction values for DCT coefficient (0,0) */
EXTERN int MPEdc_dct_pred[3];
/* macroblock side information array */
EXTERN struct MPEmbinfo *MPEmbinfo;
/* motion estimation parameters */
EXTERN struct MPEmotion_data *MPEmotion_data;
/* clipping (=saturation) table */
EXTERN unsigned char *MPEclp;

/* name strings */
EXTERN char MPEid_string[256];
EXTERN char MPEiqname[256], MPEniqname[256];
EXTERN char MPEstatname[256];
EXTERN char MPEerrortext[256];

/* file descriptors */
EXTERN GSTREAM *MPEoutfile; 
EXTERN FILE    *MPEstatfile; 

EXTERN int MPEquiet; /* suppress warnings */


/* coding model parameters */

EXTERN int MPEn; /* number of frames in Group of Pictures */
EXTERN int MPEm; /* distance between I/P frames */
EXTERN int MPEtc0; /* timecode of first frame */
EXTERN int MPEmpeg1; /* ISO/IEC IS 11172-2 sequence */
EXTERN int MPEfieldpic; /* use field pictures */

/* sequence specific data (sequence header) */

EXTERN int MPEhorizontal_size, MPEvertical_size; /* frame size (pels) */
EXTERN int MPEwidth, MPEheight; /* encoded frame size (pels) multiples of 16 or 32 */
EXTERN int MPEchrom_width,MPEchrom_height,MPEblock_count;
EXTERN int MPEmb_width, MPEmb_height; /* frame size (macroblocks) */
EXTERN int MPEwidth2, MPEheight2, MPEmb_height2, MPEchrom_width2; /* picture size */
EXTERN int MPEaspectratio; /* aspect ratio information (pel or display) */
EXTERN int MPEframe_rate_code; /* coded value of frame rate */
EXTERN int MPEframe_rate_extension_d; /* frame rate extension denominator */
EXTERN double MPEframe_rate; /* frames per second */
EXTERN double MPEbit_rate; /* bits per second */
EXTERN int MPEvbv_buffer_size; /* size of VBV buffer (* 16 kbit) */
EXTERN int MPEconstrparms; /* constrained parameters flag (MPEG-1 only) */
EXTERN int MPEload_iquant, MPEload_niquant; /* use non-default quant. matrices */
EXTERN int MPEload_ciquant,MPEload_cniquant;


/* sequence specific data (sequence extension) */

EXTERN int MPEprofile, MPElevel; /* syntax / parameter constraints */
EXTERN int MPEprog_seq; /* progressive sequence */
EXTERN int MPEchroma_format;
EXTERN int MPElow_delay; /* no B pictures, skipped pictures */


/* sequence specific data (sequence display extension) */

EXTERN int MPEvideo_format; /* component, PAL, NTSC, SECAM or MAC */
EXTERN int MPEcolor_primaries; /* source primary chromaticity coordinates */
EXTERN int MPEtransfer_characteristics; /* opto-electronic transfer char. (gamma) */
EXTERN int MPEmatrix_coefficients; /* Eg,Eb,Er / Y,Cb,Cr matrix coefficients */
EXTERN int MPEdisplay_horizontal_size, MPEdisplay_vertical_size; /* display size */


/* picture specific data (picture header) */

EXTERN int MPEtemp_ref; /* temporal reference */
EXTERN int MPEpict_type; /* picture coding type (I, P or B) */
EXTERN int MPEvbv_delay; /* video buffering verifier delay (1/90000 seconds) */


/* picture specific data (picture coding extension) */

EXTERN int MPEforw_hor_f_code, MPEforw_vert_f_code;
EXTERN int MPEback_hor_f_code, MPEback_vert_f_code; /* motion vector ranges */
EXTERN int MPEdc_prec; /* DC coefficient precision for intra coded blocks */
EXTERN int MPEpict_struct; /* picture structure (frame, top / bottom field) */
EXTERN int MPEtopfirst; /* display top field first */
/* use only frame prediction and frame DCT (I,P,B,current) */
EXTERN int MPEframe_pred_dct_tab[3], MPEframe_pred_dct;
EXTERN int MPEconceal_tab[3]; /* use concealment motion vectors (I,P,B) */
EXTERN int MPEqscale_tab[3], MPEq_scale_type; /* linear/non-linear quantizaton table */
EXTERN int MPEintravlc_tab[3], MPEintravlc; /* intra vlc format (I,P,B,current) */
EXTERN int MPEaltscan_tab[3], MPEaltscan; /* alternate scan (I,P,B,current) */
EXTERN int MPErepeatfirst; /* repeat first field after second field */
EXTERN int MPEprog_frame; /* progressive frame */


#ifdef __cplusplus
}
#endif
