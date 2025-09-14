// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::codec::RefpackDeflate and RefpackInflate.
*/

#ifndef RW_CORE_CODEC_REFPACK_H
#define RW_CORE_CODEC_REFPACK_H

#include "EAAssert/eaassert.h"
#include "rw/core/codec/icodec.h"
#include "coreallocator/icoreallocator_interface.h"

#include "job_manager/entry_point.h"
#include "job_manager/job_environment.h"

#if defined _MSC_VER
#pragma once
#pragma warning(push)
#pragma warning(disable: 4251)  // class 'member' needs to have dll-interface to be used by clients class 'class'
#endif

namespace rw {
namespace core {
namespace codec {

/**
 \brief class RefpackDeflate implements a compressor for the refpack algorithm.

 This algorithm decompresses fast, which usually makes it the best
 codec to use if your objective is to reduce load times.

 \par
 The level parameter to the Init function and the constructor must be
 one of DEFLATE_BEST_SPEED or DEFLATE_BEST_SIZE.  If you use
 DEFLATE_BEST_SIZE, a 128K sliding window is used to decompress the data.
 If you use DEFLATE_BEST_SPEED, the sliding window is only 16K.

 \par
 Because it is not possible to know before the compression is done how
 large a buffer may be required to hold the compressed data, we suggest
 that the output buffer be 20% larger than the input buffer.  If the
 output buffer is not large enough to hold the compressed data, a buffer
 overrun will occur.  In debug configurations, the buffer size is checked
 and a failed assertion will be thrown.
*/
class RWCODEC_API RefpackDeflate : public ICodec
{
public:
    RefpackDeflate(EA::Allocator::ICoreAllocator* allocator, unsigned int level);
    void ReleaseTempBuffer();

#if !defined(EA_PLATFORM_PS3_SPU)

    virtual ~RefpackDeflate();

    virtual void    Init(unsigned int level);
    virtual void    End();
    virtual Status  Exec(unsigned int flags);
    virtual EA::Jobs::EntryPoint GetJobEntryPoint(const EA::Jobs::JobEnvironment environment = EA::Jobs::JOB_ENVIRONMENT_DEFAULT);
    virtual void GetJobParameters(EA::Jobs::Param params[4],
            const unsigned int flags,
            const EA::Jobs::JobEnvironment environment = EA::Jobs::JOB_ENVIRONMENT_DEFAULT);

private:

    void AllocateWindowBuffer(unsigned int flags);
    static EA::Jobs::EntryPoint sJobEntryPoint;
#if defined(EA_PLATFORM_PS3)
    static EA::Jobs::EntryPoint sSpuJobEntryPoint;
#endif  // EA_PLATFORM_PS3

#endif  // !defined(EA_PLATFORM_PS3_SPU)

    typedef uint64_t gint_t;
    typedef unsigned int hash_t;

    static gint_t MatchLength(unsigned char *s, unsigned char *d, gint_t maxmatch);
    static inline hash_t GetHash(const unsigned char *cptr);
    static void Compress(EA::Jobs::Param, EA::Jobs::Param, EA::Jobs::Param, EA::Jobs::Param);

    unsigned int mWindowSize;
    void *mWindowBuf;

#if defined(EA_COMPILER_MSVC)
    unsigned char pad0[8];  // pad class size to a multiple of 16 bytes
#endif
};

#if !defined(EA_PLATFORM_PS3_SPU)
/**
 \brief RefpackInflate implements a decompressor for refpack-compressed data streams.  The decompressor respects the
 INFLATE_TEMP_BUFFER_NOT_NEEDED flag so it requires no temporary buffers if you use that flag.

 \par
 Unlike the Refpack compressor, you don't need to perform the
 decompression in a single call.  You may call Exec repeatedly
 with more compressed data in the input buffer or more room in
 the output buffer until the decompression is complete.
*/
class RWCODEC_API RefpackInflate : public ICodec
{
public:
    explicit RefpackInflate(EA::Allocator::ICoreAllocator* allocator);
    virtual ~RefpackInflate();

    virtual void    Init(unsigned int level);
    virtual void    End();
    virtual Status  Exec(unsigned int flags);

private:
    enum DecompressState
    {
        GET_TYPE,
        GET_LENGTH,
        GET_CONTROL_BYTE,
        TEST_CONTROL_BYTE,
        GET_CONTROL_BYTES,
        GET_SHORT_RUN_BYTES,
        GET_LONG_RUN_BYTES,
        GET_VERY_LONG_RUN_BYTES,
        GET_SHORT_REF_BYTES,
        GET_LONG_REF_BYTES,
        GET_VERY_LONG_REF_BYTES,
        COPY_LITERAL,
        COPY_REFERENCE,
        SETUPFLUSH,
        FLUSHWINDOW
    };
    /* next input byte */
    const unsigned char * mNextIn;
    /* next output byte should be put there */
    unsigned char * mNextOut;
    /* decompress state */
    DecompressState mState;
    /* state to return to after COPY_LITERAL completes */
    DecompressState mNextState;
    /* amount of data that needs to be read in before anything can be done. */
    int             mBytesNeeded;
    /* length of the decompressed file. */
    int             mOriginalLength;
    /* number of bytes to copy */
    unsigned int    mRunLength;
    unsigned int    mControl;
    /* reference pointer for referring back */
    unsigned char * mRefPtr;
    /* base address of reference buffer */
    unsigned char * mRefWindowBase;
    /* current write position in temp buffer */
    unsigned char * mRefWindowPtr;
    int64_t         mNumBytesDecompressed;
    /* size of lookback buffer (128K or 16K) */
    int             mWindowSize;
    /* Init/End/Inflate return value */
    Status          mStatus;

    ICodec::Status  Decompress();
};

/** \brief Refpack source buffer to destination buffer (non-streamed)
    decompression class.

    This class decompresses a source buffer to a destination buffer.
    Unlike RefpackInflate, this class does not operate on data streams;
    the entire source buffer must be in memory when the call to
    Decompress is made.
    \sa RefpackInflate, RefpackDeflate
*/

class RWCODEC_API RefpackBuf : public IBufCodec
{
public:
    /** \brief Decompress a Refpack-compressed buffer

    This function decompresses a refpack-compressed buffer.  Refpack
    has the property that the decompression may be done "in-place".
    That means the source data may be placed into the destination buffer
    and overwritten as the decompression is done.  This feature eliminates
    the need for a temporary buffer to store the compressed data, which
    helps reduce heap fragmentation and memory highwater mark.  For the
    in-place decompression to work, the source data must be placed at
    the <b>end</b> of the destination buffer, not the beginning.  Furthermore,
    it is necessary to allocate an indeterminate amount of <i>overlap</i>
    space.  Overlap refers to the last literal or reference at the end
    of the compressed buffer - it may not overlap with the destination
    buffer.  In practice, 16 bytes is nearly always (99.99% of the time)
    more than enough; however the amount can only be computed when the
    data is being compressed.  Tools such as glump have a failoverlap
    option, which aborts the compression if the in-place decompression
    needs more overlap space than the number provided.

    \param dstBuf Buffer to hold the decompressed data.  It is the
    responsibility of the caller to ensure this buffer is large enough.
    \param srcBuf Buffer with all of the compressed data.  It is the
    responsibility of the caller to ensure this buffer holds all the
    compressed data.
    \return Size of the decompressed data
    \see GetDecompressedSize
    */
    virtual unsigned int Decompress(void *dstBuf, const void *srcBuf);

    /** \brief Return the decompressed size of a Refpack-compressed buffer

    This function returns the amount of space requires to hold an
    uncompressed buffer.  The size of uncompressed buffer is stored in the
    first 6 bytes of the buffer, so it is only necessary to supply six
    bytes of data at the input buffer.
    \param inputBuf Source buffer.  At least the first six bytes must be
    valid.  Data following the first six bytes is ignored for this function
    \return Number of bytes required to hold the decompressed data
    \sa Decompress
    */
    virtual unsigned int GetDecompressedSize(const void *inputBuf);
};

#endif  // EA_PLATFORM_PS3_SPU
}}}

#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif
