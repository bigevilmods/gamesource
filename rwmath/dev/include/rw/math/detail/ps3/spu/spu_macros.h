// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SPU_MACROS_H
#define RW_MATH_VPU_DETAIL_SPU_MACROS_H

#if defined _MSC_VER
    #pragma once
#endif

// This is a "helper" macro.  It converts a vector representing a comparison to
// a conditional "code".  The code is 4bits (4 elements of 1 bit).  Where the elements
// are either 0 or 1 depending on whether the input vector was true/false for the
// corresponding element.
#define RWMATH_VPU_DETAIL_VecToConditionCode( a1 ) \
    spu_extract(__builtin_spu_gather_0((vector signed int)(a1)), 0)

#define RWMATH_VPU_DETAIL_VecAllTrue( a1 ) \
    (RWMATH_VPU_DETAIL_VecToConditionCode(a1) == 0xf )

#define RWMATH_VPU_DETAIL_VecAnyTrue( a1 ) \
    (RWMATH_VPU_DETAIL_VecToConditionCode(a1) != 0x0 )

#define RWMATH_VPU_DETAIL_VecAllFalse( a1 ) \
    (RWMATH_VPU_DETAIL_VecToConditionCode(a1) == 0x0 )

#define RWMATH_VPU_DETAIL_VecAnyFalse( a1 ) \
    (RWMATH_VPU_DETAIL_VecToConditionCode(a1) != 0xf )

#define RW_MATH_VPU_Xor_R(a1, a2) \
    ((VectorIntrinsic)__builtin_si_xor((vector signed char)(a1), (vector signed char)(a2)))

#define RW_MATH_VPU_Or_R(a1, a2) \
    ((VectorIntrinsic)__builtin_si_or((vector signed char)(a1), (vector signed char)(a2)))

#define RW_MATH_VPU_And_R(a1, a2) \
    ((VectorIntrinsic)__builtin_si_and((vector signed char)(a1), (vector signed char)(a2)))

#define RW_MATH_VPU_And_Rc(a1, a2) \
    ((VectorIntrinsic)__builtin_si_andc((vector signed char)(a1), (vector signed char)(a2)))

#define RW_MATH_VPU_Not_R(a1, a2) \
    ((VectorIntrinsic)__builtin_si_nor((vector signed char)(a1), (vector signed char)(a2)))

// Usage:
//   For each bit 'b' in a3 the corresponding output bit will be taken from:
//      a1 if 'b' is 0
//      a2 if 'b' is 1
#define RW_MATH_VPU_Select_R(a1, a2, a3) \
    ((VectorIntrinsic)__builtin_spu_sel_7((vector unsigned char)a1, (vector unsigned char)a2, (vector unsigned char)a3))

#define RWMATH_VPU_DETAIL_VecCmpEq( a1, a2 ) \
    ((VectorIntrinsic)__builtin_spu_cmpeq_6 ((a1), (a2)))

#define RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord( a1, a2 ) \
    ((VectorIntrinsic)__builtin_spu_cmpeq_4((vector unsigned int)(a1), (vector unsigned int)(a2)))

#define RWMATH_VPU_DETAIL_VecCmpGT(a1, a2) \
    ((VectorIntrinsic)__builtin_spu_cmpgt_6((a1), (a2)))

#define RW_MATH_VPU_IsGreaterEqual_R(a1, a2) \
    ((VectorIntrinsic)(RW_MATH_VPU_Xor_R(RWMATH_VPU_DETAIL_VecCmpGT(a2, a1), ((vector unsigned int)(0xffffffff)) )))

#define RWMATH_VPU_DETAIL_VecAllEq(a1, a2) \
    RWMATH_VPU_DETAIL_VecAllTrue( RWMATH_VPU_DETAIL_VecCmpEq((a1),(a2)) )

#define RWMATH_VPU_DETAIL_VecAllLE( a1, a2 ) \
    RWMATH_VPU_DETAIL_VecAllFalse( RWMATH_VPU_DETAIL_VecCmpGT((a1), (a2)) )

