// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_INLINE_H
#define RW_MATH_VEC_OPERATION_INLINE_H

namespace rw
{
namespace math
{
namespace vpl
{


    // VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT vtbl
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecPermute(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam permuteConstant)
    {
        uint8x16_t inUint0 = vreinterpretq_u8_f32(v0);
        uint8x16_t inUint1 = vreinterpretq_u8_f32(v1);
        uint8x16_t pattern = vreinterpretq_u8_f32(permuteConstant);
        uint8x8x4_t tableIn = {{ vget_low_u8(inUint0), vget_high_u8(inUint0), vget_low_u8(inUint1), vget_high_u8(inUint1) }};
        uint8x8_t lowOut = vtbl4_u8(tableIn, vget_low_u8(pattern));
        uint8x8_t highOut = vtbl4_u8(tableIn, vget_high_u8(pattern));
        return vreinterpretq_f32_u8(vcombine_u8(lowOut, highOut));
    }

	// VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vld1q_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset)
	{
		return vreinterpretq_f32_u8(
			vld1q_u8((const uint8_t *)base + offset) 
			);
	}

	// VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vst1q_u8
	RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset)
	{
		vst1q_u8((uint8_t *)base + offset, vreinterpretq_u8_f32(v0));
	}

	// VPL_PLATFORM_RATING_3
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecFloor(VectorIntrinsicInParam v0)
	{
		VectorIntrinsic ret;
		RW_MATH_VPU_Floor(ret, v0);
		return ret;
	}

	// VPL_PLATFORM_RATING_2
	VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0)
	{
		return vmulq_n_f32( vcvtq_f32_u32( vreinterpretq_u32_f32(v0) ), static_cast<float>( 1.0f / (1 << ScaleExponent)) );
	}
	
	// VPL_PLATFORM_RATING_2
	VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0)
	{
	   	return vreinterpretq_f32_u32( vcvtq_u32_f32( vmulq_n_f32(v0, static_cast<float>(1 << ScaleExponent) ) ));
	}

	// VPL_PLATFORM_RATING_2
	VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0)
	{
		return vmulq_n_f32( vcvtq_f32_s32( vreinterpretq_s32_f32(v0) ), static_cast<float>( 1.0f / (1 << ScaleExponent) ));
	}
	
	// VPL_PLATFORM_RATING_2
	VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0)
	{
	   	return vreinterpretq_f32_s32(  vcvtq_s32_f32(vmulq_n_f32(v0, static_cast<float>(1 << ScaleExponent)) ) );
	}

	// VPL_PLATFORM_RATING_3
	VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0)
	{
		rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
		VectorIntrinsic result;
		RW_MATH_VPU_PermuteSingle_I(result, v0, PermConstant);
		return result;
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vdupq_lane_u8
	VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0)
	{
		detail::Assert_ValueInRange< ElementIndex, 0, 15 >();
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t ret = (ElementIndex >= 8) ?
				vdupq_lane_u8(vget_high_u8(a), ElementIndex - 8) :
				vdupq_lane_u8(vget_low_u8(a), ElementIndex);
		return vreinterpretq_f32_u8(ret);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vaddq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vaddq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vsubq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vsubq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqaddq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vqaddq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqsubq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vqsubq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vminq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vminq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vmaxq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vmaxq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vceqq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vceqq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcgtq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vcgtq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcltq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint8x16_t a = vreinterpretq_u8_f32(v0);
		uint8x16_t b = vreinterpretq_u8_f32(v1);
		return vreinterpretq_f32_u8( vcltq_u8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vaddq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_s8( vaddq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vsubq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_s8( vsubq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqaddq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_s8( vqaddq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqsubq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_s8( vqsubq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vminq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_s8( vminq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vmaxq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_s8( vmaxq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vceqq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_u8( vceqq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcgtq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_u8( vcgtq_s8(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcltq_s8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int8x16_t a = vreinterpretq_s8_f32(v0);
		int8x16_t b = vreinterpretq_s8_f32(v1);
		return vreinterpretq_f32_u8( vcltq_s8(a, b) );
	}


	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vaddq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vaddq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vsubq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vsubq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqaddq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vqaddq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqsubq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vqsubq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vminq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vminq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vmaxq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vmaxq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vceqq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vceqq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcgtq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vcgtq_u16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcltq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t b = vreinterpretq_u16_f32(v1);
		return vreinterpretq_f32_u16( vcltq_u16(a, b) );
	}


	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vaddq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_s16( vaddq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vsubq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_s16( vsubq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqaddq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_s16( vqaddq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqsubq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_s16( vqsubq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vminq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_s16( vminq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vmaxq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_s16( vmaxq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vceqq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_u16( vceqq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcgtq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_u16( vcgtq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcltq_s16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int16x8_t a = vreinterpretq_s16_f32(v0);
		int16x8_t b = vreinterpretq_s16_f32(v1);
		return vreinterpretq_f32_u16( vcltq_s16(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vaddq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vaddq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vsubq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vsubq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqaddq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vqaddq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqsubq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vqsubq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vminq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vminq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vmaxq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vmaxq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vceqq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vceqq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcgtq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vcgtq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcltq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		uint32x4_t a = vreinterpretq_u32_f32(v0);
		uint32x4_t b = vreinterpretq_u32_f32(v1);
		return vreinterpretq_f32_u32( vcltq_u32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vaddq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_s32( vaddq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vsubq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_s32( vsubq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqaddq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_s32( vqaddq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vqsubq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_s32( vqsubq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vminq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_s32( vminq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vmaxq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_s32( vmaxq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vceqq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_u32( vceqq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcgtq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_u32( vcgtq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vcltq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
	{
		int32x4_t a = vreinterpretq_s32_f32(v0);
		int32x4_t b = vreinterpretq_s32_f32(v1);
		return vreinterpretq_f32_u32( vcltq_s32(a, b) );
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vdupq_n_s8
	VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char()
	{
		detail::Assert_5BitSignedValue< Value >();
		int8x16_t res = vdupq_n_s8( Value );
		return vreinterpretq_f32_s8(res);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vdupq_n_s16
	VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short()
	{
		detail::Assert_5BitSignedValue< Value >();
		int16x8_t res = vdupq_n_s16( Value );
		return vreinterpretq_f32_s16(res);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vdupq_n_s32
	VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int()
	{
		detail::Assert_5BitSignedValue< Value >();
		int32x4_t res = vdupq_n_s32( Value );
		return vreinterpretq_f32_s32(res);
	}

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT vdupq_n_s32
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int()
    {
        int32x4_t res = vdupq_n_s32( Value );
        return vreinterpretq_f32_s32(res);
    }

	// VPL_PLATFORM_RATING_2
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{
		return vreinterpretq_f32_u16( vmvnq_u16(vorrq_u16(vreinterpretq_u16_f32(v0), vreinterpretq_u16_f32(v1)) ));
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vzipq_f32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{		
		return vzipq_f32(v0, v1).val[1];
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vzipq_f32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{
		return vzipq_f32(v0, v1).val[0];
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vshlq_n_u32
	VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0)
	{
		return vreinterpretq_f32_u32(vshlq_n_u32(vreinterpretq_u32_f32(v0), Shift)); 
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vshrq_n_u32
	VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0)
	{
		return vreinterpretq_f32_u32(vshrq_n_u32(vreinterpretq_u32_f32(v0), Shift)); 
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vshrq_n_s32
	VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0)	
	{
		return vreinterpretq_f32_s32(vshrq_n_s32(vreinterpretq_s32_f32(v0), Shift)); 
	}

	// VPL_PLATFORM_RATING_2
	// VPL_PLATFORM_EQUIVALENT vdupq_lane_u16
	VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half (VectorIntrinsicInParam v0)
	{
		detail::Assert_ValueInRange< ElementIndex, 0, 7 >();
		uint16x8_t a = vreinterpretq_u16_f32(v0);
		uint16x8_t ret = (ElementIndex >= 4) ?
				vdupq_lane_u16(vget_high_u16(a), ElementIndex - 4) :
				vdupq_lane_u16(vget_low_u16(a), ElementIndex);
		return vreinterpretq_f32_u16(ret);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vshlq_u32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    shiftCount )  
	{
		return vreinterpretq_f32_u32(vshlq_u32(vreinterpretq_u32_f32(v0), vreinterpretq_s32_f32(shiftCount)));
	}

	// VPL_PLATFORM_RATING_2
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    shiftCount )  
	{
		return vreinterpretq_f32_u32(vshlq_u32(vreinterpretq_u32_f32(v0), vnegq_s32(vreinterpretq_s32_f32(shiftCount))));
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vshlq_s32
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    shiftCount )  
	{
		return vreinterpretq_f32_s32(vshlq_s32(vreinterpretq_s32_f32(v0), vreinterpretq_s32_f32(shiftCount)));
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vzipq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{		
		return vreinterpretq_f32_u8(vzipq_u8(vreinterpretq_u8_f32(v0), vreinterpretq_u8_f32(v1)).val[1]);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vzipq_u8
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{
		return vreinterpretq_f32_u8(vzipq_u8(vreinterpretq_u8_f32(v0), vreinterpretq_u8_f32(v1)).val[0]);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vzipq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{		
		return vreinterpretq_f32_u16(vzipq_u16(vreinterpretq_u16_f32(v0), vreinterpretq_u16_f32(v1)).val[1]);
	}

	// VPL_PLATFORM_RATING_1
	// VPL_PLATFORM_EQUIVALENT vzipq_u16
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    v1 )  
	{
		return vreinterpretq_f32_u16(vzipq_u16(vreinterpretq_u16_f32(v0), vreinterpretq_u16_f32(v1)).val[0]);
	}

	// VPL_PLATFORM_RATING_2
	VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    shiftAmount )  
	{
		int byteValue = vgetq_lane_u8 (vreinterpretq_u8_f32(shiftAmount), 0) / 8;

		return VecPermute(v0, v0, detail::gShiftLeftArray[byteValue]);
	}

	// VPL_PLATFORM_RATING_2
	VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount  (  VectorIntrinsicInParam    v0,  VectorIntrinsicInParam    shiftAmount )  
	{
		int byteValue = vgetq_lane_u8 (vreinterpretq_u8_f32(shiftAmount), 0) / 8;

		return VecPermute(v0, v0, detail::gShiftRightArray[byteValue]);
	}

}
}
}

#ifdef RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#include "rw/math/vpl/detail/simulated/vec_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{
	
	// TODO: we should implemente it with the following piece of code
	// which should work but isn't, because sony maps the intrinsics incorrectly
	// see issue 1046 on vita dev net.
	// return vreinterpretq_f32_s32(vshlq_s32(vreinterpretq_s32_f32(v0), vnegq_s32(vreinterpretq_s32_f32(shiftCount))));
	using rw::math::vpl::detail::simulated_vecops::VecShiftRightUsingVector_Int;

    using rw::math::vpl::detail::simulated_vecops::VecRotateLeftWithMaskedInsertionImm;
    using rw::math::vpl::detail::simulated_vecops::VecLog2Est;
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
