// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <math.h>
#include "gimex.h"
#include "dctgimex.h"

void initquant(int *quant,int quality)
{
    int x;
    int y;

    int l;
    int h;

    l = 8  +(100-quality)*(30-8)/100; /* 8..10 -> 30..60 */
    h = 10 +(100-quality)*(60-12)/100;

    for (y=0; y<8; ++y)
        for (x=0; x<8; ++x)
            *quant++ = (int) ((x+y)*(h-l)/14+l);
}

/****************************************************************/
/*  Forward Discrete Cosine Transform                           */
/****************************************************************/

#define DCTPI 3.14159265358979323846

void initfdct(double coeff[8][8])
{
    int i, j;
    double s;

    for (i=0; i<8; i++)
    {
        s = (i==0) ? sqrt(0.125) : 0.5;

        for (j=0; j<8; j++)
            coeff[i][j] = s * cos((DCTPI/8.0)*i*(j+0.5));
    }
}

void fdct(double coeff[8][8],short *block)
{
    int i, j, k;
    double s;
    double tmp[64];

    for (i=0; i<8; i++)
        for (j=0; j<8; j++)
        {
            s = 0.0;

            for (k=0; k<8; k++)
                s += coeff[j][k] * block[8*i+k];

            tmp[8*i+j] = s;
        }

    for (j=0; j<8; j++)
        for (i=0; i<8; i++)
        {
            s = 0.0;

            for (k=0; k<8; k++)
                s += coeff[i][k] * tmp[8*k+j];

            block[8*i+j] = (int)floor(s+0.5);
        }
}

/****************************************************************/
/*  RGB to Dct                                                  */
/****************************************************************/

static void rgbtoycbcr(DCTCOORD *rgb, DCTCOORD *ycbcr)
{
    int Red;
    int Green;
    int Blue;
    int Y;
    int Cb;
    int Cr;

    Red   = rgb->x;
    Green = rgb->y;
    Blue  = rgb->z;

    Y  =  0.2990*Red+0.5870*Green+0.1140*Blue;
    Cb = -0.1687*Red-0.3313*Green+0.5000*Blue+128;
    Cr =  0.5000*Red-0.4187*Green-0.0813*Blue+128;

    ycbcr->x = gmax(0,gmin(255,Y));
    ycbcr->y = gmax(0,gmin(255,Cb));
    ycbcr->z = gmax(0,gmin(255,Cr));
}

static int DCQuant(int v,int q)
{
    if (v>0)
    {
        v = (v+q/2)/q;
    }
    v = gmax(0,gmin(255,v));
    return(v);
}

static int ACQuant(int v,int q)
{
    if (v<0)
    {
        v = -((-v+q/2)/q);
    }
    else if (v>0)
    {
        v = (v+q/2)/q;
    }
    v = gmax(-128,gmin(127,v));
    return(v);
}

#define ZIG(a,b,c,d,e,f,g,h) zo[0]=a; zo[1]=b; zo[2]=c; zo[3]=d; zo[4]=e; zo[5]=f; zo[6]=g; zo[7]=h; zo+=8


void initzigorder(int *zo)
{
    ZIG( 0, 1, 5, 6,14,15,27,28);
    ZIG( 2, 4, 7,13,16,26,29,42);
    ZIG( 3, 8,12,17,25,30,41,43);
    ZIG( 9,11,18,24,31,40,44,53);
    ZIG(10,19,23,32,39,45,52,54);
    ZIG(20,22,33,38,46,51,55,60);
    ZIG(21,34,37,47,50,56,59,61);
    ZIG(35,36,48,49,57,58,62,63);
}

void DCTCompress(int *quant,unsigned char *dctdata,unsigned char *AYCbCr,int width,int height,int step)
{
    int bx;
    int by;
    int x;
    int y;
    int prevdc=128;
    int dc;
    int q;
    unsigned char *s;
    short *d;
    short block[8*8];
    int zigorder[8*8];
    double coeff[8][8]; /* transform coefficients */

    initzigorder(zigorder);
    initfdct(coeff);

    for (by=0; by<height; by+=step*8)
    {
        for (bx=0; bx<width; bx+=step*8)
        {
            /* copy to 8x8 block of shorts */

            d = block;
            for (y=0; y<8; ++y)
            {
                s = AYCbCr+((by+y*step)*width+bx)*4;
                for (x=0; x<8; ++x)
                {
                    *d++ = *s;
                    s += step*4;
                }
            }

            /* forward DCT */

            fdct(coeff,block);

            /* quantize */

            for (y=0; y<8; ++y)
            {
                for (x=0; x<8; ++x)
                {
                    q = quant[y*8+x];
                    if (x==0 && y==0)
                        block[y*8+x] = DCQuant(block[y*8+x],q);
                    else
                        block[y*8+x] = ACQuant(block[y*8+x],q);
                }
            }

            /* delta encode AC component */

            dc = block[0];
            block[0] = (dc-prevdc);
            prevdc = dc;

            /* zigzag output */

            for (y=0; y<8; ++y)
            {
                for (x=0; x<8; ++x)
                {
                    dctdata[zigorder[y*8+x]] = block[y*8+x];
                }
            }
            dctdata += 64;
        }
    }

    return;
}

