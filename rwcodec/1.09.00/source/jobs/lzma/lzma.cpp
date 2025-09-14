// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

#if defined EA_PLATFORM_PS3_SPU && !defined EA_ASSERT_ENABLED
#define EA_ASSERT_ENABLED
#endif

#include "EAAssert/eaassert.h"
#include "detail/decompressors.h"

#ifdef EA_PLATFORM_PS3_SPU
#include <edge/lzma/edgelzma_spu.h>
#else
EA_DISABLE_ALL_VC_WARNINGS()
#include "LzmaLib.h"
EA_RESTORE_ALL_VC_WARNINGS()
#endif

namespace rw {
namespace core {
namespace codec {
namespace detail {

    static inline void CreateProperties(unsigned char *properties)
    {
        properties[0] = LZMA_PROPERTIES_BYTE_0;
        properties[1] = properties[2] = properties[4] = 0;
        properties[3] = 1;
    }

#ifdef EA_PLATFORM_PS3_SPU
    void Decompress(void *outBuf, size_t outBufSize, const void *inBuf, size_t inputSize)
    {
        // The LZMA implementation in Sony's Edge library uses two discrete
        // 64kb buffers. To keep the same behavior we have upped the window
        // size to 64kb and enforce in the tool that the chunks are no larger
        // than 64kb.
        const size_t maxWindowSize = 1 << 16;
        EA_ASSERT(static_cast<char *>(outBuf) + maxWindowSize < static_cast<const char *>(inBuf));

        unsigned char properties[5];
        CreateProperties(properties);

        edgeLzmaInflateRawData(static_cast<unsigned char *>(outBuf), 
                outBufSize, 
                properties,
                sizeof properties,
                static_cast<const unsigned char *>(inBuf), 
                inputSize);
    }
#else
    void DecompressLzma(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize)
    {
        unsigned char *outBuf = reinterpret_cast<unsigned char *>(outputBuffer);
        size_t outBufSize = static_cast<size_t>(outputBufferSize);
        const unsigned char *inBuf = reinterpret_cast<const unsigned char *>(inputBuffer);
        size_t inputSize = static_cast<size_t>(inputBufferSize);

        unsigned char properties[5];
        CreateProperties(properties);

        int rv = LzmaUncompress(outBuf, &outBufSize, inBuf, &inputSize, properties, sizeof properties);
        EA_ASSERT(rv == SZ_OK);
        (void)rv;
    }
#endif

}}}}
