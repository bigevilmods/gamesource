// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_MEM_OPERATION_INLINE_H
#define RW_MATH_MEM_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_DOC The cache line size for x86 processors is at least
    // 32bytes.  However, the actual size depends on the specific architecture.  For
    // example Pentium 4's use a 64byte cache line.
    //
    // This function gives passes a hint of \c _MM_HINT_T0 to \c _mm_prefetch.  This hint
    // indicates that the processor should prefetch data into L1 cache, and place the data
    // in L2 if it becomes evicted from L1.  Other hints are available.  To make use
    // of these hints, use the \c _mm_prefetch intrinsic directly with the desired
    // hint.
    //
    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT _mm_prefetch(ptr, _MM_HINT_T0)
    RW_MATH_FORCE_INLINE VPL_API void MemCachePrefetch( const void *base, int offset )
    {
        const char * ptr = ((const char*)base) + offset;
        _mm_prefetch(ptr, _MM_HINT_T0);
    }


} // namespace vpl
} // namespace math
} // namespace rw

#ifdef RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#include "rw/math/vpl/detail/simulated/mem_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{
    using rw::math::vpl::detail::simulated_memops::MemCacheZero;
} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS
#endif


