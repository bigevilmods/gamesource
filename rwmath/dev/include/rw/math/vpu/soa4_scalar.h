// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_SCALAR_H
#define RW_MATH_SOA4_SCALAR_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/soa4_soa4base.h"
#include "rw/math/vpu/soa4_float.h"
#include "rw/math/vpu/soa4_maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

    RW_MATH_FORCE_INLINE Soa4Float Floor(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Ceil(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Round(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Modulo(Soa4Float::InParam a, Soa4Float::InParam modulus);

    RW_MATH_FORCE_INLINE Soa4Float Abs(Soa4Float::InParam a);
    
    RW_MATH_FORCE_INLINE Soa4Float Min(Soa4Float::InParam a, Soa4Float::InParam b);

    RW_MATH_FORCE_INLINE Soa4Float Min(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam c);

    RW_MATH_FORCE_INLINE Soa4Float Max(Soa4Float::InParam a, Soa4Float::InParam b);

    RW_MATH_FORCE_INLINE Soa4Float Max(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam c);

    RW_MATH_FORCE_INLINE Soa4Float Lerp(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam t);

    RW_MATH_FORCE_INLINE Soa4Float Reciprocal(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float ReciprocalFast(Soa4Float::InParam a);
    
    RW_MATH_FORCE_INLINE Soa4Float Sqrt(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float SqrtFast(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float SqrtEstimate(Soa4Float::InParam a);

    RW_MATH_FORCE_INLINE Soa4Float InvSqrt(Soa4Float::InParam a);    
    RW_MATH_FORCE_INLINE Soa4Float InvSqrtFast(Soa4Float::InParam a);
    
    RW_MATH_FORCE_INLINE Soa4Float Pow(Soa4Float::InParam base, Soa4Float::InParam exponent);    
    RW_MATH_FORCE_INLINE Soa4Float Clamp(Soa4Float::InParam value, Soa4Float::InParam min, Soa4Float::InParam max);

    RW_MATH_FORCE_INLINE Soa4Float Log(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Exp(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Sgn(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float SgnNonZero(Soa4Float::InParam a);

    RW_MATH_FORCE_INLINE Soa4Float MultAdd(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam c);

    RW_MATH_FORCE_INLINE Soa4MaskScalar CompLessThan(Soa4Float::InParam a, Soa4Float::InParam b);
    RW_MATH_FORCE_INLINE Soa4MaskScalar CompGreaterThan(Soa4Float::InParam a, Soa4Float::InParam b);
    RW_MATH_FORCE_INLINE Soa4MaskScalar CompLessEqual(Soa4Float::InParam a, Soa4Float::InParam b);
    RW_MATH_FORCE_INLINE Soa4MaskScalar CompGreaterEqual(Soa4Float::InParam a, Soa4Float::InParam b);
    RW_MATH_FORCE_INLINE Soa4MaskScalar CompEqual(Soa4Float::InParam a, Soa4Float::InParam b);
    RW_MATH_FORCE_INLINE Soa4MaskScalar CompNotEqual(Soa4Float::InParam a, Soa4Float::InParam b);
    RW_MATH_FORCE_INLINE Soa4Float   Select(Soa4MaskScalar::InParam mask, Soa4Float::InParam trueValue, Soa4Float::InParam falseValue );
} // namespace vpu
} // namespace math
} // namespace rw

#include "rw/math/vpu/detail/soa4_scalar_operation_inline.h"

#endif
