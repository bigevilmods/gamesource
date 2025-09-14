// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  PIC - Macintosh Pict Gimex                      */
/*                                                                  */
/*  by Frank Barchard, Clarence Co, Gerry Shaw and Laurent Ancessi  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.99 931208 CC Neuron pictoimg code                           */
/*    1.00 940701 FB Initial PC plugin version                      */
/*    1.00 940722 GS Initial macintosh version                      */
/*    1.01 940726 GS Added write support                            */
/*    1.02 940728 FB Merged with PC plugin                          */
/*    1.03 940811 FB Started integrating Clarence Co's PICT code    */
/*    1.04 940813 FB ispic, working on 2,4,8 bit                    */
/*    1.05 940814 FB 32 bit,pict 1,1 bit,multi-pixmap               */
/*    1.06 940815 FB colourtables, 16 bit, skip quicktime           */
/*    1.07 940816 GS fixed colourtable write bug, unpack if bpp=1   */
/*    1.08 940926 FB clip height, reset count, psx tested           */
/*    1.09 941001 FB SGI version                                    */
/*    1.10 941010 FB about on PC indicates no canexport             */
/*    1.11 941212 LA Mac palette was writting 1 too few colours     */
/*    1.12 950102 FB ispic lower to 5% for first test               */
/*    1.13 950529 FB ginstance signature                            */
/*    1.14 950606 FB .pct extension (Photoshop)                     */
/*    1.15 950814 FB aligned planes                                 */
/*    1.16 951109 FB memory leak on 8 bit fixed                     */
/*    1.17 951112 FB canexport 32 bit; set alphabits; comments      */
/*    1.18 990420 FB export fail on 8 bit                           */
/*    1.19 990823 FB dpi                                            */
/*    1.20 000606 FB PIC_is does a parse to be sure                 */
/*    1.21 000720 FB PIC_is more robust                             */
/*    1.22 001017 FB PIC_is dont return 5 anymore, info more robust */
/*    1.23 020410 FB check for a few formats that are NOT pict      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Macintosh specific version uses Mac OS                           */
/* Files: picgimex.h picread.cpp picwrite.cpp (boff.h boff.c for Mac)   */
/*                                                                  */
/* Based on Clarence Co pictoimg code                               */
/*                                                                  */
/* Can parse PICT Version 1.0, 2.0 and 2.0 Extended file formats.   */
/* 1,2,4,8,16,24,32 bit depths.                                     */
/* Planar and Chunky supported.                                     */
/* Decompresses runlength.                                          */
/* Multi-PixMap images supported (created by Animator)              */
/* Sequential and non-sequential palettes supported.                */
/* Skips Quicktime opcodes.                                         */
/*                                                                  */
/* Tested with PICT files generated in Adobe PhotoShop 2.5, 3DO     */
/* Animator, Debabilizer, and Image Alchemy.                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* PICTs are structurally variable length records with opcode as    */
/* first field.  Record length based on opcode.                     */
/* PICT version 1 uses 1 byte opcodes.  PICT 2 uses 2 byte opcodes  */
/*                                                                  */
/* 1,2,4,8 indexed, 16,24,32 direct (32 bit with alpha)             */
/* Runlength compressed lines.                                      */
/* Chunky or planar.  Planar more common.                           */
/* Top to bottom, left to right                                     */
/* Can be broken into several rectangles, from Bottom to Top. This  */
/* feature is used by Animator.                                     */
/*                                                                  */
/* Colour Table is 16 bit per channel.                              */
/* Usually colour index followed by value.  If flag set, ignore     */
/*   index and just do sequentially.  (used by Animator)            */
/* Black and White images have no colour table                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "picgimex.h"

/****************************************************************/
/*  Common Functions (for all machines)                         */
/****************************************************************/

#ifdef MSDOS
#define EXPORTS 0
#else
#define EXPORTS 1
#endif


int parsepict(GSTREAM *g, GINFO *ginfo, char *dest, int rowbytes);

GABOUT * GCALL PIC_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('P','I','C','T');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;          /* supports importing */
        info->canexport       = 1;          /* supports exporting */
        info->importpacked    = 1;          /* supports importing packed */
        info->exportpacked    = 0;          /* supports exporting packed */
        info->import8         = 1;          /* supports importing 8 bit indexed */
        info->export8         = 0;          /* supports exporting 8 bit indexed */
        info->import32        = 1;          /* supports importing 32 bit direct rgb */
        info->export32        = 1;          /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('P','I','C','T');   /* mac file system types used */
        info->mactype[1]      = GMAKEID('V','D','A','N');   /* mac file system types used */

        strcpy(info->extensions[0], ".pict");               /* extensions */
        strcpy(info->extensions[1], ".pic");
        strcpy(info->extensions[2], ".pct");
        strcpy(info->authorstr,     "Frank/Gerry/Laurent"); /* authors */
        strcpy(info->versionstr,    "1.23");                /* version # */
        strcpy(info->shorttypestr,  "PICT");                /* type */
        strcpy(info->wordtypestr,   "MacPICT");             /* wordtype */
        strcpy(info->longtypestr,   "Macintosh Pict");      /* longtype */
    }
    return(info);
}

#undef EXPORTS

/* ispic */
/* two methods */
/* 1) check rectangle values are relatively right */
/* 2) check for size low 16 bits and opcode */

int GCALL PIC_is(GSTREAM *g)
{
    PICTHDR1 picheader;
    int ok=0;
    int flen;
    int left,right,top,bottom;
    char *op;

    flen = (int)glen(g);
    if (flen > PICTHEADERSIZE+12)
    {
        bool nonpict=false;
        char headerbuf[4];

        gseek(g, 0);

        if (gread(g, (void *)&headerbuf, 4))
        {
            if (ggetm(headerbuf,4)==0x4d5a9000 // exe
             || ggetm(headerbuf,2)==0x4c01) // obj
                nonpict = true;
        }

        if (!nonpict)
        {
            gseek(g, PICTHEADERSIZE);
            if (gread(g, (void *)&picheader, 12))
            {
                op = (char *)&picheader + 10;
                if (!*op)
                    ++op;

                left   = (int)PICGETFIELD(picheader.Frame.left  );
                right  = (int)PICGETFIELD(picheader.Frame.right );
                top    = (int)PICGETFIELD(picheader.Frame.top   );
                bottom = (int)PICGETFIELD(picheader.Frame.bottom);

                /* must pass this first pass */
                if (left<right && top<bottom)
                {
					int fsize1 = (int) PICGETFIELD(picheader.FileSize1);

                    if (left   >= 0
                     && left   <= 1024
                     && right  > 0
                     && right  <= 2048
                     && top    >= 0
                     && top    <= 1024
                     && bottom > 0
                     && bottom <= 2048)
                        ok += 35;   /* good bounding box... like a PICT. */

                    if (*op==0x11)
                        ok += 25;   /* first opcode looks reasonable for a PICT. */
                    
					if (fsize1==(unsigned int)((flen-PICTHEADERSIZE)&0xffff))
                        ok += 35;   /* it is a PICT! */

                    if (ok)
                    {
                        GINFO ginfo;
                        ok = parsepict(g, &ginfo, (char *) 0, 0);
                        if (ok)
                        {
                            int cpos=(int)gtell(g);
                            int mpos=(int)glen(g);
                            ok = (int)(90.0f * (float)cpos/mpos);  // scale ok by how far we parsed
                        }
                    }
                }
            }
        }
    }
    return(ok);
}

