// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2 cache management functions.

#ifndef RW_CACHEMANAGER_H

#if defined(RWCROSS)

#include "../../../../win/rw/core/cachemanager/cachemanager.h"

#else

#define RW_CACHEMANAGER_H
#include <rw/core/base/ostypes.h>


namespace rw
{
namespace cachemanager
{

static const uint32_t DEFAULTBATCHSIZE = 1024;

static const size_t LINESIZE = 128;

inline void HintRead(const void *address, size_t size)
{
    for (int i = 0; i < static_cast<int>(size); i += 128)
    {
        __dcbt(i, address);
    }
}

inline void HintRewrite(void *address, size_t size)
{
    for (int i = 0; i < static_cast<int>(size); i += 128)
    {
        __dcbt((int)i, address);
    }
}

inline void HintWrite(void *address, size_t size)
{
    for (int i = 0; i < static_cast<int>(size); i += 128)
    {
        __dcbz128((int)i, address);
    }
}

inline void HintWritePrefetch(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}


} // namespace cachemanager
} // namespace rw

#endif // defined(RWCROSS)
#endif //RW_CACHEMANAGER_H
