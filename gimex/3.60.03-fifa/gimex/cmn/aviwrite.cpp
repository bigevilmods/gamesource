// (c) 1994-2007 Electronic Arts Inc.

#include <stdio.h>
#include <string.h>
#include "gimex.h"
#include "avigimex.h"

// keyframe interval is normally 30, but can be changed with this macro
#define KEYFRAMEINTERVAL 30

#if defined(_MSC_VER) && !defined(_XBOX)
#  define __MSWINDOWS__
#endif

#ifdef __MSWINDOWS__
#include <avivfw.h>
#endif


/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static char charlower(char &c)
{
    if (c>='A' && c<='Z')
        c = c-'A'+'a';
    return c;
}

#define gtolower(a) (((a)>='A'&&(a)<='Z') ? ((a)-'A'+'a') : (a))

static unsigned int idlower(unsigned int upperid)
{
    char buf[4];
    gputm(buf,upperid,4);
    charlower(buf[0]);
    charlower(buf[1]);
    charlower(buf[2]);
    charlower(buf[3]);
    return ggetm(buf,4);
}

int AVIwriteheaders(GINSTANCE *gx, const GINFO *ginfo)
{
    int ok=0;
    char buf[56];
    int hsize;
    int i;
    int bpp;
    float fps;
    int TimeBetweenFrames;
    int palettesize;
    int numberofcolours;
    int datahandler;

    GSTREAM *g;
    AVIINFO *avi;

    g = gx->gstream;
    avi = (AVIINFO *) (gx->gref);


    datahandler = 0; /* default */
    if (avi->BitmapHeader->bmiHeader->Compression>1)
        datahandler = idlower(avi->BitmapHeader->bmiHeader->Compression);
    bpp = avi->Bpp;
    numberofcolours = ginfo->numcolours;
    if (bpp>8) numberofcolours = 0;
    palettesize = 4*numberofcolours;
    fps = 30.0f;
    if (ginfo->fps>=0.01f && ginfo->fps<=1000.0f)
        fps = ginfo->fps;
    TimeBetweenFrames = (int)(1000000.0f/fps);

    /* Output initial file chunk header (Leaving buf+4 for later) */
    gputm(buf,GMAKEID('R','I','F','F'),4);
    gputm(buf+8,GMAKEID('A','V','I',' '),4);
    ok = gwrite(g,buf,12);

    /* Output AVI header chunk header */
    hsize = 192+palettesize;
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,hsize,4);
    gputm(buf+8,GMAKEID('h','d','r','l'),4);
    ok = gwrite(g,buf,12);

    /* Output AVI header */
    gputm(buf,GMAKEID('a','v','i','h'),4);
    gputi(buf+4,56,4);
    ok = gwrite(g,buf,8);

    gputi(buf, TimeBetweenFrames, 4);   /* TimeBetweenFrames (1000000/15fps) */
    gputi(buf+4, 0, 4);                 /* MaxBps (Data size * fps, Added later) */
    gputi(buf+8, 0, 4);                 /* PaddingGranularity */
    gputi(buf+12, 0x00000010, 4);       /* Flags (Index exists) */
    gputi(buf+16, gx->frames, 4);       /* TotalFrames */
    gputi(buf+20, 0, 4);                /* InitFrames */
    gputi(buf+24, 1, 4);                /* Streams */
    gputi(buf+28, 0, 4);                /* SuggestedBufferSize (Max frame data size, Modified later) */
    gputi(buf+32, ginfo->width, 4);     /* Width */
    gputi(buf+36, ginfo->height, 4);    /* Height */
    gputi(buf+40, TimeBetweenFrames,4); /* TimeScale */
    gputi(buf+44, 0, 4);                /* DataRate */
    gputi(buf+48, 0, 4);                /* StartTime */
    gputi(buf+52, 0, 4);                /* DataLength */
    ok = gwrite(g,buf,56);

    /* Output video stream header chunk header */
    hsize -= 76;
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,hsize,4);
    gputm(buf+8,GMAKEID('s','t','r','l'),4);
    ok = gwrite(g,buf,12);

    /* Output video stream header */
    gputm(buf,GMAKEID('s','t','r','h'),4);
    gputi(buf+4,56,4);
    ok = gwrite(g,buf,8);

    // convert to lower case the header fourCC
    unsigned int newFourCC = 0;
    unsigned int curFourCC = avi->BitmapHeader->bmiHeader->Compression;
    for(int k = 3; k >= 0; k--)
    {
        unsigned char cC = gtolower(((curFourCC)>>(k<<3))&0xff);
        newFourCC       |= ((int)(cC)<<(k<<3));
    }


    gputm(buf, GMAKEID('v','i','d','s'), 4); /* DataType */
    gputi(buf+4,  newFourCC, 4);        /* DataHandler */
    gputi(buf+8,  0x00000000, 4);       /* Flags */
    gputi(buf+12, 0, 4);                /* Priority */
    gputi(buf+16, 0, 4);                /* InitialFrames */
    gputi(buf+20, TimeBetweenFrames, 4);/* TimeScale */
    gputi(buf+24, 0x000f4240, 4);       /* DataRate */
    gputi(buf+28, 0, 4);                /* StartTime */
    gputi(buf+32, gx->frames, 4);       /* DataLength */
    gputi(buf+36, 0x00080000, 4);       /* SuggestedBufferSize (See above)*/
    gputi(buf+40, avi->Quality, 4);     /* Quality (10000 Max) */
    gputi(buf+44, 0, 4);                /* SampleSize */
    gputi(buf+48, 0, 4);                /* Reserved */
    gputi(buf+52, 0, 4);                /* Reserved */
    ok = gwrite(g,buf,56);

    /* Output Bitmap info header */
    hsize = 40+palettesize;
    gputm(buf,GMAKEID('s','t','r','f'),4);
    gputi(buf+4,hsize,4);
    ok = gwrite(g,buf,8);

    gputi(buf, avi->BitmapHeader->bmiHeader->Size, 4);                   /* Size */
    gputi(buf+4, avi->BitmapHeader->bmiHeader->Width, 4);                /* Width */
    gputi(buf+8, avi->BitmapHeader->bmiHeader->Height, 4);               /* Height */
    gputi(buf+12, avi->BitmapHeader->bmiHeader->Planes, 2);              /* Planes */
    gputi(buf+14, avi->BitmapHeader->bmiHeader->BitCount, 2);            /* BitCount */
    gputi(buf+16, avi->BitmapHeader->bmiHeader->Compression, 4);         /* Compression (RGB) */
    gputi(buf+20, avi->BitmapHeader->bmiHeader->ImageSize, 4);           /* ImageSize */ /* KAZ */
    gputi(buf+24, avi->BitmapHeader->bmiHeader->XpelsMeter, 4);          /* XpelsMeter */
    gputi(buf+28, avi->BitmapHeader->bmiHeader->YpelsMeter, 4);          /* YpelsMeter */
    gputi(buf+32, avi->BitmapHeader->bmiHeader->NumberOfColours, 4);     /* NumberOfColours */
    gputi(buf+36, avi->BitmapHeader->bmiHeader->ImportantColours, 4);    /* ImportantColours */
    ok = gwrite(g,buf,40);

    /* Output the palette */
    for (i = 0; i < numberofcolours; i++)
    {
        gputi(buf,   avi->BitmapHeader->bmiColours[i].rgbBlue, 1);
        gputi(buf+1, avi->BitmapHeader->bmiColours[i].rgbGreen, 1);
        gputi(buf+2, avi->BitmapHeader->bmiColours[i].rgbRed, 1);
        gputi(buf+3, avi->BitmapHeader->bmiColours[i].rgbReserved, 1); /* Padding */
        ok = gwrite(g,buf,4);
    }

    /* Output movie chunk header */
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,0,4); /* Correct size to be given later (on close) */
    gputm(buf+8,GMAKEID('m','o','v','i'),4);
    ok = gwrite(g,buf,12);

    avi->DataPosition    = (unsigned int)gtell(g)-4 /* Starts before the 'movi' subid */;
    avi->CurrentPosition = (unsigned int)gtell(g);

    return(ok);
}


int AVIwriterle8(GINSTANCE *gx, const void *source, int rowbytes)
{

    AVIINFO *avi;
    GSTREAM *g;
    int ok=0;
    int i;
    int colcount;
    unsigned char val;
    unsigned char count;
    unsigned char codebuf[2];
    unsigned char *linebuf;
    unsigned char *srcp;

    if (gx)
    {
        avi=(AVIINFO *) (gx->gref);
        g = (GSTREAM *) (gx->gstream);

        if (rowbytes < avi->Width) return(0);

        /* Move srcp to the end of the data */
        srcp = (unsigned char *)source+((avi->Height-1)*rowbytes);

        linebuf = (unsigned char *) galloc(avi->Width+1);
        if (linebuf)
        {
            for (i=0;i<avi->Height;i++)
            {
                colcount=0;
                ok = 1;
                while((colcount < avi->Width) && ok)
                {
                    val = *srcp;
                    gputi(linebuf, val, 1);
                    count=1;

                    /* Record the run of non-matching values */
                    while((*(srcp+count)!=*(srcp+count-1)) && (colcount+count < avi->Width) && (count < 255))
                    {
                        gputi(linebuf+count, *(srcp+count), 1);
                        count++;
                    }
                    /* Don't include the last value */
                    count--;

                    if (count > 2) /* Output 'count' Absolute values */
                    {
                        gputi(codebuf,0,1);
                        gputi(codebuf+1,count,1);
                        ok = gwrite(g,codebuf,2);
                        ok = gwrite(g,linebuf, count);
                        /* 16bit align */
                        if (count & 0x01) gwrite(g,linebuf,1);
                    }
                    else /* Output Encoded values */
                    {
                        count=1;

                        /* Record the run of similar values */
                        while((*(srcp+count)==val) && (colcount+count < avi->Width) && (count < 255) /* Only 8 bits */)
                        {
                            count++;
                        }

                        *codebuf = count;
                        *(codebuf+1) = val;
                        ok = gwrite(g,codebuf,2);
                    }

                    /* Advance srcp */
                    srcp+=count;
                    colcount+=count;
                }
                /* Output EOL */
                gputi(codebuf,0x00,1);
                gputi(codebuf+1,0x00,1);
                ok = gwrite(g,codebuf,2);

                srcp-=2*rowbytes;

            }
            /* Output EOF */
            gputi(codebuf,0x00,1);
            gputi(codebuf+1,0x01,1);
            ok = gwrite(g,codebuf,2);

            gfree(linebuf);
        }
    }
    return(ok);
}

