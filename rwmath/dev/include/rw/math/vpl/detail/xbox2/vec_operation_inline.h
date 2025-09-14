// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_INLINE_H
#define RW_MATH_VEC_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

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
    template <bool ByteShiftRequired, bool BitShiftRequired>
    struct VecShiftLeftImmHelper
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0);
    };

    template<>
    struct VecShiftLeftImmHelper<true, true>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = __vspltisw(0);
            const VectorIntrinsic bitShiftVector = __vspltisb(Shift & 7);
            VectorIntrinsic shiftedByBytes = __vsldoi(v0, zero, Shift/8);
            return __vsl( shiftedByBytes, bitShiftVector );
        }
    };

    template<>
    struct VecShiftLeftImmHelper<true, false>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = __vspltisw(0);
            return __vsldoi(v0, zero, Shift/8);
        }
    };

    template<>
    struct VecShiftLeftImmHelper<false, true>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic bitShiftVector = __vspltisb(Shift & 7);
            return __vsl( v0, bitShiftVector );
        }
    };

    template<>
    struct VecShiftLeftImmHelper<false, false>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return v0;
        }
    };

    // This templated structure allows for multiple implementations of vector
    // shift depending on whether or not a shift by bytes and/or a shift by
    // bits is required.
    template <bool ByteShiftRequired, bool BitShiftRequired>
    struct VecShiftRightImmHelper
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0);
    };

    template<>
    struct VecShiftRightImmHelper<true, true>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = __vspltisw(0);
            const VectorIntrinsic bitShiftVector = __vspltisb(Shift & 7);
            VectorIntrinsic shiftedByBytes = __vsldoi(zero, v0, 16-(Shift/8));
            return __vsr( shiftedByBytes, bitShiftVector );
        }
    };

    template<>
    struct VecShiftRightImmHelper<true, false>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic zero = __vspltisw(0);
            return __vsldoi(zero, v0, 16-(Shift/8));
        }
    };

    template<>
    struct VecShiftRightImmHelper<false, true>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            const VectorIntrinsic bitShiftVector = __vspltisb(Shift & 7);
            return __vsr( v0, bitShiftVector );
        }
    };

    template<>
    struct VecShiftRightImmHelper<false, false>
    {
        template <int Shift>
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return v0;
        }
    };

} // namespace detail

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vrfim / vrfim
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecFloor(VectorIntrinsicInParam v0)
    {
        return __vrfim(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vlogefp / vlogefp
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLog2Est(VectorIntrinsicInParam v0)
    {
        return __vlogefp(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vnor / vnor
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vnor(v0, v1);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vperm / vperm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecPermute(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam permuteConstant)
    {
        return __vperm(v0, v1, permuteConstant);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __lvlx / lvlx
    // VPL_PLATFORM_DOC This function makes use of a special load instruction (lvlx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.  Please
    // see the Xenon documentation for more information.
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadLeftAligned(const void *base, int offset)
    {
        return __lvlx( base, offset );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __lvrx / lvrx
    // VPL_PLATFORM_DOC This function makes use of a special load instruction (lvrx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.  Please
    // see the Xenon documentation for more information.
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadRightAligned(const void *base, int offset)
    {
        return __lvrx( base, offset );
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_DOC This function makes use of special load instructions (lvlx and lvrx)
    // that require memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.  Please
    // see the Xenon documentation for more information.
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset)
    {
        return __vor( __lvlx( base, offset ), __lvrx( base, offset + 16 ) );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmrghw / vmrghw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmrghw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmrghb / vmrghb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmrghb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmrghh / vmrghh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmrghh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmrghw / vmrghw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmrglw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmrglb / vmrglb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmrglb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmrglh / vmrglh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmrglh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vpermwi / vpermwi
    VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0)
    {
        rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
        VectorIntrinsic result;
        RW_MATH_VPU_PermuteSingle_I(result, v0, PermConstant);
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vspltb / vspltb
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ElementIndex, 0, 15 >();
        return __vspltb( v0, ElementIndex );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsplth / vsplth
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ElementIndex, 0, 7 >();
        return __vsplth( v0, ElementIndex );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vstvlx / vstvlx
    // VPL_PLATFORM_DOC This function makes use of a special store instruction (stvlx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.  Please
    // see the Xenon documentation for more information.
    RW_MATH_FORCE_INLINE VPL_API void VecStoreLeftAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        __stvlx( v0, base, offset );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vstvrx / vstvrx
    // VPL_PLATFORM_DOC This function makes use of a special store instruction (stvrx)
    // that requires memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.  Please
    // see the Xenon documentation for more information.
    RW_MATH_FORCE_INLINE VPL_API void VecStoreRightAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        __stvrx( v0, base, offset );
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_DOC This function makes use of special store instructions (stvlx and stvrx)
    // that require memory to be cacheable.  If you attempt to use this
    // function on non-cacheable memory an exception will be generated.  Please
    // see the Xenon documentation for more information.
    RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        __stvlx( v0, base, offset );
        __stvrx( v0, base, offset+16 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __stvewx / stvewx
    RW_MATH_FORCE_INLINE VPL_API void VecStoreWordElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        __stvewx( v0, base, offset );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __stvehx / stvehx
    RW_MATH_FORCE_INLINE VPL_API void VecStoreHalfElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        __stvehx( v0, base, offset );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __stvebx / stvebx
    RW_MATH_FORCE_INLINE VPL_API void VecStoreByteElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        __stvebx( v0, base, offset );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vaddubm / vaddubm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vaddubm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsububm / vsububm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsububm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vaddubm / vaddubm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vaddubm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsububm / vsububm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsububm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vaddsbs / vaddsbs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vaddsbs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubsbs / vsubsbs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubsbs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vaddubs / vaddubs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vaddubs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsububs / vsububs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsububs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vadduhm / vadduhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vadduhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubuhm / vsubuhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubuhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vadduhm / vadduhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vadduhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubuhm / vsubuhm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubuhm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vaddshs / vaddshs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vaddshs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubshs / vsubshs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubshs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vadduhs / vadduhs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vadduhs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubuhs / vsubuhs
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubuhs( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vadduwm / vadduwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vadduwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubuwm / vsubuwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubuwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vadduwm / vadduwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vadduwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubuwm / vsubuwm
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubuwm( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vaddsws / vaddsws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vaddsws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubsws / vsubsws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubsws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vadduws / vadduws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vadduws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsubuws / vsubuws
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vsubuws( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vctsxs / vctsxs
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return __vctsxs( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcfsx / vcfsx
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return __vcfsx( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vctuxs / vctuxs
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return __vctuxs( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcfux / vcfux
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return __vcfux( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vspltisb / vspltisb
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char()
    {
        detail::Assert_5BitSignedValue<Value>();
        return __vspltisb(Value);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vspltisw / vspltisw
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int()
    {
        detail::Assert_5BitSignedValue<Value>();
        return __vspltisw(Value);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int()
    {
        return __vspltw(__lvlx(&(::rw::math::vpu::detail::IntSource<Value>::value), 0), 0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vspltish / vspltish
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short()
    {
        detail::Assert_5BitSignedValue<Value>();
        return __vspltish(Value);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vrlimi / vrlimi
    VPL_API template<int Mask, int RotateLeft> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftWithMaskedInsertionImm(VectorIntrinsicInParam original, VectorIntrinsicInParam vecToInsert)
    {
        detail::Assert_ValueInRange< Mask, 0, 15 >();
        detail::Assert_ValueInRange< RotateLeft, 0, 3 >();
        return __vrlimi( original, vecToInsert, Mask, RotateLeft );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        const VectorIntrinsic shiftVec = __vspltisw( (Shift<=15) ? Shift : (Shift-32));
        return __vslw( v0, shiftVec );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        const VectorIntrinsic shiftVec = __vspltisw( (Shift<=15) ? Shift : (Shift-32));
        return __vsrw( v0, shiftVec );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        const VectorIntrinsic shiftVec = __vspltisw( (Shift<=15) ? Shift : (Shift-32));
        return __vsraw( v0, shiftVec );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 127 >();
        return detail::VecShiftLeftImmHelper< Shift >= 8, (Shift&7)!=0>::Shift<Shift>(v0);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 127 >();
        return detail::VecShiftRightImmHelper< Shift >= 8, (Shift&7)!=0>::Shift<Shift>(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vslo / vslo
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return __vslo( v0, shiftAmount );
    }

	// VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsl / vsl
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return __vsl( v0, shiftAmount );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsro / vsro
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return __vsro( v0, shiftAmount );
    }

	// VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsr / vsr
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return __vsr( v0, shiftAmount );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpequw / vcmpequw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpequw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpgtuw / vcmpgtuw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtuw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtuw( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpequw / vcmpequw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpequw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpgtsw / vcmpgtsw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtsw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtsw( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpequh / vcmpequh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpequh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpgtuh / vcmpgtuh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtuh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtuh( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpequh / vcmpequh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpequh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpgtsh / vcmpgtsh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtsh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtsh( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpequb / vcmpequb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpequb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpgtub / vcmpgtub
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtub( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtub( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpequb / vcmpequb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpequb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vcmpgtsb / vcmpgtsb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtsb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vcmpgtsb( v1, v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmaxuw / vmaxuw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmaxuw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmaxuh / vmaxuh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmaxuh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmaxub / vmaxub
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmaxub( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmaxsw / vmaxsw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmaxsw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmaxsh / vmaxsh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmaxsh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vmaxsb / vmaxsb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vmaxsb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vminuw / vminuw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vminuw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vminuh / vminuh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vminuh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vminub / vminub
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vminub( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vminsw / vminsw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vminsw( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vminsh / vminsh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vminsh( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vminsb / vminsb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return __vminsb( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vslw / vslw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return __vslw(v0, shiftCount);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsrw / vsrw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return __vsrw(v0, shiftCount);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vslw / vslw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return __vslw(v0, shiftCount);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __vsraw / vsraw
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return __vsraw(v0, shiftCount);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif
