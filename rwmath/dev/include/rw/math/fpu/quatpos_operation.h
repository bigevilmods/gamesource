// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_QUADPOS_OPERATION_INLINE_H
#define RW_MATH_FPU_QUADPOS_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/quatpos.h"
#include "rw/math/fpu/matrix44.h"
#include "rw/math/fpu/matrix44affine.h"

#include "rw/math/fpu/quaternion_operation.h"
#include "rw/math/fpu/vector3_operation.h"
#include "rw/math/fpu/matrix44_operation.h"
#include "rw/math/fpu/matrix44affine_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{

template <class T> RW_MATH_INLINE bool operator ==(const QuatPosTemplate<T>& a,        const QuatPosTemplate<T>& b)
{
    return a.quat == b.quat &&
        a.pos == b.pos;
}

template <class T> RW_MATH_INLINE bool operator !=(const QuatPosTemplate<T>& a,        const QuatPosTemplate<T>& b)
{
    return !(a==b);
}

template <class T> RW_MATH_INLINE bool   IsSimilar(const QuatPosTemplate<T>& a,        const QuatPosTemplate<T>& b,      T epsilon = SMALL_FLOAT)
{
    return IsSimilar(a.quat, b.quat, epsilon) &&
        IsSimilar(a.pos, b.pos, epsilon);
}

template <class T> RW_MATH_INLINE bool   IsValid(const QuatPosTemplate<T>& q)
{
    return IsValid(q.quat) && IsValid(q.pos);
}

template <class T> RW_MATH_INLINE QuatPosTemplate<T> Mult(const QuatPosTemplate<T>& a, const QuatPosTemplate<T>& b)
{
    return QuatPosTemplate<T>(
        Mult( a.quat, b.quat ),
        TransformPoint( b.pos, a )
        );
}

template <class T> RW_MATH_INLINE QuatPosTemplate<T>   QuatPosFromMatrix44(const Matrix44Template<T>& mat)
{
    return QuatPosTemplate<T>(
        QuaternionFromMatrix44(mat),
        GetXYZ(mat.WAxis())
        );
}

template <class T> RW_MATH_INLINE Matrix44Template<T>   Matrix44FromQuatPos(const QuatPosTemplate<T>& qp)
{
    return Matrix44FromQuaternionTranslation( qp.quat, qp.pos );
}

template <class T> RW_MATH_INLINE QuatPosTemplate<T> QuatPosFromMatrix44Affine(const Matrix44AffineTemplate<T>& mat)
{
    return QuatPosTemplate<T>(
        QuaternionFromMatrix44Affine(mat),
        mat.WAxis()
        );
}

template <class T> RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromQuatPos(const QuatPosTemplate<T> &qp)
{
    return Matrix44AffineTemplate<T>(qp.quat, qp.pos);
}

template <class T> RW_MATH_INLINE Vector3Template<T>   TransformPoint(const Vector3Template<T>& point, const QuatPosTemplate<T>& q)
{
    return TransformVector(point, q.quat) + q.pos;
}

template <class T>
RW_MATH_INLINE Vector3Template<T> QuatPosGetMatrix44AffineRowX(QuatPosTemplate<T>& quat)
{
    return QuaternionGetMatrix44AffineRowX(quat.quat);
}

template <class T>
RW_MATH_INLINE Vector3Template<T> QuatPosGetMatrix44AffineRowY(QuatPosTemplate<T>& quat)
{
    return QuaternionGetMatrix44AffineRowY(quat.quat);
}

template <class T>
RW_MATH_INLINE Vector3Template<T> QuatPosGetMatrix44AffineRowZ(QuatPosTemplate<T>& quat)
{
    return QuaternionGetMatrix44AffineRowZ(quat.quat);
}

template <class T> RW_MATH_INLINE QuatPosTemplate<T> QuatPosFromAxisRotationAngle(const Vector3Template<T>& axis, T lrAngleInRadians)
{
    return QuatPosTemplate<T> (
        QuaternionFromAxisRotationAngle(axis, lrAngleInRadians),
        Vector3Template<T>( 0.0f, 0.0f, 0.0f )
        );
}

template <class T>
RW_MATH_FORCE_INLINE QuatPosTemplate<T> Select(const MaskScalarTemplate<T> &mask, const QuatPosTemplate<T> &trueValue, const QuatPosTemplate<T> &falseValue )
{
    return QuatPosTemplate<T>(
            Select( mask, trueValue.quat, falseValue.quat ),
            Select( mask, trueValue.pos, falseValue.pos )
        );
}

template <class T> RW_MATH_INLINE QuatPosTemplate<T> Inverse(const QuatPosTemplate<T>& in)
{
    QuaternionTemplate<T> quat = Inverse(in.quat);
    Vector3Template<T> pos = TransformVector(in.pos, quat);
    pos = Negate(pos);
    return QuatPosTemplate<T>(quat, pos);
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif
