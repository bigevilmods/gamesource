// (c) 1996-2007 Electronic Arts Inc.
/* ratectl.c, bitrate control routines (linear quantization only currently) */


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
static void calc_actj _ANSI_ARGS_((unsigned char *frame));
static double var_sblk _ANSI_ARGS_((unsigned char *p, int lx));

/* rate control variables */
static int Xi, Xp, Xb, r, d0i, d0p, d0b;
static double avg_act;
static int R, T, d;
static double actsum;
static int Np, Nb, S, Q;
static int prev_mquant;

void rc_init_seq()
{
  /* reaction parameter (constant) */
  r = (int)floor(2.0*MPEbit_rate/MPEframe_rate + 0.5);

  /* average activity */
  avg_act = 400.0;

  /* remaining # of bits in GOP */
  R = 0;

  /* global complexity measure */
  Xi = (int)floor(160.0*MPEbit_rate/115.0 + 0.5);
  Xp = (int)floor( 60.0*MPEbit_rate/115.0 + 0.5);
  Xb = (int)floor( 42.0*MPEbit_rate/115.0 + 0.5);

  /* virtual buffer fullness */
  d0i = (int)floor(10.0*r/31.0 + 0.5);
  d0p = (int)floor(10.0*r/31.0 + 0.5);
  d0b = (int)floor(1.4*10.0*r/31.0 + 0.5);
/*
  d0i = (int)floor(10.0*r/(qscale_tab[0] ? 56.0 : 31.0) + 0.5);
  d0p = (int)floor(10.0*r/(qscale_tab[1] ? 56.0 : 31.0) + 0.5);
  d0b = (int)floor(1.4*10.0*r/(qscale_tab[2] ? 56.0 : 31.0) + 0.5);
*/

  STATPRINT(MPEstatfile,"\nrate control: sequence initialization\n");
  STATPRINT(MPEstatfile,
    " initial global complexity measures (I,P,B): Xi=%d, Xp=%d, Xb=%d\n",
    Xi, Xp, Xb);
  STATPRINT(MPEstatfile," reaction parameter: r=%d\n", r);
  STATPRINT(MPEstatfile,
    " initial virtual buffer fullness (I,P,B): d0i=%d, d0p=%d, d0b=%d\n",
    d0i, d0p, d0b);
  STATPRINT(MPEstatfile," initial average activity: avg_act=%.1f\n", avg_act);
}

void rc_init_GOP(np,nb)
int np,nb;
{
  /* PRATTO: don't correct for bitrate error in previous GOP */
  /* R += (int) floor((1 + np + nb) * bit_rate / frame_rate + 0.5); */
  R = (int) floor((1 + np + nb) * MPEbit_rate / MPEframe_rate + 0.5);
  Np = MPEfieldpic ? 2*np+1 : np;
  Nb = MPEfieldpic ? 2*nb : nb;

  STATPRINT(MPEstatfile,"\nrate control: new group of pictures (GOP)\n");
  STATPRINT(MPEstatfile," target number of bits for GOP: R=%d\n",R);
  STATPRINT(MPEstatfile," number of P pictures in GOP: Np=%d\n",Np);
  STATPRINT(MPEstatfile," number of B pictures in GOP: Nb=%d\n",Nb);
}

/* Note: we need to substitute K for the 1.4 and 1.0 constants -- this can
   be modified to fit image content */

/* Step 1: compute target bits for current picture being coded */
void rc_init_pict(frame)
unsigned char *frame;
{
  int Tmin;

  switch (MPEpict_type)
  {
  case I_TYPE:
    T = (int) floor(R/(1.0+Np*Xp/(Xi*1.0)+Nb*Xb/(Xi*1.4)) + 0.5);
    d = d0i;
    break;
  case P_TYPE:
    T = (int) floor(R/(Np+Nb*1.0*Xb/(1.4*Xp)) + 0.5);
    d = d0p;
    break;
  case B_TYPE:
    T = (int) floor(R/(Nb+Np*1.4*Xp/(1.0*Xb)) + 0.5);
    d = d0b;
    break;
  }

  Tmin = (int) floor(MPEbit_rate/(8.0*MPEframe_rate) + 0.5);

  if (T<Tmin)
    T = Tmin;

  S = bitcount();
  Q = 0;

  calc_actj(frame);
  actsum = 0.0;

  STATPRINT(MPEstatfile,"\nrate control: start of picture\n");
  STATPRINT(MPEstatfile," target number of bits: T=%d\n",T);
}

