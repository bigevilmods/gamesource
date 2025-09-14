// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VECUTIL_OPERATION_INLINE_H
#define RW_MATH_VECUTIL_OPERATION_INLINE_H

#include "rw/math/vpu/detail/ps3/ppu/vmx_macros.h"

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
        const VectorIntrinsic eight = RWMATH_VPU_DETAIL_VecSplatS32(8);
        const VectorIntrinsic sixteen = RWMATH_VPU_DETAIL_VecAdduwm(eight, eight);

        return RWMATH_VPU_DETAIL_VecPerm( eight, RWMATH_VPU_DETAIL_VecLvsl(0, value), sixteen);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UShort(uint32_t value)
    {
        // This code should actually be quite efficient since these constants
        // can be easily hoisted or scheduled ahead of the __lvsl
        const VectorIntrinsic eight = RWMATH_VPU_DETAIL_VecSpltish(8);
        const VectorIntrinsic sixteen = RWMATH_VPU_DETAIL_VecAdduhm(eight, eight);

        return RWMATH_VPU_DETAIL_VecPerm( eight, RWMATH_VPU_DETAIL_VecLvsl(0, value), sixteen);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UChar(uint32_t value)
    {
        return RWMATH_VPU_DETAIL_VecSplatByte( RWMATH_VPU_DETAIL_VecLvsl(0, value), 0);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_VECUTIL_OPERATION_INLINE_H
