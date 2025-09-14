// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_QUATPOS_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_QUATPOS_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/quaternion_operation.h"
#include "rw/math/vpu/matrix44_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE bool operator == (QuatPos::InParam a, QuatPos::InParam b)
{
    return a.quat == b.quat &&
           a.pos == b.pos;
}

RW_MATH_FORCE_INLINE bool operator != (QuatPos::InParam a, QuatPos::InParam b)
{
    return !(a==b);
}


RW_MATH_FORCE_INLINE bool IsSimilar(QuatPos::InParam a, QuatPos::InParam b, VecFloat::InParam epsilon)
{
    return IsSimilar(a.quat, b.quat, epsilon) &&
           IsSimilar(a.pos, b.pos, epsilon);
}

RW_MATH_FORCE_INLINE bool IsValid(QuatPos::InParam q)
{
    return IsValid(q.quat) && IsValid(q.pos);
}

RW_MATH_FORCE_INLINE Vector3 TransformPoint(Vector3::InParam point, QuatPos::InParam q)
{
    return TransformVector(point, q.quat) + q.pos;
}

RW_MATH_FORCE_INLINE QuatPos Mult(QuatPos::InParam a, QuatPos::InParam b)
{
    return QuatPos(
        Mult( a.quat, b.quat ),
        TransformPoint( b.pos, a )
    );
}

RW_MATH_INLINE QuatPos QuatPosFromMatrix44(Matrix44::InParam mat)
{
    return QuatPos(
        QuaternionFromMatrix44(mat),
        GetXYZ(mat.wAxis)
    );              
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromQuatPos(QuatPos::InParam qp)
{
    return Matrix44FromQuaternionTranslation( qp.quat, qp.pos );
}

RW_MATH_INLINE QuatPos QuatPosFromMatrix44Affine(Matrix44Affine::InParam mat)
{
    return QuatPos(
        QuaternionFromMatrix44Affine(mat),
        mat.wAxis
    );              
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromQuatPos(QuatPos::InParam qp)
{
    return Matrix44Affine(qp.quat, qp.pos);
}

RW_MATH_FORCE_INLINE Vector3 QuatPosGetMatrix44AffineRowX(QuatPos::InParam quat)
{
    return QuaternionGetMatrix44AffineRowX(quat.quat);
}

RW_MATH_FORCE_INLINE Vector3 QuatPosGetMatrix44AffineRowY(QuatPos::InParam quat)
{
    return QuaternionGetMatrix44AffineRowY(quat.quat);
}

RW_MATH_FORCE_INLINE Vector3 QuatPosGetMatrix44AffineRowZ(QuatPos::InParam quat)
{
    return QuaternionGetMatrix44AffineRowZ(quat.quat);
}

RW_MATH_FORCE_INLINE QuatPos QuatPosFromAxisRotationAngle(Vector3::InParam axis, VecFloat::InParam lrAngleInRadians)
{
    return QuatPos(
        QuaternionFromAxisRotationAngle(axis, lrAngleInRadians),
        GetVector3_Zero()
    );
}

RW_MATH_FORCE_INLINE QuatPos Select(MaskScalar::InParam mask, QuatPos::InParam trueValue, QuatPos::InParam falseValue )
{
    VectorIntrinsic quat, pos;
    RW_MATH_VPU_Select( quat, falseValue.quat.mV, trueValue.quat.mV, mask.mV );
    RW_MATH_VPU_Select( pos, falseValue.pos.mV, trueValue.pos.mV, mask.mV );
    return QuatPos(quat, pos);
}

RW_MATH_FORCE_INLINE QuatPos Inverse(QuatPos::InParam in)
{
    VectorIntrinsic quat, pos, s, v, mask;
    VectorIntrinsic two, q_cross_v, qW;
    RW_MATH_VPU_GetTwo(two);
    
    // Inverse the original QuatPos's quaternion
    RW_MATH_VPU_DotV4(s, in.quat.mV, in.quat.mV);
    RW_MATH_VPU_Reciprocal(s, s);
    RW_MATH_VPU_GetSignMaskXYZ(mask);
    RW_MATH_VPU_Xor(v, in.quat.mV, mask);
    RW_MATH_VPU_Mult(quat, v, s);

    // Transform the original QuatPos's position
    RW_MATH_VPU_Splat_I(qW, quat, VectorAxisW);
    RW_MATH_VPU_CrossV3(q_cross_v, quat, in.pos.mV);
    RW_MATH_VPU_MultAdd(pos, qW, in.pos.mV, q_cross_v);
    RW_MATH_VPU_CrossV3(pos, quat, pos);
    RW_MATH_VPU_MultAdd(pos, two, pos, in.pos.mV);

    // Negate the original QuatPos's position
    RW_MATH_VPU_Negate(pos, pos);

    return QuatPos(quat, pos);
}



} // namespace vpu
} // namespace math
} // namespace rw

#endif
