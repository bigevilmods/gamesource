// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"

#include "detail/decompressors.h"


#ifdef EA_PLATFORM_PS3_SPU
#include <edge/zlib/edgezlib_spu.h>
#else
#include "zlib/zlib.h"
#endif

namespace rw {
namespace core {
namespace codec {
namespace detail {

#ifdef EA_PLATFORM_PS3_SPU
    void Decompress(void *outBuf, size_t outBufSize, const void *inBuf, size_t inputSize)
    {
        // The Zlib driver doesn't need to assert that a certain overlap for the window is available as
        // edge reserves a section of local store for the window.
        edgeZlibInflateRawData(static_cast<unsigned char *>(outBuf), outBufSize, static_cast<const unsigned char *>(inBuf), inputSize);
    }
#else
    void DecompressZlib(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize)
    {
        void *outBuf = reinterpret_cast<void *>(outputBuffer);
        size_t outBufSize = static_cast<size_t>(outputBufferSize);
        const void *inBuf = reinterpret_cast<const void *>(inputBuffer);
        size_t inputSize = static_cast<size_t>(inputBufferSize);

        z_stream_s stream = {};
        inflateInit2(&stream, -MAX_WBITS);
        stream.next_in = static_cast<Bytef *>(const_cast<void *>(inBuf));
        stream.next_out = static_cast<Bytef *>(outBuf);
        stream.avail_in = static_cast<uInt>(inputSize);
        stream.avail_out = static_cast<uInt>(outBufSize);

        int rv = inflate(&stream, Z_FINISH);

        if (rv == Z_BUF_ERROR && !stream.avail_in )
        {
            stream.avail_in = 1;
            uInt availOut = stream.avail_out;
            stream.avail_out = 0;

            rv = inflate(&stream, Z_FINISH);
            stream.avail_out = availOut;
        }

        EA_ASSERT(rv == Z_OK || rv == Z_STREAM_END);

        inflateEnd(&stream);
    }
#endif

}}}}
