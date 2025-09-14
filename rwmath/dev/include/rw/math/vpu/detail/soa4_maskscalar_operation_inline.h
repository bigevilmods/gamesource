// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_MASKSCALAR_OPERATION_INLINE_H
#define RW_MATH_SOA4_MASKSCALAR_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_maskscalar_operation.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromBool(bool v)
{
    VectorIntrinsic result;
    RW_MATH_VPU_CreateVectorMaskFromBool(result, v);
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromBools(bool v0, bool v1, bool v2, bool v3)
{
    VectorIntrinsic result;
#ifdef RWMATH_USE_NEON
    uint32x4_t uintVec = { v0 ? 0xffffffffu : 0, v1 ? 0xffffffffu : 0, v2 ? 0xffffffffu : 0, v3 ? 0xffffffffu : 0 };
    result = vreinterpretq_f32_u32(uintVec);
#else
    VectorIntrinsic xVec, yVec, zVec, wVec;
    RW_MATH_VPU_CreateVectorMaskFromBool(xVec, v0);
    RW_MATH_VPU_CreateVectorMaskFromBool(yVec, v1);
    RW_MATH_VPU_CreateVectorMaskFromBool(zVec, v2);
    RW_MATH_VPU_CreateVectorMaskFromBool(wVec, v3);
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(result, xVec, yVec, zVec, wVec );
#endif
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromUInt(uint32_t v)
{
    detail::VectorIntrinsicUnion vU;
    vU.scalarUInt[0] = v;
    vU.scalarUInt[1] = v;
    vU.scalarUInt[2] = v;
    vU.scalarUInt[3] = v;
    return Soa4MaskScalar(vU.vector);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromUInts(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
    detail::VectorIntrinsicUnion vU;
    vU.scalarUInt[0] = v0;
    vU.scalarUInt[1] = v1;
    vU.scalarUInt[2] = v2;
    vU.scalarUInt[3] = v3;
    return Soa4MaskScalar(vU.vector);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromMaskScalar(MaskScalar::InParam v)
{
    return Soa4MaskScalar(v.mV);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromMaskScalars(MaskScalar::InParam v0, MaskScalar::InParam v1, MaskScalar::InParam v2, MaskScalar::InParam v3 )
{
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(result, v0.mV, v1.mV, v2.mV, v3.mV);
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromMask4(Mask4::InParam v)
{
    return Soa4MaskScalar(v.mV);
}


RW_MATH_FORCE_INLINE void Soa4MaskScalarToBools(Soa4MaskScalar::InParam v, bool& r0, bool& r1, bool& r2, bool& r3 )
{
    detail::VectorIntrinsicUnion vU;
    vU.vector = v.mV;
    r0 = vU.scalarUInt[0] == 0 ? false : true;
    r1 = vU.scalarUInt[1] == 0 ? false : true;
    r2 = vU.scalarUInt[2] == 0 ? false : true;
    r3 = vU.scalarUInt[3] == 0 ? false : true;
}


RW_MATH_FORCE_INLINE void Soa4MaskScalarToMaskScalars(Soa4MaskScalar::InParam v, MaskScalar& r0, MaskScalar& r1, MaskScalar& r2, MaskScalar& r3)
{
    VectorIntrinsic v0( v.mV ), v1( v.mV ), v2( v.mV ), v3( v.mV );

    RW_MATH_VPU_TransposeA4V4( r0.mV, r1.mV, r2.mV, r3.mV, v0, v1, v2, v3 );
}


RW_MATH_FORCE_INLINE Mask4 Soa4MaskScalarToMask4(Soa4MaskScalar::InParam v)
{
    return Mask4( v.mV );
}


RW_MATH_FORCE_INLINE bool IsAnyTrue(Soa4MaskScalar::InParam a)
{
    bool result;
    RW_MATH_VPU_IsAnyTrue(result, a.mV);
    return result;
}


RW_MATH_FORCE_INLINE bool IsAllTrue(Soa4MaskScalar::InParam a)
{
    bool result;
    RW_MATH_VPU_IsAllTrue(result, a.mV);
    return result;
}


RW_MATH_FORCE_INLINE bool IsAnyFalse(Soa4MaskScalar::InParam a)
{
    bool result;
    RW_MATH_VPU_IsAnyFalse(result, a.mV);
    return result;
}


RW_MATH_FORCE_INLINE bool IsAllFalse(Soa4MaskScalar::InParam a)
{
    bool result;
    RW_MATH_VPU_IsAllFalse(result, a.mV);
    return result;
}


RW_MATH_FORCE_INLINE Soa4MaskScalar Not(Soa4MaskScalar::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Not( result, a.mV );
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar And(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_And( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar Or(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Or( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar Xor(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar CompEqual(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b)
{
    VectorIntrinsic result;
    // TODO: This implementation could be changed to a single integer vector comparison on most platforms.
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Soa4MaskScalar(result);
}


RW_MATH_FORCE_INLINE Soa4MaskScalar CompNotEqual(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Xor( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}


} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_SOA4_MASKSCALAR_OPERATION_INLINE_H
