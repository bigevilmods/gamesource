// (c) 2003-2007 Electronic Arts Inc.

#include <stdio.h>

#if defined(_MSC_VER) && !defined(_XBOX)
#  define __MSWINDOWS__
#endif

#ifdef __MSWINDOWS__
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include "avivfw.h"
#include <stdlib.h>


/* Internal functions*/
// swap case on FourCC in an attempt to find
// the codec.
static void ConvertFourCC(VfwCodecInfo* format)
{
    char fourCC = 0;
    int value   = 0;
    for(int i = 3; i >= 0; i--)
    {
        fourCC = ((format->fourCC)>>(i<<3))&0xff;
        fourCC = isupper(fourCC)?tolower(fourCC):toupper(fourCC);
        value |= ((int)(fourCC)<<(i<<3));
    }
    format->fourCC = value;
}

static VFWHANDLE* FindCompressor(BITMAPINFO* srcHeader, BITMAPINFO* destHeader, int srcBpp, int destBpp, VfwCodecInfo* format)
{
    int orig_fourCC = format->fourCC;
    srcHeader->bmiHeader.biBitCount = srcBpp;
    srcHeader->bmiHeader.biSizeImage = srcHeader->bmiHeader.biWidth * srcHeader->bmiHeader.biHeight * srcHeader->bmiHeader.biBitCount / 8;
    destHeader->bmiHeader.biBitCount = destBpp;
    destHeader->bmiHeader.biSizeImage = destHeader->bmiHeader.biWidth * destHeader->bmiHeader.biHeight * destHeader->bmiHeader.biBitCount / 8;

    format->destBpp = destBpp;
    VFWHANDLE* hic = (VFWHANDLE*) ICLocate(ICTYPE_VIDEO, 0L,
                                 (LPBITMAPINFOHEADER) srcHeader,
                                 (LPBITMAPINFOHEADER) destHeader,
                                (WORD)((format->flags&FASTCOMPRESS)?ICMODE_FASTCOMPRESS:
                                ICMODE_COMPRESS));
    if(!hic)  // try other case
    {
        ConvertFourCC(format);
            //srcHeader->bmiHeader.biCompression    = BI_RGB;
        destHeader->bmiHeader.biCompression = format->fourCC;
        hic = (VFWHANDLE*) ICLocate(ICTYPE_VIDEO, 0L,
                                 (LPBITMAPINFOHEADER) srcHeader,
                                 (LPBITMAPINFOHEADER) destHeader,
                                (WORD)((format->flags&FASTCOMPRESS)?ICMODE_FASTCOMPRESS:
                                ICMODE_COMPRESS));

        if(!hic)  // if other case failed the set back to original case
            format->fourCC = orig_fourCC;
    }

    return hic;

}

static void CreateBitmapInfoA(VfwCodecInfo* format, BITMAPINFOHEADER* header,bool includeFourCC)
{


    header->biSize          = sizeof(BITMAPINFOHEADER) ;
    header->biWidth         = format->width;
    header->biHeight            = format->height;
    header->biPlanes            = 1;
    header->biBitCount      = format->srcBpp;
    if(includeFourCC)
        header->biCompression   = format->fourCC;
    else
        header->biCompression = BI_RGB;
    header->biSizeImage     = format->width*format->height*format->srcBpp/8;
    header->biXPelsPerMeter = 0;
    header->biYPelsPerMeter = 0;
    header->biClrUsed       = format->nColours;
    header->biClrImportant  = 0;

}


// create w/ extra data if extra =1
static void CreateBitmapInfo(VfwCodecInfo* format, BITMAPINFOHEADER** header, int extra)
{

    int size = sizeof(BITMAPINFOHEADER);
    if (extra)
        size += format->sizeExtraBihInfo;


    *header = (BITMAPINFOHEADER *) galloc(size);


    (*header)->biSize           = size;
    (*header)->biWidth          = format->width;
    (*header)->biHeight         = format->height;
    (*header)->biPlanes         = 1;
    (*header)->biBitCount       = format->srcBpp;
    (*header)->biCompression    = format->fourCC;
    (*header)->biSizeImage      = format->width*format->height*format->srcBpp/8;
    (*header)->biXPelsPerMeter  = 0;
    (*header)->biYPelsPerMeter  = 0;
    (*header)->biClrUsed            = format->nColours;
    (*header)->biClrImportant   = 0;

    /* Copy the extra data */
    if(format->sizeExtraBihInfo>0 && extra)
    {
        memcpy( (((char *) (*header)) + 40), format->extraBihInfo, format->sizeExtraBihInfo);
    }

}


