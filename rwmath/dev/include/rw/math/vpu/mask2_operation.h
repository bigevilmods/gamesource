// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK2_OPERATION_H
#define RW_MATH_VPU_MASK2_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask2.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Mask2 Not(Mask2::InParam a);
RW_MATH_FORCE_INLINE Mask2 And(Mask2::InParam a, Mask2::InParam b);
RW_MATH_FORCE_INLINE Mask2 Or(Mask2::InParam a, Mask2::InParam b);
RW_MATH_FORCE_INLINE Mask2 Xor(Mask2::InParam a, Mask2::InParam b);
RW_MATH_FORCE_INLINE Mask2 CompEqual(Mask2::InParam a, Mask2::InParam b);
RW_MATH_FORCE_INLINE Mask2 CompNotEqual(Mask2::InParam a, Mask2::InParam b);
RW_MATH_FORCE_INLINE MaskScalar CompAnyTrue(Mask2::InParam a);
RW_MATH_FORCE_INLINE MaskScalar CompAllTrue(Mask2::InParam a);
RW_MATH_FORCE_INLINE Mask2 Select(MaskScalar::InParam mask, Mask2::InParam trueValue, Mask2::InParam falseValue );
RW_MATH_FORCE_INLINE Mask2 Select(Mask2::InParam mask, Mask2::InParam trueValue, Mask2::InParam falseValue );
RW_MATH_FORCE_INLINE Mask2 RotateLeft(Mask2::InParam a);
RW_MATH_FORCE_INLINE Mask2 RotateRight(Mask2::InParam a);

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/mask2_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASK2_OPERATION_H
