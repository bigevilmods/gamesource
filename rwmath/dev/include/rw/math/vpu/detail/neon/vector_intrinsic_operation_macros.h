// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_VECTOR_INTRINSIC_MACRO_INLINE_H
#define RW_MATH_VPU_DETAIL_CROSS_VECTOR_INTRINSIC_MACRO_INLINE_H

#include "vector_intrinsic.h"
#include <arm_neon.h>

/*
 * Further optimizations:
 *
 * There are still some remaining optimizations that would be good to do.
 *
 * - Take advantage of instructions with a free broadcast (e.g. for vector * matrix and matrix * matrix).
 * - Take advantage of reciprocal step and reciprocal squareroot step.
 * - Transpose functions could be written in inline assembly.
 * - Look at performance of Cross/Dot functions since they are quite common and are currently fairly slow.
 * - Look at optimizing matrix/quaternion conversions.
 *
 */


#ifdef __clang__
    #define rwmath_vgetq_lane_f32(vec_src, vec_src_ind) \
        ((vec_src_ind == 0)?vgetq_lane_f32(vec_src, 0):\
        (vec_src_ind == 1)?vgetq_lane_f32(vec_src, 1):\
        (vec_src_ind == 2)?vgetq_lane_f32(vec_src, 2):\
        vgetq_lane_f32(vec_src, 3))
    
    #define rwmath_vsetq_lane_f32(value, vec_dest, vec_dest_ind)\
        ((vec_dest_ind==0)?vsetq_lane_f32(value, vec_dest, 0):\
        (vec_dest_ind==1)?vsetq_lane_f32(value, vec_dest, 1):\
        (vec_dest_ind==2)?vsetq_lane_f32(value, vec_dest, 2):\
        vsetq_lane_f32(value, vec_dest, 3))
    
    #define rwmath_vextq_f32(vec1, vec2, index) \
        ((index == 0)?vextq_f32(vec1, vec2, 0):\
        (index == 1)?vextq_f32(vec1, vec2, 1):\
        (index == 2)?vextq_f32(vec1, vec2, 2):\
        vextq_f32(vec1, vec2, 3))
#else
    #define rwmath_vgetq_lane_f32 vgetq_lane_f32
    #define rwmath_vsetq_lane_f32 vsetq_lane_f32
    #define rwmath_vextq_f32 vextq_f32
#endif

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{

    template<int ElemIndex>
    float32x4_t NeonSplatElement(float32x4_t v);

    template<> __attribute__ ((always_inline)) inline
    float32x4_t NeonSplatElement<0>(float32x4_t v)
    {
        return vdupq_lane_f32(vget_low_f32(v), 0);
    }

    template<> __attribute__ ((always_inline)) inline
    float32x4_t NeonSplatElement<1>(float32x4_t v)
    {
        return vdupq_lane_f32(vget_low_f32(v), 1);
    }

    template<> __attribute__ ((always_inline)) inline
    float32x4_t NeonSplatElement<2>(float32x4_t v)
    {
        return vdupq_lane_f32(vget_high_f32(v), 0);
    }

    template<> __attribute__ ((always_inline)) inline
    float32x4_t NeonSplatElement<3>(float32x4_t v)
    {
        return vdupq_lane_f32(vget_high_f32(v), 1);
    }

    void TransposeA4V4(VectorIntrinsic &outX, VectorIntrinsic &outY, VectorIntrinsic &outZ, VectorIntrinsic &outW,
                       VectorIntrinsicInParam inX, VectorIntrinsicInParam inY, VectorIntrinsicInParam inZ, VectorIntrinsicInParam inW);

    bool FloatingPointCompareEqual(VectorIntrinsicInParam a, VectorIntrinsicInParam b);

#ifdef __llvm__
    VectorIntrinsic VPU_SquareRoot(VectorIntrinsicInParam in_vec);
    VectorIntrinsic VPU_SquareRootFast(VectorIntrinsicInParam in_vec);
    VectorIntrinsic VPU_SquareRootEstimate(VectorIntrinsicInParam in_vec);
#endif
}

#define RWMATH_VPU_DETAIL_VecNmsub(a, b, c) \
    vmlsq_f32(c, a, b)

#define RWMATH_VPU_DETAIL_VecMadd(a, b, c) \
    vmlaq_f32(c, a, b)

#define RWMATH_VPU_DETAIL_VecMul( a, b ) \
    vmulq_f32( a, b )

#define RW_MATH_VPU_V2_RotateLeft( ret, vec) \
    ret = vrev64q_f32(vec)

// TODO Do full vector rotate followed by insertion of x in correct location
#define RW_MATH_VPU_V3_RotateLeft( ret, vec) \
    RW_MATH_VPU_PermuteSingle_I(ret, vec, 0x1203)

// TODO Do full vector rotate followed by insertion of z in correct location
#define RW_MATH_VPU_V3_RotateRight( ret, vec) \
    RW_MATH_VPU_PermuteSingle_I(ret, vec, 0x2013)

#define RW_MATH_VPU_V4_RotateLeft( ret, vec) \
    ret = rwmath_vextq_f32(vec, vec, 1);

#define RW_MATH_VPU_RotateLeft_V_I(ret, in_vec, rotateAmount) \
    ret = rwmath_vextq_f32(in_vec, in_vec, rotateAmount);

#define RW_MATH_VPU_V4_RotateRight( ret, vec) \
    ret = rwmath_vextq_f32(vec, vec, 3);

// 1) macro must prefix with namespace, for example rw::math::vpu --> rwmath_vpu
// 2) first parameter must be the return value
// 3) Vector size postfix (only specified when needed, usually assume V4)
//    V2 - 2 element vector
//    V3 - 3 element vector
//    V4 - 4 element vector (default, if not specified or not specific to a macro)
// 4) Vector array postfix (only specified when needed, usually assume A1)
//    A1 - array of vector size 1 or vector[1] (default, if not specified or not specific to a macro)
//    A2 - array of vector size 2 or vector[2]
//    A3 - array of vector size 3 or vector[3]
//    A4 - array of vector size 4 or vector[4]
//       for example: RW_MATH_VPU_IsZeroA1V3(ret, in_vec0); // one vector as input, treat all vector as vector3
//       for example: RW_MATH_VPU_IsZeroA2V3(ret, in_vec0, in_vec1); // two vectors as input, treat all vector as vector3
//       for example: RW_MATH_VPU_IsZeroA1V4(ret, in_vec0); // one vector as input, treat all vector as vector4
// 4) postfix must be specified for following types
//    a) I - immediate value is required for one or more parameters
//    b) R - macro behave like a function call, where it feels like return value
//
//       for example: myvalue = RW_MATH_VPU_Splat_IR(x, 2)
//       postfix IR - immediate and behave as function that return value.

// t0 = b & c;
// t1 = a & ^c;
// r = t0 | t1;
#define RW_MATH_VPU_Select(ret, a, b, mask) \
    ret = vbslq_f32(vreinterpretq_u32_f32(mask), (b), (a))

RW_MATH_FORCE_INLINE VectorIntrinsic RW_MATH_VPU_LoadFromAlignedFloatPointer( const float *ptr )
{
    // Note: This cast to float32_t is required in iPhone and Android (and perhaps some other platforms as well).
    return vld1q_f32((float32_t*)ptr);
}

#define RW_MATH_VPU_UnalignedLoad(ret, address) \
{ \
    ret = vld1q_f32(reinterpret_cast<float32_t*>(const_cast<float*>(address)));  \
}

#define RW_MATH_VPU_UnalignedStore2(destination, vector) \
{ \
    vst1q_lane_f32(reinterpret_cast<float32_t*>(&destination.x) , vector, 0); \
    vst1q_lane_f32(reinterpret_cast<float32_t*>(&destination.y), vector, 1); \
}

#define RW_MATH_VPU_UnalignedStore3(destination, vector) \
{ \
    vst1q_lane_f32(reinterpret_cast<float32_t*>(&destination.x) , vector, 0); \
    vst1q_lane_f32(reinterpret_cast<float32_t*>(&destination.y), vector, 1); \
    vst1q_lane_f32(reinterpret_cast<float32_t*>(&destination.z), vector, 2); \
}

#define RW_MATH_VPU_UnalignedStore4(destination, vector) \
{ \
    vst1q_f32(reinterpret_cast<float32_t*>(&destination.x), vector); \
}


#define RW_MATH_VPU_IsGreater(ret, a, b) \
        ret = vreinterpretq_f32_u32(vcgtq_f32((a), (b)))

#define RW_MATH_VPU_IsGreaterEqual(ret, a, b) \
        ret = vreinterpretq_f32_u32(vcgeq_f32((a), (b)))

#define RW_MATH_VPU_IsEqual(ret, a, b) \
        ret = vreinterpretq_f32_u32(vceqq_f32((a), (b)))

// The scalar comparison macros assume that all elements of the vector are
// identical.  The return result is either boolean or an integer that indicates
// true/false.
#define RW_MATH_VPU_Scalar_IsEqual(result, a, b) \
        result = (rwmath_vgetq_lane_f32(a,0) == rwmath_vgetq_lane_f32(b,0))

#define RW_MATH_VPU_Scalar_IsGreater(result, a, b) \
        result = (rwmath_vgetq_lane_f32(a,0) > rwmath_vgetq_lane_f32(b,0))

#define RW_MATH_VPU_Scalar_IsGreaterEqual(result, a, b) \
        result = (rwmath_vgetq_lane_f32(a,0) >= rwmath_vgetq_lane_f32(b,0))

#define RW_MATH_VPU_Scalar_IsLess(result, a, b) \
        result = (rwmath_vgetq_lane_f32(a,0) < rwmath_vgetq_lane_f32(b,0))

#define RW_MATH_VPU_Scalar_IsLessEqual(result, a, b) \
        result = (rwmath_vgetq_lane_f32(a,0) <= rwmath_vgetq_lane_f32(b,0))

#define RW_MATH_VPU_CreateVectorMaskFromBool( ret, boolValue ) \
    { \
        unsigned int value = boolValue ? 0xffffffff : 0; \
        ret = vreinterpretq_f32_u32(vdupq_n_u32(value)); \
    }


// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV4(ret, f0, f1, f2, f3) \
    ret = (float32x4_t){f0, f1, f2, f3}


// create a vector intrinsic from a pointer to a float.
// The float data will be splatted across the entire vector.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    ret = vdupq_n_f32(*fPtr)


// create a vector intrinsic data from 1 float
// ret - VectorIntrinsic
// f0 - float
#define RW_MATH_VPU_CreateVectorV1(ret, f0) \
    ret = vdupq_n_f32(f0)


// create a vector intrinsic data from two floats
#define RW_MATH_VPU_CreateVectorV2(ret, f0, f1) \
    ret = (float32x4_t){f0, f1, f0, f0}


// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV3(ret, f0, f1, f2) \
    ret = (float32x4_t){f0, f1, f2, f2}


#define RW_MATH_VPU_SwizzleStore_V_V(ret, vec_dest, vec_dest_index, vec_src, vec_src_index) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion temp_src, temp_dest; \
        temp_src.vector = vec_src; \
        temp_dest.vector = vec_dest; \
        temp_dest.scalarFloat[vec_dest_index] = temp_src.scalarFloat[vec_src_index]; \
        ret = temp_dest.vector; \
    }


// swizzle an element of a vector to another element of a vector
//
// ret - VectorIntrinsic
// in_vec0 - VectorIntrinsic
// in_index0_immediate - integer value (immediate value only)
// in_vec1 - VectorIntrinsic
// in_index1_immediate - integer value (immediate value only)
#define RW_MATH_VPU_SwizzleStore_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    ret = rwmath_vsetq_lane_f32( rwmath_vgetq_lane_f32(vec_src, vec_src_index_immediate), vec_dest, vec_dest_index_immediate )


#define RW_MATH_VPU_SwizzleStore_I_V(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion temp_src; \
        temp_src.vector = vec_src; \
        ret = rwmath_vsetq_lane_f32( temp_src.scalarFloat[vec_src_index], vec_dest, vec_dest_index_immediate); \
    }


#define RW_MATH_VPU_SwizzleStore_V_I(ret, vec_dest, vec_dest_index, vec_src, vec_src_index_immediate) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion temp_dest; \
        temp_dest.vector = vec_dest; \
        temp_dest.scalarFloat[vec_dest_index] = rwmath_vgetq_lane_f32(vec_src, vec_src_index_immediate); \
        ret = temp_dest.vector; \
    }


#define RW_MATH_VPU_SwizzleYZWX(ret, vec_in) \
    ret = rwmath_vextq_f32(vec_in, vec_in, 1)


//
// create a vector's xaxis that contain input vector's specified axis
// ret[0] = in_vec[in_index]
//
// ret - VectorIntrinsic
// in_vec0 - VectorIntrinsic
// in_index0_immediate - integer value (immediate value only)
// in_vec1 - VectorIntrinsic
// in_index1_immediate - integer value (immediate value only)
// in_vec2 - VectorIntrinsic
// in_index2_immediate - integer value (immediate value only)
// in_vec3 - VectorIntrinsic
// in_index3_immediate - integer value (immediate value only)
#define RW_MATH_VPU_CreateVectorAxisA1_I(ret, in_vec0, in_index0_immediate) \
    RW_MATH_VPU_SwizzleStore_I_I(ret, in_vec0, VectorAxisX, in_vec0, in_index0_immediate); \


#define RW_MATH_VPU_CreateVectorAxisA2_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate) \
    RW_MATH_VPU_SwizzleStore_I_I(ret, in_vec0, VectorAxisX, in_vec0, in_index0_immediate); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisY, in_vec1, in_index1_immediate); \


#define RW_MATH_VPU_CreateVectorAxisA3_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate) \
    RW_MATH_VPU_SwizzleStore_I_I(ret, in_vec0, VectorAxisX, in_vec0, in_index0_immediate); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisY, in_vec1, in_index1_immediate); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisZ, in_vec2, in_index2_immediate); \


#define RW_MATH_VPU_CreateVectorAxisA4_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate, in_vec3, in_index3_immediate) \
    RW_MATH_VPU_SwizzleStore_I_I(ret, in_vec0, VectorAxisX, in_vec0, in_index0_immediate); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisY, in_vec1, in_index1_immediate); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisZ, in_vec2, in_index2_immediate); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisW, in_vec3, in_index3_immediate); \


#define RW_MATH_VPU_GetZero(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 0.0f)


#define RW_MATH_VPU_GetHalf(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 0.5f)


#define RW_MATH_VPU_GetOne(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 1.0f)


#define RW_MATH_VPU_GetTwo(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 2.0f)


#define RW_MATH_VPU_GetNegOne(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, -1.0f)


#define RW_MATH_VPU_GetTrue(ret) \
    ret = vreinterpretq_f32_u32(vdupq_n_u32(0xffffffff))

#define RW_MATH_VPU_CompAnyTrueV3( ret, a ) \
    { \
        float32x4_t comp_temp = rwmath_vsetq_lane_f32(0.0f, a, 3); \
        RW_MATH_VPU_CompAnyTrueV4(ret, comp_temp); \
    }

#define RW_MATH_VPU_CompAllTrueV3( ret, a ) \
    { \
        float32x4_t comp_temp = vreinterpretq_f32_u32(vsetq_lane_u32(0xffffffffu, vreinterpretq_u32_f32(a), 3)); \
        RW_MATH_VPU_CompAllTrueV4(ret, comp_temp); \
    }

#define RW_MATH_VPU_CompAnyTrueV4( ret, a ) \
    { \
        uint32x4_t uintVec = vreinterpretq_u32_f32(a); \
        uint32x2_t combined = vorr_u32( vget_low_u32(uintVec), vget_high_u32(uintVec) ); \
        combined = vorr_u32( combined, vrev64_u32(combined) ); \
        ret = vreinterpretq_f32_u32(vcombine_u32(combined, combined)); \
    }

#define RW_MATH_VPU_CompAllTrueV4( ret, a ) \
    { \
        uint32x4_t uintVec = vreinterpretq_u32_f32(a); \
        uint32x2_t combined = vand_u32( vget_low_u32(uintVec), vget_high_u32(uintVec) ); \
        combined = vand_u32( combined, vrev64_u32(combined) ); \
        ret = vreinterpretq_f32_u32(vcombine_u32(combined, combined)); \
    }

#define RW_MATH_VPU_GetPI(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 3.14159265358979323846f)

#define RW_MATH_VPU_Get2PI(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 6.28318530717958f)

#define RW_MATH_VPU_Get2PIReciprocal(ret) \
    RW_MATH_VPU_CreateVectorV1(ret, 0.1591549430918953f)

#define RW_MATH_VPU_GetI(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 1.0f, 0.0f, 0.0f, 0.0f);


#define RW_MATH_VPU_GetJ(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 1.0f, 0.0f, 0.0f);


#define RW_MATH_VPU_GetK(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 0.0f, 1.0f, 0.0f);


#define RW_MATH_VPU_GetL(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 0.0f, 0.0f, 1.0f);


#define RW_MATH_VPU_GetSignMaskXYZW(ret) \
    ret = vreinterpretq_f32_u32(vdupq_n_u32(0x80000000))


#ifdef EA_PLATFORM_PSP2
    // SNC compiler bug workaround
    #define RW_MATH_VPU_GetSignMaskXYZ(ret) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion temp_mask;\
        temp_mask.scalarUInt[0] = 0x80000000; \
        temp_mask.scalarUInt[1] = 0x80000000; \
        temp_mask.scalarUInt[2] = 0x80000000; \
        temp_mask.scalarUInt[3] = 0x00000000; \
        ret = temp_mask.vector; \
    }
#else
    #define RW_MATH_VPU_GetSignMaskXYZ(ret) \
    { \
        uint32x4_t uintVec = {0x80000000, 0x80000000, 0x80000000, 0}; \
        ret = vreinterpretq_f32_u32(uintVec); \
    }
