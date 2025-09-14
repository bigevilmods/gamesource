// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_MACRO_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_MACRO_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_axis.h"

namespace rw
{
namespace math
{
namespace vpu
{

// Shuffles a single vector according to an SSE shuffle pattern
#if EA_SSE == 2
    #define RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, pattern ) \
        _mm_castsi128_ps(_mm_shuffle_epi32( _mm_castps_si128(vec), pattern ))
#else
    #define RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, pattern ) \
        _mm_shuffle_ps( vec, vec, pattern )
#endif

#define RW_MATH_VPU_V2_RotateLeft( ret, vec) \
	{ \
		ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, RWMATH_VPU_DETAIL_sseShuffle( 1,0,2,3 ) ); \
    }

#define RW_MATH_VPU_V3_RotateLeft( ret, vec) \
	{ \
		ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, RWMATH_VPU_DETAIL_sseShuffle( 1,2,0,3 ) ); \
    }

#define RW_MATH_VPU_V3_RotateRight( ret, vec) \
	{ \
		ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, RWMATH_VPU_DETAIL_sseShuffle( 2,0,1,3 ) ); \
    }

#define RW_MATH_VPU_V4_RotateLeft( ret, vec) \
	{ \
		ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, RWMATH_VPU_DETAIL_sseShuffle( 1,2,3,0 ) ); \
    }

#define RW_MATH_VPU_V4_RotateRight( ret, vec) \
	{ \
		ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, RWMATH_VPU_DETAIL_sseShuffle( 3,0,1,2 ) ); \
    }

#define RW_MATH_VPU_LoadFromAlignedFloatPointer( address ) \
    _mm_load_ps( address )

#define RW_MATH_VPU_UnalignedLoad(ret, address ) \
    ret = _mm_loadu_ps( address )

#define RW_MATH_VPU_UnalignedStore2(dest, vector ) \
    { \
        _mm_store_ss(&dest.x, vector); \
        _mm_store_ss(&dest.y, RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R(vector, 1)); \
    }

#define RW_MATH_VPU_UnalignedStore3(dest, vector ) \
    { \
        _mm_store_ss(&dest.x, vector); \
        _mm_store_ss(&dest.y, RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R(vector, 1)); \
        _mm_store_ss(&dest.z, RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R(vector, 2)); \
    }

#define RW_MATH_VPU_UnalignedStore4(dest, vector ) \
        _mm_storeu_ps(&dest.x, vector)

#define RWMATH_VPU_DETAIL_sseShuffle( x_index, y_index, z_index, w_index ) \
    ( ((w_index)<<6) | ((z_index)<<4) | ((y_index)<<2) | (x_index) )

#define RWMATH_VPU_DETAIL_sseShuffleSplat( index ) \
    RWMATH_VPU_DETAIL_sseShuffle(index, index, index, index)

#define RWMATH_VPU_DETAIL_sseCompMask( x_flag, y_flag, z_flag, w_flag ) \
    ( ((w_flag)<<3) | ((z_flag)<<2) | ((y_flag)<<1) | (x_flag) )

//
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

#define RW_MATH_VPU_Select(ret, a, b, c) \
    ret = _mm_or_ps(_mm_andnot_ps(c, a), _mm_and_ps(b, c)); \

#define RW_MATH_VPU_Select_R(a, b, c) \
    _mm_or_ps(_mm_andnot_ps(c, a), _mm_and_ps(b, c))

#define RW_MATH_VPU_IsGreater(ret, a, b) \
    ret = _mm_cmpgt_ps(a, b);

#define RW_MATH_VPU_IsGreater_R(a, b) \
    _mm_cmpgt_ps(a, b)

#define RW_MATH_VPU_IsGreaterEqual(ret, a, b) \
    ret = _mm_cmpge_ps(a, b);

#define RW_MATH_VPU_IsGreaterEqual_R(a, b) \
    _mm_cmpge_ps(a, b)

#define RW_MATH_VPU_IsEqual(ret, a, b) \
    ret = _mm_cmpeq_ps(a, b);

#define RW_MATH_VPU_IsEqual_R(a, b) \
    _mm_cmpeq_ps(a, b)

// The scalar comparison macros assume that all elements of the vector are identical.  The return result is either boolean or
// an integer that indicates true/false.
#define RW_MATH_VPU_Scalar_IsEqual(result, a, b) \
    { \
        float aFloat, bFloat; \
        _mm_store_ss( &aFloat, a ); \
        _mm_store_ss( &bFloat, b ); \
        result = (aFloat == bFloat); \
    }

#define RW_MATH_VPU_Scalar_IsGreater(result, a, b) \
    { \
        float aFloat, bFloat; \
        _mm_store_ss( &aFloat, a ); \
        _mm_store_ss( &bFloat, b ); \
        result = (aFloat > bFloat); \
    }

#define RW_MATH_VPU_Scalar_IsGreaterEqual(result, a, b) \
    { \
        float aFloat, bFloat; \
        _mm_store_ss( &aFloat, a ); \
        _mm_store_ss( &bFloat, b ); \
        result = (aFloat >= bFloat); \
    }


#define RW_MATH_VPU_Scalar_IsLess(result, a, b) \
    { \
        float aFloat, bFloat; \
        _mm_store_ss( &aFloat, a ); \
        _mm_store_ss( &bFloat, b ); \
        result = (aFloat < bFloat); \
    }


#define RW_MATH_VPU_Scalar_IsLessEqual(result, a, b) \
    { \
        float aFloat, bFloat; \
        _mm_store_ss( &aFloat, a ); \
        _mm_store_ss( &bFloat, b ); \
        result = (aFloat <= bFloat); \
    }


// create a vector intrinsic from a pointer to a float.
// The float data will be splatted across the entire vector.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    { \
        ret = _mm_set1_ps(*(fPtr)); \
    }

#define RW_MATH_VPU_CreateVectorMaskFromBool( ret, boolValue ) \
    { \
        rw::math::detail::Flt32UInt32Union masks[2]; \
        masks[0].i = 0; \
        masks[1].i = 0xFFFFFFFF; \
        ret = _mm_set1_ps( masks[ boolValue ? 1 : 0 ].f ); \
    }

//----------------------------------------------------------------------------
// _asmCreateVector()
//----------------------------------------------------------------------------
// create a vector intrinsic data from 1 float
// ret - VectorIntrinsic
// f0 - float
#define RW_MATH_VPU_CreateVectorV1(ret, f0) \
    { \
        ret = _mm_set1_ps(f0); \
    } \

#define RW_MATH_VPU_CreateVectorFromScalar_R( f0 ) \
        _mm_set1_ps(f0)

// create a vector intrinsic data from two floats
#define RW_MATH_VPU_CreateVectorV2(ret, f0, f1) \
    { \
        ret = _mm_set_ps(f0, f0, f1, f0); \
    } \

// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV3(ret, f0, f1, f2) \
    { \
        ret = _mm_set_ps(f0, f2, f1, f0); \
    } \

// create a vector intrinsic data from 4 floats
#define RW_MATH_VPU_CreateVectorV4(ret, f0, f1, f2, f3) \
    { \
        ret = _mm_set_ps(f3, f2, f1, f0); \
    } \

#define RW_MATH_VPU_SetWToZero(ret, v ) \
    RW_MATH_VPU_And( ret, v, rw::math::vpu::detail::gXYZMask );

//----------------------------------------------------------------------------
// _asmSwizzleStore()
//----------------------------------------------------------------------------
// swizzle an element of a vector to another element of a vector
//
// ret - VectorIntrinsic
// in_vec0 - VectorIntrinsic
// in_index0_immediate - integer value (immediate value only)
// in_vec1 - VectorIntrinsic
// in_index1_immediate - integer value (immediate value only)
#define RW_MATH_VPU_SwizzleStore_V_V(ret, vec_dest, vec_dest_index, vec_src, vec_src_index) \
    { \
        detail::VectorIntrinsicUnion _asmSwizzleStore_V_V_result; \
        detail::VectorIntrinsicUnion _asmSwizzleStore_V_V_source; \
        _asmSwizzleStore_V_V_result.vector = vec_dest; \
        _asmSwizzleStore_V_V_source.vector = vec_src; \
        _asmSwizzleStore_V_V_result.scalarUInt[vec_dest_index] = _asmSwizzleStore_V_V_source.scalarUInt[vec_src_index]; \
        ret = _asmSwizzleStore_V_V_result.vector; \
    } \

template <enum rw::math::vpu::VectorAxis>
RW_MATH_FORCE_INLINE VectorIntrinsic SwizzleStoreShuffleHelper(VectorIntrinsic mixed_vector, VectorIntrinsic vec_dest);

template <> RW_MATH_FORCE_INLINE VectorIntrinsic SwizzleStoreShuffleHelper<VectorAxisX>(VectorIntrinsic mixed_vector, VectorIntrinsic vec_dest)
{
   return _mm_shuffle_ps( mixed_vector, vec_dest, RWMATH_VPU_DETAIL_sseShuffle( 2,0,2,3) );
}