static int AVIencodergbline(const void *source, void *dest, int len, int bpp)
{
    int ok=1;
    int j;
    unsigned char *dstp;
    unsigned char *srcp;
    unsigned int r;
    unsigned int gr;
    unsigned int b;
    unsigned int rgb;

    dstp = (unsigned char *)dest;
    *dstp = 0;
    srcp = (unsigned char *)source;

    switch(bpp)
    {
        case 1:
            for(j = 0; j < len; j++)
            {
                *dstp |= (*srcp&1)<<(7-(j&7));
                if ((j&7)==7)
                {
                    dstp++;
                    *dstp = 0;
                }
                srcp++;
            }
            break;
        case 4:
            for(j = 0; j < len; j++)
            {
                if (j&1)
                {
                    *dstp |= (*srcp&15);
                    dstp++;
                }
                else
                {
                    *dstp = (unsigned char)((*srcp&15)<<4);
                }
                srcp++;
            }
            break;
        case 8:
            for(j = 0; j < len; j++)
            {
                gputi(dstp, *srcp, 1);
                srcp++;
                dstp++;
            }
            break;
        case 16:
            for(j = 0; j < len; j++)
            {
                r =  ((((ARGB *)srcp)->r)*31+128)/255;
                gr = ((((ARGB *)srcp)->g)*31+128)/255;
                b =  ((((ARGB *)srcp)->b)*31+128)/255;

                rgb = r<<10 | gr<<5 | b;
                gputi(dstp,rgb,2);
                dstp+=2;
                srcp+=4;
            }
            break;
        case 24:
            for(j = 0; j < len; j++)
            {
                gputi(dstp++, ((ARGB *)srcp)->b, 1);
                gputi(dstp++, ((ARGB *)srcp)->g, 1);
                gputi(dstp++, ((ARGB *)srcp)->r, 1);
                srcp+=4;
            }
            break;
        case 32:
            for(j = 0; j < len; j++)
            {
                gputi(dstp++, ((ARGB *)srcp)->b, 1);
                gputi(dstp++, ((ARGB *)srcp)->g, 1);
                gputi(dstp++, ((ARGB *)srcp)->r, 1);
                gputi(dstp++, ((ARGB *)srcp)->a, 1); /* Pad */
                srcp+=4;
            }
            break;
        default:
            ok = 0;
            break;
    }

    return(srcp - (unsigned char *)source);
}

