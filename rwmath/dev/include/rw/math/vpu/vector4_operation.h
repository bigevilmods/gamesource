// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR4_OPERATION_H
#define RW_MATH_VPU_VECTOR4_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask4.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector4 Vector4FromFloat(float xyzw);

RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, Vector4::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, Vector4::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Vector4  operator *  (VecFloat::InParam s, Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4  operator *  (Vector4::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator *  (Vector4::InParam v, Vector4::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator /  (Vector4::InParam v, Vector4::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator /  (Vector4::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator +  (Vector4::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator +  (VecFloat::InParam s, Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4  operator -  (VecFloat::InParam s, Vector4::InParam v);

RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, float a);
RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, float a);
RW_MATH_FORCE_INLINE Vector4  operator *  (float s, Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4  operator *  (Vector4::InParam v, float s);
RW_MATH_FORCE_INLINE Vector4  operator /  (Vector4::InParam v, float s);
RW_MATH_FORCE_INLINE Vector4  operator +  (Vector4::InParam v, float s);
RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam v, float s);
RW_MATH_FORCE_INLINE Vector4  operator +  (float s, Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4  operator -  (float s, Vector4::InParam v);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator *  (VecFloatRef<INDEX> s, Vector4::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator *  (Vector4::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator /  (Vector4::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator +  (Vector4::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator +  (VecFloatRef<INDEX> s, Vector4::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Vector4  operator -  (VecFloatRef<INDEX> s, Vector4::InParam v);

RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Vector4  operator *  (Vector4::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator *  (VecFloatRefIndex::InParam s, Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4  operator /  (Vector4::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator +  (Vector4::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector4  operator +  (VecFloatRefIndex::InParam s, Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4  operator -  (VecFloatRefIndex::InParam s, Vector4::InParam v);

RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4  operator +  (Vector4::InParam a, Vector4::InParam b);

RW_MATH_FORCE_INLINE bool operator ==(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE bool operator !=(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE bool IsSimilar(Vector4::InParam a, Vector4::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));

RW_MATH_FORCE_INLINE Vector4 Negate(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v,        float s);
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, VecFloatRef<INDEX> s);
RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam v,        float s);
template<VectorAxis INDEX> Vector4 Divide(Vector4::InParam v, VecFloatRef<INDEX> s);
RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Vector4 Add(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4 Subtract(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE VecFloat Dot(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4 Reciprocal(Vector4::InParam a);
RW_MATH_FORCE_INLINE Vector4 MultAdd(Vector4::InParam a, Vector4::InParam b, Vector4::InParam c);
RW_MATH_FORCE_INLINE Vector4 DivideByW(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat Magnitude(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeFast(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeEstimate(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4 NormalizeFast(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4 NormalizeEstimate(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4 Normalize(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4 Normalize(Vector4::InParam v, VecFloat& magnitude);
RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Vector4::InParam v, Vector4& result);
RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Vector4::InParam v, Vector4& result);
RW_MATH_FORCE_INLINE Vector4 Lerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector4 SLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector4 NLerpEstimate(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector4 NLerpFast(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Vector4 NLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE bool IsValid(Vector4::InParam v);
RW_MATH_FORCE_INLINE bool IsZero(Vector4::InParam v, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsAnyNaN(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector4 Min(Vector4::InParam v0, Vector4::InParam v1);
RW_MATH_FORCE_INLINE Vector4 Max(Vector4::InParam v0, Vector4::InParam v1);
RW_MATH_FORCE_INLINE Vector4 Clamp(Vector4::InParam vecToClamp, Vector4::InParam minVecValue, Vector4::InParam maxVecValue);
RW_MATH_FORCE_INLINE Vector4 ClampToNegPosRange(Vector4::InParam vecToClamp, Vector4::InParam range);
RW_MATH_FORCE_INLINE Vector4 Abs(Vector4::InParam v0);

RW_MATH_FORCE_INLINE void SetXYZ(Vector4& v, VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z);
template<VectorAxis INDEX0, VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE void SetXYZ(Vector4& v, VecFloatRef<INDEX0> x, VecFloatRef<INDEX1> y, VecFloatRef<INDEX2> z);
RW_MATH_FORCE_INLINE void SetXYZ(Vector4& v, VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z);
RW_MATH_FORCE_INLINE Vector3 GetXYZ(Vector4::InParam v);
RW_MATH_FORCE_INLINE Vector2  GetXY(Vector4::InParam v);

RW_MATH_FORCE_INLINE VecFloat MagnitudeXYZ(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeSquaredXYZ(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeRecipXYZ(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat DotXYZ(Vector4::InParam v0, Vector4::InParam v1);
RW_MATH_FORCE_INLINE Vector4 CrossXYZ(Vector4::InParam v0, Vector4::InParam v1);

template<int PermConstant>
RW_MATH_FORCE_INLINE Vector4 Permute(Vector4::InParam v);

RW_MATH_FORCE_INLINE Mask4   CompLessThan(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Mask4   CompGreaterThan(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Mask4   CompLessEqual(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Mask4   CompGreaterEqual(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Mask4   CompEqual(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Mask4   CompNotEqual(Vector4::InParam a, Vector4::InParam b);
RW_MATH_FORCE_INLINE Vector4 Select(MaskScalar::InParam mask, Vector4::InParam trueValue, Vector4::InParam falseValue );
RW_MATH_FORCE_INLINE Vector4 Select(Mask4::InParam mask, Vector4::InParam trueValue, Vector4::InParam falseValue );

RW_MATH_FORCE_INLINE VecFloat MaxComponent(Vector4::InParam v);
RW_MATH_FORCE_INLINE VecFloat MinComponent(Vector4::InParam v);

RW_MATH_FORCE_INLINE Vector4 SqrtFast(Vector4::InParam a);
RW_MATH_FORCE_INLINE Vector4 InvSqrtFast(Vector4::InParam a);
RW_MATH_FORCE_INLINE Vector4 ReciprocalFast(Vector4::InParam a);

RW_MATH_FORCE_INLINE Vector4 RotateLeft(Vector4::InParam a);
RW_MATH_FORCE_INLINE Vector4 RotateRight(Vector4::InParam a);

RW_MATH_FORCE_INLINE Vector4 LoadUnAlignedPacked(const rw::math::fpu::Vector4 &value);
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Vector4 &destination, Vector4::InParam value);

#if RW_MATH_ENABLE_DEPRECATED_API
    
    RW_MATH_FORCE_INLINE void Negate(Vector4::InParam vec, Vector4& r);    
    RW_MATH_FORCE_INLINE void Mult(Vector4::InParam vec, VecFloat::InParam scalar, Vector4& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Mult(Vector4::InParam v, VecFloatRef<INDEX> s, Vector4& r);    
    RW_MATH_FORCE_INLINE void Mult(Vector4::InParam v, VecFloatRefIndex::InParam s, Vector4& r);    
    RW_MATH_FORCE_INLINE void Divide(Vector4::InParam vec, VecFloat::InParam scalar, Vector4& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Divide(Vector4::InParam v, VecFloatRef<INDEX> s, Vector4& r);    
    RW_MATH_FORCE_INLINE void Divide(Vector4::InParam v, VecFloatRefIndex::InParam s, Vector4& r);    
    RW_MATH_FORCE_INLINE void Add(Vector4::InParam a, Vector4::InParam b, Vector4& r);    
    RW_MATH_FORCE_INLINE void Subtract(Vector4::InParam a, Vector4::InParam b, Vector4& r);    
    RW_MATH_FORCE_INLINE void Dot(Vector4::InParam a, Vector4::InParam b, VecFloat& r);    
    RW_MATH_FORCE_INLINE void Mult(Vector4::InParam a, Vector4::InParam b, Vector4& r);    
    RW_MATH_FORCE_INLINE void Normalize(Vector4::InParam vec, Vector4& r);    
    RW_MATH_FORCE_INLINE void Lerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t, Vector4& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Lerp(Vector4::InParam v0, Vector4::InParam v1, VecFloatRef<INDEX> t, Vector4& r);    
    RW_MATH_FORCE_INLINE void Lerp(Vector4::InParam v0, Vector4::InParam v1, VecFloatRefIndex::InParam t, Vector4& r);    
    RW_MATH_FORCE_INLINE void SLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t, Vector4& r);    
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloatRef<INDEX> t, Vector4& r);    
    RW_MATH_FORCE_INLINE void SLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloatRefIndex::InParam t, Vector4& r);    
    RW_MATH_FORCE_INLINE void NLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t, Vector4& r);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void NLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloatRef<INDEX> t, Vector4& r);    
    RW_MATH_FORCE_INLINE void NLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloatRefIndex::InParam t, Vector4& r);    
    RW_MATH_FORCE_INLINE void CrossXYZ(Vector4::InParam v0, Vector4::InParam v1, Vector4& result);
#endif

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vector4_operation_inline.h"
#endif

#endif
