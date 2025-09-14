// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SIMULATED_MEM_OPERATION_INLINE_H
#define RW_MATH_SIMULATED_MEM_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{
namespace detail
{
namespace simulated_memops
{

    RW_MATH_FORCE_INLINE VPL_API void MemCachePrefetch( const void *, int )
    {
        // Do nothing
    }

    RW_MATH_FORCE_INLINE VPL_API void MemCacheZero( void *, int )
    {
        // Do nothing
    }

} // namespace simulated_memops
} // namespace detail
} // namespace vpl
} // namespace math
} // namespace rw

#endif
