// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gimex.h"

#ifdef MDECCHUNK

#define wopenstr wopenmdc
#define writestr writemdc
#define wclosestr wclosemdc

#endif

#include "strgimex.h"

#include "codex.h"
#include "refcodex.h"
#include "huffcodex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#ifdef PI
#undef PI
#endif
#define PI 3.14159265358979323846

void init_fdct(STRINFO *mdec)
{
    int i, j;
    double s;

    for (i=0; i<8; i++)
    {
        s = (i==0) ? sqrt(0.125) : 0.5;

        for (j=0; j<8; j++)
            mdec->c[j][i] = s * cos((PI/8.0)*i*(j+0.5));
    }
}

static void fdct(STRINFO *mdec,STR_LONG *datain,STR_LONG *dataout)
{
    int i, j;
    double s;
    double tmp[64];

    for (i=0; i<8; i++)
    {
        for (j=0; j<8; j++)
        {
            s = mdec->c[0][j] * datain[8*i+0]
              + mdec->c[1][j] * datain[8*i+1]
              + mdec->c[2][j] * datain[8*i+2]
              + mdec->c[3][j] * datain[8*i+3]
              + mdec->c[4][j] * datain[8*i+4]
              + mdec->c[5][j] * datain[8*i+5]
              + mdec->c[6][j] * datain[8*i+6]
              + mdec->c[7][j] * datain[8*i+7];

            tmp[8*i+j] = s;
        }
    }

    for (j=0; j<8; j++)
    {
        for (i=0; i<8; i++)
        {
            s = mdec->c[0][i] * tmp[8*0+j]
              + mdec->c[1][i] * tmp[8*1+j]
              + mdec->c[2][i] * tmp[8*2+j]
              + mdec->c[3][i] * tmp[8*3+j]
              + mdec->c[4][i] * tmp[8*4+j]
              + mdec->c[5][i] * tmp[8*5+j]
              + mdec->c[6][i] * tmp[8*6+j]
              + mdec->c[7][i] * tmp[8*7+j];

            dataout[8*i+j] = (int)floor(s+0.5);
        }
    }
}

#if 0
#define YUV(red,green,blue,Y,Cb,Cr) \
    if ((red & green & blue) == 0xff) \
    {\
        *Y=127; *Cb=0; *Cr=0; \
    } else\
    {\
        *Y  = (STR_LONG) (((double) 0.29900 * red + (double) 0.58700 * green + (double) 0.11400 * blue) - (double) 128.0);\
        *Cb = (STR_LONG) ( (double)-0.16870 * red - (double) 0.33130 * green + (double) 0.50000 * blue);\
        *Cr = (STR_LONG) ( (double) 0.50000 * red - (double) 0.41870 * green - (double) 0.08130 * blue);\
    }
#endif

#define YUVA(red,green,blue,alpha,Y,Cb,Cr,A) \
    {\
        *Y  = (STR_LONG) (((double) 0.29900 * red + (double) 0.58700 * green  + (double) 0.11400 * blue)) - 128;\
        *Cb = (STR_LONG) ( (double)-0.16870 * red + (double)-0.33130 * green) +  (blue>>1);\
        *Cr =               (red>>1)   + (STR_LONG) ((double)-0.41870 * green  + (double)-0.08130 * blue);\
        *A  = alpha - 128;\
    }

/* Convert RGBA frame to Y Cr Cb A and perform DCT on blocks           */
/* Formula and constants need a little more tweaking to be "stable" */

