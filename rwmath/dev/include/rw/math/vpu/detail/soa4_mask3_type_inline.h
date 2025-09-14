// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_MASK3_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_MASK3_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask3.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"


namespace rw {
namespace math {
namespace vpu {

Soa4Mask3::Soa4Mask3()
{
}


Soa4Mask3::Soa4Mask3( const Soa4Mask3& m )
    : x( m.x ),
      y( m.y ),
      z( m.z )
{
}


Soa4Mask3& Soa4Mask3::operator=( const Soa4Mask3& m )
{
    x = m.x;
    y = m.y;
    z = m.z;

    return *this;
}


Soa4Mask3::Soa4Mask3( Soa4MaskScalar::InParam s )
    : x( s ),
      y( s ),
      z( s )
{
}


Soa4Mask3::Soa4Mask3( Soa4MaskScalar::InParam _x, Soa4MaskScalar::InParam _y, Soa4MaskScalar::InParam _z )
    : x( _x ),
      y( _y ),
      z( _z )
{
}


Soa4Mask3::Soa4Mask3( VectorIntrinsicInParam _x, VectorIntrinsicInParam _y, VectorIntrinsicInParam _z )
    : x( _x ),
      y( _y ),
      z( _z )
{
}


const Soa4MaskScalar& Soa4Mask3::X() const
{
    return x;
}


Soa4MaskScalar& Soa4Mask3::X()
{
    return x;
}


const Soa4MaskScalar& Soa4Mask3::Y() const
{
    return y;
}


Soa4MaskScalar& Soa4Mask3::Y()
{
    return y;
}

const Soa4MaskScalar& Soa4Mask3::Z() const
{
    return z;
}


Soa4MaskScalar& Soa4Mask3::Z()
{
    return z;
}

}}}

#endif

