// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX33_OPERATION_INLINE_H
#define RW_MATH_FPU_MATRIX33_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"

#include "rw/math/fpu/quaternion_operation.h"
#include "rw/math/fpu/vector3_operation.h"
#include "rw/math/fpu/vector4_operation.h"

namespace rw {
namespace math {
namespace fpu {

/*!
Negate matrix \m m

\param m The matrix to negate.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Negate(const Matrix33Template<T> &m)
{
    return Matrix33Template<T>( Negate(m.xAxis), Negate(m.yAxis), Negate(m.zAxis) );
}

/*!
Multiply the input matrix by a scalar

\param m The matrix to multiply.
\param a The scalar to multiply the matrix with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Mult(const Matrix33Template<T> &m, T a)
{
    return Matrix33Template<T>(
        a * m.xAxis,
        a * m.yAxis,
        a * m.zAxis
        );
}

/*!
Multiply the input matrix by a vector

\param m The matrix to multiply.
\param v The vector to multiply the matrix with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Vector3Template<T> Mult(const Vector3Template<T> &v, const Matrix33Template<T> &m)
{
    return Vector3Template<T>(
        m.xAxis.x * v.x + m.yAxis.x * v.y + m.zAxis.x * v.z,
        m.xAxis.y * v.x + m.yAxis.y * v.y + m.zAxis.y * v.z,
        m.xAxis.z * v.x + m.yAxis.z * v.y + m.zAxis.z * v.z
        );
}

/*!
Multiply the input matrix by a matrix

\param m The matrix to multiply.
\param b The matrix to multiply the matrix with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Mult(const Matrix33Template<T> &m, const Matrix33Template<T> &b)
{
    Matrix33Template<T> result;
    for (int i = 0; i < 3; ++i)
    {
        result.SetElem(i, 0, m.GetRow(i).x * b.xAxis.x + m.GetRow(i).y * b.yAxis.x + m.GetRow(i).z * b.zAxis.x );
        result.SetElem(i, 1, m.GetRow(i).x * b.xAxis.y + m.GetRow(i).y * b.yAxis.y + m.GetRow(i).z * b.zAxis.y );
        result.SetElem(i, 2, m.GetRow(i).x * b.xAxis.z + m.GetRow(i).y * b.yAxis.z + m.GetRow(i).z * b.zAxis.z );
    }
    return result;
}

/*!
Divide the input matrix by a scalar

\param m The matrix to divide.
\param a The scalar to divide the matrix with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Divide(const Matrix33Template<T> &m, T a)
{
    return Matrix33Template<T>(
        Divide(m.xAxis, a),
        Divide(m.yAxis, a),
        Divide(m.zAxis, a)
        );
}

/*!
Add matrix \m m by matrix \m b

\param m The matrix to add.
\param b The matrix to add matrix \m m with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Add(const Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    return Matrix33Template<T>(
        Add(a.xAxis, b.xAxis),
        Add(a.yAxis, b.yAxis),
        Add(a.zAxis, b.zAxis)
        );
}

/*!
Subtract matrix \m m by matrix \m b

\param m The matrix to subtract.
\param b The matrix to subtract matrix \m m with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Subtract(const Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    return Matrix33Template<T>(
        Subtract(a.xAxis, b.xAxis),
        Subtract(a.yAxis, b.yAxis),
        Subtract(a.zAxis, b.zAxis)
        );
}

/*!
Test the equality of the two matrices

\param a The first matrix to test.
\param b The second matrix to test.

\return The boolean result of the operation.
*/
template<class T> RW_MATH_INLINE bool operator ==(const Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    return a.xAxis == b.xAxis &&
        a.yAxis == b.yAxis &&
        a.zAxis == b.zAxis;
}

/*!
Test the inequality of the two matrices

\param a The first matrix to test.
\param b The second matrix to test.

\return The boolean result of the operation.
*/
template<class T> RW_MATH_INLINE bool operator !=(const Matrix33Template<T> &a,const Matrix33Template<T> &b)
{
    return !(a==b);
}

/*!
Test whether the two matrices are similar based on the epsilon value

\param a The first matrix to test.
\param b The second matrix to test.
\param epsilon The allowable difference between the corresponding elements in the matrices

\return The boolean result of the operation.
*/
template<class T> RW_MATH_INLINE bool IsSimilar(const Matrix33Template<T> &a, const Matrix33Template<T> &b, T epsilon = SMALL_FLOAT)
{
    return IsSimilar( a.xAxis, b.xAxis, epsilon ) &&
        IsSimilar( a.yAxis, b.yAxis, epsilon ) &&
        IsSimilar( a.zAxis, b.zAxis, epsilon );
}

/*!
 * Determine if any element of the matrix is NaN.
 */
template<class T> RW_MATH_INLINE bool IsAnyNaN(const Matrix33Template<T> &m)
{
    return (bool)(IsAnyNaN(m.xAxis) || IsAnyNaN(m.yAxis) || IsAnyNaN(m.zAxis));
}

/*!
Add matrix a by matrix b

\param a The matrix to add.  It will store the result of the operation
\param b The matrix to add matrix a with.
*/
template<class T> RW_MATH_INLINE void SelfAdd(Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    a = Add(a,b);
}

/*!
Subtract matrix a by matrix b

\param a The matrix to subtract.  It will store the result of the operation
\param b The matrix to subtract matrix a with.

*/
template<class T> RW_MATH_INLINE void SelfSubtract(Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    a = Subtract(a,b);
}

/*!
Multiply matrix m by scalar a

\param m The matrix to multiply.  It will store the result of the operation
\param a The scalar to multiply the matrix with.

*/
template<class T> RW_MATH_INLINE void SelfMult(Matrix33Template<T> &m, T a)
{
    m = Mult(m, a);
}

/*!
Divide matrix a by matrix b

\param a The matrix to divide.  It will store the result of the operation
\param b The scalar to divide the matrix with.

*/
template<class T> RW_MATH_INLINE void SelfDivide(Matrix33Template<T> &m, T a)
{
    m = Divide(m, a);
}

/*!
Calculate the determinant of the specified matrix

\param matrix The matrix to perform the operation on.

\return The resulting value of the operation.
*/
template<class T> RW_MATH_INLINE T Determinant(const Matrix33Template<T> &m)
{
    return m.xAxis.x*m.yAxis.y*m.zAxis.z + m.xAxis.y*m.yAxis.z*m.zAxis.x + m.xAxis.z*m.yAxis.x*m.zAxis.y
        - m.xAxis.x*m.yAxis.z*m.zAxis.y - m.xAxis.y*m.yAxis.x*m.zAxis.z - m.xAxis.z*m.yAxis.y*m.zAxis.x ;
}

/*!
Transpose the specified matrix

\param m The matrix to transpose

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Transpose(const Matrix33Template<T> &m)
{
    return Matrix33Template<T> (m.xAxis.x, m.yAxis.x, m.zAxis.x,
                                m.xAxis.y, m.yAxis.y, m.zAxis.y,
                                m.xAxis.z, m.yAxis.z, m.zAxis.z);
}

/*!
Computes an inverse matrix given an input matrix.

\param m The matrix that should be inversed.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Inverse(const Matrix33Template<T> &m)
{
    T determinant;
    return Inverse( m, determinant );    
}

/*!
Computes an inverse matrix given an input matrix.

\param m The matrix that should be inversed.
\param determinant The determinant of the input matrix returned by the function

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Inverse(const Matrix33Template<T> &m, T& determinant)
{
    determinant = Determinant(m);
    RW_MATH_ASSERT_MSG((determinant != 0.0f), ("Matrix is singular."));
    if(determinant==0.0f)
    {
        return Matrix33Template<T>();
    }
    else
    {
        Matrix33Template<T> result;
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
        } // for(int i=0; i<3; ++i)
        return result;
    }
}

/*!
Converts a quaternion to a matrix.

\param q The quaternion to be converted

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromQuaternion(const QuaternionTemplate<T> &q)
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
    return Matrix33Template<T> (
        one-two*(yy+zz), two*(xy+wz), two*(xz-wy),
        two*(xy-wz), one-two*(xx+zz), two*(yz+wx),
        two*(xz+wy), two*(yz-wx), one-two*(xx+yy)
        );
}

/*!
Builds a matrix that rotates on an arbitrary axis

\param inputAxis The vector containing the axis of rotation
\param angle The counter-clockwise angle of rotation in radian, using right hand rule

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromAxisRotationAngle(const Vector3Template<T> &axis, T angle)
{
    T s, c;

    SinCos(angle, s, c);
    T t = 1.0f - c;
    T tx = t * axis.x;
    T ty = t * axis.y;
    T tz = t * axis.z;
    T sx = s * axis.x;
    T sy = s * axis.y;
    T sz = s * axis.z;

    return Matrix33Template<T>(
        tx * axis.x + c ,
        tx * axis.y + sz,
        tx * axis.z - sy,

        ty * axis.x - sz,
        ty * axis.y + c ,
        ty * axis.z + sx,

        tz * axis.x + sy,
        tz * axis.y - sx,
        tz * axis.z + c
        );
}

/*!
Constructs a matrix based on the Euler angles

\description The XYZ convention is also referred as pitch-roll-yaw

mathworld.wolfram.com/EulerAngles.html

\param euler The vector containing euler angles

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromEulerXYZ(const Vector3Template<T> &euler)
{
    T ci, cj, ch, si, sj, sh;

    SinCos(euler.x, si, ci);
    SinCos(euler.y, sj, cj);
    SinCos(euler.z, sh, ch);

    T cc = ci * ch;
    T cs = ci * sh;
    T sc = si * ch;
    T ss = si * sh;

    return Matrix33Template<T>(
        cj * ch , cj * sh , -sj ,
        sj * sc - cs, sj * ss + cc, cj * si ,
        sj * cc + ss, sj * cs - sc, cj * ci
        );
}

/*!
Constructs a Euler angles vector based on the input matrix

\description The XYZ convention is also referred as pitch-roll-yaw

mathworld.wolfram.com/EulerAngles.html

\param m The input matrix

\return A new instance of the resulting Euler angles vector.
*/
template<class T> RW_MATH_INLINE Vector3Template<T> EulerXYZFromMatrix33(const Matrix33Template<T>& m)
{
    T cy = Sqrt(m.xAxis.x * m.xAxis.x + m.xAxis.y * m.xAxis.y);

    if (cy > 0.001f)// SMALL_REAL
    {
        return Vector3Template<T>(
            ATan2( m.yAxis.z, m.zAxis.z),
            ATan2(-m.xAxis.z, cy),
            ATan2( m.xAxis.y, m.xAxis.x)
            );
    }
    else
    {
        return Vector3Template<T>(
            ATan2(-m.zAxis.y, m.yAxis.y),
            ATan2(-m.xAxis.z, cy),
            0.0f
            );
    }
}

/*!
Get the normal error of the specified matrix

\param matrix The matrix to perform the operation on.

\return The resulting value of the operation.
*/
template<class T> RW_MATH_INLINE T GetNormalError(const Matrix33Template<T>& matrix)
{
    Vector3Template<T> errorVec( MagnitudeSquared( matrix.XAxis() ) - static_cast<T>(1.0f),
        MagnitudeSquared( matrix.YAxis() ) - static_cast<T>(1.0f),
        MagnitudeSquared( matrix.ZAxis() ) - static_cast<T>(1.0f) );
    T error = MagnitudeSquared(errorVec);
    return error;
}

/*!
Get the orthogonality error of the specified matrix

\param matrix The matrix to perform the operation on.

\return The resulting value of the operation.
*/
template<class T> RW_MATH_INLINE T GetOrthogonalError(const Matrix33Template<T>& matrix)
{
    Matrix33Template<T> identity;
    identity.SetIdentity();

    Matrix33Template<T> result = Mult(matrix, Transpose(matrix));
    SelfSubtract(result, identity);

    Vector3Template<T> errorVec( MagnitudeSquared( result.XAxis() ),
                                 MagnitudeSquared( result.YAxis() ),
                                 MagnitudeSquared( result.ZAxis() ) );

    T error = MagnitudeSquared(errorVec);
    return error;
}

/*!
Get the error of the orthogonality of the rows in the specified matrix

\param matrix The matrix to perform the operation on.

\return The resulting value of the operation.
*/
template<class T> RW_MATH_INLINE T GetOrthogonalRowSetError(const Matrix33Template<T>& matrix)
{
    Vector3Template<T> errorVec( Dot( matrix.XAxis(), matrix.YAxis() ),
                                 Dot( matrix.YAxis(), matrix.ZAxis() ),
                                 Dot( matrix.ZAxis(), matrix.XAxis() ) );
    T error = MagnitudeSquared(errorVec);
    return error;
}

template<class T> RW_MATH_INLINE Matrix33Template<T> SLerp(const Matrix33Template<T>& lFrom, const Matrix33Template<T>& lTo, T lrParam, T& lprAngle)
{
    Matrix33Template<T> lReturnmatrix;
    Matrix33Template<T> lScratch;
    Vector3Template<T>  axis;
    T lrAngle;

    // Do we need to interpolate?
    if(lrParam >= static_cast<T>(1.0f))
    {
        lprAngle = 0.0f;
        return lTo;
    }

    // Interpolate - lerp or slerp?
    lScratch = Mult( InverseOrthonormal(lFrom), lTo );

    QueryRotate(lScratch, axis, lrAngle );

    if( lrParam <= static_cast<T>(0.0f) )
    {
        lprAngle = lrAngle;
        return lFrom;
    }

    // Lerp? (faster than slerp)
    if( lrAngle < MATRIX_MIN_SLERP_ANGLE )
    {
        // Get lerp matrix
        Matrix33Template<T> lLerpmat;

        lLerpmat.XAxis() = lTo.XAxis() - lFrom.XAxis();
        lLerpmat.YAxis() = lTo.YAxis() - lFrom.YAxis();
        lLerpmat.ZAxis() = lTo.ZAxis() - lFrom.ZAxis();

        // Do lerp
        lLerpmat *= lrParam;

        lReturnmatrix.XAxis() = Normalize(lFrom.XAxis() + lLerpmat.XAxis());
        lReturnmatrix.YAxis() = Normalize(lFrom.YAxis() + lLerpmat.YAxis());
        lReturnmatrix.ZAxis() = Normalize(lFrom.ZAxis() + lLerpmat.ZAxis());
    }
    else
        // SLerp
    {
        Matrix33Template<T> lRotatematrix;

        // Remove the translation for now
        lReturnmatrix = lFrom;

        // Rotate the new matrix
        lRotatematrix = Matrix33FromAxisRotationAngle( axis, ( lrAngle * lrParam ) );
        lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );
    }

