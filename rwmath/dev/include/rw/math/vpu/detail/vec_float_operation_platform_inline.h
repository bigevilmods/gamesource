// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VEC_FLOAT_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_VEC_FLOAT_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if defined RWCROSS || defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined EA_PLATFORM_REVOLUTION || (defined EA_PLATFORM_POSIX && !(defined RW_MATH_TARGET_SUPPORTS_SSE))
    #include "rw/math/vpu/detail/cross/vec_float_operation_platform_inline.h"
#else

#include "rw/math/vpu/detail/vec_float_macros.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE VecFloat Negate(VecFloat::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Negate(ret, a.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE VecFloat Negate(VecFloatRef<INDEX> a) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_Negate(ret, aVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Negate(VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Negate(ret, aVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Add(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfAdd(VecFloatRef<INDEX> &a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Add, a, b);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfAdd(VecFloatRef<INDEX> &a, VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Add, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfAdd(VecFloatRef<INDEX> &a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Add, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfAdd(VecFloatRef<INDEX> &a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Add, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Add, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, VecFloatRef<INDEX> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Add, a, b);
}


RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Add, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Add, a, b);
}

RW_MATH_FORCE_INLINE float& SelfAdd(float& a, VecFloat::InParam b) 
{
    a += b.GetFloat();
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE float& SelfAdd(float& a, VecFloatRef<INDEX> b) 
{
    a += b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfAdd(float& a, VecFloatRefIndex::InParam b) 
{
    a += b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfAdd(float& a, const float& b) 
{
    a += b;
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Subtract(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfSubtract(VecFloatRef<INDEX> &a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Subtract, a, b);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfSubtract(VecFloatRef<INDEX> &a, VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Subtract, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfSubtract(VecFloatRef<INDEX> &a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Subtract, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfSubtract(VecFloatRef<INDEX> &a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Subtract, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Subtract, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, VecFloatRef<INDEX> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Subtract, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Subtract, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Subtract, a, b);
}

RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, VecFloat::InParam b) 
{
    a -= b.GetFloat();
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, VecFloatRef<INDEX> b) 
{
    a -= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, VecFloatRefIndex::InParam b) 
{
    a -= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, const float& b) 
{
    a -= b;
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Mult(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfMult(VecFloatRef<INDEX> &a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Mult, a, b);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfMult(VecFloatRef<INDEX> &a, VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Mult, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfMult(VecFloatRef<INDEX> &a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Mult, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfMult(VecFloatRef<INDEX> &a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Mult, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Mult, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, VecFloatRef<INDEX> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Mult, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Mult, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Mult, a, b);
}

RW_MATH_FORCE_INLINE float& SelfMult(float& a, VecFloat::InParam b) 
{
    a *= b.GetFloat();
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE float& SelfMult(float& a, VecFloatRef<INDEX> b) 
{
    a *= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfMult(float& a, VecFloatRefIndex::InParam b) 
{
    a *= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfMult(float& a, const float& b) 
{
    a *= b;
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Divide(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfDivide(VecFloatRef<INDEX> &a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Divide, a, b);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfDivide(VecFloatRef<INDEX> &a, VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Divide, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfDivide(VecFloatRef<INDEX> &a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Divide, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& SelfDivide(VecFloatRef<INDEX> &a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Divide, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Divide, a, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, VecFloatRef<INDEX> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Divide, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Divide, a, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Divide, a, b);
}

RW_MATH_FORCE_INLINE float& SelfDivide(float& a, VecFloat::InParam b) 
{
    a /= b.GetFloat();
    return a;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE float& SelfDivide(float& a, VecFloatRef<INDEX> b) 
{
    a /= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfDivide(float& a, VecFloatRefIndex::InParam b) 
{
    a /= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfDivide(float& a, const float& b) 
{
    a /= b;
    return a;
}

#define RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOAT(op, a, b) \
    return VecFloat(op(a.mV, b.mV));

#define RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREF(op, a, b) \
    return VecFloat(op(a.mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREFINDEX(op, a, b) \
    return VecFloat(op(a.mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOAT_FLOAT(op, a, b) \
    return VecFloat(op(a.mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOAT(op, a, b) \
    return VecFloat(op(VecFloat(a).mV, b.mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREF(op, a, b) \
    return VecFloat(op(VecFloat(a).mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREFINDEX(op, a, b) \
return VecFloat(op(VecFloat(a).mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_FLOAT(op, a, b) \
return VecFloat(op(VecFloat(a).mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOAT(op, a, b) \
    return VecFloat(op(VecFloat(a).mV, b.mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREF(op, a, b) \
    return VecFloat(op(VecFloat(a).mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(op, a, b) \
    return VecFloat(op(VecFloat(a).mV, VecFloat(b).mV));

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_FLOAT(op, a, b) \
    return VecFloat(op(VecFloat(a).mV, VecFloat(b).mV));

RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, bVec);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_Add(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX> a, VecFloatRef<INDEX_OTHER> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX_OTHER);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Add(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Add(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Add(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a) + b;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloatRef<INDEX> b) 
{
    return VecFloat(a) + b;
}

RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a) + b;
}

RW_MATH_FORCE_INLINE VecFloat Add(const float& a, const float& b) 
{
    return VecFloat(a) + b;
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, bVec);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_Subtract(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX> a, VecFloatRef<INDEX_OTHER> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX_OTHER);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Subtract(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Subtract(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1(bVec, b);
    RW_MATH_VPU_Subtract(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a) - b;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloatRef<INDEX> b) 
{
    return VecFloat(a) - b;
}

RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a) - b;
}

RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, const float& b) 
{
    return VecFloat(a) - b;
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, a.mV, b);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_Mult(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX> a, VecFloatRef<INDEX_OTHER> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX_OTHER);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_MultFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Mult(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_MultFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, b.mV, a);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, bVF, a);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, bVF, a);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, const float& b) 
{
    return VecFloat(a) * b;
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideFloat(ret, a.mV, b);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_Divide(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX, VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX> a, VecFloatRef<INDEX_OTHER> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX_OTHER);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_DivideFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Divide(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_DivideFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a) / b;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloatRef<INDEX> b) 
{
    return VecFloat(a) / b;
}

RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a) / b;
}

RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, const float& b) 
{
    return VecFloat(a) / b;
}

}}}

#endif
#endif
