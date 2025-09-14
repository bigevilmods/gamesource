// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             ILBM - Amiga InterLeaved BitMap Gimex                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*               Original Latenight Code by Brad Gour               */
/*             Initial Gimex conversion by Erik Pauker              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.90 870101 BG Latenight 2                                    */
/*    1.00 940517 EP Based on latenight lbm module                  */
/*    1.01 940531 FB Updated to 1.03 Gimex spec                     */
/*    1.02 940614 FB partially updated to Gimex 1.5                 */
/*    1.03 940715 FB complete 1.5 update.  Considerable rewrite     */
/*    1.04 940715 FB started 24 bit, better parsing                 */
/*    1.05 940717 FB better parsing, error checking                 */
/*    1.06 940717 FB 24 bit, use mask on 24 bit                     */
/*    1.07 941001 FB casting for SGI                                */
/*    1.08 941027 FB export work buffer size increased for packing  */
/*    1.09 941028 FB move max rowbytes to lbmgimex.h                */
/*    1.10 941224 FB word align on write                            */
/*    1.11 941227 FB support x/y location                           */
/*    1.12 950131 FB treat -128 code as run for Photoshop style     */
/*    1.13 950516 FB openlbm return 1 or 0 (number of frames)       */
/*    1.14 950529 FB ginstance signature                            */
/*    1.15 951212 FB optimized getplanerow, alpharow                */
/*    1.16 961107 FB crng, anno & grab chunk import/export          */
/*    1.17 961120 FB crng dont write if 0 size                      */
/*    1.18 970618 GS check before writing optional fields           */
/*    1.19 970814 KO 24bit write                                    */
/*    1.20 980501 KO islbm made more strict, MAT files are similar  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1-8,24 bit                                                       */
/* Reentrant                                                        */
/* Files: lbmwrite.c lbmread.c lbmgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* ILBM files support 1-8 bit indexed, and 24 bit direct, with      */
/* RLE on bit planes.                                               */
/* There is an optional mask plane.                                 */
/* PBM is a packed pixel (chunky) 8 bit variant.                    */
/* Supports default position, center point and comment              */
/*                                                                  */
/* Dump of some dpaint colour range animation chunks:               */
/* 003a0: 43524e47 00000008 152a0099 0000000f  CRNG.....*......     */
/* 003b0: 43524e47 00000008 2b950132 0002101f  CRNG....+..2....     */
/* 003c0: 43524e47 00000008 172a0265 0000202f  CRNG.....*.e.. /     */
/* 003d0: 43524e47 00000008 2ebf04cb 0002303f  CRNG.....?.K..0?     */
/* 003e0: 43524e47 00000008 1f2a0999 0000404f  CRNG.....*....@O     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "lbmgimex.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

#define GOFFSETRED      (&((ARGB*)0)->r - (unsigned char*)(0))
#define GOFFSETGREEN    (&((ARGB*)0)->g - (unsigned char*)(0))
#define GOFFSETBLUE     (&((ARGB*)0)->b - (unsigned char*)(0))
#define GOFFSETALPHA    (&((ARGB*)0)->a - (unsigned char*)(0))

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

static long NextField(char *FilePtr)
{
    return(8+((ggetm(FilePtr+4,4)+1)&~1));
}

static void CnvtPalette(char *SrcPtr, GINFO *ginfo)
{
    int i;
    int ColourCount;
    unsigned char *ptr;

    ptr = (unsigned char *) SrcPtr + 8;       /* skip the "CMAP" and length field */

    ColourCount = (int)ggetm(SrcPtr+4,4)/3;

    if (ColourCount > 256)
    {
        ColourCount = 256;
    }

    ginfo->numcolours = ColourCount;

    for (i = 0; i < ColourCount; ++i)
    {
        ginfo->colourtbl[i].a = 255;
        ginfo->colourtbl[i].r = *ptr++;
        ginfo->colourtbl[i].g = *ptr++;
        ginfo->colourtbl[i].b = *ptr++;
    }
}

