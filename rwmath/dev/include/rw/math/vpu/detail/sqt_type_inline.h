// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SQT_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SQT_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vec_float_operation.h"

#include "rw/math/fpu/sqt.h"

namespace rw
{

namespace math
{

namespace vpu
{

template<class T>
RW_MATH_FORCE_INLINE Sqt::Sqt(const fpu::SqtTemplate<T> &v)
:scale(Vector3(v.scale))
,quat(Quaternion(v.quat))
,trans(Vector3(v.trans))
{
}

RW_MATH_FORCE_INLINE Sqt::Sqt()
{
}

RW_MATH_FORCE_INLINE Sqt::Sqt(const Sqt& rhs)
{
    scale.mV = rhs.scale.mV;
    quat.mV = rhs.quat.mV;
    trans.mV = rhs.trans.mV;
}

RW_MATH_FORCE_INLINE Sqt::Sqt(Vector3::InParam s, Quaternion::InParam q, Vector3::InParam t)
:scale(s)
,quat(q)
,trans(t)
{
}

RW_MATH_FORCE_INLINE Sqt::Sqt(VectorIntrinsicInParam s, VectorIntrinsicInParam q, VectorIntrinsicInParam t)
:scale(s)
,quat(q)
,trans(t)
{
}

RW_MATH_FORCE_INLINE Sqt::Sqt(float scaleX, float scaleY, float scaleZ, float quatX, float quatY, float quatZ, float quatW, float transX, float transY, float transZ)
:scale(scaleX, scaleY, scaleZ)
,quat(quatX, quatY, quatZ, quatW)
,trans(transX, transY, transZ)
{
}

RW_MATH_FORCE_INLINE Sqt::Sqt( const float xyz_xyzw_xyz[10] )
:scale(RW_MATH_VPU_LoadFromAlignedFloatPointer(xyz_xyzw_xyz))
,quat(RW_MATH_VPU_LoadFromAlignedFloatPointer(&xyz_xyzw_xyz[4]))
,trans(RW_MATH_VPU_LoadFromAlignedFloatPointer(&xyz_xyzw_xyz[8]))
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*xyz_xyzw_xyz, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Sqt::Sqt(VecFloat::InParam scaleX, VecFloat::InParam scaleY, VecFloat::InParam scaleZ, VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam transX, VecFloat::InParam transY, VecFloat::InParam transZ)
:scale(scaleX, scaleY, scaleZ)
,quat( quatX, quatY, quatZ, quatW )
,trans(transX, transY, transZ)
{
}

RW_MATH_FORCE_INLINE Sqt::Sqt(VecFloatRefIndex::InParam scaleX, VecFloatRefIndex::InParam scaleY, VecFloatRefIndex::InParam scaleZ, VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam transX, VecFloatRefIndex::InParam transY, VecFloatRefIndex::InParam transZ)
:scale(scaleX, scaleY, scaleZ)
,quat( quatX, quatY, quatZ, quatW )
,trans(transX, transY, transZ)
{
}

template<VectorAxis S_X_INDEX, VectorAxis S_Y_INDEX, VectorAxis S_Z_INDEX, VectorAxis R_X_INDEX, VectorAxis R_Y_INDEX, VectorAxis R_Z_INDEX, VectorAxis R_W_INDEX, VectorAxis T_X_INDEX, VectorAxis T_Y_INDEX, VectorAxis T_Z_INDEX>
RW_MATH_FORCE_INLINE Sqt::Sqt(VecFloatRef<S_X_INDEX> scaleX, VecFloatRef<S_Y_INDEX> scaleY, VecFloatRef<S_Z_INDEX> scaleZ, VecFloatRef<R_X_INDEX> quatX, VecFloatRef<R_Y_INDEX> quatY, VecFloatRef<R_Z_INDEX> quatZ, VecFloatRef<R_W_INDEX> quatW, VecFloatRef<T_X_INDEX> transX, VecFloatRef<T_Y_INDEX> transY, VecFloatRef<T_Z_INDEX> transZ)
:scale(scaleX, scaleY, scaleZ)
,quat( quatX, quatY, quatZ, quatW )
,trans(transX, transY, transZ)
{
}

RW_MATH_FORCE_INLINE Sqt& Sqt::operator = (Sqt::InParam s)
{
    scale.mV = s.scale.mV;
    quat.mV = s.quat.mV;
    trans.mV = s.trans.mV;
    return *this;
}

RW_MATH_FORCE_INLINE void Sqt::Set(float scaleX, float scaleY, float scaleZ, float quatX, float quatY, float quatZ, float quatW, float transX, float transY, float transZ)
{
    scale.Set( scaleX, scaleY, scaleZ );
    quat.Set( quatX, quatY, quatZ, quatW );
    trans.Set( transX, transY, transZ );
}

RW_MATH_FORCE_INLINE void Sqt::Set(VecFloat::InParam scaleX, VecFloat::InParam scaleY, VecFloat::InParam scaleZ, VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam transX, VecFloat::InParam transY, VecFloat::InParam transZ)
{
    scale.Set( scaleX, scaleY, scaleZ );
    quat.Set( quatX, quatY, quatZ, quatW );
    trans.Set( transX, transY, transZ );
}

RW_MATH_FORCE_INLINE void Sqt::Set(VecFloatRefIndex::InParam scaleX, VecFloatRefIndex::InParam scaleY, VecFloatRefIndex::InParam scaleZ, VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam transX, VecFloatRefIndex::InParam transY, VecFloatRefIndex::InParam transZ)
{
    scale.Set( scaleX, scaleY, scaleZ );
    quat.Set( quatX, quatY, quatZ, quatW );
    trans.Set( transX, transY, transZ );
}

template<VectorAxis S_X_INDEX, VectorAxis S_Y_INDEX, VectorAxis S_Z_INDEX, VectorAxis R_X_INDEX, VectorAxis R_Y_INDEX, VectorAxis R_Z_INDEX, VectorAxis R_W_INDEX, VectorAxis T_X_INDEX, VectorAxis T_Y_INDEX, VectorAxis T_Z_INDEX>
RW_MATH_FORCE_INLINE void Sqt::Set(VecFloatRef<S_X_INDEX> scaleX, VecFloatRef<S_Y_INDEX> scaleY, VecFloatRef<S_Z_INDEX> scaleZ, VecFloatRef<R_X_INDEX> quatX, VecFloatRef<R_Y_INDEX> quatY, VecFloatRef<R_Z_INDEX> quatZ, VecFloatRef<R_W_INDEX> quatW, VecFloatRef<T_X_INDEX> transX, VecFloatRef<T_Y_INDEX> transY, VecFloatRef<T_Z_INDEX> transZ)
{
    scale.Set( scaleX, scaleY, scaleZ );
    quat.Set( quatX, quatY, quatZ, quatW );
    trans.Set( transX, transY, transZ );
}

RW_MATH_FORCE_INLINE void Sqt::SetScale(Vector3::InParam v)
{
    scale = v;
}

RW_MATH_FORCE_INLINE void Sqt::SetQuaternion(Quaternion::InParam q)
{
    quat = q;
}

RW_MATH_FORCE_INLINE void Sqt::SetTranslation(Vector3::InParam v)
{
    trans = v;
}

RW_MATH_FORCE_INLINE void Sqt::SetZero()
{
    scale.SetZero();
    quat.SetZero();
    trans.SetZero();
}

RW_MATH_FORCE_INLINE void Sqt::SetIdentity()
{
    scale = GetVector3_One();
    quat.SetIdentity();
    trans.SetZero();
}

RW_MATH_FORCE_INLINE Vector3& Sqt::GetScale()
{
    return scale;
}

RW_MATH_FORCE_INLINE const Vector3& Sqt::GetScale() const
{
    return scale;
}

RW_MATH_FORCE_INLINE Quaternion& Sqt::GetQuaternion()
{
    return quat;
}

RW_MATH_FORCE_INLINE const Quaternion& Sqt::GetQuaternion() const
{
    return quat;
}

RW_MATH_FORCE_INLINE Vector3& Sqt::GetTranslation()
{
    return trans;
}

RW_MATH_FORCE_INLINE const Vector3& Sqt::GetTranslation() const
{
    return trans;
}

}

}

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Sqt& t)
{
    EndianSwap(t.scale);
    EndianSwap(t.quat);
    EndianSwap(t.trans);
}

}

#endif
