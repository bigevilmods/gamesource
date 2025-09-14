// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "yuvgimex.h"

/****************************************************************/
/*  RGB to YUV                                                  */
/****************************************************************/

static void rgbtoyuv(YUVCOORD *rgb, YUVCOORD *yuv)
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

    Y  = (int)( 0.2990f*Red+0.5870f*Green+0.1140f*Blue);
    Cb = (int)(-0.1687f*Red-0.3313f*Green+0.5000f*Blue);
    Cr = (int)( 0.5000f*Red-0.4187f*Green-0.0813f*Blue);

    yuv->x = Y;
    yuv->y = Cb;
    yuv->z = Cr;
}

static int YUVQuant(int v,int q)
{
    if (v<0)
    {
        v = -((-v+q/2)/q*q);
    }
    else if (v>0)
    {
        v = (v+q/2)/q*q;
    }
    return(v);
}

static unsigned char *YEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3, int quant)
{
    YUVCOORD rgb;
    YUVCOORD yuv[4];

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[3]);

    *d++ = gmax(0,gmin(255,YUVQuant(yuv[0].x,quant)));
    *d++ = gmax(0,gmin(255,YUVQuant(yuv[1].x,quant)));
    *d++ = gmax(0,gmin(255,YUVQuant(yuv[2].x,quant)));
    *d++ = gmax(0,gmin(255,YUVQuant(yuv[3].x,quant)));

    return(d);
}

static unsigned char *CbEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3, int quant)
{
    YUVCOORD rgb;
    YUVCOORD yuv[4];
    int uavg;
    int ucount;
    int uweight;

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[3]);

    uavg = ucount = 0;
    uweight = (yuv[0].x+50)/64; uavg += yuv[0].y*uweight; ucount += uweight;
    uweight = (yuv[1].x+50)/64; uavg += yuv[1].y*uweight; ucount += uweight;
    uweight = (yuv[2].x+50)/64; uavg += yuv[2].y*uweight; ucount += uweight;
    uweight = (yuv[3].x+50)/64; uavg += yuv[3].y*uweight; ucount += uweight;
    if (!ucount) ++ucount;
    uavg = uavg/ucount;

    *d++ = gmax(-128,gmin(127,YUVQuant(uavg,quant)));

    return(d);
}

static unsigned char *CrEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3,int quant)
{
    YUVCOORD rgb;
    YUVCOORD yuv[4];
    int uavg;
    int ucount;
    int uweight;

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyuv(&rgb,&yuv[3]);

/*    vavg = (yuv[0].z+yuv[1].z+yuv[2].z+yuv[3].z)\4; */

    uavg = ucount = 0;
    uweight = (yuv[0].x+50)/64; uavg += yuv[0].z*uweight; ucount += uweight;
    uweight = (yuv[1].x+50)/64; uavg += yuv[1].z*uweight; ucount += uweight;
    uweight = (yuv[2].x+50)/64; uavg += yuv[2].z*uweight; ucount += uweight;
    uweight = (yuv[3].x+50)/64; uavg += yuv[3].z*uweight; ucount += uweight;
    if (!ucount) ++ucount;
    uavg = uavg/ucount;


    *d++ = gmax(-128,gmin(127,YUVQuant(uavg,quant)));

    return(d);
}

static void YUVfile(int width, int height, char *infileptr, int rowbytes, char *outfileptr, int YQuant, int CbQuant, int CrQuant)
{
    int x;
    int y;
    unsigned char *s1;
    unsigned char *s2;
    unsigned char *d;

    d = (unsigned char *) outfileptr;

    width =  width;
    height = height;

    /* encode Y channel */
    for (y=0; y<height-1; y+=2)
    {
        s1 = (unsigned char*)infileptr+y*rowbytes;
        s2 = s1+rowbytes;
        for (x=0; x<width; x+=2)
        {
            d = YEncode(d,s1,s1+4,s2,s2+4,YQuant);
            s1 += 2*4;
            s2 += 2*4;
        }
    }

    /* encode Cb channel */

    for (y=0; y<height-1; y+=2)
    {
        s1 = (unsigned char*)infileptr+y*rowbytes;
        s2 = s1+rowbytes;
        for (x=0; x<width; x+=2)
        {
            d = CbEncode(d,s1,s1+4,s2,s2+4,CbQuant);
            s1 += 2*4;
            s2 += 2*4;
        }
    }

    /* encode Cr channel */

    for (y=0; y<height-1; y+=2)
    {
        s1 = (unsigned char*)infileptr+y*rowbytes;
        s2 = s1+rowbytes;
        for (x=0; x<width; x+=2)
        {
            d = CrEncode(d,s1,s1+4,s2,s2+4,CrQuant);
            s1 += 2*4;
            s2 += 2*4;
        }
    }
}



/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL YUV_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL YUV_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    unsigned char yuvheader[10];
    char *outfile;
    int flen;
    int ok=0;
    int quality;
    int YQuant;
    int CbQuant;
    int CrQuant;
    int width;
    int height;
    char *d;

    if (ginfo->bpp>8)
    {
        width =  (ginfo->width)/2*2;
        height = (ginfo->height)/2*2;

        d = (char*)yuvheader;
        gputm(d, 0xc8fb, 2); d += 2;
        gputm(d, width*height*3+18, 3); d += 3;
        gputm(d, 0, 1);  d += 1;
        gputm(d, width, 2); d += 2;
        gputm(d, height, 2); d += 2;

        quality = ginfo->quality;
        YQuant  = (100-quality)*7/100+1;   /* 1..8 */
        CbQuant = (100-quality)*12/100+2;  /* 2..14 */
        CrQuant = (100-quality)*17/100+4;  /* 4..21 */
        if (quality==100)
        {
            YQuant  = 1;
            CbQuant = 1;
            CrQuant = 1;
        }

        ok = gwrite(gx->gstream,yuvheader,10);
        if (ok)
        {
            flen = width*height*3/2;
            outfile = (char *) galloc(flen);
            if (outfile)
            {
                YUVfile(width, height, source, rowbytes, outfile,YQuant,CbQuant,CrQuant);
                ok = gwrite(gx->gstream,outfile,flen);
                gfree(outfile);
            }
        }
    }

    return(ok);
}

int GCALL YUV_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