static void RGB_YUV_DCT(STRINFO *mdec,STR_UWORD xres,STR_UWORD yres,ARGB *Frame,int rowbytes,STR_LONG *DCT_Output, int alpha)
{
    STR_LONG    Y1[64];
    STR_LONG    Y2[64];
    STR_LONG    Y3[64];
    STR_LONG    Y4[64];

    STR_LONG    Cb[64];
    STR_LONG    Cr[64];

    STR_LONG    Cb1[64];
    STR_LONG    Cr1[64];
    STR_LONG    Cb2[64];
    STR_LONG    Cr2[64];
    STR_LONG    Cb3[64];
    STR_LONG    Cr3[64];
    STR_LONG    Cb4[64];
    STR_LONG    Cr4[64];

    STR_LONG    A1[64];
    STR_LONG    A2[64];
    STR_LONG    A3[64];
    STR_LONG    A4[64];

    ARGB    *p;

    STR_UWORD row,column,pixely,loop,loop2;

    for (column=0;column<(xres>>4);++column)
    {
        for (row=0;row<(yres>>4);++row)
        {
            loop=0;
            for (pixely=0;pixely<8;++pixely)
            {
                p = (ARGB *)((char *) Frame + ((row*16)+pixely)*rowbytes) + column*16;
                YUVA(p[0].r,p[0].g,p[0].b,p[0].a,&Y1[loop+0],&Cb1[loop+0],&Cr1[loop+0],&A1[loop+0]);
                YUVA(p[1].r,p[1].g,p[1].b,p[1].a,&Y1[loop+1],&Cb1[loop+1],&Cr1[loop+1],&A1[loop+1]);
                YUVA(p[2].r,p[2].g,p[2].b,p[2].a,&Y1[loop+2],&Cb1[loop+2],&Cr1[loop+2],&A1[loop+2]);
                YUVA(p[3].r,p[3].g,p[3].b,p[3].a,&Y1[loop+3],&Cb1[loop+3],&Cr1[loop+3],&A1[loop+3]);
                YUVA(p[4].r,p[4].g,p[4].b,p[4].a,&Y1[loop+4],&Cb1[loop+4],&Cr1[loop+4],&A1[loop+4]);
                YUVA(p[5].r,p[5].g,p[5].b,p[5].a,&Y1[loop+5],&Cb1[loop+5],&Cr1[loop+5],&A1[loop+5]);
                YUVA(p[6].r,p[6].g,p[6].b,p[6].a,&Y1[loop+6],&Cb1[loop+6],&Cr1[loop+6],&A1[loop+6]);
                YUVA(p[7].r,p[7].g,p[7].b,p[7].a,&Y1[loop+7],&Cb1[loop+7],&Cr1[loop+7],&A1[loop+7]);
                loop += 8;
            }

            loop=0;
            for (pixely=0;pixely<8;++pixely)
            {
                p = (ARGB *)((char *) Frame + ((row*16)+pixely)*rowbytes) + column*16+8;
                YUVA(p[0].r,p[0].g,p[0].b,p[0].a,&Y2[loop+0],&Cb2[loop+0],&Cr2[loop+0],&A2[loop+0]);
                YUVA(p[1].r,p[1].g,p[1].b,p[1].a,&Y2[loop+1],&Cb2[loop+1],&Cr2[loop+1],&A2[loop+1]);
                YUVA(p[2].r,p[2].g,p[2].b,p[2].a,&Y2[loop+2],&Cb2[loop+2],&Cr2[loop+2],&A2[loop+2]);
                YUVA(p[3].r,p[3].g,p[3].b,p[3].a,&Y2[loop+3],&Cb2[loop+3],&Cr2[loop+3],&A2[loop+3]);
                YUVA(p[4].r,p[4].g,p[4].b,p[4].a,&Y2[loop+4],&Cb2[loop+4],&Cr2[loop+4],&A2[loop+4]);
                YUVA(p[5].r,p[5].g,p[5].b,p[5].a,&Y2[loop+5],&Cb2[loop+5],&Cr2[loop+5],&A2[loop+5]);
                YUVA(p[6].r,p[6].g,p[6].b,p[6].a,&Y2[loop+6],&Cb2[loop+6],&Cr2[loop+6],&A2[loop+6]);
                YUVA(p[7].r,p[7].g,p[7].b,p[7].a,&Y2[loop+7],&Cb2[loop+7],&Cr2[loop+7],&A2[loop+7]);
                loop += 8;
            }

            loop=0;
            for (pixely=8;pixely<16;++pixely)
            {
                p = (ARGB *)((char *) Frame + ((row*16)+pixely)*rowbytes) + column*16;
                YUVA(p[0].r,p[0].g,p[0].b,p[0].a,&Y3[loop+0],&Cb3[loop+0],&Cr3[loop+0],&A3[loop+0]);
                YUVA(p[1].r,p[1].g,p[1].b,p[1].a,&Y3[loop+1],&Cb3[loop+1],&Cr3[loop+1],&A3[loop+1]);
                YUVA(p[2].r,p[2].g,p[2].b,p[2].a,&Y3[loop+2],&Cb3[loop+2],&Cr3[loop+2],&A3[loop+2]);
                YUVA(p[3].r,p[3].g,p[3].b,p[3].a,&Y3[loop+3],&Cb3[loop+3],&Cr3[loop+3],&A3[loop+3]);
                YUVA(p[4].r,p[4].g,p[4].b,p[4].a,&Y3[loop+4],&Cb3[loop+4],&Cr3[loop+4],&A3[loop+4]);
                YUVA(p[5].r,p[5].g,p[5].b,p[5].a,&Y3[loop+5],&Cb3[loop+5],&Cr3[loop+5],&A3[loop+5]);
                YUVA(p[6].r,p[6].g,p[6].b,p[6].a,&Y3[loop+6],&Cb3[loop+6],&Cr3[loop+6],&A3[loop+6]);
                YUVA(p[7].r,p[7].g,p[7].b,p[7].a,&Y3[loop+7],&Cb3[loop+7],&Cr3[loop+7],&A3[loop+7]);
                loop += 8;
            }

            loop=0;
            for (pixely=8;pixely<16;++pixely)
            {
                p = (ARGB *)((char *) Frame + ((row*16)+pixely)*rowbytes) + column*16+8;
                YUVA(p[0].r,p[0].g,p[0].b,p[0].a,&Y4[loop+0],&Cb4[loop+0],&Cr4[loop+0],&A4[loop+0]);
                YUVA(p[1].r,p[1].g,p[1].b,p[1].a,&Y4[loop+1],&Cb4[loop+1],&Cr4[loop+1],&A4[loop+1]);
                YUVA(p[2].r,p[2].g,p[2].b,p[2].a,&Y4[loop+2],&Cb4[loop+2],&Cr4[loop+2],&A4[loop+2]);
                YUVA(p[3].r,p[3].g,p[3].b,p[3].a,&Y4[loop+3],&Cb4[loop+3],&Cr4[loop+3],&A4[loop+3]);
                YUVA(p[4].r,p[4].g,p[4].b,p[4].a,&Y4[loop+4],&Cb4[loop+4],&Cr4[loop+4],&A4[loop+4]);
                YUVA(p[5].r,p[5].g,p[5].b,p[5].a,&Y4[loop+5],&Cb4[loop+5],&Cr4[loop+5],&A4[loop+5]);
                YUVA(p[6].r,p[6].g,p[6].b,p[6].a,&Y4[loop+6],&Cb4[loop+6],&Cr4[loop+6],&A4[loop+6]);
                YUVA(p[7].r,p[7].g,p[7].b,p[7].a,&Y4[loop+7],&Cb4[loop+7],&Cr4[loop+7],&A4[loop+7]);
                loop += 8;
            }

            /* Down sample Cr and Cb */

            for (loop2=0;loop2<4;++loop2)
            {
                for (loop=0;loop<4;++loop)
                {
                    Cb[0+loop+(loop2<<3)]=((Cb1[0+(loop<<1)+(loop2<<4)]+Cb1[1+(loop<<1)+(loop2<<4)]+Cb1[8+(loop<<1)+(loop2<<4)]+Cb1[9+(loop<<1)+(loop2<<4)]))/4;
                    Cb[4+loop+(loop2<<3)]=((Cb2[0+(loop<<1)+(loop2<<4)]+Cb2[1+(loop<<1)+(loop2<<4)]+Cb2[8+(loop<<1)+(loop2<<4)]+Cb2[9+(loop<<1)+(loop2<<4)]))/4;
                    Cb[32+loop+(loop2<<3)]=((Cb3[0+(loop<<1)+(loop2<<4)]+Cb3[1+(loop<<1)+(loop2<<4)]+Cb3[8+(loop<<1)+(loop2<<4)]+Cb3[9+(loop<<1)+(loop2<<4)]))/4;
                    Cb[36+loop+(loop2<<3)]=((Cb4[0+(loop<<1)+(loop2<<4)]+Cb4[1+(loop<<1)+(loop2<<4)]+Cb4[8+(loop<<1)+(loop2<<4)]+Cb4[9+(loop<<1)+(loop2<<4)]))/4;

                    Cr[0+loop+(loop2<<3)]=((Cr1[0+(loop<<1)+(loop2<<4)]+Cr1[1+(loop<<1)+(loop2<<4)]+Cr1[8+(loop<<1)+(loop2<<4)]+Cr1[9+(loop<<1)+(loop2<<4)]))/4;
                    Cr[4+loop+(loop2<<3)]=((Cr2[0+(loop<<1)+(loop2<<4)]+Cr2[1+(loop<<1)+(loop2<<4)]+Cr2[8+(loop<<1)+(loop2<<4)]+Cr2[9+(loop<<1)+(loop2<<4)]))/4;
                    Cr[32+loop+(loop2<<3)]=((Cr3[0+(loop<<1)+(loop2<<4)]+Cr3[1+(loop<<1)+(loop2<<4)]+Cr3[8+(loop<<1)+(loop2<<4)]+Cr3[9+(loop<<1)+(loop2<<4)]))/4;
                    Cr[36+loop+(loop2<<3)]=((Cr4[0+(loop<<1)+(loop2<<4)]+Cr4[1+(loop<<1)+(loop2<<4)]+Cr4[8+(loop<<1)+(loop2<<4)]+Cr4[9+(loop<<1)+(loop2<<4)]))/4;
                }
            }

            /* DCT the 8x8 blocks and add to output buffer - cr cb y1 y2 y3 y4 order */

            fdct(mdec,&Cr[0],DCT_Output+64*0);
            fdct(mdec,&Cb[0],DCT_Output+64*1);
            fdct(mdec,&Y1[0],DCT_Output+64*2);
            fdct(mdec,&Y2[0],DCT_Output+64*3);
            fdct(mdec,&Y3[0],DCT_Output+64*4);
            fdct(mdec,&Y4[0],DCT_Output+64*5);
            DCT_Output+=64*6;

            if (alpha)
            {
                fdct(mdec,&A1[0],DCT_Output+64*0);
                fdct(mdec,&A2[0],DCT_Output+64*1);
                fdct(mdec,&A3[0],DCT_Output+64*2);
                fdct(mdec,&A4[0],DCT_Output+64*3);
                DCT_Output+=64*4;
            }
        }
    }
}


