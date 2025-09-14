// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR4_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR4_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/static_asserts.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/vector3_operation.h"

#include "rw/math/vpu/vector_intrinsic_operation.h"

#include "rw/math/vpu/detail/vector4_operation_platform_inline.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector4 Vector4FromFloat(float xyzw)
{
    return Vector4(xyzw, xyzw, xyzw, xyzw);
}

RW_MATH_FORCE_INLINE bool IsZero(Vector4::InParam v, VecFloat::InParam tolerance) 
{
    return _asmIsZeroV4(v.mV, tolerance.mV);
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Vector4::InParam v) 
{
    bool equal;
    RW_MATH_VPU_IsEqualV4(equal, v.mV, v.mV);
    return !equal;
}

RW_MATH_FORCE_INLINE bool IsSimilar(Vector4::InParam a, Vector4::InParam b, VecFloat::InParam tolerance) 
{
    return _asmIsSimilarV4(a.mV, b.mV, tolerance.mV);
}

RW_MATH_FORCE_INLINE bool IsValid(Vector4::InParam v) 
{
    return (IsValid(v.X()) && IsValid(v.Y()) && IsValid(v.Z()) && IsValid(v.W()));
}

RW_MATH_FORCE_INLINE bool operator == (Vector4::InParam a, Vector4::InParam b) 
{
    return  _asmIsEqualV4(a.mV, b.mV);
}

RW_MATH_FORCE_INLINE bool operator != (Vector4::InParam a, Vector4::InParam b) 
{
    return  !_asmIsEqualV4(a.mV, b.mV);
}

// basic functions
RW_MATH_FORCE_INLINE Vector4 Negate(Vector4::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Negate(result, v.mV);
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Min(Vector4::InParam v0, Vector4::InParam v1) 
{
    VectorIntrinsic r;
    RW_MATH_VPU_Min(r, v0.mV, v1.mV);
    return Vector4(r);
}

RW_MATH_FORCE_INLINE Vector4 Max(Vector4::InParam v0, Vector4::InParam v1) 
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    return Vector4(r);
}

RW_MATH_FORCE_INLINE Vector4 Clamp(Vector4::InParam v0, Vector4::InParam v1, Vector4::InParam v2)
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    RW_MATH_VPU_Min(r, r, v2.mV);
    return Vector4(r);
}