static void DestroyBitmapInfo(BITMAPINFOHEADER *bih)
{
    gfree(bih);
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

    return((int)srcp - (int)source);
}

/* Returns the number of bytes written to dest */
static int AVIencodergb(const void *source, void *dest, VfwCodecInfo *avi, int rowbytes)
{
    int ok=0;
    int i;
    int bpl;
    unsigned char *dstp;
    unsigned char *srcp;

    bpl = ((avi->width*avi->srcBpp+31)&~31)/8;

    if (rowbytes >= bpl)
    {
        /* Move srcp to the end of the data */
        srcp = (unsigned char *)source+((avi->height-1)*rowbytes);
        dstp = (unsigned char *)dest;

        for (i = 0; i < avi->height; i++)
        {
            ok = AVIencodergbline((const void *) srcp, (void *)dstp, avi->width, avi->srcBpp);
            srcp -= rowbytes;
            dstp += bpl;
        }

    }

    return((int)(dstp - (unsigned char *)dest));
}



static int AVIdecodergb(const void *source, void *dest, VfwCodecInfo *avi, int rowbytes)
{
    int ok=1;
    int i;
    int j;
    int bpl;
    unsigned char *dstp;
    unsigned char *srcp;

    if (avi)
    {
        if (rowbytes < avi->width) return(0);

        /* Move srcp to the end of the data */
        dstp = (unsigned char *)dest+((avi->height-1)*rowbytes);

        bpl = ((avi->width*avi->srcBpp+31)&~31)/8;

        for (i = 0; i < avi->height; i++)
        {
            srcp = (unsigned char *)(source)+i*bpl;
            switch(avi->srcBpp)
            {
                case 1:
                    for(j = 0; j < avi->width; j++)
                    {
                        *dstp++ = (unsigned char)((*srcp>>(7-(j&7)))&1);
                        if ((j&7)==7)
                            srcp++;
                    }
                    break;
                case 4:
                    for(j = 0; j < avi->width; j++)
                    {
                        if (j&1)
                        {
                            *dstp = (unsigned char)(*srcp&15);
                            srcp++;
                        }
                        else
                        {
                            *dstp = (unsigned char)((*srcp>>4)&15);
                        }
                        dstp++;
                    }
                    break;
                case 8:
                    for(j = 0; j < avi->width; j++)
                    {
                        *dstp = (unsigned char)ggeti(srcp,1);
                        dstp++;
                        srcp++;
                    }
                    break;
                case 16:
                    for(j = 0; j < avi->width; j++)
                    {
                        /* 555 */
                        ((ARGB *)dstp)->b = (unsigned char)((((*srcp)&31) * 255 + 16)/31);
                        ((ARGB *)dstp)->g = (unsigned char)((((((*(srcp+1))&3)<<3) | (((*srcp)>>5)&7)) * 255 + 16)/31);
                        ((ARGB *)dstp)->r = (unsigned char)(((((*(srcp+1))>>2)&31) * 255 + 16)/31);
                        ((ARGB *)dstp)->a = (unsigned char)255;
                        dstp+=4;
                        srcp+=2;
                    }
                    break;
                case 24:
                    for(j = 0; j < avi->width; j++)
                    {
                        ((ARGB *)dstp)->b = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->g = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->r = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->a = (unsigned char)255;
                        dstp+=4;
                    }
                    break;
                case 32:
                    for(j = 0; j < avi->width; j++)
                    {
                        ((ARGB *)dstp)->b = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->g = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->r = (unsigned char)ggeti(srcp++,1);
                        ((ARGB *)dstp)->a = (unsigned char)ggeti(srcp++,1);
                        dstp+=4;
                    }
                    break;
                default:
                    ok = 0;
                    break;
            }
            dstp-=2*rowbytes;
        }
    }
    return(ok);
}

/*
static BITMAPINFOHEADER* allocBih(int size)
{
    return ((BITMAPINFOHEADER *) galloc(size));
}

static void freeBih(BITMAPINFOHEADER* bih)
{
    gfree(bih);
}
*/

