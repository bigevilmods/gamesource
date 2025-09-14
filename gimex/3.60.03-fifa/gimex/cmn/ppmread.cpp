// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 PPM - Portable Pixel Map Gimex                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940917 FB Based on HSI Raw and Targa                     */
/*    1.01 950529 FB ginstance signature                            */
/*    1.02 951218 FB export put newlines in header; no nl on pix 0  */
/*    1.03 961015 FB export dont put space followed by nl           */
/*    1.04 961018 FB import 1 and 8 bit grey scale                  */
/*    1.05 970327 FB export set ok return for gwrite                */
/*    1.06 010305 FB CR/LF ok and YUV extension P7                  */
/*    1.07 010307 FB careful about CR/LF just before bin data       */
/*    1.08 010311 FB yuv correct colours, wrong brightness          */
/*    1.09 010313 FB yuv export to confirm YUV                      */
/*    1.10 020227 FB leading spaces ok in ascii. comment cr lf      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1(bw),8(grey),24 bit, YUY2 (P7 extension)                        */
/* Reentrant                                                        */
/* Files: ppmread.cpp ppmwrite.cpp ppmgimex.h                       */
/*                                                                  */
/* Bits per channel set based on log of maximum colour component.   */
/* Binary format is limited to 8 bits per channel, but ascii is not */
/* PPM export on Photoshop mac only support PPM 24 bit.             */
/* PPM export on Alchemy supports 1,8,24 binary only.               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* 1,8,24 bit                                                       */
/* YUV support added as an extension                                */
/* Format support grey scale and black and white, but no indexed    */
/* colour modes.                                                    */
/* Pixels can be written in Binary or Ascii.                        */
/*                                                                  */
/* Nintendo provided                                                */
/* r = (1164*((s32)pin[i].y -  16) +                                */
/*      1596*((s32)pin[i].v - 128) + 500)/1000;                     */
/* g = (1164*((s32)pin[i].y -  16) -                                */
/*      813*((s32)pin[i].v - 128) -                                 */
/*      391*((s32)pin[i].u - 128) + 500)/1000;                      */
/* b = (1164*((s32)pin[i].y -  16) +                                */
/*      2018*((s32)pin[i].u - 128) + 500)/1000;                     */
/*                                                                  */
/* standard, which is a bit off                                     */
/* a = 255;                // 1.142                                 */
/* r = gmax(0,gmin(255,(int)(1.142*(y0-16) + 1.596*cr)));           */
/* g = gmax(0,gmin(255,(int)(1.142*(y0-16) - 0.813*cr - 0.391*cb))) */
/* b = gmax(0,gmin(255,(int)(1.142*(y0-16)            + 2.018*cb))) */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "ppmgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int PPMreadval(GSTREAM *g, int asciimode)
{
    unsigned char buf[4];
    int val;
    int dig;
    int more;
    int ok=0;

    val = 0;
    do
    {
        more = 0;
        ok = gread(g,buf,1);
        if (ok)
        {
            if (asciimode)
            {
                if (buf[0]=='#')    /* comment to end of line */
                {
                    while (buf[0]!=0 && buf[0]!=10 && buf[0]!=13 && ok)
                        ok = gread(g,buf,1);

                    if (ok) // handle cr/lf
                    {
                        GPOS p = gtell(g);

                        more = 0;
                        ok = gread(g,buf+1,1);
                        if ((buf[0]==10 && buf[1]==13) || (buf[0]==13 && buf[1]==10) && ok)
                            more = 1;
                        if (!more)
                            gseek(g,p);
                    }

                    more = 1;
                }
                else
                {
                    dig = buf[0]-'0';
                    if ((unsigned char) dig <= (unsigned char) 9)
                    {
                        val = val*10+dig;
                        more = 1;
                    }
                }
// works cleaner, but slows it down
//                if (gtell(g)>=glen(g))
//                    more = 0;
            }
            else
                val = (int) buf[0];

        }
    } while (more);

    // strip only cr/lf
    if (asciimode==2 && ok)
    {
        GPOS p = gtell(g);

        more = 0;
        ok = gread(g,buf+1,1);
        if ((buf[0]==10 && buf[1]==13) || (buf[0]==13 && buf[1]==10) && ok)
            more = 1;
        if (!more)
            gseek(g,p);
    }
    // strip all white spaces
    if (asciimode==1 && ok)
    {
        GPOS p;
        do
        {
            p = gtell(g);
            more = 0;
            ok = gread(g,buf+1,1);
            if ((buf[1]==10 || buf[1]==13 || buf[1]==' ' || buf[1]=='\t') && ok)
                more = 1;
            ok = 1; // do not report error on end stripping... pick it up on next digit, if there is one
        } while (more);

        gseek(g,p);
    }

    if (!ok)
    {
        val = -1;
    }

    return(val);
}

