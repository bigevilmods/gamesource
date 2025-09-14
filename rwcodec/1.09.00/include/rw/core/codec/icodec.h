// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::codec::ICodec and IBufCodec
*/

#ifndef RW_CORE_CODEC_ICODEC_H
#define RW_CORE_CODEC_ICODEC_H

#include "EAAssert/eaassert.h"
#include "coreallocator/icoreallocator_interface.h"

#include "job_manager/job_environment.h"
#include "job_manager/entry_point.h"
#include "rw/core/codec/config.h"

#if defined _MSC_VER
    #pragma once
#endif

/**
\namespace rw::core::codec
\brief Contains a base for compression and decompression codec implementations.
*/

namespace rw {
namespace core {
namespace codec {

/**
\brief class ICodec is an abstract base class for codec implementations.

 Implementations must provide methods for Init, End, and Exec.
*/
class RWCODEC_API ICodec
{
public:

#if !defined(EA_PLATFORM_PS3_SPU)
    virtual ~ICodec();
#endif

    /**
    \brief Possible return values from Exec.
    */
    enum Status
    {
        STREAM_OK,      ///< Need more room in output or more data at input.
        STREAM_END,     ///< End of stream has been reached.
        STREAM_ERROR    ///< There was an error in the data stream.
    };

    /**
     \brief Compression level flags

     These values are provided as suggestions,
     but the caller can pass any value from 0-9 (inclusive).  Depending
     on the codec, this will map to increasing compression ratios.
     However, (and this is also dependent on the codec being used) more
     highly compressed data may use more resources (memory or CPU cycles
     or both) to decompress at runtime.  Keep that in mind when choosing
     a compression level.
    */
    enum DeflateInitFlags
    {
        DEFLATE_NONE = 0,           ///< No compression, not currently supported, for use in user-defined codecs
        DEFLATE_BEST_SPEED = 1,     ///< Minimal compression ratio, best speed, fewest resources
        DEFLATE_BEST_SIZE = 9       ///< Maximum compression ratio, slowest speed
    };

    /** \brief Decompression initialization flags

    These flags may be passed as a hint to the decompressor, which will allow
    it to use fewer resources when doing the decompression.
    */
    enum InflateInitFlags
    {
        /// Hint for the decompressor that previously-decoded data
        /// will be available for the decompressor to refer to in
        /// future calls, which may avoid the necessity for a temporary
        /// buffer.
        INFLATE_TEMP_BUFFER_NOT_NEEDED = 0
    };

    /**
       \brief Flags which can be passed to the Exec method during compression.

       The flags parameter is ignored during decompression.
    */
    enum ExecFlags
    {
        /// Don't flush the output buffer (default)
        NO_FLUSH = 0,

        /// Flush the output buffer to create a seek point in the middle
        /// of a compression stream.  Usually not used.
        FLUSH = 1,

        /// Pass this flag when the last block of data to be compressed
        /// is reached.
        FINISH = 2
    };

#if !defined(EA_PLATFORM_PS3_SPU)

    void            SetOutputBuffer(void *outputbuf, size_t outputbufsize);
    void            SetInputBuffer(const void *in, size_t InputBufSize);

    /**
    \brief Get the amount of data available at the input buffer.  When this
     is 0, give the codec more data with SetInputBuffer.
    */
    size_t          GetInputBytesAvailable() const
    { return mInputBytesAvailable; }

    /**
    \brief Get the amount of data available at the output buffer.
    When this is 0, give the codec more output space with SetOutputBuffer.
    */
    size_t          GetOutputBytesAvailable() const
    { return mOutputBytesAvailable; }

    /**
    \brief Get the total number of bytes output so far from this stream.
    */
    uint64_t        GetTotalOut() const
    { return mOutputBytes; }

    /**
    \brief Re-initialize the codec so another stream may be processed.

    Same as doing an End(); Init(level).
    */
    void            Reset();

    // Pure virtual methods that derived classes must implement.

    /**
    \brief Initialize the codec object.

    Each call to Init should have a corresponding call to End.  A second
    call to Init or calling Init after reset will be ignored.  In debug,
    if you call Init a second time with a different compression level, an
    assert will fire.  If you want to change the compression level, call
    End first, then Init with the new compression level.

    \param level A number from 0 to 9 representing how hard to try to compress
    the data.  0 means no compression, 9 means maximum compression ratio.
    Generally, the higher the compression ratio, the more resources the
    codec will need, both in terms of CPU time and memory footprint.  For
    decompressors, this parameter is ignored.
    \sa End
    */
    virtual void    Init(unsigned int level) = 0;

    /**
    \brief Release any resources allocated by the object.

    Each Init call should have a corresponding End call to release
    any resources.  The class destructor should also call End.
    \sa Init
    */
    virtual void    End() = 0;

    /**
    Process as much data as possible until one of the following
     conditions is true:
     - there is no more room in the output buffer
     - there is more data available in the input buffer.
     - the end of stream is reached

    \par
    Exec may be called repeatedly with more data at the input
    pointer or more buffer space available at the output pointer.
    \param flags One of the ExecFlags.  Only meaningful during
    compression.
    \return One of the Status values
    \sa Status
    */
    virtual Status  Exec(unsigned int flags) = 0;

    /**
    Retrieve the job entry point associated with this codec. This method,
    along with GetJobParameters, is a replacement for the old SetJob method.
    */
    virtual EA::Jobs::EntryPoint GetJobEntryPoint(const EA::Jobs::JobEnvironment environment = EA::Jobs::JOB_ENVIRONMENT_DEFAULT)
    {
        (void)environment;
        EA_FAIL();
        return EA::Jobs::EntryPoint();
    }

    /**
    Given an array of parameters, GetJobParameters sets these to the values 
    necessary in order to schedule a compression job. This method, along with
    GetJobEntryPoint, is a replacement for the old SetJob method.
    */
    virtual void GetJobParameters(EA::Jobs::Param params[4], 
            const unsigned int flags, 
            const EA::Jobs::JobEnvironment environment = EA::Jobs::JOB_ENVIRONMENT_DEFAULT)
    {
        (void)params;
        (void)flags;
        (void)environment;
        EA_FAIL();
    }

protected:
    ICodec(EA::Allocator::ICoreAllocator* allocator, unsigned int level);

    void ClearValues();

#else

protected:

    uint32_t        _vtable;    // space for the vtable

#endif

    EA::Allocator::ICoreAllocator* mAllocator;

    bool            mReady;
    unsigned int    mLevel;

    size_t          mInputBytesAvailable;
    size_t          mOutputBytesAvailable;
    uint64_t        mOutputBytes;

    const void *    mInputBuffer;
    void *          mOutputBuffer;
};

/**
\brief For buffer-to-buffer decompression.  Usually refpack.
*/
class RWCODEC_API IBufCodec
{
public:
    /**
     \brief Given a compressed buffer, decompress it to outputBuf.
     The output buffer needs to be large enough to hold all
     the uncompressed data.  If it is not, a buffer overrun
     will result.
     */
    virtual unsigned int Decompress(void *dstBuf, const void *srcBuf) = 0;

    /**
     \brief Given a compressed buffer, get the size of the buffer
     required to store the decompressed output.
    */
    virtual unsigned int GetDecompressedSize(const void *inputBuf) = 0;

    IBufCodec() {}
    virtual ~IBufCodec() {}
};



}}}

#endif
