// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_MACROS_H
#define RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_MACROS_H

#if defined _MSC_VER
    #pragma once
#endif

// Define operation functions
#define RWMATH_DETAIL_VEC_FLOAT_asmAdd( a, b ) \
    ((a) + (b))

#define RWMATH_DETAIL_VEC_FLOAT_asmSubtract( a, b ) \
    ((a) - (b))

#define RWMATH_DETAIL_VEC_FLOAT_asmMult( a, b ) \
    ((a) * (b))

#define RWMATH_DETAIL_VEC_FLOAT_asmDivide( a, b ) \
    ((a) / (b))

// Define common macros for operating on vec_float types.
#define RWMATH_DETAIL_SELF_OPERATION__VECFLOAT_VECFLOAT(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat()) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_VECFLOATREF(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_VECFLOATREFINDEX(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOAT_FLOAT(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), b ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), b ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), (b).GetFloat() ) ); \
    return a;

#define RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(op, a, b) \
    (a).SetFloat( RWMATH_DETAIL_VEC_FLOAT ## op( (a).GetFloat(), b ) ); \
    return a;

#endif