/* VFW API functions*/
VFWHANDLE *VfwBeginDecompress(VfwCodecInfo* format)
{
        BITMAPINFOHEADER *srcHeader;
        BITMAPINFOHEADER *dstHeader;
        //if(format->fourCC == 1162629709)
        //{
        //  format->fourCC = 1;
        //}

        CreateBitmapInfo(format,&srcHeader,1);  // extra data
        CreateBitmapInfo(format,&dstHeader,0);  // no extra data for dstHeader

        dstHeader->biCompression = BI_RGB;

        format->lastFrame = (unsigned char *) galloc(format->height*((format->width*format->srcBpp+31)&~31)/8);

        VFWHANDLE* hic = (VFWHANDLE *) ICLocate(ICTYPE_VIDEO, 0L,
                                      srcHeader,dstHeader,
                                      (format->flags & FASTDECOMPRESS) ? ICMODE_FASTDECOMPRESS : ICMODE_DECOMPRESS);





        ICINFO icinfo;
        ICGetInfo((HIC)hic, &icinfo, sizeof(icinfo));
//wprintf(L"icinfo.szDescription %s\n",(char *)&icinfo.szDescription[0]);
//wprintf(L"icinfo.szDriver %s\n",icinfo.szDriver);
//wprintf(L"icinfo.szName %s\n",icinfo.szName);


#if 0
        int size = ICDecompressGetFormat((HIC)hic, srcHeader, 0);

        BITMAPINFOHEADER *bihtemp = (BITMAPINFOHEADER*) galloc(size);

        ICDecompressGetFormat((HIC)hic,srcHeader,bihtemp);


//printf("bihtemp->biCompression 0x%x\n",bihtemp->biCompression);
//printf("bihtemp->biBitCount %d\n",bihtemp->biBitCount);

        gfree(bihtemp);

//printf("srcHeader->biCompression %x\n",srcHeader->biCompression);
#endif


        if(hic)
        {
            if (ICDecompressBegin((HIC)hic,srcHeader,dstHeader) == ICERR_OK)
            {
                DestroyBitmapInfo(srcHeader);
                DestroyBitmapInfo(dstHeader);
                return hic;
            }
        }
        DestroyBitmapInfo(srcHeader);
        DestroyBitmapInfo(dstHeader);
        return NULL;
}

int VfwDecompress(unsigned char* dest, const unsigned char* src, VFWHANDLE* decompressor, VfwCodecInfo* format, int size)
{
    if(decompressor)
    {
        BITMAPINFOHEADER *srcHeader;
        BITMAPINFOHEADER *dstHeader;
        CreateBitmapInfo(format,&srcHeader,1);
        CreateBitmapInfo(format,&dstHeader,0);

        dstHeader->biCompression        = BI_RGB;
        srcHeader->biSizeImage = size;

        if(ICDecompress((HIC)decompressor, format->flags, srcHeader, (void *) src, dstHeader, dest) == ICERR_OK)
        {
            DestroyBitmapInfo(srcHeader);
            DestroyBitmapInfo(dstHeader);
            return 1;   // dest has the decompressed data. format->srcBpp specifies bpp
        }
        DestroyBitmapInfo(srcHeader);
        DestroyBitmapInfo(dstHeader);
    }


    return 0;
}

// Decompress and convert dest to 32 bpp. ARGB. User needs to allocate width*height*4 bytes
int VfwDecompress32(unsigned char* dest, const unsigned char* src,  VFWHANDLE* decompressor, VfwCodecInfo* format, int size)
{
    if(decompressor)
    {
        BITMAPINFOHEADER *srcHeader;
        BITMAPINFOHEADER *dstHeader;
        CreateBitmapInfo(format,&srcHeader,1);
        CreateBitmapInfo(format,&dstHeader,0);
        dstHeader->biCompression        = BI_RGB;

        unsigned char* temp = format->lastFrame;

/*
        int size = ICDecompressGetFormat((HIC)decompressor, srcHeader, 0);
        BITMAPINFOHEADER *bihtemp = (BITMAPINFOHEADER*) galloc(size);
        ICDecompressGetFormat((HIC)decompressor,srcHeader,bihtemp);
*/
        srcHeader->biSizeImage = size;

        if(ICDecompress((HIC)decompressor, 0, srcHeader, (void *) src, dstHeader, temp) == ICERR_OK)
        {
            int ok = AVIdecodergb(temp, dest, format, format->width*4); // converts dest to 32 bit
            DestroyBitmapInfo(srcHeader);
            DestroyBitmapInfo(dstHeader);
            return ok;
        }
        DestroyBitmapInfo(srcHeader);
        DestroyBitmapInfo(dstHeader);
    }


    return 0;
}

