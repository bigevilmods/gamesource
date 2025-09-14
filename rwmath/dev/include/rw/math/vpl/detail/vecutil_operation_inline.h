// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VECUTIL_OPERATION_DETAIL_INLINE_H
#define RW_MATH_VECUTIL_OPERATION_DETAIL_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic_operation.h"

#include "rw/math/detail/static_asserts.h"
#include "rw/math/vpl/detail/vec_operation_static_asserts.h"

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined EA_PLATFORM_REVOLUTION || defined RWCROSS || defined EA_PLATFORM_WINDOWS || defined EA_PLATFORM_POSIX || defined EA_PROCESSOR_ARM || defined EA_PLATFORM_APPLE || defined RWMATH_USE_NEON || defined EA_PLATFORM_AIRPLAY || defined RW_MATH_TARGET_SUPPORTS_SSE
    #ifndef RW_MATH_TAG_API_DURING_PREPROCESSING
        #include "rw/math/vpl/detail/simulated/vec_operation_inline.h"    
    #endif
	#include "rw/math/vpl/detail/cross/vecutil_operation_inline.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpl/detail/ps3/ppu/vecutil_operation_inline.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpl/detail/ps3/spu/vecutil_operation_inline.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpl/detail/xbox2/vecutil_operation_inline.h"
#else
    #error Unsupported platform
#endif

#include "rw/math/vpl/detail/vec_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{

namespace detail
{
	extern const ::rw::math::vpu::VectorIntrinsic vecFloatHalfConversionConstants[3];
}

#if VPL_SUPPORTED_PLATFORM( VPL_SSE | VPL_BROADWAY ) || VPL_CURRENT_PLATFORM == 0	
    using rw::math::vpl::detail::simulated_vecops::VecShiftRightImm_UInt;
	using rw::math::vpl::detail::simulated_vecops::VecCompGreaterThan_UInt;
	using rw::math::vpl::detail::simulated_vecops::VecAddSat_Int;
	using rw::math::vpl::detail::simulated_vecops::VecShiftLeftImm_UInt;
#endif

#if VPL_SUPPORTED_PLATFORM( VPL_BROADWAY ) || VPL_CURRENT_PLATFORM == 0		
	using rw::math::vpl::detail::simulated_vecops::VecSplat_Word;
	using rw::math::vpl::detail::simulated_vecops::VecAnd;
	using rw::math::vpl::detail::simulated_vecops::VecSelect;