/* intlog2: returns integer log to the base 2 of a number. */

static int PPMintlog2(unsigned int l)
{
    int i;

    i = 0;
    while (l)
    {
        l >>= 1;
        ++i;
    }
    return(i);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL PPM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','P','P','M');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('P','P','M',' ');   /* mac file system types used */
        strcpy(info->extensions[0], ".ppm");                /* extensions */
        strcpy(info->extensions[1], ".pbm");
        strcpy(info->extensions[2], ".pgm");
        strcpy(info->extensions[3], ".pnm");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.10");                /* version # */
        strcpy(info->shorttypestr,  "PPM");                 /* type */
        strcpy(info->wordtypestr,   "PPM");                 /* wordtype */
        strcpy(info->longtypestr,   "Portable Pixel Map");  /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* has 6 byte id */

int GCALL PPM_is(GSTREAM *g)
{
    unsigned char ppmheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,ppmheader,4))
    {
        if (ppmheader[0]=='P'
         && ppmheader[1]>='1'
         && ppmheader[1]<='6'
         && ppmheader[2]<=' ')
        {
            ok = 80;
        }

        if (ppmheader[0]=='P'   // YUV extension
         && ppmheader[1]=='7'
         && ppmheader[2]<=' ')
        {
            ok = 79;
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL PPM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','P','P','M');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL PPM_info(GINSTANCE *gx, int framenum)
{
    unsigned char buf[4];
    GINFO *ginfo=0;
    int i;
    int ppmtype;
    int width;
    int height;
    int maxval;
    int bpp;
    int bpc;
    int subtype;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream,buf,1))
    {
        ppmtype= PPMreadval(gx->gstream,1);

        int asciimode = (ppmtype<=3)?1:0;
        width  = PPMreadval(gx->gstream,1);
        height = PPMreadval(gx->gstream,1);
        maxval = PPMreadval(gx->gstream,2-asciimode);
        bpc = PPMintlog2(maxval);

        subtype = asciimode;   /* 0 for binary, 1 for ascii */
        if (ppmtype==7)
            subtype = 2;

        switch (ppmtype)
        {
            case 1:   bpp = 1;  break;
            case 2:   bpp = 8;  break;
            case 3:   bpp = 24; break;
            case 4:   bpp = 1;  break;
            case 5:   bpp = 8;  break;
            case 6:   bpp = 24; break;
            case 7:   bpp = 24; break;
            default:  bpp = 24; break;
        }
        if (bpp==1)
            bpc = 8;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','P','P','M');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;     /* Current frame */
            ginfo->width            = width;        /* Width of bitmap in pixels */
            ginfo->height           = height;       /* Height of bitmap in pixels */
            ginfo->bpp              = bpp>8?32:8;   /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;          /* Bits per pixel in original image (1 to 32) */
            ginfo->numcolours       = 0;            /* Number of colours in original indexed palette */
            ginfo->subtype          = subtype;      /* Internal format sub-type 0-default */
            ginfo->packed           = 0;            /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;            /* Number of bits in alpha channel */
            ginfo->redbits          = bpc;          /* Number of bits in red channel */
            ginfo->greenbits        = bpc;          /* Number of bits in grreen channel */
            ginfo->bluebits         = bpc;          /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            if (bpp==1)
            {
                ginfo->numcolours   = 2;            /* Number of colours in original indexed palette */
                for (i=0; i<2; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = i*255;
                    ginfo->colourtbl[i].g = i*255;
                    ginfo->colourtbl[i].b = i*255;
                }
            }
            else if (bpp<=8)
            {
                ginfo->numcolours   = 256;            /* Number of colours in original indexed palette */
                for (i=0; i<256; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = i;
                    ginfo->colourtbl[i].g = i;
                    ginfo->colourtbl[i].b = i;
                }
            }
        }
    }
    return(ginfo);
}

int GCALL PPM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    unsigned char buf[4];
    int ppmtype;
    int width;
    int height;
    int maxval;
    int bitx;
    int x;
    int y;
    int pr;
    int pg;
    int pb;
    int bpp=ginfo->originalbpp;
    int ok=0;
    unsigned char *d;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream,buf,1))
    {
        ppmtype= PPMreadval(gx->gstream,1);

        int asciimode = (ppmtype<=3)?1:0;
        width  = PPMreadval(gx->gstream,1);
        height = PPMreadval(gx->gstream,1);
        maxval = PPMreadval(gx->gstream,2-asciimode);

        ok = 1;
        for (y=0; y<ginfo->height && ok; ++y)
        {
            d = (unsigned char *) (dest+rowbytes*y);

            for (x=0; x<ginfo->width; ++x)
            {
                if (bpp==1)
                {
                    pr = PPMreadval(gx->gstream,asciimode);
                    if (asciimode)
                        *d++ = pr;
                    else
                    {
                        for (bitx=0; bitx<8 && x+bitx<ginfo->width; ++bitx)
                        {
                            *d++ = (pr>>7)&1;
                            pr = (pr<<1)&0xfe;
                        }
                        x += 8-1;
                    }
                }
                else if (bpp<=8)
                {
                    pr = PPMreadval(gx->gstream,asciimode)*255/maxval;
                    *d++ = pr;
                }
                // EA extension
                else if (ppmtype==7)
                {
                    int y0;
                    int y1;
                    int cb;
                    int cr;

                    y0 = PPMreadval(gx->gstream,asciimode)*255/maxval;
                    cb = PPMreadval(gx->gstream,asciimode)*255/maxval-128;
                    y1 = PPMreadval(gx->gstream,asciimode)*255/maxval;
                    cr = PPMreadval(gx->gstream,asciimode)*255/maxval-128;
                    pr = y0;

                    ((ARGB *)d)->a = 255;                // 1.164
                    ((ARGB *)d)->r = gmax(0,gmin(255,(int)( 1.164*(y0-16) + 1.596*cr )));
                    ((ARGB *)d)->g = gmax(0,gmin(255,(int)( 1.164*(y0-16) - 0.813*cr - 0.391*cb )));
                    ((ARGB *)d)->b = gmax(0,gmin(255,(int)( 1.164*(y0-16)            + 2.018*cb )));
                    d+=4;
                    ++x;

                    ((ARGB *)d)->a = 255;
                    ((ARGB *)d)->r = gmax(0,gmin(255,(int)( 1.164*(y1-16) + 1.596*cr )));
                    ((ARGB *)d)->g = gmax(0,gmin(255,(int)( 1.164*(y1-16) - 0.813*cr - 0.391*cb )));
                    ((ARGB *)d)->b = gmax(0,gmin(255,(int)( 1.164*(y1-16)            + 2.018*cb )));
                    d+=4;

                }
                else
                {
                    pr = PPMreadval(gx->gstream,asciimode)*255/maxval;
                    pg = PPMreadval(gx->gstream,asciimode)*255/maxval;
                    pb = PPMreadval(gx->gstream,asciimode)*255/maxval;
                    ((ARGB *)d)->a = 255;
                    ((ARGB *)d)->r = pr;
                    ((ARGB *)d)->g = pg;
                    ((ARGB *)d)->b = pb;
                    d+=4;
                }
                if (pr<0) ok = 0;
            }
        }
    }
    return(ok);
}

int GCALL PPM_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


