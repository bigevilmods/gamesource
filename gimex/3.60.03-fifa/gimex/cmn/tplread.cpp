// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*              TPL - Nintendo GameCube Texture Gimex               */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 000502 FB First draft based on TIM                       */
/*    1.01 000503 FB I8 import                                      */
/*    1.02 000504 FB export                                         */
/*    1.03 001030 FB signature changed to 0020af30                  */
/*    1.04 010305 FB subtype1+8888 to enable 32bit cluts            */
/*    1.05 010319 FB write tev clut (32 bpp)                        */
/*    1.06 010423 FB tev clut swizzle fixed                         */
/*    1.07 010424 FB import numcolours                              */
/*    1.08 010430 ID import new TEV format                          */
/*    1.09 010611 ID big fix on tile alignment for 8888 format      */
/*    1.10 010810 FB shift 5a3 bits on export                       */
/*    1.11 010814 ID fix of block size on awidth/aheight            */
/*    1.12 010815 ID added CMPR type reader                         */
/*    1.13 010815 ID fixed CMPR boundary pad                        */
/*    1.14 020714 FB IA8 8800 argb bits                             */
/*    1.15 020717 FB IA8 numcolours                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: tplread.cpp tplwrite.cpp tplgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Big endian format                                                */
/* Multiframe                                                       */
/* Mipmaps                                                          */
/*                                                                  */
/* Example Image:                                                   */
/* --------------                                                   */
/*                                                                  */
/* cartex.tpl                                                       */
/* 00000: 0020af30 00000001 0000000c 00000014  . /0............     */
/* 00010: 00000000 01000100 00000004 00000040  ...............@     */
/* 00020: 00000001 00000001 00000001 00000001  ................     */
/* 00030: 00000000 00000000 00000000 00000000  ................     */
/* 00040: 50a250a2 50a250a2 58a258c3 58c358c3  P"P"P"P"X"XCXCXC     */
/* 00050: 50a258c3 58c358c3 50a258c3 58c358c3  P"XCXCXCP"XCXCXC     */
/* 00060: 50a258a2 58a260a2 58c358c3 60a260a2  P"X"X"`"XCXC`"`"     */
/* 00070: 58c358c3 60a260a2 58c358c3 60a260a2  XCXC`"`"XCXC`"`"     */
/* 00080: 60a260a2 60a260a2 68a268a2 60a260a2  `"`"`"`"h"h"`"`"     */
/* 00090: 60a260a2 60a260a2 60a260a2 68a268a2  `"`"`"`"`"`"h"h"     */
/*                                                                  */
/* tg-02.tpl                                                        */
/* 00000: 009ac12f 00000003 0000000c 00000024  ..A/...........$     */
/* 00010: 00000000 00000048 00000000 0000006c  .......H.......l     */
/* 00020: 00000000 00800080 00000001 000000a0  ...............      */
/* 00030: 00000001 00000001 00000001 00000001  ................     */
/* 00040: 00000000 00000000 00800080 00000001  ................     */
/* 00050: 000040a0 00000001 00000001 00000001  ..@ ............     */
/* 00060: 00000001 00000000 00000000 00800080  ................     */
/* 00070: 00000001 000080a0 00000001 00000001  ....... ........     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "tplgimex.h"
#include "pshgimex.h"
#include "codex.h"
#include "dxtcodex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

#define BLOCKWIDTH   4
#define BLOCKHEIGHT  4


//-------------------------------------------------------------------------------------------------------
// these are DXT block offsets. The x and y specify the DXT block in a linear scanline ordering.
// the width is the number of DXT blocks that I have in a scanline.
// returns a block ordering so multiply by 8 to get the byte offset.

static int SwizzleDXT1GetIndex( int x, int y, int width)
{
    int NumberOfBlocksX = (1+width)>>1;
    int XBlock = x>>1;
    int YBlock = y>>1;
    int XPix   = x&1;
    int YPix   = y&1;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<2) + // the offset to the current tile
            ((YPix<<1) + (XPix));                    // within the tile the current block (note * by 8 to get address)
}

//--------------------------------------------------------------------------------------------------------
// assumes that DXT blocks have also been swizzled.

static void TPLRGSHUnorderBlocks(int width, int height, int rowbytes, unsigned char *source, unsigned char *dest)
{
    int i;
    int j;
    int k;
    unsigned char *s = source;
    unsigned char *d;
	int blockwidth;
	int blockheight;
    for (i=0; i<height; i+=BLOCKHEIGHT)
    {
        blockheight = gmin(BLOCKHEIGHT, height-i);
        d = dest + i*rowbytes;
        for (j=0; j<width; j+=BLOCKWIDTH)
        {
            blockwidth = gmin(BLOCKWIDTH, width-j);
            s = (source + (SwizzleDXT1GetIndex(j>>2,i>>2,width>>2)*64));
			for (k=0; k<blockheight; k++)
            {
                memcpy(d + k*rowbytes, s, 4 * blockwidth);
                s += 4 * BLOCKWIDTH;
            }
            d += 4 * BLOCKWIDTH;
        }
    }
}

/* convert GameCube 16 bit entry to 32 bit ARGB entry */