static void LBMUnPackBits(char **srcHandle, char **destHandle, int n)
{
    int  i;
    int  c;
    char val;
    char *s;
    char *d;

    s = *srcHandle;
    d = *destHandle;

    while (n>0)
    {
        c = (int) ((signed char) *s++);
        if (c >= 0)         /* (0-127) copy n+1 bytes literally */
        {
            ++c;
            n -= c;
            for (i = 0; i < c; ++i)
            {
                *d++ = *s++;
            }
        }
        else /* if (c != -128) \* (-1..-127) repeat byte -n+1 times *\ */
        {
            c = -c+1;
            n -= c;
            val = *s++;
            for (i = 0; i < c; ++i)
            {
                *d++ = val;
            }
        }
    }
    *srcHandle  = s;
    *destHandle = d;
}

static void SetAlphaRow(char *d, int width)
{
    int x;

    for (x=0; x<width; ++x)
    {
        *(d+GOFFSETALPHA) = (unsigned char)255;
        d += 4;
    }
}

static void ExtendAlphaRow(char *d, int width)
{
    int x;

    for (x=0; x<width; ++x)
    {
        *(d+GOFFSETALPHA) = ((signed char) *(d+GOFFSETALPHA))>>7;   /* asr */
        d += 4;
    }
}


static void GetPlaneRow(char *dest, char *source, int width, int plane, int planes)
{
    int i;
    unsigned char bits;
    char bitplane;
    int destmod=1;
    int channel;
    int bitsperchannel;
    int channeltbl[4];

    channeltbl[0] =      GOFFSETRED;
    channeltbl[1] =      GOFFSETGREEN;
    channeltbl[2] =      GOFFSETBLUE;
    channeltbl[3] =      GOFFSETALPHA;

    if (planes>=16)
    {
        bitsperchannel = gmin(8,planes/3);
        channel = channeltbl[plane/bitsperchannel];
        dest += channel;
        plane %= bitsperchannel;
        destmod = 4;
    }

    bitplane = (char) (1<<plane);
    for (i=0; i<width; i+=8)
    {
        bits = (unsigned char) *source++;
        if (bits&(1<<7)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<6)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<5)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<4)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<3)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<2)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<1)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<0)) *dest |= bitplane;
        dest += destmod;
    }
}

static int UnPackILBMPic(LBMHeaderStruct *Hdr, char *Data, GINFO *ginfo, char *dest, int rowbytes)
{
    int row;
    int plane;
    int planes;
    int width;
    int BytesPerPlaneRow;
    char *workdest;
    char *LinePtr;
    int MaxHeight;
    char *IFFWorkPtr;
    int ok=0;

    planes = LBMGETFIELD(Hdr->nPlanes);
    IFFWorkPtr = galloc(MAXROWBYTES);
    if (IFFWorkPtr)
    {
        /* calculate bytes per plane, rounded to word */

        width = LBMGETFIELD(Hdr->w);
        BytesPerPlaneRow = ((LBMGETFIELD(Hdr->w)+15)&~15)/8;

        /* unpack it one line at a time */

        LinePtr = dest;

        MaxHeight = ginfo->height;

        for (row = 0; row < MaxHeight; ++row)
        {
            memset(LinePtr, 0, rowbytes);
            for (plane=0; plane<planes; ++plane)
            {
                workdest = IFFWorkPtr;
                if (LBMGETFIELD(Hdr->compression))
                {
                    /* unpack a row */
                    LBMUnPackBits(&Data, &workdest, BytesPerPlaneRow);
                    workdest = IFFWorkPtr;
                }
                else
                {
                    memcpy(workdest, Data, BytesPerPlaneRow);
                    Data += BytesPerPlaneRow;
                }

                /* convert bit-plane info to byte per pixel format */
                GetPlaneRow(LinePtr, workdest, width, plane, planes);
            }

            /* if masking, DPaint seems to add an extra plane (?) */

            if (LBMGETFIELD(Hdr->masking)==1)
            {
                /* unpack a row */

                workdest = IFFWorkPtr;
                LBMUnPackBits(&Data, &workdest, BytesPerPlaneRow);

                if (planes>=16)
                {
                    GetPlaneRow(LinePtr, workdest, width, 31, 32);
                    ExtendAlphaRow(LinePtr, width);
                }
            }
            else
                if (planes>=16)
                    SetAlphaRow(LinePtr, width);

            LinePtr += rowbytes;
        }
        gfree(IFFWorkPtr);
        ok = 1;
    }
    return(ok);
}

