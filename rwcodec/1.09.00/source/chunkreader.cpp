// (c) Electronic Arts. All Rights Reserved.

#include <new>
#include <stdlib.h>

#include "coreallocator/icoreallocator_interface.h"
#include "EAAssert/eaassert.h"
#include "eathread/eathread_atomic.h"
#include "eathread/eathread_futex.h"
#include "EAStdC/EAEndian.h"
#include "job_manager/job_instance_handle.h"
#include "job_manager/job_scheduler.h"
#include "rw/core/codec/chunkcodec.h"

#include "detail/decompressors.h"
#include "detail/jobdatum.h"
#include "detail/packinfo.h"

using namespace EA::Allocator;
using namespace EA::StdC;
using namespace EA::Jobs;
using namespace EA::Thread;
using namespace rw::core::codec::detail;

namespace rw {
namespace core {
namespace codec {

static EntryPoint DecompressionJobs[CHUNK_COMPRESSION_TYPE_NUM];

#ifdef EA_PLATFORM_PS3
static unsigned char LzxJobCode[] __attribute__((aligned(128), section(".spu_image"))) = {
    #include "rwcodec_lzx_job.job.h"
};

static unsigned char ZlibJobCode[] __attribute__((aligned(128), section(".spu_image"))) = {
    #include "rwcodec_zlib_job.job.h"
};

static unsigned char RefpackJobCode[] __attribute__((aligned(128), section(".spu_image"))) = {
    #include "rwcodec_refpack_job.job.h"
};

static unsigned char UncompressedJobCode[] __attribute__((aligned(128), section(".spu_image"))) = {
    #include "rwcodec_uncompressed_job.job.h"
};

static unsigned char LzmaJobCode[] __attribute__((aligned(128), section(".spu_image"))) = {
    #include "rwcodec_lzma_job.job.h"
};

#endif

struct StaticInitializationOfEntryPoints
{
    StaticInitializationOfEntryPoints()
    {
#ifdef EA_PLATFORM_PS3
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_ZLIB] = EntryPoint(JOB_ENVIRONMENT_SPU_THREADS, ZlibJobCode, sizeof ZlibJobCode);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_REFPACK] = EntryPoint(JOB_ENVIRONMENT_SPU_THREADS, RefpackJobCode, sizeof RefpackJobCode);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_LZX] = EntryPoint(JOB_ENVIRONMENT_SPU_THREADS, LzxJobCode, sizeof LzxJobCode);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_UNCOMPRESSED] = EntryPoint(
                JOB_ENVIRONMENT_SPU_THREADS, 
                UncompressedJobCode, 
                sizeof UncompressedJobCode);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_LZMA] = EntryPoint(JOB_ENVIRONMENT_SPU_THREADS, LzmaJobCode, sizeof LzmaJobCode);
#else
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_ZLIB] = EntryPoint((EntryPoint::LocalJob)DecompressZlib);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_REFPACK] = EntryPoint((EntryPoint::LocalJob)DecompressRefpack);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_LZX] = EntryPoint((EntryPoint::LocalJob)DecompressLZX);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_UNCOMPRESSED] = EntryPoint((EntryPoint::LocalJob)DecompressUncompressed);
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_LZMA] = EntryPoint((EntryPoint::LocalJob)DecompressLzma);
#endif
        DecompressionJobs[CHUNK_COMPRESSION_TYPE_LZ4] = EntryPoint((EntryPoint::LocalJob)DecompressLz4);
    }

} StaticInitializationOfEntryPoints_instance;

class DecompressionGroup
{
    JobScheduler &mJobScheduler;
    Futex mFutex;
    JobInstanceHandle mGatherHandle;
    Event mEnableGatherHandleEvent;
    JobAffinity mAffinity;
    JobPriority mPriority;

    DecompressionGroup(const DecompressionGroup&);
    DecompressionGroup &operator =(const DecompressionGroup &);

public:
    DecompressionGroup(JobScheduler &scheduler, JobAffinity affinity, JobPriority priority)
        : mJobScheduler(scheduler),
          mAffinity(affinity),
          mPriority(priority)
    {}

    void AddChunkDecompressionJob(void *outputBuffer,
            size_t outputBufferSize,
            const void *inputBuffer,
            size_t inputBufferSize,
            CHUNK_COMPRESSION_TYPE type)
    {
        // There should only be one thread queueing up decompression jobs per 
        // decompression group. If another thread attempts to do so at the same 
        // time it will trigger this abort. 
        if (!mFutex.TryLock())
        {
            EA_FAIL_MSG("You are trying to add to a group on which you are already sleeping. Don't do this.");
            abort();
        }

        if (!mGatherHandle.Valid())
        {
            mGatherHandle = mJobScheduler.AddNotReady(EntryPoint());

            // The gather job can run before its time if we're slow at adding jobs which 
            // causes a problem when we need to schedule dependencies. 
            mEnableGatherHandleEvent = mGatherHandle.AddBarrier();
        }

        EntryPoint entryPoint = DecompressionJobs[type];
        entryPoint.SetAffinity(mAffinity);
        entryPoint.SetPriority(mPriority);

        JobInstanceHandle decompressJob = mJobScheduler.Add(entryPoint,
                reinterpret_cast<Param>(outputBuffer),
                static_cast<Param>(outputBufferSize),
                reinterpret_cast<Param>(inputBuffer),
                static_cast<Param>(inputBufferSize));
        mGatherHandle.DependsOn(decompressJob);

        mFutex.Unlock();
    }

    bool IsDone() const { return mGatherHandle.IsDone(); }

    void SleepOn()
    {
        AutoFutex AF(mFutex);
        if (!mGatherHandle.Valid())
            return;

        mEnableGatherHandleEvent.Run();
        mGatherHandle.SleepOn();
        mGatherHandle = JobInstanceHandle();
    }
};

ChunkReader::ChunkReader(void *decompressorHandle)
    : mChunkDecompressor(static_cast<ChunkDecompressor *>(decompressorHandle)),
      mSegmentCount(0),
      mLastDecompressedSegment(0),
      mCompressedDataSize(0),
      mUncompressedDataSize(0),
      mType(CHUNK_COMPRESSION_TYPE_UNKNOWN)
{
    memset(&mHeader, 0, sizeof mHeader);
}

ChunkReader::ChunkReader()
{
    memset(this, 0, sizeof *this);
}

ChunkReader::~ChunkReader()
{
}

DecompressionGroupHandle ChunkReader::GetNewDecompressionGroupHandle() const
{
    EA_ASSERT(mChunkDecompressor);

    ICoreAllocator &allocator = mChunkDecompressor->mAllocator;
    return new (allocator.Alloc(sizeof(DecompressionGroup), "codec::DecompressionGroup", EA::Allocator::MEM_PERM, 16))
        DecompressionGroup(*mChunkDecompressor->mJobScheduler, mChunkDecompressor->mAffinity, mChunkDecompressor->mPriority);
}

void ChunkReader::ReleaseDecompressionGroupHandle(DecompressionGroupHandle group) const
{
    EA_ASSERT(group);
    EA_ASSERT(mChunkDecompressor);

    group->~DecompressionGroup();
    ICoreAllocator &allocator = mChunkDecompressor->mAllocator;
    allocator.Free(group);
}

void ChunkReader::ReadFileHeader(const void *rawHeader)
{
    EA_ASSERT(rawHeader);

    // ReadFileHeader has to reset the internal state of the object so
    // that if it is recycled from use with another file we don't see
    // odd behavior.
    new (this) ChunkReader(mChunkDecompressor);

    const ChunkPackHeader *header = static_cast<const ChunkPackHeader *>(rawHeader);
    memcpy(&mHeader.ID[0], &header->ID[0], sizeof mHeader.ID);
    mHeader.VersionNum = ReadFromBigEndianUint32(&header->VersionNum);
    mHeader.UncompressedLength = ReadFromBigEndianUint32(&header->UncompressedLength);
    mHeader.BlockSize = ReadFromBigEndianUint32(&header->BlockSize);
    mHeader.NumSegments = ReadFromBigEndianUint32(&header->NumSegments);
    mHeader.Alignment = ReadFromBigEndianUint32(&header->Alignment);

    mType = mHeader.GetType();

    EA_ASSERT(mHeader.Alignment == MINIMUM_CHUNK_ALIGNMENT);
    EA_ASSERT(mHeader.Alignment % MINIMUM_CHUNK_ALIGNMENT == 0);
    EA_ASSERT_MSG(mHeader.VersionNum == ChunkPackHeader::CHUNK_FILE_VERSION_NUMBER, 
            "Compressed with a different version of chunkpack then it is being decompressed with!");
    EA_ASSERT_MSG(mType != CHUNK_COMPRESSION_TYPE_UNKNOWN, "Unknown compression type in file header.");
}