#endif



#define RW_MATH_VPU_GetSignMaskXZ(ret) \
    { \
        uint32x4_t uintVec = {0x80000000, 0, 0x80000000, 0}; \
        ret = vreinterpretq_f32_u32(uintVec); \
    }


#define RW_MATH_VPU_GetSignMaskYW(ret) \
    { \
        uint32x4_t uintVec = {0, 0x80000000, 0, 0x80000000}; \
        ret = vreinterpretq_f32_u32(uintVec); \
    }

// ret - float
// in_vec - VectorIntrinsic
// in_index_immediate - integer value (immediate value)
#define RW_MATH_VPU_GetFloatAxis_I(ret, in_vec, in_index_immediate) \
    ret = rwmath_vgetq_lane_f32(in_vec, in_index_immediate)

#define RW_MATH_VPU_Floor(ret, a1) \
    { \
        VectorIntrinsic zero = vdupq_n_f32(0.0f); \
        VectorIntrinsic one = vdupq_n_f32(1.0f); \
        VectorIntrinsic maxMantissa = vdupq_n_f32(8388608.0f); \
        VectorIntrinsic negMaxMantissa = vdupq_n_f32(-8388608.0f); \
        VectorIntrinsic isNegativeMask = RW_MATH_VPU_IsGreater_R( zero, a1); \
        VectorIntrinsic truncatingValue = RW_MATH_VPU_Select_R( negMaxMantissa, maxMantissa, isNegativeMask ); \
        VectorIntrinsic truncatedValue = RW_MATH_VPU_Subtract_R(a1, truncatingValue); \
        /* Note: Normally a FPU floor is done by subtracting and then adding a 'truncating' value to the input. */ \
        /* For an example of this, refer to the optimized PPU code for rw::math::fpu::Floor.  Unfortunately,    */ \
        /* this fails to work on the SPU for very large inputs.  Subtracting the truncating value leaves the    */ \
        /* input unchanged (expected) however, adding the truncating value back on introduces a small error.    */ \
        /* The SPU code works around this by checking to see if the input is modified when the truncating value */ \
        /* is subtracted from the input.  If it is not changed, then just return the original value since it    */ \
        /* is already "floored".                                                                                */ \
        VectorIntrinsic truncatedValueUnchanged = RW_MATH_VPU_IsEqual_R( truncatedValue, a1 ); \
        truncatedValue = RW_MATH_VPU_Select_R( RW_MATH_VPU_Add_R( truncatedValue, truncatingValue), a1, truncatedValueUnchanged ); \
        VectorIntrinsic subtractOneMask = RW_MATH_VPU_IsGreater_R( truncatedValue, a1); \
        VectorIntrinsic subtractValue; \
        RW_MATH_VPU_And( subtractValue, subtractOneMask, one ); \
        ret = RW_MATH_VPU_Subtract_R( truncatedValue, subtractValue ); \
    }

#define RW_MATH_VPU_Ceil(ret, a1) \
    { \
        VectorIntrinsic negA1, floorNegA1; \
        RW_MATH_VPU_Negate(negA1, a1); \
        RW_MATH_VPU_Floor(floorNegA1, negA1); \
        RW_MATH_VPU_Negate(ret, floorNegA1); \
    }

// Rounds the value towards zero. i.e. truncates the value.
#define RW_MATH_VPU_RoundToZero(ret, a1) \
    { \
        VectorIntrinsic zero = vdupq_n_f32(0.0f); \
        VectorIntrinsic maxMantissa = vdupq_n_f32(8388608.0f); \
        VectorIntrinsic negMaxMantissa = vdupq_n_f32(-8388608.0f); \
        VectorIntrinsic isNegativeMask = RW_MATH_VPU_IsGreater_R( zero, a1); \
        VectorIntrinsic truncatingValue = RW_MATH_VPU_Select_R( negMaxMantissa, maxMantissa, isNegativeMask ); \
        VectorIntrinsic truncatedValue = RW_MATH_VPU_Subtract_R(a1, truncatingValue); \
        /* Note: Normally a FPU truncation is done by subtracting and then adding a 'truncating' value to the input. */ \
        /* For an example of this, refer to the optimized PPU code for rw::math::fpu::Floor.  Unfortunately,         */ \
        /* this fails to work on the SPU for very large inputs.  Subtracting the truncating value leaves the         */ \
        /* input unchanged (expected) however, adding the truncating value back on introduces a small error.         */ \
        /* The SPU code works around this by checking to see if the input is modified when the truncating value      */ \
        /* is subtracted from the input.  If it is not changed, then just return the original value since it         */ \
        /* is already "truncated".                                                                                   */ \
        VectorIntrinsic truncatedValueUnchanged = RW_MATH_VPU_IsEqual_R( truncatedValue, a1 ); \
        ret = RW_MATH_VPU_Select_R( RW_MATH_VPU_Add_R( truncatedValue, truncatingValue), a1, truncatedValueUnchanged ); \
    }

#define RW_MATH_VPU_Mult(ret, in_vec0, in_vec1) \
    ret = vmulq_f32( in_vec0, in_vec1 );

#define RW_MATH_VPU_MultFloat(ret, in_vec, in_float) \
    ret = vmulq_n_f32( in_vec, (in_float) );

#define RW_MATH_VPU_SetWToZero(ret, in_vec ) \
    ret = rwmath_vsetq_lane_f32(0.0f, in_vec, 3);

#define RW_MATH_VPU_Negate(ret, in_vec) \
    ret = vnegq_f32(in_vec);


#define RW_MATH_VPU_NegateXZ(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic _asmNegateXZ_mask; \
        RW_MATH_VPU_GetSignMaskXZ(_asmNegateXZ_mask); \
        RW_MATH_VPU_Xor(ret, in_vec, _asmNegateXZ_mask); \
    }


#define RW_MATH_VPU_NegateYW(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic _asmNegateYW_mask; \
        RW_MATH_VPU_GetSignMaskYW(_asmNegateYW_mask); \
        RW_MATH_VPU_Xor(ret, in_vec, _asmNegateYW_mask); \
    }


#define RW_MATH_VPU_ProductDifference(ret, v0, v1, v2, v3) \
    { \
        VectorIntrinsic pd_temp; \
        RW_MATH_VPU_Mult(pd_temp, v0, v1); \
        RW_MATH_VPU_MultSubNeg(ret, v2, v3, pd_temp); \
    }


#define RW_MATH_VPU_ReciprocalEstimate(ret, in_vec) \
    ret = vrecpeq_f32(in_vec);


#define RW_MATH_VPU_ReciprocalFast(ret, in_vec) \
    { \
        /* Do one iteration of Newton-Raphson estimation */ \
        ::rw::math::vpu::VectorIntrinsic estimate; \
        RW_MATH_VPU_ReciprocalEstimate( estimate,  in_vec ); \
        ret = RWMATH_VPU_DETAIL_VecMul(estimate, vrecpsq_f32(estimate, in_vec)); \
    }


#define RW_MATH_VPU_Reciprocal(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic secondEstimate; \
        /* Do two iteration of Newton-Raphson estimation */ \
        RW_MATH_VPU_ReciprocalFast( secondEstimate,  in_vec ); \
        ret = RWMATH_VPU_DETAIL_VecMul(secondEstimate, vrecpsq_f32(secondEstimate, in_vec)); \
    }


#define RW_MATH_VPU_ReciprocalSquareRootEstimate(ret, in_vec) \
    ret = vrsqrteq_f32(in_vec)


#define RW_MATH_VPU_ReciprocalSquareRootFast(ret, in_vec) \
    { \
        /* Do one iteration of Newton-Raphson estimation */ \
        ::rw::math::vpu::VectorIntrinsic estimate; \
        RW_MATH_VPU_ReciprocalSquareRootEstimate(estimate, in_vec); \
        ret = RWMATH_VPU_DETAIL_VecMul(estimate, vrsqrtsq_f32(RWMATH_VPU_DETAIL_VecMul(estimate, estimate), in_vec)); \
    }


#define RW_MATH_VPU_ReciprocalSquareRoot(ret, in_vec) \
    { \
        /* Do two iterations of Newton-Raphson estimation */ \
        ::rw::math::vpu::VectorIntrinsic secondEstimate; \
        RW_MATH_VPU_ReciprocalSquareRootFast( secondEstimate, in_vec ); \
        ret = RWMATH_VPU_DETAIL_VecMul(secondEstimate, vrsqrtsq_f32(RWMATH_VPU_DETAIL_VecMul(secondEstimate, secondEstimate), in_vec)); \
    }


#if __llvm__

#define RW_MATH_VPU_SquareRoot(ret, in_vec) \
    ret = rw::math::vpu::detail::VPU_SquareRoot(in_vec)

#define RW_MATH_VPU_SquareRootFast(ret, in_vec) \
    ret = rw::math::vpu::detail::VPU_SquareRootFast(in_vec)

#define RW_MATH_VPU_SquareRootEstimate(ret, in_vec) \
    ret = rw::math::vpu::detail::VPU_SquareRootEstimate(in_vec)

