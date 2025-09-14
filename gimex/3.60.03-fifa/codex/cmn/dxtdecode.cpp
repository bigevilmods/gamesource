// (c) 1995-2007 Electronic Arts Inc.

#ifndef __TEXREAD
#define __TEXREAD 1

#include <string.h>
#include "codex.h"
#include "dxtcodex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#define NOTTHREEEIGHTHS 1

#define GET555(argb,p)  argb.a = (unsigned char) 255, argb.r = (unsigned char) ((((p)&31)*255+16)/31), argb.g = (unsigned char) ((((p>>5)&31)*255+16)/31), argb.b = (unsigned char) ((((p>>10)&31)*255+16)/31)
#define GET565(argb,p)  argb.a = (unsigned char) 255, argb.r = (unsigned char) ((((p>>11)&31)*255+16)/31), argb.g = (unsigned char) ((((p>>5)&63)*255+32)/63), argb.b = (unsigned char) (((p&31)*255+16)/31)
#define TERPC(dc,c1,c2,mulc1,mulc2,divc) dc = (unsigned char) ((c1*mulc1+c2*mulc2)/divc)
#define TERP(dargb,argb1,argb2,mul1,mul2,div) dargb.a=(unsigned char) 255,TERPC(dargb.r,argb1.r,argb2.r,mul1,mul2,div),TERPC(dargb.g,argb1.g,argb2.g,mul1,mul2,div),TERPC(dargb.b,argb1.b,argb2.b,mul1,mul2,div)
#define TERPNODIV(dargb,argb1,argb2,mul1,mul2,div) dargb.a=(unsigned char) 255,TERPNODIVC(dargb.r,argb1.r,argb2.r,mul1,mul2,div),TERPNODIVC(dargb.g,argb1.g,argb2.g,mul1,mul2,div),TERPNODIVC(dargb.b,argb1.b,argb2.b,mul1,mul2,div)
#define TERPNODIVC(dc,c1,c2,mulc1,mulc2,divc) dc = (unsigned char) ((c1*mulc1+c2*mulc2)>>divc)

static void s3decode(ARGB *d, int rowbytes, unsigned char *blockbuf, int is555,int dxt1b)
{
    unsigned int c0;
    unsigned int c1;
    ARGB clut[4];
    int y;
    if(!dxt1b)                                // if not bigendian format.
    {
        c0 = ggeti(blockbuf+0,2);
        c1 = ggeti(blockbuf+2,2);
    }
    else                                      // bigendian format do flip bytes.
    {
        c0 = ggetm(blockbuf+0,2);
        c1 = ggetm(blockbuf+2,2);
    }

// by default we decode using the 3/8 5/8 fast interpolation rather than
// the slower DXT standard 1/3 2/3 method.
#ifdef NOTTHREEEIGHTHS                        // faster nicer decodeing if we use three eights.
    if (is555)                                // calculate my loopup table here.
    {
        if (c0>c1)
        {
            GET555(clut[0],c0);
            GET555(clut[1],c1);
            TERP(clut[2],clut[0],clut[1],2,1,3);
            TERP(clut[3],clut[0],clut[1],1,2,3);
        }
        else
        {
            GET555(clut[0],c0);
            GET555(clut[1],c1);
            TERP(clut[2],clut[0],clut[1],1,1,2);
            clut[3].a = clut[3].r = clut[3].g = clut[3].b = 0;
        }
    }
    else
    {
        if (c0>c1)
        {
            GET565(clut[0],c0);
            GET565(clut[1],c1);
            TERP(clut[2],clut[0],clut[1],2,1,3);
            TERP(clut[3],clut[0],clut[1],1,2,3);
        }
        else
        {
            GET565(clut[0],c0);
            GET565(clut[1],c1);
            TERP(clut[2],clut[0],clut[1],1,1,2);
            clut[3].a = clut[3].r = clut[3].g = clut[3].b = 0;
        }
    }
#else
    if (is555)                                               // otherwise do fast calculation
    {                                                        // of lookup table.
        if (c0>c1)
        {
            GET555(clut[0],c0);
            GET555(clut[1],c1);
            TERPNODIV(clut[2],clut[0],clut[1],5,3,3);        // these use shifts rather than divide... small fraction faster.
            TERPNODIV(clut[3],clut[0],clut[1],3,5,3);
        }
        else
        {
            GET555(clut[0],c0);
            GET555(clut[1],c1);
            TERPNODIV(clut[2],clut[0],clut[1],1,1,1);
            clut[3].a = clut[3].r = clut[3].g = clut[3].b = 0;
        }
    }
    else
    {
        if (c0>c1)
        {
            GET565(clut[0],c0);
            GET565(clut[1],c1);
            TERPNODIV(clut[2],clut[0],clut[1],5,3,3);
            TERPNODIV(clut[3],clut[0],clut[1],3,5,3);
        }
        else
        {
            GET565(clut[0],c0);
            GET565(clut[1],c1);
            TERPNODIV(clut[2],clut[0],clut[1],1,1,1);
            clut[3].a = clut[3].r = clut[3].g = clut[3].b = 0;
        }
    }
#endif
    if(!dxt1b)                                        // not bigendian format so
    {                                                 // groups of 2 bits are in
        for (y=0; y<4; ++y)                           // scanline order.
        {
            d[0] = clut[(blockbuf[4+y]>>0)&3];
            d[1] = clut[(blockbuf[4+y]>>2)&3];
            d[2] = clut[(blockbuf[4+y]>>4)&3];
            d[3] = clut[(blockbuf[4+y]>>6)&3];
            d = (ARGB *)((unsigned char *)d+rowbytes);
        }
    }
    else
    {
        for (y=0; y<4; ++y)                          // bigendian format so groups
        {                                             // of 2 bits (lookup info) is
            d[3] = clut[(blockbuf[4+y]>>0)&3];        // in flipped scan line order.
            d[2] = clut[(blockbuf[4+y]>>2)&3];
            d[1] = clut[(blockbuf[4+y]>>4)&3];
            d[0] = clut[(blockbuf[4+y]>>6)&3];
            d = (ARGB *)((unsigned char *)d+rowbytes);
        }
    }

}

static void dxt5decode(ARGB *d, int rowbytes, unsigned char *blockbuf)
{
    unsigned char alpha0, alpha1, alpha2, alpha3, alpha4, alpha5, alpha6, alpha7;
    unsigned char *curPtr = (unsigned char*)blockbuf;
    int count = 0;

    alpha0 = *curPtr; curPtr++;
    alpha1 = *curPtr; curPtr++;
    if (alpha0 > alpha1)
    {
        // 8-alpha block:  derive the other six alphas.
        // Bit code 000 = alpha0, 001 = alpha1, others are interpolated.
        alpha2 = (unsigned char)((6 * alpha0 + 1 * alpha1 + 3) / 7);    // bit code 010
        alpha3 = (unsigned char)((5 * alpha0 + 2 * alpha1 + 3) / 7);    // bit code 011
        alpha4 = (unsigned char)((4 * alpha0 + 3 * alpha1 + 3) / 7);    // bit code 100
        alpha5 = (unsigned char)((3 * alpha0 + 4 * alpha1 + 3) / 7);    // bit code 101
        alpha6 = (unsigned char)((2 * alpha0 + 5 * alpha1 + 3) / 7);    // bit code 110
        alpha7 = (unsigned char)((1 * alpha0 + 6 * alpha1 + 3) / 7);    // bit code 111
    }
    else
    {
        // 6-alpha block.
        // Bit code 000 = alpha0, 001 = alpha1, others are interpolated.
        alpha2 = (unsigned char)((4 * alpha0 + 1 * alpha1 + 2) / 5);    // Bit code 010
        alpha3 = (unsigned char)((3 * alpha0 + 2 * alpha1 + 2) / 5);    // Bit code 011
        alpha4 = (unsigned char)((2 * alpha0 + 3 * alpha1 + 2) / 5);    // Bit code 100
        alpha5 = (unsigned char)((1 * alpha0 + 4 * alpha1 + 2) / 5);    // Bit code 101
        alpha6 = 0;                                                     // Bit code 110
        alpha7 = 255;                                                   // Bit code 111
    }

    // let's decode 2 rows at a time (24 bits)
    for ( int k = 0; k < 2; k++ ) // for every two rows
    {
        unsigned long src = *(unsigned long*)curPtr; curPtr += 3; // advance 24 bits (3 bytes)

        for ( int j = 0; j < 2; j++ ) // for each row
        {
            for ( int i = 0; i < 4; i++ ) // for each 3 bit entry in this row
            {
                unsigned char bitCode = (unsigned char)(src & 0x7); src >>= 3;
                switch( bitCode )
                {
                case 0:
                    d[count].a = alpha0;
                    break;
                case 1:
                    d[count].a = alpha1;
                    break;
                case 2:
                    d[count].a = alpha2;
                    break;
                case 3:
                    d[count].a = alpha3;
                    break;
                case 4:
                    d[count].a = alpha4;
                    break;
                case 5:
                    d[count].a = alpha5;
                    break;
                case 6:
                    d[count].a = alpha6;
                    break;
                case 7:
                    d[count].a = alpha7;
                    break;
                }
                count++;
            }
        }
    }
    d = (ARGB *)((unsigned char *)d+rowbytes*4);
}





#undef GET555
#undef GET565
#undef TERPC
#undef TERP

#define DUP4(x) (unsigned char) (((x)<<4)|(x))
static void dxt3decode(ARGB *d, int rowbytes, unsigned char *blockbuf)
{
    int y;

    for (y=0; y<4; ++y)
    {
        d[0].a = DUP4(blockbuf[y*2+0]&15);
        d[1].a = DUP4(blockbuf[y*2+0]>>4);
        d[2].a = DUP4(blockbuf[y*2+1]&15);
        d[3].a = DUP4(blockbuf[y*2+1]>>4);
        d = (ARGB *)((unsigned char *)d+rowbytes);
    }
}
#undef DUP4

#define S3TCGETFIELD(field) ggetm((void *)&field, sizeof(field))

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* first 4 bytes make hold a signature */

bool GCALL DXT_is(const void *compresseddata)
{
    bool ok=false;
    DXTHEADER *header = (DXTHEADER*)compresseddata;

    if ( (ggetm(header->code, sizeof(header->code)) == DXT_DXT1_CODE    ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_DXT3_CODE    ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_DXT5_CODE    ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_DXT1B_CODE   ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT1_CODE ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT3_CODE ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT5_CODE ) ||
         (ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT1B_CODE) )

    {
        ok = true;
    }
    return(ok);
}

int GCALL DXT3_is(const void *compresseddata)
{
    DXTHEADER *header = (DXTHEADER*)compresseddata;
    int ok = 0;

    if (ggetm(header->code, sizeof(header->code)) == DXT_DXT3_CODE    ||
        ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT3_CODE  )
    {
        ok = 100;
    }
    return(ok);
}

int GCALL DXT5_is(const void *compresseddata)
{
    DXTHEADER *header = (DXTHEADER*)compresseddata;
    int ok = 0;

    if (ggetm(header->code, sizeof(header->code)) == DXT_DXT5_CODE   ||
        ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT5_CODE )
    {
        ok = 100;
    }
    return(ok);
}


int GCALL DXT1B_is(const void *compresseddata)
{
    DXTHEADER *header = (DXTHEADER*)compresseddata;
    int ok    = 0;
    if (ggetm(header->code, sizeof(header->code)) == DXT_DXT1B_CODE ||
        ggetm(header->code, sizeof(header->code)) == DXT_BIGDXT1B_CODE )
    {
        ok = 100;
    }
    return(ok);
}



/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL DXT_size(const void *compresseddata)
{
    DXTHEADER *header = (DXTHEADER*)compresseddata;
    BIGDXTHEADER * Bigheader = (BIGDXTHEADER*)compresseddata;
    int size=0;
    if((header->code[0])&0x80)
        size = ggetm(Bigheader->size,sizeof(Bigheader->size));
    else
        size = ggetm(header->size, sizeof(header->size));
    return(size);
}

int GCALL DXT_decode(void *uncompressed, const void *compressed, int *compressedsizeptr)
{
    DXTHEADER *header=(DXTHEADER*)compressed;
    BIGDXTHEADER *Bigheader=(BIGDXTHEADER*)compressed;
    unsigned char *dest = (unsigned char *)uncompressed;
    int size;
    int is555;
    int i;
    int isBig = 0;
    int isAttenuated;
    ARGB *d;

    isBig = (header->code[0] & 0x80)?1:0;             // check if high bit is set letting us know this is a BIG texture (>16MB)
    if(isBig)
    {
        size   = ggetm(Bigheader->size, sizeof(Bigheader->size));
        is555  = ggetm(Bigheader->mode, sizeof(Bigheader->mode))&DXT_ENCODE_RGB_555;
        isAttenuated = ggetm(Bigheader->mode, sizeof(Bigheader->mode))&DXT_ENCODE_RGB_ATTENUATE;
        compressed = (void *)((unsigned char *) compressed + sizeof(BIGDXTHEADER));
    }
    else
    {
        size   = ggetm(header->size, sizeof(header->size));
        is555  = ggetm(header->mode, sizeof(header->mode))&DXT_ENCODE_RGB_555;
        isAttenuated = ggetm(header->mode, sizeof(header->mode))&DXT_ENCODE_RGB_ATTENUATE;
        compressed = (void *)((unsigned char *) compressed + sizeof(DXTHEADER));
    }
    int dxt3  = DXT3_is(header);
    int dxt5  = DXT5_is(header);
    int dxt1b = DXT1B_is(header);

    if (compressedsizeptr)
    {
        if (dxt3 || dxt5)
            *compressedsizeptr = size / 4;
        else
            *compressedsizeptr = size / 8;
    }

    if (dxt3)
    {
        for (i=0; i<size; i+=64)
        {
            s3decode((ARGB*)dest, 4 * sizeof(ARGB), (unsigned char *)compressed+8,is555,dxt1b);
            dxt3decode((ARGB*)dest, 4 * sizeof(ARGB), (unsigned char *)compressed);
            dest += 64;
            compressed = (void *)((unsigned char *) compressed + 16);
        }
    }
    else if (dxt5)
    {
        for (i=0; i<size; i+=64)
        {
            s3decode((ARGB*)dest, 4 * sizeof(ARGB), (unsigned char *)compressed+8,is555,dxt1b);
            dxt5decode((ARGB*)dest, 4 * sizeof(ARGB), (unsigned char *)compressed);
            dest += 64;
            compressed = (void *)((unsigned char *) compressed + 16);
        }
    }
    else if (dxt1b)
    {
        for (i=0; i<size; i+=64)
        {
            s3decode((ARGB*)dest, 4 * sizeof(ARGB), (unsigned char *)compressed,is555,dxt1b);
            dest += 64;
            compressed = (void *)((unsigned char *) compressed + 8);
        }
    }

    else
    {
        for (i=0; i<size; i+=64)
        {
            s3decode((ARGB*)dest, 4 * sizeof(ARGB), (unsigned char *)compressed,is555,dxt1b);
            dest += 64;
            compressed = (void *)((unsigned char *) compressed + 8);
        }
    }
    if(isAttenuated)
    {
         d = (ARGB *)uncompressed;
         for(i = 0; i < size;i+=4)
         {
            if(d->a != 0)
            {
                d->r = (unsigned char)((d->r * 255 + 128) / d->a);
                d->g = (unsigned char)((d->g * 255 + 128) / d->a);
                d->b = (unsigned char)((d->b * 255 + 128) / d->a);
            }
            d++;
         }
    }

    return(size);
}

#undef S3TCGETFIELD

#endif