void ChunkReader::ReadChunkInfo(const void *info)
{
    EA_ASSERT(info);

    ++mSegmentCount;

    const PackInfo *packInfo = static_cast<const PackInfo *>(info);
    uint32_t compressedDataSize = ReadFromBigEndianUint32(&packInfo->CompressedDataSize);
    uint32_t uncompressedDataSize = (mHeader.NumSegments != mSegmentCount) ? 
        mHeader.BlockSize : 
        mHeader.UncompressedLength - (mSegmentCount - 1) * mHeader.BlockSize;

    mCompressedDataSize = static_cast<size_t>(compressedDataSize);
    mUncompressedDataSize = static_cast<size_t>(uncompressedDataSize);
    mType = static_cast<CHUNK_COMPRESSION_TYPE>(ReadFromBigEndianUint32(&packInfo->CompressionType));

    EA_ASSERT(mType >= CHUNK_COMPRESSION_TYPE_FIRST && mType < CHUNK_COMPRESSION_TYPE_UNKNOWN);
    EA_ASSERT(mCompressedDataSize != 0);
    EA_ASSERT(mUncompressedDataSize != 0);
}

size_t ChunkReader::GetCurrentChunkDataCompressedSize() const
{
    return mCompressedDataSize;
}

size_t ChunkReader::GetCurrentChunkDataDecompressedSize() const
{
    return mUncompressedDataSize;
}

void ChunkReader::DecompressChunkDataAsync(void *outputBuffer,
        size_t outputBufferSize,
        const void *inputBuffer,
        size_t inputBufferSize,
        DecompressionGroupHandle group) const
{
    // If the segment count hasn't increased then we haven't read any chunk info since our last call to DecompressChunkDataAsync
    EA_ASSERT_MSG(mSegmentCount > mLastDecompressedSegment, 
            "This chunk has already been decompressed. Are you sure you want to do this again?");

    EA_ASSERT(outputBuffer && inputBuffer);
    EA_ASSERT((reinterpret_cast<size_t>(outputBuffer) % MINIMUM_CHUNK_ALIGNMENT) == 0);
    EA_ASSERT((reinterpret_cast<size_t>(inputBuffer) % MINIMUM_CHUNK_ALIGNMENT) == 0);
    EA_ASSERT(inputBufferSize >= mCompressedDataSize);

    // Note that if the assert below triggers, the chunk size may be larger than the filesystem read
    // quanta. If you are using rwfilesystem, ensure that the specified read quanta is at least as big
    // as the chunk size (default: 128kb)
    EA_ASSERT(outputBufferSize >= mUncompressedDataSize);
    (void)inputBufferSize;
    (void)outputBufferSize;

    EA_ASSERT_MSG(mChunkDecompressor, 
            "A decompressor was not provided -- either provide one or use the synchronous DecompressChunkData function.");

#ifdef EA_PLATFORM_PS3
    EA_ASSERT((MAX_DECOMPRESSED_SIZE >= mUncompressedDataSize) && (MAX_DECOMPRESSED_SIZE >= mCompressedDataSize));
#endif

    // Note: This passes along the already figure out sizes for decompressed and compressed sizes.
    // The outputBufferSize and inputBufferSize are just used for error checking.
    group->AddChunkDecompressionJob(outputBuffer, mUncompressedDataSize, inputBuffer, mCompressedDataSize, mType);

    mLastDecompressedSegment = mSegmentCount;
}

void ChunkReader::DecompressChunkData(void *outputBuffer, 
        size_t outputBufferSize, 
        const void *inputBuffer, 
        size_t inputBufferSize,
        EA::Allocator::ICoreAllocator *) const
{
    EA_ASSERT(outputBuffer && inputBuffer);
    EA_ASSERT_MSG(mType != CHUNK_COMPRESSION_TYPE_UNKNOWN, "The file header has not been parsed with ReadFileHeader!");

    uintptr_t params[4] = {
        reinterpret_cast<uintptr_t>(outputBuffer),
        static_cast<uintptr_t>(outputBufferSize),
        reinterpret_cast<uintptr_t>(inputBuffer),
        static_cast<uintptr_t>(inputBufferSize)
    };

    switch (mType)
    {
        case CHUNK_COMPRESSION_TYPE_REFPACK:
            detail::DecompressRefpack(params[0], params[1], params[2], params[3]);
            break;
        case CHUNK_COMPRESSION_TYPE_ZLIB:
            detail::DecompressZlib(params[0], params[1], params[2], params[3]);
            break;
        case CHUNK_COMPRESSION_TYPE_LZX:
            detail::DecompressLZX(params[0], params[1], params[2], params[3]);
            break;
        case CHUNK_COMPRESSION_TYPE_LZ4:
            detail::DecompressLz4(params[0], params[1], params[2], params[3]);
            break;
        case CHUNK_COMPRESSION_TYPE_UNCOMPRESSED:
            detail::DecompressUncompressed(params[0], params[1], params[2], params[3]);
            break;
        case CHUNK_COMPRESSION_TYPE_UNKNOWN:
        default:
            EA_FAIL_MSG("Cannot decompress this type!");
            break;
    }
}

void ChunkReader::SleepOnDecompressionGroup(DecompressionGroupHandle group) const
{
    EA_ASSERT(group);

    group->SleepOn();
}

void ChunkReader::SINGLEREADER_SleepOnAllDecompressions() const
{
    EA_FAIL_MSG("Deprecated, use SleepOnDecompressionGroup instead");
    abort();
}

size_t ChunkReader::GetNumberOfSegments() const
{
    return mHeader.NumSegments;
}

size_t ChunkReader::GetDecompressedSize() const
{
    return mHeader.UncompressedLength;
}

size_t ChunkReader::GetFileHeaderSize()
{
    return sizeof(ChunkPackHeader);
}

bool ChunkReader::IsChunkPacked(const void *data)
{
    const detail::ChunkPackHeader *header = static_cast<const detail::ChunkPackHeader *>(data);

    switch (header->GetType())
    {
        case CHUNK_COMPRESSION_TYPE_UNCOMPRESSED:
        case CHUNK_COMPRESSION_TYPE_LZMA:
        case CHUNK_COMPRESSION_TYPE_LZ4:
        case CHUNK_COMPRESSION_TYPE_ZLIB:
        case CHUNK_COMPRESSION_TYPE_REFPACK:
        case CHUNK_COMPRESSION_TYPE_LZX:
            if (ReadFromBigEndianUint32(&header->VersionNum) == detail::ChunkPackHeader::CHUNK_FILE_VERSION_NUMBER)
                return true;
        default:
            break;
    }

    return false;
}

bool ChunkReader::IsValidChunkHeader(const void *data)
{
    if (!IsChunkPacked(data))
        return false;

    const detail::ChunkPackHeader *header = static_cast<const detail::ChunkPackHeader *>(data);
    uint32_t numSegments = ReadFromBigEndianUint32(&header->NumSegments);
    uint32_t blockSize = ReadFromBigEndianUint32(&header->BlockSize);
    uint32_t uncompressedLength = ReadFromBigEndianUint32(&header->UncompressedLength);
    uint32_t segments = uncompressedLength / blockSize;
    segments += (uncompressedLength % blockSize) == 0 ? 0 : 1;
    if (segments != numSegments)
        return false;

    return true;
}

size_t ChunkReader::GetDecompressedSize(const void *header)
{
    return static_cast<size_t>(ReadFromBigEndianUint32(&static_cast<const ChunkPackHeader *>(header)->UncompressedLength));
}

size_t ChunkReader::GetNumberOfSegments(const void *header)
{
    return static_cast<size_t>(ReadFromBigEndianUint32(&static_cast<const ChunkPackHeader *>(header)->NumSegments));
}

size_t ChunkReader::GetBlockSize(const void *header)
{
    return static_cast<size_t>(ReadFromBigEndianUint32(&static_cast<const ChunkPackHeader *>(header)->BlockSize));
}

size_t ChunkReader::GetChunkInfoSize()
{
    return sizeof(PackInfo);
}

size_t ChunkReader::GetExtraOffsetBeforeChunkInfo(const void * bufInPosition) const
{
    return DetailGetExtraOffsetBeforeChunkInfo(bufInPosition, mHeader.Alignment);
}

size_t ChunkReader::DetailGetExtraOffsetBeforeChunkInfo(const void * bufInPosition, uint32_t alignment)
{
    size_t shiftAmount = (alignment - sizeof(PackInfo)) - (reinterpret_cast<size_t>(bufInPosition) % alignment);
    return ((shiftAmount % alignment) + alignment) % alignment;
}

}}}