int GCALL PIC_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE*)galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames  = 1;
        gx->signature=GMAKEID('P','I','C','T');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL PIC_close(GINSTANCE *gx)
{
    int ok = 0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

/****************************************************************/
/*  Mac Pict                                                    */
/****************************************************************/

#ifdef applec

#include "BOff.cpp"

/****************************************************************/
/* Local Function Prototypes                                    */
/****************************************************************/

static int PICReadRow(char *source, char *dest, int width, int bpp);

/****************************************************************/
/* Import Functions                                             */
/****************************************************************/

GINFO * GCALL PIC_info(GINSTANCE *gx, int /*framenum*/)
{
    OSErr err;
    PicHandle pict;
    GINFO *ginfo;
    int dataLength;

    /* allocate buffer to store pict */
    dataLength = glen(gx->gstream) - PICTHEADERSIZE;    /* subtract unused header */
    pict = (PicHandle)NewHandle(dataLength);

    if (pict != 0)
    {
        PictInfo pictInfo;

        int width;
        int height;

        int bpp;
        int indexed;
        int originalbpp;
        int numcolours;
        ARGB colortbl[256];
        int packed;

        /* read pict from file into pict handle */
        gseek(gx->gstream, PICTHEADERSIZE);
        gread(gx->gstream, (char *)(*pict), dataLength);

        /* determine width and height of pict file */
        width  = (**pict).picFrame.right  - (**pict).picFrame.left;
        height = (**pict).picFrame.bottom - (**pict).picFrame.top;

        /* get the pict info from the pict handle to get the paltte */
        err = GetPictInfo(pict, &pictInfo, returnColorTable, 256, systemMethod, 0);
        if (err != noErr)
            DebugStr("\p GetPictInfo failed");

        originalbpp = pictInfo.depth;
        /* for some reason it is possible to to have a depth of 0 */
        if (originalbpp < 1)
            originalbpp = 32;

        /* To check for compressed images correctly a bottleneck must be used. */
        /* A simple hack is to check if the depth is 1 (GetPictInfo will return 1 */
        /* as the depth for compressed images*/
        packed = originalbpp == 1 ? 1 : 0;
        if (packed)
            originalbpp = 32;

        if (originalbpp <= 8)
        {
            /* indexed pict */
            bpp                 = 8;
            indexed             = 1;
            numcolours     = pictInfo.uniqueColors;
        }
        else
        {
            /* direct RBB pict */
            bpp                 = 32;
            indexed             = 0;
            numcolours     = 0;
        }

        /* fill in GINFO structure */
        ginfo = (GINFO *)galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */
            ginfo->signature        = GMAKEID('P','I','C','T'); /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->framenum         = 1;                        /* Number of frames (if more than 1, openframeblt required) */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */

            ginfo->bpp              = bpp;                      /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = originalbpp;
            ginfo->numcolours  = numcolours;          /* Number of colours in original indexed palette */

            if (indexed)
            {
                /* get color table */
                int index;
                CTabHandle ctab = pictInfo.theColorTable;
                for (index = 0; index <= (**ctab).ctSize; index++)
                {
                    ARGB color;
                    color.a = 0;

                    /* color table values are 16 (only want upper 8 bits) */
                    color.r = (unsigned char)((**ctab).ctTable[index].rgb.red >> 8);
                    color.g = (unsigned char)((**ctab).ctTable[index].rgb.green >> 8);
                    color.b = (unsigned char)((**ctab).ctTable[index].rgb.blue >> 8);

                    ginfo->colourtbl[index] = color;
                }
            }

            ginfo->subtype          = 0;    /* subtype */
            ginfo->packed           = packed;
            ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
            ginfo->framename[0]     = 0;    /* Null terminated name of frame/image */
        }

        /* finished with the pict handle and pictInfo tables */
        DisposeHandle((Handle)pict);
        DisposeHandle((Handle)pictInfo.theColorTable);

        /* handle errors */
        if (ginfo && err != noErr)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL PIC_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int width  = ginfo->width;
    int height = ginfo->height;

    CTabHandle ctab = NULL;
    BOFF* boff;
    int ok = 0;

    /* allocate buffer to store pict file into a PicHandle */
    int dataLength = glen(gx->gstream) - PICTHEADERSIZE;   /* subtract unused header */
    PicHandle pict = (PicHandle)NewHandle(dataLength);
    /* NOTE: try allocating in temp memory if alloc fails, see IM-Memory */
    if (pict == 0)
        return(ok);

    /* read pict from file into pict handle */
    gseek(gx->gstream, PICTHEADERSIZE);
    gread(gx->gstream, (char *)(*pict), dataLength);

    /* build color table if needed */
    if (ginfo->bpp==8)
        ctab = PICBuildColorTable(ginfo, ginfo->numcolours);

    /* draw PicHandle into a temp BOFF */
    boff = NewBOff(width, height, ginfo->bpp, ctab);
    StartToUseBOff(boff);
    {
        Rect bounds = {0, 0, height, width};
        SetBOff(boff);
        DrawPicture(pict, &bounds);
    }
    StopToUseBOff(boff);

    /* finished with the pict handle */
    DisposeHandle((Handle)pict);

    /* copy contents of GWorld into dest buffer */
    StartToUseBOff(boff);
    {
        char *source = (char *)GetBOffBasePtr(boff, 0, 0);
        if (source)
        {
            int row;
            int sourceRowBytes = GetBOffRowBytes(boff);

            /* clear the dest buffer, since we may not fill padding in */
            memset(dest, 0, rowbytes * height);

            for (row = 0; row < height; row++)
            {
                ok = PICReadRow(source, dest, width, ginfo->bpp);
                dest += rowbytes;
                source += sourceRowBytes;
            }
        }
    }
    StopToUseBOff(boff);

    DisposeBOff(boff);
    if (ctab)
        DisposeHandle((Handle)ctab);

    return(ok);
} /* readpic */


/****************************************************************/
/* Local Functions                                              */
/****************************************************************/

static int PICReadRow(char *source, char *dest, int width, int bpp)
{
    /* source is 8 or 32 bit boff buffer, dest is 8 or 32 gimex buffer */

    int ok = 0;
    int column;

    if (bpp == 8)
    {
        for (column = 0; column < width; column++)
            *(dest + column) = *(source + column);
        ok = 1;
    }
    else if (bpp == 32)
    {
        for (column = 0; column < width; column++)
            *((ARGB*)dest + column) = *((ARGB*)source + column);
        ok = 1;
    }

    return(ok);
}


/****************************************************************/
/* Utitity Functions                                            */
/****************************************************************/

CTabHandle PICBuildColorTable(GINFO *ginfo, int colorCount)
{
    /* rebuild a CTabHandle based on ginfo->colourtbl */

    CTabHandle ctab = NULL;
    ColorSpec* cspec;          /* current color spec being used */
    int        index;          /* color index (used in color spec array) */

    /* alloate handle for color table */
    ctab = (CTabHandle)NewHandle(8 + colorCount*sizeof(ColorSpec));

    /* fill in fields for a new colortable */
    (**ctab).ctSeed  = GetCTSeed();
    (**ctab).ctFlags = 0;
    (**ctab).ctSize  = colorCount - 1;

    /* fill in color data */
    cspec = (**ctab).ctTable;
    for (index = 0; index < colorCount; index++)
    {
        ARGB color = ginfo->colourtbl[index];

        cspec->value     = index;
        cspec->rgb.red   = color.r << 8 | color.r;
        cspec->rgb.green = color.g << 8 | color.g;
        cspec->rgb.blue  = color.b << 8 | color.b;

        cspec++;
    }

    return(ctab);
}

#else

/****************************************************************/
/* PC PICT                                                      */
/****************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "gimex.h"

/****************************************************************/
/* File IO                                                      */
/****************************************************************/

static void PICT_SkipRead(GSTREAM *g,int amount)
{
    int pos;

    pos = (int)gtell(g);
    gseek(g,pos+amount);
}

static void PICT_ReadUInt8(GSTREAM *g,UInt8 *d)
{
    char buf[4];
    unsigned int v;

    gread(g,buf,1);
    v = ggetm(buf,1);
    *d = (UInt8) v;
}

static void PICT_ReadUInt16(GSTREAM *g,UInt16 *d)
{
    char buf[4];
    unsigned int v;

    gread(g,buf,2);
    v = ggetm(buf,2);
    *d = (UInt16) v;
}

static void PICT_ReadUInt32(GSTREAM *g,UInt32 *d)
{
    char buf[4];
    unsigned int v;

    gread(g,buf,4);
    v = ggetm(buf,4);
    *d = (UInt32) v;
}

static void PICT_ReadInt16(GSTREAM *g,Int16 *d)
{
    char buf[4];
    unsigned int v;

    gread(g,buf,2);
    v = ggetm(buf,2);
    *d = (Int16) v;
}


static void PICT_ReadInt32(GSTREAM *g,Int32 *d)
{
    char buf[4];
    unsigned int v;

    gread(g,buf,4);
    v = ggetm(buf,4);
    *d = (Int32) v;
}

/****************************************************************/
/* Convert PixMap to Gimex info and bitmap                      */
/****************************************************************/

/* convert to ginfo or copys image */
/* PICT files may contain multiple pix maps. */
/* this function determines the overall dimensions and converts */
/* a portion at a time.                                         */

static int ConvertToGimex(PICT *pict,PICT_PIXMAP *pPixMap, UInt16 uNumColors, PICT_RGB16 *pColorTable, BytePtr pBitmap)
{
    GINFO *ginfo;
    int  ok=1;
    int  i;
    int  width;
    int  height;
    int  defaultx;
    int  defaulty;
    int  rightx;
    int  righty;
    int  destbpp;
    int  bpp;
    int  bpl;
    int  colours;
    int  y;
    int  x;
    char *d;
    char *s;

    ginfo = pict->ginfo;
    if (!pict->dest)         /* if dest set, dont change ginfo */
    {
        if (!pict->frames)  /* first frame */
        {
/*            memset(ginfo, 0, sizeof(GINFO)); */

            defaultx = pPixMap->Bounds.left;
            defaulty = pPixMap->Bounds.top;
            width  = abs(pPixMap->Bounds.right  - pPixMap->Bounds.left);
            height = abs(pPixMap->Bounds.bottom - pPixMap->Bounds.top);
        }
        else
        {
            /* expand frame */

            rightx = ginfo->defaultx + ginfo->width;
            righty = ginfo->defaulty + ginfo->height;

            defaultx = gmin(ginfo->defaultx,pPixMap->Bounds.left);
            defaulty = gmin(ginfo->defaulty,pPixMap->Bounds.top);

            rightx = gmax(rightx,pPixMap->Bounds.right);
            righty = gmax(righty,pPixMap->Bounds.bottom);

            width  = rightx-defaultx;
            height = righty-defaulty;
        }

        bpp = pPixMap->NumPlanes * pPixMap->CompSize;
        if (bpp==15) bpp = 16;
        colours = uNumColors;

        /* template for ginfo */

        ginfo->signature        = GMAKEID('P','I','C','T'); /* Type of file */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = 0;                        /* Current frame num */
        ginfo->width            = width;                    /* Width of bitmap in pixels */
        ginfo->height           = height;                   /* Height of bitmap in pixels */
        ginfo->subtype          = 0;                        /* subtype */
        ginfo->packed           = pPixMap->PkngType;        /* type of packing */
        ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = 0;            /* Number of bits in alpha channel */
        ginfo->redbits          = 8;            /* Number of bits in red channel */
        ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
        ginfo->bluebits         = 8;            /* Number of bits in blue channel */
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
            if (bpp<=16)
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 5;
            if (bpp>24)
                ginfo->alphabits    = 8;        /* Number of bits in alpha channel */
        }

        /* 1,2,4,8 bit grey or colour */

        else
        {
            if (!bpp)        bpp = 8;
            if (!colours)    colours = (1 << bpp);
            if (colours>256) colours = 256;

            ginfo->bpp         = 8;                        /* Bits per pixel (8 or 32) */
            ginfo->originalbpp = bpp;
            ginfo->numcolours  = colours;                  /* Number of colours in original indexed palette */

            if (!pict->frames)  /* first frame */
            {
                for (i=0; i<256; ++i)
                {
                    ginfo->colourtbl[i].a = 0;
                    ginfo->colourtbl[i].r = 0;
                    ginfo->colourtbl[i].g = 0;
                    ginfo->colourtbl[i].b = 0;
                }
            }

            if (pColorTable)
            {
                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = (unsigned char)(pColorTable[i].red>>8);
                    ginfo->colourtbl[i].g = (unsigned char)(pColorTable[i].green>>8);
                    ginfo->colourtbl[i].b = (unsigned char)(pColorTable[i].blue>>8);
                }
            }
            else    /* grey scale (white to black) */
            {
                ginfo->alphabits    = 0;
                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = (unsigned char)(255-(i*255/(colours-1)));
                    ginfo->colourtbl[i].g = (unsigned char)(255-(i*255/(colours-1)));
                    ginfo->colourtbl[i].b = (unsigned char)(255-(i*255/(colours-1)));
                }
            }
        }
    }
    else
    {
        /* now convert to gimex format in memory */

        destbpp = pPixMap->NumPlanes*pPixMap->CompSize;
        if (destbpp<=8) destbpp = 8;
        else            destbpp = 32;

        if (!pict->frames)  /* clear frame for first pixmap */
        {
            width  = ginfo->width;  /* overall frame size */
            height = ginfo->height;
            bpl = width*destbpp/8;

            for (y=0; y<height; ++y)
            {
                d = pict->dest+pict->rowbytes*y;
                memset(d,0,bpl);
            }
        }

        bpp = pPixMap->NumPlanes * pPixMap->CompSize;
        if (bpp<=8) bpp = 8;
        if (bpp==15) bpp=16;

        /* 8,16,24,or 32 bpp (source) */

        defaultx = pPixMap->Bounds.left-ginfo->defaultx;
        defaulty = pPixMap->Bounds.top -ginfo->defaulty;
        width  = abs(pPixMap->Bounds.right  - pPixMap->Bounds.left);
        height = abs(pPixMap->Bounds.bottom - pPixMap->Bounds.top);
        if (height>ginfo->height)
            height=ginfo->height;
        bpl = ((width+3)&~3)*bpp/8;

        for (y=0; y<height; ++y)
        {
            d = (char *) pict->dest + (defaulty+y)*pict->rowbytes + defaultx*destbpp/8;
            s = (char *) pBitmap + y*bpl;

            switch (bpp)
            {
                default:
                case 8:
                    memcpy(d,s,bpl);
                    break;

                case 16:
                    for (x=0; x<width; ++x)
                    {
                        unsigned int rgb;

                        rgb = ggetm(s,2);
                        ((ARGB *) d)->a = 255;
                        ((ARGB *) d)->r = (unsigned char)(SHIFT5TO8((rgb>>10)&31));
                        ((ARGB *) d)->g = (unsigned char)(SHIFT5TO8((rgb>>5)&31));
                        ((ARGB *) d)->b = (unsigned char)(SHIFT5TO8((rgb>>0)&31));
                        s += 2;
                        d += 4;
                    }
                    break;

                case 24:
                    for (x=0; x<width; ++x)
                    {
                        ((ARGB *) d)->a = 255;
                        ((ARGB *) d)->r = *(s+0);
                        ((ARGB *) d)->g = *(s+1);
                        ((ARGB *) d)->b = *(s+2);
                        s += 3;
                        d += 4;
                    }
                    break;

                case 32:
                    for (x=0; x<width; ++x)
                    {
                        ((ARGB *) d)->a = *(s+0);
                        ((ARGB *) d)->r = *(s+1);
                        ((ARGB *) d)->g = *(s+2);
                        ((ARGB *) d)->b = *(s+3);
                        s += 4;
                        d += 4;
                    }
                    break;
            }
        }
    }

    ++pict->frames;

    return(ok);
}

