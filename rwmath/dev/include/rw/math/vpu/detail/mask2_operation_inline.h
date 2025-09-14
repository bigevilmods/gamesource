// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK2_OPERATION_INLINE_H
#define RW_MATH_VPU_MASK2_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask2_operation.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{


RW_MATH_FORCE_INLINE Mask2 Not(Mask2::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Not( result, a.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 And(Mask2::InParam a, Mask2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_And( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 Or(Mask2::InParam a, Mask2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Or( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 Xor(Mask2::InParam a, Mask2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 CompEqual(Mask2::InParam a, Mask2::InParam b)
{
    VectorIntrinsic result;
    // TODO: This implementation could be changed to a single integer vector comparison.
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 CompNotEqual(Mask2::InParam a, Mask2::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompAnyTrue(Mask2::InParam a)
{
    VectorIntrinsic result, tempX, tempY;
    RW_MATH_VPU_Splat_I( tempX, a.mV, 0 );
    RW_MATH_VPU_Splat_I( tempY, a.mV, 1 );
    RW_MATH_VPU_Or( result, tempX, tempY );
    return MaskScalar( result );
}

RW_MATH_FORCE_INLINE MaskScalar CompAllTrue(Mask2::InParam a)
{
    VectorIntrinsic result, tempX, tempY;
    RW_MATH_VPU_Splat_I( tempX, a.mV, 0 );
    RW_MATH_VPU_Splat_I( tempY, a.mV, 1 );
    RW_MATH_VPU_And( result, tempX, tempY );
    return MaskScalar( result );
}

RW_MATH_FORCE_INLINE Mask2 Select(MaskScalar::InParam mask, Mask2::InParam trueValue, Mask2::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 Select(Mask2::InParam mask, Mask2::InParam trueValue, Mask2::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 RotateLeft(Mask2::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V2_RotateLeft( result, a.mV );
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask2 RotateRight(Mask2::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_V2_RotateLeft( result, a.mV );
    return Mask2(result);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_MASK3_OPERATION_INLINE_H