static int UnPackPBMPic(LBMHeaderStruct *Hdr, char *Data, GINFO *ginfo, char *dest, int rowbytes)
{
    int row;
    int BytesPerPlaneRow;
    char *workdest;
    char *LinePtr;
    int MaxHeight;
    char *IFFWorkPtr;
    int ok=0;

    IFFWorkPtr = galloc(MAXROWBYTES);
    if (IFFWorkPtr)
    {

        BytesPerPlaneRow = LBMGETFIELD(Hdr->w);

        /* unpack it one line at a time */

        LinePtr = dest;

        MaxHeight = ginfo->height;

        for (row = 0; row < MaxHeight; ++row)
        {
            workdest = IFFWorkPtr;

            if (LBMGETFIELD(Hdr->compression))
            {
                /* unpack a row */

                LBMUnPackBits(&Data, &workdest, BytesPerPlaneRow);
                workdest = IFFWorkPtr;
                memcpy(LinePtr, workdest, rowbytes);
            }
            else
            {
                memcpy(LinePtr, Data, rowbytes);
                Data += BytesPerPlaneRow;
            }

            LinePtr += rowbytes;
        }
        gfree(IFFWorkPtr);
        ok = 1;
    }
    return(ok);
}

static int ProcessILBMBitmap(GINFO *ginfo, char *dest, int rowbytes, char *BasePtr, int process, long type)
{
    LBMHeaderStruct *Hdr;
    long Offset;
    long ID;
    long FormLength;
    char *BodyPtr;
    int bpp;
    int colours = 0;
    int width;
    int height;
    int packed;
    int defaultx;
    int defaulty;
    int subtype;
    int numanims=0;
    int ok=0;

    Hdr = 0L;
    BodyPtr = 0L;
    Offset = 12;

    FormLength = ggetm(BasePtr+4,4)+8;

    do
    {
        if (Offset>=FormLength || Offset<=0)
        {
            ok = -1;
        }
        else
        {
            ID = ggetm(BasePtr+Offset,4);
            if (ID==CMAP)
            {
                if (process==GETBHDR)
                    CnvtPalette(BasePtr+Offset, ginfo);
                ok |= 1;
            }
            else if (ID==BMHD)
            {
                Hdr = (LBMHeaderStruct *)(BasePtr+Offset+8);

                width    = LBMGETFIELD(Hdr->w);               /* Width of bitmap in pixels */
                height   = LBMGETFIELD(Hdr->h);               /* Height of bitmap in pixels */
                defaultx = LBMGETFIELD(Hdr->x);               /* Original X position */
                defaulty = LBMGETFIELD(Hdr->y);               /* Original Y position */
                bpp      = LBMGETFIELD(Hdr->nPlanes);         /* bpp of original bmp */
                packed   = LBMGETFIELD(Hdr->compression);     /* Type of packing on original image. 0 none, 1 run, n other */
                subtype = (type==PBM) ? 1 : 0;

                if (process==GETBHDR)
                {
                    /* template for ginfo */

                    ginfo->signature        = GMAKEID('I','L','B','M');   /* Type of file */
                    ginfo->size             = sizeof(GINFOSHP);         /* Length of GINFO */
                    ginfo->version          = GIMEX_VERSION;
                    ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
                    ginfo->width            = width;                    /* Width of bitmap in pixels */
                    ginfo->height           = height;                   /* Height of bitmap in pixels */
                    ginfo->subtype          = subtype;                  /* subtype 0 ilbm, 1 pbm */
                    ginfo->packed           = packed;
                    ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
                    ginfo->alphabits        = 0;            /* Number of bits in alpha channel */
                    ginfo->redbits          = 8;            /* Number of bits in red channel */
                    ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
                    ginfo->bluebits         = 8;            /* Number of bits in blue channel */
                    ginfo->defaultx         = defaultx;     /* Default Coordinate Point */
                    ginfo->defaulty         = defaulty;
                    ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
                    ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */

                    if (bpp>=15)
                    {
                        ginfo->bpp          = 32;       /* Bits per pixel (8 or 32) */
                        ginfo->originalbpp  = bpp;
                        ginfo->numcolours   = 0;        /* Number of colours in original indexed palette */
                        ginfo->redbits = ginfo->greenbits = ginfo->bluebits = bpp/3;
                        ginfo->alphabits    = 0;
                    }
                    else
                    {
                        if (!bpp)        bpp = 8;
                        if (bpp>8)       bpp = 8;
                        if (!colours)    colours = (1 << bpp);
                        if (colours>256) colours = 256;

                        ginfo->bpp          = 8;                        /* Bits per pixel (8 or 32) */
                        ginfo->originalbpp  = bpp;
                        ginfo->numcolours   = colours;                  /* Number of colours in original indexed palette */
                    }
                }
                if (bpp>=15)
                    ok |= 1;    /* dont need a palette */
                ok |= 2;
            }
            else if (ID==GRAB)
            {
                if (process==GETBHDR)
                {
                    ginfo->centerx = ggetm(BasePtr+Offset+8,2);
                    ginfo->centery = ggetm(BasePtr+Offset+10,2);
                }
            }
            else if (ID==CRNG)
            {
                if (process==GETBHDR)
                {
                    GINFOSHP *ginfoshp = (GINFOSHP *)ginfo;
                    int clock = ggetm(BasePtr+Offset+10,2);
                    int flag  = ggetm(BasePtr+Offset+12,2);
                    int start = ggetm(BasePtr+Offset+14,1);
                    int end   = ggetm(BasePtr+Offset+15,1);

                    ginfoshp->palanim[numanims].dir   = flag?-1:1;
                    ginfoshp->palanim[numanims].speed = (clock+273/2)/273;
                    ginfoshp->palanim[numanims].start = start;
                    ginfoshp->palanim[numanims].end   = end;
                    ++numanims;
                    ginfoshp->numanims = numanims;
                }
            }
            else if (ID==ANNO)
            {
                if (process==GETBHDR)
                {
                    long annolen = ggetm(BasePtr+Offset+4,4);
                    if (annolen>256) annolen = 256;
                    memcpy(ginfo->comment,BasePtr+Offset+8,annolen);
                }
            }
            else if (ID==BODY)
            {
                BodyPtr = BasePtr + Offset + 8;
                ok |= 4;
            }
            Offset += NextField(BasePtr + Offset);
        }
    } while ((ok&7)!=7);

    /* We've got all the necessary information */

    if (ok==7)
    {
        ok = 1;
        if (process==DECODEILBM)
        {
            if (type==ILBM)
            {
                ok = UnPackILBMPic(Hdr, BodyPtr, ginfo, dest, rowbytes);
            }
            else if (type==PBM)
            {
                ok = UnPackPBMPic(Hdr, BodyPtr, ginfo, dest, rowbytes);
            }
        }
    }
    else
        ok = 0; /* didnt find necessary chunks */

    return(ok);
}

