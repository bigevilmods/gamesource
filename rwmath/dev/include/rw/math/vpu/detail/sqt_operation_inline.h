// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SQT_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_SQT_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/vector3_operation.h"
#include "rw/math/vpu/quaternion_operation.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/matrix44affine_operation.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44_operation.h"
#include "rw/math/vpu/quatpos.h"

namespace rw
{
namespace math
{
namespace vpu
{

    RW_MATH_FORCE_INLINE bool operator == (Sqt::InParam a, Sqt::InParam b)
    {
        return a.quat == b.quat &&
            a.scale == b.scale &&  
            a.trans == b.trans;
    }

    RW_MATH_FORCE_INLINE bool operator !=(Sqt::InParam a, Sqt::InParam b)
    {
        return !(a==b);
    }

    RW_MATH_FORCE_INLINE Sqt& operator *=(Sqt& s, Sqt::InParam b)
    {
        s = Mult(s, b);
        return s;
    }

    RW_MATH_FORCE_INLINE Sqt& operator *=(Sqt& s, QuatPos::InParam qp)
    {
        s = Mult(s, qp);
        return s;
    }

    RW_MATH_FORCE_INLINE Sqt operator * (Sqt::InParam s, Sqt::InParam b)
    {
        return Mult(s, b);
    }

    RW_MATH_FORCE_INLINE Sqt operator * (Sqt::InParam s, QuatPos::InParam qp)
    {
        return Mult(s, qp);
    }

    RW_MATH_FORCE_INLINE bool IsSimilar(Sqt::InParam a, Sqt::InParam b, VecFloat::InParam epsilon)
    {
        return IsSimilar(a.scale, b.scale, epsilon) && IsSimilar(a.quat, b.quat, epsilon) && IsSimilar(a.trans, b.trans, epsilon);
    }

    RW_MATH_FORCE_INLINE bool IsValid(Sqt::InParam s)
    {
        return IsValid(s.scale) && IsValid(s.quat) && IsValid(s.trans);
    }

    RW_MATH_FORCE_INLINE Sqt SqtFromMatrix44(Matrix44::InParam m)
    {
        Sqt r;
        Matrix44 tm = Transpose(m);
        tm.xAxis *= tm.xAxis;
        tm.yAxis *= tm.yAxis;
        tm.zAxis *= tm.zAxis;
        tm.wAxis *= tm.wAxis;
        r.scale = tm.xAxis + tm.yAxis + tm.zAxis + tm.wAxis;
        RW_MATH_VPU_SquareRoot(r.scale.mV, r.scale.mV);

        VectorIntrinsic recipScale, splatX, splatY, splatZ;;
        RW_MATH_VPU_Reciprocal(recipScale, r.scale.mV);
        RW_MATH_VPU_Splat_I( splatX, recipScale, 0 );
        RW_MATH_VPU_Splat_I( splatY, recipScale, 1 );
        RW_MATH_VPU_Splat_I( splatZ, recipScale, 2 );
        Matrix44 sm;
        sm.xAxis = Mult(m.xAxis, splatX);
        sm.yAxis = Mult(m.yAxis, splatY);
        sm.zAxis = Mult(m.zAxis, splatZ);

        r.quat = QuaternionFromMatrix44(sm);
        r.trans = m.GetRow(3); 
        return r;
    }

    RW_MATH_FORCE_INLINE Sqt SqtFromOrthonormalMatrix44(Matrix44::InParam m)
    {
        Sqt r;
        r.scale = GetVector3_One();
        r.quat = QuaternionFromMatrix44(m);
        r.trans = m.GetRow(3); 
        return r;
    }

    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromSqt(Sqt::InParam s)
    {
        return Matrix44FromScaleQuaternionTranslation(s.scale, s.quat, s.trans);
    }

    RW_MATH_FORCE_INLINE Sqt SqtFromMatrix44Affine(Matrix44Affine::InParam ma)
    {
        Sqt r;
        Matrix44Affine tm = Transpose(ma);
        tm.xAxis *= tm.xAxis;
        tm.yAxis *= tm.yAxis;
        tm.zAxis *= tm.zAxis;
        tm.wAxis *= tm.wAxis;
        r.scale = tm.xAxis + tm.yAxis + tm.zAxis + tm.wAxis;
        RW_MATH_VPU_SquareRoot(r.scale.mV, r.scale.mV);

        VectorIntrinsic recipScale, splatX, splatY, splatZ;;
        RW_MATH_VPU_Reciprocal(recipScale, r.scale.mV);
        RW_MATH_VPU_Splat_I( splatX, recipScale, 0 );
        RW_MATH_VPU_Splat_I( splatY, recipScale, 1 );
        RW_MATH_VPU_Splat_I( splatZ, recipScale, 2 );
        Matrix44Affine sma;
        sma.xAxis = Mult(ma.xAxis, splatX);
        sma.yAxis = Mult(ma.yAxis, splatY);
        sma.zAxis = Mult(ma.zAxis, splatZ);

        r.quat = QuaternionFromMatrix44Affine(sma);
        r.trans = ma.GetRow(3); 
        return r;
    }

    RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromSqt(Sqt::InParam s)
    {
        return Matrix44AffineFromScaleQuaternionTranslation(s.scale, s.quat, s.trans);
    }

    RW_MATH_FORCE_INLINE Sqt Mult(Sqt::InParam s0, Sqt::InParam s1)
    {
        Sqt r;
        r.scale = s1.scale * s0.scale;
        r.quat = s1.quat * s0.quat;
        r.trans = s1.trans + TransformVector(s0.trans * s1.scale, s1.quat);
        return r;
    }

    RW_MATH_FORCE_INLINE Sqt Mult(Sqt::InParam s, QuatPos::InParam qp)
    {
        Sqt r;
        r.scale = s.scale;
        r.quat = s.quat * qp.quat;
        r.trans = s.trans + TransformVector(qp.pos * s.scale, s.quat);
        return r;
    }

    RW_MATH_FORCE_INLINE Sqt SLerp(Sqt::InParam s0, Sqt::InParam s1, VecFloat::InParam t)
    {
        Sqt r;
        r.scale = Lerp(s0.scale, s1.scale, t);
        r.quat  = SLerp(s0.quat, s1.quat, t);
        r.trans = Lerp(s0.trans, s1.trans, t);
        return r;
    }

    RW_MATH_FORCE_INLINE Sqt Lerp(Sqt::InParam s0, Sqt::InParam s1, VecFloat::InParam t)
    {
        Sqt r;
        r.scale = Lerp(s0.scale, s1.scale, t);
        r.quat  = Lerp(s0.quat, s1.quat, t);
        r.trans = Lerp(s0.trans, s1.trans, t);
        return r;
    }

    RW_MATH_FORCE_INLINE Sqt Select(MaskScalar::InParam mask, Sqt::InParam trueValue, Sqt::InParam falseValue )
    {
        VectorIntrinsic scale, quat, trans;
        RW_MATH_VPU_Select( scale, falseValue.scale.mV, trueValue.scale.mV, mask.mV );
        RW_MATH_VPU_Select( quat, falseValue.quat.mV, trueValue.quat.mV, mask.mV );
        RW_MATH_VPU_Select( trans, falseValue.trans.mV, trueValue.trans.mV, mask.mV );
        return Sqt(scale, quat, trans);
    }

    RW_MATH_FORCE_INLINE Sqt Inverse(Sqt::InParam in)
    {
        Sqt r;
        r.scale = Reciprocal(in.scale);
        r.quat  = Conj(in.quat);
        r.trans = -TransformVector(in.trans * r.scale, r.quat);
        return r;
    }

} // namespace vpu
} // namespace math
} // namespace rw

#endif
