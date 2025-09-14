// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*       Psnr - Composite Images Together (various methods)         */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951114 FB based on crop and gx composite                 */
/*    1.01 951119 FB single image psnr with background colour       */
/*    1.02 960112 FB mask to allow channel psnring                  */
/*    1.03 960129 FB add x/y of new ginfo (adjust position bug)     */
/*    1.04 960709 FB 8 bit transparent psnr                         */
/*    1.05 960726 FB paste palette into background                  */
/*    1.06 960730 FB 8 bit psnr use foreground ginfo, not dest      */
/*    1.07 960731 FB channel mask (for clut or pixel)               */
/*    1.08 970226 FB palette psnr method                            */
/*    1.09 970925 GS need to pass blened ginfo when adding palettes */
/*    1.10 971030 KO uses the amount0/1 options to scale src/dst    */
/*    1.11 991209 FB use source ginfo->colourtbl for transparent    */
/*    1.12 991220 FB psnr                                           */
/*    1.13 000509 FB unattenuated                                   */
/*    1.14 000616 FB multiple,screen,overlay                        */
/*    1.15 000721 FB psnr use da,dr,dg,db vars                      */
/*    1.16 000922 FB added defines to header for methods            */
/*    1.17 010409 FB numcolours = max of 2 palettes                 */
/*    1.18 010705 FB totalpsnr over multiple images                 */
/*    1.19 020821 FB psnrY based on luma                            */
/*    1.20 030207 FB alphatest psnr method 10                       */
/*    1.21 030312 FB 32 bit internals, mapping to 8 on output       */
/*    1.22 030423 FB PSNRpsnrYtotal                                 */
/*    1.23 030509 FB Y = 0.299 * R + 0.587 * G + 0.114 * B          */
/*    1.24 030514 FB branched off fblend                            */
/*    1.25 030609 FB psnr function internally reused                */
/*    1.26 030619 FB alpha attenuate Y for PSNRY                    */
/*    1.27 030707 FB compare smaller of images                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* psnr()                                                           */
/*                                                                  */
/* Psnr compares two source images using PSNR                       */
/*                                                                  */
/* PSNR calculates Peak Signal to Noise Ratio                       */
/*                                                                  */
/* PSNRY calculation comes from a page comparing dxt                */
/* http://www.acm.org/sigs/sigmm/MM2000/ep/levkovich/               */
/* Original formula Y = 0.3   * R + 0.59  * G + 0.11  * B           */
/* Current formula  Y = 0.299 * R + 0.587 * G + 0.114 * B           */
/*                                                                  */
/* 601 standard is                                                  */
//static void RGBtoYUV(float &Y, float &U, float &V, float R, float G, float B)
//{
//    Y = (float)(( 0.25678843 * R + 0.50412965 * G + 0.09790586 * B) +  16.0f);
//    U = (float)((-0.14822295 * R - 0.29099280 * G + 0.43921576 * B) + 128.0f);
//    V = (float)(( 0.43921557 * R - 0.36778826 * G - 0.07142731 * B) + 128.0f);
//}
/* psnr4avi uses the current formula, which comes from YUY2         */
// http://mplayerhq.hu/DOCS/tech/colorspaces.txt
/*                                                                  */
/* PSNR standards document                                          */
// T1.TR.PP.74-2001
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fpsnr.h"
#include <string.h>
#include <math.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static __inline int SNPIXEL(int da,int sa)
{
    int p;
    p = da-sa;
    p = p*p;
    return(p);
}

static __inline float SNPIXELF(float da,float sa)
{
    float p;
    p = da-sa;
    p = p*p;
    return(p);
}


static float RGBtoY(int R, int G, int B, int A)
{
    return (float) (( 0.299f * R + 0.587f * G + 0.114f * B ) * A / 255.0f);
}

static unsigned int PSNRgetpixel(int *inflag,GBITMAP *source, GINFO *destginfo, int x, int y, unsigned int colour)
{
    int sx;
    int sy;
    int bpp=source->ginfo->bpp;
    unsigned char *s;

    sx = x + destginfo->defaultx - source->ginfo->defaultx;
    sy = y + destginfo->defaulty - source->ginfo->defaulty;

    *inflag = 0;
    if (sx>=0 && sx<source->ginfo->width
     && sy>=0 && sy<source->ginfo->height)
    {
        s = (unsigned char *)(source->image)+sy*source->rowbytes+sx*bpp/8;
        if (bpp==8)
            colour = (unsigned int) *s;
        else
        {
            colour = * (unsigned int *) s;
        }
        *inflag = 1;
    }
    return colour;
}

static int PSNRoverlay(int s, int d)
{
    int v;

    if (d<=128)
        v = gmin(255,(s*d+63)/128);
    else
        v = 255-(255-(s-128)*2)*(255-d)/255;

    return(v);
}

// overlay dp background with percentage of sp foreground

// return values
double PSNRpsnr;
double PSNRpsnrtotal;
double PSNRpsnrY;
double PSNRpsnrYtotal;
double PSNRpsnrMY;
double PSNRpsnrMYtotal;

// accumulators for multiple images - zero to reset
double PSNRpsnrtotalpixels=0.0;
double PSNRpsnrtotalsum=0.0;
double PSNRpsnrYtotalpixels=0.0;
double PSNRpsnrYtotalsum=0.0;
double PSNRpsnrMYtotalpixels=0.0;
double PSNRpsnrMYtotalsum=0.0;

void PSNR_save(double s[6])
{
    s[0] = PSNRpsnrtotalpixels;
    s[1] = PSNRpsnrtotalsum;
    s[2] = PSNRpsnrYtotalpixels;
    s[3] = PSNRpsnrYtotalsum;
    s[4] = PSNRpsnrMYtotalpixels;
    s[5] = PSNRpsnrMYtotalsum;
}