static int ProcessILBM(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes, int process)
{
    long FLength;
    long CurOffset = 0;
    long HunkID;
    long HunkLen;
    int ok=0;
    char *ref=gx->gref;

    FLength = glen(gx->gstream);

    do
    {
        HunkID =  ggetm(ref,4);           /* get type of hunk */
        HunkLen = (ggetm(ref+4,4)+1)&~1;  /* get length of hunk (round to word) */

        if (HunkID==FORM && HunkLen>0 && HunkLen<FLength)
        {
            /* process a form */

            HunkID =  ggetm(ref+CurOffset+8,4);        /* get type of hunk */
            if (HunkID==ILBM || HunkID==PBM)
            {
                ok = ProcessILBMBitmap(ginfo, dest, rowbytes, ref + CurOffset, process, HunkID);
            }
        }
        CurOffset += HunkLen;

        if ((CurOffset>FLength) || (CurOffset<=0))
        {
            ok = -1;
        }
    } while (!ok);

    if (ok<0) ok = 0;

    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL LBM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('I','L','B','M');
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
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('I','L','B','M');   /* mac file system types used */
        strcpy(info->extensions[0], ".lbm");                /* extensions */
        strcpy(info->extensions[1], ".ilbm");
        strcpy(info->extensions[2], ".bbm");
#ifndef PLUGIN
        strcpy(info->extensions[3], ".iff");
#endif
        strcpy(info->authorstr,     "Frank Barchard");      /* authors */
        strcpy(info->versionstr,    "1.20");                /* version # */
        strcpy(info->shorttypestr,  "LBM");                 /* type */
        strcpy(info->wordtypestr,   "ILBM");                /* wordtype */
        strcpy(info->longtypestr,   "Amiga ILBM");           /* longtype */
    }
    return(info);
}

