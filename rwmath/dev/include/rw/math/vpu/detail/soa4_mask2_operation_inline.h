// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_MASK2_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_MASK2_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask2_operation.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Soa4Mask2 Not(Soa4Mask2::InParam a)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Not( x, a.x.mV );
    RW_MATH_VPU_Not( y, a.y.mV );
    
    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Mask2 And(Soa4Mask2::InParam a, Soa4Mask2::InParam b)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_And( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_And( y, a.y.mV, b.y.mV );

    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Mask2 Or(Soa4Mask2::InParam a, Soa4Mask2::InParam b)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Or( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Or( y, a.y.mV, b.y.mV );

    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Mask2 Xor(Soa4Mask2::InParam a, Soa4Mask2::InParam b)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Xor( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Xor( y, a.y.mV, b.y.mV );

    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompEqual(Soa4Mask2::InParam a, Soa4Mask2::InParam b)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Xor( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Not( x, x );
    RW_MATH_VPU_Xor( y, a.y.mV, b.y.mV );
    RW_MATH_VPU_Not( y, y );

    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Mask2 CompNotEqual(Soa4Mask2::InParam a, Soa4Mask2::InParam b)
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Xor( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Xor( y, a.y.mV, b.y.mV );

    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4MaskScalar CompAnyTrue(Soa4Mask2::InParam a)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Or( ret, a.x.mV, a.y.mV );
    return Soa4MaskScalar( ret );
}


RW_MATH_FORCE_INLINE Soa4MaskScalar CompAllTrue(Soa4Mask2::InParam a)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_And( ret, a.x.mV, a.y.mV );
    return Soa4MaskScalar( ret );
}


RW_MATH_FORCE_INLINE Soa4Mask2 Select(Soa4MaskScalar::InParam mask, Soa4Mask2::InParam trueValue, Soa4Mask2::InParam falseValue )
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.mV );

    return Soa4Mask2( x, y );
}


RW_MATH_FORCE_INLINE Soa4Mask2 Select(Soa4Mask2::InParam mask, Soa4Mask2::InParam trueValue, Soa4Mask2::InParam falseValue )
{
    VectorIntrinsic x, y;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.x.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.y.mV );

    return Soa4Mask2( x, y );
}



} // namespace vpu
} // namespace math
} // namespace rw

#endif

