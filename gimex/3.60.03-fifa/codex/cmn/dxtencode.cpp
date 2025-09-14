// (c) 2007 Electronic Arts Inc.

#include "eadxt.h"
#include "eadxt.cpp"

#ifndef __S3WRITE2
#define __S3WRITE2  1

#if defined(_MSC_VER) && _MSC_VER>=1400 && !defined(__ICL)
#define ENABLE_OMP 1
#endif

// remove CODEXTIMING if not timing

#if defined(_MSC_VER)
//#define CODEXTIMING 1
#endif

#if defined(_MSC_VER)
#pragma warning(push,1)
#endif

#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#pragma warning(disable: 4514)
#endif

#include "codex.h"
#include "dxtcodex.h"
#include "string.h"


#define S3TCPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

/******************************************************/
/** Timer for debug                                  **/
/******************************************************/

#ifdef CODEXTIMING
#if defined(_MSC_VER)
#include "windows.h"
#else
#include <time.h>
#include <sys/time.h>
#endif

#ifdef _MSC_VER
static inline double i64tod(__int64 it)
{
    return (double) it;
    //t = ((double)(unsigned int)(t1.HighPart)*4294967296.0) + ((double)(unsigned int)(t1.LowPart));
}

double SPIT_gettime(void)
{
    double t=0;

    static double freqs=0.0;
    if (!freqs)
    {
        LARGE_INTEGER t1;
        BOOL ok = QueryPerformanceFrequency(&t1);
        if (ok)
        {
            freqs = 1000.0 / i64tod(t1.QuadPart);
//            printf("PerformanceFrequency is %I64d\n",t1.QuadPart);
        }
    }

    if (freqs)
    {
        LARGE_INTEGER t2;
        BOOL ok = QueryPerformanceCounter(&t2);
        if (ok)
        {
            t = i64tod(t2.QuadPart);
            return t * freqs;
        }
    }
    return 0;
}

#else

static inline double SPIT_gettime(void)
{
    double t=0;

    {
        struct timeval tv;
        struct timezone tz; // not used
        gettimeofday(&tv,&tz);
        t = (double)tv.tv_sec*1000.0+(double)tv.tv_usec/1000.0;
    }
    return(t);
}

#endif
#endif


/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/


int GCALL DXT_encode(void *compressed, const void *uncompressed, int numbytes, int *options)
{
    int width= 0;
    int height= 0;
    int rowbytes= 0;
    int encodetype=DXT_ENCODE_RGB_FULL;
    int fmode= 0;
//    int rgb555=0;
//    int BigEndian = 0;
    int bigSize   = 0;
    int quality=50;
//    float weight[] = {0.3f,0.4f,0.3f};
    DXTHEADER *header = (DXTHEADER*)compressed;
    BIGDXTHEADER *Bigheader = (BIGDXTHEADER*)compressed;          // just in case my image size is greater
                                                                  // than 16MB (3 bytes DXTHEADER SIZE FIELD)
    if(numbytes > 16000000)      // bigger than 16BM then have to use alternate header.
        bigSize = 1;

    if (options)
    {
        encodetype = options[0];
        quality    = options[1]; // 0 fast, 50 medium, 100 slow
    }

/*  0 for no alpha,                                                 */
/*  1 for explicit alpha,                                           */
/*  2 for interpolated alpha.                                       */
/*  4 for bgr555 little endian                                      */
/*  8 for gsh bigendian                                             */
    fmode = 0;
    int BytesPerBlock=8;
    if (encodetype&DXT_ENCODE_ALPHA_EXPLICIT)
    {
        fmode = EADXT_MODE_ALPHA_EXPLICIT; // 1 = DXT3
        BytesPerBlock = 16;
    }
    else if (encodetype&DXT_ENCODE_ALPHA_INTERPOLATED)
    {
        fmode = EADXT_MODE_ALPHA_INTERPOLATED;
        BytesPerBlock = 16;
    }
//    else
//        mode = DXT_MODE_RGB;
//    if (encodetype & DXT_ENCODE_RGB_555)
//    {
//        rgb555 = 1;
//        mode |= DXT_MODE_RGB555;
//    }
    if(encodetype & DXT_DXT1B_ENCODEOPTION /*DXT_DXT1B_CODE*/)
    {
//        BigEndian = 1;                      // set the bigendian flag.
//        mode = DXT_MODE_RGB;               // only DXT1 mode
    }
    width = 4;
    rowbytes = 16;
    height = numbytes/rowbytes;
    if(encodetype & DXT_ENCODE_ALPHA_EXPLICIT)
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, DXT_DXT3_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT3_CODE);
    }
    else if(encodetype & DXT_ENCODE_ALPHA_INTERPOLATED)
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, DXT_DXT5_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT5_CODE);

    }
    else if(encodetype & DXT_DXT1B_ENCODEOPTION)
    {
        if(!bigSize)
        {
            S3TCPUTFIELD(header->code, DXT_DXT1B_CODE);          // we have set the BigEndian flag so mark as Bigendian GSH style DXT.
            encodetype = encodetype&~DXT_DXT1B_ENCODEOPTION;     // just bookeeping so get rid of here so it doesn't interfere
        }                                                         // with compression to DXT1.
        else
        {
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT1B_CODE);    // we have set the BigEndian flag so mark as Bigendian GSH style DXT.
            encodetype = encodetype&~DXT_DXT1B_ENCODEOPTION;     // just bookeeping so get rid of here so it doesn't interfere
        }                                                         // with compression to DXT1.
    }
    else
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, DXT_DXT1_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, DXT_BIGDXT1_CODE);
    }
    //S3TCPUTFIELD(header->code, (encodetype & DXT_ENCODE_ALPHA_EXPLICIT) ? DXT_DXT3_CODE : DXT_DXT1_CODE);
    if(!bigSize)
    {
        S3TCPUTFIELD(header->size, numbytes);
        S3TCPUTFIELD(header->mode, fmode);
    }
    else
    {
        S3TCPUTFIELD(Bigheader->size, numbytes);
        S3TCPUTFIELD(Bigheader->mode, fmode);
    }

#ifdef CODEXTIMING
    double t=SPIT_gettime();
#endif
    {

#if ENABLE_OMP
        #pragma omp parallel for
#endif
        for (int y=0; y<height/4; ++y)
        {
            char *rowout = (char *)compressed + sizeof(DXTHEADER) + BytesPerBlock*(y);
            char *rowin = (char *) uncompressed + 4*4*4 * y;
            eadxt(rowout, rowin, 4*4, 1, fmode, quality);
        }
    }

#ifdef CODEXTIMING
    t=SPIT_gettime()-t;

    printf("DXT time %5.2f ms\n",t);
#endif

    return ((width + 3) / 4) * ((height + 3) / 4) * BytesPerBlock + sizeof(DXTHEADER);
}

#undef S3TCPUTFIELD

#endif