    lprAngle = lrAngle - (lrAngle * lrParam);
    return lReturnmatrix;

}

template<class T> RW_MATH_INLINE Matrix33Template<T> SLerpByAngle(const Matrix33Template<T>& lFrom, const Matrix33Template<T>& lTo, T lrMaxAngle, T& lprAngle)
{
    Matrix33Template<T> lReturnmatrix;
    Matrix33Template<T> lRotatematrix;
    Vector3Template<T>  axis;
    T lrAngle;
    T lrDeltaAngle;

    // Get axis and angle between From and To
    Matrix33Template<T> lScratch = InverseOrthonormal(lFrom);
    lScratch = Mult( lScratch, lTo );

    QueryRotate(lScratch, axis, lrAngle );
    if( IsZero(axis) )
    {
        lprAngle = 0.0f;
        return lTo;
    }

    // Copy from and remove translation
    lReturnmatrix = lFrom;

    // Rotate about axis by no more than maximum specified angle
    lrDeltaAngle = Min(lrAngle, lrMaxAngle);

    lRotatematrix = Matrix33FromAxisRotationAngle( axis, lrDeltaAngle );
    lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

    lprAngle = lrAngle - lrDeltaAngle;

    return lReturnmatrix;
}


/*!
Computes an orthonormal matrix given an input matrix.

\description The function accepts a matrix as input and returns an orthonormalized
matrix that is "similar".  How similar the resulting matrix is to the input
matrix depends heavily on how close the input matrix is to being
orthonormal.

\par Preconditions
The input matrix must consist of four row vectors that are linearly
independent.  Put another way, the input matrix must have a rank of four.

mathworld.wolfram.com/LinearlyIndependent.html

\param m The matrix that should be orthonormalized.

\result The orthonormalized result.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> OrthoNormalize(const Matrix33Template<T>& matrix)
{
    Vector3Template<T> *vpU, *vpV, *vpW;

    Vector3Template<T> right         = Normalize(matrix.XAxis());
    Vector3Template<T> up            = Normalize(matrix.YAxis());
    Vector3Template<T> at            = Normalize(matrix.ZAxis());

    Vector3Template<T> vInner( Magnitude(matrix.XAxis()),
        Magnitude(matrix.YAxis()),
        Magnitude(matrix.ZAxis()) );

    if (vInner.x > 0.0f)
    {
        if (vInner.y > 0.0f)
        {
            if (vInner.z > 0.0f)
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
    return Matrix33Template<T>(
        right,
        up,
        at
        );
}

/*!
Transform a vector with the specified matrix.

\param v The vector to transform.
\param m The transformation matrix.

\return A new instance of the resulting vector.
*/
template<class T>
RW_MATH_INLINE Vector3Template<T> Transform(const Vector3Template<T>& v, const Matrix33Template<T>& m)
{
    return Vector3Template<T>(
        m.xAxis.x * v.x + m.yAxis.x * v.y + m.zAxis.x * v.z,
        m.xAxis.y * v.x + m.yAxis.y * v.y + m.zAxis.y * v.z,
        m.xAxis.z * v.x + m.yAxis.z * v.y + m.zAxis.z * v.z
        );
}

template<class T> RW_MATH_INLINE bool IsValid(const Matrix33Template<T>& a)
{
    return IsValid(a.XAxis()) &&
        IsValid(a.YAxis()) &&
        IsValid(a.ZAxis());
}

/*!
Test whether the matrix is normalized.  

\param a The matrix to perform the operation on.
\param tolerance The allowable difference between expected and actual result.

\return The boolean result of the operation.
*/
template<class T> RW_MATH_INLINE bool IsNormal(const Matrix33Template<T>& matrix, T tolerance = EPSILON)
{
    return IsZero( GetNormalError(matrix), tolerance );
}

/*!
Test whether the rows in the matrix are orthogonal.  

\param a The matrix containing the rows to be tested.
\param tolerance The allowable difference between expected and actual result.

\return The resulting value of the operation.
*/
template<class T>  RW_MATH_INLINE bool IsOrthogonalRowSet(const Matrix33Template<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetOrthogonalRowSetError(a), tolerance );
}

/*!
Test whether the matrix is orthogonal.  

\param a The matrix to perform the operation on.
\param tolerance The allowable difference between expected and actual result.

\return The resulting value of the operation.
*/
template<class T>  RW_MATH_INLINE bool IsOrthogonal(const Matrix33Template<T>& a, T tolerance = EPSILON)
{
    return IsZero( GetOrthogonalError(a), tolerance );
}


/*!
Test whether the matrix is a rotation matrix.  

\param a The matrix to perform the operation on.
\param tolerance The allowable difference between expected and actual result.

\return True if \c a is a rotation matrix, otherwise false.
*/
template<class T>  RW_MATH_INLINE bool IsRotation(const Matrix33Template<T>& a, T tolerance = EPSILON)
{
    return IsOrthogonal(a, tolerance) && IsSimilar(Determinant(a), static_cast<T>(1.0f), tolerance);
}

/*!
Computes a inverse given an input matrix which should be orthonormalized.

\warning The input matrix must be orthonormalized.

\param m The matrix that should be inverse orthonormalized.

\result The orthonormalized result.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> InverseOrthonormal(const Matrix33Template<T>& in)
{
    RW_MATH_ASSERT_MSG((IsOrthogonal<T>(in) && IsNormal<T>(in)), ("Input matrix is non orthonormal."));
    return Transpose(in);
}

/*!
Builds a matrix that rotates on the x-axis

\param angle The counter-clockwise angle of rotation in radian, using right hand rule

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromXRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix33Template<T>(
        one,   zero,   zero,
        zero,   c,   s,
        zero,  -s,   c
        );
}

/*!
Builds a matrix that rotates on the y-axis

\param angle The counter-clockwise angle of rotation in radian, using right hand rule

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromYRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix33Template<T>(
        c,   zero,  -s,
        zero,   one,   zero,
        s,   zero,   c );
}

/*!
Builds a matrix that rotates on the z-axis

\param angle The counter-clockwise angle of rotation in radian, using right hand rule

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromZRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero(0.0f);
    const T one(1.0f);
    return Matrix33Template<T>(
        c,   s,   zero,
        -s,   c,   zero,
        zero,   zero,   one );
}

/*!
Rotate a matrix around the x-axis

\param src The input matrix

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> RotateAroundXAxis(const Matrix33Template<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //    [         e00                e01                e02       ]
    //    [ e20 SIN + e10 COS  e21 SIN + e11 COS  e22 SIN + e12 COS ]
    //    [ e20 COS - e10 SIN  e21 COS - e11 SIN  e22 COS - e12 SIN ]
    T s,c;
    SinCos(angle, s, c);

    // rotate incrementally about x
    return Matrix33Template<T>(
        m.XAxis(),
        m.ZAxis() * s + m.YAxis() * c,
        m.ZAxis() * c - m.YAxis() * s
    );
}

/*!
Rotate a matrix around the y-axis

\param src The input matrix

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> RotateAroundYAxis(const Matrix33Template<T>&m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e00 COS - e20 SIN  e01 COS - e21 SIN  e02 COS - e22 SIN ]
    //  [         e10                e11                e12       ]
    //  [ e00 SIN + e20 COS  e01 SIN + e21 COS  e02 SIN + e22 COS ]
    T s,c;

    SinCos(angle, s, c);

    // rotate incrementally about y
    return Matrix33Template<T>(
        m.XAxis() * c - m.ZAxis() * s,
        m.YAxis(),
        m.XAxis() * s + m.ZAxis() * c
    );
}

/*!
Rotate a matrix around the z-axis

\param src The input matrix

\return A new instance of the resulting matrix.
*/
template<class T>  RW_MATH_INLINE Matrix33Template<T> RotateAroundZAxis(const Matrix33Template<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e10 SIN + e00 COS  e11 SIN + e01 COS  e12 SIN + e02 COS ]
    //  [ e10 COS - e00 SIN  e11 COS - e01 SIN  e12 COS - e02 SIN ]
    //  [         e20                e21                e22       ]
    T s,c;

    SinCos(angle, s, c);

    // rotate incrementally about z
    return Matrix33Template<T>(
        m.YAxis() * s + m.XAxis() * c,
        m.YAxis() * c - m.XAxis() * s,
        m.ZAxis()
    );
}


/*!
Scales the rows of a matrix by the specified values

\description The last row of the matrix is not scaled

\param src The input matrix
\param r0 The scale factor for row 0
\param r1 The scale factor for row 1
\param r2 The scale factor for row 2

\return A new instance of the resulting matrix.
*/
template<class T>  RW_MATH_INLINE Matrix33Template<T> ScaleRow(const Matrix33Template<T>& m, T r0, T r1, T r2)
{
    return Matrix33Template<T>(
        Mult( m.xAxis, r0 ),
        Mult( m.yAxis, r1 ),
        Mult( m.zAxis, r2 )
        );
}

/*!
Scales the columns of a matrix by the specified values

\description The last column of the matrix is not scaled

\param src The input matrix
\param r0 The scale factor for column 0
\param r1 The scale factor for column 1
\param r2 The scale factor for column 2

\return A new instance of the resulting matrix.
*/
template<class T>  RW_MATH_INLINE Matrix33Template<T> ScaleColumn(const Matrix33Template<T>& m, T c0, T c1, T c2 )
{
    Vector3Template<T> rowMultiplier(c0, c1, c2);
    return Matrix33Template<T>(
        Mult( m.xAxis, rowMultiplier ),
        Mult( m.yAxis, rowMultiplier ),
        Mult( m.zAxis, rowMultiplier )
        );
}

