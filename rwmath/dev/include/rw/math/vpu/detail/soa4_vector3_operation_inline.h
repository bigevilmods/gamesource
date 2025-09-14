// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_VECTOR3_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_VECTOR3_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_axis.h"
#include "rw/math/vpu/soa4_float_operation.h"
#include "rw/math/vpu/soa4_scalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3(Vector3::InParam v)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Splat_I( x, v.mV, VectorAxisX );
    RW_MATH_VPU_Splat_I( y, v.mV, VectorAxisY );
    RW_MATH_VPU_Splat_I( z, v.mV, VectorAxisZ );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam v2, Vector3::InParam v3)
{
    VectorIntrinsic x, y, z;

    RW_MATH_VPU_TransposeA4V3( x, y, z, v0.mV, v1.mV, v2.mV, v3.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3s(Vector3::InParam v0, Vector3::InParam v1, Vector3::InParam v2, Vector3::InParam v3)
{
    VectorIntrinsic x, y, z;

    RW_MATH_VPU_TransposeA4V3( x, y, z, v0.mV, v1.mV, v2.mV, v3.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Soa4Vector3FromVector3(const Vector3* v)
{
    VectorIntrinsic x, y, z;

    RW_MATH_VPU_TransposeA4V3( x, y, z, v[0].mV, v[1].mV, v[2].mV, v[3].mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE void Vector3FromSoa4Vector3(Soa4Vector3::InParam v, Vector3& v0, Vector3& v1, Vector3& v2, Vector3& v3)
{
    VectorIntrinsic x, y, z;
    x = v.x.mV;
    y = v.y.mV;
    z = v.z.mV;

    RW_MATH_VPU_TransposeA4V4( v0.mV, v1.mV, v2.mV, v3.mV, x, y, z, x );
}


RW_MATH_FORCE_INLINE void Vector3FromSoa4Vector3(Soa4Vector3::InParam v2, Vector3* v)
{
    VectorIntrinsic x, y, z;
    x = v2.x.mV;
    y = v2.y.mV;
    z = v2.z.mV;

    RW_MATH_VPU_TransposeA4V4( v[0].mV, v[1].mV, v[2].mV, v[3].mV, x, y, z, y );
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator *= (Soa4Vector3& v, Soa4Float::InParam a) 
{
    VectorIntrinsic retX, retY, retZ;
    RW_MATH_VPU_Mult( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Mult( retY, v.y.mV, a.mV );
    RW_MATH_VPU_Mult( retZ, v.z.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;
    v.z.mV = retZ;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator /= (Soa4Vector3& v, Soa4Float::InParam a)
{
    VectorIntrinsic retX, retY, retZ;
    RW_MATH_VPU_Divide( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Divide( retY, v.y.mV, a.mV );
    RW_MATH_VPU_Divide( retZ, v.z.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;
    v.z.mV = retZ;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator += (Soa4Vector3& v, Soa4Float::InParam a)
{    
    VectorIntrinsic retX, retY, retZ;
    RW_MATH_VPU_Add( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Add( retY, v.y.mV, a.mV );
    RW_MATH_VPU_Add( retZ, v.z.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;
    v.z.mV = retZ;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator -= (Soa4Vector3& v, Soa4Float::InParam a)
{
    VectorIntrinsic retX, retY, retZ;
    RW_MATH_VPU_Subtract( retX, v.x.mV, a.mV );
    RW_MATH_VPU_Subtract( retY, v.y.mV, a.mV );
    RW_MATH_VPU_Subtract( retZ, v.z.mV, a.mV );
    v.x.mV = retX;
    v.y.mV = retY;
    v.z.mV = retZ;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator *  (Soa4Float::InParam s, Soa4Vector3::InParam v)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.mV, v.y.mV );
    RW_MATH_VPU_Mult( ret.z.mV, s.mV, v.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator *  (Soa4Vector3::InParam v, Soa4Float::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.mV, v.y.mV );
    RW_MATH_VPU_Mult( ret.z.mV, s.mV, v.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator *  (Soa4Vector3::InParam v, Soa4Vector3::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.x.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.y.mV, v.y.mV );
    RW_MATH_VPU_Mult( ret.z.mV, s.z.mV, v.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator /  (Soa4Vector3::InParam v, Soa4Vector3::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.y.mV );
    RW_MATH_VPU_Divide( ret.z.mV, v.z.mV, s.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator /  (Soa4Vector3::InParam v, Soa4Float::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.mV );
    RW_MATH_VPU_Divide( ret.z.mV, v.z.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator +  (Soa4Vector3::InParam v, Soa4Float::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Add( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Add( ret.y.mV, v.y.mV, s.mV );
    RW_MATH_VPU_Add( ret.z.mV, v.z.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Vector3::InParam v, Soa4Float::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, v.y.mV, s.mV );
    RW_MATH_VPU_Subtract( ret.z.mV, v.z.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator +  (Soa4Float::InParam s, Soa4Vector3::InParam v)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Add( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Add( ret.y.mV, v.y.mV, s.mV );
    RW_MATH_VPU_Add( ret.z.mV, v.z.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Float::InParam s, Soa4Vector3::InParam v)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, s.mV, v.y.mV );
    RW_MATH_VPU_Subtract( ret.z.mV, s.mV, v.z.mV );

    return ret;
}



RW_MATH_FORCE_INLINE Soa4Vector3& operator *= (Soa4Vector3& v, Soa4Vector3::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Mult( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, v.y.mV, s.y.mV );
    RW_MATH_VPU_Mult( ret.z.mV, v.z.mV, s.z.mV );
    
    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;
    v.z.mV = ret.z.mV;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator /= (Soa4Vector3& v, Soa4Vector3::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.y.mV );
    RW_MATH_VPU_Divide( ret.z.mV, v.z.mV, s.z.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;
    v.z.mV = ret.z.mV;
    
    return v;
}



RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Vector3::InParam v)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Negate( x, v.x.mV );
    RW_MATH_VPU_Negate( y, v.y.mV );
    RW_MATH_VPU_Negate( z, v.z.mV );
    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator += (Soa4Vector3& v, Soa4Vector3::InParam b)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Add( ret.x.mV, v.x.mV, b.x.mV );
    RW_MATH_VPU_Add( ret.y.mV, v.y.mV, b.y.mV );
    RW_MATH_VPU_Add( ret.z.mV, v.z.mV, b.z.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;
    v.z.mV = ret.z.mV;

    return v;
}


RW_MATH_FORCE_INLINE Soa4Vector3& operator -= (Soa4Vector3& v, Soa4Vector3::InParam b)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, v.x.mV, b.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, v.y.mV, b.y.mV );
    RW_MATH_VPU_Subtract( ret.z.mV, v.z.mV, b.z.mV );

    v.x.mV = ret.x.mV;
    v.y.mV = ret.y.mV;
    v.z.mV = ret.z.mV;

    return v;    
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator -  (Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, a.y.mV, b.y.mV );
    RW_MATH_VPU_Subtract( ret.z.mV, a.z.mV, b.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3  operator +  (Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Add( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Add( ret.y.mV, a.y.mV, b.y.mV );
    RW_MATH_VPU_Add( ret.z.mV, a.z.mV, b.z.mV );

    return ret;   
}


RW_MATH_FORCE_INLINE Soa4Vector3 Negate(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Negate( x, v.x.mV );
    RW_MATH_VPU_Negate( y, v.y.mV );
    RW_MATH_VPU_Negate( z, v.z.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Add(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Add( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Add( ret.y.mV, a.y.mV, b.y.mV );
    RW_MATH_VPU_Add( ret.z.mV, a.z.mV, b.z.mV );

    return ret;   
}


RW_MATH_FORCE_INLINE Soa4Vector3 Subtract(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Subtract( ret.x.mV, a.x.mV, b.x.mV );
    RW_MATH_VPU_Subtract( ret.y.mV, a.y.mV, b.y.mV );
    RW_MATH_VPU_Subtract( ret.z.mV, a.z.mV, b.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Float Dot(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic x, ret;
    RW_MATH_VPU_Mult( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_MultAdd( ret, a.y.mV, b.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, a.z.mV, b.z.mV, ret );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Cross(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic tmp, i, j, k;
    RW_MATH_VPU_Mult( tmp, a.y.mV, b.z.mV );
    RW_MATH_VPU_MultSubNeg( i, a.z.mV, b.y.mV, tmp );

    RW_MATH_VPU_Mult( tmp, a.z.mV, b.x.mV );
    RW_MATH_VPU_MultSubNeg( j, a.x.mV, b.z.mV, tmp );

    RW_MATH_VPU_Mult( tmp, a.x.mV, b.y.mV );
    RW_MATH_VPU_MultSubNeg( k, a.y.mV, b.x.mV, tmp );

    return Soa4Vector3( i, j, k );
}


RW_MATH_FORCE_INLINE Soa4Vector3 MultAdd(Soa4Vector3::InParam a, Soa4Vector3::InParam b, Soa4Vector3::InParam c)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_MultAdd( x, a.x.mV, b.x.mV, c.x.mV );
    RW_MATH_VPU_MultAdd( y, a.y.mV, b.y.mV, c.y.mV );
    RW_MATH_VPU_MultAdd( z, a.z.mV, b.z.mV, c.z.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Mult(Soa4Vector3::InParam v, Soa4Float::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.mV, v.y.mV );
    RW_MATH_VPU_Mult( ret.z.mV, s.mV, v.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3 Mult(Soa4Vector3::InParam s, Soa4Vector3::InParam v)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Mult( ret.x.mV, s.x.mV, v.x.mV );
    RW_MATH_VPU_Mult( ret.y.mV, s.y.mV, v.y.mV );
    RW_MATH_VPU_Mult( ret.z.mV, s.z.mV, v.z.mV );

    return ret;
}


RW_MATH_FORCE_INLINE Soa4Vector3 Divide(Soa4Vector3::InParam v, Soa4Float::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.mV );
    RW_MATH_VPU_Divide( ret.z.mV, v.z.mV, s.mV );

    return ret;    
}


RW_MATH_FORCE_INLINE Soa4Vector3 Divide(Soa4Vector3::InParam v, Soa4Vector3::InParam s)
{
    Soa4Vector3 ret;
    RW_MATH_VPU_Divide( ret.x.mV, v.x.mV, s.x.mV );
    RW_MATH_VPU_Divide( ret.y.mV, v.y.mV, s.y.mV );
    RW_MATH_VPU_Divide( ret.z.mV, v.z.mV, s.z.mV );

    return ret;
}



RW_MATH_FORCE_INLINE Soa4Vector3 Reciprocal(Soa4Vector3::InParam a)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Reciprocal( x, a.x.mV );
    RW_MATH_VPU_Reciprocal( y, a.y.mV );
    RW_MATH_VPU_Reciprocal( z, a.z.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Float Magnitude(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_SquareRoot( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeFast(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_SquareRootFast( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeEstimate(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_SquareRootEstimate( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeSquared(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Float MagnitudeRecip(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_ReciprocalSquareRoot( mag, ret );

    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Vector3 NormalizeFast(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_ReciprocalSquareRootFast( mag, ret );

    VectorIntrinsic v2x, v2y, v2z;
    RW_MATH_VPU_Mult( v2x, v.x.mV, mag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, mag );
    RW_MATH_VPU_Mult( v2z, v.z.mV, mag );

    return Soa4Vector3( v2x, v2y, v2z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 NormalizeEstimate(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_ReciprocalSquareRootEstimate( mag, ret );

    VectorIntrinsic v2x, v2y, v2z;
    RW_MATH_VPU_Mult( v2x, v.x.mV, mag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, mag );
    RW_MATH_VPU_Mult( v2z, v.z.mV, mag );

    return Soa4Vector3( v2x, v2y, v2z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Normalize(Soa4Vector3::InParam v)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_ReciprocalSquareRoot( mag, ret );

    VectorIntrinsic v2x, v2y, v2z;
    RW_MATH_VPU_Mult( v2x, v.x.mV, mag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, mag );
    RW_MATH_VPU_Mult( v2z, v.z.mV, mag );

    return Soa4Vector3( v2x, v2y, v2z );
}


RW_MATH_FORCE_INLINE Soa4Float NormalizeReturnMagnitude(Soa4Vector3::InParam v, Soa4Vector3& result)
{
    VectorIntrinsic x, ret, mag;
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( ret, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( ret, v.z.mV, v.z.mV, ret );
    RW_MATH_VPU_SquareRoot( mag, ret );

    VectorIntrinsic invMag;
    RW_MATH_VPU_Reciprocal( invMag, mag );

    VectorIntrinsic v2x, v2y, v2z;
    RW_MATH_VPU_Mult( v2x, v.x.mV, invMag );
    RW_MATH_VPU_Mult( v2y, v.y.mV, invMag );
    RW_MATH_VPU_Mult( v2z, v.z.mV, invMag );

    result = Soa4Vector3( v2x, v2y, v2z );
    return Soa4Float( mag );
}


RW_MATH_FORCE_INLINE Soa4Float CheckedNormalizeReturnMagnitude(Soa4Vector3::InParam v, Soa4Vector3& result)
{
    VectorIntrinsic x, magnitudeSquared, zero, zeroMask, magnitudeRecip;
    RW_MATH_VPU_GetZero( zero );
    RW_MATH_VPU_Mult( x, v.x.mV, v.x.mV );
    RW_MATH_VPU_MultAdd( magnitudeSquared, v.y.mV, v.y.mV, x );
    RW_MATH_VPU_MultAdd( magnitudeSquared, v.z.mV, v.z.mV, magnitudeSquared );    
    RW_MATH_VPU_ReciprocalSquareRoot( magnitudeRecip, magnitudeSquared );
    RW_MATH_VPU_IsGreater( zeroMask, magnitudeSquared, zero );

    VectorIntrinsic v2x, v2y, v2z, v2xChecked, v2yChecked, v2zChecked, magnitude, magnitudeChecked;
    RW_MATH_VPU_Mult( v2x, v.x.mV, magnitudeRecip );
    RW_MATH_VPU_Mult( v2y, v.y.mV, magnitudeRecip );
    RW_MATH_VPU_Mult( v2z, v.z.mV, magnitudeRecip );
    RW_MATH_VPU_Select( v2xChecked, zero, v2x, zeroMask ); 
    RW_MATH_VPU_Select( v2yChecked, zero, v2y, zeroMask );
    RW_MATH_VPU_Select( v2zChecked, zero, v2z, zeroMask );
    RW_MATH_VPU_Mult( magnitude, magnitudeSquared, magnitudeRecip );
    RW_MATH_VPU_Select( magnitudeChecked, zero, magnitude, zeroMask );

    result = Soa4Vector3( v2xChecked, v2yChecked, v2zChecked );
    return Soa4Float( magnitudeChecked );
}


RW_MATH_FORCE_INLINE Soa4Float CosAngleBetweenNormalizedVectors(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1)
{
    Soa4Float one( RW_MATH_VPU_GetOne_R() );
    return Clamp( Dot( v0, v1 ), Negate( one ), one );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Lerp(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    RW_MATH_VPU_Lerp( z, v0.z.mV, v1.z.mV, t.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 SLerp(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t)
{
    Soa4Float dp = Dot( v0, v1 );

    VectorIntrinsic theta, sinTheta, sinThetaRecip;
    RW_MATH_VPU_ACos( theta, dp.mV );
    RW_MATH_VPU_Sin( sinTheta, theta);
    RW_MATH_VPU_Reciprocal( sinThetaRecip, sinTheta );

    VectorIntrinsic one, oneMinusT, sinOneMinusTTerm, sinTTerm, aX, aY, aZ, bX, bY, bZ;
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
    RW_MATH_VPU_Mult( aZ, v0.z.mV, sinOneMinusTTerm );
    RW_MATH_VPU_Mult( bX, v1.x.mV, sinTTerm );
    RW_MATH_VPU_Mult( bY, v1.y.mV, sinTTerm );
    RW_MATH_VPU_Mult( bZ, v1.z.mV, sinTTerm );

    Soa4Vector3 slerpValue( Add( Soa4Vector3( aX, aY, aZ ), Soa4Vector3( bX, bY, bZ ) ) );

    return slerpValue;
}


RW_MATH_FORCE_INLINE Soa4Vector3 NLerpEstimate(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    RW_MATH_VPU_Lerp( z, v0.z.mV, v1.z.mV, t.mV );
    return NormalizeEstimate( Soa4Vector3( x, y, z ) );
}


RW_MATH_FORCE_INLINE Soa4Vector3 NLerpFast(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    RW_MATH_VPU_Lerp( z, v0.z.mV, v1.z.mV, t.mV );
    return NormalizeFast( Soa4Vector3( x, y, z ) );
}


RW_MATH_FORCE_INLINE Soa4Vector3 NLerp(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Float::InParam t)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Lerp( x, v0.x.mV, v1.x.mV, t.mV );
    RW_MATH_VPU_Lerp( y, v0.y.mV, v1.y.mV, t.mV );
    RW_MATH_VPU_Lerp( z, v0.z.mV, v1.z.mV, t.mV );
    return Normalize( Soa4Vector3( x, y, z ) );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Min(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Min( x, v0.x.mV, v1.x.mV );
    RW_MATH_VPU_Min( y, v0.y.mV, v1.y.mV );
    RW_MATH_VPU_Min( z, v0.z.mV, v1.z.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Max(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Max( x, v0.x.mV, v1.x.mV );
    RW_MATH_VPU_Max( y, v0.y.mV, v1.y.mV );
    RW_MATH_VPU_Max( z, v0.z.mV, v1.z.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Clamp(Soa4Vector3::InParam vecToClamp, Soa4Vector3::InParam minVecValue, Soa4Vector3::InParam maxVecValue)
{
    VectorIntrinsic rX, rY, rZ;
    RW_MATH_VPU_Max( rX, vecToClamp.x.mV, minVecValue.x.mV );
    RW_MATH_VPU_Max( rY, vecToClamp.y.mV, minVecValue.y.mV );
    RW_MATH_VPU_Max( rZ, vecToClamp.z.mV, minVecValue.z.mV );
    RW_MATH_VPU_Min( rX, rX, maxVecValue.x.mV );
    RW_MATH_VPU_Min( rY, rY, maxVecValue.y.mV );
    RW_MATH_VPU_Min( rZ, rZ, maxVecValue.z.mV );
    return Soa4Vector3( rX, rY, rZ );
}


RW_MATH_FORCE_INLINE Soa4Vector3 ClampToNegPosRange(Soa4Vector3::InParam vecToClamp, Soa4Vector3::InParam range)
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_ClampToNegPosRange( x, vecToClamp.x.mV, range.x.mV );
    RW_MATH_VPU_ClampToNegPosRange( y, vecToClamp.y.mV, range.y.mV );
    RW_MATH_VPU_ClampToNegPosRange( z, vecToClamp.z.mV, range.z.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Abs(Soa4Vector3::InParam v0)
{
    VectorIntrinsic negX, negY, negZ, x, y, z;
    RW_MATH_VPU_Negate( negX, v0.x.mV );
    RW_MATH_VPU_Negate( negY, v0.y.mV );
    RW_MATH_VPU_Negate( negZ, v0.z.mV );
    RW_MATH_VPU_Max( x, v0.x.mV, negX );
    RW_MATH_VPU_Max( y, v0.y.mV, negY );
    RW_MATH_VPU_Max( z, v0.z.mV, negZ );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectors(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1)
{
    VectorIntrinsic ret;
    Soa4Vector3 normalizedV0 = Normalize( v0 );
    Soa4Vector3 normalizedV1 = Normalize( v1 );
    Soa4Float dot = Dot( normalizedV0, normalizedV1 );
    RW_MATH_VPU_ACos( ret, dot.mV );

    return Soa4Float( ret );
}


RW_MATH_FORCE_INLINE Soa4Float AngleBetweenVectors(Soa4Vector3::InParam v0, Soa4Vector3::InParam v1, Soa4Vector3::InParam windAxis)
{
    VectorIntrinsic angle, zero, mask, twoPi, twoPiMinusAngle, result;
    Soa4Vector3 normalizedV0 = NormalizeFast( v0 );
    Soa4Vector3 normalizedV1 = NormalizeFast( v1 );
    Soa4Float dot = Dot( normalizedV0, normalizedV1 );
    RW_MATH_VPU_ACos( angle, dot.mV );

    Soa4Vector3 norm = Cross( normalizedV0, normalizedV1 );
    dot = Dot( norm, windAxis );
    RW_MATH_VPU_GetZero( zero );
    RW_MATH_VPU_IsGreater( mask, zero, dot.mV );
    RW_MATH_VPU_CreateVectorV1( twoPi, TWO_PI );
    RW_MATH_VPU_Subtract( twoPiMinusAngle, twoPi, angle );
    RW_MATH_VPU_Select( result, angle, twoPiMinusAngle, mask );

    return Soa4Float( result );
}


RW_MATH_FORCE_INLINE Soa4Mask3 CompLessThan(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic xMask, yMask, zMask;
    RW_MATH_VPU_IsGreater( xMask, b.x.mV, a.x.mV );
    RW_MATH_VPU_IsGreater( yMask, b.y.mV, a.y.mV );
    RW_MATH_VPU_IsGreater( zMask, b.z.mV, a.z.mV );

    return Soa4Mask3( xMask, yMask, zMask );
}


RW_MATH_FORCE_INLINE Soa4Mask3 CompGreaterThan(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic xMask, yMask, zMask;
    RW_MATH_VPU_IsGreater( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsGreater( yMask, a.y.mV, b.y.mV );
    RW_MATH_VPU_IsGreater( zMask, a.z.mV, b.z.mV );

    return Soa4Mask3( xMask, yMask, zMask );
}


RW_MATH_FORCE_INLINE Soa4Mask3 CompLessEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{    
    VectorIntrinsic xMask, yMask, zMask;
    RW_MATH_VPU_IsGreaterEqual( xMask, b.x.mV, a.x.mV );
    RW_MATH_VPU_IsGreaterEqual( yMask, b.y.mV, a.y.mV );
    RW_MATH_VPU_IsGreaterEqual( zMask, b.z.mV, a.z.mV );

    return Soa4Mask3( xMask, yMask, zMask );
}


RW_MATH_FORCE_INLINE Soa4Mask3 CompGreaterEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic xMask, yMask, zMask;
    RW_MATH_VPU_IsGreaterEqual( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsGreaterEqual( yMask, a.y.mV, b.y.mV );
    RW_MATH_VPU_IsGreaterEqual( zMask, a.z.mV, b.z.mV );

    return Soa4Mask3( xMask, yMask, zMask );
}


RW_MATH_FORCE_INLINE Soa4Mask3 CompEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic xMask, yMask, zMask;
    RW_MATH_VPU_IsEqual( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsEqual( yMask, a.y.mV, b.y.mV );
    RW_MATH_VPU_IsEqual( zMask, a.z.mV, b.z.mV );

    return Soa4Mask3( xMask, yMask, zMask );
}


RW_MATH_FORCE_INLINE Soa4Mask3 CompNotEqual(Soa4Vector3::InParam a, Soa4Vector3::InParam b)
{
    VectorIntrinsic xMask, yMask, zMask;
    RW_MATH_VPU_IsEqual( xMask, a.x.mV, b.x.mV );
    RW_MATH_VPU_IsEqual( yMask, a.y.mV, b.y.mV );
    RW_MATH_VPU_IsEqual( zMask, a.z.mV, b.z.mV );
    RW_MATH_VPU_Not( xMask, xMask );
    RW_MATH_VPU_Not( yMask, yMask );
    RW_MATH_VPU_Not( zMask, zMask );

    return Soa4Mask3( xMask, yMask, zMask );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Select(Soa4MaskScalar::InParam mask, Soa4Vector3::InParam trueValue, Soa4Vector3::InParam falseValue )
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.mV );
    RW_MATH_VPU_Select( z, falseValue.z.mV, trueValue.z.mV, mask.mV );

    return Soa4Vector3( x, y, z );
}


RW_MATH_FORCE_INLINE Soa4Vector3 Select(Soa4Mask3::InParam mask, Soa4Vector3::InParam trueValue, Soa4Vector3::InParam falseValue )
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.x.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.y.mV );
    RW_MATH_VPU_Select( z, falseValue.z.mV, trueValue.z.mV, mask.z.mV );

    return Soa4Vector3( x, y, z );
}


}}}

#endif

