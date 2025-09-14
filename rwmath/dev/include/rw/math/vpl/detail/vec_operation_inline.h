// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_DETAIL_INLINE_H
#define RW_MATH_VEC_OPERATION_DETAIL_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpl
{

#if VPL_SUPPORTED_PLATFORM( VPL_SSE | VPL_PPU | VPL_SPU | VPL_XENON | VPL_SSE2 | VPL_NEON)
    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_2
    // VPL_PPU_EQUIVALENT vec_abs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAbs(VectorIntrinsicInParam v)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Abs(result, v);
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_add_ps
    // VPL_PPU_EQUIVALENT vec_add / vaddfp
    // VPL_XENON_EQUIVALENT __vaddfp / vaddfp
    // VPL_SPU_EQUIVALENT spu_add / fa
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RW_MATH_VPU_Add_R(v0, v1);
    }

    // VPL_NEON_RATING_2
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API bool VecAllTrue_Word(VectorIntrinsicInParam v)
    {
        bool result;
        RW_MATH_VPU_IsAllTrue(result, v);
        return result;
    }

    // VPL_NEON_RATING_2
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API bool VecAllFalse_Word(VectorIntrinsicInParam v)
    {
        bool result;
        RW_MATH_VPU_IsAllFalse(result, v);
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_and_ps
    // VPL_PPU_EQUIVALENT vec_and / vand
    // VPL_XENON_EQUIVALENT __vand / vand
    // VPL_SPU_EQUIVALENT spu_and / and
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAnd(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RW_MATH_VPU_And_R(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_andnot_ps
    // VPL_PPU_EQUIVALENT vec_andc / vandc
    // VPL_XENON_EQUIVALENT __vandc / vandc
    // VPL_SPU_EQUIVALENT spu_andc / andc
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAndComplement(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RW_MATH_VPU_And_Rc(v0, v1);
    }

    // VPL_NEON_RATING_2
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API bool VecAnyTrue_Word(VectorIntrinsicInParam v)
    {
        bool result;
        RW_MATH_VPU_IsAnyTrue(result, v);
        return result;
    }

    // VPL_NEON_RATING_2
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API bool VecAnyFalse_Word(VectorIntrinsicInParam v)
    {
        bool result;
        RW_MATH_VPU_IsAnyFalse(result, v);
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_cmpeq_ps
    // VPL_PPU_EQUIVALENT vec_cmpeq / vcmpeqfp
    // VPL_XENON_EQUIVALENT __vcmpeqfp / vcmpeqfp 
    // VPL_SPU_EQUIVALENT spu_cmpeq / fceq
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_IsEqual( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_SSE_EQUIVALENT _mm_cmpge_ps
    // VPL_PPU_EQUIVALENT vec_cmpge / vcmpgefp
    // VPL_XENON_EQUIVALENT __vcmpgefp / vcmpgefp
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_IsGreaterEqual( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_cmpgt_ps 
    // VPL_PPU_EQUIVALENT vec_cmpgt / vcmpgtfp
    // VPL_XENON_EQUIVALENT __vcmpgtfp / vcmpgtfp
    // VPL_SPU_EQUIVALENT spu_cmpgt / fcgt
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_IsGreater( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_IsGreaterEqual( result, v1, v0 );
        return result;
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_IsGreater( result, v1, v0 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetOne()
    {
        return RW_MATH_VPU_GetOne_R();
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetHalf()
    {
        return RW_MATH_VPU_GetHalf_R();
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetTwo()
    {
        return RW_MATH_VPU_GetTwo_R();
    }

    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_setzero_ps
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetZero()
    {
        return RW_MATH_VPU_GetZero_R();
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegOne()
    {
        return RW_MATH_VPU_GetNegOne_R();
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegHalf()
    {
        return RW_MATH_VPU_GetNegHalf_R();
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegTwo()
    {
        return RW_MATH_VPU_GetNegTwo_R();
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegZero()
    {
        VectorIntrinsic result;
        RW_MATH_VPU_GetSignMaskXYZW(result);
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_SSE_EQUIVALENT _mm_max_ps
    // VPL_PPU_RATING_1
    // VPL_PPU_EQUIVALENT vec_max / vmaxfp
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_XENON_EQUIVALENT __vmaxfp / vmaxfp
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Max( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_SSE_EQUIVALENT _mm_min_ps
    // VPL_PPU_RATING_1
    // VPL_PPU_EQUIVALENT vec_min / vminfp
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_XENON_EQUIVALENT __vminfp / vminfp
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Min( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    // VPL_SSE_EQUIVALENT _mm_mul_ps
    // VPL_XENON_EQUIVALENT __vmulfp / vmulfp128
    // VPL_SPU_EQUIVALENT spu_mul / fm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMult(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Mult( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    // VPL_PPU_EQUIVALENT vec_madd / vmaddfp
    // VPL_XENON_EQUIVALENT __vmaddfp / vmaddfp
    // VPL_SPU_EQUIVALENT spu_madd / fma
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultAdd(VectorIntrinsicInParam mult0, VectorIntrinsicInParam mult1, VectorIntrinsicInParam add)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_MultAdd( result, mult0, mult1, add );
        return result;
    }

    // VPL_NEON_RATING_2
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_XENON_EQUIVALENT __vmsum3fp / vmsum3fp128
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultSum3(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_DotV3( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_2
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_2
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_XENON_EQUIVALENT __vmsum4fp / vmsum4fp128
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultSum4(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_DotV4( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    // VPL_PPU_EQUIVALENT vec_nmsub / vnmsubfp
    // VPL_XENON_EQUIVALENT __vnmsubfp / vnmsubfp
    // VPL_SPU_EQUIVALENT spu_nmsub / fnms
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNegMultSub(VectorIntrinsicInParam mult0, VectorIntrinsicInParam mult1, VectorIntrinsicInParam sub)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_MultSubNeg( result, mult0, mult1, sub );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNot(VectorIntrinsicInParam v0)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Not( result, v0 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_or_ps
    // VPL_PPU_EQUIVALENT vec_or / vor
    // VPL_XENON_EQUIVALENT __vor / vor
    // VPL_SPU_EQUIVALENT spu_or / or
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecOr(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Or( result, v0, v1 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_SSE_EQUIVALENT _mm_rcp_ps
    // VPL_PPU_EQUIVALENT vec_re / vrefp
    // VPL_XENON_EQUIVALENT __vrefp / vrefp
    // VPL_SPU_EQUIVALENT spu_re
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecRecipEst(VectorIntrinsicInParam v0)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_ReciprocalEstimate( result, v0 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_SSE_EQUIVALENT _mm_rsqrt_ps
    // VPL_PPU_EQUIVALENT vec_rsqrte / vrsqrtefp
    // VPL_XENON_EQUIVALENT __vrsqrtefp / vrsqrtefp
    // VPL_SPU_EQUIVALENT spu_rsqrte
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecRecipSqrtEst(VectorIntrinsicInParam v0)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_ReciprocalSquareRootEstimate( result, v0 );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    VPL_API template<int RotateAmount> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftImm(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< RotateAmount, 0, 3 >();
        VectorIntrinsic result;
        RW_MATH_VPU_RotateLeft_V_I( result, v0, RotateAmount );
        return result;
    }

	// Specialization for a rotation by 0.
    template<> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftImm<0>(VectorIntrinsicInParam v0)
    {
        return v0;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_2
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    // VPL_PPU_EQUIVALENT vec_sel / vsel
    // VPL_XENON_EQUIVALENT __vsel / vsel
    // VPL_SPU_EQUIVALENT spu_sel / selb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSelect(VectorIntrinsicInParam if0Result, VectorIntrinsicInParam if1Result, VectorIntrinsicInParam selectionMask)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Select( result, if0Result, if1Result, selectionMask );
        return result;
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_2
    // VPL_XENON_RATING_1
    // VPL_PPU_EQUIVALENT vec_splat / vspltw
    // VPL_XENON_EQUIVALENT __vspltw / vspltw
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Word(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ElementIndex, 0, 3 >();
        VectorIntrinsic result;
        RW_MATH_VPU_Splat_I(result, v0, ElementIndex);
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_sub_ps
    // VPL_PPU_EQUIVALENT vec_sub / vsubfp
    // VPL_XENON_EQUIVALENT __vsubfp / vsubfp
    // VPL_SPU_EQUIVALENT spu_sub / fs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RW_MATH_VPU_Subtract_R(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_SSE_EQUIVALENT _mm_xor_ps
    // VPL_PPU_EQUIVALENT vec_xor / vxor
    // VPL_XENON_EQUIVALENT __vxor / vxor
    // VPL_SPU_EQUIVALENT spu_xor / xor
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecXor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RW_MATH_VPU_Xor_R(v0, v1);
    }

#endif

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    // VPL_BROADWAY_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoad(const void *base, int offset)
    {
        return *reinterpret_cast<const VectorIntrinsic*>(reinterpret_cast<const char*>(base)+offset);
    }

    // VPL_NEON_RATING_1
    // VPL_SSE_RATING_1
    // VPL_PPU_RATING_1
    // VPL_SPU_RATING_1
    // VPL_XENON_RATING_1
    // VPL_BROADWAY_RATING_2
    RW_MATH_FORCE_INLINE VPL_API void VecStore(VectorIntrinsicInParam v0, void *base, int offset)
    {
        *reinterpret_cast<VectorIntrinsic*>(reinterpret_cast<char*>(base)+offset) = v0;
    }

} // namespace vpl
} // namespace math
} // namespace rw

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined EA_PLATFORM_REVOLUTION || defined RWCROSS
    #include "rw/math/vpl/detail/cross/vec_operation_inline.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpl/detail/ps3/ppu/vec_operation_inline.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpl/detail/ps3/spu/vec_operation_inline.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpl/detail/xbox2/vec_operation_inline.h"
#elif defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/vpl/detail/x86/vec_operation_inline.h"
#elif defined EA_PLATFORM_AIRPLAY
    #include "rw/math/vpl/detail/cross/vec_operation_inline.h"
#elif defined EA_PROCESSOR_ARM || defined EA_PLATFORM_POSIX
    #ifdef RWMATH_USE_NEON
        #include "rw/math/vpl/detail/neon/vec_operation_inline.h"
    #else
        #include "rw/math/vpl/detail/cross/vec_operation_inline.h"
    #endif
#else
    #error Unsupported platform
#endif

#endif
