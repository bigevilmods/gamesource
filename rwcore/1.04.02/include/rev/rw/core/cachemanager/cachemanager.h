// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Wii cache management functions.

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

inline void HintRead(const void *RWUNUSED(address), size_t RWUNUSED(size))
{
}

inline void HintRewrite(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}

inline void HintWrite(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}

inline void HintWritePrefetch(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}

} // namespace cachemanager
} // namespace rw

#endif // defined(RWCROSS)
#endif //RW_CACHEMANAGER_H
