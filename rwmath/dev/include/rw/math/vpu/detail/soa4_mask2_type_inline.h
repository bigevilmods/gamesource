// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_MASK2_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_MASK2_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask2.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"


namespace rw {
namespace math {
namespace vpu {

Soa4Mask2::Soa4Mask2()
{
}


Soa4Mask2::Soa4Mask2( const Soa4Mask2& m )
    : x( m.x ),
      y( m.y )
{
}


Soa4Mask2& Soa4Mask2::operator=( const Soa4Mask2& m )
{
    x = m.x;
    y = m.y;
    return *this;
}


Soa4Mask2::Soa4Mask2( Soa4MaskScalar::InParam s )
    : x( s ),
      y( s )
{
}


Soa4Mask2::Soa4Mask2( Soa4MaskScalar::InParam _x, Soa4MaskScalar::InParam _y )
    : x( _x ),
      y( _y )
{
}


Soa4Mask2::Soa4Mask2( VectorIntrinsicInParam _x, VectorIntrinsicInParam _y )
    : x( _x ),
      y( _y )
{
}


const Soa4MaskScalar& Soa4Mask2::X() const
{
    return x;
}


Soa4MaskScalar& Soa4Mask2::X()
{
    return x;
}


const Soa4MaskScalar& Soa4Mask2::Y() const
{
    return y;
}


Soa4MaskScalar& Soa4Mask2::Y()
{
    return y;
}

}}}

#endif

