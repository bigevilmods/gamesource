// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASKSCALAR_OPERATION_INLINE_H
#define RW_MATH_VPU_MASKSCALAR_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/maskscalar_operation.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{


RW_MATH_FORCE_INLINE MaskScalar Not(MaskScalar::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Not( result, a.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar And(MaskScalar::InParam a, MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_And( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Or(MaskScalar::InParam a, MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Or( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Xor(MaskScalar::InParam a, MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompEqual(MaskScalar::InParam a, MaskScalar::InParam b)
{
    VectorIntrinsic result;
    // TODO: This implementation could be changed to a single integer vector comparison on most platforms.
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompNotEqual(MaskScalar::InParam a, MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Select(MaskScalar::InParam mask, MaskScalar::InParam trueValue, MaskScalar::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return MaskScalar(result);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_MASKSCALAR_OPERATION_INLINE_H
