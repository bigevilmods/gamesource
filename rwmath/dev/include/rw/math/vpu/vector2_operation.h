// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR2_OPERATION_H
#define RW_MATH_VPU_VECTOR2_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector2.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask2.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector2 Vector2FromFloat(float xy);

RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector2  operator *  (VecFloat::InParam s, Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2  operator *  (Vector2::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator *  (Vector2::InParam v, Vector2::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator /  (Vector2::InParam v, Vector2::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator /  (Vector2::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator +  (Vector2::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator -  (Vector2::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator +  (VecFloat::InParam s, Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2  operator -  (VecFloat::InParam s, Vector2::InParam v);

RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, Vector2::InParam s);
RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, Vector2::InParam s);
RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, float a);
RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, float a);
RW_MATH_FORCE_INLINE Vector2  operator *  (float s,  Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2  operator *  (Vector2::InParam v, float s);
RW_MATH_FORCE_INLINE Vector2  operator /  (Vector2::InParam v, float s);
RW_MATH_FORCE_INLINE Vector2  operator +  (Vector2::InParam v, float s);
RW_MATH_FORCE_INLINE Vector2  operator -  (Vector2::InParam v, float s);
RW_MATH_FORCE_INLINE Vector2  operator +  (float s, Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2  operator -  (float s, Vector2::InParam v);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator *  (VecFloatRef<INDEX> s, Vector2::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator *  (Vector2::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator /  (Vector2::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator +  (Vector2::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator -  (Vector2::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator +  (VecFloatRef<INDEX> s, Vector2::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2  operator -  (VecFloatRef<INDEX> s, Vector2::InParam v);

RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector2  operator *  (Vector2::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator *  (VecFloatRefIndex::InParam s, Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2  operator /  (Vector2::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator +  (Vector2::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator -  (Vector2::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector2  operator +  (VecFloatRefIndex::InParam s, Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2  operator -  (VecFloatRefIndex::InParam s, Vector2::InParam v);

RW_MATH_FORCE_INLINE Vector2  operator -  (Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, Vector2::InParam b);
RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, Vector2::InParam b);
RW_MATH_FORCE_INLINE Vector2  operator -  (Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Vector2  operator +  (Vector2::InParam a, Vector2::InParam b);

RW_MATH_FORCE_INLINE bool operator ==(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE bool operator !=(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE bool IsSimilar(Vector2::InParam a, Vector2::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool IsAnyNaN(Vector2::InParam v);

RW_MATH_FORCE_INLINE Vector2 Negate(Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2 Add(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Vector2 Subtract(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE VecFloat Dot(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE VecFloat Cross(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Vector2 MultAdd(Vector2::InParam a, Vector2::InParam b, Vector2::InParam c);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, VecFloatRef<INDEX> s);
                           RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, VecFloat::InParam s);
                           RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, float s);
                           RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, VecFloatRefIndex::InParam s);
                           RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam a, Vector2::InParam b);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, VecFloatRef<INDEX> s);
                           RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, VecFloat::InParam s);
                           RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, float s);
                           RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, VecFloatRefIndex::InParam s);
                           RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam a, Vector2::InParam b);

RW_MATH_FORCE_INLINE Vector2 Reciprocal(Vector2::InParam a);
RW_MATH_FORCE_INLINE VecFloat Magnitude(Vector2::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeFast(Vector2::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeEstimate(Vector2::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Vector2::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2 NormalizeFast(Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2 NormalizeEstimate(Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2 Normalize(Vector2::InParam v);
RW_MATH_FORCE_INLINE Vector2 Normalize(Vector2::InParam v, VecFloat& magnitude);
RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Vector2::InParam v, Vector2& result);
RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Vector2::InParam v, Vector2& result);
RW_MATH_FORCE_INLINE VecFloat CosAngleBetweenNormalizedVectors(Vector2::InParam v0, Vector2::InParam v1);
RW_MATH_FORCE_INLINE Vector2 Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector2 SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector2 NLerpEstimate(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector2 NLerpFast(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector2 NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE bool IsValid(Vector2::InParam v);
RW_MATH_FORCE_INLINE bool IsZero(Vector2::InParam v, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE Vector2 Min(Vector2::InParam v0, Vector2::InParam v1);
RW_MATH_FORCE_INLINE Vector2 Max(Vector2::InParam v0, Vector2::InParam v1);
RW_MATH_FORCE_INLINE Vector2 Clamp(Vector2::InParam vecToClamp, Vector2::InParam minVecValue, Vector2::InParam maxVecValue);
RW_MATH_FORCE_INLINE Vector2 ClampToNegPosRange(Vector2::InParam vecToClamp, Vector2::InParam range);
RW_MATH_FORCE_INLINE Vector2 Abs(Vector2::InParam v0);
RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectors(Vector2::InParam v0, Vector2::InParam v1);
RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectorsFast(Vector2::InParam v0, Vector2::InParam v1);

template<int PermConstant>
RW_MATH_FORCE_INLINE Vector2 Permute(Vector2::InParam v);

RW_MATH_FORCE_INLINE Mask2   CompLessThan(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Mask2   CompGreaterThan(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Mask2   CompLessEqual(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Mask2   CompGreaterEqual(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Mask2   CompEqual(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Mask2   CompNotEqual(Vector2::InParam a, Vector2::InParam b);
RW_MATH_FORCE_INLINE Vector2 Select(MaskScalar::InParam mask, Vector2::InParam trueValue, Vector2::InParam falseValue );
RW_MATH_FORCE_INLINE Vector2 Select(Mask2::InParam mask, Vector2::InParam trueValue, Vector2::InParam falseValue );

RW_MATH_FORCE_INLINE VecFloat MaxComponent(Vector2::InParam v);
RW_MATH_FORCE_INLINE VecFloat MinComponent(Vector2::InParam v);

RW_MATH_FORCE_INLINE Vector2 SqrtFast(Vector2::InParam a);
RW_MATH_FORCE_INLINE Vector2 InvSqrtFast(Vector2::InParam a);
RW_MATH_FORCE_INLINE Vector2 ReciprocalFast(Vector2::InParam a);

RW_MATH_FORCE_INLINE Vector2 RotateLeft(Vector2::InParam a);
RW_MATH_FORCE_INLINE Vector2 RotateRight(Vector2::InParam a);

RW_MATH_FORCE_INLINE Vector2 LoadUnAlignedPacked(const rw::math::fpu::Vector2 &value);
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Vector2 &destination, Vector2::InParam value);

#if RW_MATH_ENABLE_DEPRECATED_API

    
    RW_MATH_FORCE_INLINE void Negate(Vector2::InParam vec, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Mult(Vector2::InParam vec, VecFloat::InParam scalar, Vector2& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Mult(Vector2::InParam v, VecFloatRef<INDEX> s, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Mult(Vector2::InParam v, VecFloatRefIndex::InParam s, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Divide(Vector2::InParam vec, VecFloat::InParam scalar, Vector2& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Divide(Vector2::InParam v, VecFloatRef<INDEX> s, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Divide(Vector2::InParam v, VecFloatRefIndex::InParam s, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Add(Vector2::InParam a, Vector2::InParam b, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Subtract(Vector2::InParam a, Vector2::InParam b, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Mult(Vector2::InParam a, Vector2::InParam b, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Normalize(Vector2::InParam vec, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t, Vector2& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRef<INDEX> t, Vector2& r);
    
    RW_MATH_FORCE_INLINE void Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRefIndex::InParam t,   Vector2& r);
    
    RW_MATH_FORCE_INLINE void SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t, Vector2& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRef<INDEX> t, Vector2& r);
    
    RW_MATH_FORCE_INLINE void SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRefIndex::InParam t,   Vector2& r);
    
    RW_MATH_FORCE_INLINE void NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t, Vector2& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRef<INDEX> t, Vector2& r);
    
    RW_MATH_FORCE_INLINE void NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRefIndex::InParam t,   Vector2& r);
     
    RW_MATH_FORCE_INLINE void Dot(Vector2::InParam a, Vector2::InParam b, VecFloat& r);

#endif

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vector2_operation_inline.h"
#endif

#endif
