// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       SCT - Scitex Gimex                         */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 960303 FB initial version based on HSI                   */
/*    1.01 990823 FB dpi                                            */
/*    1.02 990830 FB aspect in photoshop friendly exp notation      */
/*    1.03 000621 FB width & height were flipped. fixed             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* Reentrant                                                        */
/* Line buffered                                                    */
/* Files: sctread.cpp sctwrite.cpp sctgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* SCT files are from Scitex and supported by Photoshop.            */
/* They are a simple format, but are in CMYK colour space.          */
/* Documenation on the format was unavailable at the time of        */
/* coding, so this header is reverse engineered.                    */
/*                                                                  */
/* header includes width and aspect.                                */
/* dpi = width/aspect                                               */
/*                                                                  */
/* CMYK->CMY                                                        */
/* Cyan=minimum(1,Cyan*(1-Black)+Black)                             */
/* Magenta=minimum(1,Magenta*(1-Black)+Black)                       */
/* Yellow=minimum(1,Yellow*(1-Black)+Black)                         */
/*                                                                  */
/* CMY->RGB                                                         */
/* Red   = 1-Cyan  (0<=Cyan<=1)                                     */
/* Green = 1-Magenta   (0<=Magenta<=1)                              */
/* Blue  = 1-Yellow    (0<=Yellow<=1)                               */
/*                                                                  */
/* This is a dump of the header on an 24 bit example.               */
/*                                                                  */
/* 00000: 43310000 00000000 00000000 00000000  C1..............     */
/*        comment (usually pathname)                                */
/* 00010: 00000000 00000000 00000000 00000000  ................     */
/* 00020: 00000000 00000000 00000000 00000000  ................     */
/* 00030: 00000000 00000000 00000000 00000000  ................     */
/* 00040: 00000000 00000000 00000000 00000000  ................     */
/* 00050: 43540000 00000000 00000000 00000000  CT..............     */
/*        signature                                                 */
/* 00060: 00000000 00000000 00000000 00000000  ................     */
/* 00070: 00000000 00000000 00000000 00000000  ................     */
/* 00080: 00000000 00000000 00000000 00000000  ................     */
/* 00090: 00000000 00000000 00000000 00000000  ................     */
/* 000a0: 00000000 00000000 00000000 00000000  ................     */
/* 000b0: 00000000 00000000 00000000 00000000  ................     */
/* 000c0: 00000000 00000000 00000000 00000000  ................     */
/* 000d0: 00000000 00000000 00000000 00000000  ................     */
/* 000e0: 00000000 00000000 00000000 00000000  ................     */
/* 000f0: 00000000 00000000 00000000 00000000  ................     */
/* 00100: 00000000 00000000 00000000 00000000  ................     */
/* 00110: 00000000 00000000 00000000 00000000  ................     */
/*                                                                  */
/* ....                                                             */
/*                                                                  */
/* 003d0: 00000000 00000000 00000000 00000000  ................     */
/* 003e0: 00000000 00000000 00000000 00000000  ................     */
/* 003f0: 00000000 00000000 00000000 00000000  ................     */
/* 00400: 0004000f 2b2e3130 34393238 3537452b  ....+.10492857E+     */
/*        channels aspect x/y                                       */
/* 00410: 30332b2e 37353134 32383630 452b3032  03+.75142860E+02     */
/* 00420: 2b303030 30303030 31343639 2b303030  +00000001469+000     */
/*        width/height.  scan for +'s                               */
/* 00430: 30303030 31303532 00000000 00000000  00001052........     */
/* 00440: 00000000 00000000 00000000 00000000  ................     */
/* 00450: 00000000 00000000 00000000 00000000  ................     */
/* 00460: 00000000 00000000 00000000 00000000  ................     */
/* 00470: 00000000 00000000 00000000 00000000  ................     */
/* 00480: 00000000 00000000 00000000 00000000  ................     */
/* 00490: 00000000 00000000 00000000 00000000  ................     */
/* ....                                                             */
/* 007b0: 00000000 00000000 00000000 00000000  ................     */
/* 007c0: 00000000 00000000 00000000 00000000  ................     */
/* 007d0: 00000000 00000000 00000000 00000000  ................     */
/* 007e0: 00000000 00000000 00000000 00000000  ................     */
/* 007f0: 00000000 00000000 00000000 00000000  ................     */
/* 00800: 17171817 1619191a 1b1e1f1c 17309db8  .............0.8     */
/*        data CMYK                                                 */
/* 00810: b4b5b0b1 adc2b9bb bcc4babc c2bfbec0  4501-B9;<D:<B?>@     */
/* 00820: c6c3c4c9 c8c9c9c2 c1c0babb c1c3c3bf  FCDIHIIBA@:;ACC?     */
/* 00830: bfbfbfb2 bbbcbabe c4c1c3c3 c5c6c5c2  ???2;<:>DACCEFEB     */
/* 00840: c2c6c1c4 c3c3c2bb babcc2c3 c1b7bac0  BFADCCB;:<BCA7:@     */
/* 00850: c1bebdc1 c8c7bfbf b9c3bfbd c3c7c0bf  A>=AHG??9C?=CG@?     */
/* 00860: b9bebcaf bbbcc1bf b7b7b5b5 a8a8aeae  9></;<A?7755((..     */
/* 00870: afaeaba6 a9aaa9a6 aba5a59c a2a8969e  /.+&)*)&+%%."(..     */
/* ...                                                              */
/* 5e59e0: 474d5153 51524a4c 4a484c4f 4f4c4c53  GMQSQRJLJHLOOLLS    */
/* 5e59f0: 5d574c2e 38515e5a 5450444c 75644559  ]WL.8Q^ZTPDLudEY    */
/* 5e5a00: 3632526b 5d453e53 63494d42 4b333f53  62Rk]E>ScIMBK3?S    */
/* 5e5a10: 504f4548 51565053 534c4933 4a534b48  POEHQVPSSLI3JSKH    */
/* 5e5a20: 3d434d46 413d4745 353f513e 3d545754  =CMFA=GE5?Q>=TWT    */
/* 5e5a30: 37454650 47494b5a 4e474c4c 45383b4f  7EFPGIKZNGLLE8;O    */
/* 5e5a40: 513e4535 324b566a 5753474d 4546684d  Q>E52KVjWSGMEFhM    */
/* 5e5a50: 35463835 574c4b52 4e514945 4c4c595d  5F85WLKRNQIELLY]    */
/* 5e5a60: 464b5051 54525c5a 49535553 564d5446  FKPQTR\ZISUSVMTF    */
/* 5e5a70: 453f4343 57544858 53514941 3f564e53  E?CCWTHXSQIA?VNS    */
/* 5e5a80: 50433748 4c53573d 4d513e45 544e4745  PC7HLSW=MQ>ETNGE    */
/* 5e5a90: 5a42484d 474f5545 35373f36 3133292e  ZBHMGOUE57?613).    */
/* 5e5aa0: 2e2e2e2e 2e2e2e2f                    ......./            */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gimex.h"
#include "sctgimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL SCT_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','S','C','T');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
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
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','S','C','T');   /* mac file system types used */
        strcpy(info->extensions[0], ".sct");                /* extensions */
        strcpy(info->extensions[1], ".sc");                 /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "SCT");                 /* type */
        strcpy(info->wordtypestr,   "SCT");                 /* wordtype */
        strcpy(info->longtypestr,   "Scitex SCT");             /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* signature at 0x50: CT and channels/mask at 0x400: 0x0104000f or 0x01030007 */

int GCALL SCT_is(GSTREAM *g)
{
    unsigned char sctheader[4];
    int flen;
    int ok=0;

    flen = (int)glen(g);
    if (flen>2048)
    {
        gseek(g,0x50);
        if (gread(g,sctheader,4))
        {
            if (ggetm(sctheader,4)==0x43540000)
            {
                ok = 50;
                gseek(g,0x400);
                if (gread(g,sctheader,4))
                {
                    if (ggetm(sctheader,4)==0x0104000f
                     || ggetm(sctheader,4)==0x01030007)
                        ok = 100;
                }
            }
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL SCT_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','S','C','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL SCT_info(GINSTANCE *gx, int framenum)
{
    char sctheader[256];
    GINFO *ginfo=0;
    int ok=0;
    GPOS framesize;
    int cmyk;
    double faspectx;
    double faspecty;
    double fwidth;
    double fheight;
    double dpi;

    gseek(gx->gstream,0x400);
    framesize = glen(gx->gstream);
    ok = gread(gx->gstream, sctheader, 256);   /* read header */
    if (ok)
    {
        cmyk = 0;
        if (ggetm(sctheader,4)==0x0104000f)    /* 4 channels = CMYK, 3 = rgb */
            cmyk = 1;
        sscanf(sctheader+4,"%14lf%14lf%12lf%12lf",&faspectx,&faspecty,&fheight,&fwidth);

        dpi = 0.0;
        if (faspectx)
            dpi = fwidth/faspectx;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','S','C','T');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Current frame num */
            ginfo->width            = (int) fwidth;             /* Width of bitmap in pixels */
            ginfo->height           = (int) fheight;            /* Height of bitmap in pixels */
            ginfo->framesize        = (int)framesize;
            ginfo->subtype          = cmyk;                     /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;            /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
            ginfo->bpp              = 32;           /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 24;
            ginfo->numcolours       = 0;        /* Number of colours in original indexed palette */
            ginfo->dpi              = (float) dpi;
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}



int GCALL SCT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    unsigned char *linebuf;
    int width=ginfo->width;
    int awidth=(ginfo->width+3)&~3;
    int bpl;
    int x;
    int y;
    int ok=0;
    int sc;
    int sm;
    int sy;
    int sk;
    int cmyk;
    ARGB *d;

    gseek(gx->gstream,0x800);
    cmyk = ginfo->subtype;
    bpl = awidth*(cmyk?4:3);
    linebuf = (unsigned char *) galloc(bpl);
    if (linebuf)
    {
        ok = 1;

        for (y=0; y<ginfo->height && ok; ++y)
        {
            d = (ARGB *) (dest+rowbytes*y);

            ok = gread(gx->gstream,linebuf,bpl);

            if (cmyk)
            {
                for (x=0; x<width; ++x)
                {
                    sc = linebuf[awidth*0+x];
                    sm = linebuf[awidth*1+x];
                    sy = linebuf[awidth*2+x];
                    sk = linebuf[awidth*3+x];

                    d->a = (unsigned char)(255);
                    d->r = (unsigned char)(255-((255-sc)*sk/255+(255-sk)));
                    d->g = (unsigned char)(255-((255-sm)*sk/255+(255-sk)));
                    d->b = (unsigned char)(255-((255-sy)*sk/255+(255-sk)));

                    ++d;
                }
            }
            else
            {
                for (x=0; x<width; ++x)
                {
                    d->a = 255;
                    d->r = linebuf[awidth*0+x];
                    d->g = linebuf[awidth*1+x];
                    d->b = linebuf[awidth*2+x];

                    ++d;
                }
            }

        }
        gfree(linebuf);
    }

    return(ok);
}

int GCALL SCT_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

