// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_QUATERNION_OPERATION_H
#define RW_MATH_VPU_QUATERNION_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask4.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Quaternion& operator *= (Quaternion& v, VecFloat::InParam a);    
RW_MATH_FORCE_INLINE Quaternion& operator /= (Quaternion& v, VecFloat::InParam a);    
RW_MATH_FORCE_INLINE Quaternion& operator += (Quaternion& v, VecFloat::InParam a);    
RW_MATH_FORCE_INLINE Quaternion& operator -= (Quaternion& v, VecFloat::InParam a);    

RW_MATH_FORCE_INLINE Quaternion operator * (Quaternion::InParam v, VecFloat::InParam s);    
RW_MATH_FORCE_INLINE Quaternion operator * (VecFloat::InParam s, Quaternion::InParam v);    
RW_MATH_FORCE_INLINE Quaternion operator / (Quaternion::InParam v, VecFloat::InParam s);    
RW_MATH_FORCE_INLINE Quaternion operator + (Quaternion::InParam v, VecFloat::InParam s);    
RW_MATH_FORCE_INLINE Quaternion operator - (Quaternion::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Quaternion operator + (VecFloat::InParam s, Quaternion::InParam v);    
RW_MATH_FORCE_INLINE Quaternion operator - (VecFloat::InParam s, Quaternion::InParam v);

RW_MATH_FORCE_INLINE Quaternion& operator *= (Quaternion& v, float a);    
RW_MATH_FORCE_INLINE Quaternion& operator /= (Quaternion& v, float a);
RW_MATH_FORCE_INLINE Quaternion& operator += (Quaternion& v, float a);
RW_MATH_FORCE_INLINE Quaternion& operator -= (Quaternion& v, float a);

RW_MATH_FORCE_INLINE Quaternion operator * (Quaternion::InParam v, float s);
RW_MATH_FORCE_INLINE Quaternion operator * (float s, Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion operator / (Quaternion::InParam v, float s);
RW_MATH_FORCE_INLINE Quaternion operator + (Quaternion::InParam v, float s);
RW_MATH_FORCE_INLINE Quaternion operator - (Quaternion::InParam v, float s);
RW_MATH_FORCE_INLINE Quaternion operator + (float s, Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion operator - (float s, Quaternion::InParam v);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion& operator *= (Quaternion& v, VecFloatRef<INDEX> a);    
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion& operator /= (Quaternion& v, VecFloatRef<INDEX> a);    
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion& operator += (Quaternion& v, VecFloatRef<INDEX> a);    
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion& operator -= (Quaternion& v, VecFloatRef<INDEX> a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator * (Quaternion::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator * (VecFloatRef<INDEX> s, Quaternion::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator / (Quaternion::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator + (Quaternion::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator - (Quaternion::InParam v, VecFloatRef<INDEX> s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator + (VecFloatRef<INDEX> s, Quaternion::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion operator - (VecFloatRef<INDEX> s, Quaternion::InParam v);

RW_MATH_FORCE_INLINE Quaternion& operator *= (Quaternion& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Quaternion& operator /= (Quaternion& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Quaternion& operator += (Quaternion& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Quaternion& operator -= (Quaternion& v, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Quaternion operator * (Quaternion::InParam v, VecFloatRefIndex::InParam s);

RW_MATH_FORCE_INLINE Quaternion operator * (VecFloatRefIndex::InParam s, Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion operator / (Quaternion::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Quaternion operator + (Quaternion::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Quaternion operator - (Quaternion::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Quaternion operator + (VecFloatRefIndex::InParam s, Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion operator - (VecFloatRefIndex::InParam s, Quaternion::InParam v);

RW_MATH_FORCE_INLINE Quaternion operator - (Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion& operator += (Quaternion& v, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion& operator -= (Quaternion& v, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion operator - (Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion operator + (Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion operator * (Quaternion::InParam a, Quaternion::InParam b);

RW_MATH_FORCE_INLINE bool operator == (Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE bool operator != (Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE bool IsSimilar(Quaternion::InParam a, Quaternion::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool IsSimilarRotation(Quaternion::InParam a, Quaternion::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool IsValid(Quaternion::InParam q);
RW_MATH_FORCE_INLINE bool IsIdentity(Quaternion::InParam q, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsUnit(Quaternion::InParam q);
RW_MATH_FORCE_INLINE bool IsPure(Quaternion::InParam q);
RW_MATH_FORCE_INLINE bool IsAnyNaN(Quaternion::InParam q);


RW_MATH_FORCE_INLINE Quaternion Pow(Quaternion::InParam v, VecFloat::InParam e);
RW_MATH_FORCE_INLINE Quaternion Negate(Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion Mult(Quaternion::InParam v, VecFloat::InParam s);
RW_MATH_FORCE_INLINE Quaternion MultByVector(Quaternion::InParam q, Vector4::InParam v);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion Mult(Quaternion::InParam v, VecFloatRef<INDEX> s);
RW_MATH_FORCE_INLINE Quaternion Mult(Quaternion::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Quaternion Mult(Quaternion::InParam v, float s);
RW_MATH_FORCE_INLINE Quaternion Divide(Quaternion::InParam v, VecFloat::InParam s);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Quaternion Divide(Quaternion::InParam v, VecFloatRef<INDEX> s);
RW_MATH_FORCE_INLINE Quaternion Divide(Quaternion::InParam v, VecFloatRefIndex::InParam s);
RW_MATH_FORCE_INLINE Quaternion Divide(Quaternion::InParam q, float s);
RW_MATH_FORCE_INLINE Quaternion Add(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion Subtract(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE VecFloat Dot(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion Mult(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE VecFloat Magnitude(Quaternion::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Quaternion::InParam v);
RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion NormalizeFast(Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion NormalizeEstimate(Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion Normalize(Quaternion::InParam v);
RW_MATH_FORCE_INLINE Quaternion Normalize(Quaternion::InParam v, VecFloat& magnitude);

RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Quaternion::InParam v, Quaternion& result);
RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Quaternion::InParam v, Quaternion& result);

RW_MATH_FORCE_INLINE Quaternion Lerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Quaternion SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Quaternion NLerpEstimate(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Quaternion NLerpFast(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Quaternion NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t);
RW_MATH_FORCE_INLINE Quaternion SLerpNoCheck(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t);

RW_MATH_FORCE_INLINE Quaternion Inverse(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Quaternion Conj(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Quaternion Concat(Quaternion::InParam a, Quaternion::InParam b);

RW_MATH_FORCE_INLINE uint32_t CompressRotation(Quaternion::InParam quat);
RW_MATH_FORCE_INLINE Quaternion UnCompressRotation(uint32_t rotation);

RW_MATH_FORCE_INLINE Quaternion QuaternionFromAxisRotationAngle(Vector3::InParam axis, VecFloat::InParam angle_radians);
RW_MATH_FORCE_INLINE void AxisAngleFromQuaternion(Quaternion::InParam q, Vector3& axis_out, VecFloat& angle_out);

RW_MATH_FORCE_INLINE Vector3 EulerXYZFromQuaternion(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Vector3 TransformVector(Vector3::InParam v, Quaternion::InParam q);
RW_MATH_FORCE_INLINE Quaternion QuaternionFromMatrix44(Matrix44::InParam m, VecFloat::InParam epsilon = VecFloat(ZERO_FLOAT));
RW_MATH_FORCE_INLINE Quaternion QuaternionFromMatrix44Affine(Matrix44Affine::InParam m, VecFloat::InParam epsilon = VecFloat(ZERO_FLOAT));
RW_MATH_INLINE Quaternion QuaternionFromMatrix33(Matrix33::InParam m, VecFloat::InParam epsilon = VecFloat(ZERO_FLOAT));
RW_MATH_FORCE_INLINE Quaternion QuaternionFromEulerXYZ(Vector3::InParam euler);
RW_MATH_FORCE_INLINE Quaternion QuaternionFromLookDirection(Vector3::InParam lookDirection, Vector3::InParam up);
RW_MATH_FORCE_INLINE Quaternion QuaternionFromLookAt(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition);
RW_MATH_FORCE_INLINE Vector3 QuaternionGetMatrix44AffineRowX(Quaternion::InParam quat);
RW_MATH_FORCE_INLINE Vector3 QuaternionGetMatrix44AffineRowY(Quaternion::InParam quat);
RW_MATH_FORCE_INLINE Vector3 QuaternionGetMatrix44AffineRowZ(Quaternion::InParam quat);
RW_MATH_FORCE_INLINE Quaternion Min(Quaternion::InParam v0, Quaternion::InParam v1);
RW_MATH_FORCE_INLINE Quaternion Max(Quaternion::InParam v0, Quaternion::InParam v1);
RW_MATH_FORCE_INLINE Quaternion Clamp(Quaternion::InParam quatToClamp, Quaternion::InParam minQuatValue, Quaternion::InParam maxQuatValue);
RW_MATH_FORCE_INLINE Quaternion Abs(Quaternion::InParam v0);

RW_MATH_FORCE_INLINE Mask4      CompLessThan(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Mask4      CompGreaterThan(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Mask4      CompLessEqual(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Mask4      CompGreaterEqual(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Mask4      CompEqual(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Mask4      CompNotEqual(Quaternion::InParam a, Quaternion::InParam b);
RW_MATH_FORCE_INLINE Quaternion Select(MaskScalar::InParam mask, Quaternion::InParam trueValue, Quaternion::InParam falseValue );
RW_MATH_FORCE_INLINE Quaternion Select(Mask4::InParam mask, Quaternion::InParam trueValue, Quaternion::InParam falseValue );

RW_MATH_FORCE_INLINE Vector3 GetXYZ(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Quaternion LoadUnAlignedPacked(const rw::math::fpu::Quaternion &value);
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Quaternion &destination, Quaternion::InParam value);

#if RW_MATH_ENABLE_DEPRECATED_API

    RW_MATH_FORCE_INLINE void Negate(Quaternion::InParam vec, Quaternion& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Mult(Quaternion::InParam v, VecFloatRef<INDEX> s, Quaternion& r);
    RW_MATH_FORCE_INLINE void Mult(Quaternion::InParam v, VecFloatRefIndex::InParam s, Quaternion& r);
    RW_MATH_FORCE_INLINE void Mult(Quaternion::InParam a, Quaternion::InParam b, Quaternion& r);
    RW_MATH_FORCE_INLINE void Mult(Quaternion::InParam vec, VecFloat::InParam scalar, Quaternion& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Divide(Quaternion::InParam v, VecFloatRef<INDEX> s, Quaternion& r);
    RW_MATH_FORCE_INLINE void Divide(Quaternion::InParam v, VecFloatRefIndex::InParam s, Quaternion& r);
    RW_MATH_FORCE_INLINE void Divide(Quaternion::InParam vec, VecFloat::InParam scalar, Quaternion& r);

    RW_MATH_FORCE_INLINE void Add(Quaternion::InParam a, Quaternion::InParam b, Quaternion& r);
    RW_MATH_FORCE_INLINE void Subtract(Quaternion::InParam a, Quaternion::InParam b, Quaternion& r);

    RW_MATH_FORCE_INLINE void Dot(Quaternion::InParam a, Quaternion::InParam b, VecFloat& r);
    RW_MATH_FORCE_INLINE void Normalize(Quaternion::InParam vec, Quaternion& r);

    RW_MATH_FORCE_INLINE void Lerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t, Quaternion& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void Lerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRef<INDEX> t, Quaternion& r);
    RW_MATH_FORCE_INLINE void Lerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRefIndex::InParam t, Quaternion& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRef<INDEX> t, Quaternion& r);
    RW_MATH_FORCE_INLINE void SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t, Quaternion& r);
    RW_MATH_FORCE_INLINE void SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRefIndex::InParam t,   Quaternion& r);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRef<INDEX> t, Quaternion& r);
    RW_MATH_FORCE_INLINE void NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t, Quaternion& r);
    RW_MATH_FORCE_INLINE void NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRefIndex::InParam t, Quaternion& r);


    RW_MATH_FORCE_INLINE void Inverse(Quaternion::InParam q, Quaternion& r);
    RW_MATH_FORCE_INLINE void Conj(Quaternion::InParam q, Quaternion& r);
    RW_MATH_FORCE_INLINE void QuaternionFromAxisAngle(Vector3::InParam axis, VecFloat::InParam angle_radians, Quaternion& r);

    template<VectorAxis INDEX> 
    RW_MATH_FORCE_INLINE void QuaternionFromAxisAngle(Vector3::InParam axis, VecFloatRef<INDEX> angle_radians, Quaternion& r);
    RW_MATH_FORCE_INLINE void QuaternionFromAxisAngle(Vector3::InParam axis, VecFloatRefIndex::InParam angle_radians, Quaternion& r);
    RW_MATH_FORCE_INLINE void EulerXYZFromQuaternion(Quaternion::InParam q, Vector3& r);
    RW_MATH_FORCE_INLINE void Vector3Transform(Quaternion::InParam q, Vector3::InParam v, Vector3& r);
    RW_MATH_FORCE_INLINE void QuaternionFromMatrix44(Matrix44::InParam m, Quaternion& q, VecFloat::InParam epsilon = VecFloat(ZERO_FLOAT));
    RW_MATH_FORCE_INLINE void QuaternionFromMatrix33(Matrix33::InParam m, Quaternion& q, VecFloat::InParam epsilon = VecFloat(ZERO_FLOAT));
    RW_MATH_FORCE_INLINE void QuaternionFromEulerXYZ(Vector3::InParam euler, Quaternion& r);

#endif

}}}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/quaternion_operation_inline.h"
#endif

#endif