#else
// Note: It is difficult to directly compute the squareroot of a value using VMX, so this function
// computes x * x^(-1/2).  Of course this will give a mathematically undefined result for x = 0, so
// the code has a comparison that returns 0 if x = 0.
#define RW_MATH_VPU_SquareRoot(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic zero = RW_MATH_VPU_GetZero_R(); \
        ::rw::math::vpu::VectorIntrinsic isZeroMask; \
        RW_MATH_VPU_IsEqual( isZeroMask, zero, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic result = RWMATH_VPU_DETAIL_VecMul(in_vec, recipSqrt ); \
        ret = RW_MATH_VPU_Select_R( result, zero, isZeroMask ); \
    }

#define RW_MATH_VPU_SquareRootFast(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic zero = RW_MATH_VPU_GetZero_R(); \
        ::rw::math::vpu::VectorIntrinsic isZeroMask; \
        RW_MATH_VPU_IsEqual( isZeroMask, zero, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRootFast( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic result; \
        RW_MATH_VPU_Mult( result, in_vec, recipSqrt ); \
        ret = RW_MATH_VPU_Select_R( result, zero, isZeroMask ); \
    }

#define RW_MATH_VPU_SquareRootEstimate(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic zero = RW_MATH_VPU_GetZero_R(); \
        ::rw::math::vpu::VectorIntrinsic isZeroMask; \
        RW_MATH_VPU_IsEqual( isZeroMask, zero, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic result; \
        RW_MATH_VPU_Mult( result, in_vec, recipSqrt ); \
        ret = RW_MATH_VPU_Select_R( result, zero, isZeroMask ); \
    }
#endif


#define RW_MATH_VPU_Divide(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        RW_MATH_VPU_Reciprocal( reciprocal, in_vec1 ); \
        ret = RWMATH_VPU_DETAIL_VecMul( reciprocal, in_vec0 ); \
    }


#define RW_MATH_VPU_DivideFloat(ret, in_vec, in_float) \
    { \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        float floatRecip = 1.0f/(in_float); \
        RW_MATH_VPU_CreateVectorV1( reciprocal, floatRecip ); \
        ret = RWMATH_VPU_DETAIL_VecMul( reciprocal, in_vec ); \
    }


// ret = in_vec0 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_DivideAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic splattedAxis = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( in_vec_scale, in_index_immediate ); \
        RW_MATH_VPU_Divide( ret, in_vec0, splattedAxis ); \
    } \

// ret0 = in_vec0 / in_vec_scale[in_index_immediate];
// ret1 = in_vec1 / in_vec_scale[in_index_immediate];
// ret2 = in_vec2 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_DivideAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic splattedAxis = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( in_vec_scale, in_index_immediate ); \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        RW_MATH_VPU_Reciprocal( reciprocal, splattedAxis ); \
        ret0 = RWMATH_VPU_DETAIL_VecMul( in_vec0, reciprocal ); \
        ret1 = RWMATH_VPU_DETAIL_VecMul( in_vec1, reciprocal ); \
        ret2 = RWMATH_VPU_DETAIL_VecMul( in_vec2, reciprocal ); \
    } \


// ret = in_vec0 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic splattedAxis = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( in_vec_scale, in_index_immediate ); \
        ret = RWMATH_VPU_DETAIL_VecMul( in_vec0, splattedAxis ); \
    } \


// ret0 = in_vec0 * in_vec_scale[in_index_immediate];
// ret1 = in_vec1 * in_vec_scale[in_index_immediate];
// ret2 = in_vec2 * in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic splattedAxis = RWMATH_VPU_DETAIL_VecSplat( in_vec_scale, in_index_immediate ); \
        ret0 = RWMATH_VPU_DETAIL_VecMul( in_vec0, splattedAxis ); \
        ret1 = RWMATH_VPU_DETAIL_VecMul( in_vec1, splattedAxis ); \
        ret2 = RWMATH_VPU_DETAIL_VecMul( in_vec2, splattedAxis ); \
    } \


#define RW_MATH_VPU_MultAdd(ret, in_vec0, in_vec1, in_vec2) \
    ret = RWMATH_VPU_DETAIL_VecMadd( in_vec0, in_vec1, in_vec2 )


#define RW_MATH_VPU_MultSubNeg(ret, in_vec0, in_vec1, in_vec2) \
    ret = RWMATH_VPU_DETAIL_VecNmsub(in_vec0, in_vec1, in_vec2)


#define RW_MATH_VPU_Add(ret, in_vec0, in_vec1) \
    ret = vaddq_f32(in_vec0, in_vec1)


#define RW_MATH_VPU_Subtract(ret, in_vec0, in_vec1) \
    ret = vsubq_f32(in_vec0, in_vec1)


#define RW_MATH_VPU_And(ret, in_vec0, in_vec1) \
    ret = vreinterpretq_f32_u32(vandq_u32( vreinterpretq_u32_f32(in_vec0), vreinterpretq_u32_f32(in_vec1) ))

#define RW_MATH_VPU_And_R(in_vec0, in_vec1) \
    vreinterpretq_f32_u32(vandq_u32( vreinterpretq_u32_f32(in_vec0), vreinterpretq_u32_f32(in_vec1) ))

#define RW_MATH_VPU_And_Rc(in_vec0, in_vec1) \
    vreinterpretq_f32_u32(vbicq_u32( vreinterpretq_u32_f32(in_vec0), vreinterpretq_u32_f32(in_vec1) ))

#define RW_MATH_VPU_Or(ret, in_vec0, in_vec1) \
    ret = vreinterpretq_f32_u32(vorrq_u32( vreinterpretq_u32_f32(in_vec0), vreinterpretq_u32_f32(in_vec1) ))


#define RW_MATH_VPU_Not(ret, in_vec) \
    ret = vreinterpretq_f32_u32(vmvnq_u32( vreinterpretq_u32_f32(in_vec) ))

#define RW_MATH_VPU_Xor_R(in_vec0, in_vec1) \
    vreinterpretq_f32_u32(veorq_u32( vreinterpretq_u32_f32(in_vec0), vreinterpretq_u32_f32(in_vec1) ))

#define RW_MATH_VPU_Xor(ret, in_vec0, in_vec1) \
    ret = vreinterpretq_f32_u32(veorq_u32( vreinterpretq_u32_f32(in_vec0), vreinterpretq_u32_f32(in_vec1) ))


#define RW_MATH_VPU_ProductSum3(ret, v0, v1, v2, v3, v4, v5) \
    RW_MATH_VPU_Mult   (ret, v0, v1); \
    RW_MATH_VPU_MultAdd(ret, v2, v3, ret); \
    RW_MATH_VPU_MultAdd(ret, v4, v5, ret); \


// sets ret to a bool value indicating whether or not all elements of in_uint32x2 are true.
#define RW_MATH_DETAIL_AllTrueInt2(ret, in_uint32x2) \
    { \
        uint64_t scalar = vget_lane_u64( vreinterpret_u64_u32(in_uint32x2), 0 ); \
        ret = scalar == 0xffffffffffffffffull; \
    }

// sets ret to a bool value indicating whether or not all elements of in_uint32x2 are false.
#define RW_MATH_DETAIL_AllFalseInt2(ret, in_uint32x2) \
    { \
        uint64_t scalar = vget_lane_u64( vreinterpret_u64_u32(in_uint32x2), 0 ); \
        ret = scalar == 0; \
    }

// sets ret to a bool value indicating whether or not any elements of in_uint32x2 are true.
#define RW_MATH_DETAIL_AnyTrueInt2(ret, in_uint32x2) \
    { \
        uint64_t scalar = vget_lane_u64( vreinterpret_u64_u32(in_uint32x2), 0 ); \
        ret = scalar != 0; \
    }

// sets ret to a bool value indicating whether or not any elements of in_uint32x2 are false.
#define RW_MATH_DETAIL_AnyFalseInt2(ret, in_uint32x2) \
    { \
        uint64_t scalar = vget_lane_u64( vreinterpret_u64_u32(in_uint32x2), 0 ); \
        ret = scalar != 0xffffffffffffffffull; \
    }

#define RW_MATH_VPU_IsEqualV2(ret, in_vec0, in_vec1) \
    { \
        uint32x2_t compResult = vceq_f32( vget_low_f32(in_vec0), vget_low_f32(in_vec1) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, compResult); \
    }


#define RW_MATH_VPU_IsEqualV3(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic isequal_temp0, isequal_temp1; \
        RW_MATH_VPU_IsEqual( isequal_temp0, in_vec0, in_vec1 ); \
        RW_MATH_VPU_CompAllTrueV3(isequal_temp1, isequal_temp0); \
        RW_MATH_DETAIL_AllTrueInt2(ret, vreinterpret_u32_f32(vget_low_f32(isequal_temp1))); \
    }


#define RW_MATH_VPU_IsEqualV4(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic isequal_temp0, isequal_temp1; \
        RW_MATH_VPU_IsEqual( isequal_temp0, in_vec0, in_vec1 ); \
        RW_MATH_VPU_CompAllTrueV4(isequal_temp1, isequal_temp0); \
        RW_MATH_DETAIL_AllTrueInt2(ret, vreinterpret_u32_f32(vget_low_f32(isequal_temp1))); \
    }


#define RW_MATH_VPU_Abs(ret, in_vec) \
    ret = vabsq_f32(in_vec)


#define RW_MATH_VPU_IsZeroA1V2(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Abs(temp_v, in_vec); \
        uint32x2_t xyResult = vreinterpret_u32_f32(vget_low_f32(RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, temp_v))); \
        RW_MATH_DETAIL_AllTrueInt2(ret, xyResult); \
    }


#define RW_MATH_VPU_IsZeroA1V3(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Abs(temp_v, in_vec); \
        uint32x4_t mask = vreinterpretq_u32_f32(RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, temp_v)); \
        uint32x2_t halfMask = vand_u32( vget_low_u32(mask), vdup_lane_u32(vget_high_u32(mask),0) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, halfMask); \
    }


#define RW_MATH_VPU_IsZeroA1V4(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Abs(temp_v, in_vec); \
        VectorIntrinsic mask = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, temp_v); \
        RW_MATH_VPU_IsAllTrue(ret, mask); \
    }


#define RW_MATH_VPU_IsZeroA2V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic abs_0, abs_1, maskCombined; \
        RW_MATH_VPU_Abs(abs_0, in_vec0); \
        RW_MATH_VPU_Abs(abs_1, in_vec1); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        RW_MATH_VPU_And(maskCombined, mask0, mask1); \
        RW_MATH_VPU_IsAllTrue(ret, mask); \
    }


#define RW_MATH_VPU_IsZeroA3V3(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic abs_0, abs_1, abs_2, maskCombined; \
        RW_MATH_VPU_Abs(abs_0, in_vec0); \
        RW_MATH_VPU_Abs(abs_1, in_vec1); \
        RW_MATH_VPU_Abs(abs_2, in_vec2); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        RW_MATH_VPU_And(maskCombined, mask0, mask1); \
        RW_MATH_VPU_And(maskCombined, maskCombined, mask2); \
        uint32x4_t maskUint = vreinterpretq_u32_f32(maskCombined); \
        uint32x2_t halfMask = vand_u32( vget_low_u32(maskUint), vdup_lane_u32(vget_high_u32(maskUint),0) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, halfMask); \
    }


#define RW_MATH_VPU_IsZeroA3V4(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic abs_0, abs_1, abs_2, maskCombined; \
        RW_MATH_VPU_Abs(abs_0, in_vec0); \
        RW_MATH_VPU_Abs(abs_1, in_vec1); \
        RW_MATH_VPU_Abs(abs_2, in_vec2); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        RW_MATH_VPU_And(maskCombined, mask0, mask1); \
        RW_MATH_VPU_And(maskCombined, maskCombined, mask2); \
        RW_MATH_VPU_IsAllTrue(ret, maskCombined); \
    }


#define RW_MATH_VPU_IsZeroA4V3(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic abs_0, abs_1, abs_2, abs_3, mask01, mask23, maskCombined; \
        RW_MATH_VPU_Abs(abs_0, in_vec0); \
        RW_MATH_VPU_Abs(abs_1, in_vec1); \
        RW_MATH_VPU_Abs(abs_2, in_vec2); \
        RW_MATH_VPU_Abs(abs_3, in_vec3); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        VectorIntrinsic mask3 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_3); \
        RW_MATH_VPU_And(mask01, mask0, mask1); \
        RW_MATH_VPU_And(mask23, mask2, mask3); \
        RW_MATH_VPU_And(maskCombined, mask01, mask23); \
        uint32x4_t maskUint = vreinterpretq_u32_f32(maskCombined); \
        uint32x2_t halfMask = vand_u32( vget_low_u32(maskUint), vdup_lane_u32(vget_high_u32(maskUint),0) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, halfMask); \
    }


#define RW_MATH_VPU_IsZeroA4V4(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic abs_0, abs_1, abs_2, abs_3, mask01, mask23, maskCombined; \
        RW_MATH_VPU_Abs(abs_0, in_vec0); \
        RW_MATH_VPU_Abs(abs_1, in_vec1); \
        RW_MATH_VPU_Abs(abs_2, in_vec2); \
        RW_MATH_VPU_Abs(abs_3, in_vec3); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        VectorIntrinsic mask3 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_3); \
        RW_MATH_VPU_And(mask01, mask0, mask1); \
        RW_MATH_VPU_And(mask23, mask2, mask3); \
        RW_MATH_VPU_And(maskCombined, mask01, mask23); \
        RW_MATH_VPU_IsAllTrue(ret, maskCombined); \
    }


#define RW_MATH_VPU_IsSimilarA1V2(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Subtract(temp_v, in_vec0, in_vec1); \
        RW_MATH_VPU_Abs(temp_v, temp_v); \
        uint32x2_t xyResult = vreinterpret_u32_f32(vget_low_f32(RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, temp_v))); \
        RW_MATH_DETAIL_AllTrueInt2(ret, xyResult); \
    }


#define RW_MATH_VPU_IsSimilarA1V3(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Subtract(temp_v, in_vec0, in_vec1); \
        RW_MATH_VPU_Abs(temp_v, temp_v); \
        uint32x4_t mask = vreinterpretq_u32_f32(RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, temp_v)); \
        uint32x2_t halfMask = vand_u32( vget_low_u32(mask), vdup_lane_u32(vget_high_u32(mask),0) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, halfMask); \
    }


#define RW_MATH_VPU_IsSimilarA1V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Subtract(temp_v, in_vec0, in_vec1); \
        RW_MATH_VPU_Abs(temp_v, temp_v); \
        VectorIntrinsic mask = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, temp_v); \
        RW_MATH_VPU_IsAllTrue(ret, mask); \
    }


#define RW_MATH_VPU_IsSimilarA3V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x, temp_y, temp_z, abs_0, abs_1, abs_2, maskCombined; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Abs(abs_0, temp_x); \
        RW_MATH_VPU_Abs(abs_1, temp_y); \
        RW_MATH_VPU_Abs(abs_2, temp_z); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        RW_MATH_VPU_And(maskCombined, mask0, mask1); \
        RW_MATH_VPU_And(maskCombined, maskCombined, mask2); \
        uint32x4_t maskUint = vreinterpretq_u32_f32(maskCombined); \
        uint32x2_t halfMask = vand_u32( vget_low_u32(maskUint), vdup_lane_u32(vget_high_u32(maskUint),0) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, halfMask); \
    }


#define RW_MATH_VPU_IsSimilarA4V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x, temp_y, temp_z, temp_w, abs_0, abs_1, abs_2, abs_3, mask01, mask23, maskCombined; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Subtract(temp_w, in_vec0_w, in_vec1_w); \
        RW_MATH_VPU_Abs(abs_0, temp_x); \
        RW_MATH_VPU_Abs(abs_1, temp_y); \
        RW_MATH_VPU_Abs(abs_2, temp_z); \
        RW_MATH_VPU_Abs(abs_3, temp_w); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        VectorIntrinsic mask3 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_3); \
        RW_MATH_VPU_And(mask01, mask0, mask1); \
        RW_MATH_VPU_And(mask23, mask2, mask3); \
        RW_MATH_VPU_And(maskCombined, mask01, mask23); \
        uint32x4_t maskUint = vreinterpretq_u32_f32(maskCombined); \
        uint32x2_t halfMask = vand_u32( vget_low_u32(maskUint), vdup_lane_u32(vget_high_u32(maskUint),0) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, halfMask); \
    }


#define RW_MATH_VPU_IsSimilarA3V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x, temp_y, temp_z, abs_0, abs_1, abs_2, maskCombined; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Abs(abs_0, temp_x); \
        RW_MATH_VPU_Abs(abs_1, temp_y); \
        RW_MATH_VPU_Abs(abs_2, temp_z); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        RW_MATH_VPU_And(maskCombined, mask0, mask1); \
        RW_MATH_VPU_And(maskCombined, maskCombined, mask2); \
        RW_MATH_VPU_IsAllTrue(ret, maskCombined); \
    }


#define RW_MATH_VPU_IsSimilarA4V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x, temp_y, temp_z, temp_w, abs_0, abs_1, abs_2, abs_3, mask01, mask23, maskCombined; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Subtract(temp_w, in_vec0_w, in_vec1_w); \
        RW_MATH_VPU_Abs(abs_0, temp_x); \
        RW_MATH_VPU_Abs(abs_1, temp_y); \
        RW_MATH_VPU_Abs(abs_2, temp_z); \
        RW_MATH_VPU_Abs(abs_3, temp_w); \
        VectorIntrinsic mask0 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_0); \
        VectorIntrinsic mask1 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_1); \
        VectorIntrinsic mask2 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_2); \
        VectorIntrinsic mask3 = RW_MATH_VPU_IsGreaterEqual_R(in_tolerance, abs_3); \
        RW_MATH_VPU_And(mask01, mask0, mask1); \
        RW_MATH_VPU_And(mask23, mask2, mask3); \
        RW_MATH_VPU_And(maskCombined, mask01, mask23); \
        RW_MATH_VPU_IsAllTrue(ret, maskCombined); \
    }


#define RW_MATH_VPU_CrossV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic crossV1, crossV2; \
        RW_MATH_VPU_V3_RotateLeft( crossV2, in_vec1 ); \
        RW_MATH_VPU_V3_RotateLeft( crossV1, in_vec0 ); \
        ::rw::math::vpu::VectorIntrinsic crossV3  = RWMATH_VPU_DETAIL_VecMul(in_vec0, crossV2); \
        crossV3 = RWMATH_VPU_DETAIL_VecNmsub(crossV1, in_vec1, crossV3); \
        RW_MATH_VPU_V3_RotateLeft(ret, crossV3); \
        ret = rwmath_vsetq_lane_f32( 0.0f, ret, 3 ); \
    }


#define RW_MATH_VPU_DotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float32x2_t lowHalf = vget_low_f32(temp_v); \
        ret = vdupq_n_f32(vget_lane_f32(lowHalf,0)+vget_lane_f32(lowHalf,1)); \
    }


#define RW_MATH_VPU_DotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float32x2_t lowHalf = vget_low_f32(temp_v); \
        float32x2_t leftPSum = vpadd_f32(lowHalf, lowHalf/*don't care value*/ ); \
        ret = vdupq_n_f32(vget_lane_f32(leftPSum,0)+vget_lane_f32(vget_high_f32(temp_v),0)); \
    }


#define RW_MATH_VPU_DotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float32x2_t leftPSum = vpadd_f32(vget_low_f32(temp_v), vget_high_f32(temp_v)); \
        ret = vdupq_n_f32(vget_lane_f32(leftPSum,0)+vget_lane_f32(leftPSum,1)); \
    }


#define RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootFastDotV2(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootEstimateDotV2(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootFastDotV3(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootFastDotV4(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootEstimateDotV4(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootFast( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRoot( ret, dotproduct ); \
    }


#define RW_MATH_VPU_ReciprocalSquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootFast( ret, dotproduct ); \
    }

#define RW_MATH_VPU_NormalizeV2(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV2(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeEstimateV2(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV2(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeFastV2(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV2(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeV3(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV3(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeEstimateV3(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeFastV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

#define RW_MATH_VPU_NormalizeEstimateV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    }

//NOTE the last several lines in this macro will zero the magnitude (ret_mag) if in_vec is zero (done elementwise)
#define RW_MATH_VPU_NormalizeWithMagnitudeV2(ret, ret_mag, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV2(temp_dot, in_vec, in_vec); \
        ::rw::math::vpu::VectorIntrinsic one_over_rqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot(one_over_rqrt, temp_dot); \
        ::rw::math::vpu::VectorIntrinsic result_mag; \
        RW_MATH_VPU_Mult(result_mag, temp_dot, one_over_rqrt); \
        RW_MATH_VPU_Mult(ret, in_vec, one_over_rqrt); \
        ::rw::math::vpu::VectorIntrinsic zero; \
        RW_MATH_VPU_GetZero(zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero, input_is_zero); \
    }


//NOTE the last several lines in this macro will zero the magnitude (ret_mag) if in_vec is zero (done elementwise)
#define RW_MATH_VPU_NormalizeWithMagnitudeV3(ret, ret_mag, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV3(temp_dot, in_vec, in_vec); \
        ::rw::math::vpu::VectorIntrinsic one_over_rqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot(one_over_rqrt, temp_dot); \
        ::rw::math::vpu::VectorIntrinsic result_mag; \
        RW_MATH_VPU_Mult(result_mag, temp_dot, one_over_rqrt); \
        RW_MATH_VPU_Mult(ret, in_vec, one_over_rqrt); \
        ::rw::math::vpu::VectorIntrinsic zero; \
        RW_MATH_VPU_GetZero(zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero, input_is_zero); \
    }

//NOTE the last several lines in this macro will zero the magnitude (ret_mag) if in_vec is zero (done elementwise)
#define RW_MATH_VPU_NormalizeWithMagnitudeV4(ret, ret_mag, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV4(temp_dot, in_vec, in_vec); \
        ::rw::math::vpu::VectorIntrinsic one_over_rqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot(one_over_rqrt, temp_dot); \
        ::rw::math::vpu::VectorIntrinsic result_mag; \
        RW_MATH_VPU_Mult(result_mag, temp_dot, one_over_rqrt); \
        RW_MATH_VPU_Mult(ret, in_vec, one_over_rqrt); \
        ::rw::math::vpu::VectorIntrinsic zero; \
        RW_MATH_VPU_GetZero(zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero, input_is_zero); \
    }


#define RW_MATH_VPU_Lerp(ret, in_vec0, in_vec1, in_t) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_lerp_v; \
        RW_MATH_VPU_Subtract(temp_lerp_v, in_vec1, in_vec0); \
        RW_MATH_VPU_MultAdd(ret, temp_lerp_v, in_t, in_vec0); \
    }

#define RWMATH_VPU_DETAIL_VecSplat(in_vec0, in_index_immediate) \
    rw::math::vpu::detail::NeonSplatElement<in_index_immediate>(in_vec0)

#define RW_MATH_VPU_Splat_I(ret, in_vec0, in_index_immediate) \
    ret = rw::math::vpu::detail::NeonSplatElement<in_index_immediate>(in_vec0)


#define RW_MATH_VPU_Splat(ret, in_vec0, in_index) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion temp_src; \
        temp_src.vector = in_vec0; \
        ret = vdupq_n_f32(temp_src.scalarFloat[in_index]); \
    }

#define RW_MATH_VPU_TransformVectorV3(ret, in_vec, in_x, in_y, in_z) \
    { \
        ::rw::math::vpu::VectorIntrinsic xSplat, ySplat, zSplat; \
        RW_MATH_VPU_Splat_I( xSplat, in_vec, 0 ); \
        RW_MATH_VPU_Splat_I( ySplat, in_vec, 1 ); \
        RW_MATH_VPU_Splat_I( zSplat, in_vec, 2 ); \
           RW_MATH_VPU_Mult( ret, in_x, xSplat ); \
        RW_MATH_VPU_MultAdd( ret, in_y, ySplat, ret ); \
        RW_MATH_VPU_MultAdd( ret, in_z, zSplat, ret ); \
    }

#define RW_MATH_VPU_ExtractXElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    { \
        float32x2x2_t ul = vtrn_f32( vget_low_f32(in_x), vget_low_f32(in_y) ); \
        ret = vcombine_f32(ul.val[0], vget_low_f32(in_z)); \
    }

#define RW_MATH_VPU_ExtractYElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    { \
        float32x2x2_t ul = vtrn_f32( vget_low_f32(in_x), vget_low_f32(in_y) ); \
        ret = vcombine_f32(ul.val[1], vdup_lane_f32(vget_low_f32(in_z),1)); \
    }

#define RW_MATH_VPU_ExtractZElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    { \
        float32x2x2_t ur = vtrn_f32( vget_high_f32(in_x), vget_high_f32(in_y) ); \
        ret = vcombine_f32(ur.val[0], vget_high_f32(in_z)); \
    }


// The following implementation was tried for
// RW_MATH_VPU_ExtractXElementsAsVectorA4V4 but it seems to be incorrectly
// compiled by GCC on iPhone in opt builds.
//        float32x2x2_t ul = vtrn_f32( vget_low_f32(in_x), vget_low_f32(in_y) );
//        float32x2x2_t ll = vtrn_f32( vget_low_f32(in_z), vget_low_f32(in_w) );
//        ret = vcombine_f32(ul.val[0], ll.val[0]);
// So a more straightforward approach was taken below.
#define RW_MATH_VPU_ExtractXElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    ret = (float32x4_t){ rwmath_vgetq_lane_f32(in_x, 0), rwmath_vgetq_lane_f32(in_y, 0), rwmath_vgetq_lane_f32(in_z, 0), rwmath_vgetq_lane_f32(in_w, 0) }


#define RW_MATH_VPU_ExtractYElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        float32x2x2_t ul = vtrn_f32( vget_low_f32(in_x), vget_low_f32(in_y) ); \
        float32x2x2_t ll = vtrn_f32( vget_low_f32(in_z), vget_low_f32(in_w) ); \
        ret = vcombine_f32(ul.val[1], ll.val[1]); \
    }


#define RW_MATH_VPU_ExtractZElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        float32x2x2_t ur = vtrn_f32( vget_high_f32(in_x), vget_high_f32(in_y) ); \
        float32x2x2_t lr = vtrn_f32( vget_high_f32(in_z), vget_high_f32(in_w) ); \
        ret = vcombine_f32(ur.val[0], lr.val[0]); \
    }


