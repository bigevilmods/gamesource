// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASKSCALAR_OPERATION_H
#define RW_MATH_VPU_MASKSCALAR_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE MaskScalar Not(MaskScalar::InParam a);
RW_MATH_FORCE_INLINE MaskScalar And(MaskScalar::InParam a, MaskScalar::InParam b);
RW_MATH_FORCE_INLINE MaskScalar Or(MaskScalar::InParam a, MaskScalar::InParam b);
RW_MATH_FORCE_INLINE MaskScalar Xor(MaskScalar::InParam a, MaskScalar::InParam b);
RW_MATH_FORCE_INLINE MaskScalar CompEqual(MaskScalar::InParam a, MaskScalar::InParam b);
RW_MATH_FORCE_INLINE MaskScalar CompNotEqual(MaskScalar::InParam a, MaskScalar::InParam b);
RW_MATH_FORCE_INLINE MaskScalar Select(MaskScalar::InParam mask, MaskScalar::InParam trueValue, MaskScalar::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/maskscalar_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASKSCALAR_OPERATION_H
