// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                         FLI - Movie Gimex                        */
/*                                                                  */
/*                       by Frank Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 950823 FB Initial version based on flicview and cflic    */
/*    1.01 951115 FB Sign extension bug in int align skip rows      */
/*    1.02 951123 FB export uncompressed flc                        */
/*    1.03 951201 FB identical consecutive frames 0 len ok          */
/*    1.04 960507 GS better check in gis function                   */
/*    1.05 970416 FB removed hex example in write for GCC SGI       */
/*    1.06 970822 KO Close writes framecount to header              */
/*    1.07 000720 FB write allow 10 more bytes for 'chunk' buffer   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* Reentrant                                                        */
/* Files: fliread.cpp, fliwrite.cpp, fligimex.h                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* FLI is an 8 bit animation format originally used by              */
/* Autodesk Animator.                                               */
/*                                                                  */
/* [header]   128 bytes                                             */
/*                                                                  */
/* Chunk format per frame:                                          */
/*                                                                  */
/*   [size][type][subchunks][reserved]                              */
/*      4      2       2         8                                  */
/*                                                                  */
/* SubChunk format:                                                 */
/*                                                                  */
/*   [size][type]                                                   */
/*      4      2                                                    */
/*                                                                  */
/* types:                                                           */
/*   4  FLITYPE_Colour256                                           */
/*   7  FLITYPE_intAlignedDelta                                     */
/*   11 FLITYPE_Colour64                                            */
/*   12 FLITYPE_ByteAlignedDelta                                    */
/*   13 FLITYPE_BlackScreen                                         */
/*   15 FLITYPE_ByteRunLength                                       */
/*   16 FLITYPE_NoCompression                                       */
/*   18 FLITYPE_PostageStamp                                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <stdlib.h>
#include "gimex.h"
#include "fligimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static void FLIloadPalette(char *chunk,ARGB *colourtbl,int maxrgb)
{
    int i;
    int j;
    int colours;
    int startColour;
    int num_packets;

    chunk += 6;
    num_packets = ggeti(chunk,2);
    chunk += 2;

    for (i=0; i<num_packets; ++i)
    {
        startColour = READBYTE();
        colours = READBYTE();

        if (!colours)
            colours = 256;

        for (j=0; j<colours; ++j)
        {
            colourtbl[startColour+j].a = 255;
            colourtbl[startColour+j].r = (unsigned char)(((unsigned char) chunk[0])*255/maxrgb);
            colourtbl[startColour+j].g = (unsigned char)(((unsigned char) chunk[1])*255/maxrgb);
            colourtbl[startColour+j].b = (unsigned char)(((unsigned char) chunk[2])*255/maxrgb);
            chunk += 3;
        }
    }
}

void FLIreadNoCompression(char *chunk, char *frame,int width,int height)
{
    int y;

    chunk += 6;
    for (y=0; y<height; ++y)
    {
        memcpy(frame,chunk,width);
        chunk += width;
        frame += width;
    }
}

void FLIreadByteRunLength(char *chunk, char *frame,int width,int height)
{
    int y;
    unsigned int widthCount;

    chunk += 6;
    for (y=0; y<height;++y)
    {
        widthCount=0;

        ++chunk;

        /* read pixel packets */

        while (widthCount<(unsigned int)width)
        {
            int size;
            unsigned int realSize;

            /* read size of packet segment */
            size = (int)(signed char) READBYTE();

            realSize = (unsigned int)abs(size);

            if((widthCount+realSize) > (unsigned int)width)
                realSize = width - widthCount;

            /* is size is positive is is run length */
            if (size < 0)
            {
                /* size is actual number of pixels to read */

                memcpy(frame,chunk,realSize);
                chunk += realSize;
            }
            else
            {
                /* do run length */
                unsigned char singleColour;

                singleColour = (unsigned char)ggeti(chunk,1);
                chunk += 1;

                memset(frame,singleColour,realSize);
            }
            frame += realSize;
            widthCount+=realSize;
        }
    }
}

