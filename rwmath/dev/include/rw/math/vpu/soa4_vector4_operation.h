// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_VECTOR4_OPERATION_H
#define RW_MATH_VPU_SOA4_VECTOR4_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/soa4_vector4.h"
#include "rw/math/vpu/soa4_vector3.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/soa4_mask4.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Vector4 Soa4Vector4FromVector4(Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4 Soa4Vector4FromVector4(Vector4::InParam v0, Vector4::InParam v1, Vector4::InParam v2, Vector4::InParam v3);
RW_MATH_FORCE_INLINE Soa4Vector4 Soa4Vector4FromVector4s(Vector4::InParam v0, Vector4::InParam v1, Vector4::InParam v2, Vector4::InParam v3);
RW_MATH_FORCE_INLINE Soa4Vector4 Soa4Vector4FromVector4(const Vector4* v);

RW_MATH_FORCE_INLINE void Vector4FromSoa4Vector4(Soa4Vector4::InParam, Vector4& v0, Vector4& v1, Vector4& v2, Vector4& v3);
RW_MATH_FORCE_INLINE void Vector4FromSoa4Vector4(Soa4Vector4::InParam, Vector4* v);

RW_MATH_FORCE_INLINE Soa4Vector4& operator *= (Soa4Vector4& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4& operator *= (Soa4Vector4& v, Soa4Vector4::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4& operator /= (Soa4Vector4& v, Soa4Vector4::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4& operator /= (Soa4Vector4& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4& operator += (Soa4Vector4& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4& operator -= (Soa4Vector4& v, Soa4Float::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4  operator *  (Soa4Float::InParam s, Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4  operator *  (Soa4Vector4::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4  operator *  (Soa4Vector4::InParam v, Soa4Vector4::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4  operator /  (Soa4Vector4::InParam v, Soa4Vector4::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4  operator /  (Soa4Vector4::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4  operator +  (Soa4Vector4::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4  operator -  (Soa4Vector4::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4  operator +  (Soa4Float::InParam s, Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4  operator -  (Soa4Float::InParam s, Soa4Vector4::InParam v);

RW_MATH_FORCE_INLINE Soa4Vector4  operator -  (Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4& operator += (Soa4Vector4& v, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4& operator -= (Soa4Vector4& v, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4  operator -  (Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4  operator +  (Soa4Vector4::InParam a, Soa4Vector4::InParam b);

RW_MATH_FORCE_INLINE Soa4Vector4 Negate(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4 Mult(Soa4Vector4::InParam v, Soa4Float::InParam s);

RW_MATH_FORCE_INLINE Soa4Vector4 Divide(Soa4Vector4::InParam v, Soa4Float::InParam s);
RW_MATH_FORCE_INLINE Soa4Vector4 Add(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4 Subtract(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Float Dot(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4 Mult(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4 Divide(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4 Reciprocal(Soa4Vector4::InParam a);
RW_MATH_FORCE_INLINE Soa4Vector4 MultAdd(Soa4Vector4::InParam a, Soa4Vector4::InParam b, Soa4Vector4::InParam c);
RW_MATH_FORCE_INLINE Soa4Float Magnitude(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeFast(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeEstimate(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeSquared(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Float MagnitudeRecip(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4 NormalizeFast(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4 NormalizeEstimate(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4 Normalize(Soa4Vector4::InParam v);
RW_MATH_FORCE_INLINE Soa4Vector4 Normalize(Soa4Vector4::InParam v, Soa4Float& magnitude);
RW_MATH_FORCE_INLINE Soa4Float NormalizeReturnMagnitude(Soa4Vector4::InParam v, Soa4Vector4& result);
RW_MATH_FORCE_INLINE Soa4Float CheckedNormalizeReturnMagnitude(Soa4Vector4::InParam v, Soa4Vector4& result);
RW_MATH_FORCE_INLINE Soa4Vector4 Lerp(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector4 SLerp(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector4 NLerpEstimate(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector4 NLerpFast(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector4 NLerp(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1, Soa4Float::InParam t);
RW_MATH_FORCE_INLINE Soa4Vector4 Min(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector4 Max(Soa4Vector4::InParam v0, Soa4Vector4::InParam v1);
RW_MATH_FORCE_INLINE Soa4Vector4 Clamp(Soa4Vector4::InParam vecToClamp, Soa4Vector4::InParam minVecValue, Soa4Vector4::InParam maxVecValue);
RW_MATH_FORCE_INLINE Soa4Vector4 ClampToNegPosRange(Soa4Vector4::InParam vecToClamp, Soa4Vector4::InParam range);
RW_MATH_FORCE_INLINE Soa4Vector4 Abs(Soa4Vector4::InParam v0);

RW_MATH_FORCE_INLINE Soa4Mask4   CompLessThan(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4   CompGreaterThan(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4   CompLessEqual(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4   CompGreaterEqual(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4   CompEqual(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Mask4   CompNotEqual(Soa4Vector4::InParam a, Soa4Vector4::InParam b);
RW_MATH_FORCE_INLINE Soa4Vector4 Select(Soa4MaskScalar::InParam mask, Soa4Vector4::InParam trueValue, Soa4Vector4::InParam falseValue );
RW_MATH_FORCE_INLINE Soa4Vector4 Select(Soa4Mask4::InParam mask, Soa4Vector4::InParam trueValue, Soa4Vector4::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_vector4_operation_inline.h"
#endif

#endif