#define RWMATH_VPU_DETAIL_VecAnyGT( a1, a2 ) \
    RWMATH_VPU_DETAIL_VecAnyTrue( RWMATH_VPU_DETAIL_VecCmpGT((a1),(a2)) )

#define RWMATH_VPU_DETAIL_VecMadd( a1, a2, a3 ) \
    __builtin_spu_madd_1((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecNmsub( a1, a2, a3 ) \
    __builtin_spu_nmsub_0 ((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecRe( a1 ) \
    (VectorIntrinsic)__builtin_si_fi((vector signed char)(a1), __builtin_si_frest((vector signed char)(a1)))

#define RWMATH_VPU_DETAIL_VecSqrtRe( a1 ) \
    (VectorIntrinsic)__builtin_si_fi((vector signed char)(a1), __builtin_si_frsqest((vector signed char)(a1)))

#define RWMATH_VPU_DETAIL_VecSplat( a1, a2 ) \
    RWMATH_VPU_DETAIL_VecPerm((a1), /*dummy arg*/(a1), __builtin_si_orbi(__builtin_si_ila(0x00010203), (a2)*4))

/* NOTE: SPU code should probably avoid doing this splat and use the immediate value instructions. */
#define RWMATH_VPU_DETAIL_VecSplatS32( a1 ) \
    (VectorIntrinsic)((vec_int4)((signed int)(a1)))

#define RWMATH_VPU_DETAIL_VecSld( a1, a2, a3 ) \
    RWMATH_VPU_DETAIL_VecPerm( (a1), (a2), rw::math::vpu::detail::gShiftLeftDoubleConstants[(a3)])

#define RWMATH_VPU_DETAIL_VecPerm(a1, a2, a3) \
    ((VectorIntrinsic)__builtin_spu_shuffle_5((vector signed int)(a1), (vector signed int)(a2), (vector unsigned char)(a3)))

#define RWMATH_VPU_DETAIL_VecMin(a1, a2) \
    (RW_MATH_VPU_Select_R((a1), (a2), RWMATH_VPU_DETAIL_VecCmpGT((a1), (a2))))

#define RWMATH_VPU_DETAIL_VecMax(a1, a2) \
    (RW_MATH_VPU_Select_R((a2), (a1), RWMATH_VPU_DETAIL_VecCmpGT((a1), (a2))))

#define RWMATH_VPU_DETAIL_VecAbs(a1) \
    RW_MATH_VPU_And_R((a1), rw::math::vpu::detail::gExponentAndMantissa)

#define RWMATH_VPU_DETAIL_VecMergeH( a1, a2 ) \
    RWMATH_VPU_DETAIL_VecPerm( (a1), (a2), rw::math::vpu::detail::gMergeHighPermuteConstant)

#define RWMATH_VPU_DETAIL_VecMergeL( a1, a2 ) \
    RWMATH_VPU_DETAIL_VecPerm( (a1), (a2), rw::math::vpu::detail::gMergeLowPermuteConstant)

#define RWMATH_VPU_DETAIL_VecZero() \
    ((VectorIntrinsic)(0.0f))

#if (__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)
// GCC 4.1 switched the order of parameters for the __builtin_si_sfi intrinsic.
#define RWMATH_VPU_DETAIL_SubtractIntVectorFromImmediate( vec, immediate ) \
        __builtin_si_sfi( (vector signed char)vec, immediate )
#else
#define RWMATH_VPU_DETAIL_SubtractIntVectorFromImmediate( vec, immediate ) \
        __builtin_si_sfi( immediate, (vector signed char)vec )

#endif

// This macro will take the x component of an vector and convert it to a bool
// The x component should be either 0x00000000 or 0xffffffff.
#define RWMATH_VPU_DETAIL_MaskVectorXValueToBool(ret, intValue) \
{ \
    vector signed char negateMask = RWMATH_VPU_DETAIL_SubtractIntVectorFromImmediate(intValue,0); \
    union { bool boolValue; signed char charValue; } boolUnion; \
    boolUnion.charValue = spu_extract( negateMask, 3 ); \
    ret = boolUnion.boolValue; \
}

#endif
