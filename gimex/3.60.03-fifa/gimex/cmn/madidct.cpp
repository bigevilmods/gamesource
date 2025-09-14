// (c) 1998-2007 Electronic Arts Inc.

#include "madgimex.h"
#include "maddec.h"

#define a1  0x0B505
#define a2  0x08A8C
#define a3  0x14E7B
#define a4  0x061F8

#define fixedmult(a,b) ((int)((double)(a) * (double)(b) / 65536.0))

int idctinput[8*8];

int const idctprescale[8*8] =
{
    0x02000, 0x01712, 0x0187e, 0x01b37, 0x02000, 0x028ba, 0x03b21, 0x073fc,
    0x01712, 0x010a2, 0x011a8, 0x0139f, 0x01712, 0x01d5d, 0x02aa1, 0x0539f,
    0x0187e, 0x011a8, 0x012bf, 0x014d4, 0x0187e, 0x01f2c, 0x02d41, 0x058c5,
    0x01b37, 0x0139f, 0x014d4, 0x01725, 0x01b37, 0x022a3, 0x03249, 0x062a3,
    0x02000, 0x01712, 0x0187e, 0x01b37, 0x02000, 0x028ba, 0x03b21, 0x073fc,
    0x028ba, 0x01d5d, 0x01f2c, 0x022a3, 0x028ba, 0x033d6, 0x04b42, 0x0939f,
    0x03b21, 0x02aa1, 0x02d41, 0x03249, 0x03b21, 0x04b42, 0x06d41, 0x0d650,
    0x073fc, 0x0539f, 0x058c5, 0x062a3, 0x073fc, 0x0939f, 0x0d650, 0x1a463,
};


static int work[8*8];

static void IdctColumn( int* src, int* dest )
{
    int t1,t2,t3,t4,t5,t6,t7,t8,t9;

    if( (src[1] | src[2] | src[3] | src[4] | src[5] | src[6] | src[7]) == 0 )
    {
        dest[0*8] = src[0];
        dest[1*8] = src[0];
        dest[2*8] = src[0];
        dest[3*8] = src[0];
        dest[4*8] = src[0];
        dest[5*8] = src[0];
        dest[6*8] = src[0];
        dest[7*8] = src[0];
    }
    else
    {
        t1 = src[5] - src[3];
        t2 = src[1] + src[7];
        t3 = src[1] - src[7];
        t4 = src[5] + src[3];

        t5 = t2 - t4;
        t4 += t2;

        t2 = fixedmult( t1 + t3, a4 );
        t1 = fixedmult( t1, a2 ) + t2;
        t5 = fixedmult( t5, a1 );
        t3 = fixedmult( t3, a3 ) - t2;

        t4 += t3;
        t3 += t5;
        t5 += t1;

        t6 = src[0] + src[4];
        t7 = src[0] - src[4];
        t8 = fixedmult( src[2] - src[6], a1 );
        t9 = src[2] + src[6] + t8;

        t2 = t6 - t9;
        t6 += t9;
        t9 = t7 - t8;
        t7 += t8;

        dest[0*8] = t6 + t4;
        dest[1*8] = t7 + t3;
        dest[2*8] = t9 + t5;
        dest[3*8] = t2 + t1;
        dest[4*8] = t2 - t1;
        dest[5*8] = t9 - t5;
        dest[6*8] = t7 - t3;
        dest[7*8] = t6 - t4;
    }
}


static void IdctRow( int* src, int* dest )
{
    int t1,t2,t3,t4,t5,t6,t7,t8,t9;

    t1 = src[5] - src[3];
    t2 = src[1] + src[7];
    t3 = src[1] - src[7];
    t4 = src[5] + src[3];

    t5 = t2 - t4;
    t4 += t2;

    t2 = fixedmult( t1 + t3, a4 );
    t1 = fixedmult( t1, a2 ) + t2;
    t5 = fixedmult( t5, a1 );
    t3 = fixedmult( t3, a3 ) - t2;

    t4 += t3;
    t3 += t5;
    t5 += t1;

    t6 = src[0] + src[4];
    t7 = src[0] - src[4];
    t8 = fixedmult( src[2] - src[6], a1 );
    t9 = src[2] + src[6] + t8;

    t2 = t6 - t9;
    t6 += t9;
    t9 = t7 - t8;
    t7 += t8;

    dest[0] = t6 + t4;
    dest[1] = t7 + t3;
    dest[2] = t9 + t5;
    dest[3] = t2 + t1;
    dest[4] = t2 - t1;
    dest[5] = t9 - t5;
    dest[6] = t7 - t3;
    dest[7] = t6 - t4;
}


void idctcompute( int *dest, int stride )
{
    IdctColumn( idctinput+0*8, work+0 );
    IdctColumn( idctinput+1*8, work+1 );
    IdctColumn( idctinput+2*8, work+2 );
    IdctColumn( idctinput+3*8, work+3 );
    IdctColumn( idctinput+4*8, work+4 );
    IdctColumn( idctinput+5*8, work+5 );
    IdctColumn( idctinput+6*8, work+6 );
    IdctColumn( idctinput+7*8, work+7 );
    IdctRow( work+0*8, dest+0*stride );
    IdctRow( work+1*8, dest+1*stride );
    IdctRow( work+2*8, dest+2*stride );
    IdctRow( work+3*8, dest+3*stride );
    IdctRow( work+4*8, dest+4*stride );
    IdctRow( work+5*8, dest+5*stride );
    IdctRow( work+6*8, dest+6*stride );
    IdctRow( work+7*8, dest+7*stride );
}

#undef fixedmult