template <> RW_MATH_FORCE_INLINE VectorIntrinsic SwizzleStoreShuffleHelper<VectorAxisY>(VectorIntrinsic mixed_vector, VectorIntrinsic vec_dest)
{
    return _mm_shuffle_ps( mixed_vector, vec_dest, RWMATH_VPU_DETAIL_sseShuffle( 0,2,2,3) );
}

template <> RW_MATH_FORCE_INLINE VectorIntrinsic SwizzleStoreShuffleHelper<VectorAxisZ>(VectorIntrinsic mixed_vector, VectorIntrinsic vec_dest)
{
   return _mm_shuffle_ps( vec_dest, mixed_vector, RWMATH_VPU_DETAIL_sseShuffle( 0,1,2,0) );
}

template <> RW_MATH_FORCE_INLINE VectorIntrinsic SwizzleStoreShuffleHelper<VectorAxisW>(VectorIntrinsic mixed_vector, VectorIntrinsic vec_dest)
{
   return _mm_shuffle_ps( vec_dest, mixed_vector, RWMATH_VPU_DETAIL_sseShuffle( 0,1,0,2) );
}

// 'mixed_vector' stores the src element that is to be inserted in the destination AND the element that is in
// the same half of the destination register as where the element is being inserted.
#define RW_MATH_VPU_SwizzleStore_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    { \
        VectorIntrinsic mixed_vector = _mm_shuffle_ps( vec_dest, vec_src, RWMATH_VPU_DETAIL_sseShuffle( vec_dest_index_immediate ^ 1, vec_dest_index_immediate ^ 1, \
            vec_src_index_immediate, vec_src_index_immediate) ); \
        ret = SwizzleStoreShuffleHelper<vec_dest_index_immediate>(mixed_vector, vec_dest); \
    }


#define RW_MATH_VPU_SwizzleStore_I_V(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index) \
    { \
        VectorIntrinsic src_splat; \
        RW_MATH_VPU_Splat(src_splat, vec_src, vec_src_index); \
        VectorIntrinsic temp_result; \
        RW_MATH_VPU_SwizzleStore_I_I(temp_result, vec_dest, vec_dest_index_immediate, src_splat, VectorAxisX); \
        ret= temp_result; \
    } \


#define RW_MATH_VPU_SwizzleStore_V_I(ret, vec_dest, vec_dest_index, vec_src, vec_src_index_immediate) \
    { \
        VectorIntrinsic temp_vec= vec_dest; \
        reinterpret_cast<float*>(&temp_vec)[vec_dest_index] = reinterpret_cast<const float*>(&vec_src)[vec_src_index_immediate]; \
        ret = temp_vec; \
    } \


#define RW_MATH_VPU_SwizzleYZWX(ret, vec_in) \
    ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec_in, RWMATH_VPU_DETAIL_sseShuffle( 1,2,3,0) ); \


//----------------------------------------------------------------------------
// _asmCreateVectorAxis()
//----------------------------------------------------------------------------
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
    { \
        VectorIntrinsic partial_result = _mm_shuffle_ps( in_vec0, in_vec1, RWMATH_VPU_DETAIL_sseShuffle( in_index0_immediate, 0, in_index1_immediate, 0 ) ); \
        ret= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( partial_result, RWMATH_VPU_DETAIL_sseShuffle( 0, 2, 0, 0 ) ); \
    } \

#define RW_MATH_VPU_CreateVectorAxisA3_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate) \
    { \
        VectorIntrinsic partial_result = _mm_shuffle_ps( in_vec0, in_vec1, RWMATH_VPU_DETAIL_sseShuffle( in_index0_immediate, 0, in_index1_immediate, 0 ) ); \
        ret= _mm_shuffle_ps( partial_result, in_vec2, RWMATH_VPU_DETAIL_sseShuffle( 0, 2, in_index2_immediate, 0 ) ); \
    } \


#define RW_MATH_VPU_CreateVectorAxisA4_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate, in_vec3, in_index3_immediate) \
    { \
        VectorIntrinsic partial_result = _mm_shuffle_ps( in_vec0, in_vec1, RWMATH_VPU_DETAIL_sseShuffle( in_index0_immediate, 0, in_index1_immediate, 0 ) ); \
        VectorIntrinsic partialResult23= _mm_shuffle_ps( in_vec2, in_vec3, RWMATH_VPU_DETAIL_sseShuffle( in_index2_immediate, 0, in_index3_immediate, 0 ) ); \
        ret= _mm_shuffle_ps( partial_result, partialResult23, RWMATH_VPU_DETAIL_sseShuffle( 0, 2, 0, 2 ) ); \
    } \


//----------------------------------------------------------------------------
// _asmGetZero()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetZero(ret) \
    { \
        ret = _mm_setzero_ps(); \
    } \

#define RW_MATH_VPU_GetZero_R() \
    _mm_setzero_ps()

//----------------------------------------------------------------------------
// _asmGetHalf()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetHalf(ret) \
    { \
        ret = _mm_set1_ps(0.5f); \
    } \

#define RW_MATH_VPU_GetHalf_R() \
    _mm_set1_ps(0.5f)

#define RW_MATH_VPU_GetNegHalf_R() \
    _mm_set1_ps(-0.5f)

//----------------------------------------------------------------------------
// _asmGetOne()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetOne(ret) \
    { \
        ret = _mm_set1_ps(1.0f); \
    } \

#define RW_MATH_VPU_GetOne_R() \
    _mm_set1_ps(1.0f)

#define RW_MATH_VPU_GetNegOne(ret) \
    { \
        ret = _mm_set1_ps(-1.0f); \
    } \

#define RW_MATH_VPU_GetNegOne_R() \
    _mm_set1_ps(-1.0f)

//----------------------------------------------------------------------------
// _asmGetTwo()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetTwo(ret) \
    { \
        ret = _mm_set1_ps(2.0f); \
    } \

#define RW_MATH_VPU_GetTwo_R() \
    _mm_set1_ps(2.0f)

#define RW_MATH_VPU_GetNegTwo_R() \
    _mm_set1_ps(-2.0f)

#define RW_MATH_VPU_GetTrue_R() \
    _mm_cmpeq_ps( _mm_setzero_ps(), _mm_setzero_ps() )

#define RW_MATH_VPU_GetTrue(ret) \
    { \
        VectorIntrinsic gettrue_zero = _mm_setzero_ps(); \
        ret = _mm_cmpeq_ps( gettrue_zero, gettrue_zero ); \
    }

//----------------------------------------------------------------------------
// _asmGetPI()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetPI(ret) \
    { \
        ret = _mm_set1_ps(3.14159265358979323846f); \
    } \

#define RW_MATH_VPU_GetPI_R() \
    _mm_set1_ps(3.14159265358979323846f)

#define RW_MATH_VPU_Get2PI(ret) \
    ret = _mm_set1_ps(6.28318530717958f);

#define RW_MATH_VPU_Get2PIReciprocal(ret) \
    ret = _mm_set1_ps(0.1591549430918953f);

//----------------------------------------------------------------------------
// _asmGetI()
//----------------------------------------------------------------------------
// ret= 1,0,0,0
#define RW_MATH_VPU_GetI(ret) \
    { \
        ret = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f); \
    } \

#define RW_MATH_VPU_GetI_R() \
    _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f)

//----------------------------------------------------------------------------
// _asmGetJ()
//----------------------------------------------------------------------------
// ret= 0,1,0,0
#define RW_MATH_VPU_GetJ(ret) \
    { \
        ret = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f); \
    } \

#define RW_MATH_VPU_GetJ_R() \
    _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f)

//----------------------------------------------------------------------------
// _asmGetK()
//----------------------------------------------------------------------------
// ret= 0,0,1,0
#define RW_MATH_VPU_GetK(ret) \
    { \
        ret = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f); \
    } \

#define RW_MATH_VPU_GetK_R() \
    _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f)

//----------------------------------------------------------------------------
// _asmGetL()
//----------------------------------------------------------------------------
// ret= 0,0,0,1
#define RW_MATH_VPU_GetL(ret) \
    { \
        ret = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f); \
    } \

#define RW_MATH_VPU_GetL_R() \
    _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)

//----------------------------------------------------------------------------
// _asmGetSignMaskXYZW()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetSignMaskXYZW(ret) \
    { \
        rw::math::detail::Flt32UInt32Union _asmGetSignMaskXYZW_temp; \
        _asmGetSignMaskXYZW_temp.i = 0x80000000; \
        ret = _mm_set1_ps(_asmGetSignMaskXYZW_temp.f); \
    } \


//----------------------------------------------------------------------------
// _asmGetSignMaskXYZ()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetSignMaskXYZ(ret) \
    { \
        rw::math::detail::Flt32UInt32Union _asmGetSignMaskXYZ_temp; \
        _asmGetSignMaskXYZ_temp.i = 0x80000000; \
        ret = _mm_set_ps(0.0f, _asmGetSignMaskXYZ_temp.f, _asmGetSignMaskXYZ_temp.f, _asmGetSignMaskXYZ_temp.f); \
    } \


//----------------------------------------------------------------------------
// _asmGetSignMaskXZ()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetSignMaskXZ(ret) \
    { \
        rw::math::detail::Flt32UInt32Union _asmGetSignMaskXY_temp; \
        _asmGetSignMaskXY_temp.i = 0x80000000; \
        ret = _mm_set_ps(0.0f, _asmGetSignMaskXY_temp.f, 0.0f, _asmGetSignMaskXY_temp.f); \
    } \


//----------------------------------------------------------------------------
// _asmGetSignMaskYW()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetSignMaskYW(ret) \
    { \
        rw::math::detail::Flt32UInt32Union _asmGetSignMaskYW_temp; \
        _asmGetSignMaskYW_temp.i = 0x80000000; \
        ret = _mm_set_ps(_asmGetSignMaskYW_temp.f, 0.0f, _asmGetSignMaskYW_temp.f, 0.0f); \
    } \


//----------------------------------------------------------------------------
// _asmGetFloatAxis()
//----------------------------------------------------------------------------
//
// ret - float
// in_vec - VectorIntrinsic
// in_index_immediate - integer value (immediate value)
#define RW_MATH_VPU_GetFloatAxis_I(ret, in_vec, in_index_immediate) \
    { \
        detail::VectorIntrinsicUnion temp; \
        temp.vector = in_vec; \
        ret= temp.scalarFloat[in_index_immediate]; \
    } \

#define RW_MATH_VPU_Floor(ret, a1) \
    { \
        VectorIntrinsic zero = RW_MATH_VPU_GetZero_R(); \
        VectorIntrinsic one = RW_MATH_VPU_GetOne_R(); \
        VectorIntrinsic maxMantissa = _mm_set1_ps(8388608.0); \
        VectorIntrinsic negMaxMantissa = _mm_set1_ps(-8388608.0); \
        VectorIntrinsic isNegativeMask = RW_MATH_VPU_IsGreater_R( zero, a1); \
        VectorIntrinsic truncatingValue = RW_MATH_VPU_Select_R( negMaxMantissa, maxMantissa, isNegativeMask ); \
        VectorIntrinsic truncatedValue = RW_MATH_VPU_Add_R( RW_MATH_VPU_Subtract_R(a1, truncatingValue), truncatingValue ); \
        VectorIntrinsic subtractOneMask = RW_MATH_VPU_IsGreater_R( truncatedValue, a1); \
        VectorIntrinsic subtractValue = RW_MATH_VPU_And_R( subtractOneMask, one ); \
        ret = RW_MATH_VPU_Subtract_R( truncatedValue, subtractValue ); \
    }

#define RW_MATH_VPU_Ceil(ret, a1) \
    { \
        VectorIntrinsic negA1, floorNegA1; \
        RW_MATH_VPU_Negate(negA1, a1); \
        RW_MATH_VPU_Floor(floorNegA1, negA1); \
        RW_MATH_VPU_Negate(ret, floorNegA1); \
    }

#define RW_MATH_VPU_RoundToZero(ret, a1) \
    { \
        VectorIntrinsic zero = RW_MATH_VPU_GetZero_R(); \
        VectorIntrinsic one = RW_MATH_VPU_GetOne_R(); \
        VectorIntrinsic negOne = RW_MATH_VPU_GetNegOne_R(); \
        VectorIntrinsic maxMantissa = _mm_set1_ps(8388608.0); \
        VectorIntrinsic negMaxMantissa = _mm_set1_ps(-8388608.0); \
        VectorIntrinsic isNegativeMask = RW_MATH_VPU_IsGreater_R( zero, a1); \
        VectorIntrinsic truncatingValue = RW_MATH_VPU_Select_R( negMaxMantissa, maxMantissa, isNegativeMask ); \
        VectorIntrinsic truncatedValue = RW_MATH_VPU_Add_R( RW_MATH_VPU_Subtract_R(a1, truncatingValue), truncatingValue ); \
        VectorIntrinsic subtractOneMaskIfPosInput = RW_MATH_VPU_IsGreater_R( truncatedValue, a1); \
        VectorIntrinsic subtractValueIfPosInput = RW_MATH_VPU_And_R( subtractOneMaskIfPosInput, negOne ); \
        VectorIntrinsic subtractOneMaskIfNegInput = RW_MATH_VPU_IsGreater_R( a1, truncatedValue); \
        VectorIntrinsic subtractValueIfNegInput = RW_MATH_VPU_And_R( subtractOneMaskIfNegInput, one ); \
        VectorIntrinsic modifier; \
        RW_MATH_VPU_Select( modifier, subtractValueIfPosInput, subtractValueIfNegInput, isNegativeMask ); \
        ret = RW_MATH_VPU_Add_R( truncatedValue, modifier ); \
    }

#define RW_MATH_VPU_Mult_R( in_vec0, in_vec1 ) \
    _mm_mul_ps( in_vec0, in_vec1 )

#define RW_MATH_VPU_Mult(ret, in_vec0, in_vec1) \
    ret= _mm_mul_ps( in_vec0, in_vec1 ); \


//----------------------------------------------------------------------------
// _asmMultFloat()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_MultFloat(ret, in_vec, in_float) \
    { \
        VectorIntrinsic floatVec= _mm_set1_ps(in_float); \
        ret= _mm_mul_ps( in_vec, floatVec ); \
    } \


//----------------------------------------------------------------------------
// _asmNegate()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_Negate(ret, in_vec) \
    { \
        VectorIntrinsic zero_vec = _mm_setzero_ps(); \
        ret= _mm_sub_ps(zero_vec , in_vec ); \
    } \
    

#define RW_MATH_VPU_NegateXZ(ret, in_vec) \
    { \
        VectorIntrinsic _asmNegateXZ_mask; \
        RW_MATH_VPU_GetSignMaskXZ(_asmNegateXZ_mask); \
        RW_MATH_VPU_Xor(ret, in_vec, _asmNegateXZ_mask); \
    } \


#define RW_MATH_VPU_NegateYW(ret, in_vec) \
    { \
        VectorIntrinsic _asmNegateYW_mask; \
        RW_MATH_VPU_GetSignMaskYW(_asmNegateYW_mask); \
        RW_MATH_VPU_Xor(ret, in_vec, _asmNegateYW_mask); \
    } \


//----------------------------------------------------------------------------
// _asmProductDifference()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_ProductDifference(ret, v0, v1, v2, v3) \
    ret = _mm_sub_ps(_mm_mul_ps(v0, v1), _mm_mul_ps(v2, v3)); \


//----------------------------------------------------------------------------
// _asmReciprocalEstimate()
//----------------------------------------------------------------------------
#if defined(RW_MATH_NO_NONDETERMINISTIC_SSE_INTRINSICS)
#define RW_MATH_VPU_ReciprocalEstimate(ret, in_vec) \
    ret = _mm_div_ps( _mm_set1_ps(1.0f), in_vec ); 
#else 
#define RW_MATH_VPU_ReciprocalEstimate(ret, in_vec) \
    ret = _mm_rcp_ps( in_vec ); 
#endif 



//----------------------------------------------------------------------------
// _asmReciprocalFast()
//----------------------------------------------------------------------------
// Do one iteration of Newton-Raphson estimation
#if defined(RW_MATH_NO_NONDETERMINISTIC_SSE_INTRINSICS)
#define RW_MATH_VPU_ReciprocalFast(ret, in_vec) \
    { \
        VectorIntrinsic rf_two = _mm_set1_ps(2.0f); \
        VectorIntrinsic estimate = _mm_div_ps(_mm_set1_ps(1.0f), in_vec); \
        VectorIntrinsic two_minus_v_times_estimate = _mm_sub_ps( rf_two, _mm_mul_ps(estimate, in_vec) ); \
        ret = _mm_mul_ps( two_minus_v_times_estimate , estimate ); \
    }
#else
#define RW_MATH_VPU_ReciprocalFast(ret, in_vec) \
    { \
        VectorIntrinsic rf_two = _mm_set1_ps(2.0f); \
        VectorIntrinsic estimate = _mm_rcp_ps(in_vec); \
        VectorIntrinsic two_minus_v_times_estimate = _mm_sub_ps( rf_two, _mm_mul_ps(estimate, in_vec) ); \
        ret = _mm_mul_ps( two_minus_v_times_estimate , estimate ); \
    } 
#endif


//----------------------------------------------------------------------------
// _asmReciprocal()
//----------------------------------------------------------------------------
// Do two iterations of Newton-Raphson estimation
#define RW_MATH_VPU_Reciprocal(ret, in_vec) \
    { \
        VectorIntrinsic recip_two = _mm_set1_ps(2.0f); \
        VectorIntrinsic recip_fast;  \
        RW_MATH_VPU_ReciprocalFast(recip_fast, in_vec); \
        VectorIntrinsic two_minus_v_times_estimate = _mm_sub_ps( recip_two, _mm_mul_ps(recip_fast, in_vec) ); \
        ret = _mm_mul_ps( two_minus_v_times_estimate , recip_fast ); \
    } \


