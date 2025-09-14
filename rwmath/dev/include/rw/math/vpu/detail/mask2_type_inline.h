// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MASK2_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_MASK2_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask2.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Mask2::Mask2()
{
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Mask2::Mask2(const Mask2 &v)
    :mV(v.mV)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2(v.mV);
#endif
}

RW_MATH_FORCE_INLINE Mask2& Mask2::operator =(const Mask2&v)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2(v.mV);
#endif
    mV = v.mV;
    return *this;
}
#endif

RW_MATH_FORCE_INLINE Mask2::Mask2(VectorIntrinsicInParam v)
    :mV(v)
{
}

RW_MATH_FORCE_INLINE Mask2::Mask2(bool x, bool y)
{
    VectorIntrinsic xVec, yVec;
    RW_MATH_VPU_CreateVectorMaskFromBool(xVec, x);
    RW_MATH_VPU_CreateVectorMaskFromBool(yVec, y);
    RW_MATH_VPU_SwizzleStore_I_I( mV, xVec, VectorAxisY, yVec, VectorAxisX );
}

RW_MATH_FORCE_INLINE Mask2::Mask2(uint32_t xMask, uint32_t yMask)
{
    detail::VectorIntrinsicUnion value;
    value.scalarUInt[0] = xMask;
    value.scalarUInt[1] = yMask;
    mV = value.vector;
}

RW_MATH_FORCE_INLINE Mask2::Mask2(MaskScalar::InParam s)
    :mV(s.mV)
{
}

RW_MATH_FORCE_INLINE Mask2::Mask2(MaskScalar::InParam x, MaskScalar::InParam y)
{
    RW_MATH_VPU_CreateVectorAxisA2_I(mV, x.mV, VectorAxisX, y.mV, VectorAxisY);
}

RW_MATH_FORCE_INLINE MaskScalar Mask2::GetX() const
{
    VectorIntrinsic result;
    RW_MATH_VPU_Splat_I( result, mV, 0 );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Mask2::GetY() const
{
    VectorIntrinsic result;
    RW_MATH_VPU_Splat_I( result, mV, 1 );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE void Mask2::SetX(MaskScalar::InParam v)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, v.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Mask2::SetY(MaskScalar::InParam v)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, v.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Mask2::SetTrue()
{
    RW_MATH_VPU_GetTrue(mV);
}

RW_MATH_FORCE_INLINE void Mask2::SetFalse()
{
    RW_MATH_VPU_GetZero(mV);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_DETAIL_MASK2_TYPE_INLINE_H
