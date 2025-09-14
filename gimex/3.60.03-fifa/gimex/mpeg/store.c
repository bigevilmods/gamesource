// (c) 1996-2007 Electronic Arts Inc.
/* store.c, picture output routines                                         */


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
#include "mpeg2dec.h"


/* private prototypes */
static void conv422to444 _ANSI_ARGS_((unsigned char *src, unsigned char *dst));
static void conv420to422 _ANSI_ARGS_((unsigned char *src, unsigned char *dst));


/* private variables */
static unsigned char *u422, *v422, *u444, *v444;


void MPD_renderinit(void)
{
  if (MPDchroma_format!=CHROMA444)
  {
    if (!(u422 = (unsigned char *) galloc((MPDcoded_picture_width>>1)*MPDcoded_picture_height)))
      Error("mpgread: galloc failed");

    if (!(v422 = (unsigned char *) galloc((MPDcoded_picture_width>>1)*MPDcoded_picture_height)))
      Error("mpgread: galloc failed");

    if (!(u444 = (unsigned char *) galloc(MPDcoded_picture_width*MPDcoded_picture_height)))
      Error("mpgread: galloc failed");

    if (!(v444 = (unsigned char *) galloc(MPDcoded_picture_width*MPDcoded_picture_height)))
      Error("mpgread: galloc failed");
  }
}

void MPD_render(unsigned char *src[], char *dest, int rowbytes)
{
  int i, j;
  int y, u, v;
  int crv, cbu, cgu, cgv;
  unsigned char *py, *pu, *pv;
  ARGB *outptr;

  if (MPDchroma_format==CHROMA444)
  {
    u444 = src[1];
    v444 = src[2];
  }
  else
  {
    if (MPDchroma_format==CHROMA420)
    {
      conv420to422(src[1],u422);
      conv420to422(src[2],v422);
      conv422to444(u422,u444);
      conv422to444(v422,v444);
    }
    else
    {
      conv422to444(src[1],u444);
      conv422to444(src[2],v444);
    }
  }

  /* matrix coefficients */
  crv = MPDinverse_table_6_9[MPDmatrix_coefficients][0];
  cbu = MPDinverse_table_6_9[MPDmatrix_coefficients][1];
  cgu = MPDinverse_table_6_9[MPDmatrix_coefficients][2];
  cgv = MPDinverse_table_6_9[MPDmatrix_coefficients][3];
  
  for (i=0; i<MPDvertical_size; i++)
  {
    py = src[0] + MPDcoded_picture_width*i;
    pu = u444   + MPDcoded_picture_width*i;
    pv = v444   + MPDcoded_picture_width*i;

    outptr = (ARGB*) dest;

    for (j=0; j<MPDhorizontal_size; j++)
    {
      u = *pu++ - 128;
      v = *pv++ - 128;
      y = 76309 * (*py++ - 16); /* (255/219)*65536 */

      outptr->a = 0xff;
      outptr->r = MPDclip[(y + crv*v + 32768)>>16];
      outptr->g = MPDclip[(y - cgu*u - cgv*v + 32768)>>16];
      outptr->b = MPDclip[(y + cbu*u + 32786)>>16];
      ++outptr;
    }

    dest += rowbytes;
  }
}

void MPD_renderrestore(void)
{
  if (MPDchroma_format!=CHROMA444)
  {
    gfree(u422);
    gfree(v422);
    gfree(u444);
    gfree(v444);
  }
}

/* horizontal 1:2 interpolation filter */
static void conv422to444(src,dst)
unsigned char *src,*dst;
{
  int i, i2, w, j, im3, im2, im1, ip1, ip2, ip3;

  w = MPDcoded_picture_width>>1;

  if (MPDbase.MPEG2_Flag)
  {
    for (j=0; j<MPDcoded_picture_height; j++)
    {
      for (i=0; i<w; i++)
      {
        i2 = i<<1;
        im2 = (i<2) ? 0 : i-2;
        im1 = (i<1) ? 0 : i-1;
        ip1 = (i<w-1) ? i+1 : w-1;
        ip2 = (i<w-2) ? i+2 : w-1;
        ip3 = (i<w-3) ? i+3 : w-1;

        /* FIR filter coefficients (*256): 21 0 -52 0 159 256 159 0 -52 0 21 */
        /* even samples (0 0 256 0 0) */
        dst[i2] = src[i];

        /* odd samples (21 -52 159 159 -52 21) */
        dst[i2+1] = MPDclip[(int)(21*(src[im2]+src[ip3])
                                 -52*(src[im1]+src[ip2]) 
                                +159*(src[i]+src[ip1])+128)>>8];
      }
      src+= w;
      dst+= MPDcoded_picture_width;
    }
  }
  else
  {
    for (j=0; j<MPDcoded_picture_height; j++)
    {
      for (i=0; i<w; i++)
      {

        i2 = i<<1;
        im3 = (i<3) ? 0 : i-3;
        im2 = (i<2) ? 0 : i-2;
        im1 = (i<1) ? 0 : i-1;
        ip1 = (i<w-1) ? i+1 : w-1;
        ip2 = (i<w-2) ? i+2 : w-1;
        ip3 = (i<w-3) ? i+3 : w-1;

        /* FIR filter coefficients (*256): 5 -21 70 228 -37 11 */
        dst[i2] = MPDclip[(int)(5*src[im3]
                              -21*src[im2]
                              +70*src[im1]
                             +228*src[i]
                              -37*src[ip1]
                              +11*src[ip2]+128)>>8];

       dst[i2+1] = MPDclip[(int)(5*src[ip3]
                               -21*src[ip2]
                               +70*src[ip1]
                              +228*src[i]
                               -37*src[im1]
                               +11*src[im2]+128)>>8];
      }
      src+= w;
      dst+= MPDcoded_picture_width;
    }
  }
}