static int AVIwritergb(GINSTANCE *gx, const void *source, int rowbytes)
{
    AVIINFO *avi;
    GSTREAM *g;
    int ok=0;
    int i;
    int bpl;
    unsigned char *linebuf;
    unsigned char *srcp;

    if (gx)
    {
        avi=(AVIINFO *) (gx->gref);
        g = (GSTREAM *) (gx->gstream);

        if (rowbytes < avi->Width) return(0);

        bpl = ((avi->Width*avi->Bpp+31)&~31)/8;

        /* Move srcp to the end of the data */
        srcp = (unsigned char *)source+((avi->Height-1)*rowbytes);

        linebuf = (unsigned char *) galloc(bpl);
        if (linebuf)
        {
            for (i = 0; i < avi->Height; i++)
            {
                ok = AVIencodergbline(srcp, linebuf, avi->Width, avi->Bpp);
                ok = gwrite(g,linebuf,bpl);
                srcp-=rowbytes;
            }
            gfree(linebuf);
        }
    }
    return(ok);
}


int AVIwriteindex(GINSTANCE *gx)
{
    char buf[16];
    int frames;
    int i;
    int ok=0;
    AVIINFO *avi;
    GSTREAM *g;

    avi = (AVIINFO *) gx->gref;
    g = gx->gstream;
    frames = gmin(gx->framenum+1,gx->frames);

    ok = gseek(g, avi->CurrentPosition);

    gputm(buf,GMAKEID('i','d','x','1'),4);
    gputi(buf+4,frames*16,4);
    ok = gwrite(g,buf,8);
    avi->CurrentPosition += 8;

    for(i = 0; i < frames; i++)
    {
        gputm(buf,avi->ChunkOffsetTbl[i].Identifier,4);
        gputi(buf+4,avi->ChunkOffsetTbl[i].Flags,4);
        gputi(buf+8,(unsigned int)avi->ChunkOffsetTbl[i].Offset,4);
        gputi(buf+12,avi->ChunkOffsetTbl[i].Length,4);
        gwrite(g,buf,16);
        avi->CurrentPosition += 16;
    }
    return(ok);
}


