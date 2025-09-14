// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_MEM_OPERATION_INLINE_H
#define RW_MATH_MEM_OPERATION_INLINE_H

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_DOC The cacheline size prefetched is normally 32 bytes.  Although, it is theoretically possible to change the cacheline size on Broadway to other sizes.
    //
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT dcbt
    RW_MATH_FORCE_INLINE VPL_API void MemCachePrefetch( const void *base, int offset )
    {
        register const void * rBase = base;
        register int rOffset = offset;
        asm { dcbt rOffset, rBase };
    }

    // VPL_PLATFORM_DOC The cacheline size zeroed is normally 32 bytes.  Although, it is theoretically possible to change the cacheline size on Broadway to other sizes.
    //
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT dcbz
    RW_MATH_FORCE_INLINE VPL_API void MemCacheZero( void *base, int offset )
    {
        register void * rBase = base;
        register int rOffset = offset;
        asm { dcbz rOffset, rBase };
    }


} // namespace vpl
} // namespace math
} // namespace rw

#endif

