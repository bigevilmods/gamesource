// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_QUATPOS_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_QUATPOS_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vec_float_operation.h"

#include "rw/math/fpu/quatpos.h"

namespace rw
{

namespace math
{

namespace vpu
{

template<class T>
RW_MATH_FORCE_INLINE QuatPos::QuatPos(const fpu::QuatPosTemplate<T> &v) 
    :quat(Quaternion(v.quat))
    ,pos(Vector3(v.pos))
{
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos() 
{
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos(const QuatPos& rhs) 
{
    quat.mV = rhs.quat.mV;
    pos.mV = rhs.pos.mV;
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos(Quaternion::InParam q, Vector3::InParam p) 
:quat(q)
,pos(p)
{
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos(VectorIntrinsicInParam q, VectorIntrinsicInParam p) 
:quat(q)
,pos(p)
{
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos(float quatX, float quatY, float quatZ, float quatW, float posX, float posY, float posZ) 
    :quat( quatX, quatY, quatZ, quatW )
    ,pos(posX, posY, posZ)
{
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos( const float xyzw[7] ) 
    :quat(RW_MATH_VPU_LoadFromAlignedFloatPointer(xyzw))
    ,pos(RW_MATH_VPU_LoadFromAlignedFloatPointer(&xyzw[4]))
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*xyzw, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos(VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam posX, VecFloat::InParam posY, VecFloat::InParam posZ) 
    :quat( quatX, quatY, quatZ, quatW )
    ,pos(posX, posY, posZ)
{
}

RW_MATH_FORCE_INLINE QuatPos::QuatPos(VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam posX, VecFloatRefIndex::InParam posY, VecFloatRefIndex::InParam posZ) 
    :quat( quatX, quatY, quatZ, quatW )
    ,pos(posX, posY, posZ)
{
}

template<VectorAxis QUAT_XSRC_INDEX, VectorAxis QUAT_YSRC_INDEX, VectorAxis QUAT_ZSRC_INDEX, VectorAxis QUAT_WSRC_INDEX, VectorAxis POS_XSRC_INDEX, VectorAxis POS_YSRC_INDEX, VectorAxis POS_ZSRC_INDEX>
RW_MATH_FORCE_INLINE QuatPos::QuatPos(VecFloatRef<QUAT_XSRC_INDEX> quatX, VecFloatRef<QUAT_YSRC_INDEX> quatY, VecFloatRef<QUAT_ZSRC_INDEX> quatZ, VecFloatRef<QUAT_WSRC_INDEX> quatW, VecFloatRef<POS_XSRC_INDEX> posX, VecFloatRef<POS_YSRC_INDEX> posY, VecFloatRef<POS_ZSRC_INDEX> posZ) 
    :quat( quatX, quatY, quatZ, quatW )
    ,pos(posX, posY, posZ)
{
}

RW_MATH_FORCE_INLINE QuatPos& QuatPos::operator = (QuatPos::InParam q) 
{
    quat.mV = q.quat.mV;
    pos.mV = q.pos.mV;
    return *this;
}

RW_MATH_FORCE_INLINE void QuatPos::Set(float quatX, float quatY, float quatZ, float quatW, float posX, float posY, float posZ) 
{
    quat.Set( quatX, quatY, quatZ, quatW );
    pos.Set( posX, posY, posZ );
}

RW_MATH_FORCE_INLINE void QuatPos::Set(VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam posX, VecFloat::InParam posY, VecFloat::InParam posZ) 
{
    quat.Set( quatX, quatY, quatZ, quatW );
    pos.Set( posX, posY, posZ );
}

RW_MATH_FORCE_INLINE void QuatPos::Set(VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam posX, VecFloatRefIndex::InParam posY, VecFloatRefIndex::InParam posZ) 
{
    quat.Set( quatX, quatY, quatZ, quatW );
    pos.Set( posX, posY, posZ );
}

template<VectorAxis QUAT_XSRC_INDEX, VectorAxis QUAT_YSRC_INDEX, VectorAxis QUAT_ZSRC_INDEX, VectorAxis QUAT_WSRC_INDEX, VectorAxis POS_XSRC_INDEX, VectorAxis POS_YSRC_INDEX, VectorAxis POS_ZSRC_INDEX>
RW_MATH_FORCE_INLINE void  QuatPos::Set(VecFloatRef<QUAT_XSRC_INDEX> quatX, VecFloatRef<QUAT_YSRC_INDEX> quatY, VecFloatRef<QUAT_ZSRC_INDEX> quatZ, VecFloatRef<QUAT_WSRC_INDEX> quatW, VecFloatRef<POS_XSRC_INDEX> posX, VecFloatRef<POS_YSRC_INDEX> posY, VecFloatRef<POS_ZSRC_INDEX> posZ) 
{
    quat.Set( quatX, quatY, quatZ, quatW );
    pos.Set( posX, posY, posZ );
}

RW_MATH_FORCE_INLINE void QuatPos::SetQuaternion(Quaternion::InParam q) 
{
    quat = q;
}

RW_MATH_FORCE_INLINE void QuatPos::SetPos(Vector3::InParam v) 
{
    pos = v;
}

RW_MATH_FORCE_INLINE void QuatPos::SetZero() 
{
    quat.SetZero();
    pos.SetZero();
}

RW_MATH_FORCE_INLINE void QuatPos::SetIdentity() 
{
    quat.SetIdentity();
    pos.SetZero();
}

RW_MATH_FORCE_INLINE Quaternion& QuatPos::GetQuaternion() 
{
    return quat;
}

RW_MATH_FORCE_INLINE const Quaternion& QuatPos::GetQuaternion() const 
{
    return quat;
}

RW_MATH_FORCE_INLINE Vector3& QuatPos::GetPos() 
{
    return pos;
}

RW_MATH_FORCE_INLINE const Vector3& QuatPos::GetPos() const 
{
    return pos;
}

}

}

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::QuatPos& t)
{
    EndianSwap(t.quat);
    EndianSwap(t.pos);
}

}

#endif