static BytePtr ExpandBuf(BytePtr PixBuf,BytePtr pBuf, UInt16* pLen, UInt16 BytesPerPix)
{
    BytePtr pSrc;
    BytePtr pDst;
    UInt16 i;

    pSrc = pBuf;
    pDst = PixBuf;

    switch(BytesPerPix)
    {
        case 16:
        case 8:
        case 32:
            return(pBuf);

        case 4:
            for (i = 0; i < *pLen; i++)
            {
                *pDst++ = (unsigned char)((*pSrc >> 4) & 15);
                *pDst++ = (unsigned char)(*pSrc++ & 15);
            }
            *pLen *= 2;
            break;

        case 2:
            for (i = 0; i < *pLen; i++)
            {
                *pDst++ = (unsigned char)((*pSrc >> 6) & 3);
                *pDst++ = (unsigned char)((*pSrc >> 4) & 3);
                *pDst++ = (unsigned char)((*pSrc >> 2) & 3);
                *pDst++ = (unsigned char)(*pSrc++ & 3);
            }
            *pLen *= 4;
            break;

        case 1:
            for (i = 0; i < *pLen; i++)
            {
                *pDst++ = (unsigned char)((*pSrc >> 7) & 1);
                *pDst++ = (unsigned char)((*pSrc >> 6) & 1);
                *pDst++ = (unsigned char)((*pSrc >> 5) & 1);
                *pDst++ = (unsigned char)((*pSrc >> 4) & 1);
                *pDst++ = (unsigned char)((*pSrc >> 3) & 1);
                *pDst++ = (unsigned char)((*pSrc >> 2) & 1);
                *pDst++ = (unsigned char)((*pSrc >> 1) & 1);
                *pDst++ = (unsigned char)(*pSrc++ & 1);
            }
            *pLen *= 8;
            break;

        default:
            ;
    }
    return(PixBuf);
}

static unsigned char *UnPackBits(GSTREAM *g,PICT_PIXMAP *pPixMap,UInt16 uRowBytes)
{
    BytePtr pLineBuf=NULL;
    BytePtr pExpandBuf=NULL;
    BytePtr pBitmap=NULL;
    BytePtr ptrBmp=NULL;
    BytePtr ptrEndBmp=NULL;
    BytePtr pBytePixels=NULL;
    UInt16 i,j,k,l;
    UInt32 uPixWidth,uPixHeight;
    UInt16 uLineLen;
    UInt16 uLen;
    UInt16 uBufLen;
    UInt16 uPkPixSize;
    UInt16 uLineBytes=0;

    if (pPixMap->PixSize <= 8)
        uRowBytes &= 0x7FFF;

    uPixWidth =  abs(pPixMap->Bounds.right - pPixMap->Bounds.left);
    uPixHeight = abs(pPixMap->Bounds.bottom - pPixMap->Bounds.top);

    uPixWidth = (uPixWidth+3)&~3;

    uPkPixSize = 1;

    if (pPixMap->PixSize == 16)
    {
        uPkPixSize = 2;
        uPixWidth *= 2;
    }
    else if (pPixMap->PixSize == 32)
    {
        /* could be 3 or 4 */

        uPixWidth *= pPixMap->NumPlanes;
    }

    if (uRowBytes == 0)
        uRowBytes = (UInt16)uPixWidth;

    if (uPixWidth*uPixHeight>0 && uPixWidth*uPixHeight<=4096*4096)
    {
        pBitmap = (BytePtr) galloc(uPixWidth*uPixHeight);
        pLineBuf = (BytePtr )galloc(uRowBytes + 100);    /* allocate 100 lines extra at the end since we might overshoot */
        pExpandBuf = (BytePtr )galloc(uPixWidth*4+100); // *4+100 for slopage
    }

    if (pBitmap && pLineBuf && pExpandBuf)
    {
        if ((uRowBytes < 8) || (pPixMap->PkngType == 1))
        {
             /* unpacked data */

            for (i=0; i<uPixHeight; ++i)
            {
                ptrBmp = pBitmap + (UInt32)i * (UInt32)uPixWidth;
                ptrEndBmp = ptrBmp+uPixWidth;
                uBufLen = (unsigned short)(gmin(uRowBytes,(ptrEndBmp-ptrBmp)*4));
                gread(g,pLineBuf,uBufLen);
                pBytePixels = ExpandBuf(pExpandBuf,pLineBuf,&uBufLen,pPixMap->PixSize);

                for (j = 0; j<uBufLen && ptrBmp<ptrEndBmp; j++)
                {
                     *ptrBmp++ = *pBytePixels++;
                }
            }
        }
        else
        {
            for (i = 0; i < uPixHeight; i++)
            {
                uLineBytes = 0;

                ptrBmp = pBitmap + (UInt32)i * (UInt32)uPixWidth;
                ptrEndBmp = ptrBmp+uPixWidth;

                /* Byte count for the current line */

                uLineLen = 0;
                if (uRowBytes > 250 /* || pPixMap->PixSize > 8 */)
                {
                    PICT_ReadUInt16(g,&uLineLen);
                }
                else
                {
                    PICT_ReadUInt8(g,(UInt8 *)&uLineLen);
                }

                /* Read the whole line into the line buffer */
                if (uLineLen>0 && uLineLen<(uRowBytes+100))
                    gread(g,pLineBuf,uLineLen);
                else
                    break;

                for (j = 0; j < uLineLen;)
                {
                    if (pLineBuf[j] & 0x80)
                    {

                        /* Flag-counter byte is negative: data is packed. */
                        /* No. of times data(next) byte is repeated is */
                        /* two's complement of the flag-counter byte. */

                        uLen    = (unsigned short)(((pLineBuf[j] ^ 255) & 255) + 2);
                        uBufLen = (unsigned short)(gmin(uPkPixSize,(ptrEndBmp-ptrBmp)*4));

                        pBytePixels = ExpandBuf(pExpandBuf,pLineBuf + j+1, &uBufLen, pPixMap->PixSize);

                        for (k = 0; k<uLen && ptrBmp<ptrEndBmp; k++)
                        {
                            for (l = 0; l<uBufLen && ptrBmp<ptrEndBmp; l++)
                            {
                                *ptrBmp++ = *pBytePixels++;
                            }
                            pBytePixels -= uBufLen;
                        }
                        j = (unsigned short)(j + 1 + uPkPixSize);
                    }
                    else
                    {
                        /* Flag-counter byte is positive: data is unpacked. */
                        /* No. of incompressible data bytes is flag-counter bytes. */

                        uLen = (unsigned short)((pLineBuf[j] & 255) + 1);

                        uBufLen = (unsigned short)(gmin(uLen * uPkPixSize,(ptrEndBmp-ptrBmp)*4));
                        pBytePixels = ExpandBuf(pExpandBuf,pLineBuf + j+1, &uBufLen,pPixMap->PixSize);

                        for (k = 0; k<uBufLen && ptrBmp<ptrEndBmp; k++)
                        {
                            *ptrBmp++ = *pBytePixels++;
                        }

                        j = (unsigned short)(j + uLen * uPkPixSize + 1);
                    }

                    uLineBytes = (unsigned short)(uLineBytes + uLen);
                }
            }
        }
    }
    if (pLineBuf)   gfree(pLineBuf);
    if (pExpandBuf) gfree(pExpandBuf);

    return(pBitmap);
}

static void SkipPolyOrRegion (GSTREAM *g)
{
    UInt16 count;

    PICT_ReadUInt16(g,&count);
    PICT_SkipRead(g,count-2);

}