//----------------------------------------------------------------------------
// _asmReciprocalSquareRootEstimate()
//----------------------------------------------------------------------------
#if defined(RW_MATH_NO_NONDETERMINISTIC_SSE_INTRINSICS)
#define RW_MATH_VPU_ReciprocalSquareRootEstimate(ret, in_vec) \
    ret= _mm_div_ps( _mm_set1_ps(1.0f), _mm_sqrt_ps(in_vec) );
#else
#define RW_MATH_VPU_ReciprocalSquareRootEstimate(ret, in_vec) \
    ret= _mm_rsqrt_ps( in_vec );
#endif


#define RW_MATH_VPU_RotateLeft_V_I(ret, in_vec, rotateAmount) \
    ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec, RWMATH_VPU_DETAIL_sseShuffle( (0+rotateAmount)&3, (1+rotateAmount)&3, (2+rotateAmount)&3, (3+rotateAmount)&3 ) )

//----------------------------------------------------------------------------
// _asmReciprocalSquareRootFast()
//----------------------------------------------------------------------------
// Do one iteration of Newton-Raphson estimation
#if defined(RW_MATH_NO_NONDETERMINISTIC_SSE_INTRINSICS)
#define RW_MATH_VPU_ReciprocalSquareRootFast(ret, in_vec) \
    { \
        VectorIntrinsic rsqf_one = _mm_set1_ps(1.0f); \
        VectorIntrinsic rsqf_half = _mm_set1_ps(0.5f); \
        VectorIntrinsic estimate = _mm_div_ps( rsqf_one, _mm_sqrt_ps(in_vec) ); \
        VectorIntrinsic one_minus_v_times_squared_estimate = _mm_sub_ps(rsqf_one, _mm_mul_ps( in_vec, _mm_mul_ps(estimate, estimate) )); \
        ret = _mm_add_ps( estimate, _mm_mul_ps( one_minus_v_times_squared_estimate , _mm_mul_ps(estimate, rsqf_half) )); \
    }
#else
#define RW_MATH_VPU_ReciprocalSquareRootFast(ret, in_vec) \
    { \
        VectorIntrinsic rsqf_one = _mm_set1_ps(1.0f); \
        VectorIntrinsic rsqf_half = _mm_set1_ps(0.5f); \
        VectorIntrinsic estimate = _mm_rsqrt_ps(in_vec); \
        VectorIntrinsic one_minus_v_times_squared_estimate = _mm_sub_ps(rsqf_one, _mm_mul_ps( in_vec, _mm_mul_ps(estimate, estimate) )); \
        ret = _mm_add_ps( estimate, _mm_mul_ps( one_minus_v_times_squared_estimate , _mm_mul_ps(estimate, rsqf_half) )); \
    } 
#endif

//----------------------------------------------------------------------------
// _asmReciprocalSquareRoot()
//----------------------------------------------------------------------------
// Do two iterations of Newton-Raphson estimation
#define RW_MATH_VPU_ReciprocalSquareRoot(ret, in_vec) \
    { \
        VectorIntrinsic rsq_one = _mm_set1_ps(1.0f); \
        VectorIntrinsic rsq_half = _mm_set1_ps(0.5f); \
        VectorIntrinsic recip_sqrt_fast; \
        RW_MATH_VPU_ReciprocalSquareRootFast(recip_sqrt_fast, in_vec); \
        VectorIntrinsic one_minus_v_times_squared_estimate = _mm_sub_ps(rsq_one, _mm_mul_ps( in_vec, _mm_mul_ps(recip_sqrt_fast, recip_sqrt_fast) )); \
        ret= _mm_add_ps( recip_sqrt_fast, _mm_mul_ps( one_minus_v_times_squared_estimate , _mm_mul_ps(recip_sqrt_fast, rsq_half) )); \
    } \


//----------------------------------------------------------------------------
// _asmDivide()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_Divide(ret, in_vec0, in_vec1) \
    ret= _mm_div_ps( in_vec0, in_vec1 ); \


#define RW_MATH_VPU_DivideFloat(ret, in_vec, in_float) \
    { \
        ret= _mm_mul_ps( in_vec , _mm_set1_ps(1.0f/in_float) ); \
    } \


//----------------------------------------------------------------------------
// _asmDivideAxis()
//----------------------------------------------------------------------------
// ret = in_vec0 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_DivideAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    { \
        VectorIntrinsic divisor = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec_scale, RWMATH_VPU_DETAIL_sseShuffleSplat(in_index_immediate) ); \
        ret= _mm_div_ps( in_vec0, divisor ); \
    } \


// ret0 = in_vec0 / in_vec_scale[in_index_immediate];
// ret1 = in_vec1 / in_vec_scale[in_index_immediate];
// ret2 = in_vec2 / in_vec_scale[in_index_immediate];
// \todo We could use a reciprocal here to avoid slow division operations.  However, that would give different results (i.e. can't take reciprocal of zero).
#define RW_MATH_VPU_DivideAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
        { \
            RW_MATH_VPU_DivideAxisA1_I(ret0, in_vec0, in_vec_scale, in_index_immediate); \
            RW_MATH_VPU_DivideAxisA1_I(ret1, in_vec1, in_vec_scale, in_index_immediate); \
            RW_MATH_VPU_DivideAxisA1_I(ret2, in_vec2, in_vec_scale, in_index_immediate); \
        } \


//----------------------------------------------------------------------------
// _asmMultAxis()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_MultAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    { \
        VectorIntrinsic splat_multiplier = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec_scale, RWMATH_VPU_DETAIL_sseShuffleSplat(in_index_immediate) ); \
        ret= _mm_mul_ps( in_vec0, splat_multiplier ); \
    } \


// ret0 = in_vec0 * in_vec_scale[in_index_immediate];
// ret1 = in_vec1 * in_vec_scale[in_index_immediate];
// ret2 = in_vec2 * in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
        { \
            RW_MATH_VPU_MultAxisA1_I(ret0, in_vec0, in_vec_scale, in_index_immediate); \
            RW_MATH_VPU_MultAxisA1_I(ret1, in_vec1, in_vec_scale, in_index_immediate); \
            RW_MATH_VPU_MultAxisA1_I(ret2, in_vec2, in_vec_scale, in_index_immediate); \
        } \


#define RW_MATH_VPU_MultAdd_R(in_vec0, in_vec1, in_vec2) \
    _mm_add_ps( _mm_mul_ps( in_vec0, in_vec1 ), in_vec2 ); \
    
#define RW_MATH_VPU_MultAdd(ret, in_vec0, in_vec1, in_vec2) \
    ret= _mm_add_ps( _mm_mul_ps( in_vec0, in_vec1 ), in_vec2 ); \


#define RW_MATH_VPU_MultSubNeg(ret, in_vec0, in_vec1, in_vec2) \
    ret = _mm_sub_ps(in_vec2, _mm_mul_ps(in_vec0, in_vec1)); \


#define RW_MATH_VPU_Add(ret, in_vec0, in_vec1) \
    ret= _mm_add_ps( in_vec0, in_vec1 ); \

#define RW_MATH_VPU_Add_R(in_vec0, in_vec1) \
    _mm_add_ps( in_vec0, in_vec1 )

#define RW_MATH_VPU_Subtract(ret, in_vec0, in_vec1) \
    ret= _mm_sub_ps( in_vec0, in_vec1 ); \

#define RW_MATH_VPU_Subtract_R(in_vec0, in_vec1) \
    _mm_sub_ps( in_vec0, in_vec1 )

#define RW_MATH_VPU_And(ret, in_vec0, in_vec1) \
        ret = _mm_and_ps(in_vec0, in_vec1); \

#define RW_MATH_VPU_And_R(in_vec0, in_vec1) \
         _mm_and_ps(in_vec0, in_vec1)

#define RW_MATH_VPU_And_Rc(in_vec0, in_vec1) \
         _mm_andnot_ps(in_vec1, in_vec0)

#define RW_MATH_VPU_Or(ret, in_vec0, in_vec1) \
        ret = _mm_or_ps(in_vec0, in_vec1); \


#define RW_MATH_VPU_Not(ret, in_vec) \
    { \
        rw::math::detail::Flt32UInt32Union _asmNot_0xf; \
        _asmNot_0xf.i = 0xFFFFFFFF; \
        ret = _mm_xor_ps(in_vec, _mm_set1_ps(_asmNot_0xf.f)); \
    } \


#define RW_MATH_VPU_Xor(ret, in_vec0, in_vec1) \
        ret = _mm_xor_ps(in_vec0, in_vec1); \

#define RW_MATH_VPU_Xor_R(in_vec0, in_vec1) \
        _mm_xor_ps(in_vec0, in_vec1)

#define RW_MATH_VPU_ProductSum3(ret, v0, v1, v2, v3, v4, v5) \
    RW_MATH_VPU_Mult   (ret, v0, v1); \
    RW_MATH_VPU_MultAdd(ret, v2, v3, ret); \
    RW_MATH_VPU_MultAdd(ret, v4, v5, ret); \


//----------------------------------------------------------------------------
// _asmIsEqualV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsEqualV2(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic comparisonMask = _mm_cmpeq_ps( in_vec0, in_vec1 ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= ((msbMask & RWMATH_VPU_DETAIL_sseCompMask(1,1,0,0)) == RWMATH_VPU_DETAIL_sseCompMask(1,1,0,0)); \
    } \


#define RW_MATH_VPU_IsEqualV3(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic comparisonMask = _mm_cmpeq_ps( in_vec0, in_vec1 ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= ((msbMask & RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)) == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)); \
    } \


#define RW_MATH_VPU_IsEqualV4(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic comparisonMask = _mm_cmpeq_ps( in_vec0, in_vec1 ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= (msbMask == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    } \


#define RW_MATH_VPU_Abs(ret, in_vec) \
     ret= _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec), in_vec ); \



//----------------------------------------------------------------------------
// _asmIsZeroV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsZeroA1V2(ret, in_vec, in_tolerance) \
    { \
        VectorIntrinsic abs_value = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec), in_vec ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_value, in_tolerance ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= ((msbMask & RWMATH_VPU_DETAIL_sseCompMask(1,1,0,0)) == RWMATH_VPU_DETAIL_sseCompMask(1,1,0,0)); \
    } \


#define RW_MATH_VPU_IsZeroA1V3(ret, in_vec, in_tolerance) \
    { \
        VectorIntrinsic abs_value = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec), in_vec ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_value, in_tolerance ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= ((msbMask & RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)) == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)); \
    } \


#define RW_MATH_VPU_IsZeroA1V4(ret, in_vec, in_tolerance) \
    { \
        VectorIntrinsic abs_value = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec), in_vec ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_value, in_tolerance ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= (msbMask == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    } \


#define RW_MATH_VPU_IsZeroA2V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        VectorIntrinsic abs_val0 = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec0), in_vec0 ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_val0, in_tolerance ); \
        VectorIntrinsic abs_val1; \
        RW_MATH_VPU_Abs(abs_val1, in_vec1); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val1, in_tolerance ) ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= (msbMask == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    } \


#define RW_MATH_VPU_IsZeroA3V3(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        VectorIntrinsic abs_val0 = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec0), in_vec0 ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_val0, in_tolerance ); \
        VectorIntrinsic abs_val1, abs_val2; \
        RW_MATH_VPU_Abs(abs_val1, in_vec1); \
        RW_MATH_VPU_Abs(abs_val2, in_vec2); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val1, in_tolerance ) ); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val2, in_tolerance ) ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= ((msbMask & RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)) == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)); \
    } \


#define RW_MATH_VPU_IsZeroA3V4(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        VectorIntrinsic abs_val0 = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec0), in_vec0 ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_val0, in_tolerance ); \
        VectorIntrinsic abs_val1, abs_val2; \
        RW_MATH_VPU_Abs(abs_val1, in_vec1); \
        RW_MATH_VPU_Abs(abs_val2, in_vec2); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val1, in_tolerance ) ); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val2, in_tolerance ) ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= (msbMask == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    } \


#define RW_MATH_VPU_IsZeroA4V3(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        VectorIntrinsic abs_val0 = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec0), in_vec0 ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_val0, in_tolerance ); \
        VectorIntrinsic abs_val1, abs_val2, abs_val3; \
        RW_MATH_VPU_Abs(abs_val1, in_vec1); \
        RW_MATH_VPU_Abs(abs_val2, in_vec2); \
        RW_MATH_VPU_Abs(abs_val3, in_vec3); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val1, in_tolerance ) ); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val2, in_tolerance ) ); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val3, in_tolerance ) ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= ((msbMask & RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)) == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,0)); \
    } \


#define RW_MATH_VPU_IsZeroA4V4(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        VectorIntrinsic abs_val0 = _mm_max_ps( _mm_mul_ps( _mm_set1_ps(-1.0f), in_vec0), in_vec0 ); \
        VectorIntrinsic comparisonMask = _mm_cmple_ps( abs_val0, in_tolerance ); \
        VectorIntrinsic abs_val1, abs_val2, abs_val3; \
        RW_MATH_VPU_Abs(abs_val1, in_vec1); \
        RW_MATH_VPU_Abs(abs_val2, in_vec2); \
        RW_MATH_VPU_Abs(abs_val3, in_vec3); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val1, in_tolerance ) ); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val2, in_tolerance ) ); \
        comparisonMask = _mm_and_ps( comparisonMask, _mm_cmple_ps( abs_val3, in_tolerance ) ); \
        int msbMask = _mm_movemask_ps( comparisonMask ); \
        ret= (msbMask == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    } \


//----------------------------------------------------------------------------
// _asmIsSimilarV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsSimilarA1V2(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        VectorIntrinsic temp_diff= _mm_sub_ps( in_vec0, in_vec1 ); \
        bool temp_result; \
        RW_MATH_VPU_IsZeroA1V2(temp_result, temp_diff, in_tolerance); \
        ret= temp_result; \
    } \


#define RW_MATH_VPU_IsSimilarA1V3(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        VectorIntrinsic temp_diff= _mm_sub_ps( in_vec0, in_vec1 ); \
        bool temp_result; \
        RW_MATH_VPU_IsZeroA1V3(temp_result, temp_diff, in_tolerance); \
        ret= temp_result; \
    } \


#define RW_MATH_VPU_IsSimilarA1V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        VectorIntrinsic temp_diff= _mm_sub_ps( in_vec0, in_vec1 ); \
        bool temp_result; \
        RW_MATH_VPU_IsZeroA1V4(temp_result, temp_diff, in_tolerance); \
        ret= temp_result; \
    } \


#define RW_MATH_VPU_IsSimilarA3V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        VectorIntrinsic diff_x= _mm_sub_ps( in_vec0_x, in_vec1_x ); \
        VectorIntrinsic diff_y= _mm_sub_ps( in_vec0_y, in_vec1_y ); \
        VectorIntrinsic diff_z= _mm_sub_ps( in_vec0_z, in_vec1_z ); \
        bool is_zero; \
        RW_MATH_VPU_IsZeroA3V3(is_zero, diff_x, diff_y, diff_z, in_tolerance); \
        ret= is_zero; \
    } \


#define RW_MATH_VPU_IsSimilarA4V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        VectorIntrinsic diff_x= _mm_sub_ps( in_vec0_x, in_vec1_x ); \
        VectorIntrinsic diff_y= _mm_sub_ps( in_vec0_y, in_vec1_y ); \
        VectorIntrinsic diff_z= _mm_sub_ps( in_vec0_z, in_vec1_z ); \
        VectorIntrinsic diff_w= _mm_sub_ps( in_vec0_w, in_vec1_w ); \
        bool is_zero; \
        RW_MATH_VPU_IsZeroA4V3(is_zero, diff_x, diff_y, diff_z, diff_w, in_tolerance); \
        ret= is_zero; \
    } \




#define RW_MATH_VPU_IsSimilarA4V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        VectorIntrinsic diff_x= _mm_sub_ps( in_vec0_x, in_vec1_x ); \
        VectorIntrinsic diff_y= _mm_sub_ps( in_vec0_y, in_vec1_y ); \
        VectorIntrinsic diff_z= _mm_sub_ps( in_vec0_z, in_vec1_z ); \
        VectorIntrinsic diff_w= _mm_sub_ps( in_vec0_w, in_vec1_w ); \
        bool is_zero; \
        RW_MATH_VPU_IsZeroA4V4(is_zero, diff_x, diff_y, diff_z, diff_w, in_tolerance); \
        ret= is_zero; \
    } \


//----------------------------------------------------------------------------
// _asmCross()
//----------------------------------------------------------------------------
// Note: This code has the nice side-effect that the w component
// of 'temp_result' is 0.  So, the code can also be used for Vector4.
#define RW_MATH_VPU_CrossV3(ret, in_vec0, in_vec1) \
    { \
        const int YXZ_SHUFFLE = RWMATH_VPU_DETAIL_sseShuffle(1, 2, 0, 0); \
        const int ZXY_SHUFFLE = RWMATH_VPU_DETAIL_sseShuffle(2, 0, 1, 0); \
        ::rw::math::vpu::VectorIntrinsic v0YZX = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec0, YXZ_SHUFFLE ); \
        ::rw::math::vpu::VectorIntrinsic v0ZXY = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec0, ZXY_SHUFFLE ); \
        ::rw::math::vpu::VectorIntrinsic v1YZX = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec1, YXZ_SHUFFLE ); \
        ::rw::math::vpu::VectorIntrinsic v1ZXY = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec1, ZXY_SHUFFLE ); \
        ::rw::math::vpu::VectorIntrinsic temp_diff= _mm_sub_ps( _mm_mul_ps( v0YZX, v1ZXY ), _mm_mul_ps( v1YZX, v0ZXY )); \
        ret= temp_diff; \
    }


//----------------------------------------------------------------------------
#define RW_MATH_VPU_DotV2(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_product = _mm_mul_ps(in_vec0, in_vec1); \
        VectorIntrinsic splat_x_product= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( temp_product, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisX) ); \
        VectorIntrinsic splat_y_product= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( temp_product, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisY) ); \
        ret= _mm_add_ps( splat_x_product, splat_y_product ); \
    } \

