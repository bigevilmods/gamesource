// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    PCX - PC eXhange Gimex                        */
/*                                                                  */
/*                   by FrANK G. Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940514 FB Based on bmp 1.07 and pcx plugin               */
/*    1.01 950529 FB ginstance signature                            */
/*    1.02 960228 FB export 8, mem leak                             */
/*    1.03 960301 FB dcx multiframe,24 bit out,import bpl           */
/*    1.04 970624 GS pad line buffer to handle bad runlengths       */
/*    1.05 990507 KO export, proper RLE, palette header 0x0C        */
/*    1.06 990823 FB dpi                                            */
/*    1.07 000316 FB packed=0 to do inefficient rle (byte pairs)    */
/*    1.08 000317 FB subtype=1 allow 32 bpp                         */
/*    1.09 040121 FB more strick PCX_is function                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1,4,8,24,32* bit  (32 bit nonstandard)                           */
/* Reentrant                                                        */
/* read supports unpacking.                                         */
/* 4 bits handled as 4 plane of 1 bit and 1 plane of 4 bit.         */
/* Files: pcxread.cpp pcxwrite.cpp pcxgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* PCX files support 1,2,3,4,8 and 24 bit depths.                   */
/* Runlength compression                                            */
/*                                                                  */
/* Tested types are all those supported by Photoshop, Alchemy and   */
/* Debabilizer.                                                     */
/*                                                                  */
/*  status    bits/pixel/plane  # planes  interpretation            */
/*  ------    ----------------  --------  --------------            */
/*    ok              1             1     monochrome                */
/*    ok              1             2     4 colour                  */
/*  implimented       1             3     8 colour                  */
/*    ok              1             4     16 colour                 */
/*    ok              2             1     4 colour cga header pal   */
/*  not implimented   2             2     16 colour                 */
/*  implimented       4             1     16 colour ega header pal  */
/*    yes             8             1     256 colour palette at eof */
/*    yes             8             3     16.7M colour              */
/*    yes             8             4     ARGB extension            */
/*                                                                  */
/* * uses header palette unless version is 5 or greater, in which   */
/* case uses palette at end of file                                 */
/*                                                                  */
/* Photoshop expects byte 0x0C between the image and the palette    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <stdlib.h>
#include <string.h>
#include "gimex.h"
#include "pcxgimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

static int PCXreadline(GSTREAM *g,unsigned char *d,unsigned char *linebuf,GINFO *ginfo,int bpl)
{
    int width;
    int bpp;
    int packed;
    int planes;
    int rowbytes;
    int ok=0;
    int x=0;
    int i;
    int count;
    unsigned char *s=linebuf;

    width   = ginfo->width;
    bpp     = ginfo->originalbpp;
    packed  = ginfo->packed;

    count = width;
    planes = ginfo->subtype;
    rowbytes = bpl;/*(((count*bpp+15)&~15)>>3); */
    if (bpp>8)
    {
        rowbytes *= bpp/8;
    }
    s = linebuf;

    if (packed)               /* run length compressed images */
    {
        unsigned int r;

        /* decompress the line */

        for (x=0; x<rowbytes; x += count)
        {
            ok = gread(g, s, 1);
            count = (int) *s;
            if (((unsigned int) count&0xc0)==0xc0)
            {
                count &= 0x3f;
                count = gmin(count,rowbytes-x);
                ok = gread(g, s, 1);
                r = (unsigned int) *s;

                for (i=0; i<count; ++i)
                {
                    *s++ = (unsigned char) r;
                }
            }
            else
            {
                count = 1;
                ++s;
            }
        }
    }
    else
    {
        ok = gread(g, s, rowbytes);
        if (rowbytes&1)
        {
            ok = gread(g, s+rowbytes, 1);
        }
    }
    s = linebuf;
    count = width;

    if (ok)
    {
        switch (bpp)
        {
            case 1:     /* 1 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    *d++ = (unsigned char) ((*s>>(7-(i&7)))&1);
                    if ((i&7)==7)
                        ++s;
                }
                break;

            case 2:     /* 2 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    *d++ = (unsigned char) ((*s>>(6-(i&3)*2))&3);
                    if ((i&3)==3)
                        ++s;
                }
                break;

            case 3:     /* 3 bit indexed image */

                for (i=0; i<count; ++i)   /* 3 planes of 1 bit */
                {
                    unsigned int b0;
                    unsigned int b1;
                    unsigned int b2;

                    b0 = (*(s          )>>(7-(i&7)))&1;
                    b1 = (*(s+count/8  )>>(7-(i&7)))&1;
                    b2 = (*(s+count/8*2)>>(7-(i&7)))&1;
                    *d++ = (unsigned char) (b0|(b1<<1)|(b2<<2));
                    if ((i&7)==7)
                        ++s;
                }
                break;

            case 4:     /* 4 bit indexed image */

                if (planes==1)  /* 1 plane of 4 bits */
                {
                    for (i=0; i<count; ++i)
                    {
                        if (i&1)
                            *d++ = (unsigned char) (*s++&15);
                        else
                            *d++ = (unsigned char) (*s>>4);
                    }
                }
                else            /* 4 planes of 1 bit */
                {
                    for (i=0; i<count; ++i)
                    {
                        unsigned int b0;
                        unsigned int b1;
                        unsigned int b2;
                        unsigned int b3;

                        b0 = (*(s          )>>(7-(i&7)))&1;
                        b1 = (*(s+count/8  )>>(7-(i&7)))&1;
                        b2 = (*(s+count/8*2)>>(7-(i&7)))&1;
                        b3 = (*(s+count/8*3)>>(7-(i&7)))&1;
                        *d++ = (unsigned char) (b0|(b1<<1)|(b2<<2)|(b3<<3));
                        if ((i&7)==7)
                            ++s;
                    }
                }
                break;

            case 8:     /* 8 bit indexed image */
                for (i=0; i<count; ++i)
                {
                    *d++ = *s++;
                }
                break;

            case 24:    /* 24 bit direct image (3 planes of 1 byte) */
                for (i=0; i<count; ++i)
                {
                    ((ARGB *) d)->r = *(s);
                    ((ARGB *) d)->g = *(s+count);
                    ((ARGB *) d)->b = *(s+count*2);
                    ((ARGB *) d)->a = 255;
                    d += 4;
                    s += 1;
                }
                break;

            case 32:    /* 32 bit direct image (4 planes of 1 byte) */
                for (i=0; i<count; ++i)
                {
                    ((ARGB *) d)->r = *(s);
                    ((ARGB *) d)->g = *(s+count);
                    ((ARGB *) d)->b = *(s+count*2);
                    ((ARGB *) d)->a = *(s+count*3);
                    d += 4;
                    s += 1;
                }
                break;
        }
    }
    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL PCX_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','P','C','X');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('P','C','X',' ');   /* mac file system types used */
        info->mactype[1]      = GMAKEID('B','I','N','A');
        strcpy(info->extensions[0], ".pcx");                /* extensions */
        strcpy(info->extensions[1], ".dcx");                /* extensions */
        strcpy(info->extensions[2], ".pcc");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.09");                /* version # */
        strcpy(info->shorttypestr,  "PCX");                 /* type */
        strcpy(info->wordtypestr,   "PCX");                 /* wordtype */
        strcpy(info->longtypestr,   "ZSoft PCX");           /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* signature and version number */
// STRUC PCX_Header
// .Manufacturer   resb    1       ; should always be 0Ah
// .Version        resb    1       ; 
// .Encoding       resb    1       ; should always be 01h
// .BitsPerPixel   resb    1       ; 
// .XMin           resw    1       ; image width = XMax-XMin
// .YMin           resw    1       ; image height = YMax-YMin
// .XMax           resw    1
// .YMax           resw    1
// .VertDPI        resw    1       ; 
// .Palette        resb    48      ; 
// .Reserved       resb    1
// .ColorPlanes    resb    1       ; 
// .BytesPerLine   resw    1       ; 
// .PaletteType    resw    1
// .HScrSize       resw    1       ; only supported by
// .VScrSize       resw    1       ; PC Paintbrush IV or higher
// .Filler         resb    56
// ENDSTRUC

int GCALL PCX_is(GSTREAM *g)
{
    unsigned char pcxbuf[16]; /* Signature and Version */
    int ok=0;

    gseek(g, 0);
    if (gread(g, pcxbuf, 16))
    {
        if (pcxbuf[0]==10
		&& (pcxbuf[1]==0 || pcxbuf[1]==2 || pcxbuf[1]==3 || pcxbuf[1]==5) // version
		&&  pcxbuf[2]<=1 // encoding
		&& (pcxbuf[3]==1 || pcxbuf[3]==4 || pcxbuf[3]==8 || pcxbuf[3]==24 || pcxbuf[3]==32)) // bpp
		{
            ok = 50;    /* PCX. */
			int width =  ggeti(pcxbuf+8,2)-ggeti(pcxbuf+4,2);
			int height = ggeti(pcxbuf+10,2)-ggeti(pcxbuf+6,2);
			if (width>0 && width<65000 && height>0 && height<65000)
				ok = 95;
		}
        else if (ggeti(pcxbuf,4)==0x3ade68b1)
            ok = 100;   /* DCX. */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL PCX_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int numframes=1;
    int ok=0;
    void *dcxheader;
    unsigned char pcxbuf[4]; /* Signature and Version */

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = numframes;
        gx->signature=GMAKEID('.','P','C','X');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

        gseek(g, 0);
        if (gread(g, pcxbuf, 4))
        {
            ok = 1;
            if (ggeti(pcxbuf,4)==0x3ade68b1)
            {
                numframes = -1;
                do
                {
                    ++numframes;
                    ok = gread(g, pcxbuf, 4);
                } while (ok && ggeti(pcxbuf,4));
            }
            dcxheader = galloc(4+numframes*4+4);
            if (dcxheader)
            {
                gx->gref = dcxheader;
                gseek(gx->gstream,(GPOS)0);
                ok = gread(gx->gstream, dcxheader, 4+numframes*4+4);
            }
        }
        ok = gx->frames = numframes;
        *gxptr = gx;
    }
    return(ok);
}

GINFO * GCALL PCX_info(GINSTANCE *gx, int framenum)
{
    PCXHeaderStruct pcxheader;
    unsigned char palbuf[256*4];
    GINFO *ginfo=0;
    int bpp;
    int colours;
    int version;
    int wherearecolours;
    int i;
    int width;
    int height;
    int defaultx;
    int pal;
    int defaulty;
    int planes;
    int packed;
    int dpi;
    int ok=0;
    int frameoff=0;
    unsigned char *dcxheader=(unsigned char *)gx->gref;

    if (ggeti(dcxheader,4)==0x3ade68b1)
        frameoff = ggeti(dcxheader+4+framenum*4,4);

    gseek(gx->gstream,frameoff);
    ok = gread(gx->gstream, (char *) &pcxheader, 128);   /* read common header */
    if (ok)
    {
        width    = PCXGETFIELD(pcxheader.xmax)-PCXGETFIELD(pcxheader.xmin)+1;       /* Width of bitmap in pixels */
        height   = PCXGETFIELD(pcxheader.ymax)-PCXGETFIELD(pcxheader.ymin)+1;       /* Height of bitmap in pixels */
        defaultx = PCXGETFIELD(pcxheader.xmin);
        defaulty = PCXGETFIELD(pcxheader.ymin);
        bpp      = PCXGETFIELD(pcxheader.bpp);      /* Bits per pixel per plane */
        planes   = PCXGETFIELD(pcxheader.planes);   /* Number of planes 1,3 or 4 */
        packed   = PCXGETFIELD(pcxheader.encoding); /* Compression scheme (0 for none, 1 RLE) */
        version  = PCXGETFIELD(pcxheader.version);  /* Version number */
        dpi      = PCXGETFIELD(pcxheader.hdotpinch);/* DPI */
        pal      = PCXGETFIELD(pcxheader.palinterp);/* 1=colour or b&w, 2=grey scale, 3=EA argb palette */

        if (!planes)   planes = 1;
        if (!bpp)      bpp = 8;

        if (framenum < gx->frames)
            ginfo = (GINFO *) galloc(sizeof(GINFO));

        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','P','C','X');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 /* Number of frames (if more than 1, openframeblt required) */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* Use subtype for number of planes */
            ginfo->packed           = packed;
            ginfo->quality          = 100;      /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = (planes==4 || pal==3)?8:0; /* Number of bits in alpha channel */
            ginfo->redbits          = 8;        /* Number of bits in red channel */
            ginfo->greenbits        = 8;        /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;        /* Number of bits in blue channel */
            ginfo->centerx          = 0;        /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = defaultx; /* Default Coordinate Point */
            ginfo->defaulty         = defaulty;
            ginfo->numhotspots      = 0;        /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;        /* Null terminated name of frame/image */
            ginfo->dpi              = (float) dpi;

            bpp = bpp*planes;
            if (bpp>8)
            {
                ginfo->bpp              = 32;                       /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = planes*8;
                ginfo->numcolours  = 0;                             /* Number of colours in original indexed palette */
            }
            else
            {
                int extplanes = (pal==3)?4:3;

                colours = (1 << bpp);
                wherearecolours = 0; /* in header */
                if (colours>16)
                    wherearecolours = 1; /* at end of file */

                if (wherearecolours)
                {
                    if (framenum<gx->frames-1)
                        frameoff = ggeti(dcxheader+4+(framenum+1)*4,4);
                    else
                        frameoff = (int)glen(gx->gstream);

                    gseek(gx->gstream,frameoff-colours*extplanes);
                    ok = gread(gx->gstream, (char *) palbuf, colours*extplanes);
                }
                else
                    memcpy(palbuf,&pcxheader.headerpal,colours * extplanes);

                if (ok)
                {
                    if (bpp==1)
                    {
                        ginfo->colourtbl[0].a = 255;
                        ginfo->colourtbl[0].r = 0;
                        ginfo->colourtbl[0].g = 0;
                        ginfo->colourtbl[0].b = 0;
                        ginfo->colourtbl[1].a = 255;
                        ginfo->colourtbl[1].r = 255;
                        ginfo->colourtbl[1].g = 255;
                        ginfo->colourtbl[1].b = 255;
                    }
                    else if (bpp==3 && planes==1)
                    {
                        ginfo->colourtbl[0].a = 255;
                        ginfo->colourtbl[0].r = 0;
                        ginfo->colourtbl[0].g = 0;
                        ginfo->colourtbl[0].b = 0;
                        ginfo->colourtbl[1].a = 255;
                        ginfo->colourtbl[1].r = 255;
                        ginfo->colourtbl[1].g = 255;
                        ginfo->colourtbl[1].b = 255;
                    }
                    else
                    {
                        for (i=0; i<colours; ++i)
                        {
                            ginfo->colourtbl[i].r = palbuf[i*extplanes+0];
                            ginfo->colourtbl[i].g = palbuf[i*extplanes+1];
                            ginfo->colourtbl[i].b = palbuf[i*extplanes+2];
                            if (extplanes==4)
                                ginfo->colourtbl[i].a = palbuf[i*extplanes+3];
                            else
                                ginfo->colourtbl[i].a = 255;
                        }
                    }
                }
                ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
                ginfo->originalbpp      = bpp;
                ginfo->numcolours  = colours;                  /* Number of colours in original indexed palette */
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

int GCALL PCX_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    void *linebuf;
    char *linebase;
    int y;
    int width;
    int height;
    int bpp;
    int bpl;
    int ok;
    int frameoff=0;
    unsigned char *dcxheader=(unsigned char *)gx->gref;
    PCXHeaderStruct pcxheader;

    if (ggeti(dcxheader,4)==0x3ade68b1)
        frameoff = ggeti(dcxheader+4+ginfo->framenum*4,4);

    gseek(gx->gstream,frameoff);
    ok = gread(gx->gstream, (char *) &pcxheader, 128);   /* read common header + os2 */
    if (ok)
    {

        width   = ginfo->width;
        height  = ginfo->height;
        bpp     = ginfo->originalbpp;
        bpl      = PCXGETFIELD(pcxheader.bpl);      /* Bytes per line per plane */
        if (!bpl)
            bpl = ((width*bpp+15)&~15)>>3;

        linebase = dest;
        linebuf = galloc(bpl*4*2);    /* *4 for 4 planes, *2 for worst case RLE */
        if (linebuf)
        {
            for (y=0; y<height; ++y)
            {
                if (ok)
                    ok = PCXreadline(gx->gstream,(unsigned char *)linebase,(unsigned char *)linebuf,ginfo,bpl);
                linebase += rowbytes;
            }
            gfree(linebuf);
        }
    }
    return(ok);
}

int GCALL PCX_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if (gx->gref)
            ok = gfree((void *) gx->gref);
        gfree((void *) gx);
    }

    return(ok);
}

