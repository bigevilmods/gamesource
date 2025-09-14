// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                        SGI - RGB Gimex                           */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940810 FB alchemy plugin                                 */
/*    1.01 950406 FB importer based on doc/code by Paul Haeberli    */
/*    1.02 950529 FB ginstance signature                            */
/*    1.03 950818 FB unsigned chars (bug fix for SGI)               */
/*    1.04 950821 FB unpack optimized for 0..255 byte               */
/*    1.05 951031 FB put RGB as default extension in about          */
/*    1.06 960506 FB 24 bit canimport set alpha to opaque              */
/*    1.07 960612 GS suppress pixmin and pixmax fields              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* canimport 8/24/32 compressed or uncompressed                        */
/* canexport 24 bit compressed                                         */
/* Reentrant                                                        */
/* Files: sgiread.cpp sgiwrite.cpp sgigimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* SGI is the native graphics file format for SGI computers.        */
/* It supports 24 and 32 bit most commonly.  In obsolete forms,     */
/* it supported grey scale, indexed colour, palettes and 332        */
/* rgb 8 bit.                                                       */
/*                                                                  */
/* Information on format available on:                              */
/* http://reality.sgi.com/grafica/sgiimage.html                     */
/*                                                                  */
/* Or try the author of the format, Paul Haeberli (paul@sgi.com)    */
/*                                                                  */
/* imagespl.sgi dump                                                */
/* -----------------                                                */
/*                                                                  */
/* 00000: 01da0101 00030174 015d0003 00000000  .Z.....t.]......     */
/* 00010: 000000ff 00000000 6e6f206e 616d6500  ........no name.     */
/* 00020: 00000000 00000000 00000000 00000000  ................     */
/* 00030: 00000000 00000000 00000000 00000000  ................     */
/* 00040: 00000000 00000000 00000000 00000000  ................     */
/* 00050: 00000000 00000000 00000000 00000000  ................     */
/* 00060: 00000000 00000000 00000000 00000005  ................     */
/* 00070: 00020000 0000015c 00020000 00000000  .......\........     */
/* 00080: 00000000 1000c9a0 00000000 00014afb  ......I ......J.     */
/* 00090: 1000a8d0 1000b938 00000000 00000000  ..(P..98........     */
/* 000a0: 00000000 00000000 00000000 00000000  ................     */
/* 000b0: 00000000 00000000 00000000 00000000  ................     */
/* 000c0: 00000000 00000000 00000000 00000000  ................     */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "sgigimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

/* DECODE - decodes run length encoded pixels */

static void SGIunpackrow(unsigned char *d, int bmod, unsigned char *s, int bpc, int pixmin, int pixmax)
{
    unsigned int pixel;
    unsigned int count;

    if (bpc==1 && pixmin==0 && pixmax==255)
    {
        do
        {
            pixel = *s;
            s += bpc;
            count = pixel&0x7f;
            if (count)
            {
                if (pixel&0x80)
                {
                    do
                    {
                        *d = *s;
                        s += bpc;
                        d += bmod;
                    } while (--count);
                }
                else
                {
                    pixel = *s;
                    s += bpc;
                    do
                    {
                        *d = (unsigned char)pixel;
                        d += bmod;
                    } while (--count);
                }
                --count;
            }
        } while (count);
    }
    else
    {
        do
        {
            pixel = ggetm(s,bpc);
            s += bpc;
            count = pixel&0x7f;
            if (count)
            {
                if (pixel&0x80)
                {
                    do
                    {
                        *d = (unsigned char)(ggetm(s,bpc)-pixmin)*255/(pixmax-pixmin);
                        s += bpc;
                        d += bmod;
                    } while (--count);
                }
                else
                {
                    pixel = ggetm(s,bpc);
                    s += bpc;
                    pixel = (pixel-pixmin)*255/(pixmax-pixmin);
                    do
                    {
                        *d = (unsigned char)pixel;
                        d += bmod;
                    } while (--count);
                }
                --count;
            }
        } while (count);
    }
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL SGI_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','S','G','I');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 79;   /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','S','G','I');   /* mac file system types used */
        strcpy(info->extensions[0], ".rgb");                /* extensions */
        strcpy(info->extensions[1], ".sgi");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.07");                /* version # */
        strcpy(info->shorttypestr,  "SGI");                 /* type */
        strcpy(info->wordtypestr,   "SGI");                 /* wordtype */
        strcpy(info->longtypestr,   "SGI RGB Image File");  /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* window values should be left/right top/bottom */
/* number of channels should be 1..4 */

int GCALL SGI_is(GSTREAM *g)
{
    SGIHeaderStruct sgiheader;
    int ok=0;

    gseek(g,(GPOS)0);
    if (glen(g)>512)
    {
        if (gread(g,(void *)&sgiheader,512))
        {
            if ((SGIGETFIELD(sgiheader.magic)==474)
             && (SGIGETFIELD(sgiheader.storage)<=1)
             && (SGIGETFIELD(sgiheader.bpc)>=1)
             && (SGIGETFIELD(sgiheader.bpc)<=2)
             && (SGIGETFIELD(sgiheader.dimension)>=1)
             && (SGIGETFIELD(sgiheader.dimension)<=3))
                ok = 95;    /* 95% odds its a sgi */
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL SGI_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','S','G','I');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL SGI_info(GINSTANCE *gx, int framenum)
{
    SGIHeaderStruct sgiheader;
    GINFO *ginfo=0;
    int width;
    int height;
    int channels;
    int bpp;
    int bpc;
    int packed;
    int i;
    int ok=0;

    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, (char *) &sgiheader, 512))   /* read header */
    {
        height = 1;
        channels = 1;
        width       = SGIGETFIELD(sgiheader.xsize);
        if (SGIGETFIELD(sgiheader.dimension)>=2)
            height      = SGIGETFIELD(sgiheader.ysize);
        if (SGIGETFIELD(sgiheader.dimension)>=3)
            channels    = SGIGETFIELD(sgiheader.zsize);
        bpc = SGIGETFIELD(sgiheader.bpc)*8;
        packed = SGIGETFIELD(sgiheader.storage);
        bpp = channels * bpc;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','S','G','I');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Frame number */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* subtype */
            ginfo->packed           = packed;
            ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = (channels>=4)?bpc:0;      /* Number of bits in alpha channel */
            ginfo->redbits          = bpc;          /* Number of bits in red channel */
            ginfo->greenbits        = bpc;          /* Number of bits in grreen channel */
            ginfo->bluebits         = bpc;          /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            strcpy(ginfo->framename,sgiheader.imagename);

            ginfo->bpp              = 32;       /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours  = 0;        /* Number of colours in original indexed palette */
            ok = 1;

            /* grey scale */
            if (bpp==8)
            {
                ginfo->bpp              = 8;        /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = bpp;
                ginfo->numcolours  = 256;        /* Number of colours in original indexed palette */
                for (i=0; i<256; ++i)
                {
                    ginfo->colourtbl[i].a = 255;
                    ginfo->colourtbl[i].r = i;
                    ginfo->colourtbl[i].g = i;
                    ginfo->colourtbl[i].b = i;
                }
            }
        }
    }

    return(ginfo);
}

int GCALL SGI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int width      = 0;
    int height     = 0;
    int channels   = 0;
    int bpp        = 0;
    int bpc        = 0;
    int packed     = 0;
    int planebytes = 0;
    int channel    = 0;
    int bmod       = 0;
    int pixmin     = 0;
    int pixmax     = 0;
    int ok         = 0;
    int packlineoffset        = 0;
    int packlinelen           = 0;
    unsigned char *linebase   = 0;
    unsigned char *d          = 0;
    unsigned char *buf        = 0;
    unsigned char *offsettbl  = 0;
    SGIHeaderStruct sgiheader;


    gseek(gx->gstream,(GPOS)0);
    if (gread(gx->gstream, (char *) &sgiheader, 512))   /* read header */
    {
        height = 1;
        channels = 1;
        width       = SGIGETFIELD(sgiheader.xsize);
        if (SGIGETFIELD(sgiheader.dimension)>=2)
            height      = SGIGETFIELD(sgiheader.ysize);
        if (SGIGETFIELD(sgiheader.dimension)>=3)
            channels    = SGIGETFIELD(sgiheader.zsize);
        bpc = SGIGETFIELD(sgiheader.bpc);
        packed = SGIGETFIELD(sgiheader.storage);
        pixmin = SGIGETFIELD(sgiheader.pixmin) & ((1<<bpc*8)-1);
        pixmax = SGIGETFIELD(sgiheader.pixmax) & ((1<<bpc*8)-1);
        bpp = channels * bpc * 8;
        planebytes = width * bpc;

        /* suppress pixmin and pixmap, remove these two lines if you want the */
        /* image to scale the values based on these values */
        pixmin = 0;
        pixmax = 255;

        buf = (unsigned char *) galloc(planebytes*4);
        if (buf)
        {
            if (packed)
            {
                /* read scanline offset table */

                offsettbl = (unsigned char *) galloc(height*channels*8);
                if (offsettbl)
                {
                    ok = gread(gx->gstream,offsettbl,height*channels*4*2);
                }
            }
            else
                ok = 1;
        }
    }

    if (ok)
    {
        /* clear all 32bit channels incase less than 32 bit */

        if (bpp>8)
        {
            for (y=0; y<ginfo->height; ++y)
            {
                d = (unsigned char *) dest + y*rowbytes;
                for (x=0; x<width; ++x)
                {
                    ((ARGB *)d)->a = 0xff;
                    ((ARGB *)d)->r = 0;
                    ((ARGB *)d)->g = 0;
                    ((ARGB *)d)->b = 0;
                    d += 4;
                }
            }
        }



        for (channel=0; channel<channels; ++channel)
        {
            if (channel<4)
            {
                for (y=0; y<ginfo->height && ok; ++y)
                {
                    linebase = (unsigned char *) dest + (height-1-y)*rowbytes;

                    if (ginfo->packed==0)
                    {
                        ok = gread(gx->gstream,buf,planebytes);
                    }
                    else
                    {
                        packlineoffset = ggetm(offsettbl+(channel*height+y)*4,4);
                        packlinelen    = ggetm(offsettbl+((channel+channels)*height+y)*4,4);
                        gseek(gx->gstream,packlineoffset);
                        ok = gread(gx->gstream,buf,packlinelen);
                    }

                    if (bpp==8)
                    {
                        d = linebase;
                        bmod = 1;
                    }
                    else
                    {
                        bmod = 4;
                        switch (channel)
                        {
                            case 0: d = &((ARGB *)linebase)->r; break;
                            case 1: d = &((ARGB *)linebase)->g; break;
                            case 2: d = &((ARGB *)linebase)->b; break;
                            case 3: d = &((ARGB *)linebase)->a; break;
                            default: ;
                        }
                    }

                    if (ginfo->packed==0)
                    {
                        for (x=0; x<width; ++x)
                        {
                            *d = (unsigned char)(ggetm(buf+x*bpc,bpc)-pixmin)*255/(pixmax-pixmin);
                            d += bmod;
                        }
                    }
                    else
                    {
                        SGIunpackrow(d,bmod,buf,bpc,pixmin,pixmax);
                    }
                }
            }
        }
    }

    if (offsettbl) gfree(offsettbl);
    if (buf) gfree(buf);
    return(ok);
}


int GCALL SGI_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


