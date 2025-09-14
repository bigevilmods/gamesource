// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAStdC/EAEndian.h"

#include "rw/core/codec/chunkcodec.h"
#include "rw/core/codec/icodec.h"
#include "rw/core/codec/refpack.h"
#include "rw/core/codec/zlib.h"
#include "detail/decompressors.h"
#include "detail/packinfo.h"
#include "LzmaLib.h"
#include "zlib/zlib.h"
#include "lz4/lz4.h"

#include <stdio.h>
#include <stdlib.h>


using namespace EA::StdC;

#if defined(EA_PLATFORM_XENON) || defined(RW_CORE_CODEC_LZXCOMPRESSION)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef EA_PLATFORM_XENON
#include <Xtl.h>
#else
#include <Windows.h>
#endif
#include <xcompress.h>

static size_t LZXCompress(void *outputBuffer, size_t outputBufferSize, const void *inputBuffer, size_t inputBufferSize)
{
    HRESULT hr;
    XMEMCOMPRESSION_CONTEXT cmpContext = NULL;

    // Create a compression context.
    if (FAILED(hr = XMemCreateCompressionContext(XMEMCODEC_LZX, NULL, 0, &cmpContext)))
    {
        return 0;
    }

#ifdef EA_ASSERT_ENABLED
    // This does a check on the desired size and sees if we have enough..
    // When we give XMemCompress a 0 for the out buffer, it gives us the needed size back
    SIZE_T neededOutBufSize = 0 ;
    XMemCompress(cmpContext, outputBuffer, &neededOutBufSize, inputBuffer, inputBufferSize);
    EA_ASSERT(outputBufferSize >= neededOutBufSize);
#endif

    SIZE_T compressedSize = outputBufferSize;

    if (FAILED(hr = XMemCompress(cmpContext, outputBuffer, &compressedSize, inputBuffer, static_cast<SIZE_T>(inputBufferSize))))
    {
        EA_FAIL();
        return 0;
    }

    XMemDestroyCompressionContext(cmpContext);
    return static_cast<size_t>(compressedSize);
}
#endif

namespace rw {
namespace core {
namespace codec {

// I'd like to use a constant here instead of a preprocessor definitition
// but SN's compiler complains that it isn't used as it only appears in
// assertion expressions. Rad. -mburke
#define BLOCK_SIZE_MULTIPLE 16

BufferedChunkCompressor::BufferedChunkCompressor(EA::Allocator::ICoreAllocator *allocator, 
        CHUNK_COMPRESSION_TYPE compressionType)
    : mAllocator(allocator),
      mCompressionType(compressionType),
      mAlignment(MINIMUM_CHUNK_ALIGNMENT)
{
    // We use this just as a default for the chunk compressor, it is a value that can be handle on SPUs
    // unless we are using LZX
    const uint32_t defaultChunkSize = MAX_DECOMPRESSED_SIZE;
    mBlockSize = defaultChunkSize;

    if (compressionType == CHUNK_COMPRESSION_TYPE_LZX)
    {
        // Since we do not need to fix on local store for X360, make it a bit bigger
        const uint32_t defaultLzxChunkSize = 256*1024;
        mBlockSize = defaultLzxChunkSize;
    }

    EA_ASSERT(mBlockSize % BLOCK_SIZE_MULTIPLE == 0);
    EA_ASSERT(mBlockSize != 0);
}

BufferedChunkCompressor::~BufferedChunkCompressor()
{
}

void BufferedChunkCompressor::SetBlockSize(uint32_t blockSize)
{
    if (blockSize == 0)
        return;

    // mBlockSize should already be set to the default value (MAX_DECOMPRESSED_SIZE)
    EA_ASSERT(mBlockSize != 0);
    EA_ASSERT(blockSize != 0);
    mBlockSize = blockSize;
    EA_ASSERT(mBlockSize % BLOCK_SIZE_MULTIPLE == 0);
}

void BufferedChunkCompressor::SetCompressionType(CHUNK_COMPRESSION_TYPE compressionType)
{
    mCompressionType = compressionType;
}

size_t BufferedChunkCompressor::Compress(void *outputBuffer, size_t outputBufferSize, const void *inputBuffer, size_t inputBufferSize)
{
    EA_ASSERT(reinterpret_cast<uintptr_t>(outputBuffer) % 16 == 0);
    EA_ASSERT(reinterpret_cast<uintptr_t>(inputBuffer) % 16 == 0);

    char *outputBufferBytePtr = static_cast<char *>(outputBuffer);
    const char *inputBufferBytePtr = static_cast<const char *>(inputBuffer);

    size_t numSegments = (inputBufferSize / mBlockSize) + ((inputBufferSize % mBlockSize) ? 1 : 0);
    memset(outputBuffer, 0, outputBufferSize);

    // Write out the file header.
    detail::ChunkPackHeader *fileHeader = reinterpret_cast<detail::ChunkPackHeader *>(outputBufferBytePtr);
    fileHeader->SetType(mCompressionType);
    WriteToBigEndian(&fileHeader->UncompressedLength, static_cast<uint32_t>(inputBufferSize));
    WriteToBigEndian(&fileHeader->BlockSize, mBlockSize);
    WriteToBigEndian(&fileHeader->VersionNum, detail::ChunkPackHeader::CHUNK_FILE_VERSION_NUMBER);
    WriteToBigEndian(&fileHeader->NumSegments, static_cast<uint32_t>(numSegments));
    WriteToBigEndian(&fileHeader->Alignment, mAlignment);

    // Move the out buffer by the size of the header...
    outputBufferBytePtr += sizeof(detail::ChunkPackHeader);

    size_t bytesToCompress = inputBufferSize;

    for (size_t i = 0; bytesToCompress != 0; ++i)
    {
        // Update the pointer so that it is on an aligned boundary.
        size_t extraOffsetSize = ChunkReader::DetailGetExtraOffsetBeforeChunkInfo(outputBufferBytePtr, mAlignment);
        outputBufferBytePtr += extraOffsetSize; 

        const void *chunkBufIn = static_cast<const void *>(inputBufferBytePtr);
        void *chunkBufOut = reinterpret_cast<void *>(static_cast<char *>(outputBufferBytePtr) + sizeof(detail::PackInfo));
        
        // We shift an uint32 size over to make room for the compressed size, which we will write after we compress
        // Output buffer remaining is calculated
        size_t chunkOutBufSize = outputBufferSize - (static_cast<char *>(chunkBufOut) - static_cast<char *>(outputBuffer));
        // Generates the input buffer size depending on what segment we are in
        size_t chunkInBufSize = bytesToCompress < mBlockSize ? bytesToCompress : mBlockSize;
        bytesToCompress -= chunkInBufSize;

        // So we now write out the compressed data
        size_t chunkOutputSize = static_cast<size_t>(CompressChunk(chunkBufOut, 
                    chunkOutBufSize, chunkBufIn, chunkInBufSize, mCompressionType));
        
        // Default to the compression type used
        // We only change this is the uncompressed size is different
        CHUNK_COMPRESSION_TYPE compressionType = mCompressionType;

        // If the compressed size is bigger or equal, just use the uncompressed chunk
        if (chunkOutputSize >= chunkInBufSize)
        {
            memcpy(chunkBufOut, chunkBufIn, chunkInBufSize);
            chunkOutputSize = chunkInBufSize;
            compressionType = CHUNK_COMPRESSION_TYPE_UNCOMPRESSED;
        }

        // START WRITING OUT THE PACK INFO
        // Now write out the size into the output buffer before the chunk
        EA_ASSERT(chunkOutputSize != 0);
        WriteToBigEndian(outputBufferBytePtr, static_cast<uint32_t>(chunkOutputSize));
        WriteToBigEndian(static_cast<char *>(outputBufferBytePtr) + sizeof(uint32_t), static_cast<uint32_t>(compressionType));

        outputBufferBytePtr += sizeof(detail::PackInfo); 
        // END WRITING OUT THE PACK INFO                   

        // Now shift the output buffer by the size of the chunk info(compressed size)
        // and the size of the compressed data
        outputBufferBytePtr += chunkOutputSize;

        // And shift the input buffer
        inputBufferBytePtr += chunkInBufSize;
    }
 
    size_t totalWritten = static_cast<size_t>(outputBufferBytePtr - static_cast<char *>(outputBuffer));
    EA_ASSERT(totalWritten <= outputBufferSize);

    // Make sure we consumed all input
    EA_ASSERT(static_cast<size_t>(inputBufferBytePtr - static_cast<const char *>(inputBuffer)) == inputBufferSize); 
    return totalWritten;
}

static void *Zalloc(void *allocator, unsigned int items, unsigned int size)
{
    return static_cast<EA::Allocator::ICoreAllocator *>(allocator)->Alloc(items * size, "zlib temp buffer", EA::Allocator::MEM_TEMP);
}

static void Zfree(void *allocator, void *address)
{
    static_cast<EA::Allocator::ICoreAllocator *>(allocator)->Free(address);
}

size_t BufferedChunkCompressor::CompressChunk(void *outputBuffer, 
        size_t outputBufferSize, 
        const void *inputBuffer, 
        size_t inputBufferSize, 
        CHUNK_COMPRESSION_TYPE compressionType)
{
    EA_ASSERT((reinterpret_cast<size_t>(outputBuffer) % MINIMUM_CHUNK_ALIGNMENT) == 0);
    bool error = false;

    // This function just compresses the output into the given buffer, the functions who call it will deal with chunk formatting
    switch (compressionType)
    {
    case CHUNK_COMPRESSION_TYPE_ZLIB:
        {
            z_stream_s stream = {};

            stream.opaque = mAllocator;
            stream.zalloc = Zalloc;
            stream.zfree = Zfree;

            deflateInit2(&stream, 
                    Z_BEST_COMPRESSION, 
                    Z_DEFLATED, 
                    -MAX_WBITS, 
                    8, // Default memlevel parameter, see zlib.h for details.
                    Z_DEFAULT_STRATEGY);

            stream.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(inputBuffer));
            stream.avail_in = static_cast<uInt>(inputBufferSize);
            stream.next_out = static_cast<Bytef *>(outputBuffer);
            stream.avail_out = static_cast<uInt>(outputBufferSize);

            int rv = deflate(&stream, Z_FINISH);
            size_t totalOut = static_cast<size_t>(stream.total_out);
            EA_ASSERT(rv == Z_STREAM_END);
            (void)rv;

            deflateEnd(&stream);
            return totalOut;
        }

    case CHUNK_COMPRESSION_TYPE_REFPACK:
        {
            RefpackDeflate refpackDeflate(mAllocator, ICodec::DEFLATE_BEST_SIZE);
            refpackDeflate.SetInputBuffer(inputBuffer, inputBufferSize);
            refpackDeflate.SetOutputBuffer(outputBuffer, outputBufferSize);

            ICodec::Status status = refpackDeflate.Exec(ICodec::FINISH);
            EA_ASSERT(status == ICodec::STREAM_END);
            (void)status;

            return static_cast<size_t>(refpackDeflate.GetTotalOut());
        }

    case CHUNK_COMPRESSION_TYPE_LZMA:
        {
            size_t localOutputBufferSize = outputBufferSize;
            unsigned char properties[5] = {};
            size_t propertiesSize = 5;

            int rv = LzmaCompress(static_cast<unsigned char *>(outputBuffer), &localOutputBufferSize, 
                    static_cast<const unsigned char *>(inputBuffer), inputBufferSize,
                    properties, 
                    &propertiesSize,
                    1,
                    LZMA_DICTIONARY_SIZE,
                    LZMA_LC,
                    LZMA_LP,
                    LZMA_PB,
                    LZMA_FB,
                    1);
            EA_ASSERT(rv == SZ_OK);
            (void)rv;

            return localOutputBufferSize;
        }

    case CHUNK_COMPRESSION_TYPE_LZ4:
        {
            // Note:  Perhaps the high compression ratio version of the compressor should also be available.
            size_t result = static_cast<size_t>(LZ4_compress_limitedOutput(static_cast<const char*>(inputBuffer), static_cast<char*>(outputBuffer), static_cast<int>(inputBufferSize), static_cast<int>(outputBufferSize)));
            if(result == 0)
            {
                // If LZ4 failed to compress the chunk it is likely because the compressed data was actually too large to fit in the outputBuffer
                // A return value of outputBufferSize + 1 is used so that the chunk compressor will fall back to an "uncompressed" block type.
                return outputBufferSize + 1;
            }

            return result;
        }

    case CHUNK_COMPRESSION_TYPE_UNCOMPRESSED:
        {
            EA_ASSERT(outputBufferSize >= inputBufferSize);
            memcpy(outputBuffer, inputBuffer, inputBufferSize);
            return inputBufferSize;
        }

#if defined(EA_PLATFORM_XENON) || defined(RW_CORE_CODEC_LZXCOMPRESSION)
    case CHUNK_COMPRESSION_TYPE_LZX:
        {
            return LZXCompress(outputBuffer, outputBufferSize, inputBuffer, inputBufferSize);
        }
#endif

    default:
        // Unknown type?
        error = true;
    }

    // This error flag is necessary because some compilers (clang specially) will complain of 
    // possible reaching the end of the function without returning a value and if we try to return one 
    // it will complain of “unreachable code”. Doing this will fool the compiler making it happy again
    if (error)
    {
        fprintf(stderr, "%s(%d): Compression type not supported!", __FILE__, __LINE__);
        exit(1);
    }
   
    return 0;
}

}}}

