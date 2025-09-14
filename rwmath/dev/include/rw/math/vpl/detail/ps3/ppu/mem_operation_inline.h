// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_MEM_OPERATION_INLINE_H
#define RW_MATH_MEM_OPERATION_INLINE_H

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_DOC The cacheline size prefetched is 128 bytes.
    //
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT dcbt
    RW_MATH_FORCE_INLINE VPL_API void MemCachePrefetch( const void *base, int offset )
    {
#ifdef __SNC__
        __builtin_dcbt( base, offset );
#else
        asm volatile ( "dcbt %[_off], %[_base]" : : [_off] "b" (offset), [_base] "r" (base) );
#endif
    }

    // VPL_PLATFORM_DOC The cacheline size zeroed is 128 bytes.
    //
    // The PS3 uses inline assembly to call the \c dcbz instruction.  The assembly code
    // intentionally does \em not list "memory" as being clobbered.  That means the compiler
    // will not reload data following the \c dcbz instruction.  Not reloading variables
    // should be much faster in most cases, but it does mean care needs to be taken when
    // writing code that uses this function.
    //
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT dcbz
    RW_MATH_FORCE_INLINE VPL_API void MemCacheZero( void *base, int offset )
    {
#ifdef __SNC__
        __builtin_dcbz( base, offset );
#else
        asm volatile ( "dcbz %[_off], %[_base]" : : [_off] "b" (offset), [_base] "r" (base) );
#endif
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif

