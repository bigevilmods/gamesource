// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  TIM - PSX Texture IMage Gimex                   */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940730 FB First draft based on targatim and Targa gimex  */
/*    1.01 940808 FB Right side up, no clut header, 4, 8 & 24 work  */
/*    1.02 950428 FB Transluciency                                  */
/*    1.03 950529 FB ginstance signature                            */
/*    1.04 970506 FB transluciency assume semi-transparent on       */
/*    1.05 970602 GS fixed bug with write error checking            */
/*    1.06 000214 FB clutx/y follow PSH convention GINFOOLDSHP         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: timread.cpp timwrite.cpp timgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Litte endian fields                                              */
/* tim files start with a header block                              */
/* following it is a clut block (if TIM_CLUT_PRESENT is set)        */
/* at the end is a pixel block                                      */
/* TIM files support 4,8,16,24 bit depths.                          */
/* CLUT is 16 bit enties                                            */
/* Often has CLUT and is 16 bit.                                    */
/* No compression                                                   */
/* flags:                                                           */
/*   TIM_CLUT_PRESENT  0x08  specifies CLUT is in file              */
/*   TIM_16_COLORS     0x00  pixels are 16 color indexes (4 bit)    */
/*   TIM_256_COLORS    0x01  pixles are 256 color indexes (8 bit)   */
/*   TIM_15_BIT_RGB    0x02  pixels are 1-5-5-5 PSX direct colors   */
/*   TIM_24_BIT_RGB    0x03  pixels are 8-8-8 PSX direct colors     */
/*   TIM_MIXED_MODE    0x04  ????? 16 bit?                          */
/*   TIM_MODE_MASK     0x03  mask for determining pixel type        */
/*                                                                  */
/* Example Image:                                                   */
/* --------------                                                   */
/* 00000: 10000000 02000000 0c000000 00000000  ................     */
/* 00010: 00000000 ac5a0000 00000000 64007400  ....,Z......d.t.     */
/* 00020: 7b7b7b7b 7b7b7b7b 7b7b9c7f 9c7f9c7f  ................     */
/* 00030: 9c7f9c7f 9c7f9c7f 9c7f9c7f 9c7f9c7f  ................     */
/* 00040: 9c7f9c7f 9c7f9c7f 9c7f9c7f bd7f9c7f  ............=...     */
/* 00050: 7b7bf862 d662196f 196f196f 196f597b  ..xbVb.o.o.o.oY.     */
/* 00060: 597b3062 4a454941 49414941 49414941  Y.0bJEIAIAIAIAIA     */
/* 00070: 49414941 493d493d 493d493d 4a394a39  IAIAI=I=I=I=J9J9     */
/* 00080: 28350731 c628a524 841c841c 841ca420  (5.1F(%$......$      */
/* 00090: a524c528 c528c528 c528c528 c528c528  %$E(E(E(E(E(E(E(     */
/* 000a0: c528c528 c528c52c c52ce62c e62ce630  E(E(E(E,E,f,f,f0     */
/* 000b0: e52ce630 e52cc628 f0493062 d6621777  e,f0e,F(pI0bVb.w     */
/* 000c0: 597b5777 597b1777 1777597b 16731777  Y.WwY..w.wY..s.w     */
/* 000d0: 597b7b7b 9c7f9c7f 9c7f7b7b bb7f9c7f  Y...........;...     */
/* 000e0: 7b7b7b7b 7b7b597b 7b7b7b7b 9c7f9c7f  ......Y.........     */
/* 000f0: 9c7f9c7f 9c7f9c7f bd7fbd7f bd7fbd7f  ........=.=.=.=.     */
/* 00100: 9c7fbd7f 9c7fbd7f 9c7f9c7f bd7fbd7f  ..=...=.....=.=.     */
/* 00110: bd7fdd7f dd7fbd7f 7b7bf862 d6623b6f  =.].].=...xbVb;o     */
/*                                                                  */
/* 16 bit is intel reversed bytes and RGB ordered bits within.      */
/* 16 bit cluts are intel reversed byes and BGR ordered bits.       */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "timgimex.h"
#include "oldpshgimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

/* convert PSX 16 bit entry to 32 bit ARGB entry */
/* with transparency and tranluciency */

#if 0

