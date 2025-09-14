// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Kettle cache management functions.

#ifndef RW_CACHEMANAGER_H
#define RW_CACHEMANAGER_H
#include <rw/core/base/ostypes.h>

namespace rw
{
namespace cachemanager
{

static const size_t DEFAULTBATCHSIZE = 0x80000000;

static const size_t LINESIZE = 1;

/**
Specifies that cache lines will be read but not written. A platform specific implementation may fetch the
lines or do nothing depending on the hardware support.
\param address The base address of the lines to fetch. This must be a multiple of the cache line size.
\param size The size to fetch. This must be a multiple of the cache line size.
*/
inline void HintRead(const void *RWUNUSED(address), size_t RWUNUSED(size))
{
}


/**
Specifies that cache lines will be read and then written. A platform specific implementation may fetch the
lines or do nothing depending on the hardware support.
\param address The base address of the lines to fetch. This must be a multiple of the cache line size.
\param size The size to fetch. This must be a multiple of the cache line size.
*/
inline void HintRewrite(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}


/**
Specifies that cache lines will be overwritten. A platform specific implementation may zero the lines or do
nothing depending on the hardware support.
\param address The base address of the lines to overwrite. This must be a multiple of the cache line size.
\param size The size to overwrite. This must be a multiple of the cache line size.
*/
inline void HintWrite(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}


/**
Specifies that cache lines will be overwritten. A platform specific implementation may fetch the lines or do
nothing depending on the hardware support.
\param address The base address of the lines to overwrite. This must be a multiple of the cache line size.
\param size The size to overwrite. This must be a multiple of the cache line size.
*/
inline void HintWritePrefetch(void *RWUNUSED(address), size_t RWUNUSED(size))
{
}


} // namespace cachemanager
} // namespace rw

#endif //RW_CACHEMANAGER_H
