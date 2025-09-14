// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/vec_float_operation_platform_inline.h"

namespace rw
{
namespace math
{
namespace vpu
{

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool operator==(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(VecFloat::InParam a, VecFloat::InParam b) 
{
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator==(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator==(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator==(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator==(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator==(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator==(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator==(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, bVF);
    return ret;
}

// operator!=

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool operator!=(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(VecFloat::InParam a, VecFloat::InParam b) 
{
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, b.mV);
    return !ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator!=(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, bVF);
    return !ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator!=(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, b.mV);
    return !ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator!=(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator!=(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, b.mV);
    return !ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator!=(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, b.mV);
    return !ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator!=(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, aVF, bVF);
    return !ret;
}

RW_MATH_FORCE_INLINE bool operator!=(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, a.mV, bVF);
    return !ret;
}

// operator >

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool operator>(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(VecFloat::InParam a, VecFloat::InParam b) 
{
    bool result;
    RW_MATH_VPU_Scalar_IsGreater(result, a.mV, b.mV);
    return result;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, a.mV, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, a.mV, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreater(ret, a.mV, bVF);
    return ret;
}

// operator <

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool operator<(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(VecFloat::InParam a, VecFloat::InParam b) 
{
    bool result;
    RW_MATH_VPU_Scalar_IsLess(result, a.mV, b.mV);
    return result;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, a.mV, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, a.mV, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsLess(ret, a.mV, bVF);
    return ret;
}


// operator >=

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool operator>=(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(VecFloat::InParam a, VecFloat::InParam b) 
{
    bool result;
    RW_MATH_VPU_Scalar_IsGreaterEqual(result, a.mV, b.mV);
    return result;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>=(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, a.mV, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>=(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>=(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>=(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, a.mV, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>=(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator>=(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator>=(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsGreaterEqual(ret, a.mV, bVF);
    return ret;
}


// operator <=

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool operator<=(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(VecFloat::InParam a, VecFloat::InParam b) 
{
    bool result;
    RW_MATH_VPU_Scalar_IsLessEqual(result, a.mV, b.mV);
    return result;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<=(VecFloat::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, a.mV, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<=(VecFloatRef<INDEX> a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<=(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<=(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, a.mV, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<=(const float& a, VecFloatRef<INDEX> b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(aVF, &a);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, b.mV);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool operator<=(VecFloatRef<INDEX> a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, aVF, bVF);
    return ret;
}

RW_MATH_FORCE_INLINE bool operator<=(VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVF, &b);
    bool ret;
    RW_MATH_VPU_Scalar_IsLessEqual(ret, a.mV, bVF);
    return ret;
}

// operator -

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Negate(ret, a.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX> a) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX);
    RW_MATH_VPU_Negate(ret, aVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Negate(ret, aVF);
    return VecFloat(ret);
}

//----------------------------------------------------------------------------
// operator +=
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Add(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE float& operator += (float& a, VecFloat::InParam b) 
{
    a += b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& operator += (float& a, VecFloatRef<INDEX1> b) 
{
    a += b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& operator += (float& a, VecFloatRefIndex::InParam b) 
{
    a += b.GetFloat();
    return a;
}


//----------------------------------------------------------------------------
// operator -=
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Subtract(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE float& operator -= (float& a, VecFloat::InParam b) 
{
    a -= b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& operator -= (float& a, VecFloatRef<INDEX1> b) 
{
    a -= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& operator -= (float& a, VecFloatRefIndex::InParam b) 
{
    a -= b.GetFloat();
    return a;
}




//----------------------------------------------------------------------------
// operator *=
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Mult(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE float& operator *= (float& a, VecFloat::InParam b) 
{
    a *= b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& operator *= (float& a, VecFloatRef<INDEX1> b) 
{
    a *= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& operator *= (float& a, VecFloatRefIndex::InParam b) 
{
    a *= b.GetFloat();
    return a;
}



//----------------------------------------------------------------------------
// operator /=
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Divide(ret, a.mV, bVec);
    const_cast<VectorIntrinsic&>(a.mV) = ret; 
    return a;
}

RW_MATH_FORCE_INLINE float& operator /= (float& a, VecFloat::InParam b) 
{
    a /= b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& operator /= (float& a, VecFloatRef<INDEX1> b) 
{
    a /= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& operator /= (float& a, VecFloatRefIndex::InParam b) 
{
    a /= b.GetFloat();
    return a;
}



//----------------------------------------------------------------------------
// operator +
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, bVec);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    RW_MATH_VPU_Add(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Add(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Add(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Add(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Add(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator + (const float& a, VecFloat::InParam b) 
{
    return VecFloat(a) + b;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator + (const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a) + b;
}

RW_MATH_FORCE_INLINE VecFloat operator + (const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a) + b;
}




//----------------------------------------------------------------------------
// operator -
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, bVec);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    RW_MATH_VPU_Subtract(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Subtract(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Subtract(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Subtract(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVec;
    RW_MATH_VPU_CreateVectorV1_FromMemory(bVec, &b);
    RW_MATH_VPU_Subtract(ret, aVF, bVec);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator - (const float& a, VecFloat::InParam b) 
{
    return VecFloat(a) - b;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator - (const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a) - b;
}

RW_MATH_FORCE_INLINE VecFloat operator - (const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a) - b;
}



//----------------------------------------------------------------------------
// operator *
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, a.mV, b);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    RW_MATH_VPU_Mult(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    RW_MATH_VPU_MultFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Mult(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Mult(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_MultFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (const float& a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, b.mV, a);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator * (const float& a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, bVF, a);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator * (const float& a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    VectorIntrinsic ret;
    RW_MATH_VPU_MultFloat(ret, bVF, a);
    return VecFloat(ret);
}



//----------------------------------------------------------------------------
// operator /
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, a.mV, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, a.mV, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideFloat(ret, a.mV, b);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    RW_MATH_VPU_Divide(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX2);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat_I(aVF, a.mV, INDEX1);
    RW_MATH_VPU_DivideFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_Divide(ret, aVF, b.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat_I(bVF, b.mV, INDEX1);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    VectorIntrinsic bVF;
    RW_MATH_VPU_Splat(bVF, b.mV, b.mIndex);
    RW_MATH_VPU_Divide(ret, aVF, bVF);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, const float& b) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVF;
    RW_MATH_VPU_Splat(aVF, a.mV, a.mIndex);
    RW_MATH_VPU_DivideFloat(ret, aVF, b);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat operator / (const float& a, VecFloat::InParam b) 
{
    return VecFloat(a) / b;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat operator / (const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a) / b;
}

RW_MATH_FORCE_INLINE VecFloat operator / (const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a) / b;
}


} // namespace vpu
} // namespace math
} // namespace rw

#endif