#ifdef __MSWINDOWS__ /* The following 4 functions are for windows use only */


    /*
     * Returns the number of bytes written to dest
     */
    static int AVIencodergb(const void *source, void *dest, AVIINFO *avi, int rowbytes)
    {
        int ok=0;
        int i;
        int bpl;
        unsigned char *dstp;
        unsigned char *srcp;

        bpl = ((avi->Width*avi->Bpp+31)&~31)/8;

        if (rowbytes >= bpl)
        {
            /* Move srcp to the end of the data */
            srcp = (unsigned char *)source+((avi->Height-1)*rowbytes);
            dstp = (unsigned char *)dest;

            for (i = 0; i < avi->Height; i++)
            {
                ok = AVIencodergbline((const void *) srcp, (void *)dstp, avi->Width, avi->Bpp);
                srcp -= rowbytes;
                dstp += bpl;
            }

        }

        return((int)(dstp - (unsigned char *)dest));
    }

    static int AVIcompressbegin(AVIINFO *avi)
    {

        VfwCodecInfo info;
        info.width     = avi->Width;
        info.height    = avi->Height;
        info.srcBpp    = avi->Bpp;
        info.fourCC    = avi->BitmapHeader->bmiHeader->Compression;
        info.nColours  = avi->BitmapHeader->bmiHeader->NumberOfColours;
        info.quality   = avi->Quality;
        info.framesize = avi->FrameSize;
        info.flags     = avi->Speed ? FASTCOMPRESS : 0;
        info.palette   = (ARGB *) &(avi->BitmapHeader->bmiColours[0]);

        bool temp;

        avi->hic = (void *) VfwBeginCompress(&info, &temp);
        VfwSetDataRate(avi->hic,avi->Quality,avi->FrameSize, avi->Fps);  // Note gx5.82 (newer) change compared to gx5.9 (older)
        if(avi->Configure)
          VfwConfigurePopup(avi->hic);
        // info now contains the proper destBpp,fourCC,quality to write into the file.
        // also the codec flags are present
        avi->BitmapHeader->bmiHeader->BitCount = info.destBpp;
        avi->BitmapHeader->bmiHeader->Compression = info.fourCC;
        //avi->Quality = info.quality;
        //avi->FrameSize = info.framesize;

        if(avi->hic)
        {
            // keep a copy of the format struct since it will be required later
            // by the compress funtion
            memcpy(&avi->format,&info,sizeof(VfwCodecInfo));
            return 1;
        }
        else
            return 0;
    }


    static int AVIcompressend(AVIINFO *avi)
    {
      return VfwEndCompress((VFWHANDLE *) avi->hic, (VfwCodecInfo *) &avi->format);
    }

    static int AVIencodeall(const void *source, void *dest, AVIINFO *avi, int rowbytes)
    {
        int keyFrameInterval = 25; /* default is Key frame every 25 frames this sould become user specified*/
        int keyFrame = avi->Framenum == 0;//((avi->Framenum % keyFrameInterval) == 0);

        int ok = VfwCompress((unsigned char *) dest, (unsigned char *) source, (VFWHANDLE *) avi->hic, (VfwCodecInfo *) &avi->format, rowbytes,
                             keyFrame /*!= 0*/, avi->Framenum, avi->FrameSize);

        // set avi->FrameFlags
        avi->FrameFlag = avi->format.frameFlags;


        return ok;
    }

    int AVIwriteall(GINSTANCE *gx, const void *source, int rowbytes)
    {
        int bpl;
        void *dest;
        AVIINFO *avi;
        int ok;

        avi = (AVIINFO*)gx->gref;

        bpl = ((avi->Width*avi->Bpp+31)&~31)/8;
        dest = galloc(bpl * avi->Height);
        ok = AVIencodeall(source, dest, avi, rowbytes);
        ok = gwrite(gx->gstream, dest, ok);
        gfree(dest);

        return(ok);
    }

#else /* Not windows */

    static int AVIcompressbegin(AVIINFO *avi)
    {
        return(0);
    }

    static int AVIcompressend(AVIINFO *avi)
    {
        return(0);
    }

    static int AVIALL_write(GINSTANCE *gx, const void *source, int rowbytes)
    {
        return(0);
    }

