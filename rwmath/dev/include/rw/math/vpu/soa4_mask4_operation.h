// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_MASK4_OPERATION_H
#define RW_MATH_VPU_SOA4_MASK4_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask4.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Mask4 Not(Soa4Mask4::InParam a);
RW_MATH_FORCE_INLINE Soa4Mask4 And(Soa4Mask4::InParam a, Soa4Mask4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4 Or(Soa4Mask4::InParam a, Soa4Mask4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4 Xor(Soa4Mask4::InParam a, Soa4Mask4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4 CompEqual(Soa4Mask4::InParam a, Soa4Mask4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4 CompNotEqual(Soa4Mask4::InParam a, Soa4Mask4::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompAnyTrue(Soa4Mask4::InParam a);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompAllTrue(Soa4Mask4::InParam a);
RW_MATH_FORCE_INLINE Soa4Mask4 Select(Soa4MaskScalar::InParam mask, Soa4Mask4::InParam trueValue, Soa4Mask4::InParam falseValue );
RW_MATH_FORCE_INLINE Soa4Mask4 Select(Soa4Mask4::InParam mask, Soa4Mask4::InParam trueValue, Soa4Mask4::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_mask4_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASK4_OPERATION_H
