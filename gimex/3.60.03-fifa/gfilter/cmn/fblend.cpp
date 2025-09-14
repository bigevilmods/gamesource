// (c) 1995-2007 Electronic Arts Inc.

#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#endif

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*       Blend - Composite Images Together (various methods)        */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951114 FB based on crop and gx composite                 */
/*    1.01 951119 FB single image blend with background colour      */
/*    1.02 960112 FB mask to allow channel blending                 */
/*    1.03 960129 FB add x/y of new ginfo (adjust position bug)     */
/*    1.04 960709 FB 8 bit transparent blend                        */
/*    1.05 960726 FB paste palette into background                  */
/*    1.06 960730 FB 8 bit blend use foreground ginfo, not dest     */
/*    1.07 960731 FB channel mask (for clut or pixel)               */
/*    1.08 970226 FB palette blend method                           */
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
/*    1.20 030207 FB alphatest blend method 10                      */
/*    1.21 030312 FB 32 bit internals, mapping to 8 on output       */
/*    1.22 030423 FB BLENDpsnrYtotal                                */
/*    1.23 030509 FB Y = 0.299 * R + 0.587 * G + 0.114 * B          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* blend(method,colour,amount0,amount1,mask,palmethod)              */
/* int method;        blending method (see below)                   */
/* int colour;        fill areas outside original images with this  */
/* float amount0;     0..100 percent to apply from image 0          */
/* float amount1;     0..100 percent to apply from image 1          */
/* int mask;          channel mask 00=leave, ff=set                 */
/* int palmmethod;    blending method for palette (see below)       */
/*                                                                  */
/*                                                                  */
/* Methods                                                          */
/* 0 BLEND_COMPOSITE  blend over unattenuated aka Wallace (default) */
/* 1 BLEND_PASTE                                                    */
/* 2 BLEND_SUB                                                      */
/* 3 BLEND_ADD                                                      */
/* 4 BLEND_REMOVE                                                   */
/* 5 BLEND_PSNR                                                     */
/* 6 BLEND_PREALPHA alpha blend preattenuated                       */
/* 7 BLEND_MULTIPLY                                                 */
/* 8 BLEND_SCREEN                                                   */
/* 9 BLEND_OVERLAY                                                  */
/* 10 BLEND_ALPHATEST - stamp if alpha is non-zero. AKA tmaskt      */
/* 11 BLEND_ALPHA   alpha blend unattenuated                        */
/*                                                                  */
/* Palette Methods                                                  */
/* 0 BLEND_PALCOMBINE combine foreground into background where not transparent */
/* 1 BLEND_PALBACK    use background palette                        */
/* 2 BLEND_PALFORE    use foreground palette                        */
/*                                                                  */
/* Blend two source images using specified method, optionally       */
/* applying only a percentage the frames.                           */
/* The position of the images are based on the defultx, defaulty    */
/* coordinates, and the resulting image is the union of the         */
/* rectangles.  Areas that fall outside the rectangles are treated  */
/* as transparent, then blended with the background colour.         */
/* The first image is considered the background, and the second     */
/* the optional foreground                                          */
/* For 8 bit, the alpha in the palette is examined for transparency */
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
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fblend.h"
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

static unsigned int BLENDgetpixel(int *inflag,GBITMAP *source, GINFO *destginfo, int x, int y, unsigned int colour)
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

static int BLENDoverlay(int s, int d)
{
    int v;

    if (d<=128)
        v = gmin(255,(s*d+63)/128);
    else
        v = 255-(255-(s-128)*2)*(255-d)/255;

    return(v);
}

// overlay dp background with percentage of sp foreground

