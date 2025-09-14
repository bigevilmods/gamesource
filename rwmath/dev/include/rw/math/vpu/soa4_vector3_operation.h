// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_VECTOR3_OPERATION_H
#define RW_MATH_VPU_SOA4_VECTOR3_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/soa4_vector2.h"
#include "rw/math/vpu/soa4_vector3.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/soa4_mask3.h"

namespace rw
{
namespace math
{
namespace vpu
{
RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3(Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam v2, Vector3::InParam v3);
RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3s(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam v2, Vector3::InParam v3);
RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3(const Vector3* v);

RW_MATH_FORCE_INLINE void Vector3FromSoa4Vector3(Soa4Vector3::InParam, Vector3& v0, Vector3& v1, Vector3& v2, Vector3& v3);
RW_MATH_FORCE_INLINE void Vector3FromSoa4Vector3(Soa4Vector3::InParam, Vector3* v);

RW_MATH_FORCE_INLINE Soa4Vector3& operator *= (Soa4Vector3& v, Soa4Vector3::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3& operator /= (Soa4Vector3& v, Soa4Vector3::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3& operator *= (Soa4Vector3& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3& operator /= (Soa4Vector3& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3& operator += (Soa4Vector3& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3& operator -= (Soa4Vector3& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3  operator *  (Soa4Vector3::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector3  operator *  (Soa4Float::InParam s, Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3  operator *  (Soa4Vector3::InParam v, Soa4Vector3::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector3  operator /  (Soa4Vector3::InParam v, Soa4Vector3::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector3  operator /  (Soa4Vector3::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector3  operator +  (Soa4Vector3::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Vector3::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector3  operator +  (Soa4Float::InParam s, Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Float::InParam s, Soa4Vector3::InParam v);

RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3& operator += (Soa4Vector3& v, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3& operator -= (Soa4Vector3& v, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3  operator +  (Soa4Vector3::InParam a, Soa4Vector3::InParam b);

RW_MATH_FORCE_INLINE Soa4Vector3 Negate(Soa4Vector3::InParam v);

RW_MATH_FORCE_INLINE Soa4Vector3 Mult(Soa4Vector3::InParam v, Soa4Float::InParam s);

RW_MATH_FORCE_INLINE Soa4Vector3 Divide(Soa4Vector3::InParam v, Soa4Float::InParam s);

RW_MATH_FORCE_INLINE Soa4Vector3 Add(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3 Subtract(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Float Dot(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3 Cross(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3 Mult(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3 Divide(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3 Reciprocal(Soa4Vector3::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector3 MultAdd(Soa4Vector3::InParam a, Soa4Vector3::InParam b, Soa4Vector3::InParam c);
RW_MATH_FORCE_INLINE Soa4Float Magnitude(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeFast(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeEstimate(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeSquared(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeRecip(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3 NormalizeFast(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3 NormalizeEstimate(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector3 Normalize(Soa4Vector3::InParam v);
RW_MATH_FORCE_INLINE Soa4Float NormalizeReturnMagnitude(Soa4Vector3::InParam v, Soa4Vector3& result);
RW_MATH_FORCE_INLINE Soa4Float CheckedNormalizeReturnMagnitude(Soa4Vector3::InParam v, Soa4Vector3& result);
RW_MATH_FORCE_INLINE Soa4Float CosAngleBetweenNormalizedVectors(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector3 Lerp(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector3 SLerp(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector3 NLerpEstimate(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector3 NLerpFast(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector3 NLerp(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector3 Min(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector3 Max(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector3 Clamp(Soa4Vector3::InParam vecToClamp, Soa4Vector3::InParam minVecValue, Soa4Vector3::InParam maxVecValue);
RW_MATH_FORCE_INLINE Soa4Vector3 ClampToNegPosRange(Soa4Vector3::InParam vecToClamp, Soa4Vector3::InParam range);
RW_MATH_FORCE_INLINE Soa4Vector3 Abs(Soa4Vector3::InParam v0);
RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectors(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1);
RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectors(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Vector3::InParam windAxis);

RW_MATH_FORCE_INLINE Soa4Mask3   CompLessThan(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3   CompGreaterThan(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3   CompLessEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3   CompGreaterEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3   CompEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask3   CompNotEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector3 Select(Soa4MaskScalar::InParam mask, Soa4Vector3::InParam trueValue, Soa4Vector3::InParam falseValue );
RW_MATH_FORCE_INLINE Soa4Vector3 Select(Soa4Mask3::InParam mask, Soa4Vector3::InParam trueValue, Soa4Vector3::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_vector3_operation_inline.h"
#endif

#endif
