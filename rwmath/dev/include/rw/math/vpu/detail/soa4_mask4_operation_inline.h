// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SOA4_MASK4_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_SOA4_MASK4_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_mask4_operation.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Soa4Mask4 Not(Soa4Mask4::InParam a)
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Not( x, a.x.mV );
    RW_MATH_VPU_Not( y, a.y.mV );
    RW_MATH_VPU_Not( z, a.z.mV );
    RW_MATH_VPU_Not( w, a.w.mV );
    
    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4Mask4 And(Soa4Mask4::InParam a, Soa4Mask4::InParam b)
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_And( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_And( y, a.y.mV, b.y.mV );
    RW_MATH_VPU_And( z, a.z.mV, b.z.mV );
    RW_MATH_VPU_And( w, a.w.mV, b.w.mV );

    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4Mask4 Or(Soa4Mask4::InParam a, Soa4Mask4::InParam b)
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Or( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Or( y, a.y.mV, b.y.mV );
    RW_MATH_VPU_Or( z, a.z.mV, b.z.mV );
    RW_MATH_VPU_Or( w, a.w.mV, b.w.mV );

    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4Mask4 Xor(Soa4Mask4::InParam a, Soa4Mask4::InParam b)
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Xor( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Xor( y, a.y.mV, b.y.mV );
    RW_MATH_VPU_Xor( z, a.z.mV, b.z.mV );
    RW_MATH_VPU_Xor( w, a.w.mV, b.w.mV );

    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4Mask4 CompEqual(Soa4Mask4::InParam a, Soa4Mask4::InParam b)
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Xor( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Not( x, x );
    RW_MATH_VPU_Xor( y, a.y.mV, b.y.mV );
    RW_MATH_VPU_Not( y, y );
    RW_MATH_VPU_Xor( z, a.z.mV, b.z.mV );
    RW_MATH_VPU_Not( z, z );
    RW_MATH_VPU_Xor( w, a.w.mV, b.w.mV );
    RW_MATH_VPU_Not( w, w );

    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4Mask4 CompNotEqual(Soa4Mask4::InParam a, Soa4Mask4::InParam b)
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Xor( x, a.x.mV, b.x.mV );
    RW_MATH_VPU_Xor( y, a.y.mV, b.y.mV );
    RW_MATH_VPU_Xor( z, a.z.mV, b.z.mV );
    RW_MATH_VPU_Xor( w, a.w.mV, b.w.mV );

    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4MaskScalar CompAnyTrue(Soa4Mask4::InParam a)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Or( ret, a.x.mV, a.y.mV );
    RW_MATH_VPU_Or( ret, ret, a.z.mV );
    RW_MATH_VPU_Or( ret, ret, a.w.mV );

    return Soa4MaskScalar( ret );
}


RW_MATH_FORCE_INLINE Soa4MaskScalar CompAllTrue(Soa4Mask4::InParam a)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_And( ret, a.x.mV, a.y.mV );
    RW_MATH_VPU_And( ret, ret, a.z.mV );
    RW_MATH_VPU_And( ret, ret, a.w.mV );

    return Soa4MaskScalar( ret );
}


RW_MATH_FORCE_INLINE Soa4Mask4 Select(Soa4MaskScalar::InParam mask, Soa4Mask4::InParam trueValue, Soa4Mask4::InParam falseValue )
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.mV );
    RW_MATH_VPU_Select( z, falseValue.z.mV, trueValue.z.mV, mask.mV );
    RW_MATH_VPU_Select( w, falseValue.w.mV, trueValue.w.mV, mask.mV );

    return Soa4Mask4( x, y, z, w );
}


RW_MATH_FORCE_INLINE Soa4Mask4 Select(Soa4Mask4::InParam mask, Soa4Mask4::InParam trueValue, Soa4Mask4::InParam falseValue )
{
    VectorIntrinsic x, y, z, w;
    RW_MATH_VPU_Select( x, falseValue.x.mV, trueValue.x.mV, mask.x.mV );
    RW_MATH_VPU_Select( y, falseValue.y.mV, trueValue.y.mV, mask.y.mV );
    RW_MATH_VPU_Select( z, falseValue.z.mV, trueValue.z.mV, mask.z.mV );
    RW_MATH_VPU_Select( w, falseValue.w.mV, trueValue.w.mV, mask.w.mV );

    return Soa4Mask4( x, y, z, w );
}



} // namespace vpu
} // namespace math
} // namespace rw

#endif

