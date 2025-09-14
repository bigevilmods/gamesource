// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_VECTOR3_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_VECTOR3_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Soa4Vector3::Soa4Vector3()
{
}


RW_MATH_FORCE_INLINE Soa4Vector3::Soa4Vector3(const Soa4Vector3& rhs)
    : x( rhs.x ),
      y( rhs.y ),
      z( rhs.z )
{
}


RW_MATH_FORCE_INLINE Soa4Vector3::Soa4Vector3(Soa4Float::InParam scalar)
    : x( scalar ),
      y( scalar ),
      z( scalar )
{
}


RW_MATH_FORCE_INLINE Soa4Vector3::Soa4Vector3(VectorIntrinsicInParam _x, VectorIntrinsicInParam _y, VectorIntrinsicInParam _z)
    : x( _x ),
      y( _y ),
      z( _z )
{
}


RW_MATH_FORCE_INLINE Soa4Vector3::Soa4Vector3(Soa4Float::InParam _x, Soa4Float::InParam _y, Soa4Float::InParam _z)
    : x( _x ),
      y( _y ),
      z( _z )
{
}


RW_MATH_FORCE_INLINE Soa4Vector3& Soa4Vector3::operator = (Soa4Vector3::InParam v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector3::X() const
{
    return x;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector3::X()
{
    return x;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector3::Y() const
{
    return y;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector3::Y()
{
    return y;
}

RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector3::Z() const
{
    return z;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector3::Z()
{
    return z;
}

}}}

#endif

