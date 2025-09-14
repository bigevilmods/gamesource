// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR2_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR2_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/static_asserts.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/trig.h"

#include "rw/math/vpu/vector_intrinsic_operation.h"

#include "rw/math/vpu/detail/vector2_operation_platform_inline.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE Vector2 Vector2FromFloat(float xy)
{
    return Vector2(xy, xy);
}

RW_MATH_FORCE_INLINE bool IsSimilar(Vector2::InParam a, Vector2::InParam b, VecFloat::InParam tolerance) 
{
    return _asmIsSimilarV2(a.mV, b.mV, tolerance.mV);
}

// basic functions
RW_MATH_FORCE_INLINE Vector2 Negate(Vector2::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Negate(result, v.mV);
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV );
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, float s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultFloat( result, v.mV, s );
    return Vector2( result );
}

template<VectorAxis INDEX> Vector2 Mult(Vector2::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Divide( result, v.mV, s.mV );
    return Vector2( result );
}

RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, float s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideFloat( ret, v.mV, s );
    return Vector2( ret );
}

template<VectorAxis INDEX> Vector2 Divide(Vector2::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Add(Vector2::InParam a, Vector2::InParam b) 
{
    return Vector2( RW_MATH_VPU_Add_R( a.mV, b.mV ) );
}

RW_MATH_FORCE_INLINE Vector2 Subtract(Vector2::InParam a, Vector2::InParam b) 
{
    return Vector2( RW_MATH_VPU_Subtract_R( a.mV, b.mV ) );
}

RW_MATH_FORCE_INLINE VecFloat Dot(Vector2::InParam a, Vector2::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV2(ret, a.mV, b.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Cross(Vector2::InParam a, Vector2::InParam b) 
{
    VectorIntrinsic a_x, a_y, b_x, b_y, prod1, prod2;
    RW_MATH_VPU_Splat_I(a_x, a.mV, VectorAxisX);
    RW_MATH_VPU_Splat_I(a_y, a.mV, VectorAxisY);
    RW_MATH_VPU_Splat_I(b_x, b.mV, VectorAxisX);
    RW_MATH_VPU_Splat_I(b_y, b.mV, VectorAxisY);
    RW_MATH_VPU_Mult( prod1, a_x, b_y );
    RW_MATH_VPU_Mult( prod2, a_y, b_x );
    return VecFloat(RW_MATH_VPU_Subtract_R( prod1, prod2 ));
}

RW_MATH_FORCE_INLINE Vector2 Mult(Vector2::InParam a, Vector2::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret,a.mV, b.mV);
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 Divide(Vector2::InParam a, Vector2::InParam b) 
{
    VectorIntrinsic ret;
#ifdef RWCROSS
    RW_MATH_VPU_Divide_V2( ret,a.mV, b.mV);
#else
    RW_MATH_VPU_Divide( ret,a.mV, b.mV);
#endif
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 Reciprocal(Vector2::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Reciprocal( ret, a.mV );
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 operator * (Vector2::InParam a, Vector2::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret,a.mV, b.mV);
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 operator / (Vector2::InParam a, Vector2::InParam b) 
{
    VectorIntrinsic ret;
#ifdef RWCROSS
    RW_MATH_VPU_Divide_V2( ret,a.mV, b.mV);
#else
    RW_MATH_VPU_Divide( ret,a.mV, b.mV);
#endif
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 MultAdd(Vector2::InParam a, Vector2::InParam b, Vector2::InParam c) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultAdd( ret, a.mV, b.mV, c.mV );
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE VecFloat Magnitude(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootDotV2(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeFast(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFastDotV2(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeEstimate(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootEstimateDotV2(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV2(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalSquareRootDotV2(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE Vector2 NormalizeEstimate(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV2(ret, v.mV);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 NormalizeFast(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV2(ret, v.mV);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 Normalize(Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV2(ret, v.mV);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 Normalize(Vector2::InParam v, VecFloat& magnitude) 
{
    VectorIntrinsic normalized;
    RW_MATH_VPU_NormalizeWithMagnitudeV2(normalized, magnitude.mV, v.mV);
    return Vector2(normalized);
}

RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Vector2::InParam v, Vector2& result) 
{
    VectorIntrinsic magnitude;
    RW_MATH_VPU_NormalizeWithMagnitudeV2(result.mV, magnitude, v.mV);
    return VecFloat(magnitude);
}

RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Vector2::InParam v, Vector2& result)
{
    VectorIntrinsic normalized;
    VectorIntrinsic magnitude;
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic minMagnitude = zero;
    RW_MATH_VPU_NormalizeWithMagnitudeV2(normalized, magnitude, v.mV);
    VectorIntrinsic magnitudeSufficientMask;
    RW_MATH_VPU_IsGreater(magnitudeSufficientMask, magnitude, minMagnitude);
    result.mV = RW_MATH_VPU_Select_R( zero, normalized, magnitudeSufficientMask );
    VectorIntrinsic magResult;
    RW_MATH_VPU_And( magResult, magnitude, magnitudeSufficientMask );
    return VecFloat(magResult);
}

RW_MATH_FORCE_INLINE VecFloat CosAngleBetweenNormalizedVectors(Vector2::InParam v0, Vector2::InParam v1) 
{
    // TODO optimize
    VecFloat one(RW_MATH_VPU_GetOne_R());
    return Clamp(Dot(v0, v1), Negate(one), one);
}

RW_MATH_FORCE_INLINE Vector2 Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic ret;
    RW_MATH_VPU_Lerp(ret, v0.mV, v1.mV, t.mV);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 NLerpEstimate(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV2(ret, lerp);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 NLerpFast(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV2(ret, lerp);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV2(ret, lerp);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE bool IsValid(Vector2::InParam v) 
{
    return (IsValid(v.X()) && IsValid(v.Y()));
}

RW_MATH_FORCE_INLINE bool IsZero(Vector2::InParam v, VecFloat::InParam tolerance) 
{
    return (IsZero(v.X(), tolerance) && IsZero(v.Y(), tolerance));
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Vector2::InParam v) 
{
    bool equal;
    RW_MATH_VPU_IsEqualV2(equal, v.mV, v.mV);
    return !equal;
}

RW_MATH_FORCE_INLINE Vector2 Min(Vector2::InParam v0, Vector2::InParam v1) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Min(result, v0.mV, v1.mV);
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Max(Vector2::InParam v0, Vector2::InParam v1) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Max(result, v0.mV, v1.mV);
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Clamp(Vector2::InParam v0, Vector2::InParam v1, Vector2::InParam v2)
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    RW_MATH_VPU_Min(r, r, v2.mV);
    return Vector2(r);
}

RW_MATH_FORCE_INLINE Vector2 ClampToNegPosRange(Vector2::InParam value, Vector2::InParam range)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ClampToNegPosRange( result, value.mV, range.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Abs(Vector2::InParam v) 
{
    return Max(v, Negate(v));
}

RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectors(Vector2::InParam v0, Vector2::InParam v1) 
{
    // TODO optimize
    Vector2 nv0, nv1;
    nv0 = Normalize(v0);
    nv1 = Normalize(v1);
    return ACos(CosAngleBetweenNormalizedVectors(nv0, nv1));
}

RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectorsFast(Vector2::InParam v0, Vector2::InParam v1) 
{
    // TODO optimize
    Vector2 nv0, nv1;
    nv0 = NormalizeFast(v0);
    nv1 = NormalizeFast(v1);
    return ACos(CosAngleBetweenNormalizedVectors(nv0, nv1));
}

RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, VecFloat::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, a.mV);
    v.mV = ret;
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Divide( result, v.mV, a.mV );
    v.mV = result;
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Add_R( v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Subtract_R( v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector2  operator * (Vector2::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2  operator * (VecFloat::InParam s, Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2  operator / (Vector2::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Divide( result, v.mV, s.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2  operator + (Vector2::InParam v, VecFloat::InParam s) 
{
    return Vector2(RW_MATH_VPU_Add_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector2  operator - (Vector2::InParam v, VecFloat::InParam s) 
{
    return Vector2(RW_MATH_VPU_Subtract_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector2  operator + (VecFloat::InParam s, Vector2::InParam v) 
{
    return Vector2(RW_MATH_VPU_Add_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector2  operator - (VecFloat::InParam s, Vector2::InParam v) 
{
    return Vector2(RW_MATH_VPU_Subtract_R(s.mV, v.mV));
}

// float
RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, float a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(a));
    v.mV = ret;
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, float a) 
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    divisor = RW_MATH_VPU_CreateVectorFromScalar_R(a);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    v.mV = result;
    return v;
}

RW_MATH_FORCE_INLINE Vector2  operator * (Vector2::InParam v, float s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2  operator * (float s, Vector2::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2  operator / (Vector2::InParam v, float s) 
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    divisor = RW_MATH_VPU_CreateVectorFromScalar_R(s);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2  operator + (Vector2::InParam v, float s) 
{
    return Vector2(RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

RW_MATH_FORCE_INLINE Vector2  operator - (Vector2::InParam v, float s) 
{
    return Vector2(RW_MATH_VPU_Subtract_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

RW_MATH_FORCE_INLINE Vector2  operator + (float s, Vector2::InParam v) 
{
    return Vector2(RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

RW_MATH_FORCE_INLINE Vector2  operator - (float s, Vector2::InParam v) 
{
    return Vector2(RW_MATH_VPU_Subtract_R(RW_MATH_VPU_CreateVectorFromScalar_R(s), v.mV));
}


// VecFloatRef
template<VectorAxis INDEX> Vector2& operator *= (Vector2& v, VecFloatRef<INDEX> a)
{
    VectorIntrinsic ret;
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I(aVI, a.mV, INDEX);
    RW_MATH_VPU_Mult( ret, v.mV, aVI);
    v.mV = ret;
    return v;
}

template<VectorAxis INDEX> Vector2& operator /= (Vector2& v, VecFloatRef<INDEX> a)
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    RW_MATH_VPU_Splat_I(divisor, a.mV, INDEX);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    v.mV = result;
    return v;
}

template<VectorAxis INDEX> Vector2& operator += (Vector2& v, VecFloatRef<INDEX> a)
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I(aVI, a.mV, INDEX);
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX> Vector2& operator -= (Vector2& v, VecFloatRef<INDEX> a)
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I(aVI, a.mV, INDEX);
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX> Vector2  operator *  (Vector2::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic ret;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I(sVI, s.mV, INDEX);
    RW_MATH_VPU_Mult( ret, v.mV, sVI);
    return Vector2(ret);
}

template<VectorAxis INDEX> Vector2  operator *  (VecFloatRef<INDEX> s, Vector2::InParam v)
{
    VectorIntrinsic ret;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I(sVI, s.mV, INDEX);
    RW_MATH_VPU_Mult( ret, v.mV, sVI);
    return Vector2(ret);
}

template<VectorAxis INDEX> Vector2  operator /  (Vector2::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    RW_MATH_VPU_Splat_I(divisor, s.mV, INDEX);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    return Vector2(result);
}

template<VectorAxis INDEX> Vector2  operator +  (Vector2::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I(sVI, s.mV, INDEX);
    return Vector2(RW_MATH_VPU_Add_R(v.mV, sVI));
}

template<VectorAxis INDEX> Vector2  operator -  (Vector2::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I(sVI, s.mV, INDEX);
    return Vector2(RW_MATH_VPU_Subtract_R(v.mV, sVI));
}

template<VectorAxis INDEX> Vector2  operator +  (VecFloatRef<INDEX> s, Vector2::InParam v)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I(sVI, s.mV, INDEX);
    return Vector2(RW_MATH_VPU_Add_R(v.mV, sVI));
}

template<VectorAxis INDEX> Vector2  operator -  (VecFloatRef<INDEX> s, Vector2::InParam v)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I(sVI, s.mV, INDEX);
    return Vector2(RW_MATH_VPU_Subtract_R(sVI, v.mV));
}

// VecFloatRefIndex
RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Mult( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Divide( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE Vector2 operator * (Vector2::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2  operator * (VecFloatRefIndex::InParam s, Vector2::InParam v) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, sVI, v.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2  operator / (Vector2::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 operator + (Vector2::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2  operator - (Vector2::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2 operator + (VecFloatRefIndex::InParam s, Vector2::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector2(ret);
}

RW_MATH_FORCE_INLINE Vector2  operator - (VecFloatRefIndex::InParam s, Vector2::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Vector2(ret);
}

//
// vector operations
//

RW_MATH_FORCE_INLINE Vector2 operator - (Vector2::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Negate(result, v.mV);
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2& operator += (Vector2& v, Vector2::InParam b) 
{
    v.mV = RW_MATH_VPU_Add_R(v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator *= (Vector2& v, Vector2::InParam b) 
{
    RW_MATH_VPU_Mult(v.mV, v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator /= (Vector2& v, Vector2::InParam b) 
{
    RW_MATH_VPU_Divide(v.mV, v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector2& operator -= (Vector2& v, Vector2::InParam b) 
{
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector2 operator - (Vector2::InParam a,Vector2::InParam b) 
{
    return Vector2(RW_MATH_VPU_Subtract_R(a.mV, b.mV));
}

RW_MATH_FORCE_INLINE Vector2 operator + (Vector2::InParam a, Vector2::InParam b) 
{
    return Vector2(RW_MATH_VPU_Add_R(a.mV, b.mV));
}

RW_MATH_FORCE_INLINE bool operator == (Vector2::InParam a, Vector2::InParam b) 
{
    return  _asmIsEqualV2(a.mV, b.mV);
}

RW_MATH_FORCE_INLINE bool operator != (Vector2::InParam a, Vector2::InParam b) 
{
    return  !_asmIsEqualV2(a.mV, b.mV);
}

template<int PermConstant>
RW_MATH_FORCE_INLINE Vector2 Permute(Vector2::InParam v)
{
    (void)rw::math::detail::Assert_ValidVector2PermConstant<PermConstant>();
    VectorIntrinsic result;
    RW_MATH_VPU_PermuteSingle_I( result, v.mV, (PermConstant<<8) );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Mask2   CompLessThan(Vector2::InParam a, Vector2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, b.mV, a.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2   CompGreaterThan(Vector2::InParam a, Vector2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2   CompLessEqual(Vector2::InParam a, Vector2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, b.mV, a.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2   CompGreaterEqual(Vector2::InParam a, Vector2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2   CompEqual(Vector2::InParam a, Vector2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2   CompNotEqual(Vector2::InParam a, Vector2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Vector2 Select(MaskScalar::InParam mask, Vector2::InParam trueValue, Vector2::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 Select(Mask2::InParam mask, Vector2::InParam trueValue, Vector2::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE VecFloat MaxComponent(Vector2::InParam v)
{
    VectorIntrinsic v0, v1;
	RW_MATH_VPU_Splat_I(v0, v.mV, VectorAxisX);
	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisY);

    RW_MATH_VPU_Max(v0, v0, v1);
	return VecFloat(v0);
}

RW_MATH_FORCE_INLINE VecFloat MinComponent(Vector2::InParam v)
{
	VectorIntrinsic v0, v1;
	RW_MATH_VPU_Splat_I(v0, v.mV, VectorAxisX);
	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisY);

    RW_MATH_VPU_Min(v0, v0, v1);
	return VecFloat(v0);
}

RW_MATH_FORCE_INLINE Vector2 SqrtFast(Vector2::InParam a)
{
	VectorIntrinsic ret;
#ifdef EA_PLATFORM_REVOLUTION
	RW_MATH_VPU_SquareRootFastV2( ret, a.mV );
#else
    RW_MATH_VPU_SquareRootFast( ret, a.mV );
#endif
	return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 InvSqrtFast(Vector2::InParam a)
{
	VectorIntrinsic ret;
#ifdef EA_PLATFORM_REVOLUTION
	RW_MATH_VPU_ReciprocalSquareRootFastV2( ret, a.mV );
#else
	RW_MATH_VPU_ReciprocalSquareRootFast( ret, a.mV );
#endif
	return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 ReciprocalFast(Vector2::InParam a)
{
	VectorIntrinsic ret;
#ifdef EA_PLATFORM_REVOLUTION
    RW_MATH_VPU_ReciprocalFastV2( ret, a.mV );
#else
	RW_MATH_VPU_ReciprocalFast( ret, a.mV );
#endif
    return Vector2( ret );
}

RW_MATH_FORCE_INLINE Vector2 RotateLeft(Vector2::InParam a)
{
	VectorIntrinsic result;
    RW_MATH_VPU_V2_RotateLeft( result, a.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 RotateRight(Vector2::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V2_RotateLeft( result, a.mV );
    return Vector2(result);
}

RW_MATH_FORCE_INLINE Vector2 LoadUnAlignedPacked(const rw::math::fpu::Vector2 &value)
{
    VectorIntrinsic result;
    RW_MATH_VPU_UnalignedLoad(result, reinterpret_cast<const float*>((&value)));
    return Vector2(result);
}

RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Vector2 &destination, Vector2::InParam value)
{
    RW_MATH_VPU_UnalignedStore2(destination, value.mV);
}


// functions return vector by reference
#if RW_MATH_ENABLE_DEPRECATED_API
RW_MATH_FORCE_INLINE void Negate(Vector2::InParam vec, Vector2& result) 
{
    result = Negate(vec);
}

RW_MATH_FORCE_INLINE void Mult(Vector2::InParam vec, VecFloat::InParam scalar, Vector2& result) 
{
    result = Mult(vec, scalar);
}

template<VectorAxis INDEX> void Mult(Vector2::InParam vec, VecFloatRef<INDEX> scalar, Vector2& result)
{
    result = Mult(vec, scalar);
}

RW_MATH_FORCE_INLINE void Mult(Vector2::InParam vec, VecFloatRefIndex::InParam scalar, Vector2& result) 
{
    result = Mult(vec, scalar);
}

RW_MATH_FORCE_INLINE void Divide(Vector2::InParam v, VecFloat::InParam s, Vector2& result) 
{
    result = Divide(v, s);
}

template<VectorAxis INDEX> void Divide(Vector2::InParam v, VecFloatRef<INDEX> s, Vector2& result)
{
    result = Divide(v, s);
}

RW_MATH_FORCE_INLINE void Divide(Vector2::InParam v, VecFloatRefIndex::InParam s, Vector2& result) 
{
    result = Divide(v, s);
}

RW_MATH_FORCE_INLINE void Add(Vector2::InParam a, Vector2::InParam b, Vector2& result) 
{
    result = Add(a, b);
}

RW_MATH_FORCE_INLINE void Subtract(Vector2::InParam a, Vector2::InParam b, Vector2& result) 
{
    result = Subtract(a, b);
}

RW_MATH_FORCE_INLINE void Mult(Vector2::InParam a, Vector2::InParam b, Vector2& result) 
{
    result = Mult(a, b);
}

RW_MATH_FORCE_INLINE void Normalize(Vector2::InParam v, Vector2& result) 
{
    result = Normalize(v);
}

RW_MATH_FORCE_INLINE void Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t, Vector2& result) 
{
    result = Lerp( v0, v1, t );
}

template<VectorAxis INDEX> void Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRef<INDEX> t, Vector2& result)
{
    result = Lerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void Lerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRefIndex::InParam t, Vector2& result) 
{
    result = Lerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t, Vector2& result) 
{
    result = SLerp( v0, v1, t );
}

template<VectorAxis INDEX> void SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRef<INDEX> t, Vector2& result)
{
    result = SLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void SLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRefIndex::InParam t, Vector2& result) 
{
    result = SLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloat::InParam t, Vector2& result) 
{
    result = NLerp( v0, v1, t );
}

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRef<INDEX> t, Vector2& result) 
{
    result = NLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void NLerp(Vector2::InParam v0, Vector2::InParam v1, VecFloatRefIndex::InParam t, Vector2& result) 
{
    result = NLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void Dot(Vector2::InParam a, Vector2::InParam b, VecFloat& r) 
{
    RW_MATH_VPU_DotV2(r.mV, a.mV, b.mV);
}

#endif

}

}

}

#endif
