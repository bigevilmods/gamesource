// (c) Electronic Arts. All Rights Reserved.

/**
\file Defines structures used internally be the ChunkDecompressor.
*/

#ifndef DETAIL_JOBDATUM_H
#define DETAIL_JOBDATUM_H

#include "EABase/eabase.h"
#include "rw/core/codec/chunkcodec.h"
#include "rw/core/codec/chunkcompressiontypes.h"

#ifndef EA_PLATFORM_PS3_SPU
#include "eathread/eathread_semaphore.h"
#endif

namespace rw {
namespace core {
namespace codec {
namespace detail {

struct EA_PREFIX_ALIGN(16) JobDatum
{
    JobDatum()
        : OutputBuffer(NULL),
          OutputBufferSize(0),
          InputBuffer(NULL),
          InputBufferSize(0),
          Type(CHUNK_COMPRESSION_TYPE_UNKNOWN),
          Allocator(NULL)
    {}

    JobDatum(void *outputBuffer, 
            size_t outputBufferSize, 
            const void *inputBuffer, 
            size_t inputBufferSize,
            CHUNK_COMPRESSION_TYPE type,
            EA::Allocator::ICoreAllocator *allocator)
        : OutputBuffer(outputBuffer),
          OutputBufferSize(outputBufferSize),
          InputBuffer(inputBuffer),
          InputBufferSize(inputBufferSize),
          Type(type),
          Allocator(allocator)
    {}

    void *OutputBuffer;
    size_t OutputBufferSize;
    const void *InputBuffer;
    size_t InputBufferSize;
    CHUNK_COMPRESSION_TYPE Type;
    EA::Allocator::ICoreAllocator *Allocator;
} EA_POSTFIX_ALIGN(16);



}}}}

#endif

