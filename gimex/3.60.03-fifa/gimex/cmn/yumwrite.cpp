// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "yumgimex.h"

/****************************************************************/
/*  RGB to YUM                                                  */
/****************************************************************/

static void rgbtoyum(YUMCOORD *rgb, YUMCOORD *yum)
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

    yum->x = Y;
    yum->y = gmax(0,gmin(255,Cb+128));
    yum->z = gmax(0,gmin(255,Cr+128));
}

static int YUMQuant(int v,int q)
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

static unsigned char *YEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3, int quant, int width)
{
    YUMCOORD rgb;
    YUMCOORD yum[4];

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyum(&rgb,&yum[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyum(&rgb,&yum[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyum(&rgb,&yum[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyum(&rgb,&yum[3]);

    d[0] = gmax(0,gmin(255,YUMQuant(yum[0].x,quant)));
    d[1] = gmax(0,gmin(255,YUMQuant(yum[1].x,quant)));

    d[width+0] = gmax(0,gmin(255,YUMQuant(yum[2].x,quant)));
    d[width+1] = gmax(0,gmin(255,YUMQuant(yum[3].x,quant)));

    return(d+2);
}

static unsigned char *CbEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3, int quant)
{
    YUMCOORD rgb;
    YUMCOORD yum[4];
    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyum(&rgb,&yum[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyum(&rgb,&yum[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyum(&rgb,&yum[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyum(&rgb,&yum[3]);

    {
        int p = (yum[0].y+yum[1].y+yum[2].y+yum[3].y+2)/4;
        *d++ = gmax(0,gmin(255,p));
    }

    return(d);
}

static unsigned char *CrEncode(unsigned char *d,unsigned char *p0,unsigned char *p1,unsigned char *p2,unsigned char *p3,int quant)
{
    YUMCOORD rgb;
    YUMCOORD yum[4];

    rgb.x = ((ARGB *)p0)->r;  /* r */
    rgb.y = ((ARGB *)p0)->g;  /* g */
    rgb.z = ((ARGB *)p0)->b;  /* b */
    rgbtoyum(&rgb,&yum[0]);

    rgb.x = ((ARGB *)p1)->r;  /* r */
    rgb.y = ((ARGB *)p1)->g;  /* g */
    rgb.z = ((ARGB *)p1)->b;  /* b */
    rgbtoyum(&rgb,&yum[1]);

    rgb.x = ((ARGB *)p2)->r;  /* r */
    rgb.y = ((ARGB *)p2)->g;  /* g */
    rgb.z = ((ARGB *)p2)->b;  /* b */
    rgbtoyum(&rgb,&yum[2]);

    rgb.x = ((ARGB *)p3)->r;  /* r */
    rgb.y = ((ARGB *)p3)->g;  /* g */
    rgb.z = ((ARGB *)p3)->b;  /* b */
    rgbtoyum(&rgb,&yum[3]);

    {
        int p = (yum[0].z+yum[1].z+yum[2].z+yum[3].z+2)/4;
        *d++ = gmax(0,gmin(255,p));
    }

    return(d);
}

static void YUMfile(int width, int height, char *infileptr, int rowbytes, char *outfileptr, int YQuant, int CbQuant, int CrQuant)
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
            d = YEncode(d,s1,s1+4,s2,s2+4,YQuant,width);
            s1 += 2*4;
            s2 += 2*4;
        }
        d+= width;
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

static void YUY2file(int width, int height, char *infileptr, int rowbytes, char *outfileptr, int YQuant, int CbQuant, int CrQuant)
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
            d = YEncode(d,s1,s1+4,s2,s2+4,YQuant,width);
            s1 += 2*4;
            s2 += 2*4;
        }
        d+= width;
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

int GCALL YUM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL YUM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    unsigned char yumheader[16];
    char *outfile;
    int flen;
    int ok=0;
    int quality;
    int YQuant;
    int CbQuant;
    int CrQuant;
    int width;
    int height;
    int isintel=ginfo->subtype;
    char *d;

    if (ginfo->subtype==1 && ginfo->bpp>8)
    {
        width =  (ginfo->width)/2*2;
        height = (ginfo->height);

        d = (char*)yumheader;
        memset(yumheader,0,sizeof(yumheader));
        if (isintel)
        {
            gputm(d, 0x68, 1); d += 4;  // shape type
            gputi(d, width, 2); d += 2;
            gputi(d, height, 2); d += 2;
        }
        else
        {
            gputm(d, 0x1d, 1); d += 4;  // shape type
            gputm(d, width, 2); d += 2;
            gputm(d, height, 2); d += 2;
        }

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

        ok = gwrite(gx->gstream,yumheader,16);
        if (ok)
        {
            flen = width*height*2;
            outfile = (char *) galloc(flen);
            if (outfile)
            {
                YUY2file(width, height, source, rowbytes, outfile,YQuant,CbQuant,CrQuant);
                ok = gwrite(gx->gstream,outfile,flen);
                gfree(outfile);
            }
        }
    }
    else if (ginfo->bpp>8)
    {
        width =  (ginfo->width)/2*2;
        height = (ginfo->height)/2*2;

        d = (char*)yumheader;
        memset(yumheader,0,sizeof(yumheader));
        if (isintel)
        {
            gputm(d, 0x6c, 1); d += 4;  // shape type
            gputi(d, width, 2); d += 2;
            gputi(d, height, 2); d += 2;
        }
        else
        {
            gputm(d, 0x1c, 1); d += 4;  // shape type
            gputm(d, width, 2); d += 2;
            gputm(d, height, 2); d += 2;
        }

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

        ok = gwrite(gx->gstream,yumheader,16);
        if (ok)
        {
            flen = width*height*3/2;
            outfile = (char *) galloc(flen);
            if (outfile)
            {
                YUMfile(width, height, source, rowbytes, outfile,YQuant,CbQuant,CrQuant);
                ok = gwrite(gx->gstream,outfile,flen);
                gfree(outfile);
            }
        }
    }

    return(ok);
}

int GCALL YUM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


