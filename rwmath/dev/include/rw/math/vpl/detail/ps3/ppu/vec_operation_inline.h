// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_INLINE_H
#define RW_MATH_VEC_OPERATION_INLINE_H

#include "rw/math/vpu/detail/ps3/ppu/vmx_macros.h"
#include "rw/math/vpl/detail/templates.h"

namespace rw
{
namespace math
{
namespace vpl
{

namespace detail
{
    // This templated structure allows for multiple implementations of vector
    // shift depending on whether or not a shift by bytes and/or a shift by
    // bits is required.
    template <bool ByteShiftRequired, bool BitShiftRequired, int ShiftAmount>
    struct VecShiftLeftImmHelper
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0);
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<true, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = RWMATH_VPU_DETAIL_VecSplatS32(0);
            const VectorIntrinsic bitShiftVector = RWMATH_VPU_DETAIL_VecSpltisb(ShiftAmount & 7);
            VectorIntrinsic shiftedByBytes = RWMATH_VPU_DETAIL_VecSld(v0, zero, ShiftAmount/8);
            return RWMATH_VPU_DETAIL_VecSl( shiftedByBytes, bitShiftVector );
        }
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<true, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = RWMATH_VPU_DETAIL_VecSplatS32(0);
            return RWMATH_VPU_DETAIL_VecSld(v0, zero, ShiftAmount/8);
        }
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<false, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic bitShiftVector = RWMATH_VPU_DETAIL_VecSpltisb(ShiftAmount & 7);
            return RWMATH_VPU_DETAIL_VecSl( v0, bitShiftVector );
        }
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<false, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return v0;
        }
    };

    // This templated structure allows for multiple implementations of vector
    // shift depending on whether or not a shift by bytes and/or a shift by
    // bits is required.
    template <bool ByteShiftRequired, bool BitShiftRequired, int ShiftAmount>
    struct VecShiftRightImmHelper
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0);
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<true, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = RWMATH_VPU_DETAIL_VecSplatS32(0);
            const VectorIntrinsic bitShiftVector = RWMATH_VPU_DETAIL_VecSpltisb(ShiftAmount & 7);
            VectorIntrinsic shiftedByBytes = RWMATH_VPU_DETAIL_VecSld(zero, v0, 16-(ShiftAmount/8));
            return RWMATH_VPU_DETAIL_VecSr( shiftedByBytes, bitShiftVector );
        }
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<true, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = RWMATH_VPU_DETAIL_VecSplatS32(0);
            return RWMATH_VPU_DETAIL_VecSld(zero, v0, 16-(ShiftAmount/8));
        }
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<false, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic bitShiftVector = RWMATH_VPU_DETAIL_VecSpltisb(ShiftAmount & 7);
            return RWMATH_VPU_DETAIL_VecSr( v0, bitShiftVector );
        }
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<false, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return v0;
        }
    };

} // namespace detail

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_floor / vrfim
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecFloor(VectorIntrinsicInParam v0)
    {
        return RWMATH_VPU_DETAIL_VecRfim(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_loge / vlogefp
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLog2Est(VectorIntrinsicInParam v0)
    {
        return RWMATH_VPU_DETAIL_VecLogefp(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_nor / vnor
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecNor(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_perm / vperm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecPermute(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam permuteConstant)
    {
        return RWMATH_VPU_DETAIL_VecPerm(v0, v1, permuteConstant);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_lvlx / lvlx
    // VPL_PLATFORM_DOC This function makes use of a special load instruction (lvlx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadLeftAligned(const void *base, int offset)
    {
        return RWMATH_VPU_DETAIL_VecLvlx(offset, base);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_lvrx / lvrx
    // VPL_PLATFORM_DOC This function makes use of a special load instruction (lvrx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadRightAligned(const void *base, int offset)
    {
        return RWMATH_VPU_DETAIL_VecLvrx(offset, base);
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_DOC This function makes use of special load instructions (lvlx and lvrx)
    // that require memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset)
    {
        return RW_MATH_VPU_Or_R( RWMATH_VPU_DETAIL_VecLvlx( offset, base ), RWMATH_VPU_DETAIL_VecLvrx( offset + 16, base ) );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_mergeh / vmrghw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeH(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_mergeh / vmrghb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeHB(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_mergeh / vmrghh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeHH(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_mergel / vmrglw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeL(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_mergel / vmrglb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeLB(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_mergel / vmrglh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeLH(v0, v1);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0)
    {
        rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
        VectorIntrinsic result;
        RW_MATH_VPU_PermuteSingle_I(result, v0, PermConstant);
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_splat / vspltb
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ElementIndex, 0, 15 >();
        return RWMATH_VPU_DETAIL_VecSplatByte(v0, ElementIndex);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_splat / vsplth
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ElementIndex, 0, 7 >();
        return RWMATH_VPU_DETAIL_VecSplatHalf(v0, ElementIndex);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_stvlx / stvlx
    // VPL_PLATFORM_DOC This function makes use of a special store instruction (stvlx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.
    RW_MATH_FORCE_INLINE VPL_API void VecStoreLeftAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        RWMATH_VPU_DETAIL_VecStvlx( v0, offset, base );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_stvrx / stvrx
    // VPL_PLATFORM_DOC This function makes use of a special store instruction (stvrx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.
    RW_MATH_FORCE_INLINE VPL_API void VecStoreRightAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        RWMATH_VPU_DETAIL_VecStvrx( v0, offset, base );
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_DOC This function makes use of special store instructions (stvlx and stvrx)
    // that require memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.
    RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        RWMATH_VPU_DETAIL_VecStvlx( v0, offset, base );
        RWMATH_VPU_DETAIL_VecStvrx( v0, offset+16, base );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_ste / stvewx
    RW_MATH_FORCE_INLINE VPL_API void VecStoreWordElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        RWMATH_VPU_DETAIL_VecStvewx( v0, offset, base );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_ste / stvehx
    RW_MATH_FORCE_INLINE VPL_API void VecStoreHalfElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        RWMATH_VPU_DETAIL_VecStvehx( v0, offset, base );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_ste / stvebx
    RW_MATH_FORCE_INLINE VPL_API void VecStoreByteElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        RWMATH_VPU_DETAIL_VecStvebx( v0, offset, base );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_add / vaddubm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAddubm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sub / vsububm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSububm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_add / vaddubm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAddubm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sub / vsububm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSububm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_adds / vaddsbs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAddsbs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_subs / vsubsbs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubsbs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_adds / vaddubs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAddubs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_subs / vsububs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSububs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_add / vadduhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAdduhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sub / vsubuhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubuhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_add / vadduhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAdduhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sub / vsubuhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubuhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_adds / vaddshs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAddshs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_subs / vsubshs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubshs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_adds / vadduhs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAdduhs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_subs / vsubuhs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubuhs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_add / vadduwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAdduwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sub / vsubuwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubuwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_add / vadduwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAdduwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sub / vsubuwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubuwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_adds / vaddsws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAddsws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_subs / vsubsws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubsws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_adds / vadduws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecAdduws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_subs / vsubuws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecSubuws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cts / vctsxs
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return RWMATH_VPU_DETAIL_VecCtsxs( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_ctf / vcfsx
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return RWMATH_VPU_DETAIL_VecCfsx( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_ctu / vctuxs
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return RWMATH_VPU_DETAIL_VecCtuxs( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_ctf / vcfux
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return RWMATH_VPU_DETAIL_VecCfux( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_splat_s8 / vspltisb
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char()
    {
        detail::Assert_5BitSignedValue<Value>();
        return RWMATH_VPU_DETAIL_VecSpltisb(Value);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_splat_s32 / vspltisw
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int()
    {
        detail::Assert_5BitSignedValue<Value>();
        return RWMATH_VPU_DETAIL_VecSplatS32(Value);
    }

    // VPL_PLATFORM_RATING_2    
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int()
    {
        return RWMATH_VPU_DETAIL_VecSplat(RWMATH_VPU_DETAIL_VecLvlx(0, &(::rw::math::vpu::detail::IntSource<Value>::value)), 0);
        
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_splat_s16 / vspltish
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short()
    {
        detail::Assert_5BitSignedValue<Value>();
        return RWMATH_VPU_DETAIL_VecSpltish(Value);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Mask, int RotateLeft> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftWithMaskedInsertionImm(VectorIntrinsicInParam original, VectorIntrinsicInParam vecToInsert)
    {
        detail::Assert_ValueInRange< Mask, 0, 15 >();
        detail::Assert_ValueInRange< RotateLeft, 0, 3 >();
        VectorIntrinsic permConstant = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>3)&1 ) * ((RotateLeft+0)&3) + ( 1-((Mask>>3)&1)  ) * 4 ),
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>2)&1 ) * ((RotateLeft+1)&3) + ( 1-((Mask>>2)&1)  ) * 5 ),
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>1)&1 ) * ((RotateLeft+2)&3) + ( 1-((Mask>>1)&1)  ) * 6 ),
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>0)&1 ) * ((RotateLeft+3)&3) + ( 1-((Mask>>0)&1)  ) * 7 )
            );
        return RWMATH_VPU_DETAIL_VecPerm(vecToInsert, original, permConstant);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        const VectorIntrinsic shiftVec = RWMATH_VPU_DETAIL_VecSplatS32( (Shift<=15) ? Shift : (Shift-32));
        return RWMATH_VPU_DETAIL_VecSlw( v0, shiftVec );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        const VectorIntrinsic shiftVec = RWMATH_VPU_DETAIL_VecSplatS32( (Shift<=15) ? Shift : (Shift-32));
        return RWMATH_VPU_DETAIL_VecSrw( v0, shiftVec );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        const VectorIntrinsic shiftVec = RWMATH_VPU_DETAIL_VecSplatS32( (Shift<=15) ? Shift : (Shift-32));
        return RWMATH_VPU_DETAIL_VecSraw( v0, shiftVec );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 127 >();
        return detail::VecShiftLeftImmHelper< (Shift >= 8), ((Shift&7)!=0), Shift>::Shift(v0);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 127 >();
        return detail::VecShiftRightImmHelper< Shift >= 8, (Shift&7)!=0, Shift>::Shift(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_slo / vslo
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return RWMATH_VPU_DETAIL_VecSlo( v0, shiftAmount );
    }

	// VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sl / vsl
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return RWMATH_VPU_DETAIL_VecSl( v0, shiftAmount );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sro / vsro
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return RWMATH_VPU_DETAIL_VecSro( v0, shiftAmount );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sr / vsr
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return RWMATH_VPU_DETAIL_VecSr( v0, shiftAmount );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpeq / vcmpequw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpequw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpgt / vcmpgtuw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtuw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtuw( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpeq / vcmpequw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpequw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpgt / vcmpgtsw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtsw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtsw( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpeq / vcmpequh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpequh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpgt / vcmpgtuh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtuh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtuh( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpeq / vcmpequh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpequh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpgt / vcmpgtsh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtsh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtsh( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpeq / vcmpequb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpequb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpgt / vcmpgtub
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtub( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtub( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpeq / vcmpequb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpequb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_cmpgt / vcmpgtsb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtsb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecCmpgtsb( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_max / vmaxuw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMaxuw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMaxuh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_max / vmaxuh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMaxub( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_max / vmaxsw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMaxsw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_max / vmaxsh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMaxsh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_max / vmaxsb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMaxsb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_min / vminuw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMinuw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_min / vminuh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMinuh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_min / vminub
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMinub( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_min / vminsw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMinsw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_min / vminsh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMinsh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_min / vminsb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMinsb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sl / vslw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return RWMATH_VPU_DETAIL_VecSlw(v0, shiftCount);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sr / vsrw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return RWMATH_VPU_DETAIL_VecSrw(v0, shiftCount);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sl / vslw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return RWMATH_VPU_DETAIL_VecSlw(v0, shiftCount);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vec_sr / vsraw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return RWMATH_VPU_DETAIL_VecSraw(v0, shiftCount);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif
