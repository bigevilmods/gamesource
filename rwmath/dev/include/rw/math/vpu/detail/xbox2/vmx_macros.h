// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VMX_MACROS_H
#define RW_MATH_VPU_DETAIL_VMX_MACROS_H

#if defined _MSC_VER
    #pragma once
#endif

#define RW_MATH_VPU_Or_R( a1, a2 ) \
    __vor((a1), (a2))

#define RW_MATH_VPU_And_R( a1, a2 ) \
    __vand((a1), (a2))

#define RW_MATH_VPU_And_Rc(a1, a2) \
    __vandc((a1), (a2))

#define RW_MATH_VPU_Xor_R(a1, a2) \
    __vxor((a1), (a2))

#define RW_MATH_VPU_Not_R(a1, a2) \
    __vnor((a1), (a2))

#define RW_MATH_VPU_Select_R( a1, a2, a3 ) \
    __vsel((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecMergeHB( a1, a2 ) \
    __vmrghb( (a1), (a2) )

#define RWMATH_VPU_DETAIL_VecMergeLB( a1, a2 ) \
    __vmrglb( (a1), (a2) )

#define RWMATH_VPU_DETAIL_VecAllEq(a1, a2) \
    RWMATH_COMPARE_ALL_TRUE(RWMATH_VPU_VMX128_Vector4Equal(a1, a2))

#define RWMATH_VPU_DETAIL_VecAnyGT( a1, a2 ) \
    RWMATH_COMPARE_ANY_TRUE(RWMATH_VPU_VMX128_Vector4Greater(a1, a2))

#define RWMATH_VPU_DETAIL_VecCmpEq( a1, a2 ) \
    __vcmpeqfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpB( a1, a2 ) \
    __vcmpbfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord( a1, a2 ) \
    __vcmpequw((a1), (a2))

#define RW_MATH_VPU_IsGreaterEqual_R(a1, a2) \
    __vcmpgefp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpGT(a1, a2) \
    __vcmpgtfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMadd( a1, a2, a3 ) \
    __vmaddfp((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecNmsub( a1, a2, a3 ) \
    __vnmsubfp((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecLvsl( a1, a2 ) \
    __lvsl((const void *)(a2), (a1))

#define RWMATH_VPU_DETAIL_VecRe( a1 ) \
    __vrefp((a1))

#define RWMATH_VPU_DETAIL_VecSqrtRe( a1 ) \
    __vrsqrtefp((a1))

#define RWMATH_VPU_DETAIL_VecSplat( a1, a2 ) \
    __vspltw ((a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatS32( a1 ) \
    __vspltisw((a1))

#define RWMATH_VPU_DETAIL_VecSld( a1, a2, a3 ) \
    __vsldoi((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecSt( vec, offset, addr) \
    __stvx((vec), (void *)(addr), (offset) )

#define RWMATH_VPU_DETAIL_VecVcfsx(a1, a2) \
      __vcfsx((a1), (a2))

#define RWMATH_VPU_DETAIL_VecPerm( a1, a2, a3 ) \
    __vperm((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecMin(a1, a2) \
    __vminfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMax(a1, a2) \
    __vmaxfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMergeH( a1, a2 ) \
    __vmrghw((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMergeL( a1, a2 ) \
    __vmrglw((a1), (a2))

#define RWMATH_VPU_DETAIL_VecZero() \
        RWMATH_VPU_DETAIL_VecSplatS32(0)

#define RWMATH_VPU_DETAIL_VecSlw(a1, a2) \
    __vslw((a1),(a2))

#define RWMATH_VPU_DETAIL_VecSrw(a1, a2) \
    __vsrw((a1),(a2))

#define RWMATH_VPU_DETAIL_VecCtsxs(a1, exp) \
     __vctsxs((a1), (exp))

#define RWMATH_VPU_DETAIL_VecStvewx( vec, offset, addr ) \
    __stvewx((vec), (void *)(addr), (offset))

//----------------------------------------------------------------------------
// VMX128 specific macros
//----------------------------------------------------------------------------

#define RWMATH_VPU_DETAIL_VecRlimi(a1, a2, writeMask, rotateLeft) \
    __vrlimi((a1), (a2), (writeMask), (rotateLeft))

// Note: This macro could be implemented for VMX, but it would introduce an extra instruction.
#define RWMATH_VPU_DETAIL_VecMul( a1, a2 ) \
    __vmulfp((a1), (a2))

#endif