/*!
Constructs a scale matrix from the input value

\par Resulting matrix
\code
[   a   0   0   ]
[   0   a   0   ]
[   0   0   a   ]
\endcode

\param a The input value

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> Matrix33FromDiagonal(T a)
{
    return Matrix33Template<T>(
        a,0,0,
        0,a,0,
        0,0,a
        );
}


template<class T> RW_MATH_INLINE Vector3Template<T> QueryRotateDegenerateUnitAxis(const Matrix33Template<T>& matrix)
{
    if(matrix.XAxis().x > matrix.YAxis().y)
    {
        if(matrix.XAxis().x > matrix.ZAxis().z)
        {
            T xVal = 1.0f + matrix.XAxis().x;
            return Normalize(Vector3Template<T>(
                xVal*xVal,
                matrix.XAxis().y + matrix.YAxis().x,
                matrix.XAxis().z + matrix.ZAxis().x
                ));
        }
        else
        {
            T zVal = 1.0f + matrix.ZAxis().z;
            return Normalize(Vector3Template<T>(
                matrix.ZAxis().x + matrix.XAxis().z,
                matrix.ZAxis().y + matrix.YAxis().z,
                zVal*zVal
                ));
        }
    }
    else
    {
        if(matrix.YAxis().y > matrix.ZAxis().z)
        {
            T yVal = 1.0f + matrix.YAxis().y;
            return Normalize(Vector3Template<T>(
                matrix.YAxis().x + matrix.XAxis().y,
                yVal*yVal,
                matrix.YAxis().z + matrix.ZAxis().y
                ));
        }
        else
        {
            T zVal = 1.0f + matrix.ZAxis().z;
            return Normalize(Vector3Template<T>(
                matrix.ZAxis().x + matrix.XAxis().z,
                matrix.ZAxis().y + matrix.YAxis().z,
                zVal*zVal
                ));
        }
    }
}


template<class T> RW_MATH_INLINE void QueryRotate(const Matrix33Template<T>& matrix, Vector3Template<T>& unitAxis, T& angle)
{
    Vector3Template<T> twoSinThetaAxis;
    T twoCosTheta = matrix.XAxis().x + matrix.YAxis().y + matrix.ZAxis().z - 1.0f;
    twoSinThetaAxis.x =matrix.YAxis().z - matrix.ZAxis().y;
    twoSinThetaAxis.y =matrix.ZAxis().x - matrix.XAxis().z;
    twoSinThetaAxis.z =matrix.XAxis().y - matrix.YAxis().x;
    T twoSinTheta = Magnitude(twoSinThetaAxis);
    if(twoSinTheta > T(0.0f))
    {
        const T lrRecipLength = 1.0f / twoSinTheta;
        unitAxis = twoSinThetaAxis * lrRecipLength;
    }
    else
    {
        unitAxis = Vector3Template<T>(0.0f, 0.0f, 0.0f);
    }
    angle = ATan2(twoSinTheta, twoCosTheta);
    if((twoSinTheta <= MINIMUM_RECIPROCAL) && (twoCosTheta <= 0.0f))
    {
        unitAxis = QueryRotateDegenerateUnitAxis(matrix);
    }
}


/*!
Transform a vector with the specified matrix.

\param vec The vector to transform.
\param m The transformation matrix.

\return A new instance of the resulting vector.
*/
template<class T> RW_MATH_INLINE void Transform(const Vector3Template<T>* const vecs, int numVectors, const Matrix33Template<T>& matrix, Vector3Template<T>* transformedVecs)
{
    for( int vecIndex = 0; vecIndex < numVectors; ++vecIndex )
    {
        transformedVecs[vecIndex] = Transform(vecs[vecIndex], matrix);
    }
}

/*!
Multiply input matrix by a scalar

\param m The matrix to multiply.
\param a The scalar to multiply the matrix with.

\return A reference to matrix \m m which stores the result of the operation.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T>& operator *=(Matrix33Template<T> &m, const T& a)
{
    m = Mult(m, a);
    return m;
}

/*!
Divide input matrix by a scalar

\param m The matrix to divide.
\param a The scalar to divide the matrix with.

\return A reference to matrix \m m which stores the result of the operation.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T>& operator /=(Matrix33Template<T> &m, const T& a)
{
    m = Divide(m, a);
    return m;
}

/*!
Multiply matrix \m m by matrix \m b

\param m The matrix to multiply.
\param b The matrix to multiply matrix \m m with.

\return A reference to matrix \m m which stores the result of the operation.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T>& operator *=(Matrix33Template<T> &m, const Matrix33Template<T> &b)
{
    m = Mult(m, b);
    return m;
}

/*!
Add matrix \m m by matrix \m b

\param m The matrix to add.
\param b The matrix to add matrix \m m with.

\return A reference to matrix \m m which stores the result of the operation.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T>& operator +=(Matrix33Template<T> &m, const Matrix33Template<T> &b)
{
    m = Add(m, b);
    return m;
}

/*!
Subtract matrix \m m by matrix \m b

\param m The matrix to subtract.
\param b The matrix to subtract matrix \m m with.

\return A reference to matrix \m m which stores the result of the operation.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T>& operator -=(Matrix33Template<T> &m, const Matrix33Template<T> &b)
{
    m = Subtract(m, b);
    return m;
}

/*!
Negate matrix \m m

\param m The matrix to negate.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator - (const Matrix33Template<T> &m)
{
    return Negate(m);
}

/*!
Multiply the input matrix by a scalar

\param m The matrix to multiply.
\param a The scalar to multiply the matrix with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator * (const Matrix33Template<T> &m, const T& a)
{
    return Mult(m, a);
}

/*!
Multiply the input matrix by a scalar

\param a The scalar to multiply the matrix with.
\param m The matrix to multiply.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator * (const T& a, const Matrix33Template<T> &b)
{
    return Mult(b, a);
}

/*!
Divide the input matrix by a scalar

\param m The matrix to divide.
\param a The scalar to divide the matrix with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator / (const Matrix33Template<T> &m, T a)
{
    return Divide(m, a);
}

/*!
Add matrix \m m by matrix \m b

\param m The matrix to add.
\param b The matrix to add matrix \m m with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator + (const Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    return Add(a, b);
}

/*!
Subtract matrix \m m by matrix \m b

\param m The matrix to subtract.
\param b The matrix to subtract matrix \m m with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator - (const Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    return Subtract(a, b);
}

/*!
Multiply matrix \m m by matrix \m b

\param m The matrix to multiply.
\param b The matrix to multiply matrix \m m with.

\return A new instance of the resulting matrix.
*/
template<class T> RW_MATH_INLINE Matrix33Template<T> operator * (const Matrix33Template<T> &a, const Matrix33Template<T> &b)
{
    return Mult(a, b);
}

/*!
Multiply the input matrix by a vector

\param v The vector to multiply the matrix with.
\param m The matrix to multiply.

\return A new instance of the resulting vector.
*/
template<class T> RW_MATH_INLINE Vector3Template<T> operator * (const Vector3Template<T> &v, const Matrix33Template<T> &m)
{
    return Mult(v, m);
}

template <class T>
RW_MATH_INLINE Matrix33Template<T> Select(const MaskScalarTemplate<T> &mask, const Matrix33Template<T> &trueValue, const Matrix33Template<T> &falseValue )
{
    return Matrix33Template<T>(
            Select( mask, trueValue.xAxis, falseValue.xAxis ),
            Select( mask, trueValue.yAxis, falseValue.yAxis ),
            Select( mask, trueValue.zAxis, falseValue.zAxis )
        );
}

// operations that return by reference
#if RW_MATH_ENABLE_DEPRECATED_API

 template<class T> RW_MATH_INLINE void Negate(const Matrix33Template<T> &m, Matrix33Template<T> &result) 
{
    result = Negate(m);
}
 template<class T> RW_MATH_INLINE void Mult(const Matrix33Template<T> &m, T a, Matrix33Template<T> &result) 
{
    result = Mult(m, a);
}
 template<class T> RW_MATH_INLINE void Mult(const Vector3Template<T> &v, const Matrix33Template<T> &m, Vector3Template<T> &result) 
{
    result = Mult(v, m);
}
 template<class T> RW_MATH_INLINE void Mult(const Matrix33Template<T> &m, const Matrix33Template<T> &b, Matrix33Template<T> &result) 
{
    result = Mult(m,b);
}
 template<class T> RW_MATH_INLINE void Divide(const Matrix33Template<T> &m, T a, Matrix33Template<T> &result) 
{
    result = Divide(m, a);
}
 template<class T> RW_MATH_INLINE void Add(const Matrix33Template<T> &a, const Matrix33Template<T> &b, Matrix33Template<T> &result) 
{
    result = Add(a,b);
}
 template<class T> RW_MATH_INLINE void Subtract(const Matrix33Template<T> &a, const Matrix33Template<T> &b, Matrix33Template<T> &result) 
{
    result = Subtract(a,b);
}

 template<class T> RW_MATH_INLINE void Transpose(const Matrix33Template<T> &m, Matrix33Template<T> &result) 
{
    result = Transpose(m);
}
 template<class T> RW_MATH_INLINE T Inverse(const Matrix33Template<T> &m, Matrix33Template<T> &result) 
{
    T determinant;
    result = Inverse(m, determinant);
    return determinant;
}

 template<class T> RW_MATH_INLINE void Matrix33FromQuaternion(const QuaternionTemplate<T> &q, Matrix33Template<T> &result) 
{
    result = Matrix33FromQuaternion(q);
}
 template<class T> RW_MATH_INLINE void Matrix33FromAxisAngle(const Vector3Template<T> &axis, T angle, Matrix33Template<T> &result) 
{
    result = Matrix33FromAxisRotationAngle(axis, angle);
}
 template<class T> RW_MATH_INLINE void Matrix33FromEulerXYZ(const Vector3Template<T> &euler, Matrix33Template<T> &result) 
{
    result = Matrix33FromEulerXYZ(euler);
}
 template<class T> RW_MATH_INLINE void EulerXYZFromMatrix33(const Matrix33Template<T>& m, Vector3Template<T> &result) 
{
    result = EulerXYZFromMatrix33(m);
}

#endif

}

}

}

#endif
