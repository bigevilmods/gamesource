// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  S3TC - S3TC texture block compression             */
/*                                                                  */
/*                 by Frank Barchard and Kaz Okuda, EAC             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 980529 KO based on source from S3                        */
/*    1.01 980601 KO decode based on Frank B's gimex decoder.       */
/*    1.02 980602 KO decode ignores compressed size if NULL.        */
/*    1.03 980628 FB decode dont TERP alpha                         */
/*    1.04 000225 FB 555 and options optional                       */
/*    1.05 000615 FB S3TC3 decode                                    */
/*    1.06 001020 FB mipmap field for shapes                        */
/*    1.07 010209 ID S3TC5 decode and S3TC2/4 unattenuate flag        */
/*    1.08 010313 ID added 3/8 5/8 interpolation for GameCube       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: s3read.c s3write.c texodex.h                              */
/*                                                                  */
/* Assumes data contains 4 by X image so each row of a block is     */
/* contiguous.                                                      */
/*                                                                  */
/* 8 byte header prepended to data so must be removed if you        */
/* want to store just the raw data. Decoder expects the header.     */
/*                                                                  */
/* typedef struct                                                   */
/* {                                                                */
/*     char code[2];     // S3TC1=0x18fb, S3TC3=0x1afb, S3TC5=0x1cfb   */
/*     char size[3];                                                */
/*     char mode[1];     // upper 4 bits are mipmap field in FSH    */
/*     char reserved[2]; // width in FSH                            */
/*                                                                  */
/* } S3TCHEADER;                                                     */
/*                                                                  */
/* Header is 2 bytes "0x18fb" followed by 3 byte uncompressed size  */
/* and 1 byte mode (Last byte reserved.  Mode is either...          */
/*  0  for no alpha,                                                */
/*  1  for explicit alpha,                                          */
/*  2  for interpolated alpha.                                      */
/*  4  for bgr555 little endian                                     */
/*  8  for rgb attenuate, ex. s3tc2 or s3tc4.                         */
/*  RGB 565 big endian is assumed.                                  */
/*  See texcodex.h for more info                                    */
/*                                                                  */
/*  NOTE: for standard S3TC 1/3 2/3 decode define NOTTHREEEIGHTS     */
/* On NVidia (only)                                                 */
/* s3tc1 decodes to 1555                                             */
/* s3tc3 decodes to 8888                                             */
/*                                                                  */
/* Very fast to decode: 80 MB/s vs 50 MB/s for refpack.             */
/* VERY slow to encode: Slower than MPeg, ~3 seconds/texture        */
/*                                                                  */
/*                                                                  */
/*  colour depth information:                                       */
/*        S3TC1: 1565                                                */
/*        S3TC2: 4565                                                */
/*        S3TC3: 4565                                                */
/*        S3TC4: 3565                                                */
/*        S3TC5: 3565                                                */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* S3TC1 files are 4x4 tiles with 2 colours and shades of 1/3 and    */
/* 2/3 between.                                                     */
/* S3TC3 also has a 4x4 tile of 4 bit alpha.                         */
/* S3TC1 format:                                                     */
/*     Two sixteen bit colour values followed by a 4X4 array of two */
/*     bit colour index values, these values correspond to a 4      */
/*     colour palette generated by interpolation of the             */
/*     aforementioned colour values. These values are 565. S3TC1     */
/*     supports an implicite alpha value when colour entry 0 <      */
/*     colour entry1. If this is the case then the colour palette   */
/*     is not 4 entries but rather the first three are colour values*/
/*     and the last value is transparent. ie                        */
/*       if(c0>c1)                                                  */
/*       {                                                          */
/*          c2 = (2*c0 + c1  )/3;                                   */
/*          c3 = (c0   + 2*c1)/3;                                   */
/*       }                                                          */
/*       else                                                       */
/*       {                                                          */
/*          c2 = (c0 + c1)/2; c3 = transparent;                     */
/*       }                                                          */
/*      For all intents and purposes this is a 1565 format.         */
/* S3TC2: same as S3TC3 with added preattenuated colour values.       */
/* S3TC3:                                                            */
/*     the colour information is the same as S3TC1 except now        */
/*     there is an explicit 8 byte alpha table with 4bits per alpha */
/*     value. this makes a total of 128 bits for alpha table and    */
/*     colour data, a 4X compression ratio.                         */
/* S3TC4: same as S3TC5 with added preattenuated colour values.       */
/* S3TC5:                                                            */
/*     S3TC5 again uses the same colour table as S3TC1 but encodes    */
/*     the alpha information in a similar manner in a 64 bit block  */
/*     before the colour information. The first two bytes are alpha */
/*     values. 8 alpha values must be generated in one of two       */
/*     methods. If a0 > a1 the then we generate the 8 alpha values  */
/*     directly by interpolation, otherwise we generate 6 via       */
/*     interpolation entries 6 and 7 in the alpha table are         */
/*     transparent and opaque respectively and entry 8 is the       */
/*     second colour value. Following this colour table is a        */
/*     4X4 array of 3 bit entries into the alpha table that         */
/*     specifies the alpha information for the 16 pixel block in    */
/*     question.                                                    */
/*                                                                  */
/* Reference to nvS3TC                                               */
/* http://www.mnwright.freeserve.co.uk/examples/meatball.htm        */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "s3tccodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL S3TC_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID('S','3','T','C');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 0;      /* supports 32 bit size field */
        info->islossy         = 1;      /* lossy compression module */
        strcpy(info->versionstr,    "1.07");     /* version # */
        strcpy(info->shorttypestr,  "s3tc");      /* type */
        strcpy(info->longtypestr,   "S3TC texture"); /* longtype */
    }
    return(info);
}

