// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_MASK2_OPERATION_H
#define RW_MATH_VPU_SOA4_MASK2_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask2.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Mask2 Not(Soa4Mask2::InParam a);
RW_MATH_FORCE_INLINE Soa4Mask2 And(Soa4Mask2::InParam a, Soa4Mask2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2 Or(Soa4Mask2::InParam a, Soa4Mask2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2 Xor(Soa4Mask2::InParam a, Soa4Mask2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2 CompEqual(Soa4Mask2::InParam a, Soa4Mask2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2 CompNotEqual(Soa4Mask2::InParam a, Soa4Mask2::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompAnyTrue(Soa4Mask2::InParam a);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompAllTrue(Soa4Mask2::InParam a);
RW_MATH_FORCE_INLINE Soa4Mask2 Select(Soa4MaskScalar::InParam mask, Soa4Mask2::InParam trueValue, Soa4Mask2::InParam falseValue );
RW_MATH_FORCE_INLINE Soa4Mask2 Select(Soa4Mask2::InParam mask, Soa4Mask2::InParam trueValue, Soa4Mask2::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_mask2_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASK2_OPERATION_H
