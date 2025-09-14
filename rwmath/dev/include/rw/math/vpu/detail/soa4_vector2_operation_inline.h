// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_VECTOR2_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_VECTOR2_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_axis.h"
#include "rw/math/vpu/soa4_float_operation.h"
#include "rw/math/vpu/soa4_scalar.h"
#include "rw/math/vpu/soa4_vector2_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2(Vector2::InParam v)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Splat_I( x, v.mV, VectorAxisX );
    RW_MATH_VPU_Splat_I( y, v.mV, VectorAxisY );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2(Vector2::InParam v0, Vector2::InParam v1, Vector2::InParam v2, Vector2::InParam v3)
{
    VectorIntrinsic x, y;

    RW_MATH_VPU_ExtractXElementsAsVectorA4V4( x, v0.mV, v1.mV, v2.mV, v3.mV );
    RW_MATH_VPU_ExtractYElementsAsVectorA4V4( y, v0.mV, v1.mV, v2.mV, v3.mV );

    return Soa4Vector2( x, y );
}

RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2s(Vector2::InParam v0, Vector2::InParam v1, Vector2::InParam v2, Vector2::InParam v3)
{
    VectorIntrinsic x, y;

    RW_MATH_VPU_ExtractXElementsAsVectorA4V4( x, v0.mV, v1.mV, v2.mV, v3.mV );
    RW_MATH_VPU_ExtractYElementsAsVectorA4V4( y, v0.mV, v1.mV, v2.mV, v3.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Soa4Vector2FromVector2(const Vector2* v)
{
    VectorIntrinsic x, y;

    RW_MATH_VPU_ExtractXElementsAsVectorA4V4( x, v[0].mV, v[1].mV, v[2].mV, v[3].mV );
    RW_MATH_VPU_ExtractYElementsAsVectorA4V4( y, v[0].mV, v[1].mV, v[2].mV, v[3].mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE void Vector2FromSoa4Vector2(Soa4Vector2::InParam v, Vector2& v0, Vector2& v1, Vector2& v2, Vector2& v3)
{
    VectorIntrinsic x, y;
    x = v.x.mV;
    y = v.y.mV;

    RW_MATH_VPU_TransposeA4V4( v0.mV, v1.mV, v2.mV, v3.mV, x, y, x, y );
}


RW_MATH_FORCE_INLINE void Vector2FromSoa4Vector2(Soa4Vector2::InParam v2, Vector2* v)
{
    VectorIntrinsic x, y;
    x = v2.x.mV;
    y = v2.y.mV;

    RW_MATH_VPU_TransposeA4V4( v[0].mV, v[1].mV, v[2].mV, v[3].mV, x, y, x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator *= (Soa4Vector2& v, Soa4Float::InParam a) 
{
    VectorIntrinsic retX, retY;
    RW_MATH_VPU_Mult( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Mult( retY, v.y.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator /= (Soa4Vector2& v, Soa4Float::InParam a)
{
    VectorIntrinsic retX, retY;
    RW_MATH_VPU_Divide( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Divide( retY, v.y.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator += (Soa4Vector2& v, Soa4Float::InParam a)
{    
    VectorIntrinsic retX, retY;
    RW_MATH_VPU_Add( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Add( retY, v.y.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator -= (Soa4Vector2& v, Soa4Float::InParam a)
{
    VectorIntrinsic retX, retY;
    RW_MATH_VPU_Subtract( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Subtract( retY, v.y.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator *  (Soa4Float::InParam s, Soa4Vector2::InParam v)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.mV, v.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator *  (Soa4Vector2::InParam v, Soa4Float::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.mV, v.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator *  (Soa4Vector2::InParam v, Soa4Vector2::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.x.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.y.mV, v.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator /  (Soa4Vector2::InParam v, Soa4Vector2::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator /  (Soa4Vector2::InParam v, Soa4Float::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator +  (Soa4Vector2::InParam v, Soa4Float::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Add( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Add( ret.y.mV, v.y.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Vector2::InParam v, Soa4Float::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, v.y.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator +  (Soa4Float::InParam s, Soa4Vector2::InParam v)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Add( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Add( ret.y.mV, v.y.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Float::InParam s, Soa4Vector2::InParam v)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, s.mV, v.y.mV );

    return ret;
}



RW_MATH_FORCE_INLINE Soa4Vector2& operator *= (Soa4Vector2& v, Soa4Vector2::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Mult( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, v.y.mV, s.y.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator /= (Soa4Vector2& v, Soa4Vector2::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.y.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;

    return v;
}



RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Vector2::InParam v)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Negate( x, v.x.mV );
    RW_MATH_VPU_Negate( y, v.y.mV );
    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator += (Soa4Vector2& v, Soa4Vector2::InParam b)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Add( ret.x.mV, v.x.mV, b.x.mV );
    RW_MATH_VPU_Add( ret.y.mV, v.y.mV, b.y.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector2& operator -= (Soa4Vector2& v, Soa4Vector2::InParam b)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, v.x.mV, b.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, v.y.mV, b.y.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;

    return v;    
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator -  (Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, a.y.mV, b.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2  operator +  (Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Add( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Add( ret.y.mV, a.y.mV, b.y.mV );

    return ret;   
}


RW_MATH_FORCE_INLINE Soa4Vector2 Negate(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Negate( x, v.x.mV );
    RW_MATH_VPU_Negate( y, v.y.mV );
    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Add(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Add( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Add( ret.y.mV, a.y.mV, b.y.mV );

    return ret;   
}


RW_MATH_FORCE_INLINE Soa4Vector2 Subtract(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, a.y.mV, b.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Float Dot(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic x, ret;
    RW_MATH_VPU_Mult( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_MultAdd( ret, a.y.mV, b.y.mV, x );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Float Cross(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic y, ret;
    RW_MATH_VPU_Mult( y, a.x.mV, b.y.mV );
    RW_MATH_VPU_MultSubNeg( ret, a.y.mV, b.x.mV, y );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Vector2 MultAdd(Soa4Vector2::InParam a, Soa4Vector2::InParam b, Soa4Vector2::InParam c)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_MultAdd( x, a.x.mV, b.x.mV, c.x.mV );
    RW_MATH_VPU_MultAdd( y, a.y.mV, b.y.mV, c.y.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Mult(Soa4Vector2::InParam v, Soa4Float::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.mV, v.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2 Mult(Soa4Vector2::InParam s, Soa4Vector2::InParam v)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.x.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.y.mV, v.y.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector2 Divide(Soa4Vector2::InParam v, Soa4Float::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector2 Divide(Soa4Vector2::InParam v, Soa4Vector2::InParam s)
{
    Soa4Vector2 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.y.mV );

    return ret;
}



RW_MATH_FORCE_INLINE Soa4Vector2 Reciprocal(Soa4Vector2::InParam a)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Reciprocal( x, a.x.mV );
    RW_MATH_VPU_Reciprocal( y, a.y.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Float Magnitude(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_SquareRoot( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeFast(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_SquareRootFast( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeEstimate(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_SquareRootEstimate( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeSquared(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeRecip(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_ReciprocalSquareRoot( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Vector2 NormalizeFast(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_ReciprocalSquareRootFast( mag, ret );

    VectorIntrinsic v2x, v2y;
    RW_MATH_VPU_Mult( v2x, v.x.mV, mag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, mag );

    return Soa4Vector2( v2x, v2y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 NormalizeEstimate(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_ReciprocalSquareRootEstimate( mag, ret );

    VectorIntrinsic v2x, v2y;
    RW_MATH_VPU_Mult( v2x, v.x.mV, mag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, mag );

    return Soa4Vector2( v2x, v2y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Normalize(Soa4Vector2::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_ReciprocalSquareRoot( mag, ret );

    VectorIntrinsic v2x, v2y;
    RW_MATH_VPU_Mult( v2x, v.x.mV, mag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, mag );

    return Soa4Vector2( v2x, v2y );
}


RW_MATH_FORCE_INLINE Soa4Float NormalizeReturnMagnitude(Soa4Vector2::InParam v, Soa4Vector2& result)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_SquareRoot( mag, ret );

    VectorIntrinsic invMag;
    RW_MATH_VPU_Reciprocal( invMag, mag );

    VectorIntrinsic v2x, v2y;
    RW_MATH_VPU_Mult( v2x, v.x.mV, invMag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, invMag );

    result = Soa4Vector2( v2x, v2y );
    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float CheckedNormalizeReturnMagnitude(Soa4Vector2::InParam v, Soa4Vector2& result)
{
    VectorIntrinsic x, magnitudeSquared, zero, zeroMask, magnitudeRecip;
    RW_MATH_VPU_GetZero( zero );
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( magnitudeSquared, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_ReciprocalSquareRoot( magnitudeRecip, magnitudeSquared );
    RW_MATH_VPU_IsGreater( zeroMask, magnitudeSquared, zero );

    VectorIntrinsic v2x, v2y, v2xChecked, v2yChecked, magnitude, magnitudeChecked;
    RW_MATH_VPU_Mult( v2x, v.x.mV, magnitudeRecip );
    RW_MATH_VPU_Mult( v2y, v.y.mV, magnitudeRecip );
    RW_MATH_VPU_Select( v2xChecked, zero, v2x, zeroMask ); 
    RW_MATH_VPU_Select( v2yChecked, zero, v2y, zeroMask );
    RW_MATH_VPU_Mult( magnitude, magnitudeSquared, magnitudeRecip );
    RW_MATH_VPU_Select( magnitudeChecked, zero, magnitude, zeroMask );

    result = Soa4Vector2( v2xChecked, v2yChecked );
    return Soa4Float( magnitudeChecked );
}


RW_MATH_FORCE_INLINE Soa4Float CosAngleBetweenNormalizedVectors(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1)
{
    Soa4Float one( RW_MATH_VPU_GetOne_R() );

    Soa4Float dot = Dot( v0, v1 );

    return Clamp( dot, Negate( one ), one );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Lerp(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 SLerp(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t)
{
    Soa4Float dp = Dot( v0, v1 );

    VectorIntrinsic theta, sinTheta, sinThetaRecip;
    RW_MATH_VPU_ACos( theta, dp.mV );
    RW_MATH_VPU_Sin( sinTheta, theta);
    RW_MATH_VPU_Reciprocal( sinThetaRecip, sinTheta );

    VectorIntrinsic one, oneMinusT, sinOneMinusTTerm, sinTTerm, aX, aY, bX, bY;
    RW_MATH_VPU_GetOne( one );
    RW_MATH_VPU_Subtract( oneMinusT, one, t.mV );

    RW_MATH_VPU_Mult( sinOneMinusTTerm, oneMinusT, theta );
    RW_MATH_VPU_Sin( sinOneMinusTTerm, sinOneMinusTTerm );
    RW_MATH_VPU_Mult( sinOneMinusTTerm, sinOneMinusTTerm, sinThetaRecip );

    RW_MATH_VPU_Mult( sinTTerm, t.mV, theta );
    RW_MATH_VPU_Sin( sinTTerm, sinTTerm );
    RW_MATH_VPU_Mult( sinTTerm, sinTTerm, sinThetaRecip );

    RW_MATH_VPU_Mult( aX, v0.x.mV, sinOneMinusTTerm );
    RW_MATH_VPU_Mult( aY, v0.y.mV, sinOneMinusTTerm );
    RW_MATH_VPU_Mult( bX, v1.x.mV, sinTTerm );
    RW_MATH_VPU_Mult( bY, v1.y.mV, sinTTerm );

    Soa4Vector2 slerpValue( Add( Soa4Vector2( aX, aY ), Soa4Vector2( bX, bY ) ) );

    return slerpValue;
}


RW_MATH_FORCE_INLINE Soa4Vector2 NLerpEstimate(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    return NormalizeEstimate( Soa4Vector2( x, y ) );
}


RW_MATH_FORCE_INLINE Soa4Vector2 NLerpFast(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    return NormalizeFast( Soa4Vector2( x, y ) );
}


RW_MATH_FORCE_INLINE Soa4Vector2 NLerp(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    return Normalize( Soa4Vector2( x, y ) );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Min(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Min( x, v0.x.mV, v1.x.mV );
    RW_MATH_VPU_Min( y, v0.y.mV, v1.y.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Max(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Max( x, v0.x.mV, v1.x.mV );
    RW_MATH_VPU_Max( y, v0.y.mV, v1.y.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Clamp(Soa4Vector2::InParam vecToClamp, Soa4Vector2::InParam minVecValue, Soa4Vector2::InParam maxVecValue)
{
    VectorIntrinsic rX, rY;
    RW_MATH_VPU_Max( rX, vecToClamp.x.mV, minVecValue.x.mV );
    RW_MATH_VPU_Max( rY, vecToClamp.y.mV, minVecValue.y.mV );
    RW_MATH_VPU_Min( rX, rX, maxVecValue.x.mV );
    RW_MATH_VPU_Min( rY, rY, maxVecValue.y.mV );

    return Soa4Vector2( rX, rY );
}


RW_MATH_FORCE_INLINE Soa4Vector2 ClampToNegPosRange(Soa4Vector2::InParam vecToClamp, Soa4Vector2::InParam range)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_ClampToNegPosRange( x, vecToClamp.x.mV, range.x.mV );
    RW_MATH_VPU_ClampToNegPosRange( y, vecToClamp.y.mV, range.y.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Abs(Soa4Vector2::InParam v0)
{
    VectorIntrinsic negX, negY, x, y;
    RW_MATH_VPU_Negate( negX, v0.x.mV );
    RW_MATH_VPU_Negate( negY, v0.y.mV );
    RW_MATH_VPU_Max( x, v0.x.mV, negX );
    RW_MATH_VPU_Max( y, v0.y.mV, negY );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectors(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1)
{
    VectorIntrinsic ret;
    Soa4Vector2 normalizedV0 = Normalize( v0 );
    Soa4Vector2 normalizedV1 = Normalize( v1 );
    Soa4Float dot = Dot( normalizedV0, normalizedV1 );
    RW_MATH_VPU_ACos( ret, dot.mV );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectorsFast(Soa4Vector2::InParam v0, Soa4Vector2::InParam v1)
{
    VectorIntrinsic ret;
    Soa4Vector2 normalizedV0 = NormalizeFast( v0 );
    Soa4Vector2 normalizedV1 = NormalizeFast( v1 );
    Soa4Float dot = Dot( normalizedV0, normalizedV1 );
    RW_MATH_VPU_ACos( ret, dot.mV );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompLessThan(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic xMask, yMask;
    RW_MATH_VPU_IsGreater( xMask, b.x.mV, a.x.mV );
    RW_MATH_VPU_IsGreater( yMask, b.y.mV, a.y.mV );

    return Soa4Mask2( xMask, yMask );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompGreaterThan(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic xMask, yMask;
    RW_MATH_VPU_IsGreater( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsGreater( yMask, a.y.mV, b.y.mV );

    return Soa4Mask2( xMask, yMask );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompLessEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{    
    VectorIntrinsic xMask, yMask;
    RW_MATH_VPU_IsGreaterEqual( xMask, b.x.mV, a.x.mV );
    RW_MATH_VPU_IsGreaterEqual( yMask, b.y.mV, a.y.mV );

    return Soa4Mask2( xMask, yMask );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompGreaterEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic xMask, yMask;
    RW_MATH_VPU_IsGreaterEqual( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsGreaterEqual( yMask, a.y.mV, b.y.mV );

    return Soa4Mask2( xMask, yMask );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic xMask, yMask;
    RW_MATH_VPU_IsEqual( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsEqual( yMask, a.y.mV, b.y.mV );

    return Soa4Mask2( xMask, yMask );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompNotEqual(Soa4Vector2::InParam a, Soa4Vector2::InParam b)
{
    VectorIntrinsic xMask, yMask;
    RW_MATH_VPU_IsEqual( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsEqual( yMask, a.y.mV, b.y.mV );
    RW_MATH_VPU_Not( xMask, xMask );
    RW_MATH_VPU_Not( yMask, yMask );

    return Soa4Mask2( xMask, yMask );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Select(Soa4MaskScalar::InParam mask, Soa4Vector2::InParam trueValue, Soa4Vector2::InParam falseValue )
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.mV );

    return Soa4Vector2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Vector2 Select(Soa4Mask2::InParam mask, Soa4Vector2::InParam trueValue, Soa4Vector2::InParam falseValue )
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.x.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.y.mV );

    return Soa4Vector2( x, y );
}




}}}

#endif

