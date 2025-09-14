// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX33_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX33_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/vector3_operation.h"
#include "rw/math/vpu/quaternion_operation.h"
#include "rw/math/vpu/detail/matrix33_operation_platform_inline.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE bool operator ==(Matrix33::InParam a, Matrix33::InParam b) 
{
    return  a.xAxis == b.xAxis &&
            a.yAxis == b.yAxis &&
            a.zAxis == b.zAxis;
}

RW_MATH_FORCE_INLINE bool operator !=(Matrix33::InParam a, Matrix33::InParam b) 
{
    return  a.xAxis != b.xAxis ||
            a.yAxis != b.yAxis ||
            a.zAxis != b.zAxis;
}


RW_MATH_FORCE_INLINE bool IsValid(Matrix33::InParam a) 
{
    return IsValid(a.xAxis) &&
           IsValid(a.yAxis) &&
           IsValid(a.zAxis);
}

RW_MATH_FORCE_INLINE bool IsSimilar(Matrix33::InParam a, Matrix33::InParam b, VecFloat::InParam epsilon ) 
{
    return _asmIsSimilarV3( a.xAxis.mV, a.yAxis.mV, a.zAxis.mV,
                            b.xAxis.mV, b.yAxis.mV, b.zAxis.mV,
                            epsilon.mV );
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Matrix33::InParam m)
{
    return (bool)(IsAnyNaN(m.xAxis) | IsAnyNaN(m.yAxis) | IsAnyNaN(m.zAxis));
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError(Matrix33::InParam matrix) 
{
    Matrix33 identity;
    identity.SetIdentity();

    Matrix33 result = Mult(matrix, Transpose(matrix));
    SelfSubtract(result, identity);

    Vector3 errorVec( MagnitudeSquared( result.XAxis() ),
                      MagnitudeSquared( result.YAxis() ),
                      MagnitudeSquared( result.ZAxis() ) );

    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError(Matrix33::InParam matrix) 
{
    Vector3 errorVec(   Dot( matrix.xAxis, matrix.yAxis ),
                        Dot( matrix.yAxis, matrix.zAxis ),
                        Dot( matrix.zAxis, matrix.xAxis ) );
    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE bool IsNormal(Matrix33::InParam matrix,        VecFloat::InParam tolerance) 
{
    return IsZero( GetNormalError(matrix), tolerance );
}

RW_MATH_FORCE_INLINE bool IsOrthogonal(Matrix33::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalError(a), tolerance );
}

RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet(Matrix33::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalRowSetError(a), tolerance );
}

RW_MATH_FORCE_INLINE bool IsRotation(Matrix33::InParam a, VecFloat::InParam tolerance)
{
    return IsOrthogonal(a, tolerance) && IsSimilar(Determinant(a), GetVecFloat_One(), tolerance);
}

RW_MATH_FORCE_INLINE Matrix33 Negate(Matrix33::InParam m) 
{
    return Matrix33( Negate(m.xAxis), Negate(m.yAxis), Negate(m.zAxis) );
}

RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, VecFloat::InParam a) 
{
    return Matrix33(
        Mult(m.xAxis, a),
        Mult(m.yAxis, a),
        Mult(m.zAxis, a)
    );
}

template<VectorAxis INDEX> Matrix33 Mult(Matrix33::InParam m, VecFloatRef<INDEX> a)
{
    return Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, VecFloatRefIndex::InParam a) 
{
    return Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix33 Divide(Matrix33::InParam m, VecFloat::InParam a) 
{
    VecFloat recipA = Reciprocal(a);
    return Mult(m, recipA);
}

template<VectorAxis INDEX> Matrix33 Divide(Matrix33::InParam m, VecFloatRef<INDEX> a)
{
    return Divide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix33 Divide(Matrix33::InParam m, VecFloatRefIndex::InParam a) 
{
    return Divide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix33 Add(Matrix33::InParam a, Matrix33::InParam b) 
{
    return Matrix33(
                Add(a.xAxis, b.xAxis),
                Add(a.yAxis, b.yAxis),
                Add(a.zAxis, b.zAxis)
            );
}

RW_MATH_FORCE_INLINE Matrix33 Subtract(Matrix33::InParam a, Matrix33::InParam b) 
{
    return Matrix33(
                Subtract(a.xAxis, b.xAxis),
                Subtract(a.yAxis, b.yAxis),
                Subtract(a.zAxis, b.zAxis)
            );
}

RW_MATH_FORCE_INLINE void SelfAdd(Matrix33& a, Matrix33::InParam b) 
{
    a = Add(a,b);
}

RW_MATH_FORCE_INLINE void SelfSubtract(Matrix33& a, Matrix33::InParam b) 
{
    a = Subtract(a,b);
}

RW_MATH_FORCE_INLINE void SelfMult(Matrix33& m, VecFloat::InParam a) 
{
    m = Mult(m, a);
}

template<VectorAxis INDEX> void SelfMult(Matrix33& m, VecFloatRef<INDEX> a)
{
    SelfMult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void SelfMult(Matrix33& m, VecFloatRefIndex::InParam a) 
{
    SelfMult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void SelfDivide(Matrix33& m, VecFloat::InParam a) 
{
    m = Divide(m, a);
}

template<VectorAxis INDEX> void SelfDivide(Matrix33& m, VecFloatRef<INDEX> a)
{
    SelfDivide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void SelfDivide(Matrix33& m, VecFloatRefIndex::InParam a) 
{
    SelfDivide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix33 InverseOrthonormal(Matrix33::InParam in) 
{
    RW_MATH_ASSERT_MSG(IsOrthogonal(in), ("Input matrix is non orthonormal."));
    return Transpose(in);
}

RW_MATH_FORCE_INLINE Matrix33 Inverse(Matrix33::InParam m) 
{
    VectorIntrinsic d;
    VectorIntrinsic x;
    VectorIntrinsic y;
    VectorIntrinsic z;
    RW_MATH_VPU_InverseA3V3(d, x, y, z, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV);
    return Matrix33(x, y, z);
}

RW_MATH_FORCE_INLINE Vector3 Transform(Vector3::InParam vec, Matrix33::InParam matrix) 
{
    return Mult(vec, matrix);
}

RW_MATH_FORCE_INLINE void Transform(const Vector3* const vecs, int numVectors, Matrix33::InParam matrix, Vector3 * transformedVecs) 
{
    for( int vecIndex = 0; vecIndex < numVectors; ++vecIndex )
    {
        transformedVecs[vecIndex] = Transform(vecs[vecIndex], matrix);
    }
}

RW_MATH_FORCE_INLINE Matrix33 RotateAroundXAxis(Matrix33::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //    [         e00                e01                e02       ]
    //    [ e20 SIN + e10 COS  e21 SIN + e11 COS  e22 SIN + e12 COS ]
    //    [ e20 COS - e10 SIN  e21 COS - e11 SIN  e22 COS - e12 SIN ]
    VecFloat s,c;
    SinCos(angle, s, c);

    // rotate incrementally about x
    return Matrix33(
        m.xAxis,
        m.zAxis * s + m.yAxis * c,
        m.zAxis * c - m.yAxis * s
        ); 
}

RW_MATH_FORCE_INLINE Matrix33 RotateAroundYAxis(Matrix33::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e00 COS - e20 SIN  e01 COS - e21 SIN  e02 COS - e22 SIN ]
    //  [         e10                e11                e12       ]
    //  [ e00 SIN + e20 COS  e01 SIN + e21 COS  e02 SIN + e22 COS ]
    VecFloat s,c;

    SinCos(angle, s, c);

    // rotate incrementally about y
    return Matrix33(
        m.xAxis * c - m.zAxis * s,
        m.yAxis,           
        m.xAxis * s + m.zAxis * c
        );                 
}

RW_MATH_FORCE_INLINE Matrix33 RotateAroundZAxis(Matrix33::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e10 SIN + e00 COS  e11 SIN + e01 COS  e12 SIN + e02 COS ]
    //  [ e10 COS - e00 SIN  e11 COS - e01 SIN  e12 COS - e02 SIN ]
    //  [         e20                e21                e22       ]
    VecFloat s,c;

    SinCos(angle, s, c);

    // rotate incrementally about z
    return Matrix33(
        m.yAxis * s + m.xAxis * c,
        m.yAxis * c - m.xAxis * s,
        m.zAxis
        ); 
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromEulerXYZ(Vector3::InParam euler) 
{
    VecFloat   ci, cj, ch, si, sj, sh;

    SinCos(euler.X(), si, ci);
    SinCos(euler.Y(), sj, cj);
    SinCos(euler.Z(), sh, ch);

    VecFloat cc = ci * ch;
    VecFloat cs = ci * sh;
    VecFloat sc = si * ch;
    VecFloat ss = si * sh;

    return Matrix33(
        cj * ch     ,  cj * sh     , -sj      , 
        sj * sc - cs,  sj * ss + cc,  cj * si ,
        sj * cc + ss,  sj * cs - sc,  cj * ci 
        );
}

RW_MATH_FORCE_INLINE Matrix33 ScaleRow(Matrix33::InParam m, VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2) 
{
    return Matrix33(
        Mult( m.xAxis, r0 ),
        Mult( m.yAxis, r1 ),
        Mult( m.zAxis, r2 )
        );
}

RW_MATH_FORCE_INLINE Matrix33 ScaleColumn(Matrix33::InParam m, VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2 ) 
{
    Vector3 rowMultiplier(c0, c1, c2);
    return Matrix33(
        Mult( m.xAxis, rowMultiplier ),
        Mult( m.yAxis, rowMultiplier ),
        Mult( m.zAxis, rowMultiplier )
        );
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromDiagonal(VecFloat::InParam a) 
{
    VecFloat zero(GetVecFloat_Zero());
    return Matrix33(
            a, zero, zero,
         zero,    a, zero,
         zero, zero,    a
     );
}

RW_MATH_FORCE_INLINE Matrix33 Select(MaskScalar::InParam mask, Matrix33::InParam trueValue, Matrix33::InParam falseValue )
{
    VectorIntrinsic xRow, yRow, zRow;
    RW_MATH_VPU_Select( xRow, falseValue.xAxis.mV, trueValue.xAxis.mV, mask.mV );
    RW_MATH_VPU_Select( yRow, falseValue.yAxis.mV, trueValue.yAxis.mV, mask.mV );
    RW_MATH_VPU_Select( zRow, falseValue.zAxis.mV, trueValue.zAxis.mV, mask.mV );
    return Matrix33(xRow, yRow, zRow);
}

// operations that return by reference
#if RW_MATH_ENABLE_DEPRECATED_API

RW_MATH_FORCE_INLINE void Negate(Matrix33::InParam m, Matrix33 &result) 
{
    result = Negate(m);
}
RW_MATH_FORCE_INLINE void Mult(Matrix33::InParam m, float a, Matrix33 &result) 
{
    result = Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void Mult(Vector3::InParam v, Matrix33::InParam m, Vector3 &result) 
{
    result = Mult(v, m);
}

RW_MATH_FORCE_INLINE void Mult(Matrix33::InParam m, Matrix33::InParam b,    Matrix33 &result) 
{
    result = Mult(m,b);
}

template<VectorAxis INDEX> void Mult(Matrix33::InParam m, VecFloatRef<INDEX> a, Matrix33 &result)
{
    result = Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void Mult(Matrix33::InParam m, VecFloatRefIndex::InParam a, Matrix33 &result) 
{
    result = Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void Divide(Matrix33::InParam m, float a, Matrix33 &result) 
{
    result = Divide(m, VecFloat(a));
}

template<VectorAxis INDEX> void Divide(Matrix33::InParam m, VecFloatRef<INDEX> a, Matrix33 &result)
{
    result = Divide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void Divide(Matrix33::InParam m, VecFloatRefIndex::InParam a, Matrix33 &result) 
{
    result = Divide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE void Add(Matrix33::InParam a, Matrix33::InParam b, Matrix33 &result) 
{
    result = Add(a,b);
}

RW_MATH_FORCE_INLINE void Subtract(Matrix33::InParam a, Matrix33::InParam b, Matrix33 &result) 
{
    result = Subtract(a,b);
}

RW_MATH_FORCE_INLINE void Transpose(Matrix33::InParam m, Matrix33 &result) 
{
    result = Transpose(m);
}

RW_MATH_FORCE_INLINE VecFloat Inverse(Matrix33::InParam m, Matrix33 &result) 
{
    VecFloat determinant;
    result = Inverse(m, determinant);
    return determinant;
}

RW_MATH_FORCE_INLINE void  Matrix33FromAxisAngle(Vector3::InParam axis, VecFloat::InParam angle, Matrix33& result) 
{
    result = Matrix33FromAxisRotationAngle(axis, angle);
}

RW_MATH_FORCE_INLINE void Matrix33FromEulerXYZ(Vector3::InParam euler, Matrix33& result) 
{
    result = Matrix33FromEulerXYZ(euler);
}

RW_MATH_FORCE_INLINE void Matrix33FromQuaternion(Quaternion::InParam q, Matrix33& result) 
{
    result = Matrix33FromQuaternion(q);
}

RW_MATH_FORCE_INLINE void EulerXYZFromMatrix33(Matrix33::InParam m, Vector3& result) 
{
    result = EulerXYZFromMatrix33(m);
}


#endif

RW_MATH_FORCE_INLINE Matrix33& operator *=(Matrix33& m, VecFloat::InParam a) 
{
    m = Mult(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix33& operator /=(Matrix33& m, VecFloat::InParam a) 
{
    m = Divide(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix33 operator * (Matrix33::InParam m, VecFloat::InParam a) 
{
    return Mult(m, a);
}

RW_MATH_FORCE_INLINE Matrix33 operator * (VecFloat::InParam a, Matrix33::InParam b) 
{
    return Mult(b, a);
}

RW_MATH_FORCE_INLINE Matrix33 operator / (Matrix33::InParam m, VecFloat::InParam a) 
{
    return Divide(m, a);
}

Matrix33& operator *=(Matrix33& m, float a)
{
    return m *= VecFloat(a);
}

Matrix33& operator /=(Matrix33& m, float a)
{
    return m /= VecFloat(a);
}

Matrix33 operator * (Matrix33::InParam m, float a)
{
    return m * VecFloat(a);
}

Matrix33 operator * (float a, Matrix33::InParam b)
{
    return VecFloat(a) * b;
}

Matrix33 operator / (Matrix33::InParam m, float a)
{
    return m / VecFloat(a);
}

template<VectorAxis INDEX> Matrix33& operator *=(Matrix33& m, VecFloatRef<INDEX> a)
{
    return m *= VecFloat(a);
}

template<VectorAxis INDEX> Matrix33& operator /=(Matrix33& m, VecFloatRef<INDEX> a)
{
    return m /= VecFloat(a);
}

template<VectorAxis INDEX> Matrix33 operator * (Matrix33::InParam m, VecFloatRef<INDEX> a)
{
    return m * VecFloat(a);
}

template<VectorAxis INDEX> Matrix33 operator * (VecFloatRef<INDEX> a, Matrix33::InParam b)
{
    return VecFloat(a) * b;
}

template<VectorAxis INDEX> Matrix33 operator / (Matrix33::InParam m, VecFloatRef<INDEX> a)
{
    return m / VecFloat(a);
}

RW_MATH_FORCE_INLINE Matrix33& operator *=(Matrix33& m, VecFloatRefIndex::InParam a) 
{
    return m *= VecFloat(a);
}

RW_MATH_FORCE_INLINE Matrix33& operator /=(Matrix33& m, VecFloatRefIndex::InParam a) 
{
    return m /= VecFloat(a);
}

RW_MATH_FORCE_INLINE Matrix33 operator * (Matrix33::InParam m, VecFloatRefIndex::InParam a) 
{
    return m * VecFloat(a);
}

RW_MATH_FORCE_INLINE Matrix33 operator * (VecFloatRefIndex::InParam a, Matrix33::InParam b) 
{
    return VecFloat(a) * b;
}

RW_MATH_FORCE_INLINE Matrix33 operator / (Matrix33::InParam m, VecFloatRefIndex::InParam a) 
{
    return m / VecFloat(a);
}

RW_MATH_FORCE_INLINE Vector3 operator * (Vector3::InParam v, Matrix33::InParam m) 
{
    return Mult(v, m);
}

RW_MATH_FORCE_INLINE Matrix33& operator *=(Matrix33& m, Matrix33::InParam b) 
{
    m = Mult(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix33& operator +=(Matrix33& m, Matrix33::InParam b) 
{
    m = Add(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix33& operator -=(Matrix33& m, Matrix33::InParam b) 
{
    m = Subtract(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix33 operator - (Matrix33::InParam m)  
{
    return Negate(m);
}


RW_MATH_FORCE_INLINE Matrix33 operator + (Matrix33::InParam a, Matrix33::InParam b) 
{
    return Add(a, b);
}

RW_MATH_FORCE_INLINE Matrix33 operator - (Matrix33::InParam a, Matrix33::InParam b) 
{
    return Subtract(a, b);
}

RW_MATH_FORCE_INLINE Matrix33 operator * (Matrix33::InParam a, Matrix33::InParam b) 
{
    return Mult(a, b);
}

}}}

#endif
