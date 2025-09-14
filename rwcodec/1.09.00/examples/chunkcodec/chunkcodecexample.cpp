// (c) Electronic Arts. All Rights Reserved.

#include <EABase/eabase.h>
#include <coreallocator/icoreallocator_interface.h>
#include <EAAssert/eaassert.h>
#include <PPMalloc/EAGeneralAllocatorDebug.h>

#if defined(EA_PLATFORM_WINDOWS)
#pragma warning(push, 0)
#include <Windows.h>
#undef SetJob
#pragma warning(pop)
#endif

#include "rw/core/codec/chunkcodec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <new>      // placement new

#if defined(EA_PLATFORM_PS3)
#include <cell/sysmodule.h>
#include <sys/spu_initialize.h>
#include <time.h>
#include <assert.h>
#include <cell/spurs.h>
#include <sys/spu_initialize.h>
#include <spu_printf.h>
#endif

#if defined(EA_COMPILER_MSVC)
#pragma warning(disable: 4291)
#endif

#include <job_manager/job_scheduler.h>

#include <job_manager/allocator.h>
#include <coreallocator/icoreallocator_interface.h>
#include <PPMalloc/EAGeneralAllocatorDebug.h>

class Allocator : public EA::Allocator::ICoreAllocator
{

public:

    Allocator():
      mHeapBuffer(0),
          mHeap(0),
          mNumAllocationsAllowedAfterTeardown(0)
      {
          mHeapBuffer = static_cast<uint8_t*>(malloc(mHeapSize + mHeapAlignment));
          mHeap = static_cast<EA::Allocator::GeneralAllocatorDebug *>(malloc(sizeof(EA::Allocator::GeneralAllocatorDebug)));
          new (mHeap) EA::Allocator::GeneralAllocatorDebug(mHeapBuffer + mHeapAlignment - (uintptr_t)mHeapBuffer % mHeapAlignment,
              mHeapSize,
              false);

          mHeap->SetOption(EA::Allocator::GeneralAllocatorDebug::kOptionEnableMetrics, true);
#if !defined(EA_PLATFORM_WIN64)
          mHeap->SetDefaultDebugDataFlag(EA::Allocator::GeneralAllocatorDebug::kDebugDataIdCallStack);
#endif
          mHeap->SetDefaultDebugDataFlag(EA::Allocator::GeneralAllocatorDebug::kDebugDataIdName);

          EA::Allocator::GeneralAllocatorDebug::Metrics memMetrics = mHeap->GetMetrics(EA::Allocator::GeneralAllocatorDebug::kMetricTypeAll);

          if (mNumAllocationsAllowedAfterTeardown == 0)
          {
              mNumAllocationsAllowedAfterTeardown = static_cast<uint64_t>(memMetrics.mnAllocationCount);
          }
          else
          {
              EA_ASSERT(mNumAllocationsAllowedAfterTeardown == static_cast<uint64_t>(memMetrics.mnAllocationCount));
          }

      }
      ~Allocator()
      {
          EA::Allocator::GeneralAllocatorDebug::Metrics memMetrics = mHeap->GetMetrics(EA::Allocator::GeneralAllocatorDebug::kMetricTypeAll);

          if(static_cast<uint64_t>(memMetrics.mnAllocationCount) > mNumAllocationsAllowedAfterTeardown)
          {
              printf("%"PRIu64" memory leaks have been detected!\n", static_cast<uint64_t>(memMetrics.mnAllocationCount) - mNumAllocationsAllowedAfterTeardown);

              printf("--------------------------Begin Allocation Dump--------------------------\n");
              mHeap->TraceAllocatedMemory(Allocator::TraceFunction);
              printf("--------------------------End Allocation Dump--------------------------\n");
          }

          mHeap->~GeneralAllocatorDebug();
          free(mHeap);
          mHeap = 0;
          free(mHeapBuffer);
          mHeapBuffer = 0;
      }

      void* Alloc(size_t size, const char *name, unsigned int flags, unsigned int align, unsigned int alignOffset)
      {
          return mHeap->MallocAlignedDebug(size, align, alignOffset, (int)flags, 0, name);
      }
      void* Alloc(size_t size, const char *name, unsigned int flags)
      {
          return mHeap->MallocDebug(size, (int)flags, 0, name);
      }
      void Free(void *block, size_t size = 0)
      {
          (void)size;
          mHeap->Free(block);
      }
private:

    // Disable copy construction
    Allocator(const Allocator &);
    const Allocator &operator=(const Allocator &);

    uint8_t                                             *mHeapBuffer;
    EA::Allocator::GeneralAllocatorDebug                *mHeap;
    uint64_t                                            mNumAllocationsAllowedAfterTeardown;

    static const int mHeapAlignment = 128;
    static const int mHeapSize = 10 * 1024 * 1024;

    static void TraceFunction(const char* pTraceText, void* /*pContext*/)
    {
        if(pTraceText)
        {
            printf("%s\n", pTraceText);
        }
    }

};

Allocator gAllocator;
EA::Allocator::ICoreAllocator *gpAllocator = &gAllocator;

struct StreamingData
{
    void * pData;
    size_t dataSize;
    const void * pCurrentPosition; // Stores how far we've read in the data
};


using namespace rw::core;

EA::Jobs::JobScheduler * InitializeJobScheduler()
{
#ifdef EA_PLATFORM_PS3
    int ret = 0;
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
    EA_ASSERT(ret == CELL_OK || ret == CELL_SYSMODULE_ERROR_DUPLICATED);
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SHEAP);
    EA_ASSERT(ret == CELL_OK || ret == CELL_SYSMODULE_ERROR_DUPLICATED);
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYNC);
    EA_ASSERT(ret == CELL_OK || ret == CELL_SYSMODULE_ERROR_DUPLICATED);
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
    EA_ASSERT(ret == CELL_OK || ret == CELL_SYSMODULE_ERROR_DUPLICATED);
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_IO);
    EA_ASSERT(ret == CELL_OK || ret == CELL_SYSMODULE_ERROR_DUPLICATED);
    ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FS );
    EA_ASSERT(ret == CELL_OK || ret == CELL_SYSMODULE_ERROR_DUPLICATED);

    ret = sys_spu_initialize(6, 0);
    EA_ASSERT(ret == CELL_OK);
    (void)ret;
#endif

    EA::Jobs::SetAllocator(gAllocator);

    EA::Jobs::JobScheduler * pScheduler = (EA::Jobs::JobScheduler *)gpAllocator->Alloc(sizeof(EA::Jobs::JobScheduler), "mpJobScheduler", EA::Allocator::MEM_TEMP);
    new (pScheduler) EA::Jobs::JobScheduler;
    pScheduler->Initialize();
    EA::Jobs::StartWithDefaultThreads(*pScheduler);

    return pScheduler;
}

void Uninitialize(EA::Jobs::JobScheduler * pScheduler)
{
    pScheduler->~JobScheduler();
    gpAllocator->Free(pScheduler);
}

void GenerateTestData(size_t dataSize, void ** ppData)
{
    void * pTestBuf = gpAllocator->Alloc(dataSize, "BufferedChunkCompressorTest::pTestBuf", EA::Allocator::MEM_TEMP);

    unsigned char *pSrc = static_cast<unsigned char *>(pTestBuf);
    unsigned char *pSrcEnd = static_cast<unsigned char *>(pTestBuf) + dataSize;

    // populate the source buffer with some compressable data
    for ( ; pSrc < pSrcEnd; )
    {
        for (int j = 0; (j < 987) && pSrc < pSrcEnd; )
        {
            for (int k = 0; (k < ((j+3)*17)) && pSrc < pSrcEnd; ++k)
            {
                *pSrc++ = static_cast<unsigned char>(k ^ j);
            }
        }
    }

    *ppData = pTestBuf;
}

StreamingData * CreateCompressedStream(void * pSourceData, size_t sourceDataSize)
{
   
    void * pStreamRawData = NULL;
    // We allocate a bit extra incase it doesn't compress, but it likely will
    // This matters much more for small files, where the ChunkCodec header is 28 bytes, which could increase
    // the file relatively a lot(is the file is like 64 bytes)
    size_t streamSize = static_cast<size_t>(sourceDataSize + codec::ChunkReader::GetFileHeaderSize());
    pStreamRawData = gpAllocator->Alloc(streamSize, "CreateStream::pStreamRawData", EA::Allocator::MEM_TEMP, codec::MINIMUM_CHUNK_ALIGNMENT);

    // Again, we are using zlib, but we could use anything
    codec::BufferedChunkCompressor bufferedChunkCompressor(gpAllocator, codec::CHUNK_COMPRESSION_TYPE_ZLIB);
    size_t compressedSize = bufferedChunkCompressor.Compress(pStreamRawData, streamSize, pSourceData, sourceDataSize);

    StreamingData * pStreamData = static_cast<StreamingData *>(gpAllocator->Alloc(sizeof(StreamingData), "CreateStream::pStreamData", EA::Allocator::MEM_TEMP));
    pStreamData->dataSize = compressedSize;
    pStreamData->pData = pStreamRawData;
    pStreamData->pCurrentPosition = pStreamRawData;

    return pStreamData;
}

size_t GetTotalStreamSize(StreamingData * pStreamData)
{
    return pStreamData->dataSize;
}

size_t GetRemainingInStream(StreamingData * pStreamData)
{
    return pStreamData->dataSize - (static_cast<const char *>(pStreamData->pCurrentPosition) - static_cast<const char *>(pStreamData->pData));
}

size_t ReadStream(void * pBufOut, size_t * pBufOutSize, size_t amountToRead, StreamingData * pStreamData )
{
    // This function reads the next X bytes from the stream and copies them out
    // It returns how much was read
    // It also decrements pBufOutSize by the amount that was read
    EA_ASSERT(amountToRead <= *pBufOutSize);

    // We make sure if they request too much, we only give them what is left
    size_t amountRemainingInStream = pStreamData->dataSize + (static_cast<const char *>(pStreamData->pData) - static_cast<const char *>(pStreamData->pCurrentPosition));
    size_t amountToActuallyRead = amountRemainingInStream >= amountToRead ? amountToRead : amountRemainingInStream;

    // Assumedly instead of a memcpy you would actually be reading from a filesystem in practice
    memcpy(pBufOut, pStreamData->pCurrentPosition, amountToActuallyRead);

    pStreamData->pCurrentPosition = static_cast<const void *>(static_cast<const char *>(pStreamData->pCurrentPosition) + amountToActuallyRead);

    *pBufOutSize -= amountToActuallyRead;  

    // While this function supports requests for too much data, we do not want it for this example
    EA_ASSERT(amountToActuallyRead == amountToRead);

    return amountToActuallyRead;
}

void DestroyStream(StreamingData * pStreamData)
{
    gpAllocator->Free(pStreamData->pData);
    gpAllocator->Free(pStreamData);
}

void FreeTestData(void * pData)
{
    gpAllocator->Free(pData);
}

// This Example loads the whole file in at the beginning(GenerateTestData)
// and then decompresses it after all the loading is finished
void BufferedChunkCompressorExample(EA::Jobs::JobScheduler * pScheduler)
{
    // Generate the source data
    const size_t TEST_BUF_SIZE = 300*1024;
    void * pSourceData = NULL;
    GenerateTestData(TEST_BUF_SIZE, &pSourceData);
    EA_ASSERT(pSourceData != NULL);

    // Allocate buffers for the compressed data, and where to uncompress the data to
    // This is a bit bigger buffer than we need, and under normal conditions the size will shrink
    uint32_t compressedBufSize = static_cast<uint32_t>(TEST_BUF_SIZE*1.01 + codec::ChunkReader::GetFileHeaderSize());
    void * pCompressedBuf = gpAllocator->Alloc(compressedBufSize, "BufferedChunkCompressorExample::pCompressedBuf", EA::Allocator::MEM_TEMP, codec::MINIMUM_CHUNK_ALIGNMENT);
    void * pUncompressedBuf = gpAllocator->Alloc(TEST_BUF_SIZE, "BufferedChunkCompressorExample::pUncompressedBuf", EA::Allocator::MEM_TEMP, codec::MINIMUM_CHUNK_ALIGNMENT);

    // Compress the data with Zlib using the BufferedChunkCompressor
    // We could also compress with CHUNK_COMPRESSION_TYPE_REFPACK on any system or CHUNK_COMPRESSION_TYPE_LZX if we are on X360
    codec::BufferedChunkCompressor bufferedChunkCompressor(gpAllocator, codec::CHUNK_COMPRESSION_TYPE_ZLIB);
    uint32_t actualCompressedSize = bufferedChunkCompressor.Compress(pCompressedBuf, compressedBufSize, pSourceData, TEST_BUF_SIZE);

    // Now decompress with the BufferedChunkDecompressor
    // This is an easier to use form that uses the ChunkDecompressor form underneath, but is less powerful(as it must be buffered)
    // Note that it we do not need to say which algorithm it was compressed with, that is handled for us
    codec::BufferedChunkDecompressor bufferedChunkDecompressor(gpAllocator, pScheduler, 1);
    bufferedChunkDecompressor.Decompress(pUncompressedBuf, TEST_BUF_SIZE, pCompressedBuf, actualCompressedSize);

    // Now we just test to make sure the data matches
    EA_ASSERT_MSG(memcmp(pSourceData, pUncompressedBuf, TEST_BUF_SIZE) == 0, "Data mismatch." ); 

    FreeTestData(pSourceData);
    gpAllocator->Free(pCompressedBuf);
    gpAllocator->Free(pUncompressedBuf);

}

// This function loads the whole file into memory and decompresses from it, but it decompresses as the file loads
// Which is one step more complicated then loading in the whole file and then decompressing
void SimpleChunkStreamingExample(EA::Jobs::JobScheduler * pScheduler)
{ 
    // We just create some data and then compress it and return this as a stream
    void * pRawData = NULL;
    const size_t EXAMPLE_STREAM_SIZE = 1000*1024;
    GenerateTestData(EXAMPLE_STREAM_SIZE, &pRawData);

    StreamingData * pStreamData = CreateCompressedStream(pRawData, EXAMPLE_STREAM_SIZE);

    // So throughout this example we are going to be reading from the stream
    // in amounts that are good ideas for reading a bit more at once, but it
    // may be worth it to investigate reading in much larger amounts at a time

    // Create the buffer we are going to be streaming the compressed data into
    size_t compressedDataSize = GetTotalStreamSize(pStreamData);
    size_t compressedDataSizeRemaining = compressedDataSize;
    void * pCompressedDataBuffer = gpAllocator->Alloc(compressedDataSize, "SimpleChunkStreamingExample::pCompressedDataBuffer", EA::Allocator::MEM_TEMP, codec::MINIMUM_CHUNK_ALIGNMENT);
    // We use this just to make it easier to move down the buffer
    char * pCurrentCompressedDataBuffer = static_cast<char *>(pCompressedDataBuffer);


    // We arbitrarily choose to use 3 jobs in this example, but this could be any value from 1-6(or even more than that in theory, but it is not useful currently)
    const uint32_t numJobsToUse = 3;

    // Now lets create the ChunkDecompressor class
    // Multiple ChunkReaders can share the jobs the ChunkDecompressor runs, so it makes better use of the jobs
    // to reuse the ChunkDecompressor(and has a bit less overhead)
    // If you do not want the decompression of two different files to effect the other, use separate
    // instances of ChunkDecompressor
    codec::ChunkDecompressor chunkDecompressor(gpAllocator, pScheduler, numJobsToUse, EA::Jobs::JOB_AFFINITY_ANY, EA::Jobs::JOB_PRIORITY_DEFAULT );


    // Now lets also create a ChunkReader
    // ChunkReaders are responsible for parsing the file(1 ChunkReader per file) and passing on data to the ChunkDecompressor
    codec::ChunkReader chunkReader(chunkDecompressor.GetHandle());
    
    // We create a new decompression group handle, which in this example is the same for all chunks
    codec::DecompressionGroupHandle hGroup = chunkReader.GetNewDecompressionGroupHandle();


    // NOTE: We don't actually check to make sure that enough is actually read in this example, we just assert if not(we don't handle if not)
    // First we just read in the header, so we can get the decompressed size an allocate its buffer
    ReadStream(static_cast<void *>(pCurrentCompressedDataBuffer), &compressedDataSizeRemaining, codec::ChunkReader::GetFileHeaderSize(), pStreamData );
 

    // Now we use the ChunkReader to read in what we just streamed in
    chunkReader.ReadFileHeader(pCurrentCompressedDataBuffer);
    pCurrentCompressedDataBuffer += codec::ChunkReader::GetFileHeaderSize();

    // Now get the results from the chunk reader to figure out how big our output file is
    size_t uncompressedBufSize = chunkReader.GetDecompressedSize();
    void * pUncompressedData = gpAllocator->Alloc( uncompressedBufSize, "SimpleChunkStreamingExample::pUncompressedData", EA::Allocator::MEM_TEMP, codec::MINIMUM_CHUNK_ALIGNMENT);
    char * pCurrentUncompressedDataBuffer = static_cast<char *>(pUncompressedData); // We use this to have a buffer we can shift more easily

    // Again, we don't check in this example that we've run past what is available
    // Iterate over all the segments
    for(uint32_t ii = 0; ii < chunkReader.GetNumberOfSegments(); ii++)
    {
        // We need to get the extra offset, as the raw data is always 16-byte aligned(the value of codec::MINIMUM_CHUNK_ALIGNMENT currently)
        size_t offsetForAlignment = chunkReader.GetExtraOffsetBeforeChunkInfo(static_cast<const void *>(pCurrentCompressedDataBuffer));

        // We read in the both the extra offset and the amount of info data at the front of the block; 
        size_t nextReadSize = offsetForAlignment + codec::ChunkReader::GetChunkInfoSize();

        ReadStream(static_cast<void *>(pCurrentCompressedDataBuffer), &compressedDataSizeRemaining, nextReadSize, pStreamData );
        
        // So we now have read in the chunk info and
        // shift the buffer by the alignment amount
        pCurrentCompressedDataBuffer += offsetForAlignment;

        chunkReader.ReadChunkInfo(static_cast<const char *>(pCurrentCompressedDataBuffer));
        pCurrentCompressedDataBuffer += codec::ChunkReader::GetChunkInfoSize();

        // We could read in both the data of this one and the info of the next, but we don't in this example
        ReadStream(static_cast<void *>(pCurrentCompressedDataBuffer), &compressedDataSizeRemaining, chunkReader.GetCurrentChunkDataCompressedSize(), pStreamData );

        size_t remainingOutputBuffer = uncompressedBufSize - (pCurrentUncompressedDataBuffer - static_cast<char *>(pUncompressedData));
        size_t remainingInputBuffer = chunkReader.GetCurrentChunkDataCompressedSize(); // Again, we assume we got as much as requested

        // And now we start the decompression of what we just read
        chunkReader.DecompressChunkDataAsync(static_cast<void *>(pCurrentUncompressedDataBuffer), remainingOutputBuffer, pCurrentCompressedDataBuffer, remainingInputBuffer, hGroup);
    
        pCurrentCompressedDataBuffer += chunkReader.GetCurrentChunkDataCompressedSize();
        pCurrentUncompressedDataBuffer += chunkReader.GetCurrentChunkDataDecompressedSize();
    }

    // Then we sleep on the decompressions
    chunkReader.SleepOnDecompressionGroup(hGroup);

    // And then destroy the group, this lets the ChunkDecompressor know that the jobs that are doing the work can stop
    chunkReader.ReleaseDecompressionGroupHandle(hGroup);

    EA_ASSERT_MSG(memcmp(pUncompressedData, pRawData, EXAMPLE_STREAM_SIZE) == 0, "Data mismatch!");

    gpAllocator->Free(pCompressedDataBuffer);
    gpAllocator->Free(pUncompressedData);
    DestroyStream(pStreamData);
    FreeTestData(pRawData);
}

int main()
{
    EA::Jobs::JobScheduler * pScheduler = InitializeJobScheduler();
    SimpleChunkStreamingExample(pScheduler);
    BufferedChunkCompressorExample(pScheduler);
    Uninitialize(pScheduler);
    return 0;
}