RW_MATH_FORCE_INLINE Vector4 ClampToNegPosRange(Vector4::InParam value, Vector4::InParam range)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ClampToNegPosRange( result, value.mV, range.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Abs(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Abs(ret, v.mV);
    return Vector4(ret);
}


RW_MATH_FORCE_INLINE VecFloat Dot(Vector4::InParam a, Vector4::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV4(ret, b.mV, a.mV);
    return VecFloat(ret);
}


RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 operator * (Vector4::InParam v, Vector4::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 operator / (Vector4::InParam v, Vector4::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide( ret, v.mV, s.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, float s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultFloat( result, v.mV, s );
    return Vector4( result );
}

template<VectorAxis INDEX> Vector4 Mult(Vector4::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Divide( result, v.mV, s.mV );
    return Vector4( result );
}

RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam v, float s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideFloat( ret, v.mV, s );
    return Vector4( ret );
}

template<VectorAxis INDEX> Vector4 Divide(Vector4::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Add(Vector4::InParam a, Vector4::InParam b) 
{
    return Vector4( RW_MATH_VPU_Add_R(a.mV, b.mV) );
}

RW_MATH_FORCE_INLINE Vector4 Subtract(Vector4::InParam a, Vector4::InParam b) 
{
    return Vector4( RW_MATH_VPU_Subtract_R(a.mV, b.mV) );
}

RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam a, Vector4::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, a.mV, b.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 Divide(Vector4::InParam a, Vector4::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide( ret, a.mV, b.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 Reciprocal(Vector4::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Reciprocal( ret, a.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 MultAdd(Vector4::InParam a, Vector4::InParam b, Vector4::InParam c) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultAdd(ret, a.mV, b.mV, c.mV);
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 DivideByW(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideAxisA1_I( ret, v.mV, v.mV, VectorAxisW );
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE VecFloat Magnitude(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootDotV4(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeFast(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFastDotV4(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeEstimate(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootEstimateDotV4(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV4(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalSquareRootDotV4(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE Vector4 NormalizeEstimate(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV4(ret, v.mV);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4 NormalizeFast(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV4(ret, v.mV);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4 Normalize(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV4(ret, v.mV);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4 Normalize(Vector4::InParam v, VecFloat& magnitude) 
{
    VectorIntrinsic normalized;
    RW_MATH_VPU_NormalizeWithMagnitudeV4(normalized, magnitude.mV, v.mV);
    return Vector4(normalized);
}

RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Vector4::InParam v, Vector4& result) 
{
    VectorIntrinsic magnitude;
    RW_MATH_VPU_NormalizeWithMagnitudeV4(result.mV, magnitude, v.mV);
    return VecFloat(magnitude);
}

RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Vector4::InParam v, Vector4& result)
{
    VectorIntrinsic normalized;
    VectorIntrinsic magnitude;
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic minMagnitude = zero;
    RW_MATH_VPU_NormalizeWithMagnitudeV4(normalized, magnitude, v.mV);
    VectorIntrinsic magnitudeSufficientMask;
    RW_MATH_VPU_IsGreater(magnitudeSufficientMask, magnitude, minMagnitude);
    result.mV = RW_MATH_VPU_Select_R( zero, normalized, magnitudeSufficientMask );
    VectorIntrinsic magResult;
    RW_MATH_VPU_And( magResult, magnitude, magnitudeSufficientMask );
    return VecFloat(magResult);
}

RW_MATH_FORCE_INLINE Vector4 Lerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic ret;
    RW_MATH_VPU_Lerp(ret, v0.mV, v1.mV, t.mV);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4 NLerpEstimate(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV4(ret, lerp);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4 NLerpFast(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV4(ret, lerp);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4 NLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV4(ret, lerp);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE void SetXYZ(Vector4& v, VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_CreateVectorAxisA4_I( result, x.mV, VectorAxisX, y.mV, VectorAxisY, z.mV, VectorAxisZ, v.mV, VectorAxisW );
    v = Vector4(result);
}

template<VectorAxis INDEX0, VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE void SetXYZ(Vector4& v,   VecFloatRef<INDEX0> x,     VecFloatRef<INDEX1> y, VecFloatRef<INDEX2> z) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_CreateVectorAxisA4_I( result, x.mV, INDEX0, y.mV, INDEX1, z.mV, INDEX2, v.mV, VectorAxisW );
    v = Vector4(result);
}

RW_MATH_FORCE_INLINE void SetXYZ(Vector4& v,   VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z) 
{
    VectorIntrinsic result = v.mV;
    RW_MATH_VPU_SwizzleStore_I_V( result, result, VectorAxisX, x.mV, x.mIndex );
    RW_MATH_VPU_SwizzleStore_I_V( result, result, VectorAxisY, y.mV, y.mIndex );
    RW_MATH_VPU_SwizzleStore_I_V( result, result, VectorAxisZ, z.mV, z.mIndex );
    v = Vector4(result);
}

RW_MATH_FORCE_INLINE Vector3 GetXYZ(Vector4::InParam v) 
{
    return Vector3(v.mV);
}

RW_MATH_FORCE_INLINE Vector2 GetXY(Vector4::InParam v) 
{
    return Vector2(v.mV);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeXYZ(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootDotV3(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeSquaredXYZ(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV3(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeRecipXYZ(Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalSquareRootDotV3(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat DotXYZ(Vector4::InParam v0,     Vector4::InParam v1) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV3(ret, v0.mV, v1.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE Vector4 CrossXYZ(Vector4::InParam v0,     Vector4::InParam v1) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_CrossV3(ret, v0.mV, v1.mV);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, VecFloat::InParam a) 
{
    RW_MATH_VPU_Mult( v.mV, v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, VecFloat::InParam a) 
{
    RW_MATH_VPU_Divide( v.mV, v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Add_R(v.mV, a.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, a.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector4  operator * (Vector4::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4  operator * (VecFloat::InParam s, Vector4::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, s.mV, v.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4  operator / (Vector4::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide( ret, v.mV, s.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4  operator + (Vector4::InParam v, VecFloat::InParam s) 
{
    return Vector4(RW_MATH_VPU_Add_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector4  operator - (Vector4::InParam v, VecFloat::InParam s) 
{
    return Vector4(RW_MATH_VPU_Subtract_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector4  operator + (VecFloat::InParam s, Vector4::InParam v) 
{
    return Vector4(RW_MATH_VPU_Add_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector4  operator - (VecFloat::InParam s, Vector4::InParam v) 
{
    return Vector4(RW_MATH_VPU_Subtract_R(s.mV, v.mV));
}

// float
RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, float a) 
{
    RW_MATH_VPU_MultFloat( v.mV, v.mV, a );
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, float a) 
{
    RW_MATH_VPU_DivideFloat( v.mV, v.mV, a );
    return v;
}

RW_MATH_FORCE_INLINE Vector4  operator * (Vector4::InParam v, float s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultFloat( result, v.mV, s );
    return Vector4( result );
}

RW_MATH_FORCE_INLINE Vector4  operator * (float s, Vector4::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultFloat( result, v.mV, s );
    return Vector4( result );
}

RW_MATH_FORCE_INLINE Vector4  operator / (Vector4::InParam v, float s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideFloat( result, v.mV, s );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4  operator + (Vector4::InParam v, float s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_CreateVectorV1( sVI, s );
    return Vector4(RW_MATH_VPU_Add_R(v.mV, sVI));
}

RW_MATH_FORCE_INLINE Vector4  operator - (Vector4::InParam v, float s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_CreateVectorV1( sVI, s );
    return Vector4(RW_MATH_VPU_Subtract_R(v.mV, sVI));
}

RW_MATH_FORCE_INLINE Vector4  operator + (float s, Vector4::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_CreateVectorV1( sVI, s );
    return Vector4(RW_MATH_VPU_Add_R(v.mV, sVI));
}

RW_MATH_FORCE_INLINE Vector4  operator - (float s, Vector4::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_CreateVectorV1( sVI, s );
    return Vector4(RW_MATH_VPU_Subtract_R(sVI, v.mV));
}

// VecFloatRef
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, VecFloatRef<INDEX> a) 
{
    RW_MATH_VPU_MultAxisA1_I( v.mV, v.mV, a.mV, INDEX );
    return v;
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, VecFloatRef<INDEX> a) 
{
    RW_MATH_VPU_DivideAxisA1_I( v.mV, v.mV, a.mV, INDEX );
    return v;
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, VecFloatRef<INDEX> a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I( aVI, a.mV, INDEX );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, VecFloatRef<INDEX> a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I( aVI, a.mV, INDEX );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator *  (Vector4::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector4(result);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator *  (VecFloatRef<INDEX> s, Vector4::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector4(result);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator /  (Vector4::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector4(result);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator +  (Vector4::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector4(ret);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator -  (Vector4::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Vector4(ret);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator +  (VecFloatRef<INDEX> s, Vector4::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector4(ret);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector4  operator -  (VecFloatRef<INDEX> s, Vector4::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Vector4(ret);
}

// VecFloatRefIndex
RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Mult( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Divide( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE Vector4 operator * (Vector4::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4  operator * (VecFloatRefIndex::InParam s, Vector4::InParam v) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, sVI, v.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4  operator / (Vector4::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4  operator + (Vector4::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4  operator - (Vector4::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4  operator + (VecFloatRefIndex::InParam s, Vector4::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector4(ret);
}

RW_MATH_FORCE_INLINE Vector4  operator - (VecFloatRefIndex::InParam s, Vector4::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Vector4(ret);
}

//
// vector operations
//

RW_MATH_FORCE_INLINE Vector4 operator - (Vector4::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Negate(result, v.mV);
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4& operator += (Vector4& v, Vector4::InParam b) 
{
    v.mV = RW_MATH_VPU_Add_R(v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator -= (Vector4& v, Vector4::InParam b) 
{
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator *= (Vector4& v, Vector4::InParam b) 
{
    RW_MATH_VPU_Mult(v.mV, v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector4& operator /= (Vector4& v, Vector4::InParam b) 
{
    RW_MATH_VPU_Divide(v.mV, v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector4 operator - (Vector4::InParam a,Vector4::InParam b) 
{
    return Vector4(RW_MATH_VPU_Subtract_R(a.mV, b.mV));
}

RW_MATH_FORCE_INLINE Vector4 operator + (Vector4::InParam a, Vector4::InParam b) 
{
    return Vector4(RW_MATH_VPU_Add_R(a.mV, b.mV));
}

template<int PermConstant>
RW_MATH_FORCE_INLINE Vector4 Permute(Vector4::InParam v)
{
    (void)rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
    VectorIntrinsic result;
    RW_MATH_VPU_PermuteSingle_I( result, v.mV, PermConstant );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompLessThan(Vector4::InParam a, Vector4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, b.mV, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompGreaterThan(Vector4::InParam a, Vector4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompLessEqual(Vector4::InParam a, Vector4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, b.mV, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompGreaterEqual(Vector4::InParam a, Vector4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompEqual(Vector4::InParam a, Vector4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompNotEqual(Vector4::InParam a, Vector4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Vector4 Select(MaskScalar::InParam mask, Vector4::InParam trueValue, Vector4::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Select(Mask4::InParam mask, Vector4::InParam trueValue, Vector4::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE VecFloat MaxComponent(Vector4::InParam v)
{
	VectorIntrinsic v0, v1;

	RW_MATH_VPU_Splat_I(v0, v.mV, VectorAxisX);
	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisY);
    RW_MATH_VPU_Max(v0, v0, v1);

	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisZ);
	RW_MATH_VPU_Max(v0, v0, v1);

	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisW);
	RW_MATH_VPU_Max(v0, v0, v1);

	return VecFloat(v0);
}

RW_MATH_FORCE_INLINE VecFloat MinComponent(Vector4::InParam v)
{
	VectorIntrinsic v0, v1;

	RW_MATH_VPU_Splat_I(v0, v.mV, VectorAxisX);
	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisY);
    RW_MATH_VPU_Min(v0, v0, v1);

	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisZ);
	RW_MATH_VPU_Min(v0, v0, v1);

	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisW);
	RW_MATH_VPU_Min(v0, v0, v1);

	return VecFloat(v0);
}

RW_MATH_FORCE_INLINE Vector4 SqrtFast(Vector4::InParam a)
{
	VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFast( ret, a.mV );
	return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 InvSqrtFast(Vector4::InParam a)
{
	VectorIntrinsic ret;
	RW_MATH_VPU_ReciprocalSquareRootFast( ret, a.mV );
	return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 ReciprocalFast(Vector4::InParam a)
{
	VectorIntrinsic ret;
	RW_MATH_VPU_ReciprocalFast( ret, a.mV );
    return Vector4( ret );
}

RW_MATH_FORCE_INLINE Vector4 RotateLeft(Vector4::InParam a)
{
	VectorIntrinsic result;
    RW_MATH_VPU_V4_RotateLeft( result, a.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 RotateRight(Vector4::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V4_RotateRight( result, a.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 LoadUnAlignedPacked(const rw::math::fpu::Vector4 &value)
{
    VectorIntrinsic result;
    RW_MATH_VPU_UnalignedLoad(result, reinterpret_cast<const float*>((&value)));
    return Vector4(result);
}

RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Vector4 &destination, Vector4::InParam value)
{
    RW_MATH_VPU_UnalignedStore4(destination, value.mV);
}

// functions return vector by reference
#if RW_MATH_ENABLE_DEPRECATED_API

RW_MATH_FORCE_INLINE void Negate(Vector4::InParam v, Vector4& result) 
{
    result = Negate(v);
}

RW_MATH_FORCE_INLINE void Mult(Vector4::InParam v, VecFloat::InParam s, Vector4& result) 
{
    result = Mult(v, s);
}

template<VectorAxis INDEX>
void Mult(Vector4::InParam v, VecFloatRef<INDEX> s, Vector4& result)
{
    result = Mult(v, s);
}

RW_MATH_FORCE_INLINE void Mult(Vector4::InParam v, VecFloatRefIndex::InParam s, Vector4& result) 
{
    result = Mult(v, s);
}

RW_MATH_FORCE_INLINE void Divide(Vector4::InParam v, VecFloat::InParam s, Vector4& result) 
{
    result = Divide(v, s);
}

RW_MATH_FORCE_INLINE void Add(Vector4::InParam a, Vector4::InParam b, Vector4& result) 
{
    result = Add(a,b);
}

RW_MATH_FORCE_INLINE void Subtract(Vector4::InParam a, Vector4::InParam b, Vector4& result) 
{
    result = Subtract(a, b);
}

RW_MATH_FORCE_INLINE void Mult(Vector4::InParam a, Vector4::InParam b, Vector4& result) 
{
    result = Mult(a, b);
}

RW_MATH_FORCE_INLINE void DotXYZ(Vector4::InParam v0,     Vector4::InParam v1, VecFloat& result) 
{
    RW_MATH_VPU_DotV3(result.mV, v0.mV, v1.mV);
}

RW_MATH_FORCE_INLINE void CrossXYZ(Vector4::InParam v0,     Vector4::InParam v1, Vector4& result) 
{
    result = CrossXYZ( v0, v1 );
}

RW_MATH_FORCE_INLINE void Normalize(Vector4::InParam v, Vector4& result) 
{
    result = Normalize(v);
}

RW_MATH_FORCE_INLINE void Lerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t, Vector4& result) 
{
    result = Lerp( v0, v1, t );
}

RW_MATH_FORCE_INLINE void SLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t, Vector4& result) 
{
    result = SLerp( v0, v1, t );
}

RW_MATH_FORCE_INLINE void NLerp(Vector4::InParam v0, Vector4::InParam v1, VecFloat::InParam t, Vector4& result) 
{
    result = NLerp( v0, v1, t );
}

RW_MATH_FORCE_INLINE void Dot(Vector4::InParam a, Vector4::InParam b, VecFloat& r) 
{
    RW_MATH_VPU_DotV4(r.mV, b.mV, a.mV);
}

#endif

} // namespace vpu
} // namespace math
} // namespace rw

#endif
