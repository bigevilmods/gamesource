// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VEC_FLOAT_MACROS_H
#define RW_MATH_VPU_DETAIL_VEC_FLOAT_MACROS_H

#if defined _MSC_VER
    #pragma once
#endif

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_VECFLOAT(op, a, b) \
    VectorIntrinsic r; \
    op(r, (a).mV, b.mV); \
    const_cast<VectorIntrinsic&>((a).mV) = r; \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_VECFLOATREF(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI; \
    RW_MATH_VPU_Splat_I( bVI, (b).mV, INDEX ); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = r; \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_VECFLOATREFINDEX(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI; \
    RW_MATH_VPU_Splat( bVI, (b).mV, (b).mIndex ); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = r; \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_FLOAT(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI = RW_MATH_VPU_CreateVectorFromScalar_R(b); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = r; \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(op, a, b) \
    VectorIntrinsic r; \
    op(r, (a).mV, b.mV); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore<INDEX, INDEX>((a).mV, r); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI; \
    RW_MATH_VPU_Splat_I( bVI, (b).mV, INDEX_OTHER ); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore<INDEX, INDEX>((a).mV, r); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI; \
    RW_MATH_VPU_Splat( bVI, (b).mV, (b).mIndex ); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore<INDEX, INDEX>((a).mV, r); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI = RW_MATH_VPU_CreateVectorFromScalar_R(b); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore<INDEX, INDEX>((a).mV, r); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic aVI; \
    RW_MATH_VPU_Splat( aVI, (a).mV, (a).mIndex ); \
    op(r, aVI, (b).mV); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore((a).mV, (a).mIndex, r, (a).mIndex); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI; \
    RW_MATH_VPU_Splat_I( bVI, (b).mV, INDEX ); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore((a).mV, (a).mIndex, r, (a).mIndex); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI; \
    RW_MATH_VPU_Splat( bVI, (b).mV, (b).mIndex ); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore((a).mV, (a).mIndex, r, (a).mIndex); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(op, a, b) \
    VectorIntrinsic r; \
    VectorIntrinsic bVI = RW_MATH_VPU_CreateVectorFromScalar_R(b); \
    op(r, (a).mV, bVI); \
    const_cast<VectorIntrinsic&>((a).mV) = _asmSwizzleStore((a).mV, (a).mIndex, r, (a).mIndex); \
    return a;

#endif