/* check for reasonable header: */

int GCALL LBM_is(GSTREAM *g)
{
    long FLength;
    char HunkHeader[12];
    long HunkID;
    long HunkLen;
    int ok=0;

    FLength = glen(g);

    gseek(g, 0);
    if (gread(g, HunkHeader, 12))         /* type and length */
    {
        HunkID =  ggetm(HunkHeader,4);    /* get type of hunk */
        HunkLen = ggetm(HunkHeader+4,4);  /* get length of hunk */

        if ((HunkID==FORM) && (HunkLen>0) && (HunkLen<=FLength))
        {
            ok = 75;
            switch(ggetm(HunkHeader+8, 4))
            {
                case ILBM:
                case PBM:
                case GMAKEID('A','N','I','M'):
                    ok = 100;
                    break;
            }
        }
    }

    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL LBM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    long FLength;
    char HunkHeader[8];
    long HunkID;
    long HunkLen;
    char *ref=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('I','L','B','M');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;

        FLength = glen(gx->gstream);

        gseek(gx->gstream, 0);
        gread(gx->gstream, HunkHeader, 8);          /* type and length */

        HunkID  = ggetm(HunkHeader,4);              /* get type of hunk */
        HunkLen = (ggetm(HunkHeader+4,4)+1)&~1;      /* get length of hunk (round to word) */

        if (HunkID==FORM && HunkLen>0 && HunkLen<=FLength)
        {
            ref = galloc(HunkLen+8);
            if (ref)
            {
                memcpy(ref, HunkHeader, 8);    /* copy over FORM header */
                ok = gread(gx->gstream, ref + 8, HunkLen);    /* read FORM */
                gx->gref = ref;
                if (ok)
                    ok = 1;
            }
        }
    }
    if (!ok && ref)
    {
        gfree((void *)ref);
        *gxptr = 0;
    }
    if (!ok && gx)
    {
        gfree((void *)gx);
        *gxptr = 0;
    }

    return(ok);
}

GINFO * GCALL LBM_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo;
    int ok;

    ginfo = (GINFO *) galloc(sizeof(GINFOSHP));
    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFOSHP));
        ok = ProcessILBM(gx, ginfo, (char *) 0, 0, GETBHDR);
        if (!ok)
        {
            gfree((void *) ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL LBM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok;

    ok = ProcessILBM(gx, ginfo, dest, rowbytes, DECODEILBM);

    return(ok);
}

int GCALL LBM_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        ok = 1;
        if (gx->gref)
            ok &= gfree((void *) gx->gref);
        ok &= gfree((void *) gx);
    }
    return(ok);
}