void FLIreadintAligned(char *chunk, char *frame,int width)
{
    /* int Aligned Delta Compression */
    /* assume all lines */

    unsigned int lineCount=0;
    unsigned int currentLine=0;
    unsigned int control = 200;

    /* get number of lines to process */
    chunk += 6;
    lineCount = READWORD();

    while (lineCount)
    {
        int lastFlag = 0;
        unsigned char lastByte=0;
        unsigned int packetControl;
        int packetCount;
        unsigned int currentX;
        char * dest;
        int pcnt;

        /* get contrpol int */
        control = READWORD();
        packetControl = ((control&0xc000) >> 14);

        packetCount=0;

        switch( packetControl )
        {
            case 0:
                packetCount = (int)(control&0x3fff);
                break;
            case 2:
                lastFlag = 1;
                lastByte = (unsigned char)(control&0x00ff);
                packetCount = READWORD();
                break;
            case 3:
            {
                int jtmp = (int)(short)control;
                currentLine += abs(jtmp);
            }
                /* reset LOOP again */
                continue;

            default:
                break;         /*Illegal int Aligned Delta Data! */
        }

        currentX=0;

        /* process the packets for this line */
        for (pcnt=0; pcnt<packetCount; ++pcnt)
        {
            unsigned char columnSkip;
            signed char packetSize;
            int ntmp;
            unsigned int realSize;

            columnSkip = (unsigned char) READBYTE();
            packetSize = (signed char)   READBYTE();
            ntmp = (int)packetSize;
            ntmp = ntmp << 1;
            realSize = (unsigned int)abs(ntmp);

            currentX += (unsigned int)columnSkip;

            dest = frame+currentLine*width+currentX;

            currentX+=realSize;

            if (packetSize < 0)
            {
                unsigned int singlePixel = (unsigned int) READWORD();
                memset(dest, singlePixel, realSize);
            }
            else
            {
                memcpy(dest,chunk,realSize);
                chunk += realSize;
            }
        }
        /* see if we have to write the last Byte */
        if(lastFlag)
        {
            dest = frame+currentLine*width+width-1;
            *dest = lastByte;
        }
        ++currentLine;
        --lineCount;
    }
}

void FLIreadByteAligned(char *chunk, char *frame,int width)
{
    /* Byte Aligned Delta Compression */
    int firstY;
    unsigned int lineCount;
    int ycnt;
    char *dest;

    chunk += 6;

    firstY = READWORD();
    lineCount = READWORD();

    /* read lineCount lines form file */

    for (ycnt=0 ;ycnt<(int)lineCount; ++ycnt)
    {
        unsigned char currentX=0;
        unsigned char numPackets=(unsigned char)READBYTE();

        dest = frame+firstY*width+currentX;

        while (numPackets>0)
        {
            unsigned char columnSkip;
            signed char packetSize;
            int ntmp;
            unsigned int realSize;

            columnSkip=(unsigned char)READBYTE();
            packetSize=  (signed char)READBYTE();

            ntmp = (int)packetSize;

            realSize = (unsigned int)abs(ntmp);

            dest += columnSkip;

            if (packetSize<0)
            {
                unsigned char singlePixel=(unsigned char)READBYTE();
                memset(dest,singlePixel,realSize);
            }
            else
            {
                memcpy(dest,chunk,realSize);
                chunk += realSize;
            }
            dest += realSize;
            currentX = (unsigned char)(currentX + realSize);
            numPackets--;
        }
        firstY++;
    }
}

/****************************************************************/
/*  Decode a frame                                              */
/*  - a frame is composed of several subchunks                  */
/****************************************************************/

#define FLITYPE_Colour256 4
#define FLITYPE_intAlignedDelta 7
#define FLITYPE_Colour64 11
#define FLITYPE_ByteAlignedDelta 12
#define FLITYPE_BlackScreen 13
#define FLITYPE_ByteRunLength 15
#define FLITYPE_NoCompression 16
#define FLITYPE_PostageStamp 18

static int FLIdecodeframe(char *chunk, FLIINFO *fli)
{
    int  i;
    int  ok=1;
    int  subchunks;
    int  type;
    int  len;
    int  width;
    int  height;
    char *frame;

    width  = fli->width;
    height = fli->height;
    frame  = fli->current;

    subchunks = ggeti(chunk+6,2);
    chunk += 16;

    for (i=0; i<subchunks; ++i)
    {
        /* whattype of sub chunk is this? */

        len  = ggeti(chunk+0,4);
        type = ggeti(chunk+4,2);

        switch(type)
        {
            case FLITYPE_intAlignedDelta:
                /* int Aligned Delta compression */
                FLIreadintAligned(chunk,frame,width);
                fli->packed = 2;
                break;

            case FLITYPE_ByteAlignedDelta:
                /* Byte aligned Delta Compression */
                FLIreadByteAligned(chunk,frame,width);
                fli->packed = 2;
                break;

            case FLITYPE_ByteRunLength:
                /* Byte Run length Compression */
                FLIreadByteRunLength(chunk,frame,width,height);
                fli->packed = 1;
                break;

            case FLITYPE_NoCompression:
                FLIreadNoCompression(chunk,frame,width,height);
                fli->packed = 0;
                break;

            case FLITYPE_PostageStamp:
                /* ignore this chunk */
                break;

            case FLITYPE_BlackScreen:
                /*clear the screen to black */
                memset(frame,0,fli->width*fli->height);
                fli->packed=0;
                break;

            case FLITYPE_Colour64:
                FLIloadPalette(chunk,fli->colourtbl,63);
                break;

            case FLITYPE_Colour256:
                FLIloadPalette(chunk,fli->colourtbl,255);
                break;

            default:
                break;
        }
        chunk += len;
    }
    return(ok);
}

/****************************************************************/
/*  Get Chunk                                                   */
/*  - loads next chunk into memory.                             */
/*  - [size][type][subchunks][reserved]                         */
/*      4      2       2         8                              */
/****************************************************************/

static char *FLIgetchunk(GSTREAM *g, FLIINFO *fli)
{
    int  ok=0;
    int len;
    int filelen;
    int curpos;
    char buf[16];
    char *chunk=0;

    curpos  =  (int)gtell(g);
    filelen = (int)glen(g);
    if (curpos<filelen)
        ok = gread(g,buf,16);
    if (ok)
    {
        len = ggeti(buf,4);
        ok=0;
        if ((len+curpos)<=filelen)
        {
            chunk =  (char *) galloc(len);
            if (chunk)
            {
                memcpy(chunk,buf,16);
                ok = 1;
                if (len-16>0)
                    ok = gread(g,chunk+16,len-16);
                if (!ok)
                {
                    gfree(chunk);
                    chunk=0;
                }
            }
        }
    }
    return(chunk);
}

static int FLIseek(GINSTANCE *gx, FLIINFO *fli, int framenum)
{
    int  frame;
    int  ok=1;
    char *chunk;

    frame = gx->framenum;

    while (frame!=framenum && ok)
    {
        /* seek backward */

        if (framenum<frame)
        {
            gseek(gx->gstream,128);
            frame = -1;
        }

        ok=0;
        chunk = FLIgetchunk(gx->gstream, fli);
        if (chunk)
        {
            switch (ggeti(chunk+4,2))
            {
                case 0xf1fa:
                    ++frame;
                    ok = FLIdecodeframe(chunk, fli);
                    break;

                case 0xf100: /* prefix header */
                default:     /* unknown chunks */
                    ok = 1;
            }
            gfree(chunk);
        }
    }
    gx->framenum = frame;
    return(ok);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL FLI_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('F','L','I','C');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('F','L','I','C');       /* mac file system types used */
        strcpy(info->extensions[0], ".fli");                    /* extensions */
        strcpy(info->extensions[1], ".flc");                    /* extensions */
        strcpy(info->authorstr,     "Frank Barchard");          /* author */
        strcpy(info->versionstr,    "1.07");                    /* version # */
        strcpy(info->shorttypestr,  "FLI");                     /* type */
        strcpy(info->wordtypestr,   "FLIC");                    /* inttype */
        strcpy(info->longtypestr,   "FLIC Animation");          /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* size = filelen and type af11 or af12 */

int GCALL FLI_is(GSTREAM *g)
{
    FLIHeaderStruct fliheader;
    int ok=0;
    int flen;

    gseek(g,(GPOS)0);
    flen = (int)glen(g);
    memset(&fliheader,0,sizeof(fliheader));
    if (gread(g,&fliheader,gmin(flen,128)))
    {
        if (FLIGETFIELD(fliheader.type)==0xaf11
         || FLIGETFIELD(fliheader.type)==0xaf12)
        {
            ok = 60;
            if ((FLIGETFIELD(fliheader.size)==(unsigned int)flen))
                ok += 40;
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL FLI_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    FLIINFO *fli;
    int  ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('F','L','I','C');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = -1;  /* keep track of which frame were pointing at */

        gseek(g,(GPOS)0);

        gx->gref = galloc(sizeof(FLIINFO));
        if (gx->gref)
        {
            fli = (FLIINFO *) gx->gref;
            memset((void *) fli,0,sizeof(FLIINFO));
            if (gread(g,&fli->header,128))
            {
                fli->width =  FLIGETFIELD(fli->header.width);
                fli->height = FLIGETFIELD(fli->header.height);
                fli->current = (char *) galloc(fli->width*fli->height);
                if (fli->current)
                {
                    fli->packed = 1;
                    fli->colours = 256;
                    ok = gx->frames = FLIGETFIELD(fli->header.frames);
                    gx->framenum = -1;
                }
            }
        }
    }
    return(ok);
}

GINFO * GCALL FLI_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    FLIINFO *fli;
    int  ok;
    int  width;
    int  height;

    fli = (FLIINFO *) gx->gref;

    ok = FLIseek(gx, fli, framenum);

    if (ok)
    {
        width =  fli->width;
        height = fli->height;

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('F','L','I','C'); /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 /* Current frame */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* Internal format */
            ginfo->packed           = fli->packed;              /* Type of packing on original image. 0=key, 1=motion */
            ginfo->quality          = 100;
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
            ginfo->bpp              = 8;    /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = 8;    /* Bits per pixel in original image (1 to 32) */
            ginfo->numcolours       = fli->colours;         /* Number of colours in original indexed palette */

            memcpy(ginfo->colourtbl,fli->colourtbl,256*4);
        }
    }
    return(ginfo);
}

int GCALL FLI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int  bpl;
    int  y;
    int  ok;
    char *d;
    char *s;
    FLIINFO *fli;

    /* seek to frame and skip header if there is one. */

    fli = (FLIINFO *) gx->gref;
    ok = FLIseek(gx, fli, ginfo->framenum);

    if (ok)
    {
        bpl = ginfo->width;
        d = dest;
        s = fli->current;
        for (y=0; y<ginfo->height; ++y)
        {
            memcpy(d,s,bpl);
            s += bpl;
            d += rowbytes;
        }
    }
    return(ok);
}

int GCALL FLI_close(GINSTANCE *gx)
{
    FLIINFO *fli;
    int ok=0;

    if (gx)
    {
        if (gx->gref)
        {
            fli = (FLIINFO *) gx->gref;
            if (fli->current)     gfree(fli->current);

            gfree((void *)gx->gref);
            gx->gref=0;
        }
        ok = gfree((void *) gx);
    }

    return(ok);
}