static void ReadRect(GSTREAM *g,PICT_RECT *pRect)
{
    PICT_ReadInt16(g,&(pRect->top));
    PICT_ReadInt16(g,&(pRect->left));
    PICT_ReadInt16(g,&(pRect->bottom));
    PICT_ReadInt16(g,&(pRect->right));
}

static void ReadPixMap(GSTREAM *g,UInt16 OpCode,PICT_PIXMAP *pPixMap)
{
    if (/* (PictHdr2.HdrOpcodeData == -2) && */
       ( (OpCode == PICTOP_DIRECTBITSRECT) ||
         (OpCode == PICTOP_DIRECTBITSREGION) ) )
    {
        /* Version 2.0 Extended format */
        PICT_ReadUInt32(g,&(pPixMap->BaseAddr));
        PICT_ReadUInt16(g,&(pPixMap->RowBytes));
    }

    ReadRect(g,&(pPixMap->Bounds));
    PICT_ReadUInt16(g,&(pPixMap->Version));
    PICT_ReadUInt16(g,&(pPixMap->PkngType));
    PICT_ReadUInt32(g,&(pPixMap->PkdSize));
    PICT_ReadUInt32(g,&(pPixMap->SrcXRes));
    PICT_ReadUInt32(g,&(pPixMap->SrcYRes));
    PICT_ReadUInt16(g,&(pPixMap->PixType));
    PICT_ReadUInt16(g,&(pPixMap->PixSize));
    PICT_ReadUInt16(g,&(pPixMap->NumPlanes));
    PICT_ReadUInt16(g,&(pPixMap->CompSize));

    if ( (OpCode == PICTOP_BITSRECT)    ||
         (OpCode == PICTOP_BITSREGION)  ||
         (OpCode == PICTOP_PKBITSRECT)  ||
         (OpCode == PICTOP_PKBITSREGION) )
    {
        /* Version 2.0 only */
        /*  4 - plane bytes */
        /*  4 - pmColorTable bytes */
        /*  4 - pmReserved bytes */
        /* 12 - total bytes to skip */
        PICT_SkipRead(g,12);
    }
    else
    {
        if ( OpCode == PICTOP_DIRECTBITSRECT ||
             OpCode == PICTOP_DIRECTBITSREGION )
        {
            /* PictHdr2.HdrOpcodeData == -2 */
            /* Version 2.0 Extended format only */
            PICT_ReadUInt16(g,&(pPixMap->pmTable));
            PICT_ReadUInt16(g,&(pPixMap->pmReserved));
            PICT_SkipRead(g,8); /* since no color table for direct bits */
        }
    }
}

static BytePtr InterleaveRGB(PICT_PIXMAP *pPixMap,BytePtr pRawBits)
{
    UInt32 uWidth,uHeight,uLineSize;
    UInt32 aWidth;
    UInt32 i,j,l;
    BytePtr pLineBuf = NULL;
    BytePtr pSrc,pDst = NULL;
    UInt16 uNumPlanes;

    uLineSize  = 0;
    uHeight    = abs(pPixMap->Bounds.bottom - pPixMap->Bounds.top);
    uWidth     = abs(pPixMap->Bounds.right - pPixMap->Bounds.left);
    uWidth     = (uWidth+3)&~3;
    uNumPlanes = pPixMap->NumPlanes;

    if (pPixMap->PixSize == 32)
       uLineSize = uWidth * uNumPlanes; /* total bytes per scan line with 3 bytes per pixel */

    /* allocate a scan line buffer */
    /* allow for overshoot by 8 bytes */
    if ( (pLineBuf = (BytePtr) galloc(uLineSize + 10)) == NULL )
       {
        /*STR_Printf("Memory allocation failure in ordering RGBs\n"); */
       }

    /* initialize source byte pointer for each component */
    pSrc      = pRawBits;

    pDst = pLineBuf;

    aWidth = (uWidth+3)&~3;
    for (i = 0; i < uHeight; i++)
    {
        for (j = 0; j < uWidth; j++)
        {
            for (l = 0; l < uNumPlanes; l++)
                *pDst++= *( pSrc + (l * aWidth) + j );
        }

        /* set ptr to start of scan line buffer buffer */
        pDst = pLineBuf;

        /* copy the reordered scan line bytes back to the source bitmap */
        memcpy(pSrc,pDst,uLineSize);

        /* go to next scan line */
        pSrc += uLineSize;
    }

    if (pLineBuf)
       gfree(pLineBuf);

    return(pRawBits);

}

static int doDirectBitsRect(GSTREAM *g, PICT *pict)
{
    PICT_PIXMAP PixMap;
    PICT_RECT rectSrc,rectDst;
    Int16 iPixWidth;
    UInt16 uMode;
    BytePtr pBitmap = NULL;
    int ok=0;

    memset((void *) &PixMap,0,sizeof(PixMap));
    ReadPixMap(g,PICTOP_DIRECTBITSRECT,&PixMap);

    if (PixMap.PkngType != 4 &&
        PixMap.PkngType != 0 &&
        PixMap.PkngType != 1 &&
        PixMap.PkngType != 3)
    {
        /*STR_Printf("Unsupported Packing Type %u\n",PixMap.PkngType); */
    }

    if (PixMap.PixType != 16)
    {
        /*STR_Printf("PixelType %u != 16 for direct pixels\n",PixMap.PixType); */
    }

    iPixWidth = (short)(PixMap.Bounds.right - PixMap.Bounds.left);

    if (PixMap.PixSize == 16)
        iPixWidth *= 2;
    else
        if (PixMap.PixSize == 32)
            iPixWidth *= 3;

    ReadRect(g,&rectSrc);
    ReadRect(g,&rectDst);
    PICT_ReadUInt16(g,&uMode);

    pBitmap = UnPackBits(g,&PixMap,0);

    if (pBitmap)
    {
        if (PixMap.PixType == 16)
        {
            if ((PixMap.PkngType == 4) &&
                (PixMap.PixSize == 32) &&
                (PixMap.CompSize == 8) &&
                ((PixMap.NumPlanes == 3) || (PixMap.NumPlanes == 4)))
            {
                /* Reorder the 32-bit RGB RLE if packed */

                pBitmap = InterleaveRGB(&PixMap,pBitmap);
            }
        }

        ok = ConvertToGimex(pict,&PixMap,0,NULL,pBitmap);
        gfree(pBitmap);
    }

    return(ok);
}

static int doDirectBitsRegion(GSTREAM *g, PICT *pict)
{
    PICT_PIXMAP PixMap;
    PICT_RECT rectSrc,rectDst;
    Int16 iPixWidth;
    UInt16 uMode;
    BytePtr pBitmap = NULL;
    int ok=0;

    memset((void *) &PixMap,0,sizeof(PixMap));
    ReadPixMap(g,PICTOP_DIRECTBITSREGION,&PixMap);

    if (PixMap.PkngType != 4  &&
        PixMap.PkngType != 0 &&
        PixMap.PkngType != 1 &&
        PixMap.PkngType != 3 )
    {
         /*STR_Printf("Unsupported Packing Type %u",PixMap.PkngType); */
    }

    iPixWidth = (short)(PixMap.Bounds.right - PixMap.Bounds.left);

    if (PixMap.PixSize == 16)
        iPixWidth *= 2;
    else if (PixMap.PixSize == 32)
        iPixWidth *= 3;

    ReadRect(g,&rectSrc);
    ReadRect(g,&rectDst);
    PICT_ReadUInt16(g,&uMode);

    SkipPolyOrRegion(g);

    pBitmap = UnPackBits(g,&PixMap,0);
    if (pBitmap)
    {
        if (PixMap.PixType == 16)
        {
            if ((PixMap.PkngType == 4) &&
                (PixMap.PixSize == 32) &&
                (PixMap.CompSize == 8) &&
                ((PixMap.NumPlanes == 3) || (PixMap.NumPlanes == 4)))
            {
               /* Reorder the 32-bit RGB RLE if packed */

               pBitmap = InterleaveRGB(&PixMap,pBitmap);
            }
        }

        ok = ConvertToGimex(pict,&PixMap,0,NULL,pBitmap);
        gfree(pBitmap);
    }

    return(ok);
}


