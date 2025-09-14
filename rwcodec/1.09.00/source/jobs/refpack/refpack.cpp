// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

#if defined(EA_PLATFORM_PS3_SPU) && !defined(EA_ASSERT_ENABLED)
#   define EA_ASSERT_ENABLED
#endif

#include "EAAssert/eaassert.h"

#include "detail/decompressors.h"
#include "jobs/refpack/refpack.h"

namespace rw { 
namespace core { 
namespace codec { 
namespace detail {

#ifdef EA_PLATFORM_PS3_SPU
    void Decompress(void *outputBuffer, size_t, void const *inputBuffer, size_t)
    {
        const size_t maxWindowSize = 1 << 15;
        EA_ASSERT(static_cast<char *>(outputBuffer) + maxWindowSize < static_cast<const char *>(inputBuffer));

        Refpack::Inflate(outputBuffer, inputBuffer);
    }
#else
    void DecompressRefpack(uintptr_t outputBuffer, uintptr_t, uintptr_t inputBuffer, uintptr_t)
    {
        Refpack::Inflate(reinterpret_cast<void *>(outputBuffer), reinterpret_cast<const void *>(inputBuffer));
    }
#endif

}}}}