void PSNR_restore(double s[6])
{
    PSNRpsnrtotalpixels   = s[0]; 
    PSNRpsnrtotalsum      = s[1]; 
    PSNRpsnrYtotalpixels  = s[2]; 
    PSNRpsnrYtotalsum     = s[3]; 
    PSNRpsnrMYtotalpixels = s[4]; 
    PSNRpsnrMYtotalsum    = s[5]; 
}

// internals
static double PSNRpsnrsum;
static double PSNRpsnrYsum;
static double PSNRpsnrMYsum;
static int PSNRpsnrchannels;

static void PSNR_pixels(int width, int height,
        const char *s0, int rowbytes0, int bpp0, ARGB *colourtbl0,
        const char *s1, int rowbytes1, int bpp1, ARGB *colourtbl1)
{
    double YSum0=0.0;
    double YSum1=0.0;
    for (int y=0; y<height; ++y)
    {
        const char *p0 = s0+y*rowbytes0;
        const char *p1 = s1+y*rowbytes1;
        for (int x=0; x<width; ++x)
        {
            const ARGB *a0=(bpp0<=8)?colourtbl0+*(unsigned char *)p0:(const ARGB *)p0;
            const ARGB *a1=(bpp1<=8)?colourtbl1+*(unsigned char *)p1:(const ARGB *)p1;

            int a = SNPIXEL(a0->a,a1->a);
            int r = SNPIXEL(a0->r,a1->r);
            int g = SNPIXEL(a0->g,a1->g);
            int b = SNPIXEL(a0->b,a1->b);

            if (PSNRpsnrchannels < 4)
                a = 0;

            float y0 = RGBtoY(a0->r,a0->g,a0->b,a0->a);
            float y1 = RGBtoY(a1->r,a1->g,a1->b,a1->a);

            YSum0 += y0;
            YSum1 += y1;
            PSNRpsnrYsum += (double) SNPIXELF(y0,y1);

            if (PSNRpsnrchannels == 4)
                PSNRpsnrsum += (double) (a+r+g+b);
            else
                PSNRpsnrsum += (double) (r+g+b);

            p0 += bpp0/8;
            p1 += bpp1/8;
        }
    }
    PSNRpsnrMYsum += (double) SNPIXELF((float)YSum0/(width*height),(float)YSum1/(width*height))*width*height;
}

// sum is sum of square errors
// sample count is width*height*channels

static double psnr(double sum, double samplecount)
{
    const double MAXPSNR=128.0;
    const double MINSUM=pow(255.0/pow(10.0,MAXPSNR/20.0),2.0); // 1.0305767983978390437526664180478e-8;  // was 0.00000001

    double MSE = sum/samplecount;

    if (MSE < MINSUM)
        MSE = MINSUM;

    double psnr = -20.0 * log10(sqrt(MSE)/255.0);

    return psnr;
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutpsnr(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('P','S','N','R');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         2;
        about->filterclass =        FCLASS_COMPOSITE;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.27");
        strcpy(about->namestr,      "Psnr");
        strcpy(about->longnamestr,  "Psnr");
        about->numoptions =         9;
    }
    return(about);
}

FPUBLIC int openpsnr(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;
    }
    *filterInstance = fx;
    return(ok);
}

FPUBLIC GINFO *infopsnr(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

    }
    return ginfo;
}

FPUBLIC int filterpsnr(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int image0=0;
    int image1=(fx->numsources>1)?1:0;

    PSNRpsnrsum = 0.0;
    PSNRpsnrYsum = 0.0;
    PSNRpsnrMYsum = 0.0;
    PSNRpsnrchannels = 3;

    if (fx->source[image0].ginfo->alphabits
     || fx->source[image1].ginfo->alphabits)
        PSNRpsnrchannels = 4;

    int width  = gmin(fx->source[image0].ginfo->width,  fx->source[image1].ginfo->width);
    int height = gmin(fx->source[image0].ginfo->height, fx->source[image1].ginfo->height);

    PSNR_pixels(width,height,
        (const char *)fx->source[image0].image,fx->source[image0].rowbytes,fx->source[image0].ginfo->bpp,fx->source[image0].ginfo->colourtbl,
        (const char *)fx->source[image1].image,fx->source[image1].rowbytes,fx->source[image1].ginfo->bpp,fx->source[image1].ginfo->colourtbl);

    {
        PSNRpsnr = psnr(PSNRpsnrsum,ginfo->width*ginfo->height*PSNRpsnrchannels);

        PSNRpsnrtotalsum += PSNRpsnrsum;
        PSNRpsnrtotalpixels += (double) (ginfo->width*ginfo->height*PSNRpsnrchannels);
        PSNRpsnrtotal = psnr(PSNRpsnrtotalsum,PSNRpsnrtotalpixels);

        PSNRpsnrY = psnr(PSNRpsnrYsum,ginfo->width*ginfo->height);

        PSNRpsnrYtotalsum += PSNRpsnrYsum;
        PSNRpsnrYtotalpixels += (double) (ginfo->width*ginfo->height);
        PSNRpsnrYtotal = psnr(PSNRpsnrYtotalsum,PSNRpsnrYtotalpixels);

        PSNRpsnrMY = psnr(PSNRpsnrYsum-PSNRpsnrMYsum,ginfo->width*ginfo->height);

        PSNRpsnrMYtotalsum += PSNRpsnrMYsum;
        PSNRpsnrMYtotalpixels += (double) (ginfo->width*ginfo->height);
        PSNRpsnrMYtotal = psnr(PSNRpsnrYtotalsum-PSNRpsnrMYtotalsum,PSNRpsnrMYtotalpixels);

    }
    return(1);
}

FPUBLIC int closepsnr(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