static ARGB TPLpixel5a3To8(unsigned short aRgb5)
{
    ARGB    aRgb8;

    if (aRgb5&0x8000)  // rgb555
    {
        aRgb8.a = (unsigned char)(255);
        aRgb8.r = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
        aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
        aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
    }
    else // argb3444
    {
        aRgb8.a = (unsigned char)((((aRgb5>>12)&7) *255+4)/7);
        aRgb8.r = (unsigned char)((((aRgb5>>8 )&15)*255+8)/15);
        aRgb8.g = (unsigned char)((((aRgb5>>4 )&15)*255+8)/15);
        aRgb8.b = (unsigned char)((((aRgb5>>0 )&15)*255+8)/15);
    }
    return(aRgb8);
}

static ARGB TPLpixel565To8(unsigned short aRgb5)
{
    ARGB    aRgb8;

    aRgb8.a = (unsigned char)(255);
    aRgb8.r = (unsigned char)((((aRgb5>>11)&31)*255+16)/31);
    aRgb8.g = (unsigned char)((((aRgb5>>5 )&63)*255+32)/63);
    aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);

    return(aRgb8);
}

static int TPLgetpixeloffset(int x, int y, int width, int bpp)
{
    int offset=0;

    switch (bpp)
    {
        case 4: // 8x8
            offset = 8*8/2*(width/8)*(y/8) + 8*8/2*(x/8) + 8/2*(y&7) + (x&7)/2;
            break;


        case 8: // 8x4
            offset = 8*4*1*(width/8)*(y/4) + 8*4*1*(x/8) + 1*8*(y&3) + 1*(x&7);
            break;

        default:
        case 16: // 4x4
            offset = 4*4*2*(width/4)*(y/4) + 4*4*2*(x/4) + 2*4*(y&3) + 2*(x&3);
            break;

        case 32: // 2 blocks of 4x4.  gives offset to AR. +4*4*2 for GB
            offset = 4*4*4*(width/4)*(y/4) + 4*4*4*(x/4) + 2*4*(y&3) + 2*(x&3);
            break;

    }
    return(offset);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL TPL_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','T','P','L');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','T','P','L');   /* mac file system types used */
        strcpy(info->extensions[0], ".tpl");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.15");                /* version # */
        strcpy(info->shorttypestr,  "TPL");                 /* type */
        strcpy(info->wordtypestr,   "TPL");              /* wordtype */
        strcpy(info->longtypestr,   "GameCube Texture"); /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* new id 0020af30 */
/* old id 009ac12f */

int GCALL TPL_is(GSTREAM *g)
{
    unsigned char tplheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,tplheader,4))
    {
        if (ggetm(tplheader,4)==0x0020af30)
            ok = 100;    /* 100% odds its a tpl */
        if (ggetm(tplheader,4)==0x009ac12f)
            ok = 99;     /* its an old tpl */
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL TPL_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    unsigned char tplheader[12];

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('.','T','P','L');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;

        gseek(g,(GPOS)0);
        if (gread(g,tplheader,12))
        {
            gx->frames = ok = ggetm(tplheader+4,4);
        }
    }
    return(ok);
}