// First multiply the vectors to get the product of each element.
// Let product have the form (px, py, pz, -)
// We need to calculate the sum px + py + pz
// Shuffle to get:
// temp1 = (py, -, -, -)
// temp2 = (pz, -, -, -)
// Calculate sum = (px + py + pz, -, -, -)
// Splat the sum in the x component to all elements.
#define RW_MATH_VPU_DotV3(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_product = _mm_mul_ps(in_vec0, in_vec1); \
        VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( temp_product, RWMATH_VPU_DETAIL_sseShuffle(1, 0, 0, 0) ); \
        VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( temp_product, RWMATH_VPU_DETAIL_sseShuffle(2, 0, 0, 0) ); \
        VectorIntrinsic sum = _mm_add_ps( temp_product, _mm_add_ps(temp1, temp2) ); \
        ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R(sum, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisX)); \
    } \


// First multiply the vectors to get the product of each element.
// Let product have the form (px, py, pz, pw)
// We need to calculate the sum px + py + pz + pw
// Shuffle to get:
// temp1 = (pz, pw, -, -)
// Add product and temp1 to get (px + pz, py + pw, -, -)
// Shuffle sum1 to build ( py + pw, -, -, - )
// Calculate temp_result = (px + py + pz + pw, -, -, - )
// Splat temp_result to all elements
// Optimization: It should be possible to skip this last instruction by some intelligent
// shuffling.  However, it *may* cause the temp_result stored in each element to be slightly
// different from one another which would temp_result in an improper VecFloat.  The optimization
// hasn't been implemented yet for this reason.
#define RW_MATH_VPU_DotV4(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_product = _mm_mul_ps(in_vec0, in_vec1); \
        VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( temp_product, RWMATH_VPU_DETAIL_sseShuffle(2, 3, 0, 0) ); \
        VectorIntrinsic sum1 = _mm_add_ps( temp_product, temp1 ); \
        VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( sum1, RWMATH_VPU_DETAIL_sseShuffle(1, 0, 0, 0) ); \
        VectorIntrinsic temp_result = _mm_add_ps( sum1, temp2 ); \
        ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R(temp_result, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisX)); \
    }


#define RW_MATH_VPU_SquareRoot(ret, in_vec) \
    ret = _mm_sqrt_ps( in_vec );

#define RW_MATH_VPU_SquareRootFast(ret, in_vec) \
    ret = _mm_sqrt_ps( in_vec )

#define RW_MATH_VPU_SquareRootEstimate(ret, in_vec) \
    RW_MATH_VPU_SquareRootFast(ret, in_vec)

#define RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootFastDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRootFast( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootEstimateDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRootEstimate( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootFastDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRootFast( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRootEstimate( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRoot( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootFastDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRootFast( ret, dotproduct ); \
    }

#define RW_MATH_VPU_SquareRootEstimateDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_SquareRootEstimate( ret, dotproduct ); \
    }

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootFast( ret, dotproduct ); \
    } \


#define RW_MATH_VPU_ReciprocalSquareRootDotV2(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV2(temp_dot, in_vec0, in_vec1); \
        VectorIntrinsic temp_result; \
        RW_MATH_VPU_ReciprocalSquareRoot(temp_result, temp_dot); \
        ret= temp_result; \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV3(temp_dot, in_vec0, in_vec1); \
        VectorIntrinsic temp_result; \
        RW_MATH_VPU_ReciprocalSquareRoot(temp_result, temp_dot); \
        ret= temp_result; \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV3(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV3(temp_dot, in_vec0, in_vec1); \
        VectorIntrinsic temp_result; \
        RW_MATH_VPU_ReciprocalSquareRootFast(temp_result, temp_dot); \
        ret= temp_result; \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV4(temp_dot, in_vec0, in_vec1); \
        VectorIntrinsic temp_result; \
        RW_MATH_VPU_ReciprocalSquareRoot(temp_result, temp_dot); \
        ret= temp_result; \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootFast( ret, dotproduct ); \
    } \

//----------------------------------------------------------------------------
#define RW_MATH_VPU_NormalizeV2(ret, in_vec) \
    { \
        VectorIntrinsic temp_one_over_sqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV2(temp_one_over_sqrt_dot, in_vec, in_vec); \
        ret= _mm_mul_ps(in_vec, temp_one_over_sqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeEstimateV2(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV2(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeFastV2(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV2(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeFastV3(ret, in_vec) \
    { \
        VectorIntrinsic temp_one_over_sqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV3(temp_one_over_sqrt_dot, in_vec, in_vec); \
        ret= _mm_mul_ps(in_vec, temp_one_over_sqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeEstimateV3(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeV3(ret, in_vec) \
    { \
        VectorIntrinsic temp_one_over_sqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV3(temp_one_over_sqrt_dot, in_vec, in_vec); \
        ret= _mm_mul_ps(in_vec, temp_one_over_sqrt_dot); \
    } \


#define RW_MATH_VPU_NormalizeV4(ret, in_vec) \
    { \
        VectorIntrinsic temp_one_over_sqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV4(temp_one_over_sqrt_dot, in_vec, in_vec); \
        ret= _mm_mul_ps(in_vec, temp_one_over_sqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeEstimateV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeFastV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

//NOTE the last several lines in this macro will zero the magnitude (ret_mag) if in_vec is zero (done elementwise)
#define RW_MATH_VPU_NormalizeWithMagnitudeV2(ret, ret_mag, in_vec) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV2(temp_dot, in_vec, in_vec); \
        VectorIntrinsic reciprocal_square_root; \
        RW_MATH_VPU_ReciprocalSquareRoot(reciprocal_square_root, temp_dot); \
        VectorIntrinsic result_mag= _mm_mul_ps( temp_dot, reciprocal_square_root ); \
        ret= _mm_mul_ps(in_vec, reciprocal_square_root); \
        VectorIntrinsic nm_zero= _mm_setzero_ps(); \
        VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, nm_zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, nm_zero, input_is_zero); \
    } \


//NOTE the last several lines in this macro will zero the magnitude (ret_mag) if in_vec is zero (done elementwise)
#define RW_MATH_VPU_NormalizeWithMagnitudeV3(ret, ret_mag, in_vec) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV3(temp_dot, in_vec, in_vec); \
        VectorIntrinsic reciprocal_square_root; \
        RW_MATH_VPU_ReciprocalSquareRoot(reciprocal_square_root, temp_dot); \
        VectorIntrinsic result_mag= _mm_mul_ps( temp_dot, reciprocal_square_root ); \
        ret= _mm_mul_ps(in_vec, reciprocal_square_root); \
        VectorIntrinsic nm_zero= _mm_setzero_ps(); \
        VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, nm_zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, nm_zero, input_is_zero); \
    } \


//NOTE the last several lines in this macro will zero the magnitude (ret_mag) if in_vec is zero (done elementwise)
#define RW_MATH_VPU_NormalizeWithMagnitudeV4(ret, ret_mag, in_vec) \
    { \
        VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV4(temp_dot, in_vec, in_vec); \
        VectorIntrinsic reciprocal_square_root; \
        RW_MATH_VPU_ReciprocalSquareRoot(reciprocal_square_root, temp_dot); \
        VectorIntrinsic result_mag= _mm_mul_ps( temp_dot, reciprocal_square_root ); \
        ret= _mm_mul_ps(in_vec, reciprocal_square_root); \
        VectorIntrinsic nm_zero= _mm_setzero_ps(); \
        VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, nm_zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, nm_zero, input_is_zero); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Lerp(ret, in_vec0, in_vec1, in_t) \
    { \
        VectorIntrinsic temp_lerp_v; \
        RW_MATH_VPU_Subtract(temp_lerp_v, in_vec1, in_vec0); \
        ret= _mm_add_ps( _mm_mul_ps( in_t, temp_lerp_v ), in_vec0 ); \
    } \


#define RW_MATH_VPU_Splat_I(ret, in_vec, in_index_immediate) \
    { \
        ret= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec, RWMATH_VPU_DETAIL_sseShuffleSplat(in_index_immediate) ); \
    } \


#define RW_MATH_VPU_Splat(ret, in_vec, in_index) \
    { \
        float temp_val = reinterpret_cast<const float*>(&in_vec)[in_index]; \
        ret = _mm_set1_ps(temp_val); \
    } \

#define RW_MATH_VPU_TransformVectorV3(ret, in_vec, in_x, in_y, in_z) \
    { \
        VectorIntrinsic splat_x= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisX) ); \
        VectorIntrinsic splat_y= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisY) ); \
        VectorIntrinsic splat_z= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( in_vec, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisZ) ); \
        VectorIntrinsic temp_result = _mm_mul_ps(in_x, splat_x); \
        RW_MATH_VPU_MultAdd(temp_result, in_y, splat_y, temp_result); \
        RW_MATH_VPU_MultAdd(temp_result, in_z, splat_z, temp_result); \
        ret = temp_result; \
    } \


// Note: RWMATH_VPU_DETAIL_VecMergeH mapping to unpacklo is intentional to
// achieve same behavior as VMX.
#define RWMATH_VPU_DETAIL_VecMergeH( in_a, in_b ) \
    _mm_unpacklo_ps( (in_a), (in_b) )

// Note: RWMATH_VPU_DETAIL_VecMergeL mapping to unpackhi is intentional to
// achieve same behavior as VMX.
#define RWMATH_VPU_DETAIL_VecMergeL( in_a, in_b ) \
    _mm_unpackhi_ps( (in_a), (in_b) )

#define RW_MATH_VPU_CompAnyTrueV3( ret, a ) \
    { \
        VectorIntrinsic allequal_temp_xxyy = RWMATH_VPU_DETAIL_VecMergeH( a, a ); \
        VectorIntrinsic allequal_temp_zzww = RWMATH_VPU_DETAIL_VecMergeL( a, a ); \
        VectorIntrinsic allequal_or_xz_xz_yw_yw = _mm_or_ps( allequal_temp_xxyy, allequal_temp_zzww ); \
        VectorIntrinsic allequal_or_xz_xz_xz_xz = RWMATH_VPU_DETAIL_VecMergeH( allequal_or_xz_xz_yw_yw, allequal_or_xz_xz_yw_yw ); \
        VectorIntrinsic allequal_y_y_y_y; \
        RW_MATH_VPU_Splat_I( allequal_y_y_y_y, a, VectorAxisY ); \
        ret = _mm_or_ps(allequal_or_xz_xz_xz_xz, allequal_y_y_y_y); \
    }

#define RW_MATH_VPU_CompAllTrueV3( ret, a ) \
    { \
        VectorIntrinsic allequal_temp_xxyy = RWMATH_VPU_DETAIL_VecMergeH( a, a ); \
        VectorIntrinsic allequal_temp_zzww = RWMATH_VPU_DETAIL_VecMergeL( a, a ); \
        VectorIntrinsic allequal_and_xz_xz_yw_yw = _mm_and_ps( allequal_temp_xxyy, allequal_temp_zzww ); \
        VectorIntrinsic allequal_and_xz_xz_xz_xz = RWMATH_VPU_DETAIL_VecMergeH( allequal_and_xz_xz_yw_yw, allequal_and_xz_xz_yw_yw ); \
        VectorIntrinsic allequal_y_y_y_y; \
        RW_MATH_VPU_Splat_I( allequal_y_y_y_y, a, VectorAxisY ); \
        ret = _mm_and_ps(allequal_and_xz_xz_xz_xz, allequal_y_y_y_y); \
    }

#define RW_MATH_VPU_CompAnyTrueV4( ret, a ) \
    { \
        VectorIntrinsic allequal_temp_xxyy = RWMATH_VPU_DETAIL_VecMergeH( a, a ); \
        VectorIntrinsic allequal_temp_zzww = RWMATH_VPU_DETAIL_VecMergeL( a, a ); \
        VectorIntrinsic allequal_or_xz_xz_yw_yw = _mm_or_ps( allequal_temp_xxyy, allequal_temp_zzww ); \
        VectorIntrinsic allequal_or_xz_xz_xz_xz = RWMATH_VPU_DETAIL_VecMergeH( allequal_or_xz_xz_yw_yw, allequal_or_xz_xz_yw_yw ); \
        VectorIntrinsic allequal_or_yw_yw_yw_yw = RWMATH_VPU_DETAIL_VecMergeL( allequal_or_xz_xz_yw_yw, allequal_or_xz_xz_yw_yw ); \
        ret = _mm_or_ps(allequal_or_xz_xz_xz_xz, allequal_or_yw_yw_yw_yw); \
    }

#define RW_MATH_VPU_CompAllTrueV4( ret, a ) \
    { \
        VectorIntrinsic allequal_temp_xxyy = RWMATH_VPU_DETAIL_VecMergeH( a, a ); \
        VectorIntrinsic allequal_temp_zzww = RWMATH_VPU_DETAIL_VecMergeL( a, a ); \
        VectorIntrinsic allequal_and_xz_xz_yw_yw = _mm_and_ps( allequal_temp_xxyy, allequal_temp_zzww ); \
        VectorIntrinsic allequal_and_xz_xz_xz_xz = RWMATH_VPU_DETAIL_VecMergeH( allequal_and_xz_xz_yw_yw, allequal_and_xz_xz_yw_yw ); \
        VectorIntrinsic allequal_and_yw_yw_yw_yw = RWMATH_VPU_DETAIL_VecMergeL( allequal_and_xz_xz_yw_yw, allequal_and_xz_xz_yw_yw ); \
        ret = _mm_and_ps(allequal_and_xz_xz_xz_xz, allequal_and_yw_yw_yw_yw); \
    }

#define RW_MATH_VPU_ExtractXElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp0, in_y); \
    } \

#define RW_MATH_VPU_ExtractYElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    { \
        /* Code could be modified to remove one merge by switching to permutes/shuffles */ \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_y); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp0, temp1); \
    } \

#define RW_MATH_VPU_ExtractZElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    { \
        /* Code could be modified to remove one merge by switching to permutes/shuffles */ \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_y); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp2, temp3); \
    } \

#define RW_MATH_VPU_ExtractXElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_w); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp0, temp1); \
    } \

#define RW_MATH_VPU_ExtractYElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_w); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp0, temp1); \
    } \

#define RW_MATH_VPU_ExtractZElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_w); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp2, temp3); \
    } \

#define RW_MATH_VPU_ExtractWElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_w); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp2, temp3); \
    } \

#define RW_MATH_VPU_TransposeA3V3(ret_x, ret_y, ret_z, in_x, in_y, in_z) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_y/*dummy arg*/); \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_y/*dummy arg*/); \
        ret_x = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp0, temp1); \
        ret_y = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp0, temp1); \
        ret_z = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp2, temp3); \
    }


#define RW_MATH_VPU_TransposeA4V4(ret_x, ret_y, ret_z, ret_w, in_x, in_y, in_z, in_w) \
    { \
        ret_x = in_x; \
        ret_y = in_y; \
        ret_z = in_z; \
        ret_w = in_w; \
        _MM_TRANSPOSE4_PS(ret_x,ret_y,ret_z,ret_w); \
    }

#define RW_MATH_VPU_TransposeA4V3(ret_x, ret_y, ret_z, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_w); \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_w); \
        ret_x = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp0, temp1); \
        ret_y = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp0, temp1); \
        ret_z = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp2, temp3); \
    }


