// (c) Electronic Arts. All Rights Reserved.

#include "rw/core/codec/chunkcodec.h"

namespace rw {
namespace core {
namespace codec {

BufferedChunkDecompressor::BufferedChunkDecompressor(EA::Allocator::ICoreAllocator *allocator, 
        EA::Jobs::JobScheduler *jobScheduler, 
        uint32_t unused, 
        EA::Jobs::JobAffinity jobAffinity, 
        EA::Jobs::JobPriority jobPriority):
    mAllocator(allocator),
    mJobScheduler(jobScheduler),
    mDecompressor(allocator, jobScheduler, unused, jobAffinity, jobPriority),
    mGroup(NULL)
{
}

void BufferedChunkDecompressor::SetNumberOfJobs(uint32_t)
{
    EA_FAIL_MSG("Deprecated, do not use.");
}

size_t BufferedChunkDecompressor::DecompressAsync(void *outputBuffer, 
        size_t outputBufferSize, 
        const void *inputBuffer, 
        size_t inputBufferSize)
{
    EA_ASSERT((reinterpret_cast<size_t>(inputBuffer) % MINIMUM_CHUNK_ALIGNMENT) == 0);
    EA_ASSERT((reinterpret_cast<size_t>(outputBuffer) % MINIMUM_CHUNK_ALIGNMENT) == 0);

    EA_ASSERT_MSG(mGroup == NULL, 
            "Please either wait for this decompression to complete or construct a new BufferedChunkDecompressor object");

    // Initialize (or reset, if this BCD object is used for multiple decompressions)
    // the ChunkReader with our decompressor.
    mReader = ChunkReader(mDecompressor.GetHandle());
    mGroup = mReader.GetNewDecompressionGroupHandle();

    const char *inputBufferPosition = static_cast<const char *>(inputBuffer);
    char *outputBufferPosition = static_cast<char *>(outputBuffer);

    // Read the file header and skip past it in the input stream.
    mReader.ReadFileHeader(inputBufferPosition); 
    inputBufferPosition += mReader.GetFileHeaderSize();

    const size_t decompressedSize = mReader.GetDecompressedSize();
    const size_t numSegments = mReader.GetNumberOfSegments(); 

    // Iterate over each segment in the buffer and decompress it.
    for (size_t i = 0; i < numSegments; ++i)
    {
        // This shift is to account for compressed data alignment
        inputBufferPosition += mReader.GetExtraOffsetBeforeChunkInfo(inputBufferPosition);

        // Parse the header information for the current chunk. This information includes 
        // the compression method for the chunk as well as its compressed size.
        mReader.ReadChunkInfo(inputBufferPosition);

        // Skip past the header data and position our pointer at the start of the raw
        // compressed data.
        inputBufferPosition += mReader.GetChunkInfoSize();

        size_t outputBufferRemaining = outputBufferSize - 
            (reinterpret_cast<size_t>(outputBufferPosition) - reinterpret_cast<size_t>(outputBuffer));
        size_t inputBufferRemaining = inputBufferSize - 
            (reinterpret_cast<size_t>(inputBufferPosition) - reinterpret_cast<size_t>(inputBuffer));

        // Queue up a decompression job.
        mReader.DecompressChunkDataAsync(outputBufferPosition, outputBufferRemaining, inputBufferPosition, inputBufferRemaining, mGroup);

        inputBufferPosition += mReader.GetCurrentChunkDataCompressedSize();
        outputBufferPosition += mReader.GetCurrentChunkDataDecompressedSize();
    }

    // Make sure we didn't over run the output buffer and that all input was consumed.
    EA_ASSERT((reinterpret_cast<size_t>(outputBufferPosition) - reinterpret_cast<size_t>(outputBuffer)) <= outputBufferSize); 
    EA_ASSERT((reinterpret_cast<size_t>(inputBufferPosition) - reinterpret_cast<size_t>(inputBuffer)) == inputBufferSize); 

    return decompressedSize;
}

size_t BufferedChunkDecompressor::Decompress(void *outputBuffer, 
        size_t outputBufferSize, 
        const void *inputBuffer, 
        size_t inputBufferSize)
{
    const size_t decompressedSize = DecompressAsync(outputBuffer, outputBufferSize, inputBuffer, inputBufferSize);
    SleepOn();

    return decompressedSize;
}

void BufferedChunkDecompressor::SleepOn()
{
    mReader.SleepOnDecompressionGroup(mGroup);
    mReader.ReleaseDecompressionGroupHandle(mGroup);
    mGroup = NULL;
}

size_t BufferedChunkDecompressor::GetDecompressedSize(const void *inputBuffer, size_t inputBufferSize)
{
    EA_ASSERT(inputBufferSize >= ChunkReader::GetFileHeaderSize());
    (void)inputBufferSize;
    return ChunkReader::GetDecompressedSize(inputBuffer);
}

}}}

