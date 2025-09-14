// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_VEC_FLOAT_OPERATION_H
#define RW_MATH_SOA4_VEC_FLOAT_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/soa4_soa4base.h"
#include "rw/math/vpu/soa4_float.h"

namespace rw
{
namespace math
{
namespace vpu
{

//
// Conversions to SoA4
//
RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromFloat(const float& v);
RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromFloats(const float& v0, const float& v1, const float& v2, const float& v3 );
RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromVecFloat(VecFloat::InParam v);
RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromVecFloats(VecFloat::InParam v0, VecFloat::InParam v1, VecFloat::InParam v2, VecFloat::InParam v3 );
RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromVector4(Vector4::InParam v);

//
// Conversions back to AoS
//
RW_MATH_FORCE_INLINE void Soa4FloatToFloats(Soa4Float::InParam v, float& r0, float& r1, float& r2, float& r3 );
RW_MATH_FORCE_INLINE void Soa4FloatToVecFloats(Soa4Float::InParam v, VecFloat& r0, VecFloat& r1, VecFloat& r2, VecFloat& r3);
RW_MATH_FORCE_INLINE Vector4 Soa4FloatToVector4(Soa4Float::InParam v);

RW_MATH_FORCE_INLINE Soa4Float  Negate(Soa4Float::InParam a);

RW_MATH_FORCE_INLINE Soa4Float& SelfAdd(Soa4Float& a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float& SelfSubtract(Soa4Float& a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float& SelfMult(Soa4Float& a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float& SelfDivide(Soa4Float& a, Soa4Float::InParam b);

RW_MATH_FORCE_INLINE Soa4Float  Add(Soa4Float::InParam a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float  Subtract(Soa4Float::InParam a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float  Mult(Soa4Float::InParam a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float  Divide(Soa4Float::InParam a, Soa4Float::InParam b);

RW_MATH_FORCE_INLINE Soa4Float  operator - (Soa4Float::InParam a);

RW_MATH_FORCE_INLINE Soa4Float& operator += (Soa4Float& a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float& operator -= (Soa4Float& a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float& operator *= (Soa4Float& a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float& operator /= (Soa4Float& a, Soa4Float::InParam b);

RW_MATH_FORCE_INLINE Soa4Float  operator + (Soa4Float::InParam a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float  operator - (Soa4Float::InParam a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float  operator * (Soa4Float::InParam a, Soa4Float::InParam b);
RW_MATH_FORCE_INLINE Soa4Float  operator / (Soa4Float::InParam a, Soa4Float::InParam b);

} // namespace vpu
} // namespace math
} // namespace rw

#include "rw/math/vpu/detail/soa4_float_operation_inline.h"

#endif
