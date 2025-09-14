// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

#ifndef EA_PLATFORM_PS3_SPU

#include "EAAssert/eaassert.h"
#include "detail/decompressors.h"
#include "lz4/lz4.h"

namespace rw {
namespace core {
namespace codec {
namespace detail {

    void DecompressLz4(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t /*inputBufferSize*/)
    {
        char *outBuf = reinterpret_cast<char *>(outputBuffer);
        int outBufSize = static_cast<int>(outputBufferSize);
        const char *inBuf = reinterpret_cast<const char *>(inputBuffer);

        int rv = LZ4_decompress_fast(inBuf, outBuf, outBufSize);
        EA_ASSERT(rv >= 0);
        EA_UNUSED(rv);
    }

}
}
}
}
#endif