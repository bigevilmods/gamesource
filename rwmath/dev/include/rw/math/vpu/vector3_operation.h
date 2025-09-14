// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR3_OPERATION_H
#define RW_MATH_VPU_VECTOR3_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector2.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask3.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector3 Vector3FromFloat(float xyz);

RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, Vector3::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, Vector3::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector3  operator *  (Vector3::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator *  (VecFloat::InParam s, Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3  operator *  (Vector3::InParam v, Vector3::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator /  (Vector3::InParam v, Vector3::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator /  (Vector3::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator +  (Vector3::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator +  (VecFloat::InParam s, Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3  operator -  (VecFloat::InParam s, Vector3::InParam v);

RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v,   float a);
RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v,   float a);
RW_MATH_FORCE_INLINE Vector3  operator *  (Vector3::InParam v,      float s);
RW_MATH_FORCE_INLINE Vector3  operator *  (float s, Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3  operator /  (Vector3::InParam v,      float s);
RW_MATH_FORCE_INLINE Vector3  operator +  (Vector3::InParam v,      float s);
RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam v,      float s);
RW_MATH_FORCE_INLINE Vector3  operator +  (float s,      Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3  operator -  (float s,      Vector3::InParam v);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator *  (Vector3::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator *  (VecFloatRef<INDEX> s, Vector3::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator /  (Vector3::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator +  (Vector3::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator +  (VecFloatRef<INDEX> s, Vector3::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector3  operator -  (VecFloatRef<INDEX> s, Vector3::InParam v);

RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector3  operator *  (Vector3::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator *  (VecFloatRefIndex::InParam s, Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3  operator /  (Vector3::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator +  (Vector3::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector3  operator +  (VecFloatRefIndex::InParam s, Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3  operator -  (VecFloatRefIndex::InParam s, Vector3::InParam v);

RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3  operator +  (Vector3::InParam a, Vector3::InParam b);

RW_MATH_FORCE_INLINE bool operator ==(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE bool operator !=(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE bool IsSimilar(Vector3::InParam a, Vector3::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool IsAnyNaN(Vector3::InParam v);

RW_MATH_FORCE_INLINE Vector3 Negate(Vector3::InParam v);

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, VecFloatRef<INDEX> s);
RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, float s);

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, VecFloatRef<INDEX> s);
RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, float s);
RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, VecFloatRefIndex::InParam s);

RW_MATH_FORCE_INLINE Vector3 Add(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3 Subtract(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE VecFloat Dot(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3 Cross(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3 Reciprocal(Vector3::InParam a);
RW_MATH_FORCE_INLINE Vector3 MultAdd(Vector3::InParam a, Vector3::InParam b, Vector3::InParam c);
RW_MATH_FORCE_INLINE VecFloat Magnitude(Vector3::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeFast(Vector3::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeEstimate(Vector3::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Vector3::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3 NormalizeFast(Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3 NormalizeEstimate(Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3 Normalize(Vector3::InParam v);
RW_MATH_FORCE_INLINE Vector3 Normalize(Vector3::InParam v, VecFloat& magnitude);
RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Vector3::InParam v, Vector3& result);
RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Vector3::InParam v, Vector3& result);
RW_MATH_FORCE_INLINE VecFloat CosAngleBetweenNormalizedVectors(Vector3::InParam v0, Vector3::InParam v1);
RW_MATH_FORCE_INLINE Vector3 Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector3 SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector3 NLerpEstimate(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector3 NLerpFast(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector3 NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE bool IsValid(Vector3::InParam v);
RW_MATH_FORCE_INLINE bool IsZero(Vector3::InParam v, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE Vector3 Min(Vector3::InParam v0, Vector3::InParam v1);
RW_MATH_FORCE_INLINE Vector3 Max(Vector3::InParam v0, Vector3::InParam v1);
RW_MATH_FORCE_INLINE Vector3 Clamp(Vector3::InParam vecToClamp, Vector3::InParam minVecValue, Vector3::InParam maxVecValue);
RW_MATH_FORCE_INLINE Vector3 ClampToNegPosRange(Vector3::InParam vecToClamp, Vector3::InParam range);
RW_MATH_FORCE_INLINE Vector3 Abs(Vector3::InParam v0);
RW_MATH_FORCE_INLINE Vector2 GetXY(Vector3::InParam v);
RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectors(Vector3::InParam v0, Vector3::InParam v1);
RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectors(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam windAxis);

template<int PermConstant>
RW_MATH_FORCE_INLINE Vector3 Permute(Vector3::InParam v);

RW_MATH_FORCE_INLINE Mask3   CompLessThan(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Mask3   CompGreaterThan(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Mask3   CompLessEqual(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Mask3   CompGreaterEqual(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Mask3   CompEqual(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Mask3   CompNotEqual(Vector3::InParam a, Vector3::InParam b);
RW_MATH_FORCE_INLINE Vector3 Select(MaskScalar::InParam mask, Vector3::InParam trueValue, Vector3::InParam falseValue );
RW_MATH_FORCE_INLINE Vector3 Select(Mask3::InParam mask, Vector3::InParam trueValue, Vector3::InParam falseValue );

RW_MATH_FORCE_INLINE VecFloat MaxComponent(Vector3::InParam v);
RW_MATH_FORCE_INLINE VecFloat MinComponent(Vector3::InParam v);

RW_MATH_FORCE_INLINE Vector3 SqrtFast(Vector3::InParam a);
RW_MATH_FORCE_INLINE Vector3 InvSqrtFast(Vector3::InParam a);
RW_MATH_FORCE_INLINE Vector3 ReciprocalFast(Vector3::InParam a);

RW_MATH_FORCE_INLINE Vector3 RotateLeft(Vector3::InParam a);
RW_MATH_FORCE_INLINE Vector3 RotateRight(Vector3::InParam a);

RW_MATH_FORCE_INLINE Vector3 LoadUnAlignedPacked(const rw::math::fpu::Vector3 &value);
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Vector3 &destination, Vector3::InParam value);

#if RW_MATH_ENABLE_DEPRECATED_API

    RW_MATH_FORCE_INLINE void Negate(Vector3::InParam vec, Vector3& r);

    template<VectorAxis INDEX> 
    RW_MATH_FORCE_INLINE void Mult(Vector3::InParam v, VecFloatRef<INDEX> s, Vector3& r);
    RW_MATH_FORCE_INLINE void Mult(Vector3::InParam v, VecFloatRefIndex::InParam s, Vector3& r);
    RW_MATH_FORCE_INLINE void Mult(Vector3::InParam a, Vector3::InParam b, Vector3& r);
    RW_MATH_FORCE_INLINE void Mult(Vector3::InParam vec, VecFloat::InParam scalar, Vector3& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Divide(Vector3::InParam v, VecFloatRef<INDEX> s, Vector3& r);
    RW_MATH_FORCE_INLINE void Divide(Vector3::InParam vec, VecFloat::InParam scalar, Vector3& r);
    RW_MATH_FORCE_INLINE void Divide(Vector3::InParam v, VecFloatRefIndex::InParam s, Vector3& r);
     
    RW_MATH_FORCE_INLINE void Add(Vector3::InParam a, Vector3::InParam b, Vector3& r);
    RW_MATH_FORCE_INLINE void Subtract(Vector3::InParam a, Vector3::InParam b, Vector3& r);
    RW_MATH_FORCE_INLINE void Dot(Vector3::InParam a, Vector3::InParam b, VecFloat& r);
    RW_MATH_FORCE_INLINE void Cross(Vector3::InParam a, Vector3::InParam b, Vector3& result);
    RW_MATH_FORCE_INLINE void Normalize(Vector3::InParam vec, Vector3& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRef<INDEX> t, Vector3& r);
    RW_MATH_FORCE_INLINE void Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRefIndex::InParam t, Vector3& r);
    RW_MATH_FORCE_INLINE void Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t, Vector3& r);
     
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRef<INDEX> t, Vector3& r);
    RW_MATH_FORCE_INLINE void SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t, Vector3& r);
    RW_MATH_FORCE_INLINE void SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRefIndex::InParam t, Vector3& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRef<INDEX> t, Vector3& r);
    RW_MATH_FORCE_INLINE void NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t, Vector3& r);
    RW_MATH_FORCE_INLINE void NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRefIndex::InParam t, Vector3& r);

#endif

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vector3_operation_inline.h"
#endif

#endif
