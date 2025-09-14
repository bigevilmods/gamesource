// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX44_OPERATION_INLINE_H
#define RW_MATH_FPU_MATRIX44_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/matrix33.h"

#include "rw/math/fpu/quaternion_operation.h"
#include "rw/math/fpu/vector3_operation.h"
#include "rw/math/fpu/vector4_operation.h"
#include "rw/math/fpu/matrix33_operation.h"

namespace rw {
namespace math {
namespace fpu {

template<class T>
RW_MATH_INLINE bool operator ==(const Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    return  a.xAxis == b.xAxis &&
            a.yAxis == b.yAxis &&
            a.zAxis == b.zAxis &&
            a.wAxis == b.wAxis;
}

template<class T>
RW_MATH_INLINE bool operator !=(const Matrix44Template<T> &a,const Matrix44Template<T> &b)
{
    return !(a==b);
}

template<class T>
RW_MATH_INLINE bool IsValid(const Matrix44Template<T> &a)
{
    return IsValid(a.XAxis()) &&
           IsValid(a.YAxis()) &&
           IsValid(a.ZAxis()) &&
           IsValid(a.WAxis());
}

template<class T>
RW_MATH_INLINE bool IsValid3x3(const Matrix44Template<T> &a)
{
    return IsValid(GetXYZ(a.XAxis())) &&
           IsValid(GetXYZ(a.YAxis())) &&
           IsValid(GetXYZ(a.ZAxis()));
}

template<class T>
RW_MATH_INLINE bool IsSimilar(const Matrix44Template<T> &a, const Matrix44Template<T> &b, T epsilon = SMALL_FLOAT)
{
    return IsSimilar( a.xAxis, b.xAxis, epsilon ) &&
           IsSimilar( a.yAxis, b.yAxis, epsilon ) &&
           IsSimilar( a.zAxis, b.zAxis, epsilon ) &&
           IsSimilar( a.wAxis, b.wAxis, epsilon );
}

/*!
 * Determine if any element of the matrix is NaN.
 */
template<class T> RW_MATH_INLINE bool IsAnyNaN(const Matrix44Template<T> &m)
{
    return IsAnyNaN(m.xAxis) || IsAnyNaN(m.yAxis) || IsAnyNaN(m.zAxis) || IsAnyNaN(m.wAxis);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> SLerp(const Matrix44Template<T> &lFrom, const Matrix44Template<T> &lTo, T lrParam, T &lprAngle)
{
    Matrix44Template<T> lReturnmatrix;
    Matrix44Template<T> lScratch;
    Vector3Template<T>  axis;
    Vector3Template<T>  lCentre;
    T lrAngle;

    // Do we need to interpolate?
    if(lrParam >= static_cast<T>(1.0f))
    {
        lprAngle = static_cast<T>(0.0f);
        return lTo;
    }

    // Interpolate - lerp or slerp?
    lScratch = Mult( InverseOrthonormal(lFrom), lTo );

    QueryRotate(lScratch, axis, lrAngle, lCentre );

    if( lrParam <= static_cast<T>(0.0f) )
    {
        lprAngle = lrAngle;
        return lFrom;
    }

    // Lerp? (faster than slerp)
    if( lrAngle < static_cast<T>(MATRIX_MIN_SLERP_ANGLE) )
    {
        // Get lerp matrix
        Matrix44Template<T> lLerpmat;

        lLerpmat.XAxis() = lTo.XAxis() - lFrom.XAxis();
        lLerpmat.YAxis() = lTo.YAxis() - lFrom.YAxis();
        lLerpmat.ZAxis() = lTo.ZAxis() - lFrom.ZAxis();
        lLerpmat.WAxis() = lTo.WAxis() - lFrom.WAxis();

        // Do lerp
        lLerpmat *= lrParam;

        lReturnmatrix.XAxis() = Normalize(lFrom.XAxis() + lLerpmat.XAxis());
        lReturnmatrix.YAxis() = Normalize(lFrom.YAxis() + lLerpmat.YAxis());
        lReturnmatrix.ZAxis() = Normalize(lFrom.ZAxis() + lLerpmat.ZAxis());
        lReturnmatrix.WAxis() = lFrom.WAxis() + lLerpmat.WAxis();
    }
    else
    // SLerp
    {
        Matrix44Template<T> lRotatematrix;

        // Remove the translation for now
        lReturnmatrix = lFrom;
        lReturnmatrix.WAxis() = GetVector4_WAxisTemplate<T>() ;

        // Rotate the new matrix
        lRotatematrix = Matrix44FromAxisRotationAngle( axis, ( lrAngle * lrParam ) );
        lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

        // Do linear interpolation on position
        lReturnmatrix.WAxis() = (lTo.WAxis() - lFrom.WAxis()) * lrParam + lFrom.WAxis();
    }

    lprAngle = lrAngle - (lrAngle * lrParam);
    return lReturnmatrix;

}

template<class T>
RW_MATH_INLINE Matrix44Template<T> SLerpByAngle(const Matrix44Template<T> &lFrom, const Matrix44Template<T> &lTo, T lrMaxAngle, T &lprAngle)
{
    Matrix44Template<T> lReturnmatrix;
    Matrix44Template<T> lRotatematrix;
    Vector3Template<T>  axis;
    Vector3Template<T>  lCentre;
    T lrAngle;
    T lrDeltaAngle;

    // Get axis and angle between From and To
    Matrix44Template<T> lScratch = InverseOrthonormal(lFrom);
    lScratch = Mult( lScratch, lTo );

    QueryRotate(lScratch, axis, lrAngle, lCentre );
    if( IsZero(axis) )
    {
        lprAngle = 0.0f;
        return lTo;
    }

    // Copy from and remove translation
    Vector4Template<T> lPosBackup = lFrom.WAxis();
    lReturnmatrix = lFrom;
    lReturnmatrix.WAxis() = GetVector4_WAxisTemplate<T>() ;

    // Rotate about axis by no more than maximum specified angle
    lrDeltaAngle = Min(lrAngle, lrMaxAngle);

    lRotatematrix = Matrix44FromAxisRotationAngle( axis, lrDeltaAngle );
    lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

    // Put back original translation
    lReturnmatrix.WAxis() = lPosBackup ;

    lprAngle = lrAngle - lrDeltaAngle;

    return lReturnmatrix;
}

template<class T>
RW_MATH_INLINE void SelfAdd(Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    a = Add(a, b);
}

template<class T>
RW_MATH_INLINE void SelfSubtract(Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    a = Subtract(a, b);
}

template<class T>
RW_MATH_INLINE void SelfMult(Matrix44Template<T> &m, T a)
{
    m = Mult(m, a);
}

template<class T>
RW_MATH_INLINE void SelfDivide(Matrix44Template<T> &m, T a)
{
    m = Divide(m, a);
}

template<class T>
RW_MATH_INLINE T Determinant(const Matrix44Template<T> &m)
{
    return
        m.xAxis.x * (
                m.yAxis.y * ( m.zAxis.z*m.wAxis.w - m.zAxis.w*m.wAxis.z ) +
                m.yAxis.z * ( m.wAxis.y*m.zAxis.w - m.zAxis.y*m.wAxis.w ) +
                m.yAxis.w * ( m.zAxis.y*m.wAxis.z - m.wAxis.y*m.zAxis.z )
                ) +
        m.xAxis.y * (
                m.yAxis.x * ( m.zAxis.w*m.wAxis.z - m.zAxis.z*m.wAxis.w ) +
                m.yAxis.z * ( m.zAxis.x*m.wAxis.w - m.wAxis.x*m.zAxis.w ) +
                m.yAxis.w * ( m.wAxis.x*m.zAxis.z - m.zAxis.x*m.wAxis.z )
                ) +
        m.xAxis.z * (
                m.yAxis.x * ( m.zAxis.y*m.wAxis.w - m.wAxis.y*m.zAxis.w ) +
                m.yAxis.y * ( m.wAxis.x*m.zAxis.w - m.zAxis.x*m.wAxis.w ) +
                m.yAxis.w * ( m.zAxis.x*m.wAxis.y - m.wAxis.x*m.zAxis.y )
                ) +
        m.xAxis.w * (
                m.yAxis.x * ( m.wAxis.y*m.zAxis.z - m.zAxis.y*m.wAxis.z ) +
                m.yAxis.y * ( m.zAxis.x*m.wAxis.z - m.wAxis.x*m.zAxis.z ) +
                m.yAxis.z * ( m.wAxis.x*m.zAxis.y - m.zAxis.x*m.wAxis.y )
                );
}

// operations that return by value

template<class T>
RW_MATH_INLINE Matrix44Template<T> Negate(const Matrix44Template<T> &m)
{
    return Matrix44Template<T>( Negate(m.xAxis), Negate(m.yAxis), Negate(m.zAxis), Negate(m.wAxis) );
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Mult(const Matrix44Template<T> &m, T a)
{
    Matrix44Template<T> rm(
            a * m.xAxis,
            a * m.yAxis,
            a * m.zAxis,
            a * m.wAxis
            );
    return rm;
}

template<class T>
RW_MATH_INLINE Vector4Template<T> Mult(const Vector4Template<T> &v, const Matrix44Template<T> &m)
{
    Vector4Template<T> rv(
            m.xAxis.x * v.x + m.yAxis.x * v.y + m.zAxis.x * v.z + m.wAxis.x * v.w,
            m.xAxis.y * v.x + m.yAxis.y * v.y + m.zAxis.y * v.z + m.wAxis.y * v.w,
            m.xAxis.z * v.x + m.yAxis.z * v.y + m.zAxis.z * v.z + m.wAxis.z * v.w,
            m.xAxis.w * v.x + m.yAxis.w * v.y + m.zAxis.w * v.z + m.wAxis.w * v.w
            );
    return rv;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Mult(const Matrix44Template<T> &m, const Matrix44Template<T> &b)
{
    Matrix44Template<T> rm(
                Mult(m.xAxis, b),
                Mult(m.yAxis, b),
                Mult(m.zAxis, b),
                Mult(m.wAxis, b)
                );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Divide(const Matrix44Template<T> &m, T a)
{
    Matrix44Template<T> rm(
            m.xAxis / a,
            m.yAxis / a,
            m.zAxis / a,
            m.wAxis / a
            );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Add(const Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    Matrix44Template<T> m(
                Add(a.xAxis, b.xAxis),
                Add(a.yAxis, b.yAxis),
                Add(a.zAxis, b.zAxis),
                Add(a.wAxis, b.wAxis)
                );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Subtract(const Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    Matrix44Template<T> m(
                Subtract(a.xAxis, b.xAxis),
                Subtract(a.yAxis, b.yAxis),
                Subtract(a.zAxis, b.zAxis),
                Subtract(a.wAxis, b.wAxis)
                );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Transpose(const Matrix44Template<T> &m)
{
    return Matrix44Template<T> (m.xAxis.x, m.yAxis.x, m.zAxis.x, m.wAxis.x,
                                m.xAxis.y, m.yAxis.y, m.zAxis.y, m.wAxis.y,
                                m.xAxis.z, m.yAxis.z, m.zAxis.z, m.wAxis.z,
                                m.xAxis.w, m.yAxis.w, m.zAxis.w, m.wAxis.w );
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Inverse(const Matrix44Template<T> &m)
{
    T determinant;
    return Inverse(m, determinant);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Inverse(const Matrix44Template<T> &m, T& determinant)
{
    determinant = Determinant(m);
    RW_MATH_ASSERT_MSG((determinant != 0.0f), ("Matrix is singular."));
    if (determinant == static_cast<T>(0.0f))
    {
        return GetMatrix44_ZeroTemplate<T>();
    }
    else
    {
        Matrix44Template<T> rm(
            ((m.yAxis.y*(m.zAxis.z*m.wAxis.w-m.zAxis.w*m.wAxis.z)+m.yAxis.z*(m.wAxis.y*m.zAxis.w-m.zAxis.y*m.wAxis.w)+m.yAxis.w*(m.zAxis.y*m.wAxis.z-m.wAxis.y*m.zAxis.z))/determinant),
            ((m.zAxis.y*(m.xAxis.z*m.wAxis.w-m.xAxis.w*m.wAxis.z)+m.zAxis.z*(m.xAxis.w*m.wAxis.y-m.xAxis.y*m.wAxis.w)+m.zAxis.w*(m.xAxis.y*m.wAxis.z-m.xAxis.z*m.wAxis.y))/determinant),
            ((m.wAxis.y*(m.xAxis.z*m.yAxis.w-m.xAxis.w*m.yAxis.z)+m.wAxis.z*(m.xAxis.w*m.yAxis.y-m.xAxis.y*m.yAxis.w)+m.wAxis.w*(m.xAxis.y*m.yAxis.z-m.xAxis.z*m.yAxis.y))/determinant),
            ((m.xAxis.y*(m.yAxis.w*m.zAxis.z-m.yAxis.z*m.zAxis.w)+m.xAxis.z*(m.yAxis.y*m.zAxis.w-m.yAxis.w*m.zAxis.y)+m.xAxis.w*(m.yAxis.z*m.zAxis.y-m.yAxis.y*m.zAxis.z))/determinant),
            ((m.yAxis.z*(m.zAxis.x*m.wAxis.w-m.wAxis.x*m.zAxis.w)+m.yAxis.w*(m.wAxis.x*m.zAxis.z-m.zAxis.x*m.wAxis.z)+m.yAxis.x*(m.zAxis.w*m.wAxis.z-m.zAxis.z*m.wAxis.w))/determinant),
            ((m.zAxis.z*(m.xAxis.x*m.wAxis.w-m.wAxis.x*m.xAxis.w)+m.zAxis.w*(m.wAxis.x*m.xAxis.z-m.xAxis.x*m.wAxis.z)+m.zAxis.x*(m.xAxis.w*m.wAxis.z-m.xAxis.z*m.wAxis.w))/determinant),
            ((m.wAxis.z*(m.xAxis.x*m.yAxis.w-m.yAxis.x*m.xAxis.w)+m.wAxis.w*(m.yAxis.x*m.xAxis.z-m.xAxis.x*m.yAxis.z)+m.wAxis.x*(m.xAxis.w*m.yAxis.z-m.xAxis.z*m.yAxis.w))/determinant),
            ((m.xAxis.z*(m.zAxis.x*m.yAxis.w-m.yAxis.x*m.zAxis.w)+m.xAxis.w*(m.yAxis.x*m.zAxis.z-m.zAxis.x*m.yAxis.z)+m.xAxis.x*(m.yAxis.z*m.zAxis.w-m.yAxis.w*m.zAxis.z))/determinant),
            ((m.yAxis.w*(m.zAxis.x*m.wAxis.y-m.wAxis.x*m.zAxis.y)+m.yAxis.x*(m.zAxis.y*m.wAxis.w-m.wAxis.y*m.zAxis.w)+m.yAxis.y*(m.wAxis.x*m.zAxis.w-m.zAxis.x*m.wAxis.w))/determinant),
            ((m.zAxis.w*(m.xAxis.x*m.wAxis.y-m.wAxis.x*m.xAxis.y)+m.zAxis.x*(m.xAxis.y*m.wAxis.w-m.xAxis.w*m.wAxis.y)+m.zAxis.y*(m.wAxis.x*m.xAxis.w-m.xAxis.x*m.wAxis.w))/determinant),
            ((m.wAxis.w*(m.xAxis.x*m.yAxis.y-m.yAxis.x*m.xAxis.y)+m.wAxis.x*(m.xAxis.y*m.yAxis.w-m.xAxis.w*m.yAxis.y)+m.wAxis.y*(m.yAxis.x*m.xAxis.w-m.xAxis.x*m.yAxis.w))/determinant),
            ((m.xAxis.w*(m.zAxis.x*m.yAxis.y-m.yAxis.x*m.zAxis.y)+m.xAxis.x*(m.yAxis.w*m.zAxis.y-m.yAxis.y*m.zAxis.w)+m.xAxis.y*(m.yAxis.x*m.zAxis.w-m.zAxis.x*m.yAxis.w))/determinant),
            ((m.yAxis.x*(m.wAxis.y*m.zAxis.z-m.zAxis.y*m.wAxis.z)+m.yAxis.y*(m.zAxis.x*m.wAxis.z-m.wAxis.x*m.zAxis.z)+m.yAxis.z*(m.wAxis.x*m.zAxis.y-m.zAxis.x*m.wAxis.y))/determinant),
            ((m.zAxis.x*(m.xAxis.z*m.wAxis.y-m.xAxis.y*m.wAxis.z)+m.zAxis.y*(m.xAxis.x*m.wAxis.z-m.wAxis.x*m.xAxis.z)+m.zAxis.z*(m.wAxis.x*m.xAxis.y-m.xAxis.x*m.wAxis.y))/determinant),
            ((m.wAxis.x*(m.xAxis.z*m.yAxis.y-m.xAxis.y*m.yAxis.z)+m.wAxis.y*(m.xAxis.x*m.yAxis.z-m.yAxis.x*m.xAxis.z)+m.wAxis.z*(m.yAxis.x*m.xAxis.y-m.xAxis.x*m.yAxis.y))/determinant),
            ((m.xAxis.x*(m.yAxis.y*m.zAxis.z-m.yAxis.z*m.zAxis.y)+m.xAxis.y*(m.zAxis.x*m.yAxis.z-m.yAxis.x*m.zAxis.z)+m.xAxis.z*(m.yAxis.x*m.zAxis.y-m.zAxis.x*m.yAxis.y))/determinant)
             );
        return rm;
    }
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromQuaternion(const QuaternionTemplate<T> &q)
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

    const T zero(static_cast<T>(0.0f));
    const T one(static_cast<T>(1.0f));
    const T two(static_cast<T>(2.0f));

    Matrix44Template<T> m(
        one-two*(yy+zz), two*(xy+wz),     two*(xz-wy),      zero,
        two*(xy-wz),     one-two*(xx+zz), two*(yz+wx),      zero,
        two*(xz+wy),     two*(yz-wx),     one-two*(xx+yy),  zero,
        zero,            zero,            zero,             one
        );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromEulerXYZ(const Vector3Template<T> &euler)
{
    T   ci, cj, ch, si, sj, sh;

    SinCos(euler.x, si, ci);
    SinCos(euler.y, sj, cj);
    SinCos(euler.z, sh, ch);

    T cc = ci * ch;
    T cs = ci * sh;
    T sc = si * ch;
    T ss = si * sh;

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);

    Matrix44Template<T> m(
        cj * ch     ,  cj * sh     , -sj      , zero,
        sj * sc - cs,  sj * ss + cc,  cj * si , zero,
        sj * cc + ss,  sj * cs - sc,  cj * ci , zero,
        zero, zero, zero, one
        );
    return m;
}

template<class T>
RW_MATH_INLINE Vector3Template<T> EulerXYZFromMatrix44(const Matrix44Template<T>& m)
{
    Matrix33Template<T> m33(GetXYZ(m.XAxis()), GetXYZ(m.YAxis()), GetXYZ(m.ZAxis()));
    return EulerXYZFromMatrix33( m33 );
}

/*!
 * Returns the inverse of a 4x4 matrix that has only a rotation component, and a translation.
 *
 * The name for this function is actually a bit misleading (but maintained for
 * backwards compatibility).  The function returns the inverse of a 4x4 matrix.
 * However, the input matrix must only contain a rotation and a translation.
 * (i.e. a matrix generated from a Quaternion and a Translation is acceptable)
 */
template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44InverseRotationalComponent(const Matrix44Template<T> &m)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);

    Matrix44Template<T> rm(
        m.xAxis.x , m.yAxis.x , m.zAxis.x, zero,
        m.xAxis.y , m.yAxis.y , m.zAxis.y, zero,
        m.xAxis.z , m.yAxis.z , m.zAxis.z, zero,
           -m.wAxis.x*m.xAxis.x-m.wAxis.y*m.xAxis.y-m.wAxis.z*m.xAxis.z,
           -m.wAxis.x*m.yAxis.x-m.wAxis.y*m.yAxis.y-m.wAxis.z*m.yAxis.z,
           -m.wAxis.x*m.zAxis.x-m.wAxis.y*m.zAxis.y-m.wAxis.z*m.zAxis.z,
        one );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromScale(const Vector3Template<T> &scale)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);

    Matrix44Template<T> m(
        scale.x, zero, zero, zero,
        zero, scale.y, zero, zero,
        zero, zero, scale.z, zero,
        zero, zero, zero, one
    );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromTranslation(const Vector3Template<T> &trans)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);

    Matrix44Template<T> m(
        one,       zero,       zero,       zero,
        zero,       one,       zero,       zero,
        zero,       zero,       one,       zero,
        trans.x, trans.y, trans.z, one
        );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromScaleQuaternion(const Vector3Template<T> &scale, const QuaternionTemplate<T> &q)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T yz = q.y * q.z;
    T wx = q.w * q.x;
    T wy = q.w * q.y;
    T wz = q.w * q.z;

    Matrix44Template<T> m(
            scale.x * (one-two*(yy+zz)), scale.x *two*(xy+wz), scale.x *two*(xz-wy), zero,
            scale.y *two*(xy-wz), scale.y * (one-two*(xx+zz)), scale.y *two*(yz+wx), zero,
            scale.z *two*(xz+wy), scale.z *two*(yz-wx), scale.z * (one-two*(xx+yy)), zero,
                    zero,                 zero,               zero, one
    );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromScaleTranslation(const Vector3Template<T> &scale, const Vector3Template<T> &trans)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);

    Matrix44Template<T> m(
        scale.x, zero, zero, zero,
        zero, scale.y, zero, zero,
        zero, zero, scale.z, zero,
        trans.x, trans.y, trans.z, one
    );
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44TransposeRotationalComponent(const Matrix44Template<T> &m)
{
    Matrix44Template<T> result = m;

    // transpose rotation component of matrix
    result.SetElem(1, m.GetElem(4));
    result.SetElem(4, m.GetElem(1));

    result.SetElem(2, m.GetElem(8));
    result.SetElem(8, m.GetElem(2));

    result.SetElem(6, m.GetElem(9));
    result.SetElem(9, m.GetElem(6));
    return result;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44Translate(const Matrix44Template<T> &m, const Vector3Template<T> &trans)
{
    // multiply translations by current matrix
    Vector4Template<T> translationVector(
        (trans.x * m.GetElem( 0)) + (trans.y * m.GetElem( 4)) + (trans.z * m.GetElem( 8)),
        (trans.x * m.GetElem( 1)) + (trans.y * m.GetElem( 5)) + (trans.z * m.GetElem( 9)),
        (trans.x * m.GetElem( 2)) + (trans.y * m.GetElem( 6)) + (trans.z * m.GetElem(10)),
        static_cast<T>(0.0f)
        );

    // and translate current matrix by rotated translation
    Matrix44Template<T> rm(
        m.xAxis,
        m.yAxis,
        m.zAxis,
        m.wAxis + translationVector
        );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromQuaternionTranslation(const QuaternionTemplate<T> &q, const Vector3Template<T> &trans)
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

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);

    Matrix44Template<T> m(
            one-two*(yy+zz),    two*(xy+wz),        two*(xz-wy),        zero,
            two*(xy-wz),        one-two*(xx+zz),    two*(yz+wx),        zero,
            two*(xz+wy),        two*(yz-wx),        one-two*(xx+yy),    zero,
            trans.x,        trans.y,        trans.z,        one
            );
    return m;
}

template<class T>
RW_MATH_FORCE_INLINE Matrix44Template<T> Matrix44FromScaleQuaternionTranslation(const Vector3Template<T> &scale, const QuaternionTemplate<T> &q, const Vector3Template<T> &trans);

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromScaleQuaternionTranslation(const Vector3Template<T> &scale, const QuaternionTemplate<T> &q, const Vector3Template<T> &trans)
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

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);

    Matrix44Template<T> m(
            scale.x*(one-two*(yy+zz)), scale.x*two*(xy+wz),     scale.x*two*(xz-wy),        zero,
            scale.y*two*(xy-wz),        scale.y*(one-two*(xx+zz)),  scale.y*two*(yz+wx),        zero,
            scale.z*two*(xz+wy),        scale.z*two*(yz-wx),        scale.z*(one-two*(xx+yy)),  zero,
            trans.x,                    trans.y,                    trans.z,                    one
            );
    return m;
}

template<class T>
RW_MATH_INLINE Vector4Template<T> Mult3x3(const Vector4Template<T> &v, const Matrix44Template<T> &m)
{
    Matrix33Template<T> m33(GetXYZ(m.XAxis()), GetXYZ(m.YAxis()), GetXYZ(m.ZAxis()));
    Vector3Template<T> v3 = Mult(GetXYZ(v), m33);
    Vector4Template<T> v4( v3.x, v3.y, v3.z, v.w );
    return v4;
}

template<class T>
RW_MATH_INLINE Vector3Template<T> Mult3x3(const Vector3Template<T> &v, const Matrix44Template<T> &m)
{
    Matrix33Template<T> m33(GetXYZ(m.XAxis()), GetXYZ(m.YAxis()), GetXYZ(m.ZAxis()));
    Vector3Template<T> v3 = Mult(v, m33);
    return v3;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Inverse3x3(const Matrix44Template<T> &m)
{
    T determinant;
    return Inverse3x3(m, determinant);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Inverse3x3(const Matrix44Template<T> &m, T &determinant)
{
    determinant = Determinant3x3(m);
    RW_MATH_ASSERT_MSG((determinant != 0.0f), ("Matrix is singular."));
    if(determinant==static_cast<T>(0.0f))
    {
        return GetMatrix44_ZeroTemplate<T>();
    }
    else
    {
        Matrix44Template<T> result;
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
            }
            result.SetElem(i,3, m.GetRow(i).w);
        }
        result.WAxis() = m.WAxis();
        return result;
    }
}

template<class T>
RW_MATH_INLINE T Determinant3x3(const Matrix44Template<T> &m)
{
    Matrix33Template<T> m33(GetXYZ(m.XAxis()), GetXYZ(m.YAxis()), GetXYZ(m.ZAxis()));
    return Determinant( m33 );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> TransformPoint(const Vector3Template<T>& pt, const Matrix44Template<T>& matrix)
{
    return Vector3Template<T>(matrix.xAxis.x * pt.x + matrix.yAxis.x * pt.y + matrix.zAxis.x * pt.z + matrix.wAxis.x,
                              matrix.xAxis.y * pt.x + matrix.yAxis.y * pt.y + matrix.zAxis.y * pt.z + matrix.wAxis.y,
                              matrix.xAxis.z * pt.x + matrix.yAxis.z * pt.y + matrix.zAxis.z * pt.z + matrix.wAxis.z);
}

template<class T>
RW_MATH_INLINE void TransformPoints(const Vector3Template<T>* const pts, int numPoints, Matrix44Template<T> matrix, Vector3Template<T>* transformedPts)
{
    for( int point = 0; point < numPoints; point++ )
    {
        transformedPts[point] = TransformPoint(pts[point], matrix);
    }
}

template<class T>
RW_MATH_INLINE Vector3Template<T> TransformVector(const Vector3Template<T>& vec, const Matrix44Template<T>& matrix)
{
    Vector3Template<T> transformedVector = Vector3Template<T>(
                            matrix.xAxis.x * vec.x + matrix.yAxis.x * vec.y + matrix.zAxis.x * vec.z,
                            matrix.xAxis.y * vec.x + matrix.yAxis.y * vec.y + matrix.zAxis.y * vec.z,
                            matrix.xAxis.z * vec.x + matrix.yAxis.z * vec.y + matrix.zAxis.z * vec.z);
    return transformedVector;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Mult(const Matrix44AffineTemplate<T> &m, const Matrix44Template<T> &b)
{
    #define rwmath_fpu_detail_TRANSFORM_VECTOR( v3, m )                 \
    Vector4Template<T>(                                        \
            m.xAxis.x * v3.x + m.yAxis.x * v3.y + m.zAxis.x * v3.z,     \
            m.xAxis.y * v3.x + m.yAxis.y * v3.y + m.zAxis.y * v3.z,     \
            m.xAxis.z * v3.x + m.yAxis.z * v3.y + m.zAxis.z * v3.z,     \
            m.xAxis.w * v3.x + m.yAxis.w * v3.y + m.zAxis.w * v3.z      \
            )

    #define rwmath_fpu_detail_TRANSFORM_POINT( v3, m )                          \
        Vector4Template<T>(                                            \
            m.xAxis.x * v3.x + m.yAxis.x * v3.y + m.zAxis.x * v3.z + m.wAxis.x, \
            m.xAxis.y * v3.x + m.yAxis.y * v3.y + m.zAxis.y * v3.z + m.wAxis.y, \
            m.xAxis.z * v3.x + m.yAxis.z * v3.y + m.zAxis.z * v3.z + m.wAxis.z, \
            m.xAxis.w * v3.x + m.yAxis.w * v3.y + m.zAxis.w * v3.z + m.wAxis.w  \
            )

    return Matrix44Template<T>(
                rwmath_fpu_detail_TRANSFORM_VECTOR(m.xAxis, b),
                rwmath_fpu_detail_TRANSFORM_VECTOR(m.yAxis, b),
                rwmath_fpu_detail_TRANSFORM_VECTOR(m.zAxis, b),
                rwmath_fpu_detail_TRANSFORM_POINT( m.wAxis, b)
                );

    #undef rwmath_fpu_detail_TRANSFORM_VECTOR
    #undef rwmath_fpu_detail_TRANSFORM_POINT
}

template<class T>
RW_MATH_INLINE void TransformVectors(const Vector3Template<T>* const vecs, int numVectors, Matrix44Template<T> matrix, Vector3Template<T> * transformedVecs)
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = TransformVector(vecs[vecIndex], matrix);
    }
}

template<class T>
RW_MATH_INLINE Vector3Template<T> Transform(const Vector4Template<T>& vec, const Matrix44Template<T>& matrix)
{
    Vector3Template<T> transformedVector = Vector3Template<T>(
                matrix.xAxis.x * vec.x + matrix.yAxis.x * vec.y + matrix.zAxis.x * vec.z + matrix.wAxis.x * vec.w,
                matrix.xAxis.y * vec.x + matrix.yAxis.y * vec.y + matrix.zAxis.y * vec.z + matrix.wAxis.y * vec.w,
                matrix.xAxis.z * vec.x + matrix.yAxis.z * vec.y + matrix.zAxis.z * vec.z + matrix.wAxis.z * vec.w);
    return transformedVector;
}

template<class T>
RW_MATH_INLINE void Transform(const Vector4Template<T>* const vecs, int numVectors, const Matrix44Template<T>& matrix, Vector3Template<T> * transformedVecs)
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = Transform(vecs[vecIndex], matrix);
    }
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromAxisRotationAngle(const Vector3Template<T>& axis, T angle)
{
    T s, c;

    SinCos(angle, s, c);
    T t = static_cast<T>(1.0f) - c;
    T tx = t * axis.x;
    T ty = t * axis.y;
    T tz = t * axis.z;
    T sx = s * axis.x;
    T sy = s * axis.y;
    T sz = s * axis.z;

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);

    Matrix44Template<T> rm(
        tx * axis.x + c ,
        tx * axis.y + sz,
        tx * axis.z - sy,
        zero,

        ty * axis.x - sz,
        ty * axis.y + c ,
        ty * axis.z + sx,
        zero,

        tz * axis.x + sy,
        tz * axis.y - sx,
        tz * axis.z + c,
        zero,

        zero,
        zero,
        zero,
        one
        );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromDiagonal(T a)
{
    Matrix44Template<T> rm(
         a,0,0,0,
         0,a,0,0,
         0,0,a,0,
         0,0,0,a
         );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromXRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    Matrix44Template<T> rm(
         one,   zero,   zero,  zero,
         zero,   c,   s,  zero,
         zero,  -s,   c,  zero,
         zero,   zero,   zero,  one
         );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromYRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    Matrix44Template<T> rm(
         c,   zero,  -s,  zero,
         zero,   one,   zero,  zero,
         s,   zero,   c,  zero,
         zero,   zero,   zero,  one
         );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromZRotationAngle(T angle)
{
    T s,c;
    SinCos(angle,s,c);
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    Matrix44Template<T> rm(
         c,   s,   zero,  zero,
        -s,   c,   zero,  zero,
         zero,   zero,   one,  zero,
         zero,   zero,   zero,  one );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> OrthoNormalize(const Matrix44Template<T>& m)
{
    // Note: This implementation is using the Gram-Schmidt orthonormalization
    // algorithm.  The current implementation is numerically unstable due to
    // rounding error. Also, this algorithm differs from the algorithm implemented
    // for matrix 3x3 orthonormalization.
    Matrix44Template<T> result;
    const int numRows = 4;
    for( int row = 0; row < numRows; ++row )
    {
        // Initialize the row to the same row from the input matrix.
        const Vector4Template<T> &inputRow = m.GetRow(row);
        Vector4Template<T> tempResult = inputRow;
        // Subtract the components of the vector that are not orthogonal to the basis created thus far.
        for( int i = 0; i < row; ++i )
        {
            tempResult -= result.GetRow(i) * Dot(result.GetRow(i), inputRow );
        }
        result.SetRow( row, Normalize(tempResult) );
    }
    return result;
}

template<class T>
RW_MATH_INLINE T GetOrthogonalError(const Matrix44Template<T>& matrix)
{
    Matrix44Template<T> identity;
    identity.SetIdentity();

    Matrix44Template<T> result = Mult(matrix, Transpose(matrix));
    SelfSubtract(result, identity);

    Vector4Template<T> errorVec( MagnitudeSquared( result.XAxis() ),
                                 MagnitudeSquared( result.YAxis() ),
                                 MagnitudeSquared( result.ZAxis() ),
                                 MagnitudeSquared( result.WAxis() ) );

    T error = MagnitudeSquared(errorVec);
    return error;
}

template<class T>
RW_MATH_INLINE T GetOrthogonalRowSetError(const Matrix44Template<T>& matrix)
{
    // Calculate error vectors that "compare" all possible pairs of elements.
    // i.e. 4 choose 2 combinations.
    Vector3Template<T> errorVec1(   Dot( matrix.XAxis(), matrix.YAxis() ),
                                                    Dot( matrix.XAxis(), matrix.ZAxis() ),
                                                    Dot( matrix.XAxis(), matrix.WAxis() ) );
    Vector3Template<T> errorVec2(   Dot( matrix.YAxis(), matrix.ZAxis() ),
                                                    Dot( matrix.YAxis(), matrix.WAxis() ),
                                                    Dot( matrix.ZAxis(), matrix.WAxis() ) );
    T error = MagnitudeSquared(errorVec1) + MagnitudeSquared(errorVec2);
    return error;
}

template<class T>
RW_MATH_INLINE bool IsOrthogonal(Matrix44Template<T> a, T tolerance=EPSILON)
{
    return IsZero( GetOrthogonalError(a), tolerance );
}

template<class T>
RW_MATH_INLINE bool IsOrthogonalRowSet(Matrix44Template<T> a, T tolerance=EPSILON)
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
template<class T> RW_MATH_INLINE bool IsRotation(const Matrix44Template<T>& a, T tolerance = EPSILON)
{
    return IsOrthogonal(a, tolerance) && IsSimilar(Determinant(a), static_cast<T>(1.0f), tolerance);
}

template<class T>
RW_MATH_INLINE T GetNormalError(const Matrix44Template<T>& matrix)
{
    Vector4Template<T> errorVec(    MagnitudeSquared( matrix.XAxis() ) - static_cast<T>(1.0f),
                                                MagnitudeSquared( matrix.YAxis() ) - static_cast<T>(1.0f),
                                                MagnitudeSquared( matrix.ZAxis() ) - static_cast<T>(1.0f),
                                                MagnitudeSquared( matrix.WAxis() ) - static_cast<T>(1.0f) );
    T error = MagnitudeSquared(errorVec);
    return error;
}

template<class T>
RW_MATH_INLINE bool IsNormal(const Matrix44Template<T>& a, T tolerance=EPSILON)
{
    return IsZero( GetNormalError(a), tolerance );
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> InverseOrthonormal(const Matrix44Template<T>& m)
{
    RW_MATH_ASSERT_MSG((IsOrthogonal<T>(m)), ("Input matrix is non orthonormal."));
    return Transpose(m);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> RotateAroundXAxis(const Matrix44Template<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //    [         e00                e01                e02                e03       ]
    //    [ e20 SIN + e10 COS  e21 SIN + e11 COS  e22 SIN + e12 COS  e23 SIN + e13 COS ]
    //    [ e20 COS - e10 SIN  e21 COS - e11 SIN  e22 COS - e12 SIN  e23 COS - e13 SIN ]
    //    [         e30                e31                e32                e33       ]
    T s,c;
    SinCos(angle, s, c);

    // rotate incrementally about x
    return Matrix44Template<T>(
        m.XAxis(),
        m.ZAxis() * s + m.YAxis() * c,
        m.ZAxis() * c - m.YAxis() * s,
        m.WAxis()
    );
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> RotateAroundYAxis(const Matrix44Template<T>& m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e00 COS - e20 SIN  e01 COS - e21 SIN  e02 COS - e22 SIN  e03 COS - e23 SIN ]
    //  [         e10                e11                e12                e13       ]
    //  [ e00 SIN + e20 COS  e01 SIN + e21 COS  e02 SIN + e22 COS  e03 SIN + e23 COS ]
    //  [         e30                e31                e32                e33       ]
    T s,c;

    SinCos(angle, s, c);

    // rotate incrementally about y
    return Matrix44Template<T>(
        m.XAxis() * c - m.ZAxis() * s,
        m.YAxis(),
        m.XAxis() * s + m.ZAxis() * c,
        m.WAxis()
    );
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> RotateAroundZAxis(Matrix44Template<T> m, T angle)
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e10 SIN + e00 COS  e11 SIN + e01 COS  e12 SIN + e02 COS  e13 SIN + e03 COS ]
    //  [ e10 COS - e00 SIN  e11 COS - e01 SIN  e12 COS - e02 SIN  e13 COS - e03 SIN ]
    //  [         e20                e21                e22                e23       ]
    //  [         e30                e31                e32                e33       ]
    T s,c;

    SinCos(angle, s, c);

    // rotate incrementally about z
    return Matrix44Template<T>(
        m.YAxis() * s + m.XAxis() * c,
        m.YAxis() * c - m.XAxis() * s,
        m.ZAxis(),
        m.WAxis()
    );
}

/*!
 * \brief Creates a perpective projection matrix for a left handed coordinate system.
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromPerspectiveLH(T w, T h, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
        two*zn/w,      zero,          zero,     zero,
            zero,  two*zn/h,          zero,     zero,
            zero,      zero,    zf/(zf-zn),      one,
            zero,      zero, zn*zf/(zn-zf),     zero );
    return rm;
}

/*!
 * \brief Creates a perpective projection matrix for a right handed coordinate system.
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromPerspectiveRH(T w, T h, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T negOne = static_cast<T>(-1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
        two*zn/w,      zero,          zero,     zero,
            zero,  two*zn/h,          zero,     zero,
            zero,      zero,    zf/(zn-zf),   negOne,
            zero,      zero, zn*zf/(zn-zf),     zero );
    return rm;
}

/*!
 * \brief Creates a perpective projection matrix for a right handed coordinate system.
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
RW_MATH_FORCE_INLINE Matrix44Template<T> Matrix44FromPerspective(T w, T h, T zn, T zf)
{
    return Matrix44FromPerspectiveRH<T>(w, h, zn, zf);
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromOrthoLH(T w, T h, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
       two/w,   zero,          zero,    zero,
        zero,  two/h,          zero,    zero,
        zero,   zero,   one/(zf-zn),    zero,
        zero,   zero,   -zn/(zf-zn),     one
        );
    return rm;
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromOrthoRH(T w, T h, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
       two/w,   zero,          zero,    zero,
        zero,  two/h,          zero,    zero,
        zero,   zero,   one/(zn-zf),    zero,
        zero,   zero,    zn/(zn-zf),     one
        );
    return rm;
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
RW_MATH_FORCE_INLINE Matrix44Template<T> Matrix44FromOrtho(T w, T h, T zn, T zf)
{
    return Matrix44FromOrthoRH<T>(w, h, zn, zf);
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromOrthoOffCenterLH(T l, T r, T b, T t, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
        two/(r-l),           zero,          zero,    zero,
             zero,      two/(t-b),          zero,    zero,
             zero,           zero,   one/(zf-zn),    zero,
      (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf),     one
      );
    return rm;
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromOrthoOffCenterRH(T l, T r, T b, T t, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
        two/(r-l),           zero,          zero,    zero,
             zero,      two/(t-b),          zero,    zero,
             zero,           zero,   one/(zn-zf),    zero,
      (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf),     one
      );
    return rm;
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
RW_MATH_FORCE_INLINE Matrix44Template<T> Matrix44FromOrthoOffCenter(T l, T r, T b, T t, T zn, T zf)
{
    return Matrix44FromOrthoOffCenterRH<T>(l, r, b, t, zn, zf);
}

/*!
 * \brief Creates a perpective projection matrix for a left handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param fov the field of view of the frustum (measured in the y plane in radians)
 * \param aspect the aspect ration (i.e. width/height) of the frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromPerspectiveFovLH(T fov, T aspect, T zn, T zf)
{
    // same as gluperspective except our fov is already in radians whereas opengl's parameter is in degrees
    T h = Tan(fov/static_cast<T>(2.0f));
    T w = h * aspect;

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    Matrix44Template<T> rm(
         one/w,   zero,            zero,       zero,
          zero,  one/h,            zero,       zero,
          zero,   zero,      zf/(zf-zn),        one,
          zero,   zero,  -zn*zf/(zf-zn),       zero
          );
    return rm;
}

/*!
 * \brief Creates a perpective projection matrix for a right handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param fov the field of view of the frustum (measured in the y plane in radians)
 * \param aspect the aspect ration (i.e. width/height) of the frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromPerspectiveFovRH(T fov, T aspect, T zn, T zf)
{
    // same as gluperspective except our fov is already in radians whereas opengl's parameter is in degrees
    T h = Tan(fov/static_cast<T>(2.0f));
    T w = h * aspect;

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T negOne = static_cast<T>(-1.0f);
    Matrix44Template<T> rm(
         one/w,   zero,            zero,       zero,
          zero,  one/h,            zero,       zero,
          zero,   zero,      zf/(zn-zf),     negOne,
          zero,   zero,   zn*zf/(zn-zf),       zero
          );
    return rm;
}

/*!
 * \brief Creates a perpective projection matrix for a right handed coordinate system.
 *
 * See \ref p_projection_matrix_functions matrices for specifics about how RWMath projection matrices transform
 * 3D points.
 *
 * \param fov the field of view of the frustum (measured in the y plane in radians)
 * \param aspect the aspect ration (i.e. width/height) of the frustum
 * \param zn the distance to the z near plane.  This should be a positive value.
 * \param zf the distance to the z far plane.  This should be a positive value.
 */
template<class T>
RW_MATH_FORCE_INLINE Matrix44Template<T> Matrix44FromPerspectiveFov(T fov, T aspect, T zn, T zf)
{
    return Matrix44FromPerspectiveFovRH<T>(fov, aspect, zn, zf);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromLookAtLH(const Vector3Template<T>& lookAt, const Vector3Template<T>& up, const Vector3Template<T>& eyePosition)
{
    // calculate the 3 axes
    Vector3Template<T> zaxis = Normalize(Subtract(lookAt, eyePosition));
    Vector3Template<T> xaxis = Normalize(Cross(up, zaxis));
    Vector3Template<T> yaxis = Cross(zaxis, xaxis);

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    Matrix44Template<T> rm(
        xaxis.x, yaxis.x, zaxis.x, zero,
        xaxis.y, yaxis.y, zaxis.y, zero,
        xaxis.z, yaxis.z, zaxis.z, zero,
        -Dot(xaxis, eyePosition), -Dot(yaxis, eyePosition), -Dot(zaxis, eyePosition), one
        );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromLookAtRH(const Vector3Template<T>& lookAt, const Vector3Template<T>& up, const Vector3Template<T>& eyePosition)
{
    // calculate the 3 axes
    Vector3Template<T> zaxis = Normalize(Subtract(eyePosition, lookAt));
    Vector3Template<T> xaxis = Normalize(Cross(up, zaxis));
    Vector3Template<T> yaxis = Cross(zaxis, xaxis);

    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    Matrix44Template<T> rm(
        xaxis.x, yaxis.x, zaxis.x, zero,
        xaxis.y, yaxis.y, zaxis.y, zero,
        xaxis.z, yaxis.z, zaxis.z, zero,
        -Dot(xaxis, eyePosition), -Dot(yaxis, eyePosition), -Dot(zaxis, eyePosition), one
        );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Matrix44FromLookAt(const Vector3Template<T>& lookAt, const Vector3Template<T>& up, const Vector3Template<T>& eyePosition)
{
    return Matrix44FromLookAtRH(lookAt, up, eyePosition);
}

/*!
 * \brief Creates an off center perspective projection matrix for a left handed coordinate system.
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromPerspectiveOffCenterLH(T l, T r, T b, T t, T zn, T zf)
{
    const T zero = static_cast<T>(0.0f);
    const T one = static_cast<T>(1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
        two*zn/(r-l),           zero,             zero,    zero,
                zero,   two*zn/(t-b),             zero,    zero,
        (l+r)/(l-r),     (t+b)/(b-t),       zf/(zf-zn),     one,
               zero,            zero,    zn*zf/(zn-zf),    zero
        );
    return rm;
}

/*!
 * \brief Creates an off center perspective projection matrix for a right handed coordinate system.
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
RW_MATH_INLINE Matrix44Template<T> Matrix44FromPerspectiveOffCenterRH(T l, T r, T b, T t, T zn, T zf)
{
    // Note: equiv to D3DXMatrixPerspectiveOffCenterRH()
    const T zero = static_cast<T>(0.0f);
    const T negOne = static_cast<T>(-1.0f);
    const T two = static_cast<T>(2.0f);
    Matrix44Template<T> rm(
        two*zn/(r-l),           zero,            zero,    zero,
                zero,   two*zn/(t-b),            zero,    zero,
         (l+r)/(r-l),    (t+b)/(t-b),      zf/(zn-zf),  negOne,
                zero,           zero,   zn*zf/(zn-zf),    zero
        );
    return rm;
}

/*!
 * \brief Creates an off center perspective projection matrix for a right handed coordinate system.
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
RW_MATH_FORCE_INLINE Matrix44Template<T> Matrix44FromPerspectiveOffCenter(T l, T r, T b, T t, T zn, T zf)
{
    return Matrix44FromPerspectiveOffCenterRH<T>(l, r, b, t, zn, zf);
}

template<class T>
RW_MATH_INLINE T GetNormalError3x3(const Matrix44Template<T>& matrix)
{
    Matrix33Template<T> m33(
                                            matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
                                            matrix.GetElem(4), matrix.GetElem(5), matrix.GetElem(6),
                                            matrix.GetElem(8), matrix.GetElem(9), matrix.GetElem(10));
    return GetNormalError<T>( m33 );
}

template<class T>
RW_MATH_INLINE bool IsNormal3x3(const Matrix44Template<T>& matrix, T tolerance=EPSILON)
{
    return IsZero( GetNormalError3x3(matrix), tolerance );
}

template<class T>
RW_MATH_INLINE T GetOrthogonalRowSetError3x3(const Matrix44Template<T>& matrix)
{
    Matrix33Template<T> m33(
                                            matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
                                            matrix.GetElem(4), matrix.GetElem(5), matrix.GetElem(6),
                                            matrix.GetElem(8), matrix.GetElem(9), matrix.GetElem(10));
    return GetOrthogonalRowSetError<T>( m33 );
}

template<class T>
RW_MATH_INLINE T GetOrthogonalError3x3(const Matrix44Template<T>& matrix)
{
    Matrix33Template<T> m33(
                                            matrix.GetElem(0), matrix.GetElem(1), matrix.GetElem(2),
                                            matrix.GetElem(4), matrix.GetElem(5), matrix.GetElem(6),
                                            matrix.GetElem(8), matrix.GetElem(9), matrix.GetElem(10));
    return GetOrthogonalError<T>( m33 );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> QueryRotateDegenerateUnitAxis(const Matrix44Template<T>& matrix)
{
    Vector3Template<T> unitAxis;
    if(matrix.XAxis().x > matrix.YAxis().y)
    {
        if(matrix.XAxis().x > matrix.ZAxis().z)
        {
            T xVal = static_cast<T>(1.0f) + matrix.XAxis().x;
            unitAxis.x =xVal*xVal;
            unitAxis.y =matrix.XAxis().y + matrix.YAxis().x;
            unitAxis.z =matrix.XAxis().z + matrix.ZAxis().x;
        }
        else
        {
            T zVal = static_cast<T>(1.0f) + matrix.ZAxis().z;
            unitAxis.z =zVal*zVal;
            unitAxis.x =matrix.ZAxis().x + matrix.XAxis().z;
            unitAxis.y =matrix.ZAxis().y + matrix.YAxis().z;
        }
    }
    else
    {
        if(matrix.YAxis().y > matrix.ZAxis().z)
        {
            T yVal = static_cast<T>(1.0f) + matrix.YAxis().y;
            unitAxis.y =yVal*yVal;
            unitAxis.z =matrix.YAxis().z + matrix.ZAxis().y;
            unitAxis.x =matrix.YAxis().x + matrix.XAxis().y;
        }
        else
        {
            T zVal = static_cast<T>(1.0f) + matrix.ZAxis().z;
            unitAxis.z =zVal*zVal;
            unitAxis.x =matrix.ZAxis().x + matrix.XAxis().z;
            unitAxis.y =matrix.ZAxis().y + matrix.YAxis().z;
        }
    }
    unitAxis = Normalize( unitAxis );
    return unitAxis;
}

template<class T>
RW_MATH_INLINE void QueryRotate(const Matrix44Template<T>& matrix, Vector3Template<T>& unitAxis, T& angle, Vector3Template<T>& center)
{
    Vector3Template<T> twoSinThetaAxis;
    T twoCosTheta = matrix.XAxis().x + matrix.YAxis().y + matrix.ZAxis().z - static_cast<T>(1.0f);
    twoSinThetaAxis.x =matrix.YAxis().z - matrix.ZAxis().y;
    twoSinThetaAxis.y =matrix.ZAxis().x - matrix.XAxis().z;
    twoSinThetaAxis.z =matrix.XAxis().y - matrix.YAxis().x;
    T twoSinTheta = Magnitude(twoSinThetaAxis);
    if(twoSinTheta > static_cast<T>(0.0f))
    {
        const T lrRecipLength = static_cast<T>(1.0f) / twoSinTheta;
        unitAxis = twoSinThetaAxis * lrRecipLength;
    }
    else
    {
        unitAxis = Vector3Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
    }
    angle = ATan2(twoSinTheta, twoCosTheta);
    if((twoSinTheta <= T(MINIMUM_RECIPROCAL)) && (twoCosTheta <= static_cast<T>(0.0f)))
    {
        unitAxis = QueryRotateDegenerateUnitAxis(matrix);
    }
    Vector3Template<T> axisXpos = Cross(unitAxis, GetXYZ(matrix.WAxis()));
    center = Cross(axisXpos, unitAxis) * static_cast<T>(0.5f);
    center += axisXpos * (static_cast<T>(0.5f) * twoSinTheta / (static_cast<T>(2.0f) - twoCosTheta));
}

template<class T>
RW_MATH_INLINE bool IsOrthogonal3x3(const Matrix44Template<T>& matrix, T tolerance=EPSILON)
{
    return IsZero( GetOrthogonalError3x3(matrix), tolerance );
}

template<class T>
RW_MATH_INLINE bool IsOrthogonalRowSet3x3(const Matrix44Template<T>& matrix, T tolerance=EPSILON)
{
    return IsZero( GetOrthogonalRowSetError3x3(matrix), tolerance );
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> OrthoNormalize3x3(const Matrix44Template<T>& matrix)
{
    Vector3Template<T> *vpU, *vpV, *vpW;

    Vector3Template<T> right         = Normalize(GetXYZ(matrix.XAxis()));
    Vector3Template<T> up            = Normalize(GetXYZ(matrix.YAxis()));
    Vector3Template<T> at            = Normalize(GetXYZ(matrix.ZAxis()));
    const Vector4Template<T> &pos   = matrix.WAxis();

    Vector3Template<T> vInner( Magnitude(GetXYZ(matrix.XAxis())),
                                              Magnitude(GetXYZ(matrix.YAxis())),
                                              Magnitude(GetXYZ(matrix.ZAxis())) );

    if (vInner.x > static_cast<T>(0.0f))
    {
        if (vInner.y > static_cast<T>(0.0f))
        {
            if (vInner.z > static_cast<T>(0.0f))
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
    Matrix44Template<T> result;
    result.XAxis() = Vector4Template<T>(right, static_cast<T>(0.0f));
    result.YAxis() = Vector4Template<T>(up,    static_cast<T>(0.0f));
    result.ZAxis() = Vector4Template<T>(at,    static_cast<T>(0.0f));
    result.WAxis() = pos;

    // Set the last column to [0,0,0,1]
    result.WAxis().w = static_cast<T>(1.0f);
    return result;
}

template<class T>
RW_MATH_INLINE Matrix33Template<T> Get3x3(const Matrix44Template<T>& matrix)
{
    return Matrix33Template<T>(GetXYZ(matrix.xAxis), GetXYZ(matrix.yAxis), GetXYZ(matrix.zAxis));
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> Set3x3(const Matrix44Template<T>& matrix, const Matrix33Template<T>& m33)
{
    return Matrix44Template<T>(m33.xAxis.x, m33.xAxis.y, m33.xAxis.z, matrix.xAxis.w,
                               m33.yAxis.x, m33.yAxis.y, m33.yAxis.z, matrix.yAxis.w,
                               m33.zAxis.x, m33.zAxis.y, m33.zAxis.z, matrix.zAxis.w,
                               matrix.wAxis.x, matrix.wAxis.y, matrix.wAxis.z, matrix.wAxis.w);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> ScaleRow(const Matrix44Template<T>& m, T r0, T r1, T r2)
{
    Vector4Template<T> row0Scale = Vector4Template<T>( r0, r0, r0, static_cast<T>(1.0f) );
    Vector4Template<T>row1Scale = Vector4Template<T>( r1, r1, r1, static_cast<T>(1.0f) );
    Vector4Template<T> row2Scale = Vector4Template<T>( r2, r2, r2, static_cast<T>(1.0f) );
    Matrix44Template<T> rm(
        Mult( row0Scale, m.xAxis ),
        Mult( row1Scale, m.yAxis ),
        Mult( row2Scale, m.zAxis ),
        m.wAxis
        );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> ScaleColumn(const Matrix44Template<T>& m, T c0, T c1, T c2 )
{
    Vector4Template<T> rowMultiplier(c0, c1, c2, static_cast<T>(1.0f));
    Matrix44Template<T> rm(
        Mult( m.xAxis, rowMultiplier ),
        Mult( m.yAxis, rowMultiplier ),
        Mult( m.zAxis, rowMultiplier ),
        m.wAxis
        );
    return rm;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T>& operator *=(Matrix44Template<T> &m, T a)
{
    m = Mult(m, a);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T>& operator /=(Matrix44Template<T> &m, T a)
{
    m = Divide(m, a);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T>& operator *=(Matrix44Template<T> &m, const Matrix44Template<T> &b)
{
    m = Mult(m, b);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T>& operator +=(Matrix44Template<T> &m, const Matrix44Template<T> &b)
{
    m = Add(m, b);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T>& operator -=(Matrix44Template<T> &m, const Matrix44Template<T> &b)
{
    m = Subtract(m, b);
    return m;
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator - (const Matrix44Template<T> &m)
{
    return Negate(m);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator * (const Matrix44Template<T> &m, T a)
{
    return Mult(m, a);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator * (T a, const Matrix44Template<T> &b)
{
    return Mult(b, a);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator / (const Matrix44Template<T> &m, T a)
{
    return Divide(m, a);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator + (const Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    return Add(a, b);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator - (const Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    return Subtract(a, b);
}

template<class T>
RW_MATH_INLINE Matrix44Template<T> operator * (const Matrix44Template<T> &a, const Matrix44Template<T> &b)
{
    return Mult(a, b);
}

template<class T>
RW_MATH_INLINE Vector4Template<T> operator * (const Vector4Template<T> &v, const Matrix44Template<T> &m)
{
    return Mult(v, m);
}

template <class T>
RW_MATH_INLINE Matrix44Template<T> Select(const MaskScalarTemplate<T> &mask, const Matrix44Template<T> &trueValue, const Matrix44Template<T> &falseValue )
{
    return Matrix44Template<T>(
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
template<class T>  bool IsRotation3x3(const Matrix44Template<T>& a, T tolerance = EPSILON)
{
    return IsOrthogonal3x3(a, tolerance) && IsSimilar(Determinant3x3(a), static_cast<T>(1.0f), tolerance);
}

#if RW_MATH_ENABLE_DEPRECATED_API


template<class T>
RW_MATH_INLINE void Negate(const Matrix44Template<T> &m,    Matrix44Template<T> &result)
{
    result = Negate(m);
}


template<class T>
RW_MATH_INLINE void Mult(const Matrix44Template<T> &m, T a, Matrix44Template<T> &result)
{
    result = Mult(m, a);
}


template<class T>
RW_MATH_INLINE void Mult(const Vector4Template<T> &v, const Matrix44Template<T> &m, Vector4Template<T> &result)
{
    result = Mult(v, m);
}


template<class T>
RW_MATH_INLINE void Mult(const Matrix44Template<T> &m,    const Matrix44Template<T> &b,    Matrix44Template<T> &result)
{
    result = Mult(m, b);
}


template<class T>
RW_MATH_INLINE void Divide(const Matrix44Template<T> &m, T a, Matrix44Template<T> &result)
{
    result = Divide(m, a);
}


template<class T>
RW_MATH_INLINE void Add(const Matrix44Template<T> &a, const Matrix44Template<T> &b, Matrix44Template<T> &result)
{
    result = Add(a, b);
}


template<class T>
RW_MATH_INLINE void Subtract(const Matrix44Template<T> &a, const Matrix44Template<T> &b, Matrix44Template<T> &result)
{
    result = Subtract(a, b);
}


template<class T>
RW_MATH_INLINE void Transpose(const Matrix44Template<T> &m, Matrix44Template<T> &result)
{
    result = Transpose(m);
}


template<class T>
RW_MATH_INLINE T Inverse(const Matrix44Template<T> &m, Matrix44Template<T> &result)
{
    T determinant;
    result = Inverse(m, determinant);
    return determinant;
}


template<class T>
RW_MATH_INLINE void Matrix44FromQuaternion(const QuaternionTemplate<T> &q, Matrix44Template<T> &result)
{
    result = Matrix44FromQuaternion<T>(q);
}


template<class T>
RW_MATH_INLINE void Matrix44FromAxisAngle(const Vector3Template<T> &axis, T angle, Matrix44Template<T> &result)
{
    result = Matrix44FromAxisRotationAngle(axis, angle);
}


template<class T>
RW_MATH_INLINE void Matrix44FromEulerXYZ(const Vector3Template<T> &euler, Matrix44Template<T> &result)
{
    result = Matrix44FromEulerXYZ(euler);
}


template<class T>
RW_MATH_INLINE void EulerXYZFromMatrix44(const Matrix44Template<T>& m, Vector3Template<T> &result)
{
    result = EulerXYZFromMatrix44(m);
}


template<class T>
RW_MATH_INLINE void Matrix44InverseRotTrans(const Matrix44Template<T> &m, Matrix44Template<T> &result)
{
    result = Matrix44InverseRotationalComponent(m);
}


template<class T>
RW_MATH_INLINE void Matrix44FromScale(const Vector3Template<T> &scale, Matrix44Template<T> &result)
{
    result = Matrix44FromScale(scale);
}


template<class T>
RW_MATH_INLINE void Matrix44FromTranslation(const Vector3Template<T> &trans, Matrix44Template<T> &result)
{
    result = Matrix44FromTranslation(trans);
}


template<class T>
RW_MATH_INLINE void Matrix44FromScaleQuaternion(const Vector3Template<T> &scale, const QuaternionTemplate<T> &q, Matrix44Template<T> &result)
{
    result = Matrix44FromScaleQuaternion(scale, q);
}


template<class T>
RW_MATH_INLINE void Matrix44FromScaleTranslation(const Vector3Template<T> &scale, const Vector3Template<T> &trans, Matrix44Template<T> &result)
{
    result = Matrix44FromScaleTranslation(scale, trans);
}


template<class T>
RW_MATH_INLINE void Matrix44Diagonal(T a, Matrix44Template<T> &result)
{
    result = Matrix44FromDiagonal<T>(a);
}


template<class T>
RW_MATH_INLINE void Matrix44RotationX(T angle, Matrix44Template<T> &result)
{
    result = Matrix44FromXRotationAngle<T>(angle);
}


template<class T>
RW_MATH_INLINE void Matrix44RotationY(T angle, Matrix44Template<T> &result)
{
    result = Matrix44FromYRotationAngle<T>(angle);
}


template<class T>
RW_MATH_INLINE void Matrix44RotationZ(T angle, Matrix44Template<T> &result)
{
    result = Matrix44FromZRotationAngle<T>(angle);
}


template<class T>
RW_MATH_INLINE void Matrix44RotateX(T angle,Matrix44Template<T> &m)
{
    m = RotateAroundXAxis<T>(m, angle);

}


template<class T>
RW_MATH_INLINE void Matrix44RotateY(T angle,Matrix44Template<T> &m)
{
     m = RotateAroundYAxis<T>(m, angle);
}


template<class T>
RW_MATH_INLINE void Matrix44RotateZ(T angle,Matrix44Template<T> &m)
{
     m = RotateAroundZAxis<T>(m, angle);
}


template<class T>
RW_MATH_INLINE void Matrix44TransposeRotation(Matrix44Template<T> &m)
{
    m = Matrix44TransposeRotationalComponent(m);
}


template<class T>
RW_MATH_INLINE void Matrix44Translate(const Vector3Template<T> &trans, Matrix44Template<T> &m)
{
    m = Matrix44Translate(m, trans);
}


template<class T>
RW_MATH_INLINE void Matrix44ScaleRow(T r0, T r1, T r2, Matrix44Template<T> &m)
{
    m = ScaleRow(m, r0, r1, r2);
}


template<class T>
RW_MATH_INLINE void Matrix44ScaleColumn(T c0, T c1, T c2, Matrix44Template<T> &m)
{
    m = ScaleColumn(m, c0, c1, c2);
}


template<class T>
RW_MATH_INLINE void Matrix44Perspective(T w, T h, T zn,T zf, Matrix44Template<T> &result)
{
    result = Matrix44FromPerspective<T>( w, h, zn, zf );
}


template<class T>
RW_MATH_INLINE void Matrix44Ortho(T w, T h, T zn,T zf, Matrix44Template<T> &result)
{
    result = Matrix44FromOrtho<T>(w, h, zn, zf);
}


template<class T>
RW_MATH_INLINE void Matrix44OrthoOffCenter(T l, T r, T b, T t, T zn, T zf, Matrix44Template<T> &result)
{
    result = Matrix44FromOrthoOffCenter<T>(l, r, b, t, zn, zf);
}


template<class T>
RW_MATH_INLINE void Matrix44FromQuaternionTranslation(const QuaternionTemplate<T> &q, const Vector3Template<T> &trans, Matrix44Template<T> &result)
{
    result = Matrix44FromQuaternionTranslation(q, trans);
}


template<class T>
RW_MATH_INLINE void Matrix44FromScaleQuaternionTranslation(const Vector3Template<T> &scale, const QuaternionTemplate<T> &q, const Vector3Template<T> &trans, Matrix44Template<T> &result)
{
    result = Matrix44FromScaleQuaternionTranslation(scale, q, trans);
}


template<class T>
RW_MATH_INLINE void Matrix44PerspectiveFov(T fov, T aspect, T zn, T zf, Matrix44Template<T> &result)
{
    result = Matrix44FromPerspectiveFov<T>(fov, aspect, zn, zf);
}


template<class T>
RW_MATH_INLINE void Matrix44LookAt(const Vector3Template<T> &eye,const Vector3Template<T> &focal,const Vector3Template<T> &up, Matrix44Template<T> &result)
{
    result = Matrix44FromLookAt(focal, up, eye);
}


template<class T>
RW_MATH_INLINE void Matrix44PerspectiveOffCenter(T l, T r, T b, T t, T zn, T zf, Matrix44Template<T> &result)
{
    result = Matrix44FromPerspectiveOffCenter<T>(l, r, b, t, zn, zf);
}


template<class T>
RW_MATH_INLINE void Mult3x3(const Vector4Template<T> &v, const Matrix44Template<T> &m, Vector4Template<T> &result)
{
    result = Mult3x3(v, m);
}


template<class T>
RW_MATH_INLINE void Mult3x3(const Vector3Template<T> &v, const Matrix44Template<T> &m, Vector3Template<T> &result)
{
    result = Mult3x3(v, m);
}


template<class T>
RW_MATH_INLINE T Matrix44Inverse3x3(const Matrix44Template<T> &m, Matrix44Template<T> &result)
{
    T determinant;
    result = Inverse3x3(m, determinant);
    return determinant;
}

#endif

#ifdef EA_PLATFORM_REVOLUTION
template<>
Vector4Template<float> Mult(const Vector4Template<float> &v, const Matrix44Template<float> &m);

template<>
Matrix44Template<float> Mult(const Matrix44Template<float> &m, const Matrix44Template<float> &b);
#endif

}}}

#endif