GINFO * GCALL TPL_info(GINSTANCE *gx, int framenum)
{
    unsigned char tplheader[12];
    unsigned char tpldescriptor[8];
    TPLClutStruct tplclut;
    TPLImageStruct tplimage;
    int  offset=0;
    int  ok=0;
    GINFO *ginfo=0;
    int  bpp=0;
    int  alphabits;
    int  redbits;
    int  greenbits;
    int  bluebits;
    int  colours=0;
    int  clut;
    int  i;
    int  width;
    int  height;

    memset(&tplclut,0,sizeof(tplclut));
    memset(&tplimage,0,sizeof(tplimage));

    if (framenum < gx->frames)
    {
        gseek(gx->gstream,(GPOS)0);
        ok = gread(gx->gstream, (char *) &tplheader, 12);   /* read file header id/flags */
    }

    if (ok)
    {
        offset = ggetm(tplheader+8,4); /* offset to descriptor table */

        gseek(gx->gstream,offset+framenum*8);
        ok = gread(gx->gstream, (char *) &tpldescriptor, 8); /* read file header id/flags */
        if (ok)
        {
            offset = ggetm(tpldescriptor+0,4); /* offset to image header */
            if (offset)
            {
                gseek(gx->gstream,offset);
                ok = gread(gx->gstream, (char *) &tplimage, sizeof(tplimage)); /* read image header */
            }

            offset = ggetm(tpldescriptor+4,4); /* offset to clut header */
            if (offset)
            {
                gseek(gx->gstream,offset);
                ok = gread(gx->gstream, (char *) &tplclut, sizeof(tplclut)); /* read optional clut header */
            }
        }
    }

    if (ok)
    {
        width       = TPLGETFIELD(tplimage.Width);  /* Width of bitmap in images */
        height      = TPLGETFIELD(tplimage.Height); /* Height of bitmap in images */
        clut = 0;
        switch (TPLGETFIELD(tplimage.ImageFormat))
        {
            case GX_TF_C4:     bpp=4,  alphabits=0, redbits=0, greenbits=0, bluebits=0; clut=1; break;
            case GX_TF_C8:     bpp=8,  alphabits=0, redbits=0, greenbits=0, bluebits=0; clut=1; break;
            case GX_TF_C14X2:  bpp=14, alphabits=0, redbits=0, greenbits=0, bluebits=0; clut=1; break;
            case GX_TF_I4:     bpp=4,  alphabits=0, redbits=4, greenbits=0, bluebits=0; break;
            case GX_TF_I8:     bpp=8,  alphabits=0, redbits=8, greenbits=0, bluebits=0; break;
            case GX_TF_IA4:    bpp=8,  alphabits=4, redbits=4, greenbits=0, bluebits=0; break;
            case GX_TF_IA8:    bpp=16, alphabits=8, redbits=8, greenbits=0, bluebits=0; break;
            case GX_TF_RGB565: bpp=16, alphabits=0, redbits=5, greenbits=6, bluebits=5; break;
            default:
            case GX_TF_RGB5A3: bpp=16, alphabits=3, redbits=5, greenbits=5, bluebits=5; break;
            case GX_TF_RGBA8:  bpp=32, alphabits=8, redbits=8, greenbits=8, bluebits=8; break;
            case GX_TF_CMPR:   bpp=16, alphabits=1, redbits=5, greenbits=6, bluebits=5; break;
//            case GX_TF_Z8:     bpp=8,  alphabits=0, redbits=0, greenbits=0, bluebits=0; break;
//            case GX_TF_Z16:    bpp=16, alphabits=0, redbits=0, greenbits=0, bluebits=0; break;
//            case GX_TF_Z24X8:  bpp=24, alphabits=0, redbits=0, greenbits=0, bluebits=0; break;
        }
        if (bpp<=8)
        {
            colours = 1<<bpp;
            if (clut)
            {
                colours     = TPLGETFIELD(tplclut.NumberOfEntries);
                if (colours>256)
                    colours = 256;
                switch (TPLGETFIELD(tplclut.ClutFormat))
                {
                    case GX_TL_IA8:     alphabits=8, redbits=8, greenbits=0, bluebits=0; break;
                    case GX_TL_RGB565:  alphabits=0, redbits=5, greenbits=6, bluebits=5; break;
                    case GX_TL_RGBA8:   alphabits=8, redbits=8, greenbits=8, bluebits=8; break;
                    default:
                    case GX_TL_RGB5A3:  alphabits=3, redbits=5, greenbits=5, bluebits=5; break;
                }
            }
        }

        ginfo = (GINFO *) galloc(sizeof(GINFOSHP));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFOSHP));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','T','P','L'); /* Type of file */
            ginfo->size             = sizeof(GINFOSHP);         /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 /* Number of frames (if more than 1, openframeblt required) */
            ginfo->width            = width;                    /* Width of bitmap in images */
            ginfo->height           = height;                   /* Height of bitmap in images */
            ginfo->bpp              = bpp>8?32:8;               /* Bits per image (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours       = colours;
            ginfo->subtype          = (bpp <=8 && (TPLGETFIELD(tplclut.ClutFormat)) == GX_TL_RGBA8)?1:0; // TEV format uv lookup into 32bpp clut                        /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
            ginfo->redbits          = redbits;      /* Number of bits in red channel */
            ginfo->greenbits        = greenbits;    /* Number of bits in grreen channel */
            ginfo->bluebits         = bluebits;     /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */

            if (bpp<=8)
            {
                if (clut)
                {
                    unsigned short rgb;
                    unsigned char pal[256*4];   /* normally 256*2, but leave space incase 24 bit */

                    offset     = TPLGETFIELD(tplclut.Offset);
                    gseek(gx->gstream,offset);
                    if(TPLGETFIELD(tplclut.ClutFormat) != GX_TL_RGBA8)
                        ok = gread(gx->gstream, (char *) &pal, colours*2); /* read image header */
                    else
                        ok = gread(gx->gstream, (char *) &pal, colours*4); /* read in colours for TEV format 32bp clut entry*/
                    switch (TPLGETFIELD(tplclut.ClutFormat))
                    {
                        case GX_TL_RGB565:
                            for (i=0; i<colours; ++i)
                            {
                                rgb = (unsigned short) ggetm(pal+i*2,2);
                                ginfo->colourtbl[i] = TPLpixel565To8(rgb);
                            }
                            break;
                        case GX_TL_RGBA8:
                        // TEV based indirect into 32 bit image to implement clut
                        // in english two 16 bit pages page1: AR
                        //                             page2: GB  each page is swizzled
                        //                             using standard methods.
                        // each page is 16x16 thus 256 entries and indexed through a
                        // 8 bit uv index in the image data. (each nibble represents u or v)
                            {
                                unsigned char * d = 0;
                                int x = 0;
                                int y = 0;
                                unsigned char * s = (unsigned char *) &ginfo->colourtbl[0];
                                for (x=0; x<16; ++x)
                                {
                                    for (y=0; y<16; ++y)         // note bigendian format so backwards.
                                    {
                                        d = &pal[0] + TPLgetpixeloffset(x,y,16,32);

                                        ((ARGB *) s)->a = d[0];
                                        ((ARGB *) s)->r = d[1];
                                        ((ARGB *) s)->g = d[16*2+0];
                                        ((ARGB *) s)->b = d[16*2+1];
                                        s += 4;
                                    }
                                }
                            }
                            break;

                        default:
                        case GX_TL_RGB5A3:
                            for (i=0; i<colours; ++i)
                            {
                                rgb = (unsigned short) ggetm(pal+i*2,2);
                                ginfo->colourtbl[i] = TPLpixel5a3To8(rgb);
                            }
                            break;
                    }
                }
                else
                {
                    switch (TPLGETFIELD(tplimage.ImageFormat))
                    {
                        case GX_TF_I4:
                            for (i=0; i<colours; ++i)
                            {
                                ginfo->colourtbl[i].a = 255;
                                ginfo->colourtbl[i].r = (unsigned char) ((i*255+8)/15);
                                ginfo->colourtbl[i].g = (unsigned char) ((i*255+8)/15);
                                ginfo->colourtbl[i].b = (unsigned char) ((i*255+8)/15);
                            }
                            break;

                        default:
                        case GX_TF_I8:
                            for (i=0; i<colours; ++i)
                            {
                                ginfo->colourtbl[i].a = 255;
                                ginfo->colourtbl[i].r = (unsigned char) (i);
                                ginfo->colourtbl[i].g = (unsigned char) (i);
                                ginfo->colourtbl[i].b = (unsigned char) (i);
                            }
                            break;

                        case GX_TF_IA4:
                            for (i=0; i<colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char) (((i>>4)*255+8)/15);
                                ginfo->colourtbl[i].r = (unsigned char) (((i&15)*255+8)/15);
                                ginfo->colourtbl[i].g = (unsigned char) (((i&15)*255+8)/15);
                                ginfo->colourtbl[i].b = (unsigned char) (((i&15)*255+8)/15);
                            }
                            break;
                    }
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

int GCALL TPL_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    unsigned char tplheader[12];
    unsigned char tpldescriptor[8];
    TPLImageStruct tplimage;
    int  offset=0;
    int  ok=0;
    int  x;
    int  y;
    int  width;
    int  height;
    int  awidth;
    int  aheight;
    int  bpp;
    int  format;
    unsigned short rgb;
    unsigned char *framebuf;
    unsigned char *s;
    unsigned char *d;

    memset(&tplimage,0,sizeof(tplimage));

    gseek(gx->gstream,(GPOS)0);
    ok = gread(gx->gstream, (char *) &tplheader, 12);   /* read file header id/flags */
    if (ok)
    {
        offset = ggetm(tplheader+8,4); /* offset to descriptor table */

        gseek(gx->gstream,offset+ginfo->framenum*8);
        ok = gread(gx->gstream, (char *) &tpldescriptor, 8); /* read file header id/flags */
        if (ok)
        {
            offset = ggetm(tpldescriptor+0,4); /* offset to image header */
            if (offset)
            {
                gseek(gx->gstream,offset);
                ok = gread(gx->gstream, (char *) &tplimage, sizeof(tplimage)); /* read image header */
            }
        }
    }

    if (ok)
    {
        width   = ginfo->width;
        height  = ginfo->height;
        bpp     = ginfo->originalbpp;
        switch(bpp)
        {
        case 4: // 8x8
            awidth  = (width+7)&~7;
            aheight = (height+7)&~7;
            break;
        case 8: // 8x4
            awidth  = (width+7)&~7;
            aheight = (height+3)&~3;
            break;

        default:
        case 16: // 4x4
            awidth  = (width+3)&~3;
            aheight = (height+3)&~3;
            break;

        case 32: // 2 blocks of 4x4.  gives offset to AR. +4*4*2 for GB
            awidth  = (width+3)&~3;
            aheight = (height+3)&~3;
            break;
        }
        format = TPLGETFIELD(tplimage.ImageFormat);

        if(format == GX_TF_CMPR) // is image DXT compressed
        {
            awidth  = (width+15)&~15;
            aheight = (height+15)&~15;           
            framebuf = (unsigned char *) galloc((awidth*aheight+2)/2 + sizeof(DXTHEADER));
            if (framebuf)
            {
                char * temp2 = 0;
                int TotalSize = awidth*aheight*sizeof(ARGB);     // calculate the total size of the image.
                int isBig = (TotalSize > 16000000)?1:0;          // any images over 16MB have to be handled with the new DXTBIG header.
                offset      = TPLGETFIELD(tplimage.Offset);      // offset to texels 
                gseek(gx->gstream,offset);
                ok = gread(gx->gstream, (char *) framebuf + sizeof(DXTHEADER), (awidth*aheight+2)/2);    /* read image skip DXTHEADER*/

                // build the DXT header for the codec to use. Assume DXT1
                // big endian.
                if(ok)
                {
                    temp2 = (char *)galloc(awidth*aheight*4);    // create working buffer to decompress into.
                    if(isBig)
                    {
                        gputm(((BIGDXTHEADER*)framebuf)->size, awidth*aheight*sizeof(ARGB), sizeof(((BIGDXTHEADER*)framebuf)->size));           // put the size
                        gputm(((BIGDXTHEADER*)framebuf)->code, DXT_BIGDXT1B_CODE, sizeof(((BIGDXTHEADER*)framebuf)->code));
                        gputm(((BIGDXTHEADER*)framebuf)->mode, DXT_ENCODE_RGB_FULL, sizeof(((BIGDXTHEADER*)framebuf)->mode)); 
                        ((BIGDXTHEADER*)framebuf)->reserved[0]  = (char)(awidth  >> 2);
                    }
                    else
                    {
                        gputm(((DXTHEADER*)framebuf)->size, awidth*aheight*sizeof(ARGB), sizeof(((DXTHEADER*)framebuf)->size));        // put the size
                        gputm(((DXTHEADER*)framebuf)->code, DXT_DXT1B_CODE, sizeof(((DXTHEADER*)framebuf)->code));
                        gputm(((DXTHEADER*)framebuf)->mode, DXT_ENCODE_RGB_FULL, sizeof(((DXTHEADER*)framebuf)->mode)); 
                        ((DXTHEADER*)framebuf)->reserved[0]  = (char)(awidth  >> 2);
                        ((DXTHEADER*)framebuf)->reserved[1]  = (char)(aheight >> 2);
                    }
                    // do the decode
                    DXT_decode(temp2,framebuf,0);
                    // now unorder my blocks realizing that they are swizzled.
                    TPLRGSHUnorderBlocks(width, height, width*4,(unsigned char *)temp2, (unsigned char *)dest);
                }
                if(framebuf)gfree(framebuf);
                if(temp2)gfree(temp2);
            }            
        } // end if CMPR ie DXT compressed
        else
        {
            framebuf = (unsigned char *) galloc(awidth*aheight*bpp/8);
            if (framebuf)
            {
                offset      = TPLGETFIELD(tplimage.Offset);  /* offset to texels */
                gseek(gx->gstream,offset);
                ok = gread(gx->gstream, (char *) framebuf, awidth*aheight*bpp/8); /* read image */
                if (ok)
                {
                    for (y=0; y<height; ++y)
                    {
                        d = (unsigned char *) dest + y*rowbytes;
                        for (x=0; x<width; ++x)
                        {
                            s = framebuf+TPLgetpixeloffset(x,y,awidth,bpp);
    
                            switch (format)
                            {
                                case GX_TF_C4: // 4 bit indexed
                                case GX_TF_I4: // 4 bit intensity
                                    if (x&1)
                                        *d++ = (unsigned char) (*s&15);
                                    else
                                        *d++ = (unsigned char) (*s>>4);
                                    break;
    
                                case GX_TF_IA8: // intensity/alpha 88
                                    ((ARGB *) d)->a = s[0];
                                    ((ARGB *) d)->r = s[1];
                                    ((ARGB *) d)->g = s[1];
                                    ((ARGB *) d)->b = s[1];
                                    d += 4;
                                    break;
    
                                case GX_TF_IA4: // intensity/alpha 44
                                case GX_TF_C8: // 8 bit indexed
                                case GX_TF_I8: // 8 bit intensity
                                    *d++ = *s;
                                    break;
    
                                case GX_TF_RGB565: // 565
                                    rgb = (unsigned short)ggetm(s,2);
                                    *(ARGB *)d = TPLpixel565To8(rgb);
                                    d += 4;
                                    break;
    
                                case GX_TF_RGB5A3: // 3444/555
                                    rgb = (unsigned short)ggetm(s,2);
                                    *(ARGB *)d = TPLpixel5a3To8(rgb);
                                    d += 4;
                                    break;
    
                                
                                case GX_TF_RGBA8: // 8888
                                    ((ARGB *) d)->a = s[0];
                                    ((ARGB *) d)->r = s[1];
                                    ((ARGB *) d)->g = s[16*2+0];
                                    ((ARGB *) d)->b = s[16*2+1];
                                    d += 4;
                                    break;
                            }
                        }
                    }
                }

                gfree(framebuf);
            }
        }// not DXT compressed.
    }
    return(ok);
}

int GCALL TPL_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

