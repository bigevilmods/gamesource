// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR3_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR3_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/static_asserts.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/trig.h"

#include "rw/math/vpu/vector_intrinsic_operation.h"

#include "rw/math/vpu/detail/vector3_operation_platform_inline.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE Vector3 Vector3FromFloat(float xyz)
{
    return Vector3(xyz, xyz, xyz);
}

RW_MATH_FORCE_INLINE bool operator == (Vector3::InParam a, Vector3::InParam b) 
{
    return  _asmIsEqualV3(a.mV, b.mV);
}

RW_MATH_FORCE_INLINE bool operator != (Vector3::InParam a, Vector3::InParam b) 
{
    return  !_asmIsEqualV3(a.mV, b.mV);
}

RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV )
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, float s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultFloat( result, v.mV, s );
    return Vector3( result );
}

template<VectorAxis INDEX> Vector3 Mult(Vector3::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Divide( result, v.mV, s.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, float s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideFloat( ret, v.mV, s );
    return Vector3( ret );
}

template<VectorAxis INDEX> Vector3 Divide(Vector3::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideAxisA1_I(result, v.mV, s.mV, INDEX);
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Add(Vector3::InParam a, Vector3::InParam b) 
{
    return Vector3( RW_MATH_VPU_Add_R( a.mV, b.mV ) );
}

RW_MATH_FORCE_INLINE Vector3 Subtract(Vector3::InParam a, Vector3::InParam b) 
{
    return Vector3( RW_MATH_VPU_Subtract_R( a.mV, b.mV ) );
}

RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam a, Vector3::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 Divide(Vector3::InParam a, Vector3::InParam b) 
{
    VectorIntrinsic ret;
#ifdef RWCROSS
    RW_MATH_VPU_Divide_V3(ret, a.mV, b.mV);
#else
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
#endif
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 Reciprocal(Vector3::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Reciprocal( ret, a.mV )
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 operator * (Vector3::InParam a, Vector3::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 operator / (Vector3::InParam a, Vector3::InParam b) 
{
    VectorIntrinsic ret;
#ifdef RWCROSS
    RW_MATH_VPU_Divide_V3(ret, a.mV, b.mV);
#else
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
#endif
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 MultAdd(Vector3::InParam a, Vector3::InParam b, Vector3::InParam c) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultAdd(ret, a.mV, b.mV, c.mV);
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE VecFloat Dot(Vector3::InParam a, Vector3::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV3(ret, a.mV, b.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE Vector3 Cross(Vector3::InParam a, Vector3::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_CrossV3(ret, a.mV, b.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE VecFloat Magnitude(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootDotV3(ret, v.mV, v.mV)
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeFast(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFastDotV3(ret, v.mV, v.mV)
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeEstimate(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootEstimateDotV3(ret, v.mV, v.mV)
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeSquared(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV3(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat MagnitudeRecip(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalSquareRootDotV3(ret, v.mV, v.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE Vector3 NormalizeEstimate(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV3(ret, v.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 NormalizeFast(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV3(ret, v.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 Normalize(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV3(ret, v.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 Normalize(Vector3::InParam v, VecFloat& magnitude) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeWithMagnitudeV3(ret, magnitude.mV, v.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE VecFloat NormalizeReturnMagnitude(Vector3::InParam v, Vector3& result) 
{
    VectorIntrinsic magnitude;
    RW_MATH_VPU_NormalizeWithMagnitudeV3(result.mV, magnitude, v.mV);
    return VecFloat(magnitude);
}

RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Vector3::InParam v, Vector3& result)
{
    VectorIntrinsic normalized;
    VectorIntrinsic magnitude;
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic minMagnitude = zero;
    RW_MATH_VPU_NormalizeWithMagnitudeV3(normalized, magnitude, v.mV);
    VectorIntrinsic magnitudeSufficientMask;
    RW_MATH_VPU_IsGreater(magnitudeSufficientMask, magnitude, minMagnitude);
    result.mV = RW_MATH_VPU_Select_R( zero, normalized, magnitudeSufficientMask );
    VectorIntrinsic magResult;
    RW_MATH_VPU_And( magResult, magnitude, magnitudeSufficientMask );
    return VecFloat(magResult);
}

RW_MATH_FORCE_INLINE VecFloat CosAngleBetweenNormalizedVectors(Vector3::InParam v0, Vector3::InParam v1) 
{
    VectorIntrinsic dotProduct, temp0, result;
    VectorIntrinsic one, negOne;
    RW_MATH_VPU_GetOne(one);
    RW_MATH_VPU_GetNegOne(negOne);
    RW_MATH_VPU_DotV3( dotProduct, v0.mV, v1.mV );
    RW_MATH_VPU_Max( temp0, dotProduct, negOne );
    RW_MATH_VPU_Min( result, temp0, one );
    return VecFloat( result );
}

RW_MATH_FORCE_INLINE Vector3 Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic ret;
    RW_MATH_VPU_Lerp(ret, v0.mV, v1.mV, t.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 NLerpEstimate(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV3(ret, lerp);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 NLerpFast(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV3(ret, lerp);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds"));
    VectorIntrinsic lerp;
    RW_MATH_VPU_Lerp(lerp, v0.mV, v1.mV, t.mV);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV3(ret, lerp);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector2 GetXY(Vector3::InParam v)
{
    return Vector2(v.mV);
}

RW_MATH_FORCE_INLINE bool IsValid(Vector3::InParam v) 
{
    return (IsValid(v.X()) && IsValid(v.Y()) && IsValid(v.Z()));
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Vector3::InParam v) 
{
    bool equal;
    RW_MATH_VPU_IsEqualV3(equal, v.mV, v.mV);
    return !equal;
}

RW_MATH_FORCE_INLINE Vector3 Negate(Vector3::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Negate(result, v.mV);
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Min(Vector3::InParam v0, Vector3::InParam v1) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Min(result, v0.mV, v1.mV);
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Max(Vector3::InParam v0, Vector3::InParam v1) 
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    return Vector3(r);
}

RW_MATH_FORCE_INLINE Vector3 Clamp(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam v2)
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    RW_MATH_VPU_Min(r, r, v2.mV);
    return Vector3(r);
}

RW_MATH_FORCE_INLINE Vector3 ClampToNegPosRange(Vector3::InParam value, Vector3::InParam range)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ClampToNegPosRange( result, value.mV, range.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Abs(Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Abs(ret, v.mV);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectors(Vector3::InParam v0, Vector3::InParam v1) 
{
    VectorIntrinsic nv0, nv1;
    RW_MATH_VPU_NormalizeFastV3(nv0, v0.mV);
    RW_MATH_VPU_NormalizeFastV3(nv1, v1.mV);
    return ACos(CosAngleBetweenNormalizedVectors(Vector3(nv0), Vector3(nv1)));
}

RW_MATH_FORCE_INLINE VecFloat AngleBetweenVectors(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam windAxis) 
{
    VectorIntrinsic nv0, nv1;
    RW_MATH_VPU_NormalizeFastV3(nv0, v0.mV);
    RW_MATH_VPU_NormalizeFastV3(nv1, v1.mV);
    VecFloat angle = ACos(CosAngleBetweenNormalizedVectors(Vector3(nv0), Vector3(nv1)));
    VectorIntrinsic norm, dotProduct;
    RW_MATH_VPU_CrossV3( norm, nv0, nv1 );
    RW_MATH_VPU_DotV3( dotProduct, norm, windAxis.mV );
    if( VecFloat(dotProduct) < VecFloat(RW_MATH_VPU_GetZero_R()) )
    {
        return(VecFloat(TWO_PI) - angle);
    }
    return(angle);
}

RW_MATH_FORCE_INLINE bool IsZero(Vector3::InParam v, VecFloat::InParam tolerance) 
{
    return _asmIsZeroV3(v.mV, tolerance.mV);
}

RW_MATH_FORCE_INLINE bool IsSimilar(Vector3::InParam a, Vector3::InParam b, VecFloat::InParam epsilon) 
{
    return _asmIsSimilarV3(a.mV, b.mV, epsilon.mV);
}


RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, VecFloat::InParam a) 
{
    RW_MATH_VPU_Mult( v.mV, v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, VecFloat::InParam a) 
{
    RW_MATH_VPU_Divide( v.mV, v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Add_R( v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Subtract_R( v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE Vector3  operator * (Vector3::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV )
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3  operator * (VecFloat::InParam s, Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV )
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3  operator / (Vector3::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide( ret, v.mV, s.mV );
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3  operator + (Vector3::InParam v, VecFloat::InParam s) 
{
    return Vector3(RW_MATH_VPU_Add_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector3  operator - (Vector3::InParam v, VecFloat::InParam s) 
{
    return Vector3(RW_MATH_VPU_Subtract_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector3  operator + (VecFloat::InParam s, Vector3::InParam v) 
{
    return Vector3(RW_MATH_VPU_Add_R(v.mV, s.mV));
}

RW_MATH_FORCE_INLINE Vector3  operator - (VecFloat::InParam s, Vector3::InParam v) 
{
    return Vector3(RW_MATH_VPU_Subtract_R(s.mV, v.mV));
}

// float
RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, float s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    v.mV = ret;
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, float s) 
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    divisor = RW_MATH_VPU_CreateVectorFromScalar_R(s);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    v.mV = result;
    return v;
}

RW_MATH_FORCE_INLINE Vector3  operator * (Vector3::InParam v, float s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3  operator * (float s, Vector3::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3  operator / (Vector3::InParam v, float s) 
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    divisor = RW_MATH_VPU_CreateVectorFromScalar_R(s);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 operator + (Vector3::InParam v, float s) 
{
    return Vector3(RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

RW_MATH_FORCE_INLINE Vector3 operator - (Vector3::InParam v, float s) 
{
    return Vector3(RW_MATH_VPU_Subtract_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

RW_MATH_FORCE_INLINE Vector3 operator + (float s, Vector3::InParam v) 
{
    return Vector3(RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

RW_MATH_FORCE_INLINE Vector3 operator - (float s, Vector3::InParam v) 
{
    return Vector3(RW_MATH_VPU_Subtract_R(RW_MATH_VPU_CreateVectorFromScalar_R(s), v.mV));
}


// VecFloatRef
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, VecFloatRef<INDEX> a) 
{
    RW_MATH_VPU_MultAxisA1_I( v.mV, v.mV, a.mV, INDEX );
    return v;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, VecFloatRef<INDEX> a) 
{
    RW_MATH_VPU_DivideAxisA1_I( v.mV, v.mV, a.mV, INDEX );
    return v;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, VecFloatRef<INDEX> a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I( aVI, a.mV, INDEX );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, VecFloatRef<INDEX> a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I( aVI, a.mV, INDEX );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3  operator *  (Vector3::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector3(result);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3  operator *  (VecFloatRef<INDEX> s, Vector3::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector3(result);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3  operator /  (Vector3::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideAxisA1_I( result, v.mV, s.mV, INDEX );
    return Vector3(result);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3  operator +  (Vector3::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector3(ret);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector3  operator -  (Vector3::InParam v, VecFloatRef<INDEX> s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Vector3(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Vector3  operator +  (VecFloatRef<INDEX> s, Vector3::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector3(ret);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Vector3  operator -  (VecFloatRef<INDEX> s, Vector3::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Vector3(ret);
}

// VecFloatRefIndex
RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Mult( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Divide( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE Vector3 operator * (Vector3::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3  operator * (VecFloatRefIndex::InParam s, Vector3::InParam v) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, sVI, v.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3  operator / (Vector3::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 operator + (Vector3::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3  operator - (Vector3::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3 operator + (VecFloatRefIndex::InParam s, Vector3::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE Vector3  operator - (VecFloatRefIndex::InParam s, Vector3::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Vector3(ret);
}

//
// vector operations
//

RW_MATH_FORCE_INLINE Vector3 operator - (Vector3::InParam v) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Negate(result, v.mV);
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3& operator += (Vector3& v, Vector3::InParam b) 
{
    v.mV = RW_MATH_VPU_Add_R(v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator -= (Vector3& v, Vector3::InParam b) 
{
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator *= (Vector3& v, Vector3::InParam b) 
{
    RW_MATH_VPU_Mult(v.mV, v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector3& operator /= (Vector3& v, Vector3::InParam b) 
{
    RW_MATH_VPU_Divide(v.mV, v.mV, b.mV);
    return v;
}

RW_MATH_FORCE_INLINE Vector3 operator - (Vector3::InParam a,Vector3::InParam b) 
{
    return Vector3(RW_MATH_VPU_Subtract_R(a.mV, b.mV));
}

RW_MATH_FORCE_INLINE Vector3 operator + (Vector3::InParam a, Vector3::InParam b) 
{
    return Vector3( RW_MATH_VPU_Add_R( a.mV, b.mV ) );
}

template<int PermConstant>
RW_MATH_FORCE_INLINE Vector3 Permute(Vector3::InParam v)
{
    (void)rw::math::detail::Assert_ValidVector3PermConstant<PermConstant>();
    VectorIntrinsic result;
    RW_MATH_VPU_PermuteSingle_I( result, v.mV, (PermConstant<<4) );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Mask3   CompLessThan(Vector3::InParam a, Vector3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, b.mV, a.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3   CompGreaterThan(Vector3::InParam a, Vector3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3   CompLessEqual(Vector3::InParam a, Vector3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, b.mV, a.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3   CompGreaterEqual(Vector3::InParam a, Vector3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3   CompEqual(Vector3::InParam a, Vector3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3   CompNotEqual(Vector3::InParam a, Vector3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Vector3 Select(MaskScalar::InParam mask, Vector3::InParam trueValue, Vector3::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Select(Mask3::InParam mask, Vector3::InParam trueValue, Vector3::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE VecFloat MaxComponent(Vector3::InParam v)
{
	VectorIntrinsic v0, v1;

	RW_MATH_VPU_Splat_I(v0, v.mV, VectorAxisX);
	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisY);
    RW_MATH_VPU_Max(v0, v0, v1);

	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisZ);
	RW_MATH_VPU_Max(v0, v0, v1);

	return VecFloat(v0);
}

RW_MATH_FORCE_INLINE VecFloat MinComponent(Vector3::InParam v)
{
	VectorIntrinsic v0, v1;

	RW_MATH_VPU_Splat_I(v0, v.mV, VectorAxisX);
	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisY);
    RW_MATH_VPU_Min(v0, v0, v1);

	RW_MATH_VPU_Splat_I(v1, v.mV, VectorAxisZ);
	RW_MATH_VPU_Min(v0, v0, v1);

	return VecFloat(v0);
}

RW_MATH_FORCE_INLINE Vector3 SqrtFast(Vector3::InParam a)
{
	VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFast( ret, a.mV );
	return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 InvSqrtFast(Vector3::InParam a)
{
	VectorIntrinsic ret;
	RW_MATH_VPU_ReciprocalSquareRootFast( ret, a.mV );
	return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 ReciprocalFast(Vector3::InParam a)
{
	VectorIntrinsic ret;
	RW_MATH_VPU_ReciprocalFast( ret, a.mV );
    return Vector3( ret );
}

RW_MATH_FORCE_INLINE Vector3 RotateLeft(Vector3::InParam a)
{
	VectorIntrinsic result;
    RW_MATH_VPU_V3_RotateLeft( result, a.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 RotateRight(Vector3::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V3_RotateRight( result, a.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 LoadUnAlignedPacked(const rw::math::fpu::Vector3 &value)
{
    VectorIntrinsic result;
    RW_MATH_VPU_UnalignedLoad(result, reinterpret_cast<const float*>((&value)));
    return Vector3(result);
}

RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Vector3 &destination, Vector3::InParam value)
{
    RW_MATH_VPU_UnalignedStore3(destination, value.mV);
}

// functions return vector by reference
#if RW_MATH_ENABLE_DEPRECATED_API
RW_MATH_FORCE_INLINE void Negate(Vector3::InParam vec, Vector3& result) 
{
    result = Negate(vec);
}

RW_MATH_FORCE_INLINE void Mult(Vector3::InParam vec, VecFloat::InParam scalar, Vector3& result) 
{
    result = Mult(vec, scalar);
}

template<VectorAxis INDEX> void Mult(Vector3::InParam vec, VecFloatRef<INDEX> scalar, Vector3& result)
{
    result = Mult(vec, scalar);
}

RW_MATH_FORCE_INLINE void Mult(Vector3::InParam vec, VecFloatRefIndex::InParam scalar, Vector3& result) 
{
    result = Mult(vec, scalar);
}

RW_MATH_FORCE_INLINE void Divide(Vector3::InParam v, VecFloat::InParam s, Vector3& result) 
{
    result = Divide(v, s);
}

template<VectorAxis INDEX> void Divide(Vector3::InParam v, VecFloatRef<INDEX> s, Vector3& result)
{
    result = Divide(v, s);
}

RW_MATH_FORCE_INLINE void Divide(Vector3::InParam v, VecFloatRefIndex::InParam s, Vector3& result) 
{
    result = Divide(v, s);
}

RW_MATH_FORCE_INLINE void Add(Vector3::InParam a, Vector3::InParam b, Vector3& result) 
{
    result = Add(a, b);
}

RW_MATH_FORCE_INLINE void Subtract(Vector3::InParam a, Vector3::InParam b, Vector3& result) 
{
    result = Subtract(a, b);
}

RW_MATH_FORCE_INLINE void Cross(Vector3::InParam a, Vector3::InParam b, Vector3& result) 
{
    result = Cross(a, b);
}

RW_MATH_FORCE_INLINE void Mult(Vector3::InParam a, Vector3::InParam b, Vector3& result) 
{
    result = Mult(a, b);
}

RW_MATH_FORCE_INLINE void Normalize(Vector3::InParam v, Vector3& result) 
{
    result = Normalize(v);
}

RW_MATH_FORCE_INLINE void Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t, Vector3& result) 
{
    result = Lerp( v0, v1, t );
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE void Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRef<INDEX> t, Vector3& result) 
{
    result = Lerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void Lerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRefIndex::InParam t, Vector3& result) 
{
    result = Lerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t, Vector3& result) 
{
    result = SLerp( v0, v1, t );
}

template<VectorAxis INDEX> void SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRef<INDEX> t, Vector3& result)
{
    result = SLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRefIndex::InParam t, Vector3& result) 
{
    result = SLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t, Vector3& result) 
{
    result = NLerp( v0, v1, t );
}

template<VectorAxis INDEX> void NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRef<INDEX> t, Vector3& result)
{
    result = NLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE void NLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloatRefIndex::InParam t, Vector3& result) 
{
    result = NLerp( v0, v1, VecFloat(t) );
}


RW_MATH_FORCE_INLINE void Dot(Vector3::InParam a, Vector3::InParam b, VecFloat& r) 
{
    RW_MATH_VPU_DotV3(r.mV, a.mV, b.mV);
}

#endif

}

}

}

#endif
