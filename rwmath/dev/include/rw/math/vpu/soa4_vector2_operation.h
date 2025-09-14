// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_VECTOR2_OPERATION_H
#define RW_MATH_VPU_SOA4_VECTOR2_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector2.h"
#include "rw/math/vpu/soa4_vector2.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/soa4_mask2.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2(Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2(Vector2::InParam v0, Vector2::InParam v1, Vector2::InParam v2, Vector2::InParam v3);
RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2s(Vector2::InParam v0, Vector2::InParam v1, Vector2::InParam v2, Vector2::InParam v3);
RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2(const Vector2* v);

RW_MATH_FORCE_INLINE void Vector2FromSoa4Vector2(Soa4Vector2::InParam, Vector2& v0, Vector2& v1, Vector2& v2, Vector2& v3);
RW_MATH_FORCE_INLINE void Vector2FromSoa4Vector2(Soa4Vector2::InParam, Vector2* v);

RW_MATH_FORCE_INLINE Soa4Vector2& operator *= (Soa4Vector2& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector2& operator /= (Soa4Vector2& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector2& operator += (Soa4Vector2& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector2& operator -= (Soa4Vector2& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector2  operator *  (Soa4Float::InParam s, Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2  operator *  (Soa4Vector2::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2  operator *  (Soa4Vector2::InParam v, Soa4Vector2::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2  operator /  (Soa4Vector2::InParam v, Soa4Vector2::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2  operator /  (Soa4Vector2::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2  operator +  (Soa4Vector2::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Vector2::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2  operator +  (Soa4Float::InParam s, Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Float::InParam s, Soa4Vector2::InParam v);

RW_MATH_FORCE_INLINE Soa4Vector2& operator *= (Soa4Vector2& v, Soa4Vector2::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2& operator /= (Soa4Vector2& v, Soa4Vector2::InParam s);

RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2& operator += (Soa4Vector2& v, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2& operator -= (Soa4Vector2& v, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2  operator +  (Soa4Vector2::InParam a, Soa4Vector2::InParam b);

RW_MATH_FORCE_INLINE Soa4Vector2 Negate(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2 Add(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2 Subtract(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Float Dot(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Float Cross(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2 MultAdd(Soa4Vector2::InParam a, Soa4Vector2::InParam b, Soa4Vector2::InParam c);

RW_MATH_FORCE_INLINE Soa4Vector2 Mult(Soa4Vector2::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2 Mult(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2 Divide(Soa4Vector2::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector2 Divide(Soa4Vector2::InParam a, Soa4Vector2::InParam b);

RW_MATH_FORCE_INLINE Soa4Vector2 Reciprocal(Soa4Vector2::InParam a);
RW_MATH_FORCE_INLINE Soa4Float Magnitude(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeFast(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeEstimate(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeSquared(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeRecip(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2 NormalizeFast(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2 NormalizeEstimate(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector2 Normalize(Soa4Vector2::InParam v);
RW_MATH_FORCE_INLINE Soa4Float NormalizeReturnMagnitude(Soa4Vector2::InParam v, Soa4Vector2& result);
RW_MATH_FORCE_INLINE Soa4Float CheckedNormalizeReturnMagnitude(Soa4Vector2::InParam v, Soa4Vector2& result);
RW_MATH_FORCE_INLINE Soa4Vector2 Lerp(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector2 SLerp(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector2 NLerpEstimate(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector2 NLerpFast(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector2 NLerp(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector2 Min(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector2 Max(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector2 Clamp(Soa4Vector2::InParam vecToClamp, Soa4Vector2::InParam minVecValue, Soa4Vector2::InParam maxVecValue);
RW_MATH_FORCE_INLINE Soa4Vector2 ClampToNegPosRange(Soa4Vector2::InParam vecToClamp, Soa4Vector2::InParam range);
RW_MATH_FORCE_INLINE Soa4Vector2 Abs(Soa4Vector2::InParam v0);
RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectors(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1);
RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectorsFast(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1);
RW_MATH_FORCE_INLINE Soa4Float CosAngleBetweenNormalizedVectors(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1);

RW_MATH_FORCE_INLINE Soa4Mask2   CompLessThan(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2   CompGreaterThan(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2   CompLessEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2   CompGreaterEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2   CompEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask2   CompNotEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector2 Select(Soa4MaskScalar::InParam mask, Soa4Vector2::InParam trueValue, Soa4Vector2::InParam falseValue );
RW_MATH_FORCE_INLINE Soa4Vector2 Select(Soa4Mask2::InParam mask, Soa4Vector2::InParam trueValue, Soa4Vector2::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_vector2_operation_inline.h"
#endif

#endif
