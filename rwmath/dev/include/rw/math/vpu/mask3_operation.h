// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK3_OPERATION_H
#define RW_MATH_VPU_MASK3_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask3.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Mask3 Not(Mask3::InParam a);
RW_MATH_FORCE_INLINE Mask3 And(Mask3::InParam a, Mask3::InParam b);
RW_MATH_FORCE_INLINE Mask3 Or(Mask3::InParam a, Mask3::InParam b);
RW_MATH_FORCE_INLINE Mask3 Xor(Mask3::InParam a, Mask3::InParam b);
RW_MATH_FORCE_INLINE Mask3 CompEqual(Mask3::InParam a, Mask3::InParam b);
RW_MATH_FORCE_INLINE Mask3 CompNotEqual(Mask3::InParam a, Mask3::InParam b);
RW_MATH_FORCE_INLINE MaskScalar CompAnyTrue(Mask3::InParam a);
RW_MATH_FORCE_INLINE MaskScalar CompAllTrue(Mask3::InParam a);
RW_MATH_FORCE_INLINE Mask3 Select(MaskScalar::InParam mask, Mask3::InParam trueValue, Mask3::InParam falseValue );
RW_MATH_FORCE_INLINE Mask3 Select(Mask3::InParam mask, Mask3::InParam trueValue, Mask3::InParam falseValue );
RW_MATH_FORCE_INLINE Mask3 RotateLeft(Mask3::InParam a);
RW_MATH_FORCE_INLINE Mask3 RotateRight(Mask3::InParam a);

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/mask3_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASK3_OPERATION_H