#endif

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic(float v0, float v1, float v2, float v3)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarFloat[0] = v0;
        vecUnion.scalarFloat[1] = v1;
        vecUnion.scalarFloat[2] = v2;
        vecUnion.scalarFloat[3] = v3;
        return vecUnion.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_UInt(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarUInt[0] = v0;
        vecUnion.scalarUInt[1] = v1;
        vecUnion.scalarUInt[2] = v2;
        vecUnion.scalarUInt[3] = v3;
        return vecUnion.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_Int(int32_t v0, int32_t v1, int32_t v2, int32_t v3)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarInt[0] = v0;
        vecUnion.scalarInt[1] = v1;
        vecUnion.scalarInt[2] = v2;
        vecUnion.scalarInt[3] = v3;
        return vecUnion.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_UShort(uint16_t v0, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4, uint16_t v5, uint16_t v6, uint16_t v7)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarUShort[0] = v0;
        vecUnion.scalarUShort[1] = v1;
        vecUnion.scalarUShort[2] = v2;
        vecUnion.scalarUShort[3] = v3;
        vecUnion.scalarUShort[4] = v4;
        vecUnion.scalarUShort[5] = v5;
        vecUnion.scalarUShort[6] = v6;
        vecUnion.scalarUShort[7] = v7;
        return vecUnion.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_Short(int16_t v0, int16_t v1, int16_t v2, int16_t v3, int16_t v4, int16_t v5, int16_t v6, int16_t v7)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarShort[0] = v0;
        vecUnion.scalarShort[1] = v1;
        vecUnion.scalarShort[2] = v2;
        vecUnion.scalarShort[3] = v3;
        vecUnion.scalarShort[4] = v4;
        vecUnion.scalarShort[5] = v5;
        vecUnion.scalarShort[6] = v6;
        vecUnion.scalarShort[7] = v7;
        return vecUnion.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_UChar(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5, uint8_t v6, uint8_t v7, uint8_t v8, uint8_t v9, uint8_t v10, uint8_t v11, uint8_t v12, uint8_t v13, uint8_t v14, uint8_t v15)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarUChar[ 0] =  v0;
        vecUnion.scalarUChar[ 1] =  v1;
        vecUnion.scalarUChar[ 2] =  v2;
        vecUnion.scalarUChar[ 3] =  v3;
        vecUnion.scalarUChar[ 4] =  v4;
        vecUnion.scalarUChar[ 5] =  v5;
        vecUnion.scalarUChar[ 6] =  v6;
        vecUnion.scalarUChar[ 7] =  v7;
        vecUnion.scalarUChar[ 8] =  v8;
        vecUnion.scalarUChar[ 9] =  v9;
        vecUnion.scalarUChar[10] = v10;
        vecUnion.scalarUChar[11] = v11;
        vecUnion.scalarUChar[12] = v12;
        vecUnion.scalarUChar[13] = v13;
        vecUnion.scalarUChar[14] = v14;
        vecUnion.scalarUChar[15] = v15;
        return vecUnion.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_Char(int8_t v0, int8_t v1, int8_t v2, int8_t v3, int8_t v4, int8_t v5, int8_t v6, int8_t v7, int8_t v8, int8_t v9, int8_t v10, int8_t v11, int8_t v12, int8_t v13, int8_t v14, int8_t v15)
    {
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion;
        vecUnion.scalarChar[ 0] =  v0;
        vecUnion.scalarChar[ 1] =  v1;
        vecUnion.scalarChar[ 2] =  v2;
        vecUnion.scalarChar[ 3] =  v3;
        vecUnion.scalarChar[ 4] =  v4;
        vecUnion.scalarChar[ 5] =  v5;
        vecUnion.scalarChar[ 6] =  v6;
        vecUnion.scalarChar[ 7] =  v7;
        vecUnion.scalarChar[ 8] =  v8;
        vecUnion.scalarChar[ 9] =  v9;
        vecUnion.scalarChar[10] = v10;
        vecUnion.scalarChar[11] = v11;
        vecUnion.scalarChar[12] = v12;
        vecUnion.scalarChar[13] = v13;
        vecUnion.scalarChar[14] = v14;
        vecUnion.scalarChar[15] = v15;
        return vecUnion.vector;
    }

	// VPL_PLATFORM_RATING_3
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilConvertFloatToHalf( VectorIntrinsicInParam vec )
	{
		VectorIntrinsic selectSignAndMantissaMask = VecSplat_Word<0>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[0] );
		VectorIntrinsic selectExpMask             = VecSplat_Word<1>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[0] );
		VectorIntrinsic expMask                   = VecSplat_Word<2>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[0] );
		VectorIntrinsic rebiasExponentValue       = VecSplat_Word<3>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[0] );;

		// Extract the exponent and change the bias to the new floating point number's range.
		VectorIntrinsic exponent = VecAnd( vec, expMask );
		exponent = VecShiftRightImm_UInt<23>( exponent );

#ifdef RW_MATH_ENABLE_ASSERTS
		::rw::math::vpu::detail::VectorIntrinsicUnion viu0, viu1;
		viu0.vector = exponent;
		viu1.vector = vec;
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[0] != (unsigned int)255, "The float in X element of the input vector is either +/-Infinity or NaN!");
        RW_MATH_ASSERT_MSG( viu0.scalarUInt[1] != (unsigned int)255, "The float in Y element of the input vector is either +/-Infinity or NaN!");
        RW_MATH_ASSERT_MSG( viu0.scalarUInt[2] != (unsigned int)255, "The float in Z element of the input vector is either +/-Infinity or NaN!");
        RW_MATH_ASSERT_MSG( viu0.scalarUInt[3] != (unsigned int)255, "The float in W element of the input vector is either +/-Infinity or NaN!");
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[0] != (unsigned int)0 || viu1.scalarUInt[0] == (unsigned int)0 || viu1.scalarUInt[0] == (unsigned int)0x80000000, "The float in X element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[1] != (unsigned int)0 || viu1.scalarUInt[1] == (unsigned int)0 || viu1.scalarUInt[1] == (unsigned int)0x80000000, "The float in Y element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[2] != (unsigned int)0 || viu1.scalarUInt[2] == (unsigned int)0 || viu1.scalarUInt[2] == (unsigned int)0x80000000, "The float in Z element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[3] != (unsigned int)0 || viu1.scalarUInt[3] == (unsigned int)0 || viu1.scalarUInt[3] == (unsigned int)0x80000000, "The float in W element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[0] == (unsigned int)0 || (viu0.scalarUInt[0] >= (unsigned int)113 && viu0.scalarUInt[0] <= (unsigned int) 142), "The float in X element of the input vector is out of range for a half float!");
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[1] == (unsigned int)0 || (viu0.scalarUInt[1] >= (unsigned int)113 && viu0.scalarUInt[1] <= (unsigned int) 142), "The float in X element of the input vector is out of range for a half float!");
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[2] == (unsigned int)0 || (viu0.scalarUInt[2] >= (unsigned int)113 && viu0.scalarUInt[2] <= (unsigned int) 142), "The float in X element of the input vector is out of range for a half float!");
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[3] == (unsigned int)0 || (viu0.scalarUInt[3] >= (unsigned int)113 && viu0.scalarUInt[3] <= (unsigned int) 142), "The float in X element of the input vector is out of range for a half float!");
#endif

		// check if exponent is zero.
		VectorIntrinsic exponentZeroMask = VecCompGreaterThan_UInt( exponent, ::rw::math::vpl::detail::vecFloatHalfConversionConstants[2] );
		exponent = VecAddSat_Int( exponent, rebiasExponentValue );
		exponent = VecAnd(exponent, exponentZeroMask);

		// Shift exponent, the sign bit, and the mantissa to correct final positions
		VectorIntrinsic shiftedExponent = VecShiftLeftImm_UInt<10>( exponent );
		VectorIntrinsic shiftedSign = VecShiftRightImm_UInt<16>( vec );
		VectorIntrinsic shiftedMantissa = VecShiftRightImm_UInt<13>( vec );

		// Combine the sign flag with the mantissa
		VectorIntrinsic partialResult = VecSelect( shiftedMantissa, shiftedSign, selectSignAndMantissaMask );
		// Combine the partial result with the exponent
		VectorIntrinsic result = VecSelect( partialResult, shiftedExponent, selectExpMask );

		return result;
	}

	// VPL_PLATFORM_RATING_3
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilConvertHalfToFloat( VectorIntrinsicInParam vec )
	{
		VectorIntrinsic selectSignAndMantissaMask = VecSplat_Word<0>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[1] );
		VectorIntrinsic selectExpMask             = VecSplat_Word<1>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[1] );
		VectorIntrinsic expMask                   = VecSplat_Word<2>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[1] );
		VectorIntrinsic rebiasExponentValue       = VecSplat_Word<3>( ::rw::math::vpl::detail::vecFloatHalfConversionConstants[1] );;

		// Extract the exponent and change the bias to the new floating point number's range.
		VectorIntrinsic exponent = VecAnd( vec, expMask );
		exponent = VecShiftRightImm_UInt<10>( exponent );

#ifdef RW_MATH_ENABLE_ASSERTS
		::rw::math::vpu::detail::VectorIntrinsicUnion viu0, viu1;
		viu0.vector = exponent;
		viu1.vector = vec;
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[0] != (unsigned int)31, "The half float in X element of the input vector is either +/-Infinity or NaN!");
        RW_MATH_ASSERT_MSG( viu0.scalarUInt[1] != (unsigned int)31, "The half float in Y element of the input vector is either +/-Infinity or NaN!");
        RW_MATH_ASSERT_MSG( viu0.scalarUInt[2] != (unsigned int)31, "The half float in Z element of the input vector is either +/-Infinity or NaN!");
        RW_MATH_ASSERT_MSG( viu0.scalarUInt[3] != (unsigned int)31, "The half float in W element of the input vector is either +/-Infinity or NaN!");
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[0] != (unsigned int)0 || viu1.scalarUInt[0] == 0 || viu1.scalarUInt[0] == (unsigned int)32768, "The half float in X element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[1] != (unsigned int)0 || viu1.scalarUInt[1] == 0 || viu1.scalarUInt[1] == (unsigned int)32768, "The half float in Y element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[2] != (unsigned int)0 || viu1.scalarUInt[2] == 0 || viu1.scalarUInt[2] == (unsigned int)32768, "The half float in Z element of the input vector is denormalized!" );
		RW_MATH_ASSERT_MSG( viu0.scalarUInt[3] != (unsigned int)0 || viu1.scalarUInt[3] == 0 || viu1.scalarUInt[3] == (unsigned int)32768, "The half float in W element of the input vector is denormalized!" );
#endif

		// check if exponent is zero.
		VectorIntrinsic exponentZeroMask = VecCompGreaterThan_UInt( exponent, ::rw::math::vpl::detail::vecFloatHalfConversionConstants[2] );
		exponent = VecAddSat_Int( exponent, rebiasExponentValue );
		exponent = VecAnd(exponent, exponentZeroMask);

		// Shift exponent, the sign bit, and the mantissa to correct final positions
		VectorIntrinsic shiftedExponent = VecShiftLeftImm_UInt<23>( exponent );
		VectorIntrinsic shiftedSign = VecShiftLeftImm_UInt<16>( vec );
		VectorIntrinsic shiftedMantissa = VecShiftLeftImm_UInt<13>( vec );

		// Combine the sign flag with the mantissa
		VectorIntrinsic partialResult = VecSelect( shiftedMantissa, shiftedSign, selectSignAndMantissaMask );
		// Combine the partial result with the exponent
		VectorIntrinsic result = VecSelect( partialResult, shiftedExponent, selectExpMask );

		return result;
	}

} // namespace vpl
} // namespace math
} // namespace rw

#endif
