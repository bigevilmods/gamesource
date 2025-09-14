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

    // VPL_PLATFORM_DOC The cacheline size prefetched is 128 bytes.
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __dcbt / dcbt
    RW_MATH_FORCE_INLINE VPL_API void MemCachePrefetch( const void *base, int offset )
    {
        __dcbt( offset, base );
    }

    // VPL_PLATFORM_DOC The cacheline size zeroed is 128 bytes.
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __dcbz128 / dcbz128
    RW_MATH_FORCE_INLINE VPL_API void MemCacheZero( void *base, int offset )
    {
        __dcbz128( offset, base );
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif

