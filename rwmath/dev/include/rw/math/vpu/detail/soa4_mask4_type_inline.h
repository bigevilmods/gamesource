// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_MASK4_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_MASK4_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask4.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"


namespace rw {
namespace math {
namespace vpu {

Soa4Mask4::Soa4Mask4()
{
}


Soa4Mask4::Soa4Mask4( const Soa4Mask4& m )
    : x( m.x ),
      y( m.y ),
      z( m.z ),
      w( m.w )
{
}


Soa4Mask4& Soa4Mask4::operator=( const Soa4Mask4& m )
{
    x = m.x;
    y = m.y;
    z = m.z;
    w = m.w;

    return *this;
}


Soa4Mask4::Soa4Mask4( Soa4MaskScalar::InParam s )
    : x( s ),
      y( s ),
      z( s ),
      w( s )
{
}


Soa4Mask4::Soa4Mask4( Soa4MaskScalar::InParam _x, Soa4MaskScalar::InParam _y, Soa4MaskScalar::InParam _z, Soa4MaskScalar::InParam _w )
    : x( _x ),
      y( _y ),
      z( _z ),
      w( _w )
{
}


Soa4Mask4::Soa4Mask4( VectorIntrinsicInParam _x, VectorIntrinsicInParam _y, VectorIntrinsicInParam _z, VectorIntrinsicInParam _w )
    : x( _x ),
      y( _y ),
      z( _z ),
      w( _w )
{
}


const Soa4MaskScalar& Soa4Mask4::X() const
{
    return x;
}


Soa4MaskScalar& Soa4Mask4::X()
{
    return x;
}


const Soa4MaskScalar& Soa4Mask4::Y() const
{
    return y;
}


Soa4MaskScalar& Soa4Mask4::Y()
{
    return y;
}


const Soa4MaskScalar& Soa4Mask4::Z() const
{
    return z;
}


Soa4MaskScalar& Soa4Mask4::Z()
{
    return z;
}


const Soa4MaskScalar& Soa4Mask4::W() const
{
    return w;
}


Soa4MaskScalar& Soa4Mask4::W()
{
    return w;
}

}}}

#endif

