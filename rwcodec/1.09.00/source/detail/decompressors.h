// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \brief        Defines some APIs used by jobs doing chunked decompression

#ifndef DETAIL_DECOMPRESSORS_H
#define DETAIL_DECOMPRESSORS_H

#include "EABase/eabase.h"

namespace rw { 
namespace core { 
namespace codec {
namespace detail {

#ifdef EA_PLATFORM_PS3_SPU
    void Decompress(void *outputBuffer, size_t outputBufferSize, const void *inputBuffer, size_t inputBufferSize);
#else
    void DecompressZlib(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize);
    void DecompressRefpack(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize);
    void DecompressLZX(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize);
    void DecompressUncompressed(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize);
    void DecompressLzma(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize);
    void DecompressLz4(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize);
#endif

}

static const uint32_t MAX_DECOMPRESSED_SIZE = 128 * 1024;
static const uint8_t LZMA_PB = 2;
static const uint8_t LZMA_LP = 0;
static const uint8_t LZMA_LC = 3;
static const uint8_t LZMA_FB = 32;
static const unsigned LZMA_DICTIONARY_SIZE = 64 * 1024;
static const uint8_t LZMA_PROPERTIES_BYTE_0 = LZMA_PB * 45 + LZMA_LP * 9 + LZMA_LC;

}}}

#endif