static unsigned int BLENDpixels(unsigned int dp,unsigned int sp,int sf,int method,double blendp,unsigned int mask,GINFO *dginfo,GINFO *sginfo)
{
    int a;
    int r;
    int g;
    int b;
    int da=((ARGB *)&dp)->a;
    int dr=((ARGB *)&dp)->r;
    int dg=((ARGB *)&dp)->g;
    int db=((ARGB *)&dp)->b;
    int sa=((ARGB *)&sp)->a;
    int sr=((ARGB *)&sp)->r;
    int sg=((ARGB *)&sp)->g;
    int sb=((ARGB *)&sp)->b;
    int ma=(mask>>24)&255;
    int mr=(mask>>16)&255;
    int mg=(mask>> 8)&255;
    int mb=(mask>> 0)&255;
    ARGB d;
    unsigned int p=dp;

    if (blendp!=100.0)
    {
        sa=(int)gmax(0,gmin(255,sa * blendp / 100.0));  // Scale the source by blendp
        sr=(int)gmax(0,gmin(255,sr * blendp / 100.0));  // Clamping to 0-255
        sg=(int)gmax(0,gmin(255,sg * blendp / 100.0));
        sb=(int)gmax(0,gmin(255,sb * blendp / 100.0));
    }

    if (dginfo->bpp==32)
    {
        switch (method)
        {
            default:

            case BLEND_COMPOSITE:
                a = 255-(255-sa)*(255-da)/255;
                if (a)
                {
                    r = gmin(255,(sa*sr+(255-sa)*da*dr/255)/a);
                    g = gmin(255,(sa*sg+(255-sa)*da*dg/255)/a);
                    b = gmin(255,(sa*sb+(255-sa)*da*db/255)/a);
                }
                else
                {
                    r = sr;
                    g = sg;
                    b = sb;
                }

                break;

            case BLEND_ALPHA:
                a = gmax(sa,da);
                r = gmin(255,(sa*sr+(255-sa)*dr)/255);
                g = gmin(255,(sa*sg+(255-sa)*dg)/255);
                b = gmin(255,(sa*sb+(255-sa)*db)/255);
                break;

            case BLEND_PASTE:
                if (sf)
                {
                    a = sa;
                    r = sr;
                    g = sg;
                    b = sb;
                }
                else
                {
                    a = da;
                    r = dr;
                    g = dg;
                    b = db;
                }
                break;

            case BLEND_SUB:
                a = da;
                r = gmax(0,dr-sr);
                g = gmax(0,dg-sg);
                b = gmax(0,db-sb);
                break;

            case BLEND_ADD:
                a = da;
                r = gmin(255,dr+sr);
                g = gmin(255,dg+sg);
                b = gmin(255,db+sb);
                break;

            case BLEND_REMOVE:
                a = gmax(0,da-sa);
                r = gmax(0,dr-sr);
                g = gmax(0,dg-sg);
                b = gmax(0,db-sb);
                break;

            case BLEND_PSNR:
                // removed to avoid globals. use psnr filter instead
                break;

            case BLEND_PREALPHA:
                a = gmax(sa,da);
                r = gmin(255,sr+(255-sa)*dr/255);
                g = gmin(255,sg+(255-sa)*dg/255);
                b = gmin(255,sb+(255-sa)*db/255);
                break;

            case BLEND_MULTIPLY:
                if (sf)
                {
                    a = gmax(sa,da);
                    r = (sr*dr/255) *sa/255+(255-sa)*dr/255;
                    g = (sg*dg/255) *sa/255+(255-sa)*dg/255;
                    b = (sb*db/255) *sa/255+(255-sa)*db/255;
                }
                else
                {
                    a = da;
                    r = dr;
                    g = dg;
                    b = db;
                }
                break;

            case BLEND_SCREEN:
                if (sf)
                {
                    a = gmax(sa,da);
                    r = (255-(255-sr)*(255-dr)/255) *sa/255+(255-sa)*dr/255;
                    g = (255-(255-sg)*(255-dg)/255) *sa/255+(255-sa)*dg/255;
                    b = (255-(255-sb)*(255-db)/255) *sa/255+(255-sa)*db/255;
                }
                else
                {
                    a = da;
                    r = dr;
                    g = dg;
                    b = db;
                }
                break;

            case BLEND_OVERLAY:
                if (sf)
                {
                    a = gmax(sa,da);
                    r = (BLENDoverlay(sr,dr)) *sa/255+(255-sa)*dr/255;
                    g = (BLENDoverlay(sg,dg)) *sa/255+(255-sa)*dg/255;
                    b = (BLENDoverlay(sb,db)) *sa/255+(255-sa)*db/255;
                }
                else
                {
                    a = da;
                    r = dr;
                    g = dg;
                    b = db;
                }
                break;

            case BLEND_ALPHATEST:
                if (sa)
                {
                    a = sa;
                    r = sr;
                    g = sg;
                    b = sb;
                }
                else
                {
                    a = da;
                    r = dr;
                    g = dg;
                    b = db;
                }
                break;
        }
        d.a = (unsigned char)(ma?a:da);
        d.r = (unsigned char)(mr?r:dr);
        d.g = (unsigned char)(mg?g:dg);
        d.b = (unsigned char)(mb?b:db);
        p = *(unsigned int *)&d;
    }
    else
    {
        sa = sginfo->colourtbl[sp&255].a;
        da = dginfo->colourtbl[dp&255].a;
        switch (method)
        {
            default:
            case BLEND_COMPOSITE:
            case BLEND_ALPHA:
            case BLEND_ALPHATEST:
                p = dp;
                if (sf && sa>=da)
                    p = sp;
                break;

            case BLEND_PASTE:
                if (sf)
                {
                    p = sp;
                }
                else
                {
                    p = dp;
                }
                break;

            case BLEND_SUB:
                p = (dp-sp)&255;
                break;

            case BLEND_ADD:
                p = (dp+sp)&255;
                break;

            case BLEND_REMOVE:
                p = gmax(0,(int)dp-(int)sp);
                break;
        }
    }
    return(p);
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutblend(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('B','L','N','D');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         2;
        about->filterclass =        FCLASS_COMPOSITE;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.23");
        strcpy(about->namestr,      "Blend");
        strcpy(about->longnamestr,  "Composite Blend");
        about->numoptions =         6;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           10;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Method");
        strcpy(about->option[0].enumlist,  "Alpha,Paste,Sub,Add,Remove,psnr,prealpha,multiple,screen,overlay,tmask");

        about->option[1].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Colour");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_FLOAT;
        about->option[2].min.f =           -200.0;
        about->option[2].max.f =           200.0;
        about->option[2].def.f =           100.0;
        strcpy(about->option[2].name,      "Foreground Percentage");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_FLOAT;
        about->option[3].min.f =           -200.0;
        about->option[3].max.f =           200.0;
        about->option[3].def.f =           100.0;
        strcpy(about->option[3].name,      "Background Percentage");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_INT|FOP_TRUE_COLOR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0xffffffff;
        strcpy(about->option[4].name,      "Mask");
        strcpy(about->option[4].enumlist,  "");

        about->option[5].type =            FOP_INT;
        about->option[5].min.i =           0;
        about->option[5].max.i =           2;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "Palette Method");
        strcpy(about->option[5].enumlist,  "Combine,Background,Foreground");
    }
    return(about);
}

FPUBLIC int openblend(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
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

FPUBLIC GINFO *infoblend(FINSTANCE *fx, int framenum)
{
    int i;
    int x;
    int y;
    int x2;
    int y2;
    int dx;
    int dy;
    int ma;
    int mr;
    int mg;
    int mb;
    GINFO *ginfo;
    int image0=0;
    int image1=0;
    int blendm=0;
    unsigned int blendc=0;
    double blendf=100.0;
    double blendb=100.0;
    unsigned int blendmask=0xffffffff;
    int blendpm=0;
// PAULDING START
    int canvasw = 0;
    int canvash = 0;
// PAULDING END

    if (fx->option)
    {
        if (fx->numoptions>=1) blendm = fx->option[0].i;
        if (fx->numoptions>=2) blendc = fx->option[1].i;
        if (fx->numoptions>=3) blendf = fx->option[2].f;
        if (fx->numoptions>=4) blendb = fx->option[3].f;
        if (fx->numoptions>=5) blendmask = fx->option[4].i;
        if (fx->numoptions>=6) blendpm = fx->option[5].i;
// PAULDING START
        if (fx->numoptions>=7) canvasw = fx->option[6].i;
        if (fx->numoptions>=8) canvash = fx->option[7].i;
// PAULDING END
    }
    if (fx->numsources>1) image1 = 1;

    ma=(blendmask>>24)&255;
    mr=(blendmask>>16)&255;
    mg=(blendmask>> 8)&255;
    mb=(blendmask>> 0)&255;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        x =  gmin(fx->source[image0].ginfo->defaultx,fx->source[image1].ginfo->defaultx);
        y =  gmin(fx->source[image0].ginfo->defaulty,fx->source[image1].ginfo->defaulty);
        x2 = gmax(fx->source[image0].ginfo->defaultx+fx->source[image0].ginfo->width,fx->source[image1].ginfo->defaultx+fx->source[image1].ginfo->width);
        y2 = gmax(fx->source[image0].ginfo->defaulty+fx->source[image0].ginfo->height,fx->source[image1].ginfo->defaulty+fx->source[image1].ginfo->height);
        dx = x-ginfo->defaultx;
        dy = y-ginfo->defaulty;
// PAULDING START
        if(canvasw != 0 && canvash != 0)
        {
            ginfo->width  = canvasw;
            ginfo->height = canvash;
            ginfo->defaultx = 0;
            ginfo->defaulty = 0;
            ginfo->centerx  = canvasw / 2;
            ginfo->centery  = canvash / 2;
        }
        else
// PAULDING END
        {
            ginfo->width  = x2-x;
            ginfo->height = y2-y;
            ginfo->defaultx += dx;
            ginfo->defaulty += dy;
            ginfo->centerx  -= dx;
            ginfo->centery  -= dy;
        }

        ginfo->numcolours = gmax(ginfo->numcolours,fx->source[image1].ginfo->numcolours);

        for (i=0; i<ginfo->numhotspots; ++i)
        {
            ginfo->hotspottbl[i][0] -= dx;
            ginfo->hotspottbl[i][1] -= dy;
        }
        for (i=0; i<256; ++i)
        {
            switch (blendpm)
            {
                default:
                case BLEND_PALCOMBINE:
                    if (fx->source[image1].ginfo->colourtbl[i].a >= ginfo->colourtbl[i].a)
                    {
                        ginfo->colourtbl[i].a = (unsigned char)(ma?fx->source[image1].ginfo->colourtbl[i].a:ginfo->colourtbl[i].a);
                        ginfo->colourtbl[i].r = (unsigned char)(mr?fx->source[image1].ginfo->colourtbl[i].r:ginfo->colourtbl[i].r);
                        ginfo->colourtbl[i].g = (unsigned char)(mg?fx->source[image1].ginfo->colourtbl[i].g:ginfo->colourtbl[i].g);
                        ginfo->colourtbl[i].b = (unsigned char)(mb?fx->source[image1].ginfo->colourtbl[i].b:ginfo->colourtbl[i].b);
                    }
                    break;

                case BLEND_PALBACK:
                    {
                        ginfo->colourtbl[i].a = fx->source[image0].ginfo->colourtbl[i].a;
                        ginfo->colourtbl[i].r = fx->source[image0].ginfo->colourtbl[i].r;
                        ginfo->colourtbl[i].g = fx->source[image0].ginfo->colourtbl[i].g;
                        ginfo->colourtbl[i].b = fx->source[image0].ginfo->colourtbl[i].b;
                    }
                    break;

                case BLEND_PALFORE:
                    {
                        ginfo->colourtbl[i].a = fx->source[image1].ginfo->colourtbl[i].a;
                        ginfo->colourtbl[i].r = fx->source[image1].ginfo->colourtbl[i].r;
                        ginfo->colourtbl[i].g = fx->source[image1].ginfo->colourtbl[i].g;
                        ginfo->colourtbl[i].b = fx->source[image1].ginfo->colourtbl[i].b;
                    }
                    break;
            }
        }
    }
    return(ginfo);
}

