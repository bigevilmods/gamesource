// (c) 1996-2007 Electronic Arts Inc.
/* getbits.c, bit level routines                                            */

/*
 * All modifications (mpeg2decode -> mpeg2play) are
 */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

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

#include "mpeg2dec.h"

/* initialize buffer, call once before first getbits or showbits */

void Initialize_Buffer(unsigned char *bitstream)
{
  MPDld->Incnt = 0;
  MPDld->Rdptr = bitstream;

#ifdef VERIFY
  /*  only the verifier uses this particular bit counter 
   *  Bitcnt keeps track of the current parser position with respect
   *  to the video elementary stream being decoded, regardless 
   *  of whether or not it is wrapped within a systems layer stream 
   */
  MPDld->Bitcnt = 0;
#endif

  MPDld->Bfr = 0;
  Flush_Buffer(0); /* fills valid data into bfr */
}


/* return next n bits (right adjusted) without advancing */

unsigned int Show_Bits(N)
int N;
{
  return MPDld->Bfr >> (32-N);
}


/* return next bit (could be made faster than Get_Bits(1)) */

unsigned int Get_Bits1()
{
  return Get_Bits(1);
}


/* advance by n bits */

void Flush_Buffer(N)
int N;
{
  int Incnt;

  MPDld->Bfr <<= N;

  Incnt = MPDld->Incnt - N;

  while (Incnt <= 24)
  {
    MPDld->Bfr |= *MPDld->Rdptr++ << (24 - Incnt);
    Incnt += 8;
  }

  MPDld->Incnt = Incnt;

#ifdef VERIFY 
  MPDld->Bitcnt += N;
#endif /* VERIFY */
}


/* return next n bits (right adjusted) */

unsigned int Get_Bits(N)
int N;
{
  unsigned int Val;

  Val = Show_Bits(N);
  Flush_Buffer(N);

  return Val;
}


void Flush_Buffer32()
{
  int Incnt;

  MPDld->Bfr = 0;

  Incnt = MPDld->Incnt - 32;

  while (Incnt <= 24)
  {
    MPDld->Bfr |= *MPDld->Rdptr++ << (24 - Incnt);
    Incnt += 8;
  }

  MPDld->Incnt = Incnt;

#ifdef VERIFY 
  MPDld->Bitcnt += 32;
#endif /* VERIFY */
}


unsigned int Get_Bits32()
{
  unsigned int l;

  l = Show_Bits(32);
  Flush_Buffer32();

  return l;
}
