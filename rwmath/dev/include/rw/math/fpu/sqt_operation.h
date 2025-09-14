// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_SQT_OPERATION_INLINE_H
#define RW_MATH_FPU_SQT_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/sqt.h"
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

template <class T> RW_MATH_INLINE bool operator ==(const SqtTemplate<T>& a,        const SqtTemplate<T>& b)
{
    return a.scale == b.scale && a.quat == b.quat && a.trans == b.trans;
}

template <class T> RW_MATH_INLINE bool operator !=(const SqtTemplate<T>& a,        const SqtTemplate<T>& b)
{
    return !(a==b);
}

template <class T> RW_MATH_INLINE SqtTemplate<T>& operator *=(SqtTemplate<T>& s, const SqtTemplate<T>& b)
{
    s = Mult(s, b);
    return s;
}

template <class T> RW_MATH_INLINE SqtTemplate<T>& operator *=(SqtTemplate<T>& s, const QuatPosTemplate<T>& qp)
{
    s = Mult(s, qp);
    return s;
}
template <class T> RW_MATH_INLINE SqtTemplate<T> operator * (const SqtTemplate<T>& s, const SqtTemplate<T>& b)
{
    return Mult(s, b);
}

template <class T> RW_MATH_INLINE SqtTemplate<T> operator * (const SqtTemplate<T>& s, const QuatPosTemplate<T>& qp)
{
    return Mult(s, qp);
}

template <class T> RW_MATH_INLINE bool   IsSimilar(const SqtTemplate<T>& a,        const SqtTemplate<T>& b,      T epsilon = SMALL_FLOAT)
{
    return IsSimilar(a.scale, b.scale, epsilon) && IsSimilar(a.quat, b.quat, epsilon) && IsSimilar(a.trans, b.trans, epsilon);
}

template <class T> RW_MATH_INLINE bool   IsValid(const SqtTemplate<T>& s)
{
    return IsValid(s.scale) && IsValid(s.quat) && IsValid(s.trans);
}

template <class T> RW_MATH_INLINE SqtTemplate<T> Mult(const SqtTemplate<T>& s0, const SqtTemplate<T>& s1)
{
    SqtTemplate<T> r;
    r.scale = s1.scale * s0.scale;
    r.quat = s1.quat * s0.quat;
    r.trans = s1.trans + TransformVector(s0.trans * s1.scale, s1.quat);
    return r;
}

template <class T> RW_MATH_INLINE SqtTemplate<T> Mult(const SqtTemplate<T>& s, const QuatPosTemplate<T>& qp)
{
    SqtTemplate<T> r;
    r.scale = s.scale;
    r.quat = s.quat * qp.quat;
    r.trans = s.trans + TransformVector(qp.pos * s.scale, s.quat);
    return r;
}

template <class T> RW_MATH_INLINE void SelfMult(SqtTemplate<T>& s, const QuatPosTemplate<T>& qp)
{
    s = Mult(s, qp);
}


template <class T> RW_MATH_INLINE SqtTemplate<T> SqtFromMatrix44(const Matrix44Template<T>& m)
{
    SqtTemplate<T> r;
    r.scale.x = Magnitude(m.xAxis);
    r.scale.y = Magnitude(m.yAxis);
    r.scale.z = Magnitude(m.zAxis);
    Matrix44Template<T> sm;
    sm.xAxis = m.xAxis / r.scale.x;
    sm.yAxis = m.yAxis / r.scale.y;
    sm.zAxis = m.zAxis / r.scale.z;
    r.quat = QuaternionFromMatrix44(sm);
    r.trans.x = m.wAxis.x;
    r.trans.y = m.wAxis.y;
    r.trans.z = m.wAxis.z;
    return r;
}

template <class T> RW_MATH_INLINE SqtTemplate<T> SqtFromOrthonormalMatrix44(const Matrix44Template<T>& m)
{
    SqtTemplate<T> r;
    r.scale = GetVector3_One();
    r.quat = QuaternionFromMatrix44(m);
    r.trans.x = m.wAxis.x;
    r.trans.y = m.wAxis.y;
    r.trans.z = m.wAxis.z; 
    return r;
}

template <class T> RW_MATH_INLINE Matrix44Template<T>   Matrix44FromSqt(const SqtTemplate<T>& s)
{
    return Matrix44FromScaleQuaternionTranslation(s.scale, s.quat, s.trans);
}

template <class T> RW_MATH_INLINE SqtTemplate<T> SqtFromMatrix44Affine(const Matrix44AffineTemplate<T>& ma)
{
    SqtTemplate<T> r;
    r.scale.x = Magnitude(ma.xAxis);
    r.scale.y = Magnitude(ma.yAxis);
    r.scale.z = Magnitude(ma.zAxis);
    Matrix44AffineTemplate<T> sma;
    sma.xAxis = ma.xAxis / r.scale.x;
    sma.yAxis = ma.yAxis / r.scale.y;
    sma.zAxis = ma.zAxis / r.scale.z;
    r.quat = QuaternionFromMatrix44Affine(sma);
    r.trans = ma.wAxis; 
    return r;
}

template <class T> RW_MATH_INLINE Matrix44AffineTemplate<T> Matrix44AffineFromSqt(const SqtTemplate<T> & s)
{
    return Matrix44AffineFromScaleQuaternionTranslation(s.scale, s.quat, s.trans);
}

template <class T> RW_MATH_INLINE SqtTemplate<T> SLerp(const SqtTemplate<T>& s0, const SqtTemplate<T>& s1, T t)
{
    SqtTemplate<T> r;
    r.scale = Lerp(s0.scale, s1.scale, t);
    r.quat  = SLerp(s0.quat, s1.quat, t);
    r.trans = Lerp(s0.trans, s1.trans, t);
    return r;
}

template <class T> RW_MATH_INLINE SqtTemplate<T> Lerp(const SqtTemplate<T>& s0, const SqtTemplate<T>& s1, T t)
{
    SqtTemplate<T> r;
    r.scale = Lerp(s0.scale, s1.scale, t);
    r.quat  = Lerp(s0.quat, s1.quat, t);
    r.trans = Lerp(s0.trans, s1.trans, t);
    return r;
}

template <class T>
RW_MATH_FORCE_INLINE SqtTemplate<T> Select(const MaskScalarTemplate<T> &mask, const SqtTemplate<T> &trueValue, const SqtTemplate<T> &falseValue )
{
    return SqtTemplate<T>(
            Select( mask, trueValue.scale, falseValue.scale),
            Select( mask, trueValue.quat, falseValue.quat ),
            Select( mask, trueValue.trans, falseValue.trans )
        );
}

template <class T> RW_MATH_INLINE SqtTemplate<T> Inverse(const SqtTemplate<T>& in)
{
        SqtTemplate<T> r;
        r.scale = Reciprocal(in.scale);
        r.quat  = Conj(in.quat);
        r.trans = -TransformVector(in.trans * r.scale, r.quat);
        return r;
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif
