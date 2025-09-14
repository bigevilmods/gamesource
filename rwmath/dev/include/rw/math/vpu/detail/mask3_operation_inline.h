// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK3_OPERATION_INLINE_H
#define RW_MATH_VPU_MASK3_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask3_operation.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{


RW_MATH_FORCE_INLINE Mask3 Not(Mask3::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Not( result, a.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 And(Mask3::InParam a, Mask3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_And( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 Or(Mask3::InParam a, Mask3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Or( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 Xor(Mask3::InParam a, Mask3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 CompEqual(Mask3::InParam a, Mask3::InParam b)
{
    VectorIntrinsic result;
    // TODO: This implementation could be changed to a single integer vector comparison.
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 CompNotEqual(Mask3::InParam a, Mask3::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompAnyTrue(Mask3::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_CompAnyTrueV3( result, a.mV );
    return MaskScalar( result );
}

RW_MATH_FORCE_INLINE MaskScalar CompAllTrue(Mask3::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_CompAllTrueV3( result, a.mV );
    return MaskScalar( result );
}

RW_MATH_FORCE_INLINE Mask3 Select(MaskScalar::InParam mask, Mask3::InParam trueValue, Mask3::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 Select(Mask3::InParam mask, Mask3::InParam trueValue, Mask3::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 RotateLeft(Mask3::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V3_RotateLeft( result, a.mV );
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask3 RotateRight(Mask3::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V3_RotateRight( result, a.mV );
    return Mask3(result);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_MASK3_OPERATION_INLINE_H
