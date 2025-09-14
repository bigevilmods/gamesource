// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_MASK3_OPERATION_H
#define RW_MATH_VPU_SOA4_MASK3_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask3.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Mask3 Not(Soa4Mask3::InParam a);
RW_MATH_FORCE_INLINE Soa4Mask3 And(Soa4Mask3::InParam a, Soa4Mask3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3 Or(Soa4Mask3::InParam a, Soa4Mask3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3 Xor(Soa4Mask3::InParam a, Soa4Mask3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3 CompEqual(Soa4Mask3::InParam a, Soa4Mask3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3 CompNotEqual(Soa4Mask3::InParam a, Soa4Mask3::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompAnyTrue(Soa4Mask3::InParam a);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompAllTrue(Soa4Mask3::InParam a);
RW_MATH_FORCE_INLINE Soa4Mask3 Select(Soa4MaskScalar::InParam mask, Soa4Mask3::InParam trueValue, Soa4Mask3::InParam falseValue );
RW_MATH_FORCE_INLINE Soa4Mask3 Select(Soa4Mask3::InParam mask, Soa4Mask3::InParam trueValue, Soa4Mask3::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_mask3_operation_inline.h"
#endif

#endif // RW_MATH_VPU_MASK3_OPERATION_H
