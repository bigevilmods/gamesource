// (c) 1996-2007 Electronic Arts Inc.
/* stats.c, coding statistics                                               */


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
static void calcSNR1 _ANSI_ARGS_((unsigned char *org, unsigned char *rec,
  int lx, int w, int h, double *pv, double *pe));


void calcSNR(org,rec)
unsigned char *org[3];
unsigned char *rec[3];
{
  int w,h,offs;
  double v,e;

  w = MPEhorizontal_size;
  h = (MPEpict_struct==FRAME_PICTURE) ? MPEvertical_size : (MPEvertical_size>>1);
  offs = (MPEpict_struct==BOTTOM_FIELD) ? MPEwidth : 0;

  calcSNR1(org[0]+offs,rec[0]+offs,MPEwidth2,w,h,&v,&e);
  STATPRINT(MPEstatfile,"Y: variance=%4.4g, MSE=%3.3g (%3.3g dB), SNR=%3.3g dB\n",
    v, e, 10.0*log10(255.0*255.0/e), 10.0*log10(v/e));

  if (MPEchroma_format!=CHROMA444)
  {
    w >>= 1;
    offs >>= 1;
  }

  if (MPEchroma_format==CHROMA420)
    h >>= 1;

  calcSNR1(org[1]+offs,rec[1]+offs,MPEchrom_width2,w,h,&v,&e);
  STATPRINT(MPEstatfile,"U: variance=%4.4g, MSE=%3.3g (%3.3g dB), SNR=%3.3g dB\n",
    v, e, 10.0*log10(255.0*255.0/e), 10.0*log10(v/e));

  calcSNR1(org[2]+offs,rec[2]+offs,MPEchrom_width2,w,h,&v,&e);
  STATPRINT(MPEstatfile,"V: variance=%4.4g, MSE=%3.3g (%3.3g dB), SNR=%3.3g dB\n",
    v, e, 10.0*log10(255.0*255.0/e), 10.0*log10(v/e));
}

static void calcSNR1(org,rec,lx,w,h,pv,pe)
unsigned char *org;
unsigned char *rec;
int lx,w,h;
double *pv,*pe;
{
  int i, j;
  double v1, s1, s2, e2;

  s1 = s2 = e2 = 0.0;

  for (j=0; j<h; j++)
  {
    for (i=0; i<w; i++)
    {
      v1 = org[i];
      s1+= v1;
      s2+= v1*v1;
      v1-= rec[i];
      e2+= v1*v1;
    }
    org += lx;
    rec += lx;
  }

  s1 /= w*h;
  s2 /= w*h;
  e2 /= w*h;
  v1 = s2 - s1*s1;

  /* PRATTO: prevent problems in calcSNR() */
  if (v1 < 0.00001) v1 = 0.00001;
  if (e2 < 0.00001) e2 = 0.00001;

  *pv = v1;     /* variance */
  *pe = e2;     /* MSE */
}

void stats()
{
  int i, j, k, nmb, mb_type;
  int n_skipped, n_intra, n_ncoded, n_blocks, n_interp, n_forward, n_backward;
  struct MPEmbinfo *mbi;

  nmb = MPEmb_width*MPEmb_height2;

  n_skipped=n_intra=n_ncoded=n_blocks=n_interp=n_forward=n_backward=0;

  for (k=0; k<nmb; k++)
  {
    mbi = MPEmbinfo+k;
    if (mbi->skipped)
      n_skipped++;
    else if (mbi->mb_type & MB_INTRA)
      n_intra++;
    else if (!(mbi->mb_type & MB_PATTERN))
      n_ncoded++;

    for (i=0; i<MPEblock_count; i++)
      if (mbi->cbp & (1<<i))
        n_blocks++;

    if (mbi->mb_type & MB_FORWARD)
    {
      if (mbi->mb_type & MB_BACKWARD)
        n_interp++;
      else
        n_forward++;
    }
    else if (mbi->mb_type & MB_BACKWARD)
      n_backward++;
  }

  STATPRINT(MPEstatfile,"\npicture statistics:\n");
  STATPRINT(MPEstatfile," # of intra coded macroblocks:  %4d (%.1f%%)\n",
    n_intra,100.0*(double)n_intra/nmb);
  STATPRINT(MPEstatfile," # of coded blocks:             %4d (%.1f%%)\n",
    n_blocks,100.0*(double)n_blocks/(MPEblock_count*nmb));
  STATPRINT(MPEstatfile," # of not coded macroblocks:    %4d (%.1f%%)\n",
    n_ncoded,100.0*(double)n_ncoded/nmb);
  STATPRINT(MPEstatfile," # of skipped macroblocks:      %4d (%.1f%%)\n",
    n_skipped,100.0*(double)n_skipped/nmb);
  STATPRINT(MPEstatfile," # of forw. pred. macroblocks:  %4d (%.1f%%)\n",
    n_forward,100.0*(double)n_forward/nmb);
  STATPRINT(MPEstatfile," # of backw. pred. macroblocks: %4d (%.1f%%)\n",
    n_backward,100.0*(double)n_backward/nmb);
  STATPRINT(MPEstatfile," # of interpolated macroblocks: %4d (%.1f%%)\n",
    n_interp,100.0*(double)n_interp/nmb);

  STATPRINT(MPEstatfile,"\nmacroblock_type map:\n");

  k = 0;

  for (j=0; j<MPEmb_height2; j++)
  {
    for (i=0; i<MPEmb_width; i++)
    {
      mbi = MPEmbinfo + k;
      mb_type = mbi->mb_type;
      if (mbi->skipped)
        STATPRINT(MPEstatfile, "%c", 'S');
      else if (mb_type & MB_INTRA)
        STATPRINT(MPEstatfile, "%c", 'I');
      else switch (mb_type & (MB_FORWARD|MB_BACKWARD))
      {
      case MB_FORWARD:
        STATPRINT(MPEstatfile, "%c",
             mbi->motion_type==MC_FIELD ? 'f' :
             mbi->motion_type==MC_DMV   ? 'p' : 'F'); break;
      case MB_BACKWARD:
        STATPRINT(MPEstatfile, "%c",
             mbi->motion_type==MC_FIELD ? 'b' : 'B'); break;
      case MB_FORWARD|MB_BACKWARD:
        STATPRINT(MPEstatfile, "%c",
             mbi->motion_type==MC_FIELD ? 'd' : 'D'); break;
      default:
        STATPRINT(MPEstatfile, "%c", '0'); break;
      }

      if (mb_type & MB_QUANT)
        STATPRINT(MPEstatfile, "%c", 'Q');
      else if (mb_type & (MB_PATTERN|MB_INTRA))
        STATPRINT(MPEstatfile, "%c", ' ');
      else
        STATPRINT(MPEstatfile, "%c", 'N');

      STATPRINT(MPEstatfile, "%c", ' ');

      k++;
    }
    STATPRINT(MPEstatfile, "%c", '\n');
  }

  STATPRINT(MPEstatfile,"\nmquant map:\n");

  k=0;
  for (j=0; j<MPEmb_height2; j++)
  {
    for (i=0; i<MPEmb_width; i++)
    {
      if (i==0 || MPEmbinfo[k].mquant!=MPEmbinfo[k-1].mquant)
        STATPRINT(MPEstatfile,"%3d",MPEmbinfo[k].mquant);
      else
        STATPRINT(MPEstatfile,"   ");

      k++;
    }
    STATPRINT(MPEstatfile, "%c", '\n');
  }

#if 0
  STATPRINT(MPEstatfile,"\ncbp map:\n");

  k=0;
  for (j=0; j<MPEmb_height2; j++)
  {
    for (i=0; i<MPEmb_width; i++)
    {
      STATPRINT(MPEstatfile,"%02x ",MPEmbinfo[k].cbp);

      k++;
    }
    STATPRINT(MPEstatfile, "%c", '\n');
  }

  if (MPEpict_struct==FRAME_PICTURE && !MPEframe_pred_dct)
  {
    STATPRINT(MPEstatfile,"\ndct_type map:\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & (MB_PATTERN|MB_INTRA))
          STATPRINT(MPEstatfile,"%d  ",MPEmbinfo[k].dct_type);
        else
          STATPRINT(MPEstatfile,"   ");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }
  }

  if (MPEpict_type!=I_TYPE)
  {
    STATPRINT(MPEstatfile,"\nforward motion vectors (first vector, horizontal):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_FORWARD)
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[0][0][0]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }

    STATPRINT(MPEstatfile,"\nforward motion vectors (first vector, vertical):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_FORWARD)
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[0][0][1]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }

    STATPRINT(MPEstatfile,"\nforward motion vectors (second vector, horizontal):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_FORWARD
            && ((MPEpict_struct==FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_FIELD) ||
                (MPEpict_struct!=FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_16X8)))
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[1][0][0]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }

    STATPRINT(MPEstatfile,"\nforward motion vectors (second vector, vertical):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_FORWARD
            && ((MPEpict_struct==FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_FIELD) ||
                (MPEpict_struct!=FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_16X8)))
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[1][0][1]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }
  }
    
  if (MPEpict_type==B_TYPE)
  {
    STATPRINT(MPEstatfile,"\nbackward motion vectors (first vector, horizontal):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_BACKWARD)
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[0][1][0]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }

    STATPRINT(MPEstatfile,"\nbackward motion vectors (first vector, vertical):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_BACKWARD)
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[0][1][1]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }

    STATPRINT(MPEstatfile,"\nbackward motion vectors (second vector, horizontal):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_BACKWARD
            && ((MPEpict_struct==FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_FIELD) ||
                (MPEpict_struct!=FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_16X8)))
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[1][1][0]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }

    STATPRINT(MPEstatfile,"\nbackward motion vectors (second vector, vertical):\n");

    k=0;
    for (j=0; j<MPEmb_height2; j++)
    {
      for (i=0; i<MPEmb_width; i++)
      {
        if (MPEmbinfo[k].mb_type & MB_BACKWARD
            && ((MPEpict_struct==FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_FIELD) ||
                (MPEpict_struct!=FRAME_PICTURE && MPEmbinfo[k].motion_type==MC_16X8)))
          STATPRINT(MPEstatfile,"%4d",MPEmbinfo[k].MV[1][1][1]);
        else
          STATPRINT(MPEstatfile,"   .");
  
        k++;
      }
      STATPRINT(MPEstatfile, "%c", '\n');
    }
  }