static void calc_actj(frame)
unsigned char *frame;
{
  int i,j,k;
  unsigned char *p;
  double actj,var;

  k = 0;

  for (j=0; j<MPEheight2; j+=16)
    for (i=0; i<MPEwidth; i+=16)
    {
      p = frame + ((MPEpict_struct==BOTTOM_FIELD)?MPEwidth:0) + i + MPEwidth2*j;

      /* take minimum spatial activity measure of luminance blocks */

      actj = var_sblk(p,MPEwidth2);
      var = var_sblk(p+8,MPEwidth2);
      if (var<actj) actj = var;
      var = var_sblk(p+8*MPEwidth2,MPEwidth2);
      if (var<actj) actj = var;
      var = var_sblk(p+8*MPEwidth2+8,MPEwidth2);
      if (var<actj) actj = var;

      if (!MPEfieldpic && !MPEprog_seq)
      {
        /* field */
        var = var_sblk(p,MPEwidth<<1);
        if (var<actj) actj = var;
        var = var_sblk(p+8,MPEwidth<<1);
        if (var<actj) actj = var;
        var = var_sblk(p+MPEwidth,MPEwidth<<1);
        if (var<actj) actj = var;
        var = var_sblk(p+MPEwidth+8,MPEwidth<<1);
        if (var<actj) actj = var;
      }

      actj+= 1.0;

      MPEmbinfo[k++].act = actj;
    }
}

void rc_update_pict()
{
  int X;

  S = bitcount() - S; /* total # of bits in picture */
  R-= S; /* remaining # of bits in GOP */
  X = (int) floor(S*((0.5*(double)Q)/(MPEmb_width*MPEmb_height2)) + 0.5);
  d+= S - T;
  avg_act = actsum/(MPEmb_width*MPEmb_height2);

  /* PRATTO: don't allow virtual buffer underflow */
  if (d < 0) d = 0;

  switch (MPEpict_type)
  {
  case I_TYPE:
    Xi = X;
    d0i = d;
    break;
  case P_TYPE:
    Xp = X;
    d0p = d;
    Np--;
    break;
  case B_TYPE:
    Xb = X;
    d0b = d;
    Nb--;
    break;
  }

  STATPRINT(MPEstatfile,"\nrate control: end of picture\n");
  STATPRINT(MPEstatfile," actual number of bits: S=%d\n",S);
  STATPRINT(MPEstatfile," average quantization parameter Q=%.1f\n",
    (double)Q/(MPEmb_width*MPEmb_height2));
  STATPRINT(MPEstatfile," remaining number of bits in GOP: R=%d\n",R);
  STATPRINT(MPEstatfile,
    " global complexity measures (I,P,B): Xi=%d, Xp=%d, Xb=%d\n",
    Xi, Xp, Xb);
  STATPRINT(MPEstatfile,
    " virtual buffer fullness (I,P,B): d0i=%d, d0p=%d, d0b=%d\n",
    d0i, d0p, d0b);
  STATPRINT(MPEstatfile," remaining number of P pictures in GOP: Np=%d\n",Np);
  STATPRINT(MPEstatfile," remaining number of B pictures in GOP: Nb=%d\n",Nb);
  STATPRINT(MPEstatfile," average activity: avg_act=%.1f\n", avg_act);
}

/* compute initial quantization stepsize (at the beginning of picture) */
int rc_start_mb()
{
  int mquant;

  if (MPEq_scale_type)
  {
    mquant = (int) floor(2.0*d*31.0/r + 0.5);

    /* clip mquant to legal (linear) range */
    if (mquant<1)
      mquant = 1;
    if (mquant>112)
      mquant = 112;

    /* map to legal quantization level */
    mquant = MPEnon_linear_mquant_table[MPEmap_non_linear_mquant[mquant]];
  }
  else
  {
    mquant = (int) floor(d*31.0/r + 0.5);
    mquant <<= 1;

    /* clip mquant to legal (linear) range */
    if (mquant<2)
      mquant = 2;
    if (mquant>62)
      mquant = 62;

    prev_mquant = mquant;
  }

/*
  STATPRINT(MPEstatfile,"rc_start_mb:\n");
  STATPRINT(MPEstatfile,"mquant=%d\n",mquant);
*/

  return mquant;
}

/* Step 2: measure virtual buffer - estimated buffer discrepancy */
int rc_calc_mquant(j)
int j;
{
  int mquant;
  double dj, Qj, actj, N_actj;

  /* measure virtual buffer discrepancy from uniform distribution model */
  dj = d + (bitcount()-S) - j*(T/(MPEmb_width*MPEmb_height2));

  /* scale against dynamic range of mquant and the bits/picture count */
  Qj = dj*31.0/r;
/*Qj = dj*(q_scale_type ? 56.0 : 31.0)/r;  */

  actj = MPEmbinfo[j].act;
  actsum+= actj;

  /* compute normalized activity */
  N_actj = (2.0*actj+avg_act)/(actj+2.0*avg_act);

  if (MPEq_scale_type)
  {
    /* modulate mquant with combined buffer and local activity measures */
    mquant = (int) floor(2.0*Qj*N_actj + 0.5);

    /* clip mquant to legal (linear) range */
    if (mquant<1)
      mquant = 1;
    if (mquant>112)
      mquant = 112;

    /* map to legal quantization level */
    mquant = MPEnon_linear_mquant_table[MPEmap_non_linear_mquant[mquant]];
  }
  else
  {
    /* modulate mquant with combined buffer and local activity measures */
    mquant = (int) floor(Qj*N_actj + 0.5);
    mquant <<= 1;

    /* clip mquant to legal (linear) range */
    if (mquant<2)
      mquant = 2;
    if (mquant>62)
      mquant = 62;

    /* ignore small changes in mquant */
    if (mquant>=8 && (mquant-prev_mquant)>=-4 && (mquant-prev_mquant)<=4)
      mquant = prev_mquant;

    prev_mquant = mquant;
  }

  Q+= mquant; /* for calculation of average mquant */

/*
  STATPRINT(MPEstatfile,"rc_calc_mquant(%d): ",j);
  STATPRINT(MPEstatfile,"bitcount=%d, dj=%f, Qj=%f, actj=%f, N_actj=%f, mquant=%d\n",
    bitcount(),dj,Qj,actj,N_actj,mquant);
*/

  return mquant;
}

/* compute variance of 8x8 block */
static double var_sblk(p,lx)
unsigned char *p;
int lx;
{
  int i, j;
  unsigned int v, s, s2;

  s = s2 = 0;

  for (j=0; j<8; j++)
  {
    for (i=0; i<8; i++)
    {
      v = *p++;
      s+= v;
      s2+= v*v;
    }
    p+= lx - 8;
  }

  return s2/64.0 - (s/64.0)*(s/64.0);
}

/* VBV calculations
 *
 * generates warnings if underflow or overflow occurs
 */

/* vbv_end_of_picture
 *
 * - has to be called directly after writing picture_data()
 * - needed for accurate VBV buffer overflow calculation
 * - assumes there is no byte stuffing prior to the next start code
 */

static int bitcnt_EOP;

void vbv_end_of_picture()
{
  bitcnt_EOP = bitcount();
  bitcnt_EOP = (bitcnt_EOP + 7) & ~7; /* account for bit stuffing */
}

/* calc_vbv_delay
 *
 * has to be called directly after writing the picture start code, the
 * reference point for vbv_delay
 */