/* old method - assume semi-transparent OFF */
static ARGB TIMpixel5To8(unsigned short aRgb5)
{
    ARGB    aRgb8;

    if (aRgb5&0x8000)   /* transluciency on */
    {
        if (aRgb5==0x8000)  /* black */
        {
            aRgb8.r = aRgb8.g = aRgb8.b = 0;
            aRgb8.a = 255;
        }
        else            /* 50% translucient */
        {
            aRgb8.b = (((aRgb5>>10)&31)*255+16)/31;
            aRgb8.g = (((aRgb5>>5 )&31)*255+16)/31;
            aRgb8.r = (((aRgb5>>0 )&31)*255+16)/31;
            aRgb8.a = 128;
        }
    }
    else
    {
        if (aRgb5==0)   /* transparent */
        {
            aRgb8.r = aRgb8.g = aRgb8.b = aRgb8.a = 0;
        }
        else            /* opaque */
        {
            aRgb8.b = (((aRgb5>>10)&31)*255+16)/31;
            aRgb8.g = (((aRgb5>>5 )&31)*255+16)/31;
            aRgb8.r = (((aRgb5>>0 )&31)*255+16)/31;
            aRgb8.a = 255;
        }
    }
    return(aRgb8);
}
#endif

static ARGB TIMpixel5To8(unsigned short aRgb5)
{
    ARGB    aRgb8;

    if (aRgb5&0x8000)   /* transluciency on */
    {
        aRgb8.b = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
        aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
        aRgb8.r = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
        aRgb8.a = (unsigned char)(128);
    }
    else
    {
        if (aRgb5==0)   /* transparent */
        {
            aRgb8.r = aRgb8.g = aRgb8.b = aRgb8.a = 0;
        }
        else            /* opaque */
        {
            aRgb8.b = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
            aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
            aRgb8.r = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
            aRgb8.a = (unsigned char)(255);
        }
    }
    return(aRgb8);
}

static int TIMreadline(GSTREAM *g,unsigned char *d,unsigned char *linebuf,GINFO *ginfo)
{
    int width;
    int bpp;
    int packed;
    int rowbytes;
    int ok=0;
    int i;
    unsigned char *s=linebuf;
    unsigned short rgb;

    width   = ginfo->width;
    bpp     = ginfo->originalbpp;
    packed  = ginfo->packed;

    rowbytes = ((width*bpp+15)&~15)>>3;
    ok = gread(g, s, rowbytes);

    switch (bpp)
    {
        case 4:     /* uncompressed 4 bit indexed image */
            for (i=0; i<width; ++i)
            {
                if (i&1)
                    *d++ = (unsigned char) (*s++>>4);
                else
                    *d++ = (unsigned char) (*s&15);
            }
            break;

        case 8:     /* uncompressed 8 bit indexed image */
            for (i=0; i<width; ++i)
            {
                *d++ = *s++;
            }
            break;

        case 15:    /* uncompressed 15 bit direct image */
        case 16:    /* uncompressed 16 bit direct image */
            for (i=0; i<width; ++i)
            {
                rgb = (unsigned short)ggeti(s,2);

                *(ARGB *)d = TIMpixel5To8(rgb);

                d += 4;
                s += 2;
            }
            break;

        case 24:    /* uncompressed 24 bit direct image */
            for (i=0; i<width; ++i)
            {
                ((ARGB *) d)->a = 0xff;
                ((ARGB *) d)->b = s[2];
                ((ARGB *) d)->g = s[1];
                ((ARGB *) d)->r = s[0];
                d += 4;
                s += 3;
            }
            break;


        case 32:    /* uncompressed 32 bit direct image */
            for (i=0; i<width; ++i)
            {
                ((ARGB *) d)->a = s[3];
                ((ARGB *) d)->r = s[2];
                ((ARGB *) d)->g = s[1];
                ((ARGB *) d)->b = s[0];
                d += 4;
                s += 4;
            }
            break;
    }
    return(ok);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL TIM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','T','I','M');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
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
        info->mactype[0]      = GMAKEID('.','T','I','M');   /* mac file system types used */
        strcpy(info->extensions[0], ".tim");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.05");                /* version # */
        strcpy(info->shorttypestr,  "TIM");                 /* type */
        strcpy(info->wordtypestr,   "TIM");              /* wordtype */
        strcpy(info->longtypestr,   "TIM PSX Texture Image"); /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* id 00000010 */

int GCALL TIM_is(GSTREAM *g)
{
    unsigned char timheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,timheader,4))
    {
        if (ggeti(timheader,4)==0x00000010)
            ok = 100;    /* 100% odds its a tim */
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL TIM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','T','I','M');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL TIM_info(GINSTANCE *gx, int framenum)
{
    TIMHeaderStruct timheader;
    TIMClutStruct   timclut;
    TIMPixelStruct  timpixel;
    unsigned char pal[256*4];   /* normally 256*2, but leave space incase 24 bit */
    GINFO *ginfo=0;
    int  bpp=0;
    int  colours;
    int  i;
    unsigned short rgb;
    int  width;
    int  height;
    int  colourbits;
    int  ok=0;
    int  defaultx;
    int  defaulty;
    int  alphabits;
    int  flags=0;
    int clutlen;

    memset(&timheader,0,sizeof(timheader));
    memset(&timclut,0,sizeof(timclut));
    memset(&timpixel,0,sizeof(timpixel));

    gseek(gx->gstream,(GPOS)0);
    ok = gread(gx->gstream, (char *) &timheader, 8);   /* read file header id/flags */
    if (ok)
    {
        flags = TIMGETFIELD(timheader.flags);           /* flags */

        clutlen = 0;
        if (flags&TIM_CLUT_PRESENT)
        {
            ok = gread(gx->gstream, (char *) &timclut, 12); /* read clut header */
            clutlen = TIMGETFIELD(timclut.length)-12;       /* size of clut chunk */
        }
        if (clutlen>0)
        {
            if (clutlen>1024)
                clutlen = 1024;
            ok = gread(gx->gstream, pal, clutlen);       /* read raw clut */
        }

        if (ok)
        {
            ok = gread(gx->gstream, (char *) &timpixel, 12); /* read pixel header */
        }
    }

    if (ok)
    {
        switch (flags&TIM_MODE_MASK)
        {
            case TIM_16_COLORS:  bpp = 4; break;
            case TIM_256_COLORS: bpp = 8; break;
            case TIM_15_BIT_RGB: bpp = 16; break;
            case TIM_24_BIT_RGB: bpp = 24; break;
            default: ;
        }
        width       = TIMGETFIELD(timpixel.width)*16/bpp;          /* Width of bitmap in pixels */
        height      = TIMGETFIELD(timpixel.height);                /* Height of bitmap in pixels */
        defaultx    = TIMGETFIELD(timpixel.x_pos);
        defaulty    = TIMGETFIELD(timpixel.y_pos);
        colours     = (TIMGETFIELD(timclut.length)-12)/2;
        colourbits  = 16;
        alphabits = 0;
        if (bpp>8) colours = 0;

        ginfo = (GINFO *) galloc(sizeof(GINFOOLDSHP));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFOOLDSHP));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','T','I','M'); /* Type of file */
            ginfo->size             = sizeof(GINFOOLDSHP);         /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
            ginfo->redbits          = 5;            /* Number of bits in red channel */
            ginfo->greenbits        = 5;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 5;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = defaultx;     /* Default Coordinate Point */
            ginfo->defaulty         = defaulty;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */

            /* 15,16,24,32 bpp true colour */
            if (bpp>8)
            {
                ginfo->bpp              = 32;       /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = bpp;
                ginfo->numcolours  = 0;        /* Number of colours in original indexed palette */
                if (bpp>16)
                    ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
            }
            /* 4 or 8 bit colour, with 16 bit palette */
            else
            {
                if (!colours)    colours = (1 << bpp);
                if (colours>256) colours = 256;

                ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = bpp;
                ginfo->numcolours  = colours;                  /* Number of colours in original indexed palette */

                ((GINFOOLDSHP *)ginfo)->clutx    = TIMGETFIELD(timclut.x_pos);
                ((GINFOOLDSHP *)ginfo)->cluty    = TIMGETFIELD(timclut.y_pos);

                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 5;
                ginfo->alphabits    = 2;
                for (i=0; i<colours; ++i)
                {
                    rgb = (unsigned short)ggeti(pal+i*2,2);
                    ginfo->colourtbl[i] = TIMpixel5To8(rgb);
                }
            }
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL TIM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    unsigned char *linebuf;
    unsigned char *linebase;
    int  y;
    int  width;
    int  height;
    int  bpp;
    int  colours;
    int  ok=0;
    GINFO *tmpinfo;

    tmpinfo = TIM_info(gx,0);
    if (tmpinfo)
    {
        gfree(tmpinfo);

        width   = ginfo->width;
        height  = ginfo->height;
        bpp     = ginfo->originalbpp;
        colours = ginfo->numcolours;

        linebase = (unsigned char *) dest;
        linebuf = (unsigned char *) galloc(((width*bpp+15)&~15)>>3);
        if (linebuf)
        {
            ok = 1;
            for (y=0; y<height && ok; ++y)
            {
                ok = TIMreadline(gx->gstream,linebase,linebuf,ginfo);
                linebase += rowbytes;
            }
            gfree(linebuf);
        }
    }
    return(ok);
}

int GCALL TIM_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

