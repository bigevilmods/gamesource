// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_VECTOR2_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_VECTOR2_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Soa4Vector2::Soa4Vector2()
{
}


RW_MATH_FORCE_INLINE Soa4Vector2::Soa4Vector2(const Soa4Vector2& rhs)
    : x( rhs.x ),
      y( rhs.y )
{
}


RW_MATH_FORCE_INLINE Soa4Vector2::Soa4Vector2(Soa4Float::InParam scalar)
    : x( scalar ),
      y( scalar )
{
}


RW_MATH_FORCE_INLINE Soa4Vector2::Soa4Vector2(VectorIntrinsicInParam _x, VectorIntrinsicInParam _y)
    : x( _x ),
      y( _y )
{
}


RW_MATH_FORCE_INLINE Soa4Vector2::Soa4Vector2(Soa4Float::InParam _x, Soa4Float::InParam _y)
    : x( _x ),
      y( _y )
{
}


RW_MATH_FORCE_INLINE Soa4Vector2& Soa4Vector2::operator = (Soa4Vector2::InParam v)
{
    x = v.x;
    y = v.y;
    return *this;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector2::X() const
{
    return x;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector2::X()
{
    return x;
}


RW_MATH_FORCE_INLINE const Soa4Float& Soa4Vector2::Y() const
{
    return y;
}


RW_MATH_FORCE_INLINE Soa4Float& Soa4Vector2::Y()
{
    return y;
}


}}}

#endif