static PICT_RGB16 *ReadColorTable(GSTREAM *g,UInt16 *pNumColors)
{
    UInt32 ctSeed;   /* id for color table = 0 */
    UInt16 ctFlags;   /* flags word = 0 */
    UInt16 ctSize;      /* ctSize + 1 color table entries */
                        /* each entry = pixel value, red, green, blue: word */
    UInt16 value;
    UInt16 i;
    PICT_RGB16 *pColorTable = NULL;

    PICT_ReadUInt32(g,&ctSeed);
    PICT_ReadUInt16(g,&ctFlags);
    PICT_ReadUInt16(g,&ctSize);
    ctSize += 1;    /* number of entries */

    /* limit the color table to 256 entries */

    pColorTable = (PICT_RGB16 *) galloc(sizeof(PICT_RGB16) * ctSize);
    if (pColorTable)
    {
        for (i=0; i<ctSize; ++i)
        {
            PICT_ReadUInt16(g,&value);

            if (ctFlags & 0x8000)
                 value = i;

            if (value>=ctSize)
                value = (unsigned short)(ctSize-1);

            PICT_ReadUInt16(g,&(pColorTable[value].red));
            PICT_ReadUInt16(g,&(pColorTable[value].green));
            PICT_ReadUInt16(g,&(pColorTable[value].blue));
        }
    }
    *pNumColors = ctSize;
    return(pColorTable);

}

/* read pixmaps */

static int doPixMap(GSTREAM *g, PICT *pict, UInt16 uRowBytes, UInt16 OpCode, BoolEnum bIsRegion)
{
    PICT_PIXMAP PixMap;
    UInt16 uPixWidth;
    UInt16 uMode;
    UInt16 uNumColors;
    PICT_RGB16* pColorTable = NULL;
    PICT_RECT rectSrc,rectDst;
    BytePtr pBitmap = NULL;
    int ok=0;

    memset((void *) &PixMap,0,sizeof(PixMap));
    uRowBytes &= 0x7FFF; /* turn off high bit, we already know to use pixmap */
    ReadPixMap(g,OpCode,&PixMap);

    PixMap.RowBytes = uRowBytes;
    uPixWidth = (unsigned short)abs(PixMap.Bounds.right - PixMap.Bounds.left);
    pColorTable = ReadColorTable(g,&uNumColors);


    ReadRect(g,&rectSrc);       /* sourceRect */
    ReadRect(g,&rectDst);       /* destRect */
    PICT_ReadUInt16(g,&uMode);  /* mode */

    /* if bIsRegion then skip the bytes */

    if (bIsRegion == BOOL_TRUE)
       SkipPolyOrRegion(g);

    pBitmap = UnPackBits(g,&PixMap, uRowBytes);

    if (pBitmap)
    {
        ok = ConvertToGimex(pict,&PixMap,uNumColors,pColorTable,pBitmap);
        gfree(pBitmap);
    }
    if (pColorTable)
        gfree(pColorTable);

    return(ok);
}

/* skip bitmaps */

static int doBitmap(GSTREAM *g, PICT *pict, UInt16 uRowBytes, BoolEnum bIsRegion)
{
    PICT_RECT rectSrc,rectDst;
    PICT_PIXMAP PixMap;
    UInt16 uMode;
    BytePtr pBitmap;
    int ok=0;

    memset((void *) &PixMap,0,sizeof(PixMap));
    ReadRect(g,&(PixMap.Bounds));
    ReadRect(g,&rectSrc);
    ReadRect(g,&rectDst);
    PICT_ReadUInt16(g,&uMode);

    /* if bIsRegion then skip the bytes */

    if (bIsRegion == BOOL_TRUE)
        SkipPolyOrRegion(g);

    PixMap.PixSize = 1;

    pBitmap = UnPackBits(g,&PixMap, uRowBytes);
    if (pBitmap)
    {
        ok = ConvertToGimex(pict,&PixMap,2,0,pBitmap);
        gfree(pBitmap);
    }

    return(ok);
}

static int doBitsRegion(GSTREAM *g, PICT *pict)
{
    UInt16 uRowBytes;
    int ok=0;

    PICT_ReadUInt16(g,&uRowBytes);

    if ((uRowBytes & PICT_USEPIXMAP))
    {
        ok = doPixMap(g,pict,uRowBytes,PICTOP_PKBITSREGION, BOOL_TRUE);
    }
    else
        ok = doBitmap(g,pict,uRowBytes,BOOL_TRUE);

    return(ok);
}

static int doBitsRect(GSTREAM *g, PICT *pict)
{
    UInt16 uRowBytes;
    int ok=0;

    PICT_ReadUInt16(g,&uRowBytes);

    if ((uRowBytes & PICT_USEPIXMAP))
    {
        ok = doPixMap(g,pict,uRowBytes,PICTOP_PKBITSRECT,BOOL_FALSE);
    }
    else
        ok = doBitmap(g,pict,uRowBytes,BOOL_FALSE);

    return(ok);
}

static void SkipPattern(GSTREAM *g)
{
    UInt16 uPatType;
    PICT_PIXMAP PixMap;
    PICT_RGB16 *pColorTable = NULL;
    UInt16  uNumColors;
    BytePtr pBitmap=NULL;

    memset((void *) &PixMap,0,sizeof(PixMap));
    PICT_ReadUInt16(g,&uPatType);
    switch (uPatType)
    {
        case 2:
            PICT_SkipRead(g,8);  /* old pattern data */
            PICT_SkipRead(g,5);  /* RGB for pattern */
            break;

        case 1:
            PICT_SkipRead(g,8);  /* old pattern data */
            ReadPixMap(g,PICTOP_FILLPATTERN,&PixMap);
            pColorTable = ReadColorTable(g,&uNumColors);
            pBitmap = UnPackBits(g,&PixMap,PixMap.RowBytes);
            if (pBitmap)
                gfree(pBitmap);
            if (pColorTable)
                gfree(pColorTable);
            break;

        default:
            PICT_SkipRead(g,6);  /* old pattern data */
    }
}

static void SkipUnassignedOps(GSTREAM *g,UInt16 Opcode)
{
    if ((Opcode >= 0x0017) && (Opcode <= 0x0019))
    {
        ;/* nodata*/
    }
    else if ( ((Opcode >= 0x0024) && (Opcode <= 0x0027)) ||
              ((Opcode >= 0x002C) && (Opcode <= 0x002F)) ||
              ((Opcode >= 0x0092) && (Opcode <= 0x0097)) ||
              ((Opcode >= 0x009A) && (Opcode <= 0x009F)) ||
              ((Opcode >= 0x00A2) && (Opcode <= 0x00AF)) )
    {
        UInt16 count;

        PICT_ReadUInt16(g,&count);
        PICT_SkipRead(g,count);
    }
    else if (((Opcode >= 0x0035) && (Opcode <= 0x0037)) ||
             ((Opcode >= 0x0045) && (Opcode <= 0x0047)) ||
             ((Opcode >= 0x0055) && (Opcode <= 0x0057)) )
    {   PICT_SkipRead(g,8);    }
    else if ((Opcode >= 0x003D) && (Opcode <= 0x003F))
    { ; /* nodata */}
    else if ((Opcode >= 0x004D) && (Opcode <= 0x000F))
    { ; /* nodata */}
    else if ((Opcode >= 0x005D) && (Opcode <= 0x005F))
    { ; /* nodata */}
    else if ((Opcode >= 0x0065) && (Opcode <= 0x0067))
    {  PICT_SkipRead(g,12); }
    else if ((Opcode >= 0x006D) && (Opcode <= 0x006F))
    {  PICT_SkipRead(g,4);  }
    else if ((Opcode >= 0x0075) && (Opcode <= 0x0077))
    { ; /* data is previously specified polygon size */}
    else if ((Opcode >= 0x007D) && (Opcode <= 0x007F))
    { ; /* nodata */}
    else if ((Opcode >= 0x0085) && (Opcode <= 0x0087))
    { ; /* data is previously specified polygon size */}
    else if ((Opcode >= 0x008D) && (Opcode <= 0x008F))
    { ; /* nodata */}
    else if ((Opcode >= 0x00B0) && (Opcode <= 0x00CF))
    { ; /* nodata */}
    else if ((Opcode >= 0x00D0) && (Opcode <= 0x00FE))
    {
        UInt32 count;
        PICT_ReadUInt32(g,&count);
        PICT_SkipRead(g,count);
    }
    else if ((Opcode >= 0x0100) && (Opcode <= 0x7FFF))
    {
        UInt8 count;
        count = (UInt8) (Opcode & 0x1100);
        PICT_SkipRead(g,2*count);
    }
}

int parsepict(GSTREAM *g, GINFO *ginfo, char *dest, int rowbytes)
{
    UInt16   OpCode = PICTOP_NOP;
    PICT     pict;
    PICTHDR1 PictHdr1;
    PICTHDR2 PictHdr2;
    BoolEnum bThreeDO = BOOL_FALSE;
    int      Pict2=1;   /* type 2 by default */
    int      ok=0;
    int      opcodes=0;

    /* pict structure to keep track of things. */

    pict.ginfo = ginfo;
    pict.dest = dest;
    pict.rowbytes = rowbytes;
    pict.frames = 0;

    gseek(g,PICTHEADERSIZE);

    /* PICT Version 1.0 Header */

    PICT_ReadUInt16(g,&(PictHdr1.FileSize1));
    ReadRect(g,&(PictHdr1.Frame));
    PICT_ReadUInt16(g,&(PictHdr1.VerOpcode));

    /* 3DO Animator pict 2 */

    if (PictHdr1.VerOpcode == 0x0000)
    {
        bThreeDO = BOOL_TRUE;

        /* 3DO animator pict file */
        PICT_SkipRead(g,28);
        PICT_ReadUInt16(g,&(PictHdr1.VerOpcode));
    }

    /* PICT 1 */

    if (PictHdr1.VerOpcode & 0x1100)
    {
        /* This is version 1.0 */

        PictHdr1.VerNum = (unsigned short)(PictHdr1.VerOpcode & 0x0011);
        PictHdr1.VerOpcode &= 0x1100;
        Pict2 = 0; /* PICT 1 */
    }

    if (Pict2)
        PICT_ReadUInt16(g,&(PictHdr1.VerNum));

    if (Pict2 && (PictHdr1.VerNum==PICT_VERS_NUM_2))
    {
        Int32 iVer;

        /* Color QuickDraw Version 2.0 format */
        /* PICT Version 2.0 Additional Header 0x0C00 */

        PICT_ReadUInt16(g,&(PictHdr2.HdrOpcode));
        PICT_ReadInt32(g,&iVer);

        if (iVer == -1)
        {
            /* Version 2.0 */
            PictHdr2.HdrOpcodeData = -1;

            /* fixed point bounding box */
            PICT_SkipRead(g,16);
            PICT_ReadUInt32(g,&(PictHdr2.Reserved2));

            if (bThreeDO == BOOL_TRUE)
               PICT_SkipRead(g,2);  /* Def Hilite as doc. in QD2.0 */
        }
        else
        {
            if (iVer & 0xFFFE0000)
            {
                /* Version 2.0 Extended */

                PictHdr2.HdrOpcodeData = -2;

                PictHdr2.Reserved1 = (UInt16)(iVer & 0x0000FFFF);
                PICT_ReadUInt32(g,&(PictHdr2.HorzPixPerInch));
                PICT_ReadUInt32(g,&(PictHdr2.VertPixPerInch));
                ginfo->dpi = (float) ((double)PictHdr2.HorzPixPerInch / 65536.0);

                /* can't use ReadRect since frame is in different order */

                PICT_ReadInt16(g,&(PictHdr2.Frame.left));
                PICT_ReadInt16(g,&(PictHdr2.Frame.bottom));
                PICT_ReadInt16(g,&(PictHdr2.Frame.right));
                PICT_ReadInt16(g,&(PictHdr2.Frame.top));
                PICT_ReadUInt32(g,&(PictHdr2.Reserved2));
            }
        }
    }

    while ((OpCode != PICTOP_ENDOFPICTURE))
    {
        /* OpCodes are WORD aligned within the PICT file */

        OpCode = 0;
        if (Pict2)
        {
            if (gtell(g) & (UInt32)1)
                PICT_SkipRead(g,1);

            PICT_ReadUInt16(g,&OpCode);
        }
        else
        {
            PICT_ReadUInt8(g,(UInt8 *)&OpCode);
        }
        ++opcodes;
        if (opcodes>100)
            OpCode = PICTOP_ENDOFPICTURE;

        switch (OpCode)
        {
            /* desired Version 2.0 + opcodes */

            case PICTOP_BITSRECT           :
            case PICTOP_PKBITSRECT         :
                 ok = doBitsRect(g,&pict);
                 break;

            case PICTOP_BITSREGION         :
            case PICTOP_PKBITSREGION       :
                 ok = doBitsRegion(g,&pict);
                 break;

            case PICTOP_DIRECTBITSRECT     :
                 ok = doDirectBitsRect(g,&pict);
                 break;

            case PICTOP_DIRECTBITSREGION   :
                 ok = doDirectBitsRegion(g,&pict);
                 break;

            case PICTOP_NOP:
                 break;

            case PICTOP_ENDOFPICTURE       :
                 break;

            case PICTOP_CLIPREGION:
                 SkipPolyOrRegion(g);
                 break;

            case PICTOP_BKGPATTERN         :
                 SkipPattern(g);
                 break;

            case PICTOP_TEXTFONT           :
                 PICT_SkipRead(g,sizeof(Int16));
                 break;

            case PICTOP_TEXTTYPEFACE       :
                 PICT_SkipRead(g,sizeof(Byte));
                 break;

            case PICTOP_TEXTMODE           :
                 PICT_SkipRead(g,sizeof(Int16));
                 break;

            case PICTOP_TEXTEXTRASPACE     :
                 PICT_SkipRead(g,sizeof(Int32));
                 break;

            case PICTOP_PENSIZE            :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 break;

            case PICTOP_PENMODE            :
                 PICT_SkipRead(g,sizeof(Int16));
                 break;

            case PICTOP_PENPATTERN         :
            case PICTOP_FILLPATTERN        :
                 SkipPattern(g);
                 break;

            case PICTOP_OVALSIZE           :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 break;

            case PICTOP_ORIGINPOINT        :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 break;

            case PICTOP_TEXTSIZE           :
                 PICT_SkipRead(g,sizeof(Int16));
                 break;

            case PICTOP_FRGNDCOLOR         :
            case PICTOP_BKGNDCOLOR         :
                 PICT_SkipRead(g,sizeof(Int32));
                 break;

            case PICTOP_TEXTRATIO          :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 break;

            case PICTOP_PICTVERSION      :
                 PICT_SkipRead(g,sizeof(Byte));
                 break;

            case PICTOP_LINE               :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 break;

            case PICTOP_LINEFROM           :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 break;

            case PICTOP_SHORTLINE          :
                 PICT_SkipRead(g,sizeof(PICT_PT));
                 PICT_SkipRead(g,sizeof(Byte));
                 PICT_SkipRead(g,sizeof(Byte));
                 break;

            case PICTOP_SHORTLINEFROM      :
                 PICT_SkipRead(g,sizeof(Byte));
                 PICT_SkipRead(g,sizeof(Byte));
                 break;

            case PICTOP_LONGTEXT           :
            {
                  UInt8 count;

                  PICT_SkipRead(g,sizeof(PICT_PT));
                  /* text length */
                  PICT_ReadUInt8(g,&count);
                  /* text */
                  PICT_SkipRead(g,(int)count);
                  break;
            }

            case PICTOP_DHTEXT             :
            case PICTOP_DVTEXT             :
            {
                  UInt8 count;

                  PICT_SkipRead(g,sizeof(Byte));
                  /* text length */
                  PICT_ReadUInt8(g,&count);
                  /* text */
                  PICT_SkipRead(g,(int)count);
                  break;
            }

            case PICTOP_DHDV               :
            {
                  UInt8 count;

                  PICT_SkipRead(g,sizeof(Byte));
                  PICT_SkipRead(g,sizeof(Byte));
                  /* text length */
                  PICT_ReadUInt8(g,&count);
                  /* text */
                  PICT_SkipRead(g,(int)count);
                  break;
            }

            case PICTOP_FRAMERECT          :
            case PICTOP_PAINTRECT          :
            case PICTOP_ERASERECT          :
            case PICTOP_INVERTRECT         :
            case PICTOP_FILLRECT           :
                PICT_SkipRead(g,sizeof(PICT_RECT));
            case PICTOP_FRAMESAMERECT      :
            case PICTOP_PAINTSAMERECT      :
            case PICTOP_ERASESAMERECT      :
            case PICTOP_INVERTSAMERECT     :
            case PICTOP_FILLSAMERECT       :
                 break;

            case PICTOP_FAMEROUNDEDRECT    :
            case PICTOP_PAINTRNDRECT       :
            case PICTOP_ERASERNDRECT       :
            case PICTOP_INVERTRNDRECT      :
            case PICTOP_FILLRNDRECT       :
                 PICT_SkipRead(g,sizeof(PICT_RECT));
            case PICTOP_FRAMESAMERNDRECT   :
            case PICTOP_PAINTSAMERNDRECT   :
            case PICTOP_ERASESAMERNDRECT   :
            case PICTOP_INVERTSAMERNDRECT  :
            case PICTOP_FILLSAMERNDRECT    :
                 break;

            case PICTOP_FRAMEOVAL          :
            case PICTOP_PAINTOVAL          :
            case PICTOP_ERASEOVAL          :
            case PICTOP_INVERTOVAL         :
            case PICTOP_FILLOVAL           :
                 PICT_SkipRead(g,sizeof(PICT_RECT));
            case PICTOP_FRAMESAMEOVAL      :
            case PICTOP_PAINTSAMEOVAL      :
            case PICTOP_ERASESAMEOVAL      :
            case PICTOP_INVERTSAMEOVAL     :
            case PICTOP_FILLSAMEOVAL       :
                 break;

            case PICTOP_FRAMEARC           :
            case PICTOP_PAINTARC           :
            case PICTOP_ERASEARC           :
            case PICTOP_INVERTARC          :
            case PICTOP_FILLARC            :
                 PICT_SkipRead(g,sizeof(PICT_RECT));
            case PICTOP_FRAMESAMEARC       :
            case PICTOP_PAINTSAMEARC       :
            case PICTOP_ERASESAMEARC       :
            case PICTOP_INVERTSAMEARC      :
            case PICTOP_FILLSAMEARC        :
                 PICT_SkipRead(g,sizeof(Int16));
                 PICT_SkipRead(g,sizeof(Int16));
                 break;

            case PICTOP_FRAMEPOLYGON       :
            case PICTOP_PAINTPOLYGON       :
            case PICTOP_ERASEPOLYGON       :
            case PICTOP_INVERTPOLYGON      :
            case PICTOP_FILLPOLYGON        :
            {
                  UInt16 count;
                  PICT_ReadUInt16(g,&count);
                  PICT_SkipRead(g,count);
            }
            case PICTOP_FRAMESAMEPOLYGON   :
            case PICTOP_PAINTSAMEPOLYGON   :
            case PICTOP_ERASESAMEPOLYGON   :
            case PICTOP_INVERTSAMEPOLYGON  :
            case PICTOP_FILLSAMEPOLYGON    :
                 break;

            case PICTOP_FRAMEREGION        :
            case PICTOP_PAINTREGION        :
            case PICTOP_ERASEREGION        :
            case PICTOP_INVERTREGION       :
            case PICTOP_FILLREGION         :
                 SkipPolyOrRegion(g);
            case PICTOP_FRAMESAMEREGION    :
            case PICTOP_PAINTSAMEREGION    :
            case PICTOP_ERASESAMEREGION    :
            case PICTOP_INVERTSAMEREGION   :
            case PICTOP_FILLSAMEREGION     :
                 break;

            case PICTOP_SHORTCOMMENT       :
                 PICT_SkipRead(g,sizeof(Int16));
                 break;

            case PICTOP_LONGCOMMENT        :
            {
                  UInt16 count;

                  PICT_SkipRead(g,sizeof(Int16));
                  PICT_ReadUInt16(g,&count);
                  if (count<sizeof(ginfo->comment))
                      gread(g,ginfo->comment,count);
                  else
                      PICT_SkipRead(g,count);
                  break;
            }

            /* The following are IGNORED Version 2.0 Opcodes */

            case PICTOP_COLORBKGNDPATTERN  :
            case PICTOP_COLORPENPATTERN    :
            case PICTOP_COLORFILLPATTERN   :
                 SkipPattern(g);
                 break;

            case PICTOP_FRXNLPENPOS        :
            case PICTOP_EXTRASPACEPERCHAR  :
                 break;

            case PICTOP_RGBFRGNDCOLOR      :
                 PICT_SkipRead(g,sizeof(PICT_RGB16));
                 break;

            case PICTOP_RGBBKGNDCOLOR      :
                 PICT_SkipRead(g,sizeof(PICT_RGB16));
                 break;

            case PICTOP_HILIGHTMODE        :
                 /* no data */
                 break;

            case PICTOP_HILIGHTCOLOR       :
                 PICT_SkipRead(g,sizeof(PICT_RGB16));
                 break;

            case PICTOP_USEHILIGHTCOLORDEF :
                 /* no data */
                 break;

            case PICTOP_RGBOPCOLOR         :
                 PICT_SkipRead(g,sizeof(PICT_RGB16));
                 break;

            case PICTOP_HEADER             :
                 PICT_SkipRead(g,24);
                 break;

            case PICTOP_COMPRESSEDQUICKTIME:
            case PICTOP_UNCOMPRESSEDQUICKTIME:
            {
                  UInt32 count;
                  PICT_ReadUInt32(g,&count);
                  PICT_SkipRead(g,count-6);
                  break;
            }

            default:
                 SkipUnassignedOps(g,OpCode);

        }
    }

    return(ok);
}

/****************************************************************/
/* Information Functions                                        */
/****************************************************************/

/* NOTE - see ConvertToGimex for filling in GINFO */
int GCALL PIC_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    return(parsepict(gx->gstream, ginfo, dest, rowbytes));
}


GINFO * GCALL PIC_info(GINSTANCE *gx, int framenum)
{
    int ok;
    GINFO *ginfo;

    ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFO));
        ok = parsepict(gx->gstream, ginfo, (char *) 0, 0);
        if (!ok)
        {
            gfree(ginfo);
            ginfo = (GINFO *) 0;
        }
    }
    return(ginfo);
}
#endif