/* Used by QuantizeEncode()                                                */
/* Encode 16bits value into 10bits and place 6bits of data into upper bits */

/* value       - value to encode                */
/* upper_count - value to stuff in upper 6 bits */

/* Quantize picture using quality factor                   */
/* Then do step one of encoding - 0 counts                 */

/* DCT_Output  - source data to be quantized               */
/* QFactor     - quality factor, lower value high quality  */
/*               32, 16, 8, 4, 2, and 1                    */
/* Enc1_Output - Output buffer for encoding step one       */
/* length      - length of DCT_Output buffer in LONGs      */

/* result      - size of encoding step one buffer in words */

static STR_ULONG QuantizeEncode(STR_LONG *DCT_Output,STR_UWORD QFactor,STR_UWORD *Enc1_Output,STR_ULONG length)
{
    int qt[64];

    STR_ULONG r_length=0;
    STR_ULONG loop;

    unsigned int offset=0,zero_count;

    int v;
    int zi;
    int ZigZag[64] =
    {
      0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
      19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
      42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
      58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
    };

    unsigned char Quantum1[64] =
    {
      0x10,0x10,0x13,0x16,0x1a,0x1b,0x1d,0x22,
      0x10,0x10,0x16,0x18,0x1b,0x1d,0x22,0x25,
      0x13,0x16,0x1a,0x1b,0x1d,0x22,0x22,0x26,
      0x16,0x16,0x1a,0x1b,0x1d,0x22,0x25,0x28,
      0x16,0x1a,0x1b,0x1d,0x20,0x23,0x28,0x30,
      0x1a,0x1b,0x1d,0x20,0x23,0x28,0x30,0x3a,
      0x1a,0x1b,0x1d,0x22,0x26,0x2e,0x38,0x45,
      0x1b,0x1d,0x23,0x26,0x2e,0x38,0x45,0x53
    };

    int Quantum2[64];
    int i;

    for (i=0; i<64; ++i)
        Quantum2[i] = ((int) Quantum1[i] * QFactor);

    for (loop=0;loop<length;loop+=256)
    {
        /* Quantize Cr/Cb and Y differently - supposedly, but tables are identical */

        v = (STR_LONG)(DCT_Output[0]*16)/(Quantum1[0]);
        if (v<0)
            v = (v-1)/2;
        else
            v = (v+1)/2;
        qt[0]=v;
        offset = 1;
        do
        {
            zi = ZigZag[offset];
            v = (DCT_Output[zi]);

            if (v)
            {
                if (v<0)
                    v = -(((-v<<4) / Quantum2[zi] + 1)>>1);
                else
                    v = ((v<<4) / Quantum2[zi] + 1)>>1;
            }

            qt[offset]=v;
            ++offset;
        } while (offset<64);

        DCT_Output += 64;

        /* Step one of encoding */

        zero_count=0;
        Enc1_Output[r_length++]=(qt[0]&0x3ff);

        /* force lower right corner to be zero */

        qt[63]=0;
        qt[62]=0;

        offset = 1;
        do
        {
            if (qt[offset] == 0)
            {
                ++zero_count;
            }
            else
            {
                int v;

                v = qt[offset];
                Enc1_Output[r_length++]=(STR_UWORD)(zero_count<<10)|(v&0x3ff);
                zero_count=0;
            }
            ++offset;
        } while (offset<62);

        /* End of block marker */

        Enc1_Output[r_length++]=0xfe00;
    }

    /* End of frame marker - just to be sure */

    Enc1_Output[r_length++]=0xfe00;
    return(r_length*2);
}

/* Used by EncodeFinal()                           */

#ifdef MSDOS

#define gputmdeclong(p,v) \
 *((unsigned short *)((char *)(p)+0)) |= ((unsigned int) v >> 16); \
 *((unsigned short *)((char *)(p)+2)) |= ((unsigned int) v >> 0);

#else

#define gputmdeclong(p,v) \
 *(((unsigned char *)(p))+1) |= ((unsigned int) v >> 24); \
 *(((unsigned char *)(p))+0) |= ((unsigned int) v >> 16); \
 *(((unsigned char *)(p))+3) |= ((unsigned int) v >> 8); \
 *(((unsigned char *)(p))+2) |= ((unsigned int) v >> 0);

#endif


/* bitstream goes like this: */
/* 8..15  0..7  24..31  16..23 */
/* 7  0   7  0   7  0    7  0 */

/* Add 'n' upper bits of value to output stream                           */
/* Assumes output buffer is initially set to zero                         */

/* Enc2_Output - point to final output buffer                             */
/* value       - data to add to buffer - starts with most significant bit */
/* bitcount    - number of bits to add to output stream                   */
/* bitpos      - position within the output stream to add the data        */

#define EncodeComp(Enc2_Output,value,bitcount,bitpos) gputmdeclong(((char *)Enc2_Output + ((*bitpos)>>4)*2), ((((unsigned int)(value) & (0xffff0000>>bitcount)) << 16) >> ((*bitpos)&15))); (*bitpos) += (bitcount);