#endif


/*
 * Init avi information similar to all frames
 */
static int AVIcodecquery(AVIINFO *avi, const GINFO *ginfo)
{

    int ok=0;

    switch(avi->Compression)
    {
        case AVI_BI_RGB:
        case AVI_RGB:
            switch(avi->Bpp)
            {
                case 1:
                case 4:
                case 8:
                case 16:
                case 24:
                case 32:
                    avi->Encoder = AVIwritergb;
                    ok = 1;
                    break;
                default:
                    ok = 0;
                    break;
            }
            break;

        case AVI_BI_RLE8:
        case AVI_RLE8:
            if (avi->Bpp == 8)
            {
                avi->Encoder = AVIwriterle8;
                ok = 1;
            }
            else
            {
                avi->Compression = AVI_BI_RGB;
                avi->BitmapHeader->bmiHeader->Compression = AVI_BI_RGB;
                avi->Encoder = AVIwritergb;
                ok = 1;
            }
            break;

        default:

            avi->Quality = gmin(ginfo->quality*100, 10000);
            if(ginfo->framesize > 2)
              avi->FrameSize = gmin(ginfo->framesize, 100000);
            else
              avi->FrameSize = 100000;

            if (AVIcompressbegin(avi))
            {
#ifdef __MSWINDOWS__
                avi->Encoder = AVIwriteall;

#else
        avi->Encoder = AVIALL_write;
#endif
            }
            else
            {
                if (avi->Bpp <= 8)
                {
                    avi->Compression = AVI_BI_RLE8;
                    avi->BitmapHeader->bmiHeader->Compression = AVI_BI_RLE8;
                    avi->Encoder = AVIwriterle8;
                }
                else
                {
                    avi->Compression = AVI_BI_RGB;
                    avi->BitmapHeader->bmiHeader->Compression = AVI_BI_RGB;
                    avi->Encoder = AVIwritergb;
                }
                avi->Quality = 10000;
                avi->FrameSize = 20000; // large framesize
            }

            ok = 1;
            break;
    }

    return(ok);
}

static void AVIinitavi(AVIINFO *avi, const GINFO *ginfo)
{
    int bpp;
    int i;
    float fps;

    bpp = ginfo->originalbpp;
         if (bpp<=1)  bpp=1;
    else if (bpp<=4)  bpp=4;
    else if (bpp<=8)  bpp=8;
    else if (bpp<=16) bpp=16;
    else if (bpp<=24) bpp=24;
    else              bpp=32;

    if (bpp == 16)
    {
        if ((ginfo->redbits > 5) || (ginfo->greenbits > 6) || (ginfo->bluebits > 5))
            bpp = 24;
    }
    fps = 15.0f;
    if (ginfo->fps>=0.01f && ginfo->fps<=1000.0f)
        fps = ginfo->fps;

    avi->Width = ginfo->width;
    avi->Height = ginfo->height;
    avi->Bpp = bpp;
    avi->Fps = fps;

    int packed = ginfo->packed;
    if (packed<=1 && (ginfo->subtype&255)==1) // -vp6 dotsubtype
        packed = GMAKEID('V','P','6','0');

    if(ginfo->packed == AVI_BI_RGB || ginfo->packed == AVI_RGB || ginfo->packed == AVI_BI_RLE8
        || ginfo->packed == AVI_RLE8)
    {
        avi->Compression = ginfo->packed;
    }
    else if (ginfo->packed == GMAKEID('C','V','I','D')) // convert to lowercase cvid
    {
        gputm(&avi->Compression,GMAKEID('c','v','i','d'),4);
    }
    else
    {
        gputm(&avi->Compression,ginfo->packed,4);
    }

    //avi->Quality = 10000;               /* default to max quality */
    //avi->FrameSize = 20000;             /* default to large framesize */
    avi->FrameFlag = AVI_KEYFRAME;      /* default is Key Frame */
    avi->Speed = ginfo->subtype;        /* 0=normal, 1=faster */

    avi->Quality = gmin(ginfo->quality*100, 10000);
    if(ginfo->framesize < 2)
      avi->FrameSize = 20000;
    else
      avi->FrameSize = gmin(ginfo->framesize, 100000);


    avi->BitmapHeader->bmiHeader = (AVIBMIStruct* )galloc(sizeof(AVIBMIStruct));

    avi->BitmapHeader->bmiHeader->Size = 40;
    avi->BitmapHeader->bmiHeader->Width = ginfo->width;
    avi->BitmapHeader->bmiHeader->Height = ginfo->height;
    avi->BitmapHeader->bmiHeader->Planes = 1;
    avi->BitmapHeader->bmiHeader->BitCount = (unsigned short) bpp;
    avi->BitmapHeader->bmiHeader->Compression = avi->Compression;
    avi->BitmapHeader->bmiHeader->ImageSize = ginfo->width*ginfo->height*bpp/8;
    avi->BitmapHeader->bmiHeader->XpelsMeter = 0;
    avi->BitmapHeader->bmiHeader->YpelsMeter = 0;
    avi->BitmapHeader->bmiHeader->NumberOfColours = ginfo->numcolours;
    avi->BitmapHeader->bmiHeader->ImportantColours = 0;
    avi->Configure = ginfo->subtype&SUBCONFIGURE?1:0;

    /* Output the palette */
    for (i = 0; i < ginfo->numcolours; i++)
    {
        avi->BitmapHeader->bmiColours[i].rgbBlue     = ginfo->colourtbl[i].b;
        avi->BitmapHeader->bmiColours[i].rgbGreen    = ginfo->colourtbl[i].g;
        avi->BitmapHeader->bmiColours[i].rgbRed      = ginfo->colourtbl[i].r;
        avi->BitmapHeader->bmiColours[i].rgbReserved = ginfo->colourtbl[i].a;
    }

}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL AVI_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = -1;
        gx->frames = numframes;
        gx->signature=GMAKEID('.','A','V','I');
        gx->size=sizeof(GINSTANCE);

        gx->gref = galloc(sizeof(AVIINFO));
        if (gx->gref)
        {
            memset((void *) (gx->gref),0,sizeof(AVIINFO));
            ((AVIINFO*)(gx->gref))->BitmapHeader = (AVIVIDSStruct *) galloc(sizeof(AVIVIDSStruct));
            ((AVIINFO*)(gx->gref))->ChunkOffsetTbl = (AVIINDXStruct *) galloc(gx->frames*sizeof(AVIINDXStruct));
            *gxptr = gx;
            ok = 1;
        }
        else
        {
            gfree(gx);
        }
    }
    return(ok);
}


int GCALL AVI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    GSTREAM *g;
    AVIINFO *avi;
    int ok=1;
    GPOS fsize;
    char buf[8];

    if(gx)
    {
        g = gx->gstream;
        avi = (AVIINFO *) (gx->gref);

        if (gx->framenum == -1)
        {
            ok = gseek(g,(GPOS)0);
            AVIinitavi(avi, ginfo);
            ok = AVIcodecquery(avi, ginfo);
            if (ok)
            {
                ok = AVIwriteheaders(gx, ginfo);
            }
        }

        if (avi->Width != ginfo->width || avi->Height != ginfo->height)
            ok = 0;

        if (ok)
        {
            gx->framenum++;
            avi->Framenum = gx->framenum;

            ok = gseek(g, avi->CurrentPosition+8); /* Add 8 byte frame header later */

            if (avi->Encoder)
            {
                ok = avi->Encoder(gx, source, rowbytes);
            }

            /* Write frame header/footer */
            fsize = gtell(g) - avi->CurrentPosition -8/* Ignore header */;

            if (avi->MaxFrameSize < fsize) avi->MaxFrameSize = fsize;

            /* Must pad chunk !! */
            if (fsize & 0x01) gwrite(g,buf,1);

#ifdef __MSWINDOWS__
            unsigned int ChunkID = (avi->FrameFlag & AVIIF_KEYFRAME)?GMAKEID('0','0','d','b'):GMAKEID('0','0','d','c');
#else
            unsigned int ChunkID = GMAKEID('0','0','d','b');
#endif
            ok = gseek(g,avi->CurrentPosition);
            gputm(buf,ChunkID,4);
            gputi(buf+4,(unsigned int)(fsize),4);
            ok = gwrite(g,buf,8);

            /* Store the index of this chunk */
            if (gx->framenum < gx->frames)
            {
                avi->ChunkOffsetTbl[gx->framenum].Identifier = ChunkID;
                avi->ChunkOffsetTbl[gx->framenum].Flags = avi->FrameFlag;
                avi->ChunkOffsetTbl[gx->framenum].Offset = avi->CurrentPosition - avi->DataPosition;
                avi->ChunkOffsetTbl[gx->framenum].Length = (unsigned int)(fsize);
            }

            avi->CurrentPosition += fsize + 8;

        }
    }
    return(ok);
}


int GCALL AVI_wclose(GINSTANCE *gx)
{
    int ok=0;
    GPOS fsize;
    char buf[4];
    AVIINFO *avi;
    GSTREAM *g;

    if (gx)
    {
        avi=(AVIINFO*) (gx->gref);
        g = gx->gstream;
        if (avi)
        {
            if (avi->CurrentPosition)
            {
                /* Store the movi LIST chunk size */
                fsize = avi->CurrentPosition - avi->DataPosition;
                ok = gseek(g,avi->DataPosition-4);
                gputi(buf,(unsigned int)(fsize),4);
                gwrite(g,buf,4);

                /* Insert the index chunk at the end of the file */
                ok = AVIwriteindex(gx);

                /* Store the file chunk size */
                fsize = avi->CurrentPosition;
                ok = gseek(g,4);
                gputi(buf,(unsigned int)(fsize)-8,4);
                ok = gwrite(g, buf,4);

                /* Store the maximum frame size */
                gputi(buf,(unsigned int)(avi->MaxFrameSize), 4);
                ok = gseek(g,0x3C); /* SuggestedBufferSize in AVI header */
                ok = gwrite(g,buf,4);

                ok = gseek(g,0x90); /* SuggestedBufferSize in StreamHeader */
                ok = gwrite(g,buf,4);

                /* Store the maximum bytes per second */
                gputi(buf,(int)(avi->MaxFrameSize*avi->Fps),4);
                ok = gseek(g,0x24);
                ok = gwrite(g,buf,4);

                /* Update the frame number in the header if it is wrong */
                gx->framenum++;
                if (gx->framenum != gx->frames)
                {
                    /* Number of frames */
                    gputi(buf,gx->framenum,4);
                    ok = gseek(g,48); /* TotalNumberOfFrames in AVI header */
                    ok = gwrite(g,buf,4);

                    ok = gseek(g,84); /* DataLength in AVI header */
                    ok = gwrite(g,buf,4);

                    ok = gseek(g,140); /* DataLength in Stream header */
                    ok = gwrite(g,buf,4);
                }
            }
            if (avi->ChunkOffsetTbl) gfree(avi->ChunkOffsetTbl);
            if (avi->CurrentFrame) gfree(avi->CurrentFrame);
            if (avi->BitmapHeader->bmiHeader) gfree(avi->BitmapHeader->bmiHeader);
            if (avi->BitmapHeader) gfree(avi->BitmapHeader);
            if (avi->hic) AVIcompressend(avi);

            ok = gfree(avi);
        }
        ok = gfree((void *) gx);
    }
    return(ok);
}


