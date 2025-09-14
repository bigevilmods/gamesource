// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS3 PPU cache management functions.

#ifndef RW_CACHEMANAGER_H

#if defined(RWCROSS)

#include "../../../../win/rw/core/cachemanager/cachemanager.h"

#else

#define RW_CACHEMANAGER_H

#if       defined(__SPU__)
#include "rw/core/cachemanager/spu_cachemanager.h"
#else

#include <ppu_intrinsics.h>

#include <rw/core/base/ostypes.h>


namespace rw
{
    /**
    Cache management functions
    */
namespace cachemanager
{

static const uint32_t DEFAULTBATCHSIZE = 1024;

static const size_t LINESIZE = 128;

inline void HintRead(const void *address, size_t size)
{
    for (uint32_t i = 0; i < size; i += 128)
    {
        __dcbt(static_cast<const uint8_t *>(address) + i);
    }
}

inline void HintRewrite(void *address, size_t size)
{
    for (uint32_t i = 0; i < size; i += 128)
    {
        __dcbt(static_cast<uint8_t *>(address) + i);
    }
}

inline void HintWrite(void *address, size_t size)
{
    for (uint32_t i = 0; i < size; i += 128)
    {
        __dcbz(static_cast<uint8_t *>(address) + i);
    }
}

inline void HintWritePrefetch(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}


} // namespace cachemanager
} // namespace rw

#endif // defined(__SPU__)
#endif // defined(RWCROSS)
#endif //RW_CACHEMANAGER_H