/* vertical 1:2 interpolation filter */
static void conv420to422(src,dst)
unsigned char *src,*dst;
{
  int w, h, i, j, j2;
  int jm6, jm5, jm4, jm3, jm2, jm1, jp1, jp2, jp3, jp4, jp5, jp6, jp7;

  w = MPDcoded_picture_width>>1;
  h = MPDcoded_picture_height>>1;

  if (MPDprogressive_frame)
  {
    /* intra frame */
    for (i=0; i<w; i++)
    {
      for (j=0; j<h; j++)
      {
        j2 = j<<1;
        jm3 = (j<3) ? 0 : j-3;
        jm2 = (j<2) ? 0 : j-2;
        jm1 = (j<1) ? 0 : j-1;
        jp1 = (j<h-1) ? j+1 : h-1;
        jp2 = (j<h-2) ? j+2 : h-1;
        jp3 = (j<h-3) ? j+3 : h-1;

        /* FIR filter coefficients (*256): 5 -21 70 228 -37 11 */
        /* New FIR filter coefficients (*256): 3 -16 67 227 -32 7 */
        dst[w*j2] = MPDclip[(int)(3*src[w*jm3]
                                -16*src[w*jm2]
                                +67*src[w*jm1]
                               +227*src[w*j]
                                -32*src[w*jp1]
                                 +7*src[w*jp2]+128)>>8];

        dst[w*(j2+1)] = MPDclip[(int)(3*src[w*jp3]
                                    -16*src[w*jp2]
                                    +67*src[w*jp1]
                                   +227*src[w*j]
                                    -32*src[w*jm1]
                                     +7*src[w*jm2]+128)>>8];
      }
      src++;
      dst++;
    }
  }
  else
  {
    /* intra field */
    for (i=0; i<w; i++)
    {
      for (j=0; j<h; j+=2)
      {
        j2 = j<<1;

        /* top field */
        jm6 = (j<6) ? 0 : j-6;
        jm4 = (j<4) ? 0 : j-4;
        jm2 = (j<2) ? 0 : j-2;
        jp2 = (j<h-2) ? j+2 : h-2;
        jp4 = (j<h-4) ? j+4 : h-2;
        jp6 = (j<h-6) ? j+6 : h-2;

        /* Polyphase FIR filter coefficients (*256): 2 -10 35 242 -18 5 */
        /* New polyphase FIR filter coefficients (*256): 1 -7 30 248 -21 5 */
        dst[w*j2] = MPDclip[(int)(1*src[w*jm6]
                                 -7*src[w*jm4]
                                +30*src[w*jm2]
                               +248*src[w*j]
                                -21*src[w*jp2]
                                 +5*src[w*jp4]+128)>>8];

        /* Polyphase FIR filter coefficients (*256): 11 -38 192 113 -30 8 */
        /* New polyphase FIR filter coefficients (*256):7 -35 194 110 -24 4 */
        dst[w*(j2+2)] = MPDclip[(int)(7*src[w*jm4]
                                    -35*src[w*jm2]
                                   +194*src[w*j]
                                   +110*src[w*jp2]
                                    -24*src[w*jp4]
                                     +4*src[w*jp6]+128)>>8];

        /* bottom field */
        jm5 = (j<5) ? 1 : j-5;
        jm3 = (j<3) ? 1 : j-3;
        jm1 = (j<1) ? 1 : j-1;
        jp1 = (j<h-1) ? j+1 : h-1;
        jp3 = (j<h-3) ? j+3 : h-1;
        jp5 = (j<h-5) ? j+5 : h-1;
        jp7 = (j<h-7) ? j+7 : h-1;

        /* Polyphase FIR filter coefficients (*256): 11 -38 192 113 -30 8 */
        /* New polyphase FIR filter coefficients (*256):7 -35 194 110 -24 4 */
        dst[w*(j2+1)] = MPDclip[(int)(7*src[w*jp5]
                                    -35*src[w*jp3]
                                   +194*src[w*jp1]
                                   +110*src[w*jm1]
                                    -24*src[w*jm3]
                                     +4*src[w*jm5]+128)>>8];

        dst[w*(j2+3)] = MPDclip[(int)(1*src[w*jp7]
                                     -7*src[w*jp5]
                                    +30*src[w*jp3]
                                   +248*src[w*jp1]
                                    -21*src[w*jm1]
                                     +5*src[w*jm3]+128)>>8];
      }
      src++;
      dst++;
    }
  }
}
