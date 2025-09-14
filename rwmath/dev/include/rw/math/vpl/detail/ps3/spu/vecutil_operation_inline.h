// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VECUTIL_OPERATION_INLINE_H
#define RW_MATH_VECUTIL_OPERATION_INLINE_H

#include "rw/math/detail/ps3/spu/spu_macros.h"

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UInt(uint32_t value)
    {
        vector unsigned int permConstant = (vector unsigned int)(3);
        vector unsigned int vecValue = spu_promote(value, 0);
        return RWMATH_VPU_DETAIL_VecPerm( vecValue, vecValue, permConstant);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UShort(uint32_t value)
    {
        vector unsigned short permConstant = (vector unsigned short)(3);
        vector unsigned short vecValue = (vector unsigned short)spu_promote(value, 0);
        return RWMATH_VPU_DETAIL_VecPerm( vecValue, vecValue, permConstant);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UChar(uint32_t value)
    {
        vector unsigned char permConstant = (vector unsigned char)(3);
        vector unsigned char vecValue = (vector unsigned char)spu_promote(value, 0);
        return RWMATH_VPU_DETAIL_VecPerm( vecValue, vecValue, permConstant);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_VECUTIL_OPERATION_INLINE_H
