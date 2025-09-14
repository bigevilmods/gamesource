// (c) 1998-2007 Electronic Arts Inc.

#include "maddec.h"

static const int zigzag[64] =
{
    0, 8, 1, 2, 9, 16, 24, 17, 10, 3, 4, 11, 18, 25, 32, 40,
    33, 26, 19, 12, 5, 6, 13, 20, 27, 34, 41, 48, 56, 49, 42, 35,
    28, 21, 14, 7, 15, 22, 29, 36, 43, 50, 57, 58, 51, 44, 37, 30,
    23, 31, 38, 45, 52, 59, 60, 53, 46, 39, 47, 54, 61, 62, 55, 63
};

extern "C"
{
extern int idctinput[8*8];


extern const unsigned short  *maddataptr;
extern unsigned int   madshiftreg;
extern int             madbitcount;

extern char            madclipbiastbl[256];
extern unsigned int   madvlctbl1[512];
extern unsigned int   madvlctbl2[256];
extern unsigned int   madvlctbl3[256];
extern unsigned int   madvlctbl4[64];
extern int            madquant[64];
}

static void discardbits(int bits)
{
    madshiftreg <<= bits;
    madbitcount -= bits;

    if (madbitcount < 16)
    {
        madshiftreg |= *maddataptr << (16-madbitcount);
        madbitcount += 16;
        ++maddataptr;
    }
}


int madvlcdecode(void)
{
    int    *entry;
    int     val;
    int     bits;
    int     index;
    int     i;

    idctinput[0] = (((int) madshiftreg) >> (32-8)) * madquant[0];
    discardbits(8);

    entry = &idctinput[1];
    for (i = 0; i < 9; ++i)
    {
        entry[0] = 0;
        entry[1] = 0;
        entry[2] = 0;
        entry[3] = 0;
        entry[4] = 0;
        entry[5] = 0;
        entry[6] = 0;

        entry += 7;
    }

    index = 1;
    for (;;)
    {
        val = madvlctbl1[madshiftreg >> (32-9)];
        bits = val & 0xff;

        if (bits > 9)
        {
            if ((bits & 0x20) == 0)
            {
                if ((bits & 0x10) == 0)
                {
                    discardbits(9);
                    val = madvlctbl2[madshiftreg >> (32-8)];
                    bits = val & 0xff;
                }
                else
                {
                    discardbits(6);
                    val = madvlctbl3[madshiftreg >> (32-8)];
                    bits = val & 0xff;
                }
            }
            else
            {
                if ((bits & 0x10) == 0)
                {
                    discardbits(6);
                    val = madshiftreg;
                    bits = 16;
                }
                else
                {
                    discardbits(2);
                    return(index);
                }
            }
        }

        discardbits(bits);
        index += (val >> 16) & 0x3f;
        idctinput[zigzag[index]] = (val >> (32-10)) * madquant[zigzag[index]];
        index += 1;
    }
}

void madoutputrow(const int *luma, const int *chroma, YUVPAIR *dest)
{
    YUVPAIR     p;

    p.y1 = madclipbiastbl[(*(luma+0) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+1) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+0) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+64) >> 16) & 0xff];
    dest[0] = p;

    p.y1 = madclipbiastbl[(*(luma+2) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+3) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+1) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(1+64)) >> 16) & 0xff];
    dest[1] = p;

    p.y1 = madclipbiastbl[(*(luma+4) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+5) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+2) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(2+64)) >> 16) & 0xff];
    dest[2] = p;

    p.y1 = madclipbiastbl[(*(luma+6) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+7) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+3) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(3+64)) >> 16) & 0xff];
    dest[3] = p;

    p.y1 = madclipbiastbl[(*(luma+8) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+9) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+4) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(4+64)) >> 16) & 0xff];
    dest[4] = p;

    p.y1 = madclipbiastbl[(*(luma+10) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+11) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+5) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(5+64)) >> 16) & 0xff];
    dest[5] = p;

    p.y1 = madclipbiastbl[(*(luma+12) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+13) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+6) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(6+64)) >> 16) & 0xff];
    dest[6] = p;

    p.y1 = madclipbiastbl[(*(luma+14) >> 16) & 0xff];
    p.y2 = madclipbiastbl[(*(luma+15) >> 16) & 0xff];
    p.cb = madclipbiastbl[(*(chroma+7) >> 16) & 0xff];
    p.cr = madclipbiastbl[(*(chroma+(7+64)) >> 16) & 0xff];
    dest[7] = p;

}