void calc_vbv_delay()
{
  double picture_delay;
  static double next_ip_delay; /* due to frame reordering delay */
  static double decoding_time;

  /* number of 1/90000 s ticks until next picture is to be decoded */
  if (MPEpict_type == B_TYPE)
  {
    if (MPEprog_seq)
    {
      if (!MPErepeatfirst)
        picture_delay = 90000.0/MPEframe_rate; /* 1 frame */
      else
      {
        if (!MPEtopfirst)
          picture_delay = 90000.0*2.0/MPEframe_rate; /* 2 frames */
        else
          picture_delay = 90000.0*3.0/MPEframe_rate; /* 3 frames */
      }
    }
    else
    {
      /* interlaced */
      if (MPEfieldpic)
        picture_delay = 90000.0/(2.0*MPEframe_rate); /* 1 field */
      else
      {
        if (!MPErepeatfirst)
          picture_delay = 90000.0*2.0/(2.0*MPEframe_rate); /* 2 flds */
        else
          picture_delay = 90000.0*3.0/(2.0*MPEframe_rate); /* 3 flds */
      }
    }
  }
  else
  {
    /* I or P picture */
    if (MPEfieldpic)
    {
      if(MPEtopfirst==(MPEpict_struct==TOP_FIELD))
      {
        /* first field */
        picture_delay = 90000.0/(2.0*MPEframe_rate);
      }
      else
      {
        /* second field */
        /* take frame reordering delay into account */
        picture_delay = next_ip_delay - 90000.0/(2.0*MPEframe_rate);
      }
    }
    else
    {
      /* frame picture */
      /* take frame reordering delay into account*/
      picture_delay = next_ip_delay;
    }

    if (!MPEfieldpic || MPEtopfirst!=(MPEpict_struct==TOP_FIELD))
    {
      /* frame picture or second field */
      if (MPEprog_seq)
      {
        if (!MPErepeatfirst)
          next_ip_delay = 90000.0/MPEframe_rate;
        else
        {
          if (!MPEtopfirst)
            next_ip_delay = 90000.0*2.0/MPEframe_rate;
          else
            next_ip_delay = 90000.0*3.0/MPEframe_rate;
        }
      }
      else
      {
        if (MPEfieldpic)
          next_ip_delay = 90000.0/(2.0*MPEframe_rate);
        else
        {
          if (!MPErepeatfirst)
            next_ip_delay = 90000.0*2.0/(2.0*MPEframe_rate);
          else
            next_ip_delay = 90000.0*3.0/(2.0*MPEframe_rate);
        }
      }
    }
  }

  if (decoding_time==0.0)
  {
    /* first call of calc_vbv_delay */
    /* we start with a 7/8 filled VBV buffer (12.5% back-off) */
    picture_delay = ((MPEvbv_buffer_size*16384*7)/8)*90000.0/MPEbit_rate;
    if (MPEfieldpic)
      next_ip_delay = (int)(90000.0/MPEframe_rate+0.5);
  }

  /* VBV checks */

  /* check for underflow (previous picture) */
  if (!MPElow_delay && (decoding_time < bitcnt_EOP*90000.0/MPEbit_rate))
  {
    /* picture not completely in buffer at intended decoding time */
    if (!MPEquiet)
      fprintf(stderr,"vbv_delay underflow! (decoding_time=%.1f, t_EOP=%.1f\n)",
        decoding_time, bitcnt_EOP*90000.0/MPEbit_rate);
  }

  /* when to decode current frame */
  decoding_time += picture_delay;

  /* warning: bitcount() may overflow (e.g. after 9 min. at 8 Mbit/s */
  MPEvbv_delay = (int)(decoding_time - bitcount()*90000.0/MPEbit_rate);

  /* check for overflow (current picture) */
  if ((decoding_time - bitcnt_EOP*90000.0/MPEbit_rate)
      > (MPEvbv_buffer_size*16384)*90000.0/MPEbit_rate)
  {
    if (!MPEquiet)
      fprintf(stderr,"vbv_delay overflow!\n");
  }

  STATPRINT(MPEstatfile,
    "\nvbv_delay=%d (bitcount=%d, decoding_time=%.2f, bitcnt_EOP=%d)\n",
    MPEvbv_delay,bitcount(),decoding_time,bitcnt_EOP);

  if (MPEvbv_delay<0)
  {
    if (!MPEquiet)
      fprintf(stderr,"vbv_delay underflow: %d\n",MPEvbv_delay);
    MPEvbv_delay = 0;
  }

  if (MPEvbv_delay>65535)
  {
    if (!MPEquiet)
      fprintf(stderr,"vbv_delay overflow: %d\n",MPEvbv_delay);
    MPEvbv_delay = 65535;
  }
}