int VfwDecompress8(unsigned char* dest, const unsigned char* src,  VFWHANDLE* decompressor, VfwCodecInfo* format, int size)
{
    if(decompressor)
    {
        BITMAPINFOHEADER *srcHeader;
        BITMAPINFOHEADER *dstHeader;
        CreateBitmapInfo(format,&srcHeader,1);
        CreateBitmapInfo(format,&dstHeader,0);
        dstHeader->biCompression        = BI_RGB;

        unsigned char* temp = format->lastFrame;
        srcHeader->biSizeImage = size;

        if(ICDecompress((HIC)decompressor, format->flags, srcHeader, (void *) src, dstHeader, temp) == ICERR_OK)
        {
            int ok = AVIdecodergb(temp, dest, format, format->width*1); // converts dest to 8 bit
            DestroyBitmapInfo(srcHeader);
            DestroyBitmapInfo(dstHeader);
            return ok;
        }
        DestroyBitmapInfo(srcHeader);
        DestroyBitmapInfo(dstHeader);
    }

    return 0;
}


int VfwEndDecompress(VFWHANDLE* decompressor, VfwCodecInfo* format)
{

    if(decompressor)
    {
        ICDecompressEnd((HIC)decompressor);
        ICClose((HIC)decompressor);
    }

    if(format->lastFrame)
    {
        gfree(format->lastFrame);

    }
    return(1);
}

// Check if I can popup a configure box, and if I
// can then pop it up to allow the user to select the
// interlace option if they need to in VP6
void VfwConfigurePopup(VFWHANDLE * hic)
{
  if(ICQueryConfigure((HIC)hic))
  {
    ICConfigure((HIC)hic,NULL);
  }
}

// Note 2006.4.5 Frank API change to fix datarate and fps bugs

void VfwSetDataRate(VFWHANDLE * hic,unsigned int quality,unsigned int framesize, float fps) // 29.97 fps recommended
{
  // Indeo 5 needs this message for data rate clamping.
  // The Morgan codec requires the message otherwise it assumes 100%
  // quality :(
  // The original version (2700) MPEG-4 V1 requires this message, period.
  // V3 (DivX) gives crap if we don't send it.  So special case it.

    int gop = 1000;
    char *gopstr = getenv("GOP");
    if (gopstr)
    {
        gop = atoi(gopstr);
    }

  ICINFO ici;
  ICGetInfo((HIC)hic, &ici, sizeof(ici));

  ICCOMPRESSFRAMES icf;
  memset(&icf, 0, sizeof(icf));

    icf.dwFlags   = (DWORD)&icf.lKeyRate;
  icf.lStartFrame = 0;
  icf.lFrameCount = 0x7FFFFFFF;//lFrameCount;
  icf.lQuality    = quality;
  icf.lDataRate   = (int)(framesize*fps);  // needs to be bytes per second not framesize.  note this should scale by FPS
  icf.lKeyRate    = gop;   // very few explicite keyframes... rely on auto keyframes
  icf.dwRate      = (int)(fps*100.0f); // 29.97 -> 2997
  icf.dwScale     = 100;

  ICSendMessage((HIC)hic, ICM_COMPRESS_FRAMES_INFO, (WPARAM)&icf, sizeof(ICCOMPRESSFRAMES));
}

/* Parameter correctCompressor returns true if the user specified compressor was found
   and false if a default compressor was found.
*/
VFWHANDLE* VfwBeginCompress(VfwCodecInfo* format, bool* correctCompressor) // return : handle to a compressor
{

    int compressionOriginal                       = 0;
    int compressionOriginalChangedCase = 0;
    int bppOriginal                                     = 0;
    int imageSizeOriginal                           = 0;
    int destOriginalBpp                              = 0;
    int ok                                                   = 0;
    ICINFO compinfo;

    BITMAPINFO *srcHeader = (BITMAPINFO*) galloc(sizeof(BITMAPINFOHEADER) + format->nColours * 4);
    BITMAPINFO *dstHeader = (BITMAPINFO*) galloc(sizeof(BITMAPINFOHEADER) + 256 * 4); // just in case codec wants to tack on a palette.
    CreateBitmapInfoA(format,&srcHeader->bmiHeader,false);
    CreateBitmapInfoA(format,&dstHeader->bmiHeader,true);
    srcHeader->bmiHeader.biCompression = BI_RGB;        // source is uncompressed

    // remember original values since they may be changed
    compressionOriginal = format->fourCC;
    bppOriginal = format->srcBpp;
    imageSizeOriginal = format->width*format->height*format->srcBpp;
    format->destBpp = dstHeader->bmiHeader.biBitCount;  // start with source bpp = dest bpp
    destOriginalBpp = format->destBpp;




    VFWHANDLE* hic = NULL;

    // copy the palette over if there is one
    memcpy(&srcHeader->bmiColors[0], format->palette, format->nColours * 4);
    memcpy(&dstHeader->bmiColors[0], format->palette, format->nColours * 4);

    hic = (VFWHANDLE*) ICLocate(ICTYPE_VIDEO, 0L,
                                (LPBITMAPINFOHEADER) srcHeader,
                                (LPBITMAPINFOHEADER) dstHeader,
                                (WORD)((format->flags&FASTCOMPRESS)?ICMODE_FASTCOMPRESS:ICMODE_COMPRESS));

    if(!hic)
    {
        int orig_fourCC = format->fourCC;
        ConvertFourCC(format);
        compressionOriginalChangedCase = format->fourCC;
        CreateBitmapInfoA(format,&srcHeader->bmiHeader,false);
        CreateBitmapInfoA(format,&dstHeader->bmiHeader,true);
        hic = (VFWHANDLE*) ICLocate(ICTYPE_VIDEO, 0L,
                                    (LPBITMAPINFOHEADER) srcHeader,
                                    (LPBITMAPINFOHEADER) dstHeader,
                                    (WORD)((format->flags&FASTCOMPRESS)?ICMODE_FASTCOMPRESS:ICMODE_COMPRESS));

        if(!hic)  // if other case failed the set back to original case
        {
            format->fourCC = orig_fourCC;
            CreateBitmapInfoA(format,&srcHeader->bmiHeader,false);
            CreateBitmapInfoA(format,&dstHeader->bmiHeader,true);
        }
    }

    //if (ICQueryConfigure((HIC) hic)) ICConfigure((HIC) hic, NULL);

    BITMAPINFO* desttype = (BITMAPINFO*) galloc(sizeof(BITMAPINFOHEADER) + 256 * 4  );  // just in case codec wants to attach a palette as well

    /* Here's the order of bit depths we use in locating an appropriate codec

            From Bpp    To Bpp(1)   To Bpp(2)   To Bpp(3)

    1.)         1           1           4           8
    2.)         4           4           8           1
    3.)         8           8           4           1

    1.)         16         16           24          32
    2.)         24         24           32          16
    3.)         32         32           24          16

    */

    int findBpp = format->srcBpp;   // start with original bit depth
    int count = 0;

        // note: kludge here to stop msvc from failing. This codec seems very badly behaved.
    while(!hic && count < 3 && (format->fourCC != 1129730893 && format->fourCC !=1668707181))   // we weren't able to find the requested compressor, try to change the bpp for same codec
        {
            switch(findBpp)
            {
                case 1:
                {
                    format->srcBpp = 1; // avi->Bpp is the 'from/source bpp'

                    //try 1 bpp
                    hic = FindCompressor(srcHeader,dstHeader, 1, 1, format);
                    if (hic)
                        break;

                    // try 4 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 1, 4, format);
                    if (hic)
                        break;

                    // try 8 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 1, 8, format);

                    findBpp = 4; // next bit depth to try is 4

                } // end 1 bpp
                break;

                case 4:
                {
                    format->srcBpp = 4; // avi->Bpp is the 'from/source bpp'

                    //try 4 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 4, 4, format);
                    if (hic)
                        break;

                    // try 8 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 4, 8, format);
                    if (hic)
                        break;

                    // try 1 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 4, 1, format);

                    findBpp = 8; // next bit depth to try is 8

                } // end 4 bpp
                break;



                case 8:
                {
                    format->srcBpp = 8; // avi->Bpp is the 'from/source bpp'

                    //try 8 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 8, 8, format);
                    if (hic)
                        break;

                    // try 4 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 8, 4, format);
                    if (hic)
                        break;

                    // try 1 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 8, 1, format);

                    findBpp = 1; // next bit depth to try is 8

                } // end 4 bpp
                break;

                case 16:
                {

                    format->srcBpp = 16;    // avi->Bpp is the 'from/source bpp'

                    //try 16bpp
                    hic = FindCompressor(srcHeader, dstHeader, 16, 16, format);
                    if (hic)
                        break;

                    // try 24bpp
                    hic = FindCompressor(srcHeader, dstHeader, 16, 24, format);
                    if (hic)
                        break;

                    // try 32 bpp
                    hic = FindCompressor(srcHeader, dstHeader, 16, 32, format);

                    findBpp = 24; // next bit depth to try is 24

                } // end 16 bpp
                break;


                case 24:
                {
                    format->srcBpp = 24;

                    // try 24
                    hic = FindCompressor(srcHeader,dstHeader, 24, 24, format);
                    if (hic)
                        break;

                    // try 32
                    hic = FindCompressor(srcHeader,dstHeader, 24, 32, format);
                    if (hic)
                        break;

                    // try 16
                    hic = FindCompressor(srcHeader, dstHeader, 24, 16, format);

                    findBpp = 32; // try 32 next


                } // end 24 bpp
                break;


                case 32:
                {
                    format->srcBpp = 32;

                    // try 32
                    hic = FindCompressor(srcHeader, dstHeader, 32, 32, format);
                    if (hic)
                        break;

                    // try 24
                    hic = FindCompressor(srcHeader, dstHeader, 32, 24, format);
                    if (hic)
                        break;


                    // try 16
                    hic = FindCompressor(srcHeader, dstHeader, 32, 16, format);

                    findBpp = 16;   // try 16 next

                } // end 32 bpp
                break;


            }   // end switch

            count++;

        }   // end while


        if(!hic)                // still no encoder, let's default to ANY codec installed on the system
        {
            ICINFO ticinfo;
            srcHeader->bmiHeader.biBitCount = bppOriginal;      // reset src header properties back to original
            srcHeader->bmiHeader.biSizeImage = imageSizeOriginal;
            format->destBpp = destOriginalBpp;
            format->srcBpp  = bppOriginal;

            // get the default

            hic = (VFWHANDLE *)ICLocate(ICTYPE_VIDEO,format->fourCC,
                                        (LPBITMAPINFOHEADER) srcHeader,
                                        0,
                                        (WORD)((format->flags&FASTCOMPRESS)?ICMODE_FASTCOMPRESS:ICMODE_COMPRESS));

            ICCompressGetFormat((HIC)hic, srcHeader, desttype);
            memcpy(dstHeader, desttype, sizeof(BITMAPINFOHEADER));

            // ensure that we will be able to decompress this thing again.
            VFWHANDLE * thic = (VFWHANDLE *)ICLocate(ICTYPE_VIDEO,0L,
                                        (LPBITMAPINFOHEADER) dstHeader,(LPBITMAPINFOHEADER) srcHeader,
                                        (WORD)((format->flags&FASTCOMPRESS)?ICMODE_FASTCOMPRESS:ICMODE_COMPRESS));

            if(hic && !thic)
            {
                ICClose((HIC)hic);
                hic = 0;
            }
            else
            {

                ICGetInfo((HIC)hic,&ticinfo,sizeof(ticinfo));

                format->destBpp = desttype->bmiHeader.biBitCount;
                format->fourCC  = ticinfo.fccHandler;//desttype->bmiHeader.biCompression;
            }
        }

        gfree(desttype);

        if(hic)
        {
            ok = ( ICCompressBegin( (HIC) hic, (LPBITMAPINFOHEADER) srcHeader, (LPBITMAPINFOHEADER) dstHeader) == ICERR_OK);
            if(ok)
            {
                ICGetInfo((HIC)hic, &compinfo, sizeof(ICINFO));
                format->flags = (int) compinfo.dwFlags;
                format->quality = (format->flags & VIDCF_QUALITY) ? format->quality : 10000;
//              if (format->flags & VIDCF_CRUNCH)
//                  printf("crunch!");
//              format->framesize = 20000;

                if(compinfo.fccHandler == (unsigned int)compressionOriginal || compinfo.fccHandler == (unsigned int)compressionOriginalChangedCase)
                {
                    *correctCompressor = true;
                }
                else
                {
                    *correctCompressor = false;
                }


                //format->lastFrame = (unsigned char *) galloc(format->height*((format->width*format->srcBpp+31)&~31)/8);
            }

        }

        gfree(srcHeader);
        gfree(dstHeader);
        if(ok)
            return hic;
        else
            return NULL;
}

int VfwCompress(unsigned char* dest, const unsigned char* src, VFWHANDLE* compressor,
                VfwCodecInfo* format, int rowbytes, int keyframe, int frameNum,
                int frameSize)
{


    unsigned char *tempdata;
    int ok=0;
    int bpl;
    int size;
    int quality;
    int requireslast;
    int crunch;

    LPBITMAPINFOHEADER lpbifrom;
    LPBITMAPINFOHEADER lpbito;

    BITMAPINFO* srcHeader = (BITMAPINFO*) galloc(sizeof(BITMAPINFOHEADER) + format->nColours*4);
    CreateBitmapInfoA(format,&srcHeader->bmiHeader,false);

    BITMAPINFOHEADER dstHeader;
    CreateBitmapInfoA(format,&dstHeader,true);
    //dstHeader.biSizeImage = frameSize; // testing... not working?
    dstHeader.biSizeImage = 0;
    dstHeader.biBitCount = format->destBpp;

    bpl = ((format->width*format->srcBpp+31)&~31)/8;
    tempdata = (unsigned char *) galloc(bpl * format->height);

    if( AVIencodergb(src, tempdata, format, rowbytes) )
    {

        quality         = (format->flags & VIDCF_QUALITY) ? format->quality : 0;
        requireslast    = (format->flags & VIDCF_TEMPORAL) && !keyframe;
        crunch          = (format->flags & VIDCF_CRUNCH);


        memcpy(srcHeader,&dstHeader,sizeof(BITMAPINFOHEADER));

        srcHeader->bmiHeader.biCompression  = BI_RGB;           // source is uncompressed
        srcHeader->bmiHeader.biBitCount = format->srcBpp;


        if(format->nColours)
        {
            memcpy(&srcHeader->bmiColors,format->palette,format->nColours*4);
        }


        lpbifrom = (LPBITMAPINFOHEADER) srcHeader;
        lpbito   = (LPBITMAPINFOHEADER) &dstHeader;

        format->frameFlags = 0;
        if (ICCompress((HIC)compressor,
                 ((keyframe?ICCOMPRESS_KEYFRAME:0)),
                 lpbito,                                /* Destination BITMAP INFO HEADER */
                 dest,                                  /* Destination data */
                 lpbifrom,                              /* Source BITMAP INFO HEADER */
                 tempdata,                              /* Source data */
                 0,                                     /* Reserved */
                 (LPDWORD)&format->frameFlags,          /* Return flags valid - AVIIF_KEYFRAME for key frame */
                 frameNum,                              /* Frame number */
                 frameSize,                             /* Requested size (Could be useful) */
                 quality,                               /* quality 0..10000 */
                 requireslast?lpbifrom:0,               /* Last frame BITMAPINFOHEADER (same as current frame) */
                 requireslast?format->lastFrame:0 )     /* Last frame if this is not a keyframe */
                == ICERR_OK)
         {

//printf("keyframe %d, format&AVIIF_KEYFRAME %x\n",keyframe,format->frameFlags&AVIIF_KEYFRAME);

                size = dstHeader.biSizeImage;
                if(size & 0x01)
                {
                    size++;
                }

                if(frameNum != 0)
                {
                    gfree(format->lastFrame);
                }
                format->lastFrame = tempdata;

                ok = size;
         }

    }

    gfree(srcHeader);
    return ok;

}

int VfwEndCompress(VFWHANDLE* compressor, VfwCodecInfo* format)
{
    if(compressor)
    {
        ICCompressEnd((HIC)compressor);
        ICClose((HIC)compressor);

        if(format->lastFrame)
            gfree(format->lastFrame);
    }
    return 1;
}


#endif  // end __MSWINDOWS__

