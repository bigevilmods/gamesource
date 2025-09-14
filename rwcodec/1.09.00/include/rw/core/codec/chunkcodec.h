// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_CODEC_CHUNKCODEC_H
#define RW_CORE_CODEC_CHUNKCODEC_H

#include "EAAssert/eaassert.h"
#include "EABase/eabase.h"

#include "job_manager/job_affinity.h"
#include "job_manager/job_priority.h"
#include "job_manager/entry_point.h"
#include "rw/core/codec/config.h"
#include "rw/core/codec/detail/chunkpackheader.h"

namespace EA {
namespace Jobs {
    class JobScheduler;
}
namespace Allocator {
    class ICoreAllocator;
}}


namespace rw {
namespace core {
namespace codec {

const uint32_t MINIMUM_CHUNK_ALIGNMENT = 16;

class ChunkReader;
class ChunkDecompressor;
class DecompressionGroup;
typedef DecompressionGroup *DecompressionGroupHandle;
typedef ChunkDecompressor *ChunkDecompressorHandle;

class RWCODEC_API ChunkDecompressor
{
    friend class ChunkReader;

    EA::Allocator::ICoreAllocator &mAllocator;
    EA::Jobs::JobScheduler *mJobScheduler;

    EA::Jobs::JobAffinity mAffinity;
    EA::Jobs::JobPriority mPriority;

    ChunkDecompressor(const ChunkDecompressor &);
    ChunkDecompressor &operator =(const ChunkDecompressor &);

public:
    ChunkDecompressor(EA::Allocator::ICoreAllocator *allocator, 
            EA::Jobs::JobScheduler *jobScheduler, 
            uint32_t unused = 1, 
            EA::Jobs::JobAffinity jobAffinity = EA::Jobs::JOB_AFFINITY_ANY, 
            EA::Jobs::JobPriority jobPriority = EA::Jobs::JOB_PRIORITY_DEFAULT);
    ~ChunkDecompressor();

    /// SetNumberOfJobs is deprecated, do not use.
    void SetNumberOfJobs(uint32_t);

    /// SetJobAffinities will set the affinity mask for the underlying chunk decompression
    /// job. Note that this will not affect any decompression jobs that are currently in flight.
    void SetJobAffinities(EA::Jobs::JobAffinity jobAffinity);

    /// SetJobAffinities will set the affinity mask for the underlying chunk decompression
    /// job. Note that this will not affect any decompression jobs that are currently in flight.
    void SetJobPriorities(EA::Jobs::JobPriority jobPriority);
    
    /// GetHandle() returns a handle that can be used for constructing a ChunkReader object.
    ChunkDecompressorHandle GetHandle() { return this; }
};


/// The ChunkReader performs the bulk of the work in decompressing chunked data. It is not
/// recommended that this be used directly but if you wish to do so you can take a look
/// at the source code for BufferedChunkDecompressor::DecompressAsync() to see how it is used.
class RWCODEC_API ChunkReader 
{
    ChunkDecompressor *mChunkDecompressor;
    detail::ChunkPackHeader mHeader;
    uint32_t mSegmentCount;
    mutable uint32_t mLastDecompressedSegment;
    size_t mCompressedDataSize;
    size_t mUncompressedDataSize;
    CHUNK_COMPRESSION_TYPE mType;

public:
    /// A default constructed ChunkReader object will have no associated ChunkDecompressor
    /// and so will not be able to perform asynchronous (parallel) decompression.
    ChunkReader();

    /// Construct a ChunkReader with a ChunkDecompressor handle.
    explicit ChunkReader(void *decompressorHandle);
    ~ChunkReader();

    /// Create a new decompression group. A decompression group is a group of jobs associated 
    /// with the same decompression batch and allows us to wait on/sleep until its completion.
    DecompressionGroupHandle GetNewDecompressionGroupHandle() const;

    /// Releases all resources associated with the decompression group.
    void ReleaseDecompressionGroupHandle(DecompressionGroupHandle group) const;

    /// Reads the file header and populates the internal ChunkReader data structures.
    void ReadFileHeader(const void *header);

    /// Reads the header before a chunk of compressed data. This header contains data
    /// like the type of compression used on the chunk as well as the size of the chunk.
    void ReadChunkInfo(const void *info);

    /// Returns the size of the current chunk of compressed data.
    size_t GetCurrentChunkDataCompressedSize() const;

    /// Returns the size of what the current chunk of compressed data will decompress to.
    size_t GetCurrentChunkDataDecompressedSize() const;

    /// Asynchronously decompress a compressed chunk. 
    void DecompressChunkDataAsync(void *outputBuffer,
            size_t outputBufferSize,
            const void *inputBuffer,
            size_t inputBufferSize,
            DecompressionGroupHandle group) const;

    /// Put the current thread to sleep until the group identified by the specified
    /// handle has finished its decompression.
    void SleepOnDecompressionGroup(DecompressionGroupHandle group) const;

    /// Synchronously decompress a compressed chunk. This is the only method usable if the
    /// ChunkReader is not constructed with a ChunkDecompressor handle and as such it requires
    /// the ICoreAllocator that is normally supplied in the process of constructing the 
    /// ChunkDecompressor.
    void DecompressChunkData(void *outputBuffer, 
            size_t outputBufferSize, 
            const void *inputBuffer, 
            size_t inputBufferSize,
            EA::Allocator::ICoreAllocator *allocator) const;

    /// SINGLEREADER_SleepOnAllDecompressions() is deprecated, do not use.
    void SINGLEREADER_SleepOnAllDecompressions() const;

    /// Returns the total number of segments in the associated file buffer.
    size_t GetNumberOfSegments() const;

    /// Returns the size of the decompressed data.
    size_t GetDecompressedSize() const;
    size_t GetExtraOffsetBeforeChunkInfo(const void *bufInPosition) const;

    /// Returns the size of the file header.
    static size_t GetFileHeaderSize();

    /// Returns true if the buffer provided is a chunk-packed buffer.
    static bool IsChunkPacked(const void *header);

    /// Returns true if the buffer provided appears to be valid chunk-packed data.
    static bool IsValidChunkHeader(const void *header);

    /// Returns the size of the decompressed data.
    static size_t GetDecompressedSize(const void *header);

    /// Returns the total number of segments in the buffer.
    static size_t GetNumberOfSegments(const void *header);

    /// Returns the size of the chunks in the associated chunk packed buffer.
    static size_t GetBlockSize(const void *header);

    /// Returns the size of the chunk info header that precedes each packed chunk.
    static size_t GetChunkInfoSize();
    static size_t DetailGetExtraOffsetBeforeChunkInfo(const void *bufInPosition, uint32_t alignment);
};


/// The buffered chunk decompressor is a friendly face wrapped around the chunk reader with the
/// purpose of decompressing an entire chunk-packed buffer into another. This class encapsulates
/// the messy details involved with iterating over chunks. 
class RWCODEC_API BufferedChunkDecompressor
{
public:
    BufferedChunkDecompressor(EA::Allocator::ICoreAllocator *allocator, 
            EA::Jobs::JobScheduler *jobScheduler, 
            uint32_t unused = 1, 
            EA::Jobs::JobAffinity jobAffinity = EA::Jobs::JOB_AFFINITY_ANY, 
            EA::Jobs::JobPriority jobPriority = EA::Jobs::JOB_PRIORITY_DEFAULT);

    /// DecompressAsync starts the decompression process and returns control to the user
    /// without waiting for it to finish. It is paired with the SleepOn() method below
    /// which will put the current thread to sleep until the decompression process is finished.
    size_t DecompressAsync(void *outputBuffer, size_t outputBufferSize, const void *inputBuffer, size_t inputBufferSize);

    /// Decompress synchronously performs a decompression from the input buffer into the output buffer. 
    size_t Decompress(void *outputBuffer, size_t outputBufferSize, const void *inputBuffer, size_t inputBufferSize);

    /// SleepOn will sleep the current thread until the decompression process associated with this
    /// BufferedChunkDecompressor object is finished.
    void SleepOn();

    /// SetNumberOfJobs is deprecated, do not use.
    void SetNumberOfJobs(uint32_t);

    /// Returns the size of the decompressed file.
    size_t GetDecompressedSize(const void *inputBuffer, size_t inputBufferSize);

private:
    EA::Allocator::ICoreAllocator *mAllocator;
    EA::Jobs::JobScheduler *mJobScheduler;
    ChunkDecompressor mDecompressor;
    ChunkReader mReader;
    DecompressionGroupHandle mGroup;

    BufferedChunkDecompressor(const BufferedChunkDecompressor &);
    BufferedChunkDecompressor &operator =(const BufferedChunkDecompressor &);
};


/// The BufferedChunkCompressor will take a provided buffer and compress it into chunks, 
/// or blocks, of a specified size.
class RWCODEC_API BufferedChunkCompressor
{
public:
    BufferedChunkCompressor(EA::Allocator::ICoreAllocator *allocator, CHUNK_COMPRESSION_TYPE compressionType);
    ~BufferedChunkCompressor();

    /// Sets the type of compression to use for this buffer.
    void SetCompressionType(CHUNK_COMPRESSION_TYPE compressionType);

    /// Sets the chunk, or block, size. By default the block size is set to 128kb though this may be tuned
    /// either for lower overhead or for better fit within SPU local store.
    void SetBlockSize(uint32_t blockSize);

    /// Perform a compression.
    size_t Compress(void *outputBuffer, size_t outputBufferSize, const void *inputBuffer, size_t inputBufferSize);

private:
    size_t CompressChunk(void *outputBuffer, 
            size_t outputBufferSize, 
            const void *inputBuffer, 
            size_t inputBufferSize, 
            CHUNK_COMPRESSION_TYPE compressionType);

    EA::Allocator::ICoreAllocator *mAllocator;
    CHUNK_COMPRESSION_TYPE mCompressionType;
    uint32_t mBlockSize;
    uint32_t mAlignment;
};


}}}

#endif