ARGB *ConvertToACbCr(char *source,int gwidth,int gheight,int rowbytes,int channels,int width,int height)
{
    ARGB *AYCbCr;
    ARGB *s;
    ARGB *d;
    DCTCOORD rgb;
    DCTCOORD yuv;
    ARGB black;
    int bmsize;
    int x;
    int y;

    /* convert entire frame to AYCbCr */

    rgb.x = 0;
    rgb.y = 0;
    rgb.z = 0;
    rgbtoycbcr(&rgb,&yuv);
    black.a = 0;
    black.r = yuv.x;
    black.g = yuv.y;
    black.b = yuv.z;

    AYCbCr = galloc(width*height*4);
    if (AYCbCr)
    {
        bmsize = width*height;
        d = (ARGB *) AYCbCr;

        for (y=0; y<gheight; ++y)
        {
            s = (ARGB *) (source+y*rowbytes);
            for (x=0; x<gwidth; ++x)
            {
                if (channels==1)
                {
                    d->r = *(unsigned char *) s;
                    ++d;
                    s = (ARGB *)((char *) s + 1);
                }
                else
                {
                    rgb.x = s->r;
                    rgb.y = s->g;
                    rgb.z = s->b;
                    rgbtoycbcr(&rgb,&yuv);
                    d->a = s->a;
                    d->r = yuv.x;
                    d->g = yuv.y;
                    d->b = yuv.z;
                    ++s;
                    ++d;
                }
            }
            for (x=gwidth; x<width; ++x)
                *d++ = black;
        }
        for (y=0; y<(height-gheight)*width; ++y)
            *d++ = black;
    }
    return(AYCbCr);
}

/* chrominance subsample width and height to 1\2 */

void SubSample(unsigned char *source, int width, int height)
{
    int x;
    int y;
    unsigned char *s;

    s = source;

    for (y=0; y<height; y+=2)
    {
        for (x=0; x<width; x+=2)
        {
            *s = ((int)s[0]       + (int)s[1*4]
                + (int)s[width*4] + (int)s[width*4+1*4])/4;
            s += 8; /* step by 2 pixels */
        }
        s += width*4;   /* skip a row */
    }
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL DCT_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL DCT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok;
    int quality;
    int width;
    int height;
    int channels;
    char *dctdata;
    ARGB *AYCbCr;
    DCTHeaderStruct dctheader;
    int quant[8*8];

    initquant(quant,ginfo->quality);

    width =  (ginfo->width+15)&~15; /* 16 due to 8x8 blocks and chrominance subsampling */
    height = (ginfo->height+15)&~15;
    channels = 3;
    if (ginfo->originalbpp<=8)        channels = 1;
    else if (ginfo->originalbpp>24)   channels = 4;

    /* DCT FILE HEADER structure 16 bytes */

    memset(&dctheader,0,sizeof(dctheader));
    DCTPUTFIELD(dctheader.chunkid, GMAKEID('D','C','T','y'));       /* chunk id 'DCTy' */
    DCTPUTFIELD(dctheader.chunksize, width*height*3*96/192+16);     /* size of chunk, including header */
    DCTPUTFIELD(dctheader.framewidth, ginfo->width);                /* exact image width in pixels */
    DCTPUTFIELD(dctheader.frameheight, ginfo->height);              /* exact image height in pixels */
    DCTPUTFIELD(dctheader.quality, ginfo->quality);                 /* quality setting 0..100 */
    DCTPUTFIELD(dctheader.channels, channels);                      /* number of channels 1,3,4 */

    ok = gwrite(gx->gstream, (char *) &dctheader, 16);

    if (ok)
    {
        ok = 0;
        AYCbCr = ConvertToACbCr(source,ginfo->width,ginfo->height,rowbytes,channels,width,height);
        if (AYCbCr)
        {
            dctdata = galloc(width*height);
            if (dctdata)
            {
                DCTCompress(quant,(unsigned char*)dctdata,&AYCbCr->r,width,height,1); /* Y plane */
                ok = gwrite(gx->gstream, dctdata, width*height);

                if (channels>1 && ok)
                {
                    SubSample(&AYCbCr->g,width,height); /* Cb subsample */
                    SubSample(&AYCbCr->b,width,height); /* Cr subsample */

                    DCTCompress(quant,(unsigned char*)dctdata,&AYCbCr->g,width,height,2); /* Cb plane */
                    ok = gwrite(gx->gstream, dctdata, width*height/4);

                    DCTCompress(quant,(unsigned char*)dctdata,&AYCbCr->b,width,height,2); /* Cr plane */
                    ok = gwrite(gx->gstream, dctdata, width*height/4);

                    if (channels>3 && ok)
                    {
                        DCTCompress(quant,(unsigned char*)dctdata,&AYCbCr->a,width,height,1); /* A plane */
                        ok = gwrite(gx->gstream, dctdata, width*height);
                    }
                }

                gfree(dctdata);
            }
            gfree(AYCbCr);
        }
    }

    return(ok);
}

int GCALL DCT_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


