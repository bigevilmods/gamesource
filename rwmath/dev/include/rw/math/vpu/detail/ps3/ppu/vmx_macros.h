// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VMX_MACROS_H
#define RW_MATH_VPU_DETAIL_VMX_MACROS_H

#ifndef __SNC__
#if CELL_SDK_VERSION >= 0x084000

// These instructions were first exposed in the 084 SDK.

#define RWMATH_VPU_DETAIL_VecLvlx( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_lvlx ((a1), (const void *)(a2))

#define RWMATH_VPU_DETAIL_VecLvrx( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_lvrx ((a1), (const void *)(a2))

#define RWMATH_VPU_DETAIL_VecStvlx( vec, offset, addr ) \
    __builtin_altivec_stvlx ((vector signed char)(vec), (offset), (void *)(addr))

#define RWMATH_VPU_DETAIL_VecStvrx( vec, offset, addr ) \
    __builtin_altivec_stvrx ((vector signed char)(vec), (offset), (void *)(addr))

#endif

#define RW_MATH_VPU_IsAnyEqual_Int(ret, in_vec0, in_vec1) \
    ret = __builtin_altivec_vcmpequw_p(__CR6_EQ_REV, (vector signed int)(in_vec0), (vector signed int)(in_vec1))

#define RW_MATH_VPU_IsAllEqual_Int(ret, in_vec0, in_vec1) \
    ret = __builtin_altivec_vcmpequw_p(__CR6_LT, (vector signed int)(in_vec0), (vector signed int)(in_vec1))

#define RW_MATH_VPU_Or_R(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vor((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_And_R(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vand((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_And_Rc(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vandc((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_Xor_R(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vxor((vector signed int)(a1), (vector signed int)(a2))


#define RW_MATH_VPU_Select_R(a1, a2, a3) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsel_4sf((vector float)(a1), (vector float)(a2), (vector signed int)(a3))

#define RWMATH_VPU_DETAIL_VecAllEq(a1, a2) \
    __builtin_altivec_vcmpeqfp_p(__CR6_LT, (vector float)(a1), (vector float)(a2))

#define RWMATH_VPU_DETAIL_VecAllGE( a1, a2 ) \
    __builtin_altivec_vcmpgefp_p(__CR6_LT, (vector float)(a1), (vector float)(a2))

#define RWMATH_VPU_DETAIL_VecAllGT( a1, a2 ) \
    __builtin_altivec_vcmpgtfp_p(__CR6_LT, (vector float)(a1), (vector float)(a2))

#define RWMATH_VPU_DETAIL_VecAnyGT( a1, a2 ) \
    __builtin_altivec_vcmpgtfp_p(__CR6_EQ_REV, (vector float)(a1), (vector float)(a2))

#define RWMATH_VPU_DETAIL_VecCmpEq( a1, a2 ) \
    (vector float)__builtin_altivec_vcmpeqfp((vector float)(a1), (vector float)(a2))

#define RWMATH_VPU_DETAIL_VecCmpB( a1, a2 ) \
    (vector float)vec_cmpb((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord( a1, a2 ) \
    (vector float)__builtin_altivec_vcmpequw((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_IsGreaterEqual_R(a1, a2) \
    (vector float)__builtin_altivec_vcmpgefp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpGT(a1, a2) \
    (vector float)__builtin_altivec_vcmpgtfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMadd( a1, a2, a3 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaddfp((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecNmsub( a1, a2, a3 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vnmsubfp((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecLd( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_lvx ((a1), (const void *)(a2))

#define RWMATH_VPU_DETAIL_VecLde( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_lvewx ((a1), (const void *)(a2))

#define RWMATH_VPU_DETAIL_VecLvsl( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_lvsl((a1), (const void *)(a2))

#define RWMATH_VPU_DETAIL_VecLvsr( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_lvsr((a1), (const void *)(a2))

#define RWMATH_VPU_DETAIL_VecSt( vec, offset, addr) \
    __builtin_altivec_stvx ((vector signed int) vec, offset, (void *) addr)

#define RWMATH_VPU_DETAIL_VecRe( a1 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vrefp((vector float)(a1))

#define RWMATH_VPU_DETAIL_VecSqrtRe( a1 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vrsqrtefp((vector float)(a1))

#define RWMATH_VPU_DETAIL_VecSplat( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vspltw ((vector signed int)(a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatHalf( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsplth((vector signed short)(a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatByte( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vspltb((vector signed char)(a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatS32( a1 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vspltisw((a1))

#define RWMATH_VPU_DETAIL_VecSpltish( a1 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vspltish((a1))

#define RWMATH_VPU_DETAIL_VecSld( a1, a2, a3 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsldoi_4sf ((vector float)(a1), (vector float)(a2), (a3))

#define RWMATH_VPU_DETAIL_VecVcfsx(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcfsx((vector signed int)(a1), (a2))

#define RWMATH_VPU_DETAIL_VecPerm(a1, a2, a3) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vperm_4si((vector signed int)(a1), (vector signed int)(a2), (vector signed char)(a3))

#define RWMATH_VPU_DETAIL_VecMin(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMinsb(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminsb((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMinub(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminub((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMinsh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminsh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMinuh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminuh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMinsw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminsw((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMinuw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vminuw((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMax(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMaxsb(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxsb((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMaxub(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxub((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMaxsh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxsh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMaxuh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxuh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMaxsw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxsw((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMaxuw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmaxuw((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMergeH( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmrghw((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMergeL( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmrglw((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMergeHH( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmrghh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMergeLH( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmrglh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMergeHB( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmrghb((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMergeLB( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vmrglb((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecZero() \
    (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplatS32(0)

#define RWMATH_VPU_DETAIL_VecSlw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vslw((vector signed int)(a1),(vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSl(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsl((vector signed int)(a1),(vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSlo(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vslo((vector signed int)(a1),(vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSrw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsrw((vector signed int)(a1),(vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSro(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsro((vector signed int)(a1),(vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSraw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsraw((vector signed int)(a1),(vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecCtsxs(a1, exp) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vctsxs( a1, exp )

#define RWMATH_VPU_DETAIL_VecCtuxs(a1, exp) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vctuxs( a1, exp )

#define RWMATH_VPU_DETAIL_VecCfsx(a1, exp) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcfsx( (vector signed int)a1, exp )

#define RWMATH_VPU_DETAIL_VecCfux(a1, exp) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcfux( (vector signed int)a1, exp )

#define RWMATH_VPU_DETAIL_VecSpltisb(imm) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vspltisb(imm)

#define RWMATH_VPU_DETAIL_VecSr(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsr((vector signed int)a1, (vector signed int)a2)

#define RWMATH_VPU_DETAIL_VecRfim(a1) \
    __builtin_altivec_vrfim(a1)

#define RWMATH_VPU_DETAIL_VecRfip(a1) \
    __builtin_altivec_vrfip(a1)

#define RWMATH_VPU_DETAIL_VecRfiz(a1) \
    __builtin_altivec_vrfiz(a1)

#define RWMATH_VPU_DETAIL_VecLogefp(a1) \
    __builtin_altivec_vlogefp(a1)

#define RWMATH_VPU_DETAIL_VecNor(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vnor((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecStvewx( vec, offset, addr ) \
    __builtin_altivec_stvewx((vector signed int)(vec), (offset), (void *)(addr))

#define RWMATH_VPU_DETAIL_VecStvehx( vec, offset, addr ) \
    __builtin_altivec_stvehx((vector signed short)(vec), (offset), (void *)(addr))

#define RWMATH_VPU_DETAIL_VecStvebx( vec, offset, addr ) \
    __builtin_altivec_stvebx((vector signed char)(vec), (offset), (void *)(addr))

#define RWMATH_VPU_DETAIL_VecAdd(a1, a2) \
    __builtin_altivec_vaddfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecSub(a1, a2) \
    __builtin_altivec_vsubfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecAddubm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vaddubm((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecAddubs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vaddubs((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecAddsbs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vaddsbs((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSububm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsububm((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSubsbs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubsbs((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSububs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsububs((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecAdduhm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vadduhm((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecAdduhs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vadduhs((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecAddshs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vaddshs((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecSubuhm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubuhm((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecSubshs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubshs((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecSubuhs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubuhs((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecAdduwm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vadduwm((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecAdduws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vadduws((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecAddsws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vaddsws((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSubuwm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubuwm((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSubsws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubsws((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSubuws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vsubuws((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecCmpequw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpequw( (vector signed int)v0, (vector signed int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtuw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpgtuw( (vector signed int)v0, (vector signed int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpequw( (vector signed int)v0, (vector signed int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtsw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpgtsw( (vector signed int)v0, (vector signed int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpequh( (vector signed short)v0, (vector signed short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtuh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpgtuh( (vector signed short)v0, (vector signed short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpequh( (vector signed short)v0, (vector signed short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtsh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpgtsh( (vector signed short)v0, (vector signed short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequb(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpequb( (vector signed char)v0, (vector signed char)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtub(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpgtub( (vector signed char)v0, (vector signed char)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequb(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpequb( (vector signed char)v0, (vector signed char)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtsb(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)__builtin_altivec_vcmpgtsb( (vector signed char)v0, (vector signed char)v1 )


#else

#define RWMATH_VPU_DETAIL_VecLvlx( a1, a2 ) \
    vec_lvlx((a1), (const float *)(a2))

#define RWMATH_VPU_DETAIL_VecLvrx( a1, a2 ) \
    vec_lvrx((a1), (const float *)(a2))

#define RWMATH_VPU_DETAIL_VecStvlx( vec, offset, addr ) \
    vec_stvlx((vec), (offset), (float *)(addr))

#define RWMATH_VPU_DETAIL_VecStvrx( vec, offset, addr ) \
    vec_stvrx((vec), (offset), (float *)(addr))

#define RW_MATH_VPU_IsAnyEqual_Int(ret, in_vec0, in_vec1) \
    ret = vec_any_eq((vector signed int)(in_vec0), (vector signed int)(in_vec1))

#define RW_MATH_VPU_IsAllEqual_Int(ret, in_vec0, in_vec1) \
    ret = vec_all_eq((vector signed int)(in_vec0), (vector signed int)(in_vec1))

#define RW_MATH_VPU_Or_R(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_or((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_And_R(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_and((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_And_Rc(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_andc((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_Xor_R(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_xor((vector signed int)(a1), (vector signed int)(a2))


#define RW_MATH_VPU_Select_R(a1, a2, a3) \
    vec_sel((vector float)(a1), (vector float)(a2), (vector unsigned int)(a3))

#define RWMATH_VPU_DETAIL_VecAllEq(a1, a2) \
    vec_all_eq((a1), (a2))

#define RWMATH_VPU_DETAIL_VecAllGE( a1, a2 ) \
    vec_all_ge((a1), (a2))

#define RWMATH_VPU_DETAIL_VecAllGT( a1, a2 ) \
    vec_all_gt((a1), (a2))

#define RWMATH_VPU_DETAIL_VecAnyGT( a1, a2 ) \
    vec_any_gt((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpEq( a1, a2 ) \
    (vector float)vec_cmpeq((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpB( a1, a2 ) \
    (vector float)vec_vcmpbfp((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord( a1, a2 ) \
    (vector float)vec_cmpeq((vector signed int)(a1), (vector signed int)(a2))

#define RW_MATH_VPU_IsGreaterEqual_R(a1, a2) \
    (vector float)vec_cmpge((a1), (a2))

#define RWMATH_VPU_DETAIL_VecCmpGT(a1, a2) \
    (vector float)vec_cmpgt((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMadd( a1, a2, a3 ) \
    vec_madd((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecNmsub( a1, a2, a3 ) \
    vec_nmsub((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecLd( a1, a2 ) \
    vec_lvx((a1), (const float *)(a2))

#define RWMATH_VPU_DETAIL_VecLde( a1, a2 ) \
    vec_lvewx ((a1), (const float *)(a2))

#define RWMATH_VPU_DETAIL_VecLvsl( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_lvsl((a1), (const float *)(a2))

#define RWMATH_VPU_DETAIL_VecLvsr( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_lvsr((a1), (const float *)(a2))

#define RWMATH_VPU_DETAIL_VecSt( vec, offset, addr) \
    vec_stvx(vec, offset, (float *)(addr))

#define RWMATH_VPU_DETAIL_VecRe( a1 ) \
    vec_re((a1))

#define RWMATH_VPU_DETAIL_VecSqrtRe( a1 ) \
    vec_rsqrte((a1))

#define RWMATH_VPU_DETAIL_VecSplat( a1, a2 ) \
    vec_vspltw ((a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatHalf( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsplth((vector signed short)(a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatByte( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_vspltb((vector signed char)(a1),  (a2))

#define RWMATH_VPU_DETAIL_VecSplatS32( a1 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_splat_s32((a1))

#define RWMATH_VPU_DETAIL_VecSpltish( a1 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_splat_s16((a1))

#define RWMATH_VPU_DETAIL_VecSpltisb(imm) \
    (::rw::math::vpu::VectorIntrinsic)vec_splat_s8(imm)

#define RWMATH_VPU_DETAIL_VecSld( a1, a2, a3 ) \
    vec_vsldoi ((a1), (a2), (a3))

#define RWMATH_VPU_DETAIL_VecVcfsx(a1, a2) \
    vec_vcfsx((vector signed int)(a1), (a2))

#define RWMATH_VPU_DETAIL_VecPerm(a1, a2, a3) \
    vec_perm((a1), (a2), (vector unsigned char)(a3))

#define RWMATH_VPU_DETAIL_VecMin(a1, a2) \
    vec_min((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMinsb(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_min((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMinub(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_min((vector unsigned char)(a1), (vector unsigned char)(a2))

#define RWMATH_VPU_DETAIL_VecMinsh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_min((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMinuh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_min((vector unsigned short)(a1), (vector unsigned short)(a2))

#define RWMATH_VPU_DETAIL_VecMinsw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_min((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMinuw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_min((vector unsigned int)(a1), (vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecMax(a1, a2) \
    vec_max((a1), (a2))

#define RWMATH_VPU_DETAIL_VecMaxsb(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_max((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMaxub(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_max((vector unsigned char)(a1), (vector unsigned char)(a2))

#define RWMATH_VPU_DETAIL_VecMaxsh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_max((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMaxuh(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_max((vector unsigned short)(a1), (vector unsigned short)(a2))

#define RWMATH_VPU_DETAIL_VecMaxsw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_max((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMaxuw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_max((vector unsigned int)(a1), (vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecMergeH( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_mergeh((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMergeL( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_mergel((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecMergeHH( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_mergeh((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMergeLH( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_mergel((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecMergeHB( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_mergeh((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecMergeLB( a1, a2 ) \
    (::rw::math::vpu::VectorIntrinsic)vec_mergel((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecZero() \
    ((::rw::math::vpu::VectorIntrinsic)0)

#define RWMATH_VPU_DETAIL_VecSlw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vslw((vector unsigned int)(a1),(vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecSl(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsl((vector unsigned int)(a1),(vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecSlo(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vslo((vector signed int)(a1),(vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSrw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsrw((vector unsigned int)(a1),(vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecSro(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsro((vector signed int)(a1),(vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSraw(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsraw((vector unsigned int)(a1),(vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecCtsxs(a1, exp) \
    (::rw::math::vpu::VectorIntrinsic)vec_vctsxs( a1, exp )

#define RWMATH_VPU_DETAIL_VecCtuxs(a1, exp) \
    (::rw::math::vpu::VectorIntrinsic)vec_vctuxs( a1, exp )

#define RWMATH_VPU_DETAIL_VecCfsx(a1, exp) \
    vec_vcfsx( (vector signed int)a1, exp )

#define RWMATH_VPU_DETAIL_VecCfux(a1, exp) \
    vec_vcfux( (vector unsigned int)a1, exp )

#define RWMATH_VPU_DETAIL_VecSr(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsr((vector unsigned int)(a1),(vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecRfim(a1) \
    vec_vrfim(a1)

#define RWMATH_VPU_DETAIL_VecRfip(a1) \
    vec_vrfip(a1)

#define RWMATH_VPU_DETAIL_VecRfiz(a1) \
    vec_vrfiz(a1)

#define RWMATH_VPU_DETAIL_VecLogefp(a1) \
    vec_vlogefp(a1)

#define RWMATH_VPU_DETAIL_VecNor(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_nor((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecStvewx( vec, offset, addr ) \
    vec_stvewx((vector signed int)(vec), (offset), (int *)(addr))

#define RWMATH_VPU_DETAIL_VecStvehx( vec, offset, addr ) \
    vec_stvehx((vector signed short)(vec), (offset), (short *)(addr))

#define RWMATH_VPU_DETAIL_VecStvebx( vec, offset, addr ) \
    vec_stvebx((vector signed char)(vec), (offset), (signed char *)(addr))

#define RWMATH_VPU_DETAIL_VecAdd(a1, a2) \
    vec_add((a1), (a2))

#define RWMATH_VPU_DETAIL_VecSub(a1, a2) \
    vec_sub((a1), (a2))

#define RWMATH_VPU_DETAIL_VecAddubm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vaddubm((vector unsigned char)(a1), (vector unsigned char)(a2))

#define RWMATH_VPU_DETAIL_VecAddubs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vaddubs((vector unsigned char)(a1), (vector unsigned char)(a2))

#define RWMATH_VPU_DETAIL_VecAddsbs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vaddsbs((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSububm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsububm((vector unsigned char)(a1), (vector unsigned char)(a2))

#define RWMATH_VPU_DETAIL_VecSubsbs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubsbs((vector signed char)(a1), (vector signed char)(a2))

#define RWMATH_VPU_DETAIL_VecSububs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsububs((vector unsigned char)(a1), (vector unsigned char)(a2))

#define RWMATH_VPU_DETAIL_VecAdduhm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vadduhm((vector unsigned short)(a1), (vector unsigned short)(a2))

#define RWMATH_VPU_DETAIL_VecAdduhs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vadduhs((vector unsigned short)(a1), (vector unsigned short)(a2))

#define RWMATH_VPU_DETAIL_VecAddshs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vaddshs((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecSubuhm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubuhm((vector unsigned short)(a1), (vector unsigned short)(a2))

#define RWMATH_VPU_DETAIL_VecSubshs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubshs((vector signed short)(a1), (vector signed short)(a2))

#define RWMATH_VPU_DETAIL_VecSubuhs(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubuhs((vector unsigned short)(a1), (vector unsigned short)(a2))

#define RWMATH_VPU_DETAIL_VecAdduwm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vadduwm((vector unsigned int)(a1), (vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecAdduws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vadduws((vector unsigned int)(a1), (vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecAddsws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vaddsws((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSubuwm(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubuwm((vector unsigned int)(a1), (vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecSubsws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubsws((vector signed int)(a1), (vector signed int)(a2))

#define RWMATH_VPU_DETAIL_VecSubuws(a1, a2) \
    (::rw::math::vpu::VectorIntrinsic)vec_vsubuws((vector unsigned int)(a1), (vector unsigned int)(a2))

#define RWMATH_VPU_DETAIL_VecCmpequw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpequw( (vector unsigned int)v0, (vector unsigned int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtuw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpgtuw( (vector unsigned int)v0, (vector unsigned int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpequw( (vector unsigned int)v0, (vector unsigned int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtsw(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpgtsw( (vector signed int)v0, (vector signed int)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpequh( (vector unsigned short)v0, (vector unsigned short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtuh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpgtuh( (vector unsigned short)v0, (vector unsigned short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpequh( (vector unsigned short)v0, (vector unsigned short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtsh(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpgtsh( (vector signed short)v0, (vector signed short)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequb(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpequb( (vector unsigned char)v0, (vector unsigned char)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtub(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpgtub( (vector unsigned char)v0, (vector unsigned char)v1 )

#define RWMATH_VPU_DETAIL_VecCmpequb(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpequb( (vector unsigned char)v0, (vector unsigned char)v1 )

#define RWMATH_VPU_DETAIL_VecCmpgtsb(v0, v1) \
    (::rw::math::vpu::VectorIntrinsic)vec_vcmpgtsb( (vector signed char)v0, (vector signed char)v1 )

#endif // EA_COMPILER_GNUC

#endif
