// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_VECTOR4_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_VECTOR4_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Soa4Vector4::Soa4Vector4()
{
}


RW_MATH_FORCE_INLINE Soa4Vector4::Soa4Vector4(const Soa4Vector4& rhs)
    : x( rhs.x ),
      y( rhs.y ),
      z( rhs.z ),
      w( rhs.w )
{
}


RW_MATH_FORCE_INLINE Soa4Vector4::Soa4Vector4(Soa4Float::InParam scalar)
    : x( scalar ),
      y( scalar ),
      z( scalar ),
      w( scalar )
{
}


RW_MATH_FORCE_INLINE Soa4Vector4::Soa4Vector4(VectorIntrinsicInParam _x, 
        VectorIntrinsicInParam _y, 
        VectorIntrinsicInParam _z, 
        VectorIntrinsicInParam _w)
    : x( _x ),
      y( _y ),
      z( _z ),
      w( _w )
{
}


RW_MATH_FORCE_INLINE Soa4Vector4::Soa4Vector4(Soa4Float::InParam _x, 
        Soa4Float::InParam _y, 
        Soa4Float::InParam _z, 
        Soa4Float::InParam _w)
    : x( _x ),
      y( _y ),
      z( _z ),
      w( _w )
{
}


RW_MATH_FORCE_INLINE Soa4Vector4& Soa4Vector4::operator = (Soa4Vector4::InParam v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;

    return *this;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector4::X() const
{
    return x;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector4::X()
{
    return x;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector4::Y() const
{
    return y;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector4::Y()
{
    return y;
}

RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector4::Z() const
{
    return z;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector4::Z()
{
    return z;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector4::W() const
{
    return w;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector4::W()
{
    return w;
}


}}}

#endif

