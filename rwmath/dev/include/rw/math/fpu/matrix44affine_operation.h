// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX44AFFINE_OPERATION_H
#define RW_MATH_FPU_MATRIX44AFFINE_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/matrix33.h"
#include "rw/math/fpu/matrix44.h"
#include "rw/math/fpu/matrix44affine.h"
#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/vector3_operation.h"
#include "rw/math/fpu/vector4_operation.h"
#include "rw/math/fpu/quaternion_operation.h"
#include "rw/math/fpu/matrix33_operation.h"
#include "rw/math/fpu/matrix44_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T>& operator *=(Matrix44AffineTemplate<T>& m, const T& a)
{
    m = Mult(m, a);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T>& operator /=(Matrix44AffineTemplate<T>& m, const T& a)
{
    m = Divide(m, a);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator * (const Matrix44AffineTemplate<T>& m, const T& a)
{
    return Mult(m, a);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator * (const T& a, const Matrix44AffineTemplate<T>& b)
{
    return Mult(b, a);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator / (const Matrix44AffineTemplate<T>& m, const T& a)
{
    return Divide(m, a);
}

template<class T>
RW_MATH_INLINE Vector4Template<T> operator * (const Vector4Template<T>& v, const Matrix44AffineTemplate<T>& m)
{
    return Mult(v, m);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T>& operator *=(Matrix44AffineTemplate<T>& m, const Matrix44AffineTemplate<T>& b)
{
    m = Mult(m, b);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T>& operator +=(Matrix44AffineTemplate<T>& m, const Matrix44AffineTemplate<T>& b)
{
    m = Add(m, b);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T>& operator -=(Matrix44AffineTemplate<T>& m, const Matrix44AffineTemplate<T>& b)
{
    m = Subtract(m, b);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator - (const Matrix44AffineTemplate<T>& m)
{
    return Negate(m);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator + (const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return Add(a, b);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator - (const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return Subtract(a, b);
}


template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> operator * (const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return Mult(a, b);
}

template<class T>
RW_MATH_INLINE bool operator ==(const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return  a.xAxis == b.xAxis &&
            a.yAxis == b.yAxis &&
            a.zAxis == b.zAxis &&
            a.wAxis == b.wAxis;
}

template<class T>
RW_MATH_INLINE bool operator !=(const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return !(a==b);
}

template<class T>
RW_MATH_INLINE bool IsSimilar(const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b, T epsilon = SMALL_FLOAT)
{
    return  IsSimilar( a.xAxis, b.xAxis, epsilon ) &&
            IsSimilar( a.yAxis, b.yAxis, epsilon ) &&
            IsSimilar( a.zAxis, b.zAxis, epsilon ) &&
            IsSimilar( a.wAxis, b.wAxis, epsilon );
}

/*!
 * Determine if any element of the matrix is NaN.
 */
template<class T> RW_MATH_INLINE bool IsAnyNaN(const Matrix44AffineTemplate<T> &m)
{
    return IsAnyNaN(m.xAxis) || IsAnyNaN(m.yAxis) || IsAnyNaN(m.zAxis) || IsAnyNaN(m.wAxis);
}

template<class T>
RW_MATH_INLINE T GetOrthogonalError3x3(const Matrix44AffineTemplate<T>& matrix)
{
    Matrix33Template<T> m33(
        matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
        matrix.GetElem(3), matrix.GetElem(4), matrix.GetElem(5),
        matrix.GetElem(6), matrix.GetElem(7), matrix.GetElem(8));
    return GetOrthogonalError( m33 );
}

template<class T>
RW_MATH_INLINE T GetOrthogonalRowSetError3x3(const Matrix44AffineTemplate<T>& matrix)
{
    Matrix33Template<T> m33(
        matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
        matrix.GetElem(3), matrix.GetElem(4), matrix.GetElem(5),
        matrix.GetElem(6), matrix.GetElem(7), matrix.GetElem(8));
    return GetOrthogonalRowSetError( m33 );
}

template<class T>
RW_MATH_INLINE T GetNormalError(const Matrix44AffineTemplate<T>& matrix)
{
    Vector4Template<T> errorVec( MagnitudeSquared( matrix.XAxis() ) - T(1.0f),
                                                MagnitudeSquared( matrix.YAxis() ) - T(1.0f),
                                                MagnitudeSquared( matrix.ZAxis() ) - T(1.0f),
                                                MagnitudeSquared( matrix.WAxis() ) );
    T error = MagnitudeSquared(errorVec);
    return error;
}

template<class T>
RW_MATH_INLINE bool IsValid(const Matrix44AffineTemplate<T>& a)
{   
    return  IsValid(a.XAxis()) &&
            IsValid(a.YAxis()) &&
            IsValid(a.ZAxis()) &&
            IsValid(a.WAxis());
}

template<class T>
RW_MATH_INLINE bool IsNormal(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetNormalError(a), tolerance );
}

template<class T>
RW_MATH_INLINE bool IsOrthogonal(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetOrthogonalError(a), tolerance );
}

template<class T>
RW_MATH_INLINE bool IsOrthogonalRowSet(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetOrthogonalRowSetError(a), tolerance );
}

/*!
Test whether the matrix is a rotation matrix.  

Note: This function returns true only if and only if this matrix is rotation
matrix in a strict mathematical sense.  So, an affine matrix with a translation
would not be considered a rotation matrix, but a matrix that does a 4D rotation
would still be considered a rotation.  If you only want to test the upper left 3x3
submatrix, you may want to use \c IsRotation3x3 instead.

\param a The matrix to perform the operation on.
\param tolerance The allowable difference between expected and actual result.

\return True if \c a is a rotation matrix, otherwise false.
*/
template<class T> RW_MATH_INLINE bool IsRotation(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsOrthogonal(a, tolerance) && IsSimilar(Determinant(a), static_cast<T>(1.0f), tolerance);
}

template<class T>
RW_MATH_INLINE void SelfAdd(Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    a = Add(a, b);
}

template<class T>
RW_MATH_INLINE void SelfSubtract(Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    a = Subtract(a, b);
}

template<class T>
RW_MATH_INLINE void SelfMult(Matrix44AffineTemplate<T>& m, T a)
{
    m = Mult(m, a);
}

template<class T>
RW_MATH_INLINE void SelfDivide(Matrix44AffineTemplate<T>& m, T a)
{
    m = Divide(m, a);
}

template<class T>
RW_MATH_INLINE T Determinant(const Matrix44AffineTemplate<T>& m)
{
    return  
        m.xAxis.x * (
        m.yAxis.y * m.zAxis.z -
        m.yAxis.z * m.zAxis.y 
        ) + 
        m.xAxis.y * (
        m.yAxis.z * m.zAxis.x -
        m.yAxis.x * m.zAxis.z
        ) + 
        m.xAxis.z * (
        m.yAxis.x * m.zAxis.y -
        m.yAxis.y * m.zAxis.x
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Negate(const Matrix44AffineTemplate<T>& m)
{
    return Matrix44AffineTemplate<T>( Negate(m.xAxis), Negate(m.yAxis), Negate(m.zAxis), Negate(m.wAxis) );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Mult(const Matrix44AffineTemplate<T>& m, T a)
{
    return Matrix44AffineTemplate<T>(
        a * m.xAxis,
        a * m.yAxis,
        a * m.zAxis,
        a * m.wAxis
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Divide(const Matrix44AffineTemplate<T>& m, T a)
{
    return Matrix44AffineTemplate<T>(
        m.xAxis / a,
        m.yAxis / a,
        m.zAxis / a,
        m.wAxis / a
        );
}

template<class T>
RW_MATH_INLINE Vector4Template<T> Mult(const Vector4Template<T>& v, const Matrix44AffineTemplate<T>& m)
{
    return Vector4Template<T>(
        m.xAxis.x * v.x + m.yAxis.x * v.y + m.zAxis.x * v.z + m.wAxis.x * v.w,
        m.xAxis.y * v.x + m.yAxis.y * v.y + m.zAxis.y * v.z + m.wAxis.y * v.w,
        m.xAxis.z * v.x + m.yAxis.z * v.y + m.zAxis.z * v.z + m.wAxis.z * v.w,
        v.w
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Mult(const Matrix44AffineTemplate<T>& m, const Matrix44AffineTemplate<T>& b)
{
    return Matrix44AffineTemplate<T>(
        TransformVector(m.xAxis, b),
        TransformVector(m.yAxis, b),
        TransformVector(m.zAxis, b),
        TransformPoint(m.wAxis, b)
        );
}


template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Add(const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return Matrix44AffineTemplate<T>(
        Add(a.xAxis, b.xAxis),
        Add(a.yAxis, b.yAxis),
        Add(a.zAxis, b.zAxis),
        Add(a.wAxis, b.wAxis)
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Subtract(const Matrix44AffineTemplate<T>& a, const Matrix44AffineTemplate<T>& b)
{
    return Matrix44AffineTemplate<T>(
        Subtract(a.xAxis, b.xAxis),
        Subtract(a.yAxis, b.yAxis),
        Subtract(a.zAxis, b.zAxis),
        Subtract(a.wAxis, b.wAxis)
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Transpose(const Matrix44AffineTemplate<T>& m)
{
    Matrix44AffineTemplate<T> result;
    // Transpose upper 3x3
    for(int row=0; row<3; ++row)
    {
        for(int column=0; column<3; ++column)
        {
            result.SetElem(row, column, m.GetRow(column).GetComponent( row) );
        }
    }
    // Set the last row to (0 0 0 1) since the last column of the input matrix is assumed to be (0 0 0 1).
    // Of course, to set this value we only need to set the last row to (0 0 0) since the upper right element
    // of an affine matrix is assumed to be 1.
    result.WAxis() = Vector3Template<T>(0.0f, 0.0f, 0.0f) ;
    return result;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> OrthoNormalize(const Matrix44AffineTemplate<T>& m)
{
    // Note: This implementation is using the Gram-Schmidt orthonormalization
    // algorithm.  The current implementation is numerically unstable. Also,
    // this algorithm differs from the algorithm implemented for matrix 3x3
    // orthonormalization.
    Matrix44AffineTemplate<T> result;

    // First, set the last row to (0,0,0,1).  This is the only value for the last row that will
    // allow us to create an ortho-normal matrix that is also Affine.
    result.WAxis() =Vector3Template<T>(0.0f, 0.0f, 0.0f);

    // Loop over rows 2, 1, and 0
    for( int row = 2; row >= 0; --row )
    {
        // Initialize the row to the same row from the input matrix.
        const Vector3Template<T> &inputRow = m.GetRow(row);
        Vector3Template<T> tempResult = inputRow;
        // Subtract the components of the vector that are not orthogonal to the basis created thus far.
        // Note: By only operating on Vector3's, we eliminate the need to remove the contribution
        // of the last row in the matrix.
        //
        // "Handwaving Proof"
        //   by Dave Cope
        // let a = (x y z w)
        // Now consider what happens when we subtract the component of 'a' that is not orthogonal to
        // the last row (0 0 0 1).
        //
        // result = a - (0 0 0 1) * Dot(a, (0 0 0 1))
        //        = (x y z w) - (0 0 0 1) * Dot((x y z w), (0 0 0 1))
        //        = (x y z w) - (0 0 0 1) * w
        //        = (x y z w) - (0 0 0 w)
        //        = (x y z 0)
        // Since all other row vectors have their w components fixed at 0, and normalizing a vector4
        // with w=0 is equivalent to normalizing a vector3 and appending a zero, we can operate on Vector3's
        // instead of Vector4's.
        for( int i = 2; i > row; --i )
        {
            tempResult -= result.GetRow(i) * Dot(result.GetRow(i), inputRow );
        }
        result.SetRow( row, Normalize(tempResult) );
    }
    return result;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> InverseOrthonormal(const Matrix44AffineTemplate<T>& m)
{
    RW_MATH_ASSERT_MSG((IsOrthogonal<T>(m)), ("Input matrix is non orthonormal."));
    return Transpose(m);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Inverse(const Matrix44AffineTemplate<T>& m)
{
    T determinant;
    return Inverse(m, determinant); 
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Inverse(const Matrix44AffineTemplate<T>& m, T& determinant)
{
    determinant = Determinant(m);
    RW_MATH_ASSERT_MSG((determinant != 0.0f), ("Matrix is singular."));
    if(determinant==T(0.0f)) 
    {
        return Matrix44AffineTemplate<T>();
    }
    else
    {
        // \todo Optimize by eliminating unnecessary operations involving 0 and 1.
        T determinantRecip = Reciprocal( determinant );
        return Matrix44AffineTemplate<T>(
                ((m.yAxis.y*(m.zAxis.z)+m.yAxis.z*(-m.zAxis.y))*determinantRecip),
                ((m.zAxis.y*(m.xAxis.z)+m.zAxis.z*(-m.xAxis.y))*determinantRecip),
                ((m.xAxis.y*m.yAxis.z-m.xAxis.z*m.yAxis.y)*determinantRecip),

                ((m.yAxis.z*(m.zAxis.x)+m.yAxis.x*(-m.zAxis.z))*determinantRecip),
                ((m.zAxis.z*(m.xAxis.x)+m.zAxis.x*(-m.xAxis.z))*determinantRecip),
                ((m.yAxis.x*m.xAxis.z-m.xAxis.x*m.yAxis.z)*determinantRecip),

                ((m.yAxis.x*(m.zAxis.y)+m.yAxis.y*(-m.zAxis.x))*determinantRecip),
                ((m.zAxis.x*(m.xAxis.y)+m.zAxis.y*(-m.xAxis.x))*determinantRecip),
                ((m.xAxis.x*m.yAxis.y-m.yAxis.x*m.xAxis.y)*determinantRecip),

                ((m.yAxis.x*(m.wAxis.y*m.zAxis.z-m.zAxis.y*m.wAxis.z)+m.yAxis.y*(m.zAxis.x*m.wAxis.z-m.wAxis.x*m.zAxis.z)+m.yAxis.z*(m.wAxis.x*m.zAxis.y-m.zAxis.x*m.wAxis.y))*determinantRecip),
                ((m.zAxis.x*(m.xAxis.z*m.wAxis.y-m.xAxis.y*m.wAxis.z)+m.zAxis.y*(m.xAxis.x*m.wAxis.z-m.wAxis.x*m.xAxis.z)+m.zAxis.z*(m.wAxis.x*m.xAxis.y-m.xAxis.x*m.wAxis.y))*determinantRecip),
                ((m.wAxis.x*(m.xAxis.z*m.yAxis.y-m.xAxis.y*m.yAxis.z)+m.wAxis.y*(m.xAxis.x*m.yAxis.z-m.yAxis.x*m.xAxis.z)+m.wAxis.z*(m.yAxis.x*m.xAxis.y-m.xAxis.x*m.yAxis.y))*determinantRecip)
            );
    }
}

template<class T>
RW_MATH_INLINE Vector3Template<T> TransformPoint(const Vector3Template<T>& pt, const Matrix44AffineTemplate<T>& matrix)
{
    return Vector3Template<T>(matrix.xAxis.x * pt.x + matrix.yAxis.x * pt.y + matrix.zAxis.x * pt.z + matrix.wAxis.x,
                              matrix.xAxis.y * pt.x + matrix.yAxis.y * pt.y + matrix.zAxis.y * pt.z + matrix.wAxis.y,
                              matrix.xAxis.z * pt.x + matrix.yAxis.z * pt.y + matrix.zAxis.z * pt.z + matrix.wAxis.z);
}

template<class T>
RW_MATH_INLINE void TransformPoints(const Vector3Template<T>* const pts, int numPoints, const Matrix44AffineTemplate<T>& matrix, Vector3Template<T>* transformedPts)
{
    for( int point = 0; point < numPoints; point++ )
    {
        transformedPts[point] = TransformPoint(pts[point], matrix);
    }
}

template<class T>
RW_MATH_INLINE Vector3Template<T> TransformVector(const Vector3Template<T>& vec, const Matrix44AffineTemplate<T>& matrix)
{
    Vector3Template<T>transformedPoint = matrix.XAxis() * vec.x;
    transformedPoint += matrix.YAxis() * vec.y;
    transformedPoint += matrix.ZAxis() * vec.z;
    return transformedPoint;
}

template<class T>
RW_MATH_INLINE void TransformVectors(const Vector3Template<T>* const vecs, int numVectors, const Matrix44AffineTemplate<T>& matrix, Vector3Template<T> * transformedVecs)
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = TransformVector(vecs[vecIndex], matrix);
    }
}

template<class T>
RW_MATH_INLINE Vector3Template<T> Transform(const Vector4Template<T>& vec, const Matrix44AffineTemplate<T>& matrix)
{
    Vector3Template<T> transformedVector = matrix.XAxis() * vec.x;
    transformedVector += matrix.YAxis() * vec.y;
    transformedVector += matrix.ZAxis() * vec.z;
    transformedVector += matrix.WAxis() * vec.w;;
    return transformedVector;
}

template<class T>
RW_MATH_INLINE void Transform(const Vector4Template<T>* const vecs, int numVectors, const Matrix44AffineTemplate<T>& matrix, Vector3Template<T> * transformedVecs)
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = Transform(vecs[vecIndex], matrix);
    }
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromQuaternion(const QuaternionTemplate<T>&q)
{
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T yz = q.y * q.z;
    T wx = q.w * q.x;
    T wy = q.w * q.y;
    T wz = q.w * q.z;

    const T zero(0.0f);
    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
        one-two*(yy+zz), two*(xy+wz),     two*(xz-wy),
        two*(xy-wz),     one-two*(xx+zz), two*(yz+wx),
        two*(xz+wy),     two*(yz-wx),     one-two*(xx+yy),
        zero,            zero,            zero
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromEulerXYZ(const Vector3Template<T>& euler)
{
    T   ci, cj, ch, si, sj, sh;

    SinCos(euler.x, si, ci);
    SinCos(euler.y, sj, cj);
    SinCos(euler.z, sh, ch);

    T cc = ci * ch;
    T cs = ci * sh;
    T sc = si * ch;
    T ss = si * sh;

    const T zero(0.0f);
    return Matrix44AffineTemplate<T>(
        cj * ch     ,  cj * sh     , -sj      ,
        sj * sc - cs,  sj * ss + cc,  cj * si ,
        sj * cc + ss,  sj * cs - sc,  cj * ci ,
        zero, zero, zero
        );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> EulerXYZFromMatrix44Affine(const Matrix44AffineTemplate<T>& matrix)
{
    Matrix33Template<T> m33(
        matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
        matrix.GetElem(3), matrix.GetElem(4), matrix.GetElem(5),
        matrix.GetElem(6), matrix.GetElem(7), matrix.GetElem(8));
    return EulerXYZFromMatrix33( m33 );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromAxisRotationAngle(const Vector3Template<T>& axis, T angle)
{
    T s, c;

    SinCos(angle, s, c);
    T t = T(1.0f) - c;
    T tx = t * axis.x;    
    T ty = t * axis.y;    
    T tz = t * axis.z;    
    T sx = s * axis.x;
    T sy = s * axis.y;
    T sz = s * axis.z;

    const T zero(0.0f);
    return Matrix44AffineTemplate<T>(
        tx * axis.x + c ,   
        tx * axis.y + sz,   
        tx * axis.z - sy,

        ty * axis.x - sz,   
        ty * axis.y + c ,   
        ty * axis.z + sx,   

        tz * axis.x + sy,   
        tz * axis.y - sx,   
        tz * axis.z + c,

        zero,
        zero,
        zero
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromXRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix44AffineTemplate<T>(
        one,    zero,   zero,
        zero,  c,       s,
        zero,  -s,      c,
        zero,  zero,   zero
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromYRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix44AffineTemplate<T>(
        c,      zero,   -s,
        zero,  one, zero,
        s,      zero,   c,
        zero,  zero,   zero );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromZRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix44AffineTemplate<T>(
        c,      s,      zero,
        -s,     c,      zero,
        zero,  zero,   one,
        zero,  zero,   zero);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> RotateAroundXAxis(const Matrix44AffineTemplate<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labelled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //    [         e00                e01                e02        0 ]
    //    [ e20 SIN + e10 COS  e21 SIN + e11 COS  e22 SIN + e12 COS  0 ]
    //    [ e20 COS - e10 SIN  e21 COS - e11 SIN  e22 COS - e12 SIN  0 ]
    //    [         e30                e31                e32        1 ]
    T s,c;
    SinCos(angle, s, c);

    // rotate incrementally about x
    return Matrix44AffineTemplate<T>(
        m.XAxis(),
        m.ZAxis() * s + m.YAxis() * c,
        m.ZAxis() * c - m.YAxis() * s,
        m.WAxis()
    );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> RotateAroundYAxis(const Matrix44AffineTemplate<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labelled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e00 COS - e20 SIN  e01 COS - e21 SIN  e02 COS - e22 SIN  0 ]
    //  [         e10                e11                e12        0 ]
    //  [ e00 SIN + e20 COS  e01 SIN + e21 COS  e02 SIN + e22 COS  0 ]
    //  [         e30                e31                e32        1 ]
    T s,c;

    SinCos(angle, s, c);

    // rotate incrementally about y
    return Matrix44AffineTemplate<T>(
        m.XAxis() * c - m.ZAxis() * s,
        m.YAxis(),
        m.XAxis() * s + m.ZAxis() * c,
        m.WAxis()
    );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> RotateAroundZAxis(const Matrix44AffineTemplate<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labelled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e10 SIN + e00 COS  e11 SIN + e01 COS  e12 SIN + e02 COS  0 ]
    //  [ e10 COS - e00 SIN  e11 COS - e01 SIN  e12 COS - e02 SIN  0 ]
    //  [         e20                e21                e22        0 ]
    //  [         e30                e31                e32        1 ]
    T s,c;

    SinCos(angle, s, c);

    // rotate incrementally about z
    return Matrix44AffineTemplate<T>(
        m.YAxis() * s + m.XAxis() * c,
        m.YAxis() * c - m.XAxis() * s,
        m.ZAxis(),
        m.WAxis()
    );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineTransposeRotationalComponent(const Matrix44AffineTemplate<T>& m)
{
    Matrix44AffineTemplate<T> result = m;

    // transpose rotation component of matrix
    result.SetElem(1, m.GetElem(3));
    result.SetElem(3, m.GetElem(1));

    result.SetElem(2, m.GetElem(6));
    result.SetElem(6, m.GetElem(2));

    result.SetElem(5, m.GetElem(7));
    result.SetElem(7, m.GetElem(5));
    return result;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineTranslate(const Matrix44AffineTemplate<T>& m, const Vector3Template<T>& trans)
{
    // multiply translations by current matrix
    Vector3Template<T> translationVector(
        (trans.x * m.GetElem(0)) + (trans.y * m.GetElem(3)) + (trans.z * m.GetElem(6)),
        (trans.x * m.GetElem(1)) + (trans.y * m.GetElem(4)) + (trans.z * m.GetElem(7)),
        (trans.x * m.GetElem(2)) + (trans.y * m.GetElem(5)) + (trans.z * m.GetElem(8))
        );

    // and translate current matrix by rotated translation    
    return Matrix44AffineTemplate<T>(
        m.xAxis,
        m.yAxis,
        m.zAxis,
        m.wAxis + translationVector
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> ScaleRow(const Matrix44AffineTemplate<T>& m, T r0, T r1, T r2)
{
    Vector3Template<T> row0Scale( r0, r0, r0 );
    Vector3Template<T> row1Scale( r1, r1, r1 );
    Vector3Template<T> row2Scale( r2, r2, r2 );
    return Matrix44AffineTemplate<T>(
        Mult( row0Scale, m.xAxis ),
        Mult( row1Scale, m.yAxis ),
        Mult( row2Scale, m.zAxis ),
        m.wAxis
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> ScaleColumn(const Matrix44AffineTemplate<T>& m, T c0, T c1, T c2)
{
    Vector3Template<T> rowMultiplier(c0, c1, c2);
    return Matrix44AffineTemplate<T>(
        Mult( m.xAxis, rowMultiplier ),
        Mult( m.yAxis, rowMultiplier ),
        Mult( m.zAxis, rowMultiplier ),
        m.wAxis
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromDiagonal(T a)
{
    return Matrix44AffineTemplate<T>(
        a,0,0,
        0,a,0,
        0,0,a,
        0,0,0
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromScale(const Vector3Template<T>& scale)
{
    const T zero(0.0f);
    return Matrix44AffineTemplate<T>(
        scale.x, zero, zero,
        zero, scale.y, zero,
        zero, zero, scale.z,
        zero, zero, zero
        );
}

/*!
 * Creates a transformation matrix that represents the translation specified by \c trans.
 */
template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromTranslation(const Vector3Template<T>& trans)
{
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix44AffineTemplate<T>(
        one,       zero,       zero,
        zero,       one,       zero,
        zero,       zero,       one,
        trans.x, trans.y, trans.z
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromScaleQuaternion(const Vector3Template<T>& scale, const QuaternionTemplate<T>&q)
{
    const T zero(0.0f);
    const T one(1.0f);
    const T two(2.0f);
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T yz = q.y * q.z;
    T wx = q.w * q.x;
    T wy = q.w * q.y;
    T wz = q.w * q.z;
    return Matrix44AffineTemplate<T>(
        scale.x * (one-two*(yy+zz)), scale.x *two*(xy+wz), scale.x *two*(xz-wy),
        scale.y *two*(xy-wz), scale.y * (one-two*(xx+zz)), scale.y *two*(yz+wx),
        scale.z *two*(xz+wy), scale.z *two*(yz-wx), scale.z * (one-two*(xx+yy)),
        zero,                 zero,               zero
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromScaleTranslation(const Vector3Template<T>& scale, const Vector3Template<T>& trans)
{
    const T zero(0.0f);
    return Matrix44AffineTemplate<T>(
        scale.x, zero, zero,
        zero, scale.y, zero,
        zero, zero, scale.z,
        trans.x, trans.y, trans.z
        );
}

/*!
 * \brief Creates an orthographic projection matrix for a left handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param w width of view frustum
 * \param h height of view frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromOrthoLH(T w, T h, T zn, T zf)
{
    const T zero(0.0f);
    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
       two/w,   zero,          zero,
        zero,  two/h,          zero,
        zero,   zero,   one/(zf-zn),
        zero,   zero,   -zn/(zf-zn)
        );
}

/*!
 * \brief Creates an orthographic projection matrix for a right handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param w width of view frustum
 * \param h height of view frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromOrthoRH(T w, T h, T zn, T zf)
{
    const T zero(0.0f);
    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
        two/w,  zero,   zero,
        zero,   two/h,  zero,
        zero,   zero,   one/(zn-zf),
        zero,   zero,   zn/(zn-zf)
        );
}

/*!
 * \brief Creates an orthographic projection matrix for a right handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param w width of view frustum
 * \param h height of view frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_FORCE_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromOrtho(T w, T h, T zn, T zf)
{
    return Matrix44AffineFromOrthoRH(w, h, zn, zf);
}

/*!
 * \brief Creates an off center orthographic projection matrix for a left handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param l left most (i.e. minimum) x value of view frustum
 * \param r right most (i.e. maximum) x value of view frustum
 * \param b bottom most (i.e. minimum) y value of view frustum
 * \param t top most (i.e. maximum) y value of view frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromOrthoOffCenterLH(T l, T r, T b, T t, T zn, T zf)
{
    const T zero(0.0f);
    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
        two/(r-l),           zero,          zero,
             zero,      two/(t-b),          zero,
             zero,           zero,   one/(zf-zn),
      (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf)
        );
}

/*!
 * \brief Creates an off center orthographic projection matrix for a right handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param l left most (i.e. minimum) x value of view frustum
 * \param r right most (i.e. maximum) x value of view frustum
 * \param b bottom most (i.e. minimum) y value of view frustum
 * \param t top most (i.e. maximum) y value of view frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromOrthoOffCenterRH(T l, T r, T b, T t, T zn, T zf)
{
    const T zero(0.0f);
    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
        two/(r-l),      zero,           zero,
        zero,           two/(t-b),      zero,
        zero,           zero,           one/(zn-zf),
        (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf)
        );
}

/*!
 * \brief Creates an off center orthographic projection matrix for a right handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param l left most (i.e. minimum) x value of view frustum
 * \param r right most (i.e. maximum) x value of view frustum
 * \param b bottom most (i.e. minimum) y value of view frustum
 * \param t top most (i.e. maximum) y value of view frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_FORCE_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromOrthoOffCenter(T l, T r, T b, T t, T zn, T zf)
{
    return Matrix44AffineFromOrthoOffCenterRH(l, r, b, t, zn, zf);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromQuaternionTranslation(const QuaternionTemplate<T>&q, const Vector3Template<T>& trans)
{
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T yz = q.y * q.z;
    T wx = q.w * q.x;
    T wy = q.w * q.y;
    T wz = q.w * q.z;

    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
        one-two*(yy+zz),    two*(xy+wz),        two*(xz-wy),
        two*(xy-wz),        one-two*(xx+zz),    two*(yz+wx),
        two*(xz+wy),        two*(yz-wx),        one-two*(xx+yy),
        trans.x,        trans.y,        trans.z
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromScaleQuaternionTranslation(const Vector3Template<T>& scale, const QuaternionTemplate<T>&q, const Vector3Template<T>& trans)
{
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T yz = q.y * q.z;
    T wx = q.w * q.x;
    T wy = q.w * q.y;
    T wz = q.w * q.z;

    const T one(1.0f);
    const T two(2.0f);
    return Matrix44AffineTemplate<T>(
        scale.x*(one-two*(yy+zz)), scale.x*two*(xy+wz),     scale.x*two*(xz-wy),
        scale.y*two*(xy-wz),        scale.y*(one-two*(xx+zz)),  scale.y*two*(yz+wx),
        scale.z*two*(xz+wy),        scale.z*two*(yz-wx),        scale.z*(one-two*(xx+yy)),
        trans.x,                    trans.y,                    trans.z
        );
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromLookAt(const Vector3Template<T>& lookAt, const Vector3Template<T>& up, const Vector3Template<T>& eyePosition)
{
    // calculate the 3 axes
    Vector3Template<T>  zaxis = Normalize(Subtract(eyePosition, lookAt));
    Vector3Template<T>  xaxis = Normalize(Cross(up, zaxis));
    Vector3Template<T>  yaxis = Cross(zaxis, xaxis);

    return Matrix44AffineTemplate<T> (
        xaxis.x, yaxis.x, zaxis.x,
        xaxis.y, yaxis.y, zaxis.y,
        xaxis.z, yaxis.z, zaxis.z,
        -Dot(xaxis, eyePosition), -Dot(yaxis, eyePosition), -Dot(zaxis, eyePosition)
        );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> QueryRotateDegenerateUnitAxis(const Matrix44AffineTemplate<T>& matrix)
{
    Matrix33Template<T> m33(
        matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
        matrix.GetElem(3), matrix.GetElem(4), matrix.GetElem(5),
        matrix.GetElem(6), matrix.GetElem(7), matrix.GetElem(8));
    return QueryRotateDegenerateUnitAxis( m33 );
}

template<class T>
RW_MATH_INLINE void QueryRotate(const Matrix44AffineTemplate<T>& matrix, Vector3Template<T>& unitAxis, T& angle, Vector3Template<T>& center)
{
    Vector3Template<T> twoSinThetaAxis;
    T twoCosTheta = matrix.XAxis().x + matrix.YAxis().y + matrix.ZAxis().z - T(1.0f);
    twoSinThetaAxis.x =matrix.YAxis().z - matrix.ZAxis().y;
    twoSinThetaAxis.y =matrix.ZAxis().x - matrix.XAxis().z;
    twoSinThetaAxis.z =matrix.XAxis().y - matrix.YAxis().x;
    T twoSinTheta = Magnitude(twoSinThetaAxis);
    if(twoSinTheta > T(0.0f))
    {
        const T lrRecipLength = T(1.0f) / twoSinTheta;
        unitAxis = twoSinThetaAxis * lrRecipLength;
    }
    else
    {
        unitAxis = Vector3Template<T>(0.0f, 0.0f, 0.0f);
    }
    angle = ATan2(twoSinTheta, twoCosTheta);
    if((twoSinTheta <= T(MINIMUM_RECIPROCAL)) && (twoCosTheta <= T(0.0f)))
    {
        unitAxis = QueryRotateDegenerateUnitAxis(matrix);
    }
    Vector3Template<T> axisXpos = Cross(unitAxis, matrix.WAxis());
    center = Cross(axisXpos, unitAxis) * T(0.5f);
    center += axisXpos * (T(0.5f) * twoSinTheta / (T(2.0f) - twoCosTheta));
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> SLerp(const Matrix44AffineTemplate<T>& lFrom, const Matrix44AffineTemplate<T>& lTo, T lrParam, T& lprAngle)
{
    Matrix44AffineTemplate<T> lReturnmatrix;
    Matrix44AffineTemplate<T> lScratch;
    Vector3Template<T>  axis;
    Vector3Template<T>  lCentre;
    T lrAngle;

    // Do we need to interpolate?
    if(lrParam >= T(1.0f))
    {
        lprAngle = T(0.0f);
        return lTo;
    }

    // Interpolate - lerp or slerp?
    lScratch = Mult( InverseOrthonormal(lFrom), lTo );

    QueryRotate(lScratch, axis, lrAngle, lCentre );

    if(lrParam <= T(0.0f))
    {
        lprAngle = lrAngle;
        return lFrom;
    }

    // Lerp? (faster than slerp)
    if( lrAngle < T(MATRIX_MIN_SLERP_ANGLE) )
    {
        // Get lerp matrix
        Matrix44AffineTemplate<T> lLerpmat;

        lLerpmat.XAxis() = lTo.XAxis() - lFrom.XAxis();
        lLerpmat.YAxis() = lTo.YAxis() - lFrom.YAxis();
        lLerpmat.ZAxis() = lTo.ZAxis() - lFrom.ZAxis();
        lLerpmat.WAxis() = lTo.WAxis() - lFrom.WAxis();

        // Do lerp
        lLerpmat = Mult( lLerpmat, lrParam);

        lReturnmatrix.XAxis() = Normalize(lFrom.XAxis() + lLerpmat.XAxis());
        lReturnmatrix.YAxis() = Normalize(lFrom.YAxis() + lLerpmat.YAxis());
        lReturnmatrix.ZAxis() = Normalize(lFrom.ZAxis() + lLerpmat.ZAxis());
        lReturnmatrix.WAxis() = lFrom.WAxis() + lLerpmat.WAxis();
    }
    else
        // SLerp
    {
        Matrix44AffineTemplate<T> lRotatematrix;

        Vector3Template<T> lStartPos = lFrom.WAxis();
        Vector3Template<T> lEndPos = lTo.WAxis();

        // Remove the translation for now
        lReturnmatrix = lFrom;
        lReturnmatrix.WAxis() =GetVector3_ZeroTemplate<T>();

        // Rotate the new matrix
        lRotatematrix = Matrix44AffineFromAxisRotationAngle( axis, ( lrAngle * lrParam ) );
        lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

        // Do linear interpolation on position
        lReturnmatrix.WAxis() = (lEndPos - lStartPos) * lrParam + lStartPos;
    }

    lprAngle = lrAngle - (lrAngle * lrParam);
    return lReturnmatrix;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> SLerpByAngle(const Matrix44AffineTemplate<T>& lFrom, const Matrix44AffineTemplate<T>& lTo, T lrMaxAngle, T& lprAngle)
{
    Matrix44AffineTemplate<T> lReturnmatrix;
    Matrix44AffineTemplate<T> lRotatematrix;
    Vector3Template<T>  axis;
    Vector3Template<T>  lCentre;
    T lrAngle;
    T lrDeltaAngle;

    // Get axis and angle between From and To
    Matrix44AffineTemplate<T> lScratch = InverseOrthonormal(lFrom);
    lScratch = Mult( lScratch, lTo );

    QueryRotate(lScratch, axis, lrAngle, lCentre );
    if( IsZero(axis) )
    {
        lprAngle = 0.0f;
        return lTo;
    }

    // Copy from and remove translation
    Vector3Template<T> lPosBackup = lFrom.WAxis();
    lReturnmatrix = lFrom;
    lReturnmatrix.WAxis() = GetVector3_ZeroTemplate<T>() ;

    // Rotate about axis by no more than maximum specified angle
    lrDeltaAngle = Min(lrAngle, lrMaxAngle);

    lRotatematrix = Matrix44AffineFromAxisRotationAngle( axis, lrDeltaAngle );
    lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

    // Put back original translation
    lReturnmatrix.WAxis() = lPosBackup ;

    lprAngle = lrAngle - lrDeltaAngle;

    return lReturnmatrix;
}

// operating as Matrix33
template<class T>
RW_MATH_INLINE bool IsValid3x3(const Matrix44AffineTemplate<T>& a)
{
    Matrix33Template<T> m33(
        a.GetElem(0), a.GetElem(1), a.GetElem(2),
        a.GetElem(3), a.GetElem(4), a.GetElem(5),
        a.GetElem(6), a.GetElem(7), a.GetElem(8));
    return IsValid( m33 );
}

template<class T>
RW_MATH_INLINE bool IsNormal3x3(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetNormalError3x3(a), tolerance );
}

template<class T>
RW_MATH_INLINE bool IsOrthogonal3x3(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetOrthogonalError3x3(a), tolerance );
}

template<class T>
RW_MATH_INLINE bool IsOrthogonalRowSet3x3(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetOrthogonalRowSetError3x3(a), tolerance );
}

template<class T>
RW_MATH_INLINE T Determinant3x3(const Matrix44AffineTemplate<T>& m)
{
    Matrix33Template<T> m33(
        m.GetElem(0), m.GetElem(1), m.GetElem(2),
        m.GetElem(3), m.GetElem(4), m.GetElem(5),
        m.GetElem(6), m.GetElem(7), m.GetElem(8));
    return Determinant( m33 );
}

template<class T>
RW_MATH_INLINE Vector4Template<T> Mult3x3(const Vector4Template<T>& v, const Matrix44AffineTemplate<T>& m)
{
    Vector3Template<T> v3_arg(v.x, v.y, v.z);
    Vector3Template<T> v3 = Mult3x3(v3_arg, m);
    return Vector4Template<T>( v3.x, v3.y, v3.z, v.w );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> Mult3x3(const Vector3Template<T>& v, const Matrix44AffineTemplate<T>& m)
{
    Matrix33Template<T> m33(
        m.GetElem(0), m.GetElem(1), m.GetElem(2),
        m.GetElem(3), m.GetElem(4), m.GetElem(5),
        m.GetElem(6), m.GetElem(7), m.GetElem(8));
    return Mult(v,m33);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Inverse3x3(const Matrix44AffineTemplate<T>& m, T& determinant)
{
    determinant = Determinant(m);
    if(determinant==T(0.0f)) 
    {
        return Matrix44AffineTemplate<T>();
    }
    else
    {
        Matrix44AffineTemplate<T> result;
        for(int i=0; i<3; ++i)
        {
            for(int j=0; j<3; ++j)
            {
                int32_t i1=(i+1)%3;
                int32_t i2=(i+2)%3;
                int32_t j1=(j+1)%3;
                int32_t j2=(j+2)%3;
                result.SetElem(j,i,
                    (m.GetRow(i1).GetComponent( j1)*m.GetRow(i2).GetComponent( j2)-m.GetRow(i2).GetComponent( j1)*m.GetRow(i1).GetComponent( j2)) / determinant
                    );
            } // for(int j=0; j<3; ++j)
            result.SetElem(3,i, T(0.0f));
        } // for(int i=0; i<3; ++i)
        return result;
    }
}

template<class T>
RW_MATH_INLINE Matrix33Template<T> Get3x3(const Matrix44AffineTemplate<T>& matrix)
{
    return Matrix33Template<T>(matrix.xAxis, matrix.yAxis, matrix.zAxis);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Set3x3(const Matrix44AffineTemplate<T>& matrix, const Matrix33Template<T>& m33)
{
    return Matrix44AffineTemplate<T>(m33.xAxis, m33.yAxis, m33.zAxis, matrix.wAxis);
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> OrthoNormalize3x3(const Matrix44AffineTemplate<T>& matrix)
{
    Vector3Template<T> *vpU, *vpV, *vpW;

    Vector3Template<T> right      = Normalize(matrix.XAxis());
    Vector3Template<T> up             = Normalize(matrix.YAxis());
    Vector3Template<T> at             = Normalize(matrix.ZAxis());
    const Vector3Template<T> &pos   = matrix.WAxis();

    Vector3Template<T> vInner( Magnitude(matrix.XAxis()),
        Magnitude(matrix.YAxis()),
        Magnitude(matrix.ZAxis()) );

    if (vInner.x > T(0.0f))
    {
        if (vInner.y > T(0.0f))
        {
            if (vInner.z > T(0.0f))
            {
                Vector3Template<T> vOuter(Abs(Dot(up, at)), Abs(Dot(at,right)), Abs(Dot(right, up)));
                if (vOuter.x < vOuter.y)
                {
                    if (vOuter.x < vOuter.z)
                    {
                        vpU = &up;
                        vpV = &at;
                        vpW = &right;
                    }
                    else
                    {
                        vpU = &right;
                        vpV = &up;
                        vpW = &at;
                    }
                }
                else
                {
                    if (vOuter.y < vOuter.z)
                    {
                        vpU = &at;
                        vpV = &right;
                        vpW = &up;
                    }
                    else
                    {
                        vpU = &right;
                        vpV = &up;
                        vpW = &at;
                    }
                }
            }
            else
            {
                vpU = &right;
                vpV = &up;
                vpW = &at;
            }
        }
        else
        {
            vpU = &at;
            vpV = &right;
            vpW = &up;
        }
    }
    else
    {
        vpU = &up;
        vpV = &at;
        vpW = &right;
    }
    *vpW = Normalize(Cross(*vpU, *vpV));
    *vpV = Normalize(Cross(*vpW, *vpU));
    Matrix44AffineTemplate<T> result(
        right,
        up,
        at,
        pos
        );
    return result;
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> InverseGeneric3x3(const Matrix44AffineTemplate<T>& in)
{
#define rwMATRIXDET(MAA, MAB, MBA, MBB)  \
    ( (MAA) * (MBB)                        \
    - (MAB) * (MBA)                        \
    )
    Matrix44AffineTemplate<T> out;
    out.XAxis().x =rwMATRIXDET(in.YAxis().y, in.YAxis().z, in.ZAxis().y, in.ZAxis().z);
    out.XAxis().y =-rwMATRIXDET(in.XAxis().y, in.XAxis().z, in.ZAxis().y, in.ZAxis().z);
    out.XAxis().z =rwMATRIXDET(in.XAxis().y, in.XAxis().z, in.YAxis().y, in.YAxis().z);
    T determinant = ((out.XAxis().x) * (in.XAxis().x) +
        (out.XAxis().y) * (in.YAxis().x) +
        (out.XAxis().z) * (in.ZAxis().x) );
    T normalize;
    if (IsZero(determinant))
    {
        normalize = T(1.0f);
    }
    else
    {
        normalize = T(1.0f) / determinant;
    }

    out.XAxis() *= normalize;
    out.YAxis().x =-rwMATRIXDET(in.YAxis().x, in.YAxis().z, in.ZAxis().x, in.ZAxis().z) * normalize;
    out.YAxis().y = rwMATRIXDET(in.XAxis().x, in.XAxis().z, in.ZAxis().x, in.ZAxis().z) * normalize;
    out.YAxis().z =-rwMATRIXDET(in.XAxis().x, in.XAxis().z, in.YAxis().x, in.YAxis().z) * normalize;
    out.ZAxis().x = rwMATRIXDET(in.YAxis().x, in.YAxis().y, in.ZAxis().x, in.ZAxis().y) * normalize;
    out.ZAxis().y =-rwMATRIXDET(in.XAxis().x, in.XAxis().y, in.ZAxis().x, in.ZAxis().y) * normalize;
    out.ZAxis().z = rwMATRIXDET(in.XAxis().x, in.XAxis().y, in.YAxis().x, in.YAxis().y) * normalize;
    out.WAxis().x =-((in.WAxis().x * out.XAxis().x) +
        (in.WAxis().y * out.XAxis().y) +
        (in.WAxis().z * out.XAxis().z));
    out.WAxis().y =-((in.WAxis().x * out.YAxis().x) +
        (in.WAxis().y * out.YAxis().y) +
        (in.WAxis().z * out.YAxis().z));
    out.WAxis().z =-((in.WAxis().x * out.ZAxis().x) +
        (in.WAxis().y * out.ZAxis().y) +
        (in.WAxis().z * out.ZAxis().z));
    return out;
#undef rwMATRIXDET
}

template<class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> InverseOfMatrixWithOrthonormal3x3(const Matrix44AffineTemplate<T>& in)
{
    Matrix44AffineTemplate<T> out;
    out.XAxis().Set(in.XAxis().x, in.YAxis().x, in.ZAxis().x);
    out.YAxis().Set(in.XAxis().y, in.YAxis().y, in.ZAxis().y);
    out.ZAxis().Set(in.XAxis().z, in.YAxis().z, in.ZAxis().z);
    out.WAxis().x =-((in.WAxis().x * in.XAxis().x) +
        (in.WAxis().y * in.XAxis().y) +
        (in.WAxis().z * in.XAxis().z));
    out.WAxis().y =-((in.WAxis().x * in.YAxis().x) +
        (in.WAxis().y * in.YAxis().y) +
        (in.WAxis().z * in.YAxis().z));
    out.WAxis().z =-((in.WAxis().x * in.ZAxis().x) +
        (in.WAxis().y * in.ZAxis().y) +
        (in.WAxis().z * in.ZAxis().z));
    return out;
}

template<class T>
RW_MATH_INLINE T GetNormalError3x3(const Matrix44AffineTemplate<T>& matrix)
{
    Matrix33Template<T> m33(
        matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
        matrix.GetElem(3), matrix.GetElem(4), matrix.GetElem(5),
        matrix.GetElem(6), matrix.GetElem(7), matrix.GetElem(8));
    return GetNormalError( m33 );
}

// --------------------------------------------------------
template<class T>
RW_MATH_INLINE T GetOrthogonalError(const Matrix44AffineTemplate<T>& matrix)
{
    Matrix44AffineTemplate<T> identity;
    identity.SetIdentity();

    Matrix44AffineTemplate<T> result = Mult(matrix, Transpose(matrix));
    SelfSubtract(result, identity);

    Vector4Template<T> errorVec( MagnitudeSquared( result.XAxis() ),
                                 MagnitudeSquared( result.YAxis() ),
                                 MagnitudeSquared( result.ZAxis() ),
                                 MagnitudeSquared( result.WAxis() ) );

    T error = MagnitudeSquared(errorVec);
    return error;
}

template<class T>
RW_MATH_INLINE T GetOrthogonalRowSetError(const Matrix44AffineTemplate<T>& matrix)
{
    // Calculate error vectors that "compare" all possible pairs of elements.
    // i.e. 4 choose 2 combinations.
    Vector3Template<T> errorVec1(    Dot( matrix.XAxis(), matrix.YAxis() ),
        Dot( matrix.XAxis(), matrix.ZAxis() ),
        Dot( matrix.XAxis(), matrix.WAxis() ) );
    Vector3Template<T> errorVec2(    Dot( matrix.YAxis(), matrix.ZAxis() ),
        Dot( matrix.YAxis(), matrix.WAxis() ),
        Dot( matrix.ZAxis(), matrix.WAxis() ) );
    T error = MagnitudeSquared(errorVec1) + MagnitudeSquared(errorVec2);
    return error;
}

template <class T>
RW_MATH_INLINE Matrix44AffineTemplate<T> Select(const MaskScalarTemplate<T> &mask, const Matrix44AffineTemplate<T> &trueValue, const Matrix44AffineTemplate<T> &falseValue )
{
    return Matrix44AffineTemplate<T>(
            Select( mask, trueValue.xAxis, falseValue.xAxis ),
            Select( mask, trueValue.yAxis, falseValue.yAxis ),
            Select( mask, trueValue.zAxis, falseValue.zAxis ),
            Select( mask, trueValue.wAxis, falseValue.wAxis )
        );
}

/*!
Test whether the upper left 3x3 portion of the matrix is a rotation matrix.  

Note: This function returns true only if and only if the upper left 3x3 portion
of the matrix is a rotation matrix.

\param a The matrix to perform the operation on.
\param tolerance The allowable difference between expected and actual result.

\return True if the upper left 3x3 portion of \c a is a rotation matrix, otherwise false.
*/
template<class T> RW_MATH_INLINE bool IsRotation3x3(const Matrix44AffineTemplate<T>& a, T tolerance = EPSILON)
{
    return IsOrthogonal3x3(a, tolerance) && IsSimilar(Determinant3x3(a), static_cast<T>(1.0f), tolerance);
}

#ifdef EA_PLATFORM_REVOLUTION
// These functions provide template specializations that are optimized for
// single precision types on the Wii.
template<>
Matrix44AffineTemplate<float> Mult(const Matrix44AffineTemplate<float> &a, const Matrix44AffineTemplate<float> &b);

template<>
Matrix44AffineTemplate<float> Inverse(const Matrix44AffineTemplate<float>& m, float& determinant);
#endif

} // namespace fpu
} // namespace math
} // namespace rw

#endif