#endif
    
#if 0
  /* useful for debugging */
  STATPRINT(MPEstatfile,"\nmacroblock info dump:\n");

  k=0;
  for (j=0; j<MPEmb_height2; j++)
  {
    for (i=0; i<MPEmb_width; i++)
    {
      STATPRINT(MPEstatfile,"%d: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
      k,
      MPEmbinfo[k].mb_type,
      MPEmbinfo[k].motion_type,
      MPEmbinfo[k].dct_type,
      MPEmbinfo[k].mquant,
      MPEmbinfo[k].cbp,
      MPEmbinfo[k].skipped,
      MPEmbinfo[k].MV[0][0][0],
      MPEmbinfo[k].MV[0][0][1],
      MPEmbinfo[k].MV[0][1][0],
      MPEmbinfo[k].MV[0][1][1],
      MPEmbinfo[k].MV[1][0][0],
      MPEmbinfo[k].MV[1][0][1],
      MPEmbinfo[k].MV[1][1][0],
      MPEmbinfo[k].MV[1][1][1],
      MPEmbinfo[k].mv_field_sel[0][0],
      MPEmbinfo[k].mv_field_sel[0][1],
      MPEmbinfo[k].mv_field_sel[1][0],
      MPEmbinfo[k].mv_field_sel[1][1]);

      k++;
    }
  }
#endif
}
