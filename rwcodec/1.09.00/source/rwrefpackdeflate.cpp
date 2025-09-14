// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of rw::core::codec::RefpackDeflate.
*/

#include <stdlib.h>
#include <new>      // placement new for CompressParams

#include "EABase/eabase.h"

#include "rw/core/codec/icodec.h"
#include "rw/core/codec/refpack.h"

#include "EAStdC/EAEndian.h"
#include "EAStdC/EAMemory.h"

#include "jobs/refpack/refpack.h"

#if defined(EA_PLATFORM_PS3)
#include <sys/process.h>    // sys_process_is_stack()
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define ROUNDUP_POWER2(n, p) ((n+p-1) & ~(p-1))

namespace rw {
namespace core {
namespace codec {

const int HASHTABLESIZE = 16384;

struct LevelToQMap
{
    int Quick;
    unsigned int WindowSize;
};

// table for mapping levels to window sizes/quick factors.
const LevelToQMap gQMapTable[10] = {
    { 0, 0 },
    { 3,   1*1024-1 },
    { 3,  16*1024-1 },
    { 1,   1*1024-1 },
    { 1,   2*1024-1 },
    { 1,   4*1024-1 },
    { 0,  16*1024-1 },
    { 0,  32*1024-1 },
    { 0,  64*1024-1 },
    { 0, 128*1024-1 },
};

#if !defined(EA_PLATFORM_PS3_SPU)

RefpackDeflate::RefpackDeflate(EA::Allocator::ICoreAllocator* allocator, unsigned int level)
: ICodec(allocator, level), mWindowSize(0), mWindowBuf(NULL)
{
    RefpackDeflate::Init(level);
}

RefpackDeflate::~RefpackDeflate()
{
    RefpackDeflate::End();

    if (mWindowBuf != NULL)
    {
        mAllocator->Free(mWindowBuf);
        mWindowBuf = NULL;
    }
}

void RefpackDeflate::Init(unsigned int level)
{
#ifdef EA_DEBUG
    if (mReady)
    {
        EA_ASSERT_MSG(level == mLevel,
            "To re-initialize with a different compression level, change the compression level, first call End");
    }
#endif

    ClearValues();

    mLevel = level;
    mReady = true;
}

void RefpackDeflate::End()
{
    if (mReady)
    {
        mReady = false;
    }
}

/**
\brief Release the temporary window buffer.

Due to the fact that it may be expensive and unnecessary to keep reallocating
the temporary window buffer, we have chosen to not release it in
RefpackDeflate::End.  Instead, the temp buffer is only deallocated when the
RefpackDeflate object is actually destroyed.  However, there may be instances
where a user needs to force a release of the temporary buffer, which is the
purpose of this fuction.  The RefpackDeflate object may continue to be used as
before, if further compression is performed, a new temp buffer will be
allocated automatically.

The temp buffer must not be released while compression is happening in another
thread.  Any other time is safe to release this buffer.
*/
void RefpackDeflate::ReleaseTempBuffer()
{
    if (mWindowBuf != NULL)
    {
        mAllocator->Free(mWindowBuf);
        mWindowBuf = NULL;
        mWindowSize = 0;
    }
}

void RefpackDeflate::AllocateWindowBuffer(unsigned int flags)
{
#if defined(EA_ASSERT_ENABLED)
    EA_ASSERT_MSG(mReady,
        ("Error - codec not initialized - called Init first"));

    EA_ASSERT_MSG(flags == ICodec::FINISH,
        ("Error - Refpack deflator cannot be done in chunks.  Must pass the ICodec::FINISH flag the first time Exec is done"));

    EA_ASSERT_MSG(mOutputBytesAvailable >= 6,
        ("The output buffer size must be at least 6 bytes to hold the header"));

#else
    // avoid compiler warning when asserts are not enabled
    (void)flags;
#endif

    if (mWindowBuf == NULL || mWindowSize < gQMapTable[mLevel].WindowSize)
    {
        if (mWindowBuf != NULL)
        {
            mAllocator->Free(mWindowBuf);
        }

        mWindowSize = gQMapTable[mLevel].WindowSize + 1;

        mWindowBuf = mAllocator->Alloc((HASHTABLESIZE + mWindowSize) * sizeof(hash_t),
            "rw::core::codec::RefpackDeflate::Hash & link temp buffer",
            EA::Allocator::MEM_TEMP, 16);
    }
}

ICodec::Status RefpackDeflate::Exec(unsigned int flags)
{
    AllocateWindowBuffer(flags);

    Compress(reinterpret_cast<EA::Jobs::Param>(this),
        reinterpret_cast<EA::Jobs::Param>(mWindowBuf), 0, 0);

    // be consistent with zlib and return STREAM_END when the compression
    // is complete.
    return ICodec::STREAM_END;
}

#if defined(EA_PLATFORM_PS3)

// Use the SPU to execute this task

static unsigned char gEncodeSpuBinary[] __attribute__((aligned(128), section(".spu_image"))) =
{
    #include "rwcodec_refpackdeflate_job.job.h"
};

EA::Jobs::EntryPoint RefpackDeflate::sSpuJobEntryPoint(
    EA::Jobs::JOB_ENVIRONMENT_SPU_THREADS, gEncodeSpuBinary, sizeof(gEncodeSpuBinary));

#endif  // EA_PLATFORM_PS3

EA::Jobs::EntryPoint RefpackDeflate::sJobEntryPoint(Compress);

/** \brief Returns the entry point used for asynchronous/parallel 
compression of data.
*/
EA::Jobs::EntryPoint RefpackDeflate::GetJobEntryPoint(const EA::Jobs::JobEnvironment env)
{
#if defined(EA_PLATFORM_PS3)
    if (env == EA::Jobs::JOB_ENVIRONMENT_SPU_THREADS)
        return sSpuJobEntryPoint;
#endif

    (void)env;

    return sJobEntryPoint;
}

/** \brief Fills out an array of parameters with which to schedule a 
parallel/asynchronous compression job.

If you plan on running the compression on an SPU, please observe the
following restrictions:
- the RefpackDeflate object will be DMAed to and from the local store,
  so the object must not be on the stack and it must be 16-byte-aligned.
- source and output data are not streamed from/to main memory, so
  everything must fit into the SPU local store (256KB).  The code size
  is about 8K in an optimized build (12K in debug), and between 68 and
  128K of temporary buffers are required, depending on the compression
  level. Therefore, the total size of the input and output buffers cannot
  be more than 175K.  Assuming about a 2:1 compression ratio, the input
  buffer can be no larger than about 116K.  If your data compresses better
  than 2:1, the input buffer can be larger than that.  You'll have to
  experiment with the source data and compression level.
If you cannot adhere to these restrictions, you will need to use the
PPU to execute the job.  Note that the output from compression done
on the SPU will be identical to that performed on the PPU.

\param params The array of parameters to fill in.
\param flags Must be rw::core::codec::ICodec::FINISH.
\param env Location/processor where the job should run
*/
void RefpackDeflate::GetJobParameters(EA::Jobs::Param params[4], const unsigned int flags, const EA::Jobs::JobEnvironment env)
{
#if defined(EA_PLATFORM_PS3)
    if (env == EA::Jobs::JOB_ENVIRONMENT_SPU_THREADS)
    {
        EA_ASSERT_MSG(sys_process_is_stack(this) == 0,
            "The RefpackDeflate object cannot be located on the stack when using an SPU job");
        EA_ASSERT_MSG(((int)this & 0xf) == 0,
            "The RefpackDeflate object must be aligned on a 16-byte boundary for DMA purposes");

        const unsigned int SIZEOF_LOCAL_STORE = (256-5)*1024;   // subtract off a bit for the JM kernel

        // prevent compiler warning
        (void)SIZEOF_LOCAL_STORE;

            // Ensure that the total amount of code, data and buffers required will fit
        // into the SPU local store.
        EA_ASSERT_MSG(
            (sizeof(gEncodeSpuBinary) +     // size of the job
            mInputBytesAvailable +          // How much data there is to encode
            mOutputBytesAvailable +         // What the maximum size of the output buffer is
            HASHTABLESIZE * sizeof(int) +   // size of the hash table (32K)
            sizeof(int) * MIN(gQMapTable[mLevel].WindowSize + 1, 16*1024))    // size of the Window buffer (max 32K)
            <= SIZEOF_LOCAL_STORE,
            "The total amount of code, data and buffers exceeds the size of the local store.  Either compress less data or use the PPU.");
    }
    else
    {
        AllocateWindowBuffer(flags);
    }
#else
    (void)env;
    AllocateWindowBuffer(flags);
#endif

    params[0] = reinterpret_cast<EA::Jobs::Param>(this);
    params[1] = reinterpret_cast<EA::Jobs::Param>(mWindowBuf);
}

#else   // EA_PLATFORM_PS3_SPU

RefpackDeflate::RefpackDeflate(EA::Allocator::ICoreAllocator *, unsigned int)
: ICodec()
{
    // empty
}

#endif  // SPU

// Common to SPU & non-SPU

RefpackDeflate::gint_t RefpackDeflate::MatchLength(unsigned char *s, unsigned char *d, RefpackDeflate::gint_t maxmatch)
{
    RefpackDeflate::gint_t current=0;

    while (maxmatch>=4)
    {
        if (s[0]!=d[0]) return current;
        if (s[1]!=d[1]) return current+1;
        if (s[2]!=d[2]) return current+2;
        if (s[3]!=d[3]) return current+3;
        s += 4;
        d += 4;
        current += 4;
        maxmatch -= 4;
    }

    if (maxmatch<1 || s[0]!=d[0]) return current;
    if (maxmatch<2 || s[1]!=d[1]) return current+1;
    if (maxmatch<3 || s[2]!=d[2]) return current+2;
    if (maxmatch<4 || s[3]!=d[3]) return current+3;

    return current+4;
}

inline RefpackDeflate::hash_t RefpackDeflate::GetHash(const unsigned char *cptr)
{
    RefpackDeflate::hash_t hash = cptr[0]*1089u + cptr[1]*33u+cptr[2];

    return hash&(HASHTABLESIZE-1);
}

#if defined(EA_ASSERT_ENABLED)
static const char *failMsg = "Output buffer is not large enough to hold all the compressed data.";
#endif

void RefpackDeflate::Compress(EA::Jobs::Param param1, EA::Jobs::Param param2, EA::Jobs::Param , EA::Jobs::Param)
{
#if defined(EA_PLATFORM_PS3_SPU)
    // make a copy of this object into def
    EA_ALIGNED(RefpackDeflate, def, 16)(NULL, 1);
    EA::Jobs::DmaGet(&def, reinterpret_cast<const void *>(param1), sizeof(RefpackDeflate));
    RefpackDeflate *self = &def;
    unsigned char lsSrcBuf[self->mInputBytesAvailable];
    unsigned char lsDstBuf[self->mOutputBytesAvailable];
    EA::Jobs::DmaGet(lsSrcBuf, self->mInputBuffer, ROUNDUP_POWER2(self->mInputBytesAvailable, 16));

    unsigned char *src = lsSrcBuf;
    unsigned char *dest = lsDstBuf;

#else

    RefpackDeflate *self = reinterpret_cast<RefpackDeflate *>(param1);

    unsigned char *dest = static_cast<unsigned char *>(self->mOutputBuffer);
    unsigned char *src = const_cast<unsigned char *>(
        static_cast<const unsigned char *>(self->mInputBuffer));

    hash_t *hashtbl = reinterpret_cast<hash_t *>(param2);
    hash_t *link = hashtbl + HASHTABLESIZE;

#endif

    gint_t tlen;
    gint_t tcost;
    gint_t run(0);
    unsigned int toffset;
    unsigned int boffset;
    gint_t blen;
    gint_t bcost;
    gint_t mlen;
    unsigned char *tptr;
    unsigned char *cptr(src);
    unsigned char *to(dest);
    unsigned char *rptr(src);
    hash_t hash;
    hash_t hoffset;
    hash_t minhoffset;

    unsigned short refsig = REF_SIG;
    size_t sizebytes = 3;
    int MaxLookBack = static_cast<int>(gQMapTable[self->mLevel].WindowSize);
#if defined(EA_PLATFORM_PS3_SPU)
    // Limit the window size to 16K on the SPU.
    MaxLookBack = MIN(MaxLookBack, 16*1024-1);
#endif
    int quick = gQMapTable[self->mLevel].Quick;

    if (self->mInputBytesAvailable >= 0x1000000)  // source size >= 2^24?  Use 32-bit header
    {
        refsig |= REF_HEADER_FLAG_SIZE32;
        sizebytes = 4;
    }

    EA::StdC::WriteToBigEndian(to, refsig);

    // we currently do not have any 3-byte endian methods
    if (sizebytes == 3)
    {
        const uint8_t* availiableCast = reinterpret_cast<uint8_t*>(&self->mInputBytesAvailable);

        // We assume that the MSB will be discarded in both cases
        #if defined(EA_SYSTEM_LITTLE_ENDIAN)
            (to+2)[0] = availiableCast[2];
            (to+2)[1] = availiableCast[1];
            (to+2)[2] = availiableCast[0];
        #else
            (to+2)[0] = availiableCast[1];
            (to+2)[1] = availiableCast[2];
            (to+2)[2] = availiableCast[3];
        #endif
    }
    else
    {
        EA::StdC::WriteToBigEndian(to+2, &self->mInputBytesAvailable, sizebytes);
    }

    size_t headerlength = sizeof(REF_SIG) + sizebytes;
    to += headerlength;

#if defined(EA_ASSERT_ENABLED)
    int destBufSize = static_cast<int>(self->mOutputBytesAvailable - headerlength);
#endif

#if defined(EA_PLATFORM_PS3_SPU)
    hash_t hashtbl[HASHTABLESIZE];
    hash_t link[MaxLookBack+1];
#endif

#if defined(EA_PLATFORM_PS3_SPU)
    // EAStdC 1.03.06 does not support the SPU.
    memset(hashtbl,0,HASHTABLESIZE*sizeof(hash_t));
#else
    // Memset32's 3rd parameter is the item count, not the size of the array,
    // therefore, divide by 4 for a 32-bit fill
    EA::StdC::Memset32(hashtbl,0,HASHTABLESIZE*sizeof(hash_t)/4);
#endif

    int srcBufSize = static_cast<int>(self->mInputBytesAvailable) - 4;

    while (srcBufSize>=0)
    {
        boffset = 0;
        blen = 2;
        bcost = 2;
        mlen = static_cast<gint_t>(MIN(srcBufSize,1028));

        if (quick >= 2 && mlen > 67)
            mlen = 67;

        tptr=cptr-1;
        hash = GetHash(cptr);
        hoffset = hashtbl[hash];
        minhoffset = static_cast<hash_t>(MAX(cptr-src-MaxLookBack,1));

        if (hoffset>=minhoffset)
        {
            do
            {
                tptr = src+hoffset;
                if (cptr[blen]==tptr[blen])
                {
                    tlen = MatchLength(cptr,tptr,mlen);

                    toffset = static_cast<unsigned int>((cptr-1)-tptr);
                    if ((toffset<1024) && (tlen<=10))
                    {
                        tcost = 2;  // two byte short form
                    }
                    else if ((toffset<16384) && (tlen<=67))
                    {
                        tcost = 3;  // three byte long form
                    }
                    else
                    {
                        tcost = 4;  // four byte very long form
                    }

                    if ((tlen-tcost+4) > (blen-bcost+4))
                    {
                        blen = tlen;
                        bcost = tcost;
                        boffset = toffset;
                        if (blen >= 1028) break;
                    }
                }
            } while ((hoffset = link[hoffset & MaxLookBack]) >= minhoffset);
        }

        if ((bcost>=blen) || (srcBufSize<4))
        {
            hoffset = static_cast<hash_t>(cptr-src);
            link[hoffset&MaxLookBack] = hashtbl[hash];
            hashtbl[hash] = hoffset;

            ++run;
            ++cptr;
            --srcBufSize;
        }
        else
        {
            while (run>3)                   // literal block of data
            {
                tlen = MIN(gint_t(112),run&~3);
                run -= tlen;
#if defined(EA_ASSERT_ENABLED)
                EA_ASSERT_MSG(destBufSize >= 1, failMsg);
                --destBufSize;
#endif
                //lint -e{961} operator precedence
                to[0] = static_cast<unsigned char>(0xe0+(tlen>>2)-1);
#if defined(EA_ASSERT_ENABLED)
                EA_ASSERT_MSG(destBufSize >= static_cast<int>(tlen), failMsg);
                destBufSize -= static_cast<int>(tlen);
#endif
                EA::StdC::Memcpy(to+1,rptr,static_cast<unsigned int>(tlen));
                rptr += tlen;
                to += 1+tlen;
            }
            if (bcost==2)                   // two byte long form
            {
#if defined(EA_ASSERT_ENABLED)
                EA_ASSERT_MSG(destBufSize >= 2, failMsg);
                destBufSize -= 2;
#endif
                to[0] = static_cast<unsigned char>(((boffset>>8)<<5) + ((blen-3)<<2) + run);
                to[1] = static_cast<unsigned char>(boffset);
                to += 2;
            }
            else if (bcost==3)              // three byte long form
            {
#if defined(EA_ASSERT_ENABLED)
                EA_ASSERT_MSG(destBufSize >= 3, failMsg);
                destBufSize -= 3;
#endif
                to[0] = static_cast<unsigned char>(0x80 + (blen-4));
                to[1] = static_cast<unsigned char>((run<<6) + (boffset>>8));
                to[2] = static_cast<unsigned char>(boffset);
                to += 3;
            }
            else                            // four byte very long form
            {
#if defined(EA_ASSERT_ENABLED)
                EA_ASSERT_MSG(destBufSize >= 4, failMsg);
                destBufSize -= 4;
#endif
                to[0] = static_cast<unsigned char>(0xc0 + ((boffset>>16)<<4) + (((blen-5)>>8)<<2) + run);
                to[1] = static_cast<unsigned char>(boffset>>8);
                to[2] = static_cast<unsigned char>(boffset);
                to[3] = static_cast<unsigned char>(blen-5);
                to += 4;
            }

            if (run > 0)
            {
#if defined(EA_ASSERT_ENABLED)
                EA_ASSERT_MSG(destBufSize >= static_cast<int>(run), failMsg);
                destBufSize -= static_cast<int>(run);
#endif
                EA::StdC::Memcpy(to, rptr, static_cast<unsigned int>(run));
                to += run;
                run = 0;
            }

            if (quick)
            {
                if (quick==3)
                {
                    hoffset = static_cast<hash_t>(cptr-src);
                    link[hoffset&MaxLookBack] = 0;
                    hashtbl[hash] = hoffset;
                }
                else if (quick==1 || (hoffset&3)==0)
                {
                    hoffset = static_cast<hash_t>(cptr-src);
                    link[hoffset&MaxLookBack] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                }
                cptr += blen;
            }
            else
            {
                for (int i=0; i < static_cast<int>(blen); ++i)
                {
                    hash = GetHash(cptr);
                    hoffset = static_cast<hash_t>(cptr-src);
                    link[hoffset&MaxLookBack] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                    ++cptr;
                }
            }

            rptr = cptr;
            srcBufSize -= static_cast<int>(blen);
        }
    }
    srcBufSize += 4;
    run += srcBufSize;
    while (run>3)                               // no match at end, use literal
    {
        tlen = MIN(gint_t(112),run&~3);
        run -= tlen;
#if defined(EA_ASSERT_ENABLED)
        EA_ASSERT_MSG(destBufSize >= 1, failMsg);
        --destBufSize;
#endif
        //lint -e{961}  operator precedence
        to[0] = static_cast<unsigned char>(0xe0+(tlen>>2)-1);
#if defined(EA_ASSERT_ENABLED)
        EA_ASSERT_MSG(destBufSize >= static_cast<int>(tlen), failMsg);
        destBufSize -= static_cast<int>(tlen);
#endif
        EA::StdC::Memcpy(to+1,rptr,static_cast<unsigned int>(tlen));
        rptr += tlen;
        to += 1+tlen;
    }

#if defined(EA_ASSERT_ENABLED)
    EA_ASSERT_MSG(destBufSize >= 1, failMsg);
    --destBufSize;
#endif
    to[0] = static_cast<unsigned char>(0xfc+run); // end of stream command + 0..3 literal
    to += 1;
    if (run > 0)
    {
#if defined(EA_ASSERT_ENABLED)
        EA_ASSERT_MSG(destBufSize >= static_cast<int>(run), failMsg);
        destBufSize -= static_cast<int>(run);
#endif
        EA::StdC::Memcpy(to,rptr,static_cast<unsigned int>(run));
        to += run;
    }

#if defined(EA_PLATFORM_PS3_SPU)
    // Dma the compressed data back to main memory
    EA::Jobs::DmaPut(self->mOutputBuffer, lsDstBuf, ROUNDUP_POWER2(to-dest, 16));
#endif

    // update the object members
    self->mOutputBytesAvailable = static_cast<unsigned int>(to-dest);
    self->mOutputBuffer = static_cast<void *>(
        static_cast<unsigned char *>(self->mOutputBuffer) + self->mOutputBytesAvailable);
    self->mInputBuffer = static_cast<const void *>(
        static_cast<const unsigned char *>(self->mInputBuffer) + self->mInputBytesAvailable);
    self->mInputBytesAvailable = 0;
    self->mOutputBytes = self->mOutputBytesAvailable;

#if defined(EA_PLATFORM_PS3_SPU)
    EA::Jobs::DmaPut(reinterpret_cast<void *>(param1), &def, sizeof(def));

    // blow away the copy of the deflate object before it goes out of scope,
    // otherwise the destructor is called and bad stuff happens because this
    // object aliases the real one.
    memset(&def, 0, sizeof(def));
#endif
}

}}}

#if defined(EA_PLATFORM_PS3_SPU)

int main(uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
    rw::core::codec::RefpackDeflate::Compress(param1, param2, param3, param4);
}

#endif // EA_PLATFORM_PS3_SPU
