// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_INLINE_H
#define RW_MATH_VEC_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT _mm_loadu_ps
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset)
    {
        const char *address = static_cast<const char *>(base) + offset;
        return _mm_loadu_ps( reinterpret_cast<const float*>(address) );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_unpacklo_ps
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_unpacklo_ps( v0, v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_unpackhi_ps
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_unpackhi_ps( v0, v1 );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_xor_ps(::rw::math::vpu::detail::gXYZWMask, _mm_or_ps(v0, v1) ); 
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_shuffle_ps
    VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0)
    {
        rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
        VectorIntrinsic result;
        RW_MATH_VPU_PermuteSingle_I(result, v0, PermConstant);
        return result;
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT _mm_storeu_ps
    RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = static_cast<char *>(base) + offset;
        _mm_storeu_ps( reinterpret_cast<float*>(address), v0 );
    }

#if VPL_SUPPORTED_PLATFORM( VPL_SSE2 )

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_add_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_add_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_sub_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_sub_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_add_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_add_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_sub_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_sub_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_adds_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_adds_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_subs_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_subs_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_adds_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_adds_epu8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_subs_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_subs_epu8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_add_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_add_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_sub_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_sub_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_add_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_add_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_sub_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_sub_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_adds_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_adds_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_subs_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_subs_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_adds_epu16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_adds_epu16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_subs_epu16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_subs_epu16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_add_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_add_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_sub_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_sub_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_add_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_add_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_sub_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_sub_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT _mm_cvttps_epi32
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();

        using namespace rw::math::vpl::detail;
        VectorIntrinsic scaledV0 = _mm_mul_ps( v0, ScaleExponentArray[ScaleExponent] );
        scaledV0 = _mm_max_ps( scaledV0, vecNegTwo31Float );
        VectorIntrinsic overflow = _mm_cmpge_ps( scaledV0, vecTwo31Float );
        return _mm_xor_ps( _mm_castsi128_ps(_mm_cvttps_epi32( scaledV0 )), overflow );
    }

    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT _mm_cvtepi32_ps
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();

        using namespace rw::math::vpl::detail;
        VectorIntrinsicSSEUnion r, v0sse2;
        v0sse2.vector = v0;
        VectorIntrinsic scale = _mm_div_ps( vecOne, ScaleExponentArray[ScaleExponent] );
        r.vector = _mm_cvtepi32_ps( v0sse2.vectorSSE2Int );
        r.vector = _mm_mul_ps( r.vector, scale );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();

        using namespace rw::math::vpl::detail;
        VectorIntrinsicSSEUnion overflow, overflowU, addval, result, v0sse2;
        v0sse2.vector = v0;
        v0sse2.vector = _mm_mul_ps( v0sse2.vector, ScaleExponentArray[ScaleExponent] );

        //check for overflow before conversion to int
        overflow.vector = _mm_cmpge_ps( v0, vecTwo31Float );
        overflowU.vector = _mm_cmpge_ps( v0, vecTwo32Float );
        VectorIntrinsic subval = _mm_and_ps( overflow.vector, vecTwo31Float );
        addval.vectorSSE2Int = _mm_slli_epi32( overflow.vectorSSE2Int, 31 );

        //bias the value to signed space if it is >= 2**31
        v0sse2.vector = _mm_sub_ps( v0sse2.vector, subval );
        //clip at zero
        v0sse2.vector = _mm_max_ps( v0sse2.vector, vecZero);

        result.vectorSSE2Int = _mm_cvttps_epi32( v0sse2.vector );
        // unbias
        result.vectorSSE2Int = _mm_add_epi32( result.vectorSSE2Int, addval.vectorSSE2Int );
        // patch up the overflow case
        result.vectorSSE2Int = _mm_or_si128( result.vectorSSE2Int, overflowU.vectorSSE2Int );

        return result.vector;
    }

    // VPL_PLATFORM_RATING_3
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();

        using namespace rw::math::vpl::detail;
        //Avoid double rounding by doing two exact conversions
        //of high and low 16-bit segments
        VectorIntrinsicSSEUnion r, v0sse2, hi16, low16;
        v0sse2.vector = v0;
        hi16.vectorSSE2Int = _mm_srli_epi32( v0sse2.vectorSSE2Int, 16 );
        low16.vectorSSE2Int = _mm_srli_epi32( _mm_slli_epi32( v0sse2.vectorSSE2Int, 16 ), 16 );
        VectorIntrinsic hi16_float = _mm_mul_ps( _mm_cvtepi32_ps( hi16.vectorSSE2Int ), maxHalfFloatFromUnsignedInteger );
        VectorIntrinsic low16_float = _mm_cvtepi32_ps( low16.vectorSSE2Int );
        r.vector = _mm_add_ps( hi16_float, low16_float );

        VectorIntrinsic scale = _mm_div_ps( vecOne, ScaleExponentArray[ScaleExponent] );
        return _mm_mul_ps( r.vector, scale );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_set1_epi8
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char()
    {
        (void)detail::Assert_5BitSignedValue<Value>();
        
        return _mm_castsi128_ps(_mm_set1_epi8( (char)Value ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_set1_epi32
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int()
    {
        (void)detail::Assert_5BitSignedValue<Value>();
        
        return _mm_castsi128_ps(_mm_set1_epi32( (char)Value ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_set1_epi32
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int()
    {
        return _mm_castsi128_ps(_mm_set1_epi32( Value ));
    }


    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_set1_epi16
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short()
    {
        (void)detail::Assert_5BitSignedValue<Value>();

        return _mm_castsi128_ps(_mm_set1_epi16( (char)Value ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_slli_epi32
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< Shift, 0, 31 >();

        return _mm_castsi128_ps(_mm_slli_epi32( _mm_castps_si128(v0), Shift ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_srli_epi32
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< Shift, 0, 31 >();

        return _mm_castsi128_ps(_mm_srli_epi32( _mm_castps_si128(v0), Shift ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_srai_epi32
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< Shift, 0, 31 >();

        return _mm_castsi128_ps(_mm_srai_epi32( _mm_castps_si128(v0), Shift ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpeq_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpeq_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpeq_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpeq_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpgt_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpgt_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmplt_epi32
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmplt_epi32( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpeq_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpeq_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpeq_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpeq_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpgt_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpgt_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmplt_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmplt_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpeq_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpeq_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpeq_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpeq_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmpgt_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmpgt_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_cmplt_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_cmplt_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1) ));
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, delta;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        delta.vectorSSE2Int = _mm_set1_epi32( (-2147483647 - 1) ); // 0x80000000
        v0sse2.vectorSSE2Int = _mm_add_epi32( v0sse2.vectorSSE2Int, delta.vectorSSE2Int );
        v1sse2.vectorSSE2Int = _mm_add_epi32( v1sse2.vectorSSE2Int, delta.vectorSSE2Int );
        r.vectorSSE2Int = _mm_cmpgt_epi32( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        return r.vector;
    }
    
    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, delta;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        delta.vectorSSE2Int = _mm_set1_epi32( (-2147483647 - 1) );
        v0sse2.vectorSSE2Int = _mm_add_epi32( v0sse2.vectorSSE2Int, delta.vectorSSE2Int );
        v1sse2.vectorSSE2Int = _mm_add_epi32( v1sse2.vectorSSE2Int, delta.vectorSSE2Int );
        r.vectorSSE2Int = _mm_cmplt_epi32( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, delta;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        delta.vectorSSE2Int = _mm_set1_epi16( (-32768) ); // 0x8000
        v0sse2.vectorSSE2Int = _mm_add_epi16( v0sse2.vectorSSE2Int, delta.vectorSSE2Int );
        v1sse2.vectorSSE2Int = _mm_add_epi16( v1sse2.vectorSSE2Int, delta.vectorSSE2Int );
        r.vectorSSE2Int = _mm_cmpgt_epi16( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, delta;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        delta.vectorSSE2Int = _mm_set1_epi16( (-32768) ); // 0x8000
        v0sse2.vectorSSE2Int = _mm_add_epi16( v0sse2.vectorSSE2Int, delta.vectorSSE2Int );
        v1sse2.vectorSSE2Int = _mm_add_epi16( v1sse2.vectorSSE2Int, delta.vectorSSE2Int );
        r.vectorSSE2Int = _mm_cmplt_epi16( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, maxsse2;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        maxsse2.vectorSSE2Int = _mm_max_epu8( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_cmpeq_epi8( maxsse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_xor_si128( r.vectorSSE2Int, _mm_set1_epi8( -1 ) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, maxsse2;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        maxsse2.vectorSSE2Int = _mm_min_epu8( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_cmpeq_epi8( maxsse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_xor_si128( r.vectorSSE2Int, _mm_set1_epi8( -1 ) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vectorSSE2Int = _mm_cmpgt_epi32( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_andnot_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_and_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vectorSSE2Int = _mm_cmpgt_epi32( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_max_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_max_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_min_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_min_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_max_epu8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_max_epu8( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vectorSSE2Int = _mm_cmpgt_epi8( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_andnot_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_and_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_min_epu8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_min_epu8( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vectorSSE2Int = _mm_cmpgt_epi8( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vector = VecCompGreaterThan_UInt( v0, v1 );
        r.vectorSSE2Int = _mm_or_si128( _mm_andnot_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_and_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vector = VecCompGreaterThan_UInt( v0, v1 );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vector = VecCompGreaterThan_UShort( v0, v1 );
        r.vectorSSE2Int = _mm_or_si128( _mm_andnot_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_and_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        cmpResult.vector = VecCompGreaterThan_UShort( v0, v1 );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, int_zero, int_min, int_max, subval, cmpResult, cmpResultN, cmpResultP, negInt, posInt;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        int_zero.vectorSSE2Int = _mm_set1_epi32( 0 );
        int_min.vectorSSE2Int = _mm_set1_epi32( (-2147483647 - 1) );
        int_max.vectorSSE2Int = _mm_set1_epi32( 2147483647 );

        cmpResult.vector = VecCompGreaterThan_Int( int_zero.vector, v0sse2.vector );
        negInt.vectorSSE2Int = _mm_and_si128( cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int );
        posInt.vectorSSE2Int = _mm_andnot_si128( cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int );

        subval.vectorSSE2Int = _mm_sub_epi32( int_min.vectorSSE2Int, negInt.vectorSSE2Int );
        cmpResultN.vector = VecCompGreaterThan_Int( subval.vector, v1sse2.vector );
        negInt.vectorSSE2Int = _mm_and_si128( cmpResultN.vectorSSE2Int, int_min.vectorSSE2Int );

        subval.vectorSSE2Int = _mm_sub_epi32( int_max.vectorSSE2Int, posInt.vectorSSE2Int );
        cmpResultP.vector = VecCompGreaterThan_Int( v1sse2.vector, subval.vector );
        posInt.vectorSSE2Int = _mm_and_si128( cmpResultP.vectorSSE2Int, int_max.vectorSSE2Int );

        cmpResult.vectorSSE2Int = _mm_or_si128( cmpResultN.vectorSSE2Int, cmpResultP.vectorSSE2Int );

        r.vectorSSE2Int = _mm_add_epi32 ( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_or_si128(negInt.vectorSSE2Int, posInt.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, r.vectorSSE2Int) );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, int_zero, int_min, int_max, addval, cmpResult, cmpResultN, cmpResultP, negInt, posInt;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        int_zero.vectorSSE2Int = _mm_set1_epi32( 0 );
        int_min.vectorSSE2Int = _mm_set1_epi32( (-2147483647 - 1) );
        int_max.vectorSSE2Int = _mm_set1_epi32( 2147483647 );

        cmpResult.vector = VecCompGreaterThan_Int( int_zero.vector, v1sse2.vector );

        addval.vectorSSE2Int = _mm_add_epi32( int_min.vectorSSE2Int, _mm_andnot_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int) );
        cmpResultN.vector = VecCompGreaterThan_Int( addval.vector, v0sse2.vector );
        negInt.vectorSSE2Int = _mm_and_si128( cmpResultN.vectorSSE2Int, int_min.vectorSSE2Int );

        addval.vectorSSE2Int = _mm_add_epi32( int_max.vectorSSE2Int, _mm_and_si128(cmpResult.vectorSSE2Int, v1sse2.vectorSSE2Int) );
        cmpResultP.vector = VecCompGreaterThan_Int( v0sse2.vector, addval.vector );
        posInt.vectorSSE2Int = _mm_and_si128( cmpResultP.vectorSSE2Int, int_max.vectorSSE2Int );

        cmpResult.vectorSSE2Int = _mm_or_si128( cmpResultN.vectorSSE2Int, cmpResultP.vectorSSE2Int );

        r.vectorSSE2Int = _mm_sub_epi32 ( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_or_si128(negInt.vectorSSE2Int, posInt.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, r.vectorSSE2Int) );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, uint_max, subval, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        uint_max.vectorSSE2Int = _mm_set1_epi32( 0xffffffff );
        subval.vectorSSE2Int = _mm_sub_epi32( uint_max.vectorSSE2Int, v0sse2.vectorSSE2Int );
        cmpResult.vector = VecCompGreaterThan_UInt( v1sse2.vector, subval.vector );
        r.vectorSSE2Int = _mm_add_epi32 ( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, uint_max.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, r.vectorSSE2Int) );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsicSSEUnion r, v0sse2, v1sse2, uint_min, cmpResult;
        v0sse2.vector = v0;
        v1sse2.vector = v1;
        uint_min.vectorSSE2Int = _mm_set1_epi32( 0 );

        cmpResult.vector = VecCompGreaterThan_UInt( v1sse2.vector, v0sse2.vector );
        r.vectorSSE2Int = _mm_sub_epi32 ( v0sse2.vectorSSE2Int, v1sse2.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, uint_min.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, r.vectorSSE2Int) );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ElementIndex, 0, 7 >();
        VectorIntrinsicSSEUnion r, v0sse2;
        v0sse2.vector = v0;
        int element = _mm_extract_epi16( v0sse2.vectorSSE2Int, ElementIndex );
        r.vectorSSE2Int = _mm_set1_epi16( (short) element );
        return r.vector;
    }
    
    // VPL_PLATFORM_RATING_3
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ElementIndex, 0, 15 >();
        VectorIntrinsicSSEUnion r, v0sse2, shiftedVal;
        v0sse2.vector = v0;
        shiftedVal.vectorSSE2Int = _mm_srli_si128( v0sse2.vectorSSE2Int, ElementIndex );
        shiftedVal.vectorSSE2Int = _mm_and_si128( shiftedVal.vectorSSE2Int, _mm_set1_epi32(0x000000ff) );
        int element = _mm_cvtsi128_si32( shiftedVal.vectorSSE2Int );
        r.vectorSSE2Int = _mm_set1_epi8( (char) element );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        VectorIntrinsicSSEUnion r, shiftCountDup0, shiftCountDup1, shiftCountDup2, shiftCountDup3, v0p0, v0p1, v0p2, v0p3;
        shiftCountDup0.vector = shiftCount;
        shiftCountDup1.vector = shiftCount;
        shiftCountDup2.vector = shiftCount;
        shiftCountDup3.vector = shiftCount;

        // shift the element 0
        v0p0.vector = VecSplat_Word<0>(v0);
        v0p0.vectorSSE2Int = _mm_srli_si128( v0p0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_slli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_srli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        v0p0.vectorSSE2Int = _mm_sll_epi32( v0p0.vectorSSE2Int, shiftCountDup0.vectorSSE2Int );
        
        // shift the element 1
        v0p1.vector = VecSplat_Word<1>(v0);
        v0p1.vectorSSE2Int = _mm_srli_si128( v0p1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_slli_si128( v0p1.vectorSSE2Int, 4 );
        shiftCountDup1.vectorSSE2Int = _mm_slli_si128( shiftCountDup1.vectorSSE2Int, 8 );
        shiftCountDup1.vectorSSE2Int = _mm_srli_si128( shiftCountDup1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_sll_epi32( v0p1.vectorSSE2Int, shiftCountDup1.vectorSSE2Int );

        // shift the element 2
        v0p2.vector = VecSplat_Word<2>(v0);
        v0p2.vectorSSE2Int = _mm_srli_si128( v0p2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_slli_si128( v0p2.vectorSSE2Int, 8 );
        shiftCountDup2.vectorSSE2Int = _mm_slli_si128( shiftCountDup2.vectorSSE2Int, 4 );
        shiftCountDup2.vectorSSE2Int = _mm_srli_si128( shiftCountDup2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_sll_epi32( v0p2.vectorSSE2Int, shiftCountDup2.vectorSSE2Int );

        // shift the element 3
        v0p3.vector = VecSplat_Word<3>(v0);
        v0p3.vectorSSE2Int = _mm_srli_si128( v0p3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_slli_si128( v0p3.vectorSSE2Int, 12 );
        shiftCountDup3.vectorSSE2Int = _mm_srli_si128( shiftCountDup3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_sll_epi32( v0p3.vectorSSE2Int, shiftCountDup3.vectorSSE2Int );

        r.vectorSSE2Int = _mm_or_si128( _mm_or_si128(v0p0.vectorSSE2Int, v0p1.vectorSSE2Int), _mm_or_si128(v0p2.vectorSSE2Int, v0p3.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        VectorIntrinsicSSEUnion r, shiftCountDup0, shiftCountDup1, shiftCountDup2, shiftCountDup3, v0p0, v0p1, v0p2, v0p3;
        shiftCountDup0.vector = shiftCount;
        shiftCountDup1.vector = shiftCount;
        shiftCountDup2.vector = shiftCount;
        shiftCountDup3.vector = shiftCount;

        // shift the element 0
        v0p0.vector = VecSplat_Word<0>(v0);
        v0p0.vectorSSE2Int = _mm_srli_si128( v0p0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_slli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_srli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        v0p0.vectorSSE2Int = _mm_srl_epi32( v0p0.vectorSSE2Int, shiftCountDup0.vectorSSE2Int );
        
        // shift the element 1
        v0p1.vector = VecSplat_Word<1>(v0);
        v0p1.vectorSSE2Int = _mm_srli_si128( v0p1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_slli_si128( v0p1.vectorSSE2Int, 4 );
        shiftCountDup1.vectorSSE2Int = _mm_slli_si128( shiftCountDup1.vectorSSE2Int, 8 );
        shiftCountDup1.vectorSSE2Int = _mm_srli_si128( shiftCountDup1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_srl_epi32( v0p1.vectorSSE2Int, shiftCountDup1.vectorSSE2Int );

        // shift the element 2
        v0p2.vector = VecSplat_Word<2>(v0);
        v0p2.vectorSSE2Int = _mm_srli_si128( v0p2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_slli_si128( v0p2.vectorSSE2Int, 8 );
        shiftCountDup2.vectorSSE2Int = _mm_slli_si128( shiftCountDup2.vectorSSE2Int, 4 );
        shiftCountDup2.vectorSSE2Int = _mm_srli_si128( shiftCountDup2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_srl_epi32( v0p2.vectorSSE2Int, shiftCountDup2.vectorSSE2Int );

        // shift the element 3
        v0p3.vector = VecSplat_Word<3>(v0);
        v0p3.vectorSSE2Int = _mm_srli_si128( v0p3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_slli_si128( v0p3.vectorSSE2Int, 12 );
        shiftCountDup3.vectorSSE2Int = _mm_srli_si128( shiftCountDup3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_srl_epi32( v0p3.vectorSSE2Int, shiftCountDup3.vectorSSE2Int );

        r.vectorSSE2Int = _mm_or_si128( _mm_or_si128(v0p0.vectorSSE2Int, v0p1.vectorSSE2Int), _mm_or_si128(v0p2.vectorSSE2Int, v0p3.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        VectorIntrinsicSSEUnion r, shiftCountDup0, shiftCountDup1, shiftCountDup2, shiftCountDup3, v0p0, v0p1, v0p2, v0p3;
        shiftCountDup0.vector = shiftCount;
        shiftCountDup1.vector = shiftCount;
        shiftCountDup2.vector = shiftCount;
        shiftCountDup3.vector = shiftCount;

        // shift the element 0
        v0p0.vector = VecSplat_Word<0>(v0);
        v0p0.vectorSSE2Int = _mm_srli_si128( v0p0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_slli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_srli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        v0p0.vectorSSE2Int = _mm_sll_epi32( v0p0.vectorSSE2Int, shiftCountDup0.vectorSSE2Int );
        
        // shift the element 1
        v0p1.vector = VecSplat_Word<1>(v0);
        v0p1.vectorSSE2Int = _mm_srli_si128( v0p1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_slli_si128( v0p1.vectorSSE2Int, 4 );
        shiftCountDup1.vectorSSE2Int = _mm_slli_si128( shiftCountDup1.vectorSSE2Int, 8 );
        shiftCountDup1.vectorSSE2Int = _mm_srli_si128( shiftCountDup1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_sll_epi32( v0p1.vectorSSE2Int, shiftCountDup1.vectorSSE2Int );

        // shift the element 2
        v0p2.vector = VecSplat_Word<2>(v0);
        v0p2.vectorSSE2Int = _mm_srli_si128( v0p2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_slli_si128( v0p2.vectorSSE2Int, 8 );
        shiftCountDup2.vectorSSE2Int = _mm_slli_si128( shiftCountDup2.vectorSSE2Int, 4 );
        shiftCountDup2.vectorSSE2Int = _mm_srli_si128( shiftCountDup2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_sll_epi32( v0p2.vectorSSE2Int, shiftCountDup2.vectorSSE2Int );

        // shift the element 3
        v0p3.vector = VecSplat_Word<3>(v0);
        v0p3.vectorSSE2Int = _mm_srli_si128( v0p3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_slli_si128( v0p3.vectorSSE2Int, 12 );
        shiftCountDup3.vectorSSE2Int = _mm_srli_si128( shiftCountDup3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_sll_epi32( v0p3.vectorSSE2Int, shiftCountDup3.vectorSSE2Int );

        r.vectorSSE2Int = _mm_or_si128( _mm_or_si128(v0p0.vectorSSE2Int, v0p1.vectorSSE2Int), _mm_or_si128(v0p2.vectorSSE2Int, v0p3.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        VectorIntrinsicSSEUnion r, shiftCountDup0, shiftCountDup1, shiftCountDup2, shiftCountDup3, v0p0, v0p1, v0p2, v0p3;
        shiftCountDup0.vector = shiftCount;
        shiftCountDup1.vector = shiftCount;
        shiftCountDup2.vector = shiftCount;
        shiftCountDup3.vector = shiftCount;

        // shift the element 0
        v0p0.vector = VecSplat_Word<0>(v0);
        v0p0.vectorSSE2Int = _mm_srli_si128( v0p0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_slli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        shiftCountDup0.vectorSSE2Int = _mm_srli_si128( shiftCountDup0.vectorSSE2Int, 12 );
        v0p0.vectorSSE2Int = _mm_sra_epi32( v0p0.vectorSSE2Int, shiftCountDup0.vectorSSE2Int );
        
        // shift the element 1
        v0p1.vector = VecSplat_Word<1>(v0);
        v0p1.vectorSSE2Int = _mm_srli_si128( v0p1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_slli_si128( v0p1.vectorSSE2Int, 4 );
        shiftCountDup1.vectorSSE2Int = _mm_slli_si128( shiftCountDup1.vectorSSE2Int, 8 );
        shiftCountDup1.vectorSSE2Int = _mm_srli_si128( shiftCountDup1.vectorSSE2Int, 12 );
        v0p1.vectorSSE2Int = _mm_sra_epi32( v0p1.vectorSSE2Int, shiftCountDup1.vectorSSE2Int );

        // shift the element 2
        v0p2.vector = VecSplat_Word<2>(v0);
        v0p2.vectorSSE2Int = _mm_srli_si128( v0p2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_slli_si128( v0p2.vectorSSE2Int, 8 );
        shiftCountDup2.vectorSSE2Int = _mm_slli_si128( shiftCountDup2.vectorSSE2Int, 4 );
        shiftCountDup2.vectorSSE2Int = _mm_srli_si128( shiftCountDup2.vectorSSE2Int, 12 );
        v0p2.vectorSSE2Int = _mm_sra_epi32( v0p2.vectorSSE2Int, shiftCountDup2.vectorSSE2Int );

        // shift the element 3
        v0p3.vector = VecSplat_Word<3>(v0);
        v0p3.vectorSSE2Int = _mm_srli_si128( v0p3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_slli_si128( v0p3.vectorSSE2Int, 12 );
        shiftCountDup3.vectorSSE2Int = _mm_srli_si128( shiftCountDup3.vectorSSE2Int, 12 );
        v0p3.vectorSSE2Int = _mm_sra_epi32( v0p3.vectorSSE2Int, shiftCountDup3.vectorSSE2Int );

        r.vectorSSE2Int = _mm_or_si128( _mm_or_si128(v0p0.vectorSSE2Int, v0p1.vectorSSE2Int), _mm_or_si128(v0p2.vectorSSE2Int, v0p3.vectorSSE2Int) );
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    VPL_API template<int Mask, int RotateLeft> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftWithMaskedInsertionImm(VectorIntrinsicInParam original, VectorIntrinsicInParam vecToInsert)
    {
        (void)detail::Assert_ValueInRange< Mask, 0, 15 >();
        (void)detail::Assert_ValueInRange< RotateLeft, 0, 3 >();

        using namespace rw::math::vpl::detail;
        VectorIntrinsicSSEUnion r, originalSSE2, vecToInsertRotated, vecToInsertL, vecToInsertR, maskSSE2;
        originalSSE2.vector = original;
        vecToInsertL.vector = vecToInsert;
        vecToInsertR.vector = vecToInsert;

        vecToInsertL.vectorSSE2Int = _mm_srli_si128( vecToInsertL.vectorSSE2Int, RotateLeft * 4);
        vecToInsertR.vectorSSE2Int = _mm_slli_si128( vecToInsertR.vectorSSE2Int, (4 - RotateLeft) * 4);
        vecToInsertRotated.vectorSSE2Int = _mm_or_si128( vecToInsertL.vectorSSE2Int, vecToInsertR.vectorSSE2Int );

        maskSSE2.vectorSSE2Int = _mm_load_si128( reinterpret_cast < const __m128i * > (&InsertionMaskArray[Mask]) );

        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(maskSSE2.vectorSSE2Int, vecToInsertRotated.vectorSSE2Int), _mm_andnot_si128(maskSSE2.vectorSSE2Int, originalSSE2.vectorSSE2Int) );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_unpacklo_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_unpacklo_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_unpacklo_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_unpacklo_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_unpackhi_epi8
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_unpackhi_epi8( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT _mm_unpackhi_epi16
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return _mm_castsi128_ps(_mm_unpackhi_epi16( _mm_castps_si128(v0), _mm_castps_si128(v1)));
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        using namespace rw::math::vpl::detail;
        VectorIntrinsicSSEUnion r, v0sse2L, v0sse2R, v64sse2, vShiftAmount, shiftAmountSSE2, cmpResult, shiftDelta, zeroSSE2;
        v0sse2L.vector = v0;
        v0sse2R.vector = v0;
        vShiftAmount.vector = shiftAmount;
        zeroSSE2.vectorSSE2Int = _mm_setzero_si128();

        v64sse2.vectorSSE2Int = _mm_load_si128( reinterpret_cast < const __m128i * > (&sixtyFourUInt) );
        vShiftAmount.vectorSSE2Int = _mm_srli_epi32( vShiftAmount.vectorSSE2Int, 3 );
        vShiftAmount.vectorSSE2Int = _mm_slli_epi32( vShiftAmount.vectorSSE2Int, 3 );
        cmpResult.vectorSSE2Int = _mm_cmpgt_epi32( vShiftAmount.vectorSSE2Int, v64sse2.vectorSSE2Int );
        shiftAmountSSE2.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v64sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, vShiftAmount.vectorSSE2Int) );

        __m128i countMask = _mm_setr_epi8(-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        __m128i _64MinusShift = _mm_sub_epi32(v64sse2.vectorSSE2Int, shiftAmountSSE2.vectorSSE2Int);

        v0sse2L.vectorSSE2Int = _mm_srl_epi64( v0sse2L.vectorSSE2Int, _mm_and_si128( shiftAmountSSE2.vectorSSE2Int, countMask ) );
        v0sse2R.vectorSSE2Int = _mm_sll_epi64( v0sse2R.vectorSSE2Int, _mm_and_si128( _64MinusShift, countMask ) );
        r.vectorSSE2Dbl = _mm_shuffle_pd( v0sse2R.vectorSSE2Dbl, zeroSSE2.vectorSSE2Dbl, _MM_SHUFFLE2(0,1) );
        r.vectorSSE2Int = _mm_or_si128( v0sse2L.vectorSSE2Int, r.vectorSSE2Int );

        shiftDelta.vectorSSE2Int = _mm_sub_epi32( vShiftAmount.vectorSSE2Int, v64sse2.vectorSSE2Int );
        shiftAmountSSE2.vectorSSE2Int =  _mm_set1_epi32( 0 );
        shiftAmountSSE2.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, shiftDelta.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, shiftAmountSSE2.vectorSSE2Int) );
        shiftAmountSSE2.vectorSSE2Int = _mm_srli_si128( shiftAmountSSE2.vectorSSE2Int, 12 );

        r.vectorSSE2Int = _mm_srl_epi64( r.vectorSSE2Int, shiftAmountSSE2.vectorSSE2Int );

        return r.vector;
    }
    
    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        using namespace rw::math::vpl::detail;
        VectorIntrinsicSSEUnion r, v0sse2L, v0sse2R, v64sse2, vShiftAmount, shiftAmountSSE2, cmpResult, shiftDelta, zeroSSE2;
        v0sse2L.vector = v0;
        v0sse2R.vector = v0;
        vShiftAmount.vector = shiftAmount;
        zeroSSE2.vectorSSE2Int = _mm_setzero_si128();

        v64sse2.vectorSSE2Int = _mm_load_si128( reinterpret_cast < const __m128i * > (&sixtyFourUInt) );
        vShiftAmount.vectorSSE2Int = _mm_srli_epi32( vShiftAmount.vectorSSE2Int, 3 );
        vShiftAmount.vectorSSE2Int = _mm_slli_epi32( vShiftAmount.vectorSSE2Int, 3 );
        cmpResult.vectorSSE2Int = _mm_cmpgt_epi32( vShiftAmount.vectorSSE2Int, v64sse2.vectorSSE2Int );
        shiftAmountSSE2.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v64sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, vShiftAmount.vectorSSE2Int) );

        __m128i countMask = _mm_setr_epi8(-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        __m128i _64MinusShift = _mm_sub_epi32(v64sse2.vectorSSE2Int, shiftAmountSSE2.vectorSSE2Int);

        v0sse2L.vectorSSE2Int = _mm_sll_epi64( v0sse2L.vectorSSE2Int, _mm_and_si128( shiftAmountSSE2.vectorSSE2Int, countMask ) );
        v0sse2R.vectorSSE2Int = _mm_srl_epi64( v0sse2R.vectorSSE2Int, _mm_and_si128( _64MinusShift, countMask ) );
        r.vectorSSE2Dbl = _mm_shuffle_pd( zeroSSE2.vectorSSE2Dbl, v0sse2R.vectorSSE2Dbl, _MM_SHUFFLE2(0,1) );
        r.vectorSSE2Int = _mm_or_si128( v0sse2L.vectorSSE2Int, r.vectorSSE2Int );

        shiftDelta.vectorSSE2Int = _mm_sub_epi32( vShiftAmount.vectorSSE2Int, v64sse2.vectorSSE2Int );
        shiftAmountSSE2.vectorSSE2Int =  _mm_set1_epi32( 0 );
        shiftAmountSSE2.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, shiftDelta.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, shiftAmountSSE2.vectorSSE2Int) );
        shiftAmountSSE2.vectorSSE2Int = _mm_srli_si128( shiftAmountSSE2.vectorSSE2Int, 12 );

        r.vectorSSE2Int = _mm_sll_epi64( r.vectorSSE2Int, shiftAmountSSE2.vectorSSE2Int );

        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecFloor(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpl::detail;
        VectorIntrinsicSSEUnion r, v0sse2, v0MinusOne, cmpResult, cmpResultNP, cmpResultN, cmpResultP, vecNegNaNSSE2, vecPosNaNSSE2, vecNaNSSE2;
        v0sse2.vector = v0;
        vecNegNaNSSE2.vector = vecNegNaN;
        vecPosNaNSSE2.vector = vecPosNaN;

        cmpResultN.vector = _mm_cmpeq_ps( v0sse2.vector, vecNegNaN );
        cmpResultP.vector = _mm_cmpeq_ps( v0sse2.vector, vecPosNaN );
        cmpResultNP.vector = _mm_or_ps( cmpResultN.vector, cmpResultP.vector );
        vecNegNaNSSE2.vectorSSE2Int = _mm_and_si128( cmpResultN.vectorSSE2Int, vecNegNaNSSE2.vectorSSE2Int );
        vecPosNaNSSE2.vectorSSE2Int = _mm_and_si128( cmpResultP.vectorSSE2Int, vecPosNaNSSE2.vectorSSE2Int );
        vecNaNSSE2.vectorSSE2Int = _mm_or_si128( vecNegNaNSSE2.vectorSSE2Int, vecPosNaNSSE2.vectorSSE2Int );
        v0MinusOne.vector = _mm_sub_ps( v0sse2.vector, vecOne );
        cmpResult.vector = _mm_cmpge_ps( v0sse2.vector, vecZero );
        v0sse2.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResult.vectorSSE2Int, v0sse2.vectorSSE2Int), _mm_andnot_si128(cmpResult.vectorSSE2Int, v0MinusOne.vectorSSE2Int) );
        
        r.vectorSSE2Int = _mm_cvttps_epi32( v0sse2.vector );
        r.vector = _mm_cvtepi32_ps( r.vectorSSE2Int );
        r.vectorSSE2Int = _mm_or_si128( _mm_and_si128(cmpResultNP.vectorSSE2Int, vecNaNSSE2.vectorSSE2Int), _mm_andnot_si128(cmpResultNP.vectorSSE2Int, r.vectorSSE2Int) );

        return r.vector;
    }

#endif

} // namespace vpl
} // namespace math
} // namespace rw

#ifdef RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#include "rw/math/vpl/detail/simulated/vec_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{

#if !VPL_SUPPORTED_PLATFORM( VPL_SSE2 )
    using rw::math::vpl::detail::simulated_vecops::VecAdd_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecSub_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_Char;
    using rw::math::vpl::detail::simulated_vecops::VecSub_Char;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_Char;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_Char;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecSub_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_Short;
    using rw::math::vpl::detail::simulated_vecops::VecSub_Short;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_Short;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_Short;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecSub_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_Int;
    using rw::math::vpl::detail::simulated_vecops::VecSub_Int;
    using rw::math::vpl::detail::simulated_vecops::VecConvertFloatToSignedFixedPoint;
    using rw::math::vpl::detail::simulated_vecops::VecConvertSignedFixedPointToFloat;
    using rw::math::vpl::detail::simulated_vecops::VecConvertFloatToUnsignedFixedPoint;
    using rw::math::vpl::detail::simulated_vecops::VecConvertUnsignedFixedPointToFloat;
    using rw::math::vpl::detail::simulated_vecops::VecLoad5BitImm_Char;
    using rw::math::vpl::detail::simulated_vecops::VecLoad5BitImm_Int;
    using rw::math::vpl::detail::simulated_vecops::VecLoadImm_Int;
    using rw::math::vpl::detail::simulated_vecops::VecLoad5BitImm_Short;
    using rw::math::vpl::detail::simulated_vecops::VecShiftLeftImm_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightImm_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightImm_Int;
    using rw::math::vpl::detail::simulated_vecops::VecCompEqual_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecCompEqual_Int;
    using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_Int;
    using rw::math::vpl::detail::simulated_vecops::VecCompLessThan_Int;
    using rw::math::vpl::detail::simulated_vecops::VecCompEqual_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecCompEqual_Short;
    using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_Short;
    using rw::math::vpl::detail::simulated_vecops::VecCompLessThan_Short;
    using rw::math::vpl::detail::simulated_vecops::VecCompEqual_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecCompEqual_Char;
    using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_Char;
    using rw::math::vpl::detail::simulated_vecops::VecCompLessThan_Char;
    using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecCompLessThan_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecCompLessThan_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecCompLessThan_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecMax_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecMax_Int;
    using rw::math::vpl::detail::simulated_vecops::VecMax_Short;
    using rw::math::vpl::detail::simulated_vecops::VecMax_Char;
    using rw::math::vpl::detail::simulated_vecops::VecMin_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecMin_Int;
    using rw::math::vpl::detail::simulated_vecops::VecMin_Short;
    using rw::math::vpl::detail::simulated_vecops::VecMin_Char;
    using rw::math::vpl::detail::simulated_vecops::VecMax_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecMax_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecMin_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecMin_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_Int;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_Int; 
    using rw::math::vpl::detail::simulated_vecops::VecSplat_Half;
    using rw::math::vpl::detail::simulated_vecops::VecSplat_Byte;
    using rw::math::vpl::detail::simulated_vecops::VecShiftLeftUsingVector_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightUsingVector_UInt;
    using rw::math::vpl::detail::simulated_vecops::VecShiftLeftUsingVector_Int;
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightUsingVector_Int;
    using rw::math::vpl::detail::simulated_vecops::VecRotateLeftWithMaskedInsertionImm;
    using rw::math::vpl::detail::simulated_vecops::VecMergeHigh_Byte;
    using rw::math::vpl::detail::simulated_vecops::VecMergeHigh_Half;
    using rw::math::vpl::detail::simulated_vecops::VecMergeLow_Byte;
    using rw::math::vpl::detail::simulated_vecops::VecMergeLow_Half;
    using rw::math::vpl::detail::simulated_vecops::VecShiftLeftByBytesUsingVectorBitCount;
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightByBytesUsingVectorBitCount;
    using rw::math::vpl::detail::simulated_vecops::VecFloor;
#endif
    using rw::math::vpl::detail::simulated_vecops::VecLog2Est;
    using rw::math::vpl::detail::simulated_vecops::VecPermute;
    using rw::math::vpl::detail::simulated_vecops::VecLoadLeftAligned;
    using rw::math::vpl::detail::simulated_vecops::VecLoadRightAligned;
    using rw::math::vpl::detail::simulated_vecops::VecStoreLeftAligned;
    using rw::math::vpl::detail::simulated_vecops::VecStoreRightAligned;
    using rw::math::vpl::detail::simulated_vecops::VecStoreWordElement;
    using rw::math::vpl::detail::simulated_vecops::VecStoreHalfElement;
    using rw::math::vpl::detail::simulated_vecops::VecStoreByteElement;
    using rw::math::vpl::detail::simulated_vecops::VecShiftLeftImm;
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightImm;
	using rw::math::vpl::detail::simulated_vecops::VecShiftLeftByBitsUsingVectorBitCount;
	using rw::math::vpl::detail::simulated_vecops::VecShiftRightByBitsUsingVectorBitCount;
} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#endif // RW_MATH_VEC_OPERATION_INLINE_H