/* Perform final encoding step.  Looks for values in      */
/* standard encoding tables and encodes a shorter version */
/* into the output stream (Huffman coding scheme          */

/* Enc1_Output - pointer to encoded data from step one    */
/* Enc2_Output - pointer to final buffer output           */

/* result      - length of final encoding in bits         */

#if 1
STR_WORD fixvalue(STR_WORD value)
{
        if(value & 0x0200)
        {
                value = value & 0x03ff;
                value = value - 0x0400;
        } else
        {
                value = value & 0x03ff;
        }
        return(value);
}
#endif

static STR_ULONG EncodeFinal(STR_UWORD *Enc1_Output,STR_ULONG *Enc2_Output,int enctype,int *enc2bits,int *enc3bits, int alpha)
{
    STR_ULONG offset=0;
    STR_ULONG bitpos=0;
#if MDEC3
        STR_LONG v1,v2,v3;
        STR_LONG prev_y,prev_cr,prev_cb;
        STR_LONG flag;
        STR_LONG bits,mask,cnt;
        STR_LONG block,blocks;
#endif

    unsigned int value;
    int done,loop;
    int lbit1;
    int lbit2;

#if MDEC3
        STR_UWORD NEnTab1[36] =
        {
                0x02,0x0001,0x00,0x0000,
                0x02,0x0002,0x00,0x4000,
                0x03,0x0000,0x00,0x8000,
                0x03,0x0003,0x00,0xa000,
                0x03,0x0004,0x00,0xc000,
                0x04,0x0005,0x00,0xe000,
                0x05,0x0006,0x00,0xf000,
                0x06,0x0007,0x00,0xf800,
                0x07,0x0008,0x00,0xfc00,
        };

        STR_UWORD NEnTab2[36] =
        {
                0x02,0x0000,0x00,0x0000,
                0x02,0x0001,0x00,0x4000,
                0x02,0x0002,0x00,0x8000,
                0x03,0x0003,0x00,0xc000,
                0x04,0x0004,0x00,0xe000,
                0x05,0x0005,0x00,0xf000,
                0x06,0x0006,0x00,0xf800,
                0x07,0x0007,0x00,0xfc00,
                0x08,0x0008,0x00,0xfe00,
        };
#endif

    unsigned int EnTab1[96*4] =
    {
        0x02,0xFE00,0x00,0x8000,
        0x03,0x0001,0x00,0xC000,
        0x03,0x03FF,0x00,0xE000,
        0x04,0x0401,0x00,0x6000,
        0x04,0x07FF,0x00,0x7000,
        0x05,0x0002,0x00,0x4000,
        0x05,0x03FE,0x00,0x4800,
        0x05,0x0801,0x00,0x5000,
        0x05,0x0BFF,0x00,0x5800,
        0x06,0x0003,0x00,0x2800,
        0x06,0x03FD,0x00,0x2C00,
        0x06,0x0C01,0x00,0x3800,
        0x06,0x0FFF,0x00,0x3C00,
        0x06,0x1001,0x00,0x3000,
        0x06,0x13FF,0x00,0x3400,
        0x07,0x0402,0x00,0x1800,
        0x07,0x07FE,0x00,0x1A00,
        0x07,0x1401,0x00,0x1C00,
        0x07,0x17FF,0x00,0x1E00,
        0x07,0x1801,0x00,0x1400,
        0x07,0x1BFF,0x00,0x1600,
        0x07,0x1C01,0x00,0x1000,
        0x07,0x1FFF,0x00,0x1200,
        0x06,0x7C1F,0x00,0x0400,
        0x08,0x0004,0x00,0x0C00,
        0x08,0x03FC,0x00,0x0D00,
        0x08,0x0802,0x00,0x0800,
        0x08,0x0BFE,0x00,0x0900,
        0x08,0x2001,0x00,0x0E00,
        0x08,0x23FF,0x00,0x0F00,
        0x08,0x2401,0x00,0x0A00,
        0x08,0x27FF,0x00,0x0B00,
        0x09,0x0005,0x00,0x2600,
        0x09,0x03FB,0x00,0x2680,
        0x09,0x0006,0x00,0x2100,
        0x09,0x03FA,0x00,0x2180,
        0x09,0x0403,0x00,0x2500,
        0x09,0x07FD,0x00,0x2580,
        0x09,0x0C02,0x00,0x2400,
        0x09,0x0FFE,0x00,0x2480,
        0x09,0x2801,0x00,0x2700,
        0x09,0x2BFF,0x00,0x2780,
        0x09,0x2C01,0x00,0x2300,
        0x09,0x2FFF,0x00,0x2380,
        0x09,0x3001,0x00,0x2200,
        0x09,0x33FF,0x00,0x2280,
        0x09,0x3401,0x00,0x2000,
        0x09,0x37FF,0x00,0x2080,
        0x0B,0x0007,0x00,0x0280,
        0x0B,0x03F9,0x00,0x02A0,
        0x0B,0x0404,0x00,0x0300,
        0x0B,0x07FC,0x00,0x0320,
        0x0B,0x0803,0x00,0x02C0,
        0x0B,0x0BFD,0x00,0x02E0,
        0x0B,0x1002,0x00,0x03C0,
        0x0B,0x13FE,0x00,0x03E0,
        0x0B,0x1402,0x00,0x0240,
        0x0B,0x17FE,0x00,0x0260,
        0x0B,0x3801,0x00,0x0380,
        0x0B,0x3BFF,0x00,0x03A0,
        0x0B,0x3C01,0x00,0x0340,
        0x0B,0x3FFF,0x00,0x0360,
        0x0B,0x4001,0x00,0x0200,
        0x0B,0x43FF,0x00,0x0220,
        0x0D,0x0008,0x00,0x01D0,
        0x0D,0x03F8,0x00,0x01D8,
        0x0D,0x0009,0x00,0x0180,
        0x0D,0x03F7,0x00,0x0188,
        0x0D,0x000A,0x00,0x0130,
        0x0D,0x03F6,0x00,0x0138,
        0x0D,0x000B,0x00,0x0100,
        0x0D,0x03F5,0x00,0x0108,
        0x0D,0x0405,0x00,0x01B0,
        0x0D,0x07FB,0x00,0x01B8,
        0x0D,0x0804,0x00,0x0140,
        0x0D,0x0BFC,0x00,0x0148,
        0x0D,0x0C03,0x00,0x01C0,
        0x0D,0x0FFD,0x00,0x01C8,
        0x0D,0x1003,0x00,0x0120,
        0x0D,0x13FD,0x00,0x0128,
        0x0D,0x1802,0x00,0x01E0,
        0x0D,0x1BFE,0x00,0x01E8,
        0x0D,0x1C02,0x00,0x0150,
        0x0D,0x1FFE,0x00,0x0158,
        0x0D,0x2002,0x00,0x0110,
        0x0D,0x23FE,0x00,0x0118,
        0x0D,0x4401,0x00,0x01F0,
        0x0D,0x47FF,0x00,0x01F8,
        0x0D,0x4801,0x00,0x01A0,
        0x0D,0x4BFF,0x00,0x01A8,
        0x0D,0x4C01,0x00,0x0190,
        0x0D,0x4FFF,0x00,0x0198,
        0x0D,0x5001,0x00,0x0170,
        0x0D,0x53FF,0x00,0x0178,
        0x0D,0x5401,0x00,0x0160,
        0x0D,0x57FF,0x00,0x0168
    };

    unsigned int EnTab2[128*4] =
    {
        0x06,0x000C,0x00,0xD000,
        0x06,0x03F4,0x00,0xD400,
        0x06,0x000D,0x00,0xC800,
        0x06,0x03F3,0x00,0xCC00,
        0x06,0x000E,0x00,0xC000,
        0x06,0x03F2,0x00,0xC400,
        0x06,0x000F,0x00,0xB800,
        0x06,0x03F1,0x00,0xBC00,
        0x06,0x0406,0x00,0xB000,
        0x06,0x07FA,0x00,0xB400,
        0x06,0x0407,0x00,0xA800,
        0x06,0x07F9,0x00,0xAC00,
        0x06,0x0805,0x00,0xA000,
        0x06,0x0BFB,0x00,0xA400,
        0x06,0x0C04,0x00,0x9800,
        0x06,0x0FFC,0x00,0x9C00,
        0x06,0x1403,0x00,0x9000,
        0x06,0x17FD,0x00,0x9400,
        0x06,0x2402,0x00,0x8800,
        0x06,0x27FE,0x00,0x8C00,
        0x06,0x2802,0x00,0x8000,
        0x06,0x2BFE,0x00,0x8400,
        0x06,0x5801,0x00,0xF800,
        0x06,0x5BFF,0x00,0xFC00,
        0x06,0x5C01,0x00,0xF000,
        0x06,0x5FFF,0x00,0xF400,
        0x06,0x6001,0x00,0xE800,
        0x06,0x63FF,0x00,0xEC00,
        0x06,0x6401,0x00,0xE000,
        0x06,0x67FF,0x00,0xE400,
        0x06,0x6801,0x00,0xD800,
        0x06,0x6BFF,0x00,0xDC00,
        0x07,0x0010,0x00,0x7C00,
        0x07,0x03F0,0x00,0x7E00,
        0x07,0x0011,0x00,0x7800,
        0x07,0x03EF,0x00,0x7A00,
        0x07,0x0012,0x00,0x7400,
        0x07,0x03EE,0x00,0x7600,
        0x07,0x0013,0x00,0x7000,
        0x07,0x03ED,0x00,0x7200,
        0x07,0x0014,0x00,0x6C00,
        0x07,0x03EC,0x00,0x6E00,
        0x07,0x0015,0x00,0x6800,
        0x07,0x03EB,0x00,0x6A00,
        0x07,0x0016,0x00,0x6400,
        0x07,0x03EA,0x00,0x6600,
        0x07,0x0017,0x00,0x6000,
        0x07,0x03E9,0x00,0x6200,
        0x07,0x0018,0x00,0x5C00,
        0x07,0x03E8,0x00,0x5E00,
        0x07,0x0019,0x00,0x5800,
        0x07,0x03E7,0x00,0x5A00,
        0x07,0x001A,0x00,0x5400,
        0x07,0x03E6,0x00,0x5600,
        0x07,0x001B,0x00,0x5000,
        0x07,0x03E5,0x00,0x5200,
        0x07,0x001C,0x00,0x4C00,
        0x07,0x03E4,0x00,0x4E00,
        0x07,0x001D,0x00,0x4800,
        0x07,0x03E3,0x00,0x4A00,
        0x07,0x001E,0x00,0x4400,
        0x07,0x03E2,0x00,0x4600,
        0x07,0x001F,0x00,0x4000,
        0x07,0x03E1,0x00,0x4200,
        0x08,0x0020,0x00,0x3000,
        0x08,0x03E0,0x00,0x3100,
        0x08,0x0021,0x00,0x2E00,
        0x08,0x03DF,0x00,0x2F00,
        0x08,0x0022,0x00,0x2C00,
        0x08,0x03DE,0x00,0x2D00,
        0x08,0x0023,0x00,0x2A00,
        0x08,0x03DD,0x00,0x2B00,
        0x08,0x0024,0x00,0x2800,
        0x08,0x03DC,0x00,0x2900,
        0x08,0x0025,0x00,0x2600,
        0x08,0x03DB,0x00,0x2700,
        0x08,0x0026,0x00,0x2400,
        0x08,0x03DA,0x00,0x2500,
        0x08,0x0027,0x00,0x2200,
        0x08,0x03D9,0x00,0x2300,
        0x08,0x0028,0x00,0x2000,
        0x08,0x03D8,0x00,0x2100,
        0x08,0x0408,0x00,0x3E00,
        0x08,0x07F8,0x00,0x3F00,
        0x08,0x0409,0x00,0x3C00,
        0x08,0x07F7,0x00,0x3D00,
        0x08,0x040A,0x00,0x3A00,
        0x08,0x07F6,0x00,0x3B00,
        0x08,0x040B,0x00,0x3800,
        0x08,0x07F5,0x00,0x3900,
        0x08,0x040C,0x00,0x3600,
        0x08,0x07F4,0x00,0x3700,
        0x08,0x040D,0x00,0x3400,
        0x08,0x07F3,0x00,0x3500,
        0x08,0x040E,0x00,0x3200,
        0x08,0x07F2,0x00,0x3300,
        0x09,0x040F,0x00,0x1300,
        0x09,0x07F1,0x00,0x1380,
        0x09,0x0410,0x00,0x1200,
        0x09,0x07F0,0x00,0x1280,
        0x09,0x0411,0x00,0x1100,
        0x09,0x07EF,0x00,0x1180,
        0x09,0x0412,0x00,0x1000,
        0x09,0x07EE,0x00,0x1080,
        0x09,0x1803,0x00,0x1400,
        0x09,0x1BFD,0x00,0x1480,
        0x09,0x2C02,0x00,0x1A00,
        0x09,0x2FFE,0x00,0x1A80,
        0x09,0x3002,0x00,0x1900,
        0x09,0x33FE,0x00,0x1980,
        0x09,0x3402,0x00,0x1800,
        0x09,0x37FE,0x00,0x1880,
        0x09,0x3802,0x00,0x1700,
        0x09,0x3BFE,0x00,0x1780,
        0x09,0x3C02,0x00,0x1600,
        0x09,0x3FFE,0x00,0x1680,
        0x09,0x4002,0x00,0x1500,
        0x09,0x43FE,0x00,0x1580,
        0x09,0x6C01,0x00,0x1F00,
        0x09,0x6FFF,0x00,0x1F80,
        0x09,0x7001,0x00,0x1E00,
        0x09,0x73FF,0x00,0x1E80,
        0x09,0x7401,0x00,0x1D00,
        0x09,0x77FF,0x00,0x1D80,
        0x09,0x7801,0x00,0x1C00,
        0x09,0x7BFF,0x00,0x1C80,
        0x09,0x7C01,0x00,0x1B00,
        0x09,0x7FFF,0x00,0x1B80
    };

#if MDEC3
        prev_y=prev_cr=prev_cb=block=blocks=0;
        lbit1=lbit2=0;
#endif

    for (;;)
    {
        value=Enc1_Output[offset++];

        /* Check to see if end of frame */

        if (value == 0xfe00)
        {
            break;
        }

#if MDEC3
        if (enctype==3)
        {

                /* version 3.0 mdec stores the DC coeffecient differently */
                v1 = (STR_LONG) (((STR_WORD)fixvalue((STR_WORD)value)));
                v2 = (v1 >> 2);

                /* take the difference between this coeff and the prev coeff - Cr Cb and Y */
                if(block == 0)
                {
                        v3 = v2 - prev_cr;
                        prev_cr = v2;
                } else if(block == 1)
                {
                        v3 = v2 - prev_cb;
                        prev_cb = v2;
                } else
                {
                        v3 = v2 - prev_y;
                        prev_y = v2;
                }

                /* check the sign, and set a flag */
                if(v3 <= 0)
                {
                        v3 = -v3;
                        flag=0;
                } else
                {
                        flag=1;
                }

                bits=8;
                do
                {
                        mask = 0x00000001;
                        cnt = bits-1;

                        /* could be replaced with a single line shift */
                        if(cnt)
                        {
                                while(cnt)
                                {
                                        mask <<= 1;
                                        cnt--;
                                }
                        }

                        mask &= v3;

                        if(mask)
                        {
                                break;

                        }
                        bits--;
                } while(bits);

                /* use different encoding tables for CrCb and Y - lookup BITS in tables and add it to output */
                if(!((block == 0) || (block == 1)))
                {
                        for(loop=0;loop<9;loop++)
                        {
                                if(bits == NEnTab1[loop*4+1])
                                {
                                        break;
                                }
                        }

                        EncodeComp(Enc2_Output,NEnTab1[loop*4+3],NEnTab1[loop*4],&bitpos);
                        lbit1 += NEnTab1[loop*4];
                } else
                {
                        for(loop=0;loop<9;loop++)
                        {
                                if(bits == NEnTab2[loop*4+1])
                                {
                                        break;
                                }
                        }

                        EncodeComp(Enc2_Output,NEnTab2[loop*4+3],NEnTab2[loop*4],&bitpos);
                        lbit1 += NEnTab2[loop*4];
                }

                if(!flag)
                {
                        mask = 0x00000001;
                        cnt = bits;

                        /* could be replaced with a single line shift */
                        if(cnt)
                        {
                                while(cnt)
                                {
                                        mask <<= 1;
                                        cnt--;
                                }
                        }

                        mask -= v3;
                        mask--;
                } else
                {
                        mask = v3;
                }

                /* add BITS bits of MASK to output stream */
                EncodeComp(Enc2_Output,(mask << (16-bits)),bits,&bitpos);

                lbit1+=bits;
                lbit2+=10;
        }
        else
#endif
        {
            /* First 10bits - actual value output */

            EncodeComp(Enc2_Output,(value << 6),10,&bitpos);
                lbit2+=10;
        }

        /* Grab next value, check to see if end of block */

        value=Enc1_Output[offset++];
        while (value != 0xfe00)
        {
            done=0;

            /* Check first encoding table */

            for (loop=0;loop<96;++loop)
            {
                if (value == EnTab1[loop*4+1])
                {
                    /* Found a match in the encoding table, encode it into the stream */

                    EncodeComp(Enc2_Output,EnTab1[loop*4+3],EnTab1[loop*4],&bitpos);
                    done=1;
                    break;
                }
            }

            /* Check second encoding table */

            if (!done)
            {
                for (loop=0;loop<128;++loop)
                {
                    if (value == EnTab2[loop*4+1])
                    {
                        /* Signal use table two */

                        EncodeComp(Enc2_Output,0,8,&bitpos);

                        /* Encode value into stream */

                        EncodeComp(Enc2_Output,EnTab2[loop*4+3],EnTab2[loop*4],&bitpos);
                        done=1;
                        break;
                    }
                }
            }

            /* Not in any tables, output true value */

            if (!done)
            {
                /* Signal for next 16bits as true value */

                loop=23;
                EncodeComp(Enc2_Output,EnTab1[loop*4+3],EnTab1[loop*4],&bitpos);

                /* Encode value into stream */

                EncodeComp(Enc2_Output,value,16,&bitpos);
            }
            value=Enc1_Output[offset++];
        }

        /* End of block marker */

        loop=0;
        EncodeComp(Enc2_Output,EnTab1[loop*4+3],EnTab1[loop*4],&bitpos);

#if MDEC3
        /* 4 extra alpha blocks ? */
        alpha ? (block = (block + 1) % YCCA_NUM_BLOCKS) : (block = (block + 1) % YCC_NUM_BLOCKS);
        blocks++;
#endif

    }

    /* End of frame marker */

#if MDEC3
    if (enctype==3)
    {
        /* End of frame marker - different for v3.0 */
        EncodeComp(Enc2_Output,(0x3ff << 6),10,&bitpos);
    }
    else
#endif
    {
        /* End of frame marker - v2.0 */
        EncodeComp(Enc2_Output,(0x1ff << 6),10,&bitpos);
    }
    *enc2bits = lbit2;
    *enc3bits = lbit1;

    return(bitpos);
}

static STR_ULONG CompressFrameMDEC(STRINFO *mdec)
{
    STR_LONG *DCT_Buffer;
    void *compBuff;
    STR_ULONG width, height, gmaxFrameSize;
    STR_ULONG dct_length,enc1_length,enc2_length,write_length;
    STR_ULONG *Enc2_Output;
    STR_UWORD xres,yres;
    STR_WORD *Enc1_Output;
    int QFactor;
    int BestQFactor;
    int LastQFactor;
    int done=0;
    int ver;
    int dct_blocks;

    width = mdec->mWidth;
    height = mdec->mHeight;

    xres = (STR_UWORD)mdec->mWidth;
    yres = (STR_UWORD)mdec->mHeight;

    compBuff = mdec->compBuff;

    gmaxFrameSize = mdec->reqFrameSize;

    /* Use an extra block for the Alpha */
    if (mdec->subtype & STR_ARGB)
        dct_blocks = YCCA_NUM_BLOCKS;
    else
        dct_blocks = YCC_NUM_BLOCKS;

    /* A better formula is needed to calc these buffer sizes */

    dct_length=((xres>>4)*(yres>>4)*dct_blocks*64*4);

    DCT_Buffer= (STR_LONG *)  galloc(dct_length);
    Enc1_Output=(STR_WORD *)  galloc(dct_length);
    Enc2_Output=(STR_ULONG *) ((char *)compBuff+8);

    write_length = gmaxFrameSize + 1;


    RGB_YUV_DCT(mdec,xres,yres,mdec->sourceFrame,mdec->rowbytes,DCT_Buffer, mdec->subtype & STR_ARGB);

    QFactor = LastQFactor = gmax(1,mdec->PrevQFactor-1);
    BestQFactor = 0;

    do
    {
        int enc2, enc3;

        memset(Enc1_Output,0,dct_length);
        memset(Enc2_Output,0,dct_length);

        enc1_length=(STR_ULONG)QuantizeEncode(DCT_Buffer,(STR_UWORD)QFactor,(STR_UWORD *)Enc1_Output,dct_length);

        enc2_length=EncodeFinal((STR_UWORD *)Enc1_Output,Enc2_Output,2,&enc2,&enc3, mdec->subtype & STR_ARGB);
        ver = 2;

#if MDEC3
// re-added Feb 14, 2000

//        enc2_length=EncodeFinal((STR_UWORD *)Enc1_Output,Enc2_Output,3,&enc2,&enc3, mdec->subtype & STR_ARGB);
//        ver = 3;

//#ifdef DEBUG
//printf("MDEC2 %d, MDEC3 %d\n",enc2a_length,enc2_length);
//#endif
//        if (enc2a_length<=enc2_length)
//        {
//            enc2_length=EncodeFinal((STR_UWORD *)Enc1_Output,Enc2_Output,2,&enc2,&enc3, mdec->subtype & STR_ARGB);
//            ver = 2;
//        }
#endif

        write_length=((enc2_length+15)&~15)/8;


        if (write_length<=gmaxFrameSize)    /* worked */
        {
            if (QFactor>1 && (QFactor<BestQFactor || !BestQFactor) && !(!BestQFactor && LastQFactor<QFactor))
            {
                BestQFactor = QFactor;
                --QFactor;      /* try better quality */
            }
            else
            {
                BestQFactor = QFactor;
                done = 1;       /* stop */
            }
        }
        else    /* didnt compress */
        {
            if (BestQFactor || QFactor)
            {
                done = 2;   /* recompress the previous and stop */
            }
            if (QFactor>=32)
            {
                done = 1;
            }
            else
            {
                ++QFactor;      /* try worse quality */
            }
        }
    } while (done!=1);


    if (mdec->subtype & STR_ARGB)   /* set high bit of version */
        ver |= 0x8000;

    mdec->PrevQFactor = QFactor;

    gputi((char *)compBuff+0,(enc1_length/4+31)&~31,2);
    gputi((char *)compBuff+2,0x3800,2);
    gputi((char *)compBuff+4,QFactor,2);
    gputi((char *)compBuff+6,ver,2);      /* version 2 or 3 */

    gfree(DCT_Buffer);
    gfree(Enc1_Output);

    mdec->frameSize = write_length;

    mdec->movieSize += mdec->frameSize;

    return(write_length);
}

static int CompressFrameRefpack(STRINFO *mdec)
{
    int width;
    int height;
    int x;
    int y;
    int ok=0;
    int packlen;
    unsigned char *s;
    unsigned char *d;
    void *compBuff;
    int write_length;

    width = mdec->mWidth;
    height = mdec->mHeight;

    write_length = mdec->reqFrameSize + 1;

    compBuff = galloc(8+mdec->width*mdec->height*2);
    if (compBuff)
    {
        d = (unsigned char *) compBuff+8;

        /* recode as PSX 16 bit */

        for (y=0; y<height; ++y)
        {
            s = (unsigned char *) mdec->sourceFrame + y*mdec->rowbytes;

            for (x=0; x<width; ++x)
            {
                unsigned int r;
                unsigned int g;
                unsigned int b;
                unsigned int rgb;

                r = ((ARGB *) s)->r >> 3;
                g = ((ARGB *) s)->g >> 3;
                b = ((ARGB *) s)->b >> 3;
                rgb = (b<<10)|(g<<5)|r;
                d[0] = rgb&255;
                d[1] = rgb>>8;
                d += 2;
                s += 4;
            }
        }

        packlen = REF_encode((char *)mdec->compBuff+8, compBuff,mdec->width*mdec->height*2,0);

        gputi((char *)mdec->compBuff+0,(packlen/4+31)&~31,2);
        gputi((char *)mdec->compBuff+2,0x3800,2);
        gputi((char *)mdec->compBuff+4,1,2);
        gputm((char *)mdec->compBuff+6,0x10fb,2);  /* version 1..3 for MDEC. 0x10fb for refpack */

        if (packlen<=(int)mdec->reqFrameSize)
            ok = 1;

        gfree(compBuff);
    }

    mdec->frameSize = write_length;
    mdec->movieSize += mdec->frameSize;

    return(ok);
}


static int CompressAlpha(STRINFO *mdec)
{
    int width;
    int height;
    int x;
    int y;
    int ok=0;
    int packlen      = 0;
    unsigned char *s = 0;
    unsigned char *d = 0;
    void *alphaBuff  = 0;
    int write_length = 0;

    width = mdec->mWidth;
    height = mdec->mHeight;

    write_length = mdec->reqFrameSize + 1;

    alphaBuff = galloc(8+mdec->width*mdec->height);
    if (alphaBuff)
    {
        /*d = (unsigned char *) alphaBuff+8; */
        d = (unsigned char *) alphaBuff;

        for (y=0; y<height; ++y)
        {
            s = (unsigned char *) mdec->sourceFrame + y*mdec->rowbytes;

            for (x=0; x<width; ++x)
            {
                unsigned char a;

#if 0
                *d = ((ARGB*)s)->a;
                d++;

#else
                /* reduce to 4 bit alpha */
                a = (unsigned char)((((unsigned int)((ARGB *)s)->a)*15+128)/255);

                if (x&1)
                {
                    *d |= a & 0xF;
                    d++;
                }
                else
                {
                    *d = (a<<4) & 0xF0;
                }
#endif
                s += 4;
            }
        }

        /*packlen = REF_encode((char *)mdec->alphaBuff, alphaBuff,mdec->width*mdec->height\2,0); */
        packlen = HUFF_encode((char *)mdec->alphaBuff,alphaBuff,mdec->width*mdec->height/2,0);
        ok = 1;

        gfree(alphaBuff);
    }

    mdec->alphaSize = packlen;
    /*mdec->movieSize += mdec->alphaSize; */

    return(ok);
}



static void StoreStrFrame(STRINFO *mdec, int subtype)
{
    STR_ULONG secNum;
    CDSECTOR sectorData;
    DCTCHUNK dctheader;
    char xa_video_sector[2336];
    int chunksize;

    memset(&sectorData,0,sizeof(sectorData));
    memset(&dctheader,0,sizeof(DCTCHUNK));
    memset(&xa_video_sector,0,2336);
    chunksize = (((mdec->frameSize+8)+3)&~3);

    if (subtype & STR_DCT)
    {
        STRPUTFIELD(dctheader.id, GMAKEID('D','C','T','m'));
        STRPUTFIELD(dctheader.frameSize, chunksize+sizeof(DCTCHUNK));
        STRPUTFIELD(dctheader.crc, 0x00000000);
        STRPUTFIELD(dctheader.width, mdec->mWidth);
        STRPUTFIELD(dctheader.height, mdec->mHeight);
        if (gwrite(mdec->outFile, (char *)(&dctheader), sizeof(DCTCHUNK)) == sizeof(DCTCHUNK))
            gwrite(mdec->outFile, (char *)(mdec->compBuff), chunksize);

        if ((mdec->alphaSize > 0) && (mdec->alphaBuff != NULL))
        {
            char buf[8];
            int alphaSize;

            alphaSize = ((mdec->alphaSize+8)+3)&~3;
            /*alphaSize = 100; */
            gputi(buf, GMAKEID('D','C','T','a'), 4);
            gputi(buf+4, alphaSize+8, 4);
            if (gwrite(mdec->outFile, buf, 8) == 8)
                gwrite(mdec->outFile, mdec->alphaBuff, alphaSize);
        }

    }
    else
    {
        for (secNum = 0; secNum < mdec->sectorsPerFrame; secNum++)
        {
            STRPUTFIELD(sectorData.id,0x0160);
            STRPUTFIELD(sectorData.type,0x8001);
            STRPUTFIELD(sectorData.secCount,secNum);
            STRPUTFIELD(sectorData.nSectors,mdec->sectorsPerFrame);
            STRPUTFIELD(sectorData.frameCount,mdec->frameNumber+1);
            STRPUTFIELD(sectorData.frameSize,chunksize);
            STRPUTFIELD(sectorData.width,mdec->mWidth);
            STRPUTFIELD(sectorData.height,mdec->mHeight);
            memcpy((char *) &sectorData.reserved[0],(char *)(mdec->compBuff),8);

            memcpy((char *) &sectorData.data[0],(char *)(mdec->compBuff) + secNum*USRSIZE*4,USRSIZE*4);

            if (subtype & STR_XA)
            {
                gputm(xa_video_sector+0,0x00014280,4);
                gputm(xa_video_sector+4,0x00014280,4);
                memcpy(xa_video_sector+8,&sectorData,2048);
                gwrite(mdec->outFile,(char *) &xa_video_sector,2336);
            }
            else
                gwrite(mdec->outFile,(char *) &sectorData,sizeof(CDSECTOR));

        }
    }
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL STR_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;
    STRINFO *mdec;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;

        mdec = (STRINFO *) galloc(sizeof(STRINFO));
        gx->gref = (void *) mdec;
        memset(mdec,0, sizeof(STRINFO));

        mdec->PrevQFactor = 1;

        init_fdct(mdec);

        mdec->outFile = g;
        ok = 1;
    }
    return(ok);
}

int GCALL STR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=0;
    STRINFO *mdec;
    int sectorsperframe;

    if (ginfo->bpp>8)
    {
        mdec = (STRINFO *) gx->gref;
        sectorsperframe = ginfo->quality;
        if (!sectorsperframe) sectorsperframe = 8;
        if (sectorsperframe<0) sectorsperframe = 1;
        if (sectorsperframe>32) sectorsperframe = 32;

        mdec->rowbytes = rowbytes;
        mdec->width = ginfo->width;
        mdec->height = ginfo->height;
        mdec->mWidth = (mdec->width) & ~15;
        mdec->mHeight = (mdec->height) & ~15;
        mdec->sectorsPerFrame = sectorsperframe;
        mdec->sourceFrame = (ARGB *) source;
        mdec->frameNumber = gx->framenum;
        mdec->reqFrameSize = sectorsperframe*USRSIZE*4-32;
        mdec->subtype = ginfo->subtype;

        mdec->compBuff = galloc(mdec->width*mdec->height*4*8);
        mdec->alphaBuff = galloc(mdec->width*mdec->height*8);  /* Just to be safe. */

        if (ginfo->subtype&STR_REF)
            ok = CompressFrameRefpack(mdec);
        else
        {
            CompressFrameMDEC(mdec);
            ok = 1;
        }


        if (ginfo->subtype&STR_DCTA)
        {
            CompressAlpha(mdec);
        }


        StoreStrFrame(mdec,ginfo->subtype);

        gfree(mdec->compBuff);
        gfree(mdec->alphaBuff);

        mdec->compBuff = 0;
        mdec->alphaBuff = 0;

        ++gx->framenum;
    }

    return(ok);
}

int GCALL STR_wclose(GINSTANCE *gx)
{
    int ok=0;
    STRINFO *mdec;

    if (gx)
    {
        mdec = (STRINFO *) gx->gref;
        if (mdec)
            gfree(mdec);

        ok = gfree((void *) gx);
    }

    return(ok);
}