FPUBLIC int filterblend(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int image0=0;
    int image1=0;
    int blendmback=0;
    int blendm=0;
    unsigned int blendc=0;
    double blend0=100.0;
    double blend1=100.0;
    unsigned int blendmask=0xffffffff;
    int blendpm=0;
// PAULDING START
    int canvasw = 0;
    int canvash = 0;
// PAULDING END
    GINFO* blend_ginfo = ginfo;

    if (fx->option)
    {
        if (fx->numoptions>=1) blendm = fx->option[0].i;
        if (fx->numoptions>=2) blendc = fx->option[1].i;
        if (fx->numoptions>=3) blend0 = fx->option[2].f;
        if (fx->numoptions>=4) blend1 = fx->option[3].f;
        if (fx->numoptions>=5) blendmask = fx->option[4].i;
        if (fx->numoptions>=6) blendpm = fx->option[5].i;
// PAULDING START
        if (fx->numoptions>=7) canvasw = fx->option[6].i;
        if (fx->numoptions>=8) canvash = fx->option[7].i;
// PAULDING END
    }
    if (fx->numsources>1) image1 = 1;

    blendmback = blendm;
    if (blendmback>=2)  // sub, add, remove, psnr
        blendmback = 1; // switch to paste for background

    // get correct ginfo to pass to blend function based on resulting color table that will be used
    if (blendpm == BLEND_PALBACK)
        blend_ginfo = fx->source[image0].ginfo;
    else if (blendpm == BLEND_PALFORE)
        blend_ginfo = fx->source[image1].ginfo;
    else if (blendpm == BLEND_PALCOMBINE)
        blend_ginfo = ginfo;

    // guard against bad parameters
    if (ginfo->bpp > 8)
        blend_ginfo = ginfo;

#if ENABLE_OMP
    #pragma omp parallel for
#endif
    for (int y=0; y<ginfo->height; ++y)
    {
        int sf;
        char *d = dest+y*rowbytes;
        for (int x=0; x<ginfo->width; ++x)
        {
            if (blendpm == BLEND_PALCOMBINE)
                blend_ginfo = fx->source[image0].ginfo;

            unsigned int p = blendc;
            unsigned int sp = BLENDgetpixel(&sf,&fx->source[image0],ginfo,x,y,0); // background
            p  = BLENDpixels(p,sp,sf,blendmback,blend0,0xffffffff,ginfo,blend_ginfo);  // blend image with background colour

            if (fx->numsources>1)
            {
                if (blendpm == BLEND_PALCOMBINE)
                    blend_ginfo = fx->source[image1].ginfo;
                sp = BLENDgetpixel(&sf,&fx->source[image1],ginfo,x,y,0); // foreground
                p  = BLENDpixels(p,sp,sf,blendm,blend1,blendmask,ginfo,blend_ginfo);   // blend foreground image
            }

            if (ginfo->bpp==8)                                       // store result
                *(unsigned char *)d = (unsigned char) p;
            else
                *(unsigned int *)d = p;

            d += ginfo->bpp/8;
        }
    }
    return(1);
}

FPUBLIC int closeblend(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

