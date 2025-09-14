// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK4_OPERATION_INLINE_H
#define RW_MATH_VPU_MASK4_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask4_operation.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{


RW_MATH_FORCE_INLINE Mask4 Not(Mask4::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Not( result, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 And(Mask4::InParam a, Mask4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_And( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 Or(Mask4::InParam a, Mask4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Or( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 Xor(Mask4::InParam a, Mask4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 CompEqual(Mask4::InParam a, Mask4::InParam b)
{
    VectorIntrinsic result;
    // TODO: This implementation could be changed to a single integer vector comparison.
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 CompNotEqual(Mask4::InParam a, Mask4::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompAnyTrue(Mask4::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_CompAnyTrueV4( result, a.mV );
    return MaskScalar( result );
}

RW_MATH_FORCE_INLINE MaskScalar CompAllTrue(Mask4::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_CompAllTrueV4( result, a.mV );
    return MaskScalar( result );
}

RW_MATH_FORCE_INLINE Mask4 Select(MaskScalar::InParam mask, Mask4::InParam trueValue, Mask4::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 Select(Mask4::InParam mask, Mask4::InParam trueValue, Mask4::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 RotateLeft(Mask4::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V4_RotateLeft( result, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4 RotateRight(Mask4::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V4_RotateRight( result, a.mV );
    return Mask4(result);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_MASK4_OPERATION_INLINE_H