#define RW_MATH_VPU_ExtractWElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        float32x2x2_t ur = vtrn_f32( vget_high_f32(in_x), vget_high_f32(in_y) ); \
        float32x2x2_t lr = vtrn_f32( vget_high_f32(in_z), vget_high_f32(in_w) ); \
        ret = vcombine_f32(ur.val[1], lr.val[1]); \
    }


#define RW_MATH_VPU_TransposeA3V3(ret_x, ret_y, ret_z, in_x, in_y, in_z) \
    { \
        float32x2x2_t ul = vtrn_f32( vget_low_f32(in_x), vget_low_f32(in_y) ); \
        float32x2x2_t ur = vtrn_f32( vget_high_f32(in_x), vget_high_f32(in_y) ); \
        ret_x = vcombine_f32(ul.val[0], vget_low_f32(in_z)); \
        ret_y = vcombine_f32(ul.val[1], vdup_lane_f32(vget_low_f32(in_z),1)); \
        ret_z = vcombine_f32(ur.val[0], vget_high_f32(in_z)); \
    }


// RW_MATH_VPU_TransposeA4V4 now calls a function since the code generated by the macro sometimes failed when inlining.
#define RW_MATH_VPU_TransposeA4V4(ret_x, ret_y, ret_z, ret_w, in_x, in_y, in_z, in_w) \
    ::rw::math::vpu::detail::TransposeA4V4(ret_x, ret_y, ret_z, ret_w, in_x, in_y, in_z, in_w)

#ifdef RWMATH_USE_NEON_INLINE_ASSEMBLY
#define RW_MATH_VPU_TransposeMatrix44(outputPtr, inputPtr)  \
    { \
        void *inputPtrCopy = (void*)(inputPtr);                 \
        asm volatile (/* Load and transpose simultaneously*/    \
            "vld4.f32 {d0,d2,d4,d6}, [%0]!      \n"             \
            "vld4.f32 {d1,d3,d5,d7}, [%0]       \n"             \
            /* Store to result */                               \
            "vstmia %1, {d0-d7}"                                \
            :"+r"(inputPtrCopy)                                 \
            :"r"(outputPtr)                                     \
            :"memory", "q0", "q1", "q2", "q3"                   \
        );                                                      \
    }
#endif

#define RW_MATH_VPU_TransposeA4V3(ret_x, ret_y, ret_z, in_x, in_y, in_z, in_w) \
    { \
        float32x2x2_t ul = vtrn_f32( vget_low_f32(in_x), vget_low_f32(in_y) ); \
        float32x2x2_t ur = vtrn_f32( vget_high_f32(in_x), vget_high_f32(in_y) ); \
        float32x2x2_t ll = vtrn_f32( vget_low_f32(in_z), vget_low_f32(in_w) ); \
        float32x2x2_t lr = vtrn_f32( vget_high_f32(in_z), vget_high_f32(in_w) ); \
        ret_x = vcombine_f32(ul.val[0], ll.val[0]); \
        ret_y = vcombine_f32(ul.val[1], ll.val[1]); \
        ret_z = vcombine_f32(ur.val[0], lr.val[0]); \
    }


#define RW_MATH_VPU_InverseA3V3(ret_d, ret_x, ret_y, ret_z, in_x, in_y, in_z) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_inverse_crossYZ; \
        ::rw::math::vpu::VectorIntrinsic temp_inverse_crossZX; \
        ::rw::math::vpu::VectorIntrinsic temp_inverse_crossXY; \
        RW_MATH_VPU_CrossV3(temp_inverse_crossYZ, in_y, in_z); \
        RW_MATH_VPU_CrossV3(temp_inverse_crossZX, in_z, in_x); \
        RW_MATH_VPU_CrossV3(temp_inverse_crossXY, in_x, in_y); \
        RW_MATH_VPU_DotV3(ret_d, in_x, temp_inverse_crossYZ); \
        ::rw::math::vpu::VectorIntrinsic temp_inverse_x; \
        ::rw::math::vpu::VectorIntrinsic temp_inverse_y; \
        ::rw::math::vpu::VectorIntrinsic temp_inverse_z; \
        RW_MATH_VPU_TransposeA3V3(temp_inverse_x, temp_inverse_y, temp_inverse_z, temp_inverse_crossYZ, temp_inverse_crossZX, temp_inverse_crossXY); \
        RW_MATH_VPU_DivideAxisA3_I(ret_x, ret_y, ret_z, temp_inverse_x, temp_inverse_y, temp_inverse_z, ret_d, VectorAxisX); \
    } \


#define RW_MATH_VPU_InverseA4V3(ret_d, ret_x, ret_y, ret_z, ret_w, in_x, in_y, in_z, in_w) \
    { \
        using namespace detail; \
        VectorIntrinsic negatePos; \
        RW_MATH_VPU_Negate(negatePos, in_w); \
        \
        /* Invert the upper left 3x3 submatrix. */                                                  \
        /*                                                                                          \
         * Note: The determinant of a Matrix44Affine is equal to the determinant of                 \
         * the upper left 3x3 submatrix.                                                            \
         *                                                                                          \
         * Proof:                                                                                   \
         *                                                                                          \
         * (Useful reference is found at http://mathworld.wolfram.com/Determinant.html)             \
         *                                                                                          \
         * The determinant of a matrix can be computed as:                                          \
         * det = sum from i=i to k of a_ij x C_ij        (Where C_ij is the cofactor of a_ij)       \
         *                                                                                          \
         * Lets choose j to be 4.                                                                   \
         * det = sum from i=i to k of a_i4 x C_i4        (Where C_ij is the cofactor of a_ij)       \
         *                                                                                          \
         * For an affine 4x4 matrix a_i1, a_i2, and a_i3 are 0, and a_i4 is 1.  So, the sum         \
         * can be rewritten as simply:                                                              \
         *                                                                                          \
         * det = C_4,4                                                                              \
         *     = -1^(4+4) x M_4,4                         (Where M_ij is the minor of a_ij)         \
         *     = M_4,4                                                                              \
         *                                                                                          \
         * Of course, this particular minor is just the determinant of the upper left 3x3 matrix.   \
         */                                                                                         \
        RW_MATH_VPU_InverseA3V3( ret_d, ret_x, ret_y, ret_z, in_x, in_y, in_z );  \
        \
        /* Now compute the translation value for the result that will cancel                        \
         * the translation component of the input.                                                  \
         */ \
        VectorIntrinsic transX, transY, transZ; \
        RW_MATH_VPU_Splat_I(transX, negatePos, VectorAxisX); \
        RW_MATH_VPU_Splat_I(transY, negatePos, VectorAxisY); \
        RW_MATH_VPU_Splat_I(transZ, negatePos, VectorAxisZ); \
        RW_MATH_VPU_Mult(ret_w, transX, ret_x); \
        RW_MATH_VPU_MultAdd(ret_w, transY, ret_y, ret_w); \
        RW_MATH_VPU_MultAdd(ret_w, transZ, ret_z, ret_w); \
    }


#define RW_MATH_VPU_MatrixFromQuaternion(ret_x, ret_y, ret_z, in_q) \
    { \
        ::rw::math::vpu::VectorIntrinsic sqrt2s = vdupq_n_f32(1.41421356237f); \
        ::rw::math::vpu::VectorIntrinsic scaled_q = RWMATH_VPU_DETAIL_VecMul(sqrt2s, in_q ); \
        ::rw::math::vpu::VectorIntrinsic halves = vdupq_n_f32(0.5f); \
        ::rw::math::vpu::VectorIntrinsic neg_qq = RWMATH_VPU_DETAIL_VecNmsub( scaled_q, scaled_q, halves ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_yzx; \
        RW_MATH_VPU_V3_RotateLeft( scaled_q_yzx, scaled_q ); \
        ::rw::math::vpu::VectorIntrinsic xy_yz_zx = RWMATH_VPU_DETAIL_VecMul( scaled_q, scaled_q_yzx ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_zxy; \
        RW_MATH_VPU_V3_RotateRight( scaled_q_zxy, scaled_q ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_w; \
        RW_MATH_VPU_Splat_I(scaled_q_w, scaled_q, VectorAxisW); \
        ::rw::math::vpu::VectorIntrinsic wz_wx_wy = RWMATH_VPU_DETAIL_VecMul(scaled_q_w, scaled_q_zxy); \
        /*                                                      \
           Calculate:                                           \
             addVec = (xy + wz, yz + wx, zx + wy) and           \
             subVec = (xy - wz, yz - wx, zx - wy)               \
         */ \
        ::rw::math::vpu::VectorIntrinsic addVec = RW_MATH_VPU_Add_R( xy_yz_zx, wz_wx_wy ); \
        ::rw::math::vpu::VectorIntrinsic subVec = RW_MATH_VPU_Subtract_R( xy_yz_zx, wz_wx_wy ); \
        /* Calculate */                                                         \
        /* diagonalValues = ( neg_xx+neg_yy, neg_yy+neg_zz, neg_zz+neg_xx ) */  \
        ::rw::math::vpu::VectorIntrinsic neg_yy_zz_xx; \
        RW_MATH_VPU_V3_RotateLeft(neg_yy_zz_xx, neg_qq); \
        ::rw::math::vpu::VectorIntrinsic diagonalValues = RW_MATH_VPU_Add_R(neg_qq, neg_yy_zz_xx); \
        RW_MATH_VPU_CreateVectorAxisA3_I( ret_x, /*(neg_yy+neg_zz)*/diagonalValues, VectorAxisY, /*xy+wz*/             addVec,         VectorAxisX, /*(zx-wy)*/         subVec,            VectorAxisZ ); \
        RW_MATH_VPU_CreateVectorAxisA3_I( ret_y, /*(xy-wz)*/        subVec,         VectorAxisX, /*(neg_zz+neg_xx)*/   diagonalValues, VectorAxisZ, /*(yz+wx)*/         addVec,            VectorAxisY ); \
        RW_MATH_VPU_CreateVectorAxisA3_I( ret_z, /*(zx+wy)*/        addVec,         VectorAxisZ, /*(yz-wx)*/           subVec,         VectorAxisY, /*(neg_xx+neg_yy)*/ diagonalValues,    VectorAxisX ); \
    }


#define RW_MATH_VPU_Min(ret, in_vec0, in_vec1) \
    ret = vminq_f32(in_vec0, in_vec1)


#define RW_MATH_VPU_Max(ret, in_vec0, in_vec1) \
    ret = vmaxq_f32(in_vec0, in_vec1)

#define RW_MATH_VPU_ClampToNegPosRange(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic negRange; \
        RW_MATH_VPU_Negate(negRange, in_vec1); \
        RW_MATH_VPU_Max(ret, negRange, in_vec0); \
        RW_MATH_VPU_Min(ret, in_vec1, ret); \
    }

RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic RWMATH_VPU_DETAIL_VecMergeH( VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    float32x2x2_t zipped = vzip_f32(vget_low_f32(a), vget_low_f32(b));
    return vcombine_f32(zipped.val[0], zipped.val[1]);
}

RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic RWMATH_VPU_DETAIL_VecMergeL( VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    float32x2x2_t zipped = vzip_f32(vget_high_f32(a), vget_high_f32(b));
    return vcombine_f32(zipped.val[0], zipped.val[1]);
}

#define RW_MATH_VPU_CreateVectorFromScalar_R(f0) \
    (vdupq_n_f32(f0))


#define RW_MATH_VPU_GetZero_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(0.0f)

#define RW_MATH_VPU_GetOne_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(1.0f)


#define RW_MATH_VPU_GetHalf_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(0.5f)


#define RW_MATH_VPU_GetNegHalf_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(-0.5f)


#define RW_MATH_VPU_GetTwo_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(2.0f)


#define RW_MATH_VPU_GetNegTwo_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(-2.0f)


#define RW_MATH_VPU_GetNegOne_R() \
    RW_MATH_VPU_CreateVectorFromScalar_R(-1.0f)


#define RW_MATH_VPU_GetTrue_R() \
    vreinterpretq_f32_u32(vdupq_n_u32(0xffffffff))


#define RW_MATH_VPU_GetI_R() (float32x4_t){1.0, 0.0, 0.0, 0.0}
#define RW_MATH_VPU_GetJ_R() (float32x4_t){0.0, 1.0, 0.0, 0.0}
#define RW_MATH_VPU_GetK_R() (float32x4_t){0.0, 0.0, 1.0, 0.0}
#define RW_MATH_VPU_GetL_R() (float32x4_t){0.0, 0.0, 0.0, 1.0}

#define RW_MATH_VPU_Add_R(a, b) \
    vaddq_f32(a, b)


#define RW_MATH_VPU_Subtract_R(a, b) \
    vsubq_f32(a, b)

inline void RW_MATH_VPU_NormalizeFastV3(::rw::math::vpu::VectorIntrinsic& ret, VectorIntrinsicInParam v) { RW_MATH_VPU_NormalizeV3(ret, v) }

#define RW_MATH_VPU_Select_R(a, b, mask) \
    vbslq_f32(vreinterpretq_u32_f32(mask), (b), (a))


#define RW_MATH_VPU_IsEqual_R(a, b) \
        vreinterpretq_f32_u32(vceqq_f32((a), (b)))

#define RW_MATH_VPU_IsGreater_R(a, b) \
        vreinterpretq_f32_u32(vcgtq_f32((a), (b)))

#define RW_MATH_VPU_IsGreaterEqual_R(a, b) \
        vreinterpretq_f32_u32(vcgeq_f32((a), (b)))

// Trig functions
#define RW_MATH_VPU_Cos(ret, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret, in_vec, cosf );

#define RW_MATH_VPU_Sin(ret, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret, in_vec, sinf );

#define RW_MATH_VPU_SinCos(ret0, ret1, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret0, in_vec, sinf ); \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret1, in_vec, cosf );

#define RW_MATH_VPU_Tan(ret, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret, in_vec, tanf );

#define RW_MATH_VPU_ACos(ret, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret, in_vec, acosf );

#define RW_MATH_VPU_ASin(ret, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret, in_vec, asinf );

#define RW_MATH_VPU_ATan(ret, in_vec) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY( ret, in_vec, atanf );

#define RW_MATH_VPU_ATan2( ret, in_y, in_x ) \
    RW_MATH_DETAIL_VECFLOAT_APPLY_BINARY( ret, in_y, in_x, atan2f );


#define RW_MATH_VPU_IsAnyTrue(ret, in_vec) \
    { \
        uint32x4_t intVec = vreinterpretq_u32_f32(in_vec); \
        uint32x2_t mask = vorr_u32( vget_low_u32(intVec), vget_high_u32(intVec) ); \
        RW_MATH_DETAIL_AnyTrueInt2(ret, mask); \
    }


#define RW_MATH_VPU_IsAllTrue(ret, in_vec) \
    { \
        uint32x4_t intVec = vreinterpretq_u32_f32(in_vec); \
        uint32x2_t mask = vand_u32( vget_low_u32(intVec), vget_high_u32(intVec) ); \
        RW_MATH_DETAIL_AllTrueInt2(ret, mask); \
    }


#define RW_MATH_VPU_IsAnyFalse(ret, in_vec) \
    { \
        uint32x4_t intVec = vreinterpretq_u32_f32(in_vec); \
        uint32x2_t mask = vand_u32( vget_low_u32(intVec), vget_high_u32(intVec) ); \
        RW_MATH_DETAIL_AnyFalseInt2(ret, mask); \
    }


#define RW_MATH_VPU_IsAllFalse(ret, in_vec) \
    { \
        uint32x4_t intVec = vreinterpretq_u32_f32(in_vec); \
        uint32x2_t mask = vorr_u32( vget_low_u32(intVec), vget_high_u32(intVec) ); \
        RW_MATH_DETAIL_AllFalseInt2(ret, mask); \
    }


#define RW_MATH_VPU_PermuteSingle_I(ret, in_vec, pattern) \
{ \
    uint64_t uintPattern = static_cast<uint64_t>(pattern); \
    uint8x8_t lowPattern = vcreate_u8( \
        ((((uintPattern&0xf000)>>12) * 4 + 0) <<  0) | \
        ((((uintPattern&0xf000)>>12) * 4 + 1) <<  8) | \
        ((((uintPattern&0xf000)>>12) * 4 + 2) << 16) | \
        ((((uintPattern&0xf000)>>12) * 4 + 3) << 24) | \
        ((((uintPattern&0x0f00)>> 8) * 4 + 0) << 32) | \
        ((((uintPattern&0x0f00)>> 8) * 4 + 1) << 40) | \
        ((((uintPattern&0x0f00)>> 8) * 4 + 2) << 48) | \
        ((((uintPattern&0x0f00)>> 8) * 4 + 3) << 56) ); \
    uint8x8_t highPattern = vcreate_u8( \
        ((((uintPattern&0x00f0)>> 4) * 4 + 0) <<  0) | \
        ((((uintPattern&0x00f0)>> 4) * 4 + 1) <<  8) | \
        ((((uintPattern&0x00f0)>> 4) * 4 + 2) << 16) | \
        ((((uintPattern&0x00f0)>> 4) * 4 + 3) << 24) | \
        ((((uintPattern&0x000f)    ) * 4 + 0) << 32) | \
        ((((uintPattern&0x000f)    ) * 4 + 1) << 40) | \
        ((((uintPattern&0x000f)    ) * 4 + 2) << 48) | \
        ((((uintPattern&0x000f)    ) * 4 + 3) << 56) ); \
    uint8x16_t inUint = vreinterpretq_u8_f32(in_vec); \
    uint8x8x2_t tableIn = { { vget_low_u8(inUint), vget_high_u8(inUint) } }; \
    uint8x8_t lowOut = vtbl2_u8(tableIn, lowPattern); \
    uint8x8_t highOut = vtbl2_u8(tableIn, highPattern); \
    ret = vreinterpretq_f32_u8(vcombine_u8(lowOut, highOut)); \
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
