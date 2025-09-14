// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "detail/decompressors.h"

#ifndef EA_PLATFORM_PS3_SPU
#include "EAStdC/EAMemory.h"
#endif

namespace rw {
namespace core {
namespace codec {
namespace detail {

#ifdef EA_PLATFORM_PS3_SPU
    void Decompress(void *localOutputBuffer, size_t outputBufferSize, const void *localInputBuffer, size_t inputBufferSize)
    {
        const qword *R = static_cast<const qword *>(localInputBuffer);
        const qword *E = reinterpret_cast<const qword *>((reinterpret_cast<size_t>(localOutputBuffer) + outputBufferSize + 127) & ~127);

        for (qword *I = static_cast<qword *>(localOutputBuffer); I != E; I += 8, R += 8)
        {
            const qword q0 = *(R + 0);
            const qword q1 = *(R + 1);
            const qword q2 = *(R + 2);
            const qword q3 = *(R + 3);
            const qword q4 = *(R + 4);
            const qword q5 = *(R + 5);
            const qword q6 = *(R + 6);
            const qword q7 = *(R + 7);
            *(I + 0) = q0;
            *(I + 1) = q1;
            *(I + 2) = q2;
            *(I + 3) = q3;
            *(I + 4) = q4;
            *(I + 5) = q5;
            *(I + 6) = q6;
            *(I + 7) = q7;
        }
    }
#else
    void DecompressUncompressed(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize)
    {
        EA_ASSERT(outputBufferSize == inputBufferSize);
        (void)outputBufferSize;

        void *outBuf = reinterpret_cast<void *>(outputBuffer);
        const void *inBuf = reinterpret_cast<const void *>(inputBuffer);
        size_t inputSize = static_cast<size_t>(inputBufferSize);

        EA::StdC::Memcpy(outBuf, inBuf, inputSize);
    }
#endif

}}}}

