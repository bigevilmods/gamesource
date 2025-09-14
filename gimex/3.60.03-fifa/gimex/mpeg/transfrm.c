// (c) 1996-2007 Electronic Arts Inc.
/* transfrm.c,  forward / inverse transformation                            */


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

/* private prototypes*/
static void add_pred _ANSI_ARGS_((unsigned char *pred, unsigned char *cur,
  int lx, short *blk));
static void sub_pred _ANSI_ARGS_((unsigned char *pred, unsigned char *cur,
  int lx, short *blk));

/* subtract prediction and transform prediction error */
void transform(pred,cur,mbi,MPEblocks)
unsigned char *pred[], *cur[];
struct MPEmbinfo *mbi;
short MPEblocks[][64];
{
  int i, j, i1, j1, k, n, cc, offs, lx;

  k = 0;

  for (j=0; j<MPEheight2; j+=16)
    for (i=0; i<MPEwidth; i+=16)
    {
      for (n=0; n<MPEblock_count; n++)
      {
        cc = (n<4) ? 0 : (n&1)+1; /* color component index */
        if (cc==0)
        {
          /* luminance */
          if ((MPEpict_struct==FRAME_PICTURE) && mbi[k].dct_type)
          {
            /* field DCT */
            offs = i + ((n&1)<<3) + MPEwidth*(j+((n&2)>>1));
            lx = MPEwidth<<1;
          }
          else
          {
            /* frame DCT */
            offs = i + ((n&1)<<3) + MPEwidth2*(j+((n&2)<<2));
            lx = MPEwidth2;
          }

          if (MPEpict_struct==BOTTOM_FIELD)
            offs += MPEwidth;
        }
        else
        {
          /* chrominance */

          /* scale coordinates */
          i1 = (MPEchroma_format==CHROMA444) ? i : i>>1;
          j1 = (MPEchroma_format!=CHROMA420) ? j : j>>1;

          if ((MPEpict_struct==FRAME_PICTURE) && mbi[k].dct_type
              && (MPEchroma_format!=CHROMA420))
          {
            /* field DCT */
            offs = i1 + (n&8) + MPEchrom_width*(j1+((n&2)>>1));
            lx = MPEchrom_width<<1;
          }
          else
          {
            /* frame DCT */
            offs = i1 + (n&8) + MPEchrom_width2*(j1+((n&2)<<2));
            lx = MPEchrom_width2;
          }

          if (MPEpict_struct==BOTTOM_FIELD)
            offs += MPEchrom_width;
        }

        sub_pred(pred[cc]+offs,cur[cc]+offs,lx,MPEblocks[k*MPEblock_count+n]);
        fdctref(MPEblocks[k*MPEblock_count+n]);
      }

      k++;
    }
}

/* inverse transform prediction error and add prediction */
void itransform(pred,cur,mbi,MPEblocks)
unsigned char *pred[],*cur[];
struct MPEmbinfo *mbi;
short MPEblocks[][64];
{
  int i, j, i1, j1, k, n, cc, offs, lx;

  k = 0;

  for (j=0; j<MPEheight2; j+=16)
    for (i=0; i<MPEwidth; i+=16)
    {
      for (n=0; n<MPEblock_count; n++)
      {
        cc = (n<4) ? 0 : (n&1)+1; /* color component index */

        if (cc==0)
        {
          /* luminance */
          if ((MPEpict_struct==FRAME_PICTURE) && mbi[k].dct_type)
          {
            /* field DCT */
            offs = i + ((n&1)<<3) + MPEwidth*(j+((n&2)>>1));
            lx = MPEwidth<<1;
          }
          else
          {
            /* frame DCT */
            offs = i + ((n&1)<<3) + MPEwidth2*(j+((n&2)<<2));
            lx = MPEwidth2;
          }

          if (MPEpict_struct==BOTTOM_FIELD)
            offs += MPEwidth;
        }
        else
        {
          /* chrominance */

          /* scale coordinates */
          i1 = (MPEchroma_format==CHROMA444) ? i : i>>1;
          j1 = (MPEchroma_format!=CHROMA420) ? j : j>>1;

          if ((MPEpict_struct==FRAME_PICTURE) && mbi[k].dct_type
              && (MPEchroma_format!=CHROMA420))
          {
            /* field DCT */
            offs = i1 + (n&8) + MPEchrom_width*(j1+((n&2)>>1));
            lx = MPEchrom_width<<1;
          }
          else
          {
            /* frame DCT */
            offs = i1 + (n&8) + MPEchrom_width2*(j1+((n&2)<<2));
            lx = MPEchrom_width2;
          }

          if (MPEpict_struct==BOTTOM_FIELD)
            offs += MPEchrom_width;
        }

        Fast_IDCT(MPEblocks[k*MPEblock_count+n]);
        add_pred(pred[cc]+offs,cur[cc]+offs,lx,MPEblocks[k*MPEblock_count+n]);
      }

      k++;
    }
}

/* add prediction and prediction error, saturate to 0...255 */
static void add_pred(pred,cur,lx,blk)
unsigned char *pred, *cur;
int lx;
short *blk;
{
  int i, j;

  for (j=0; j<8; j++)
  {
    for (i=0; i<8; i++)
      cur[i] = MPEclp[blk[i] + pred[i]];
    blk+= 8;
    cur+= lx;
    pred+= lx;
  }
}

/* subtract prediction from block data */
static void sub_pred(pred,cur,lx,blk)
unsigned char *pred, *cur;
int lx;
short *blk;
{
  int i, j;

  for (j=0; j<8; j++)
  {
    for (i=0; i<8; i++)
      blk[i] = cur[i] - pred[i];
    blk+= 8;
    cur+= lx;
    pred+= lx;
  }
}

/*
 * select between frame and field DCT
 *
 * preliminary version: based on inter-field correlation
 */
void dct_type_estimation(pred,cur,mbi)
unsigned char *pred,*cur;
struct MPEmbinfo *mbi;
{
  short blk0[128], blk1[128];
  int i, j, i0, j0, k, offs, s0, s1, sq0, sq1, s01;
  double d, r;

  k = 0;

  for (j0=0; j0<MPEheight2; j0+=16)
    for (i0=0; i0<MPEwidth; i0+=16)
    {
      if (MPEframe_pred_dct || MPEpict_struct!=FRAME_PICTURE)
        mbi[k].dct_type = 0;
      else
      {
        /* interlaced frame picture */
        /*
         * calculate prediction error (cur-pred) for top (blk0)
         * and bottom field (blk1)
         */
        for (j=0; j<8; j++)
        {
          offs = MPEwidth*((j<<1)+j0) + i0;
          for (i=0; i<16; i++)
          {
            blk0[16*j+i] = cur[offs] - pred[offs];
            blk1[16*j+i] = cur[offs+MPEwidth] - pred[offs+MPEwidth];
            offs++;
          }
        }
        /* correlate fields */
        s0=s1=sq0=sq1=s01=0;

        for (i=0; i<128; i++)
        {
          s0+= blk0[i];
          sq0+= blk0[i]*blk0[i];
          s1+= blk1[i];
          sq1+= blk1[i]*blk1[i];
          s01+= blk0[i]*blk1[i];
        }

        d = (sq0-(s0*s0)/128.0)*(sq1-(s1*s1)/128.0);

        if (d>0.0)
        {
          r = (s01-(s0*s1)/128.0)/sqrt(d);
          if (r>0.5)
            mbi[k].dct_type = 0; /* frame DCT */
          else
            mbi[k].dct_type = 1; /* field DCT */
        }
        else
          mbi[k].dct_type = 1; /* field DCT */
      }
      k++;
    }
}
