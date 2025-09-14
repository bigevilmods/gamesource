// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_MEM_OPERATION_H
#define RW_MATH_MEM_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpl/detail/vplbase.h"

namespace rw
{
namespace math
{
namespace vpl
{

#if VPL_SUPPORTED_PLATFORM( VPL_SSE | VPL_PPU | VPL_XENON | VPL_BROADWAY | VPL_SSE2 )
    /*! \brief Prefetches a cache line of memory into data cache.
     *
     *  Prefetching data before it is needed, helps prevent loads from stalling
     *  the processor. 
     *
     *  Important: The size of a cache line varies across architectures.  This
     *  function has been added so a single function may be used to prefetch
     *  data across platforms.  However, to be effective prefetching code will
     *  need to be customized for each platform to take into account cache line
     *  size, and other platform specific differences.
     *
     *  Simulated versions of this function do not perform any operation.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void MemCachePrefetch( const void *base, int offset );

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_PPU | VPL_XENON | VPL_BROADWAY )
    /*! \brief Zeros a cache line of memory.
     *
     *  Normally writing to memory, that is not currently cached, requires a
     *  load into memory followed by a modification of the cache line.  That
     *  is because most memory architectures only perform transfers of entire
     *  cache lines of memory (assuming a cacheable memory access).  The \c MemCacheZero
     *  function zeros an entire cache line without accessing main memory.  So,
     *  if data is being streamed out such that it fills entire cache lines, this
     *  function can be used to eliminate the loading of the cache line from
     *  main memory.
     *
     *  Important: The size of a cache line varies across architectures.
     *
     *  Simulated versions of this function do not perform any operation.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void MemCacheZero( void *base, int offset );

#endif

} // namespace vpl
} // namespace math
} // namespace rw

#include "rw/math/vpl/detail/mem_operation_inline.h"

#endif // RW_MATH_MEM_OPERATION_H