//----------------------------------------------------------------------------
//\todo Combine transpose and divide to hide latency of divide instruction.
#define RW_MATH_VPU_InverseA3V3(ret_d, ret_x, ret_y, ret_z, in_x, in_y, in_z) \
    { \
        VectorIntrinsic temp_crossYZ; \
        VectorIntrinsic temp_crossZX; \
        VectorIntrinsic temp_crossXY; \
        RW_MATH_VPU_CrossV3(temp_crossYZ, in_y, in_z); \
        RW_MATH_VPU_CrossV3(temp_crossZX, in_z, in_x); \
        RW_MATH_VPU_CrossV3(temp_crossXY, in_x, in_y); \
        RW_MATH_VPU_DotV3(ret_d, in_x, temp_crossYZ); \
        VectorIntrinsic temp_transpose_x; \
        VectorIntrinsic temp_transpose_y; \
        VectorIntrinsic temp_transpose_z; \
        RW_MATH_VPU_TransposeA3V3(temp_transpose_x, temp_transpose_y, temp_transpose_z, temp_crossYZ, temp_crossZX, temp_crossXY); \
        RW_MATH_VPU_DivideAxisA3_I(ret_x, ret_y, ret_z, temp_transpose_x, temp_transpose_y, temp_transpose_z, ret_d, VectorAxisX); \
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


//----------------------------------------------------------------------------
// addVec = (xy + wz, yz + wx, zx + wy) and
// subVec = (xy - wz, yz - wx, zx - wy)
// diagonalValues = ( neg_xx+neg_yy, neg_yy+neg_zz, neg_zz+neg_xx )
//ret_x = _asmCreateVectorAxis<VectorAxisY,VectorAxisX,VectorAxisZ>( /*(neg_yy+neg_zz)*/diagonalValues, /*xy+wz*/             addVec,         /*(zx-wy)*/         subVec );
//ret_y = _asmCreateVectorAxis<VectorAxisX,VectorAxisZ,VectorAxisY>( /*(xy-wz)*/        subVec,       /*(neg_zz+neg_xx)*/   diagonalValues, /*(yz+wx)*/         addVec );
//ret_z = _asmCreateVectorAxis<VectorAxisZ,VectorAxisY,VectorAxisX>( /*(zx+wy)*/        addVec,         /*(yz-wx)*/           subVec,         /*(neg_xx+neg_yy)*/ diagonalValues );
#define RW_MATH_VPU_MatrixFromQuaternion(ret_x, ret_y, ret_z, in_q) \
    { \
        VectorIntrinsic fourSqrt2s = _mm_set1_ps( SQRT_2 ); \
        VectorIntrinsic fourHalves = _mm_set1_ps( 0.5f ); \
        VectorIntrinsic scaled_q = _mm_mul_ps(fourSqrt2s, in_q); \
        VectorIntrinsic negScaled_q; \
        RW_MATH_VPU_Negate(negScaled_q, scaled_q); \
        VectorIntrinsic neg_qq = _mm_add_ps( _mm_mul_ps( negScaled_q, scaled_q), fourHalves ); \
        VectorIntrinsic scaled_q_yzx = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( scaled_q, RWMATH_VPU_DETAIL_sseShuffle(1, 2, 0, 0) ); \
        VectorIntrinsic xy_yz_zx = _mm_mul_ps( scaled_q, scaled_q_yzx ); \
        VectorIntrinsic scaled_q_zxy = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( scaled_q, RWMATH_VPU_DETAIL_sseShuffle(2, 0, 1, 0) ); \
        VectorIntrinsic splatQ= RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( scaled_q, RWMATH_VPU_DETAIL_sseShuffleSplat(VectorAxisW) ); \
        VectorIntrinsic wz_wx_wy = _mm_mul_ps(splatQ, scaled_q_zxy); \
        VectorIntrinsic addVec = _mm_add_ps( xy_yz_zx, wz_wx_wy ); \
        VectorIntrinsic subVec = _mm_sub_ps( xy_yz_zx, wz_wx_wy ); \
        VectorIntrinsic neg_yy_zz_xx = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( neg_qq, RWMATH_VPU_DETAIL_sseShuffle(1, 2, 0, 0) ); \
        VectorIntrinsic diagonalValues = _mm_add_ps(neg_qq, neg_yy_zz_xx); \
        VectorIntrinsic vecAxisX, vecAxisY, vecAxisZ; \
        RW_MATH_VPU_CreateVectorAxisA3_I(vecAxisX, diagonalValues, VectorAxisY, addVec, VectorAxisX, subVec, VectorAxisZ); \
        RW_MATH_VPU_CreateVectorAxisA3_I(vecAxisY, subVec, VectorAxisX, diagonalValues, VectorAxisZ, addVec, VectorAxisY); \
        RW_MATH_VPU_CreateVectorAxisA3_I(vecAxisZ, addVec, VectorAxisZ, subVec, VectorAxisY, diagonalValues, VectorAxisX); \
        ret_x= vecAxisX; \
        ret_y= vecAxisY; \
        ret_z= vecAxisZ; \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Min(ret, in_vec0, in_vec1) \
    ret= _mm_min_ps(in_vec0, in_vec1); \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Max(ret, in_vec0, in_vec1) \
    ret= _mm_max_ps(in_vec0, in_vec1); \

#define RW_MATH_VPU_ClampToNegPosRange(ret, in_vec0, range) \
    { \
        VectorIntrinsic signMask; \
        RW_MATH_VPU_GetSignMaskXYZW(signMask); \
        VectorIntrinsic m = _mm_andnot_ps(signMask, _mm_or_ps(_mm_cmpgt_ps(in_vec0, range), _mm_cmplt_ps(in_vec0, _mm_xor_ps(range, signMask)))); \
        ret = _mm_or_ps(_mm_andnot_ps(m, in_vec0), _mm_and_ps(m, range)); \
    }    

// Trig functions
#define RW_MATH_VPU_Cos( ret, v ) \
    { \
        VectorIntrinsic temp0; \
        RW_MATH_VPU_Abs( temp0, v ); \
     \
        VectorIntrinsic acc, temp1, cos_scale; \
        RW_MATH_VPU_Mult( acc, temp0, detail::gNegOneOverTwoPi ); \
        cos_scale = acc; \
        RW_MATH_VPU_Subtract( acc, acc, detail::gRemoveFraction ); \
        RW_MATH_VPU_Add( acc, acc, detail::gRemoveFraction ); \
        RW_MATH_VPU_Subtract( acc, acc, cos_scale ); \
        RW_MATH_VPU_Subtract( temp1, acc, detail::gOneHalf ); \
     \
        VectorIntrinsic x; \
        RW_MATH_VPU_Abs( x, temp1 ); \
        RW_MATH_VPU_Subtract( x, x, detail::gOneQuarter ); \
     \
        VectorIntrinsic x2; \
        x2  = RW_MATH_VPU_Mult_R( x, x ); \
        ret = RW_MATH_VPU_Mult_R( x, detail::gTaylorCosXCoefficient ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX3Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX5Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX7Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX9Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX11Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX13Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX15Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX17Coefficient, ret ); \
        x   = RW_MATH_VPU_Mult_R( x, x2 ); \
        ret = RW_MATH_VPU_MultAdd_R( x, detail::gTaylorCosX19Coefficient, ret ); \
    }


#define RW_MATH_VPU_Sin( ret, v ) \
    { \
        VectorIntrinsic angle; \
        RW_MATH_VPU_Subtract( angle, v, detail::gOneHalfPi ); \
        RW_MATH_VPU_Cos( ret, angle ); \
    }

#define RW_MATH_VPU_SinCos(ret0, ret1, in_vec) \
    RW_MATH_VPU_Sin( ret0, in_vec ); \
    RW_MATH_VPU_Cos( ret1, in_vec ); 

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
}

namespace detail
{
// Note: These are platform specific vector intrinsic macros.

#define RW_MATH_VPU_PermuteSingle_I(ret, vec, pattern) \
        ret = RWMATH_VPU_DETAIL_SSE_SINGLE_IMM_SHUFFLE_R( vec, RWMATH_VPU_DETAIL_sseShuffle( (((pattern) & 0xf000) >> 12), (((pattern) & 0x0f00) >>  8), (((pattern) & 0x00f0) >>  4), (((pattern) & 0x000f) >>  0) ) );

//----------------------------------------------------------------------------
// _asmSwizzleStoreV3()
//----------------------------------------------------------------------------
// 'mixed_vector' stores the src element that is to be inserted in the destination AND the element that is in
// the same half of the destination register as where the element is being inserted.
#define RW_MATH_VPU_SwizzleStoreV3_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    { \
        VectorIntrinsic mixed_vector = _mm_shuffle_ps( vec_dest, vec_src, RWMATH_VPU_DETAIL_sseShuffle( vec_dest_index_immediate ^ 1, vec_dest_index_immediate ^ 1, \
        vec_src_index_immediate, vec_src_index_immediate) ); \
        switch( vec_dest_index_immediate ) \
        { \
        case VectorAxisX: \
            ret = _mm_shuffle_ps( mixed_vector, vec_dest, RWMATH_VPU_DETAIL_sseShuffle( 2,0,2,3) ); \
            break; \
        case VectorAxisY: \
            ret = _mm_shuffle_ps( mixed_vector, vec_dest, RWMATH_VPU_DETAIL_sseShuffle( 0,2,2,3) ); \
            break; \
        case VectorAxisZ: \
            ret = _mm_shuffle_ps( vec_dest, vec_src, RWMATH_VPU_DETAIL_sseShuffle( 0,1,vec_src_index_immediate,0) ); \
            break; \
        default: \
            ret = vec_dest; \
            break; \
        } \
    } \


//----------------------------------------------------------------------------
// _asmSwizzleStoreV2()
//----------------------------------------------------------------------------
// 'mixed_vector' stores the src element that is to be inserted in the destination AND the element that is in
// the same half of the destination register as where the element is being inserted.
#define RW_MATH_VPU_SwizzleStoreV2_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    { \
        VectorIntrinsic mixed_vector = _mm_shuffle_ps( vec_dest, vec_src, RWMATH_VPU_DETAIL_sseShuffle( vec_dest_index_immediate ^ 1, vec_dest_index_immediate ^ 1, \
        vec_src_index_immediate, vec_src_index_immediate) ); \
        switch( vec_dest_index_immediate ) \
        { \
        case VectorAxisX: \
            ret = _mm_shuffle_ps( mixed_vector, vec_dest, RWMATH_VPU_DETAIL_sseShuffle( 2,0,0,0) ); \
            break; \
        case VectorAxisY: \
            ret = _mm_shuffle_ps( mixed_vector, vec_dest, RWMATH_VPU_DETAIL_sseShuffle( 0,2,0,0) ); \
            break; \
        default: \
            ret = vec_dest; \
            break; \
        } \
    } \

#define RW_MATH_VPU_IsAnyTrue(ret, in_vec) \
    { \
        int msbMask = _mm_movemask_ps( in_vec ); \
        ret = (msbMask != 0); \
    }

#define RW_MATH_VPU_IsAllTrue(ret, in_vec) \
    { \
        int msbMask = _mm_movemask_ps( in_vec ); \
        ret = (msbMask == RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    }

#define RW_MATH_VPU_IsAnyFalse(ret, in_vec) \
    { \
        int msbMask = _mm_movemask_ps( in_vec ); \
        ret = (msbMask != RWMATH_VPU_DETAIL_sseCompMask(1,1,1,1)); \
    }

#define RW_MATH_VPU_IsAllFalse(ret, in_vec) \
    { \
        int msbMask = _mm_movemask_ps( in_vec ); \
        ret = (msbMask == 0); \
    }

} // namespace detail

} // namespace math
} // namespace rw

#endif






