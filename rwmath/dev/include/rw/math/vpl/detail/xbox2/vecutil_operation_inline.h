// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VECUTIL_OPERATION_INLINE_H
#define RW_MATH_VECUTIL_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UInt(uint32_t value)
    {
        // This code should actually be quite efficient since these constants
        // can be easily hoisted or scheduled ahead of the __lvsl
        const VectorIntrinsic eight = __vspltisw(8);
        const VectorIntrinsic sixteen = __vadduwm(eight, eight);

        return __vperm( eight, __lvsl(0, static_cast<int>(value)), sixteen);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UShort(uint32_t value)
    {
        // This code should actually be quite efficient since these constants
        // can be easily hoisted or scheduled ahead of the __lvsl
        const VectorIntrinsic eight = __vspltish(8);
        const VectorIntrinsic sixteen = __vadduhm(eight, eight);

        return __vperm( eight, __lvsl(0, static_cast<int>(value)), sixteen);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UChar(uint32_t value)
    {
        return __vspltb( __lvsl(0, static_cast<int>(value)), 0);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_VECUTIL_OPERATION_INLINE_H
