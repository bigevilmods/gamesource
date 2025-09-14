// (c) 2003-2007 Electronic Arts Inc.



#include <windows.h>
#include <vfw.h>
#include "gimex.h"


struct VfwCodecInfo
{
    int width;
    int height;
    int srcBpp;
    int fourCC;
    int nColours;       // if palette attached (bpp < 8 only)
    int destBpp;
    int quality;
    int framesize; // CRUNCH
    int flags;
    int frameFlags;
    void * extraBihInfo;     // pointer to extra data written into the BITMAPINFOHEADER section of file
    int sizeExtraBihInfo;    // size of extra data (bytes)
    ARGB* palette;          // palette, for compression only, should be NULL if doesn't exist
    unsigned char* lastFrame;   // buffer large enough to hold a frame of uncompressed data, don't touch
};

typedef void VFWHANDLE;

///////////////////Decompression ///////////////////

VFWHANDLE *VfwBeginDecompress(VfwCodecInfo* format); // return: a handle to a decompressor

// size is length of compressed data
int VfwDecompress(unsigned char* dest, const unsigned char* src, VFWHANDLE* decompressor, VfwCodecInfo* format, int size);  // decompresses to default
int VfwDecompress32(unsigned char* dest, const unsigned char* src, VFWHANDLE* decompressor, VfwCodecInfo* format, int size); // decompresses to 32 bpp, RGBA
int VfwDecompress8(unsigned char* dest, const unsigned char* src,  VFWHANDLE* decompressor, VfwCodecInfo* format, int size); // decompress to 8 bpp

int VfwEndDecompress(VFWHANDLE* decompressor, VfwCodecInfo* format);

/////////////////////Compression //////////////////////

VFWHANDLE* VfwBeginCompress(VfwCodecInfo* format, bool* correctCompressor); // return : handle to a compressor
void       VfwSetDataRate(VFWHANDLE * hic,unsigned int quality,unsigned int framesize, float fps); // Note 2006.4.5 Frank API change to fix datarate and fps bugs
void       VfwConfigurePopup(VFWHANDLE * hic);

int VfwCompress(unsigned char* dest, const unsigned char* src, VFWHANDLE* compressor, VfwCodecInfo* format,
                int rowbytes, int keyframe, int frameNum, int frameSize);

int VfwEndCompress(VFWHANDLE* compressor, VfwCodecInfo* format);


////////////////////////Flags////////////////////////////

//Compression

#define FASTCOMPRESS    0x01
// tries to speed up compression, quality may suffer

// Decompression
#define FASTDECOMPRESS  0x02
// tries to speed up decompression, quality may suffer


