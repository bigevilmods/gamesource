// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

#ifdef EA_PLATFORM_PS3_SPU

#if !defined(EA_ASSERT_ENABLED)
#define EA_ASSERT_ENABLED 1
#endif

#include "EAAssert/eaassert.h"
#include "job_manager/spu_heap.h"
#include "job_util/dma.h"
#include "job_util/print.h"

#include "rw/core/codec/chunkcompressiontypes.h"
#include "detail/jobdatum.h"
#include "detail/decompressors.h"

using namespace EA::Jobs;
using namespace rw::core::codec;
using namespace rw::core::codec::detail;

#define ALIGN_TO(val, align) (((val) + (align - 1)) & ~(align - 1))

#define MAX(x, y) ((x)>(y)?(x):(y))

static void DmaPutChecked(void *outputBuffer, void const *inputBuffer, size_t size)
{
    uintptr_t const outputBufferEA = reinterpret_cast<uintptr_t>(outputBuffer);
    EA_ASSERT(outputBufferEA % 16 == 0);

    uintptr_t const alignmentMask = ~127;
    uintptr_t const outputBufferEndEA = outputBufferEA + size;
    uintptr_t const outputBufferEndAligned = outputBufferEndEA & alignmentMask;

    intptr_t const endOffset = outputBufferEndAligned - outputBufferEA;
    // The DMA amount has to be a multiple of 16 bytes.
    intptr_t const bytesToDma = MAX(endOffset, 0) & ~15;
    intptr_t const remainderBytes = size - bytesToDma;

    if (bytesToDma)
        DmaPut(outputBuffer, inputBuffer, static_cast<size_t>(bytesToDma));

    if (remainderBytes > 0)
    {
        char const *inputBytes = static_cast<char const *>(inputBuffer) + bytesToDma;
        intptr_t const offset = endOffset < 0 ? -endOffset : 0;

        // If the end offset above is negative this means that the aligned end of our data
        // block to transfer comes before the beginning because the block isn't 128-byte
        // aligned and the size is less than 128 bytes. For example, with a size of 48 bytes:
        // [                ||||||||||||    ]
        // 0                               128
        // ^                ^ outputBufferEA
        // outputBufferEndAligned
        // So when we copy our data out of the local store into the local cache-line copy 
        // we need to offset by that amount.

        do
        {
            char localData[128] __attribute__((aligned(128))) = {};
            mfc_getllar(localData, outputBufferEndAligned, 0, 0);
            mfc_read_atomic_status();
            spu_dsync();

            for (intptr_t i = 0; i < remainderBytes; ++i)
                localData[i + offset] = inputBytes[i];

            mfc_putllc(localData, outputBufferEndAligned, 0, 0);
        } while(__builtin_expect(mfc_read_atomic_status(), 0));
    }
}


int main(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize)
{
    char * const heapBegin = reinterpret_cast<char *>(
            ALIGN_TO(reinterpret_cast<uintptr_t>(SpuHeapBegin()), 128));
    char * const heapEnd = reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(SpuHeapEnd(3*1024)) & ~127);

    const size_t outputBufferSizeAligned = ALIGN_TO(outputBufferSize, 128);
    const size_t inputBufferSizeAligned = ALIGN_TO(inputBufferSize, 128);
    EA_ASSERT(outputBufferSizeAligned % 128 == 0);

    char * const localOutputBuffer = heapBegin;
    char * const localInputBuffer = heapEnd - inputBufferSizeAligned - 128;

    EA_ASSERT(reinterpret_cast<uintptr_t>(localInputBuffer) % 128 == reinterpret_cast<uintptr_t>(localOutputBuffer) % 128);

    DmaGet(localInputBuffer, reinterpret_cast<const void *>(inputBuffer), inputBufferSizeAligned);

    Decompress(localOutputBuffer, outputBufferSize, localInputBuffer, inputBufferSize);
    DmaPutChecked(reinterpret_cast<void *>(outputBuffer), localOutputBuffer, outputBufferSize);

    EA_ASSERT(SpuHeapVerifyStackSentinel(3*1024));
}

#endif

