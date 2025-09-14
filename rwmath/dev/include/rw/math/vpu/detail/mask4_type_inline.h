// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MASK4_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_MASK4_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask4.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Mask4::Mask4()
{
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Mask4::Mask4(const Mask4 &v)
    :mV(v.mV)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
}

RW_MATH_FORCE_INLINE Mask4& Mask4::operator =(const Mask4&v)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}
#endif

RW_MATH_FORCE_INLINE Mask4::Mask4(VectorIntrinsicInParam v)
    :mV(v)
{
}

RW_MATH_FORCE_INLINE Mask4::Mask4(bool x, bool y, bool z, bool w)
{
#ifdef RWMATH_USE_NEON
    uint32x4_t uintVec = { x ? 0xffffffffu : 0, y ? 0xffffffffu : 0, z ? 0xffffffffu : 0, w ? 0xffffffffu : 0 };
    mV = vreinterpretq_f32_u32(uintVec);
#else
    VectorIntrinsic xVec, yVec, zVec, wVec;
    RW_MATH_VPU_CreateVectorMaskFromBool(xVec, x);
    RW_MATH_VPU_CreateVectorMaskFromBool(yVec, y);
    RW_MATH_VPU_CreateVectorMaskFromBool(zVec, z);
    RW_MATH_VPU_CreateVectorMaskFromBool(wVec, w);
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, xVec, yVec, zVec, wVec );
#endif
}

RW_MATH_FORCE_INLINE Mask4::Mask4(uint32_t xMask, uint32_t yMask, uint32_t zMask, uint32_t wMask)
{
    detail::VectorIntrinsicUnion value;
    value.scalarUInt[0] = xMask;
    value.scalarUInt[1] = yMask;
    value.scalarUInt[2] = zMask;
    value.scalarUInt[3] = wMask;
    mV = value.vector;
}

RW_MATH_FORCE_INLINE Mask4::Mask4(MaskScalar::InParam s)
    :mV(s.mV)
{
}

RW_MATH_FORCE_INLINE Mask4::Mask4(MaskScalar::InParam x, MaskScalar::InParam y, MaskScalar::InParam z, MaskScalar::InParam w)
{
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, x.mV, y.mV, z.mV, w.mV );
}

RW_MATH_FORCE_INLINE MaskScalar Mask4::GetX() const
{
    VectorIntrinsic result;
    RW_MATH_VPU_Splat_I( result, mV, 0 );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Mask4::GetY() const
{
    VectorIntrinsic result;
    RW_MATH_VPU_Splat_I( result, mV, 1 );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Mask4::GetZ() const
{
    VectorIntrinsic result;
    RW_MATH_VPU_Splat_I( result, mV, 2 );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar Mask4::GetW() const
{
    VectorIntrinsic result;
    RW_MATH_VPU_Splat_I( result, mV, 3 );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE void Mask4::SetX(MaskScalar::InParam v)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, v.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Mask4::SetY(MaskScalar::InParam v)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, v.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Mask4::SetZ(MaskScalar::InParam v)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, v.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Mask4::SetW(MaskScalar::InParam v)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, v.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Mask4::SetTrue()
{
    RW_MATH_VPU_GetTrue(mV);
}

RW_MATH_FORCE_INLINE void Mask4::SetFalse()
{
    RW_MATH_VPU_GetZero(mV);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_DETAIL_MASK4_TYPE_INLINE_H
