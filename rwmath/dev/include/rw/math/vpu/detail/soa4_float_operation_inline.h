// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_VEC_FLOAT_OPERATION_INLINE_H_INCLUDED
#define RW_MATH_SOA4_VEC_FLOAT_OPERATION_INLINE_H_INCLUDED

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_float_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromFloat(const float& v)
{
    VectorIntrinsic result;
    RW_MATH_VPU_CreateVectorV1_FromMemory(result, &v);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromFloats(const float& v0, const float& v1, const float& v2, const float& v3 )
{
    VectorIntrinsic result;
    RW_MATH_VPU_CreateVectorV4(result, v0, v1, v2, v3);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromVecFloat(VecFloat::InParam v)
{
    return Soa4Float(v.mV);
}

RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromVecFloats(VecFloat::InParam v0, VecFloat::InParam v1, VecFloat::InParam v2, VecFloat::InParam v3 )
{
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(result, v0.mV, v1.mV, v2.mV, v3.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float  Soa4FloatFromVector4(Vector4::InParam v)
{
    return Soa4Float(v.mV);
}

RW_MATH_FORCE_INLINE void Soa4FloatToFloats(Soa4Float::InParam v, float& r0, float& r1, float& r2, float& r3 )
{
    detail::VectorIntrinsicUnion vU;
    vU.vector = v.mV;
    r0 = vU.scalarFloat[0];
    r1 = vU.scalarFloat[1];
    r2 = vU.scalarFloat[2];
    r3 = vU.scalarFloat[3];
}

RW_MATH_FORCE_INLINE void Soa4FloatToVecFloats(Soa4Float::InParam v, VecFloat& r0, VecFloat& r1, VecFloat& r2, VecFloat& r3)
{
    RW_MATH_VPU_Splat_I(r0.mV, v.mV, VectorAxisX);
    RW_MATH_VPU_Splat_I(r1.mV, v.mV, VectorAxisY);
    RW_MATH_VPU_Splat_I(r2.mV, v.mV, VectorAxisZ);
    RW_MATH_VPU_Splat_I(r3.mV, v.mV, VectorAxisW);
}

RW_MATH_FORCE_INLINE Vector4 Soa4FloatToVector4(Soa4Float::InParam v)
{
    return Vector4(v.mV);
}

RW_MATH_FORCE_INLINE Soa4Float  Negate(Soa4Float::InParam a)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Negate(ret, a.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float& SelfAdd(Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float& SelfSubtract(Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float& SelfMult(Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float& SelfDivide(Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float  Add(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float  Subtract(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float  Mult(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float  Divide(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    return Soa4Float(ret);
}


RW_MATH_FORCE_INLINE Soa4Float  operator - (Soa4Float::InParam a)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Negate(ret, a.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float& operator += (Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float& operator -= (Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float& operator *= (Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float& operator /= (Soa4Float& a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE Soa4Float  operator + (Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float  operator - (Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float  operator * (Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float  operator / (Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    return Soa4Float(ret);
}


} // namespace vpu
} // namespace math
} // namespace rw


#endif // RW_MATH_SOA4_VEC_FLOAT_OPERATION_INLINE_H_INCLUDED
