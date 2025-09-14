// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK4_OPERATION_H
#define RW_MATH_VPU_MASK4_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask4.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Mask4 Not(Mask4::InParam a);
RW_MATH_FORCE_INLINE Mask4 And(Mask4::InParam a, Mask4::InParam b);
RW_MATH_FORCE_INLINE Mask4 Or(Mask4::InParam a, Mask4::InParam b);
RW_MATH_FORCE_INLINE Mask4 Xor(Mask4::InParam a, Mask4::InParam b);
RW_MATH_FORCE_INLINE Mask4 CompEqual(Mask4::InParam a, Mask4::InParam b);
RW_MATH_FORCE_INLINE Mask4 CompNotEqual(Mask4::InParam a, Mask4::InParam b);
RW_MATH_FORCE_INLINE MaskScalar CompAnyTrue(Mask4::InParam a);
RW_MATH_FORCE_INLINE MaskScalar CompAllTrue(Mask4::InParam a);
RW_MATH_FORCE_INLINE Mask4 Select(MaskScalar::InParam mask, Mask4::InParam trueValue, Mask4::InParam falseValue );
RW_MATH_FORCE_INLINE Mask4 Select(Mask4::InParam mask, Mask4::InParam trueValue, Mask4::InParam falseValue );
RW_MATH_FORCE_INLINE Mask4 RotateLeft(Mask4::InParam a);
RW_MATH_FORCE_INLINE Mask4 RotateRight(Mask4::InParam a);

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/mask4_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASK4_OPERATION_H
