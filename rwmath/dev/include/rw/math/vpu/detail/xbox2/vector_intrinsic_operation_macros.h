// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_MACRO_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_MACRO_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif
#include <vectorintrinsics.h>

typedef __declspec(align(16)) unsigned int __vector4i[4];

namespace rw
{
namespace math
{
namespace vpu
{

#define RW_MATH_VPU_V2_RotateLeft( ret, vec) \
{ \
	ret = __vpermwi( vec, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl( 1,0,2,3 ) ); \
} \

#define RW_MATH_VPU_V3_RotateLeft( ret, vec) \
{ \
	ret = __vpermwi( vec, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl( 1,2,0,3 ) ); \
} \

#define RW_MATH_VPU_V3_RotateRight( ret, vec) \
{ \
	ret = __vpermwi( vec, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl( 2,0,1,3 ) ); \
} \

#define RW_MATH_VPU_V4_RotateLeft( ret, vec) \
{ \
	ret = __vpermwi( vec, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl( 1,2,3,0 ) ); \
} \

#define RW_MATH_VPU_V4_RotateRight( ret, vec) \
{ \
	ret = __vpermwi( vec, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl( 3,0,1,2 ) ); \
} \

#define RW_MATH_VPU_LoadFromAlignedFloatPointer( address ) \
    __lvx( address, 0 )

#define RW_MATH_VPU_UnalignedLoad(ret, address) \
    ret = __vor( __lvlx( (address), 0 ), __lvrx( (address), 16 ) ) \

#define RW_MATH_VPU_UnalignedStore(vec, address) \
    __stvlx( vec, (address), 0 ); __stvrx( vec, (address), 16 ) \

#define RW_MATH_VPU_UnalignedStore2(dest, vector) \
    __stvewx(__vspltw(vector, 0), &dest.x, 0); \
    __stvewx(__vspltw(vector, 1), &dest.x , 4);
        

#define RW_MATH_VPU_UnalignedStore3(dest, vector) \
    __stvewx(__vspltw(vector, 0), &dest.x, 0); \
    __stvewx(__vspltw(vector, 1), &dest.x, 4); \
    __stvewx(__vspltw(vector, 2), &dest.x, 8);

#define RW_MATH_VPU_UnalignedStore4(dest, vector) \
    __stvlx( vector, (&dest.x), 0 ); __stvrx( vector, (&dest.x), 16 ) \

namespace detail {

    enum VectorPermSource
    {
        X0,
        Y0,
        Z0,
        W0,
        X1,
        Y1,
        Z1,
        W1
    };

    enum Vector4IntrinsicPermute_xbox2
    {
        Vector4IntrinsicPermute_X0          = 0x00010203,    // ((i * 4) + 0) << 24 + ((i * 4) + 1) << 16 + ((i * 4) + 2) << 8 + ((i * 4) + 3)
        Vector4IntrinsicPermute_Y0          = 0x04050607,
        Vector4IntrinsicPermute_Z0          = 0x08090a0b,
        Vector4IntrinsicPermute_W0          = 0x0c0d0e0f,
        Vector4IntrinsicPermute_X1          = 0x10111213,
        Vector4IntrinsicPermute_Y1          = 0x14151617,
        Vector4IntrinsicPermute_Z1          = 0x18191a1b,
        Vector4IntrinsicPermute_W1          = 0x1c1d1e1f
    };

    #define _asmCreatePermuteInt32(index) \
        (int)::rw::math::vpu::detail::Vector4IntrinsicPermute_xbox2     \
            ( \
            (((index * 4) + 0) << 24) +  \
            (((index * 4) + 1) << 16) +  \
            (((index * 4) + 2) <<  8) + \
            ((index * 4) + 3) \
            )
    
    union vector4i_vector4_union
    {
        __vector4i vector4i;
        __vector4 vector4;
    };

    template<VectorPermSource xsrc, VectorPermSource ysrc, VectorPermSource zsrc, VectorPermSource wsrc>
    struct VectorPermuteConstant
    {
        static const vector4i_vector4_union value;
    };

    template<VectorPermSource xsrc, VectorPermSource ysrc, VectorPermSource zsrc, VectorPermSource wsrc>
    const vector4i_vector4_union VectorPermuteConstant<xsrc, ysrc, zsrc, wsrc>::value = 
    {
        _asmCreatePermuteInt32(xsrc),
        _asmCreatePermuteInt32(ysrc),
        _asmCreatePermuteInt32(zsrc),
        _asmCreatePermuteInt32(wsrc)
    };
}

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

//----------------------------------------------------------------------------
// Helper macros
//----------------------------------------------------------------------------
#define RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(x, y, z, w) \
    ((((unsigned int)(x) << 6) | ((unsigned int)(y) << 4) | ((unsigned int)(z) << 2) | ((unsigned int)(w) << 0)) & 0xFF)

#define RW_MATH_VPU_PermuteSingle_I(ret, vec, pattern) \
    ret = __vpermwi( vec, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl( (((pattern) & 0xf000) >> 12), (((pattern) & 0x0f00) >>  8), (((pattern) & 0x00f0) >>  4), (((pattern) & 0x000f) >>  0) ) )

#define RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( X_INDEX, Y_INDEX, Z_INDEX, W_INDEX ) \
        (::rw::math::vpu::detail::VectorPermuteConstant< X_INDEX, Y_INDEX, Z_INDEX, W_INDEX >::value.vector4)

#define RWMATH_VPU_DETAIL_GetSwizzleControlV( DEST, SRC ) \
        ::rw::math::vpu::detail::gSwizzleStoreConstants[(((DEST) * 4 /* 4 = Number of choices for SRC */) + (SRC))]

#define RWMATH_VPU_DETAIL_GetSwizzleControlI( DEST, SRC ) \
        (::rw::math::vpu::detail::VectorPermuteConstant< \
            (DEST==VectorAxisX) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::X0, \
            (DEST==VectorAxisY) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::Y0, \
            (DEST==VectorAxisZ) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::Z0, \
            (DEST==VectorAxisW) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::W0  \
         >::value.vector4)

#define RWMATH_VPU_DETAIL_DuplicateXtoW_R(in_vec) \
        /* The macro implements the following code:                                         \
         *   const int writeMask = ( 0 << 3 ) | ( 0 << 2 ) | ( 0 << 1 ) | ( 1 << 0 );       \
         *   const int rotateLeft = 1;                                                      \
         *   result = __vrlimi(v, v, writeMask, rotateLeft);                                \
         */                                                                                 \
        (RWMATH_VPU_DETAIL_VecRlimi(in_vec, in_vec, 1, 1 ))

#define RWMATH_VPU_DETAIL_DuplicateXYtoZW_R(in_vec) \
        /* The macro implements the following code:                                         \
         *   const int writeMask = ( 0 << 3 ) | ( 0 << 2 ) | ( 1 << 1 ) | ( 1 << 0 );       \
         *   const int rotateLeft = 2;                                                      \
         *   result = __vrlimi(v, v, writeMask, rotateLeft);                                \
         */                                                                                 \
        (RWMATH_VPU_DETAIL_VecRlimi(in_vec, in_vec, 3, 2 ))

#define RW_MATH_VPU_GetHalves(ret) \
    { \
        ret = RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 1); \
    } \

#define RW_MATH_VPU_GetSqrt2s(ret) \
    { \
        ret = ::rw::math::vpu::detail::gSqrt2s; \
    } \

#define RW_MATH_VPU_GetHalves_R() \
    ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 1))

//----------------------------------------------------------------------------
#define RW_MATH_VPU_Select(ret, a, b, c) \
    ret = RW_MATH_VPU_Select_R(a, b, c)

//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsGreater(ret, a, b) \
    ret = __vcmpgtfp(a, b);

#define RW_MATH_VPU_IsGreater_R(a, b) \
    __vcmpgtfp(a, b)

//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsGreaterEqual(ret, a, b) \
    ret = __vcmpgefp(a, b);

//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsEqual(ret, a, b) \
    ret = __vcmpeqfp(a, b);

#define RW_MATH_VPU_IsEqual_R(a, b) \
    __vcmpeqfp(a, b)

#define RWMATH_CRMASK_CR6TRUE 0x80
#define RWMATH_CRMASK_CR6FALSE 0x20

#define RWMATH_COMPARE_ALL_TRUE(X) (((X) & RWMATH_CRMASK_CR6TRUE) == RWMATH_CRMASK_CR6TRUE)
#define RWMATH_COMPARE_ANY_TRUE(X) (((X) & RWMATH_CRMASK_CR6FALSE) != RWMATH_CRMASK_CR6FALSE)

__forceinline unsigned int RWMATH_VPU_VMX128_Vector4Equal(__vector4 v1, __vector4 v2)
{
    unsigned int result;
    __vcmpeqfpR(v1, v2, &result);
    return result;
}

__forceinline unsigned int RWMATH_VPU_VMX128_Vector4Greater(__vector4 v1, __vector4 v2)
{
    unsigned int result;
    __vcmpgtfpR(v1, v2, &result);
    return result;
}

__forceinline unsigned int RWMATH_VPU_VMX128_Vector4GreaterEqual(__vector4 v1, __vector4 v2)
{
    unsigned int result;
    __vcmpgefpR(v1, v2, &result);
    return result;
}

__forceinline unsigned int RWMATH_VPU_VMX128_Vector4EqualInt(__vector4 v1, __vector4 v2)
{
    unsigned int result;
    __vcmpequwR(v1, v2, &result);
    return result;
}

__vector4 RWMATH_Sin(__vector4 v);
__vector4 RWMATH_Cos(__vector4 v);
__vector4 RWMATH_ACos(__vector4 v);
void RWMATH_SinCos(__vector4 *s, __vector4 *c, __vector4 v);
__vector4 RWMATH_Tan(__vector4 v);
__vector4 RWMATH_ATan(__vector4 v);
__vector4 RWMATH_ASin(__vector4 v);
__vector4 RWMATH_Log(__vector4 v);
__vector4 RWMATH_Pow(__vector4 v1, __vector4 v2);
__vector4 RWMATH_Exp(__vector4 v);

// The scalar comparison macros assume that all elements of the vector are identical.  The return result is either boolean or
// an integer that indicates true/false.
#define RW_MATH_VPU_Scalar_IsEqual(result, a, b) \
    result = RWMATH_COMPARE_ALL_TRUE(RWMATH_VPU_VMX128_Vector4Equal(a, b))

#define RW_MATH_VPU_Scalar_IsGreater(result, a, b) \
    result = RWMATH_COMPARE_ALL_TRUE(RWMATH_VPU_VMX128_Vector4Greater(a, b))

#define RW_MATH_VPU_Scalar_IsGreaterEqual(result, a, b) \
    result = RWMATH_COMPARE_ALL_TRUE(RWMATH_VPU_VMX128_Vector4GreaterEqual(a, b))

#define RW_MATH_VPU_Scalar_IsLess(result, a, b) \
    result = RWMATH_COMPARE_ALL_TRUE(RWMATH_VPU_VMX128_Vector4Greater(b, a))

#define RW_MATH_VPU_Scalar_IsLessEqual(result, a, b) \
    result = RWMATH_COMPARE_ALL_TRUE(RWMATH_VPU_VMX128_Vector4GreaterEqual(b, a))

// create a vector intrinsic from a pointer to a float.
// The float data will be splatted across the entire vector.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    { \
        ret = RWMATH_VPU_DETAIL_VecSplat(__lvlx((fPtr), 0), 0); \
    } \

#define RW_MATH_VPU_CreateVectorMaskFromBool( ret, boolValue ) \
    { \
        ret = ::rw::math::vpu::detail::gBooleanConstants[ boolValue ? 1 : 0 ]; \
    }

//----------------------------------------------------------------------------
// create a vector intrinsic data from 1 float
// ret - VectorIntrinsic
// f0 - float
#define RW_MATH_VPU_CreateVectorV1(ret, f0) \
    { \
        ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_CreateVectorV1_v = { f0 }; \
        ret = RWMATH_VPU_DETAIL_VecSplat(RW_MATH_VPU_CreateVectorV1_v, 0); \
    } \

#define RW_MATH_VPU_CreateVectorFromScalar_R( f0 ) \
    ::rw::math::vpu::detail::macro_impl_asmCreateVector(f0)

// create a vector intrinsic data from two floats
#define RW_MATH_VPU_CreateVectorV2(ret, f0, f1) \
    { \
        ::rw::math::vpu::VectorIntrinsic vi_temp = {f0, f1, f0, f0}; \
        ret = vi_temp; \
    } \

// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV3(ret, f0, f1, f2) \
    { \
        ::rw::math::vpu::VectorIntrinsic vi_temp = {f0, f1, f2, f0}; \
        ret = vi_temp; \
    } \

// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV4(ret, f0, f1, f2, f3) \
    { \
        ::rw::math::vpu::VectorIntrinsic vi_temp = {f0, f1, f2, f3}; \
        ret = vi_temp; \
    } \

#define RW_MATH_VPU_SetWToZero(ret, v ) \
    ret = RW_MATH_VPU_And_R( v, ::rw::math::vpu::detail::gXYZMask );

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
    ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(vec_dest, vec_src, RWMATH_VPU_DETAIL_GetSwizzleControlV(vec_dest_index, vec_src_index)); \


#define RW_MATH_VPU_SwizzleStore_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    { \
        /* Use "Vector Rotate Left Immediate and Mask Insert" intrinsic to swizzle store.           \
            const int writeMask = 1 << (3-vec_dest_index_immediate);                                \
            const int rotateLeft = (4 + vec_src_index_immediate - vec_dest_index_immediate) % 4;    \
        */ \
        ret = RWMATH_VPU_DETAIL_VecRlimi(vec_dest, vec_src, /*writeMask*/ (1 << (3-vec_dest_index_immediate)), /*rotateLeft*/ ((4 + vec_src_index_immediate - vec_dest_index_immediate) % 4) ); \
    } \


#define RW_MATH_VPU_SwizzleStore_I_V(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index) \
    { \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(vec_dest, vec_src, RWMATH_VPU_DETAIL_GetSwizzleControlV(vec_dest_index_immediate, vec_src_index)); \
    } \


#define RW_MATH_VPU_SwizzleStore_V_I(ret, vec_dest, vec_dest_index, vec_src, vec_src_index_immediate) \
    { \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(vec_dest, vec_src, RWMATH_VPU_DETAIL_GetSwizzleControlV(vec_dest_index, vec_src_index_immediate)); \
    } \


#define RW_MATH_VPU_SwizzleYZWX(ret, in_vec) \
    ret = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(in_vec, in_vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::W0, ::rw::math::vpu::detail::X0) )); \


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

// Note: It may be possible to check if in_index0_immediate is 0 and then
// simply return in_vec0.  However, this will require some testing to make sure
// the compiler optimizes away the test.
#define RW_MATH_VPU_CreateVectorAxisA1_I(ret, in_vec0, in_index0_immediate) \
    ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( in_vec0, (in_index0_immediate) );


#define RW_MATH_VPU_CreateVectorAxisA2_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate) \
    { \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec0, in_vec1, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI((::rw::math::vpu::detail::VectorPermSource)in_index0_immediate, (::rw::math::vpu::detail::VectorPermSource)((int)(in_index1_immediate)+4), ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
    } \


#define RW_MATH_VPU_CreateVectorAxisA3_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate) \
    { \
        /* result will have the first two elements in the correct position. */ \
        ::rw::math::vpu::VectorIntrinsic perm_result = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec0, in_vec1, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI((::rw::math::vpu::detail::VectorPermSource)in_index0_immediate, (::rw::math::vpu::detail::VectorPermSource)((int)in_index1_immediate+4), ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
        /* Then store the final result in the z position */ \
        RW_MATH_VPU_SwizzleStore_I_I( ret, perm_result, VectorAxisZ, in_vec2, in_index2_immediate ); \
    } \


#define RW_MATH_VPU_CreateVectorAxisA4_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate, in_vec3, in_index3_immediate) \
    { \
        /* Permute the first two values (i.e. x,y) into the ZW position of a temporary vector. */ \
        ::rw::math::vpu::VectorIntrinsic create_vector_axis_temp0 = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec0, in_vec1, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0, (::rw::math::vpu::detail::VectorPermSource)in_index0_immediate, (::rw::math::vpu::detail::VectorPermSource)(in_index1_immediate+4)) ); \
        /* Permute the last two values (i.e. z,w) into the XY position of a temporary vector. */ \
        ::rw::math::vpu::VectorIntrinsic create_vector_axis_temp1 = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec2, in_vec3, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI((::rw::math::vpu::detail::VectorPermSource)in_index2_immediate, (::rw::math::vpu::detail::VectorPermSource)(in_index3_immediate+4), ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
        /* Now combine the two temporary values into a single vector using RWMATH_VPU_DETAIL_VecSld (better than a permute). */ \
        ret = RWMATH_VPU_DETAIL_VecSld( create_vector_axis_temp0, create_vector_axis_temp1, sizeof(float)*2 ); \
    } \


#define RW_MATH_VPU_GetZero(ret) \
    ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplatS32(0); \

#define RW_MATH_VPU_GetZero_R() \
    ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplatS32(0))

#define RW_MATH_VPU_GetHalf(ret) \
    ret = RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 1); \

#define RW_MATH_VPU_GetHalf_R() \
    (RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 1))

#define RW_MATH_VPU_GetNegHalf_R() \
    (RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(-1), 1))

#define RW_MATH_VPU_GetOne(ret) \
    ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 0); \

#define RW_MATH_VPU_GetOne_R() \
    ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 0))

#define RW_MATH_VPU_GetNegOne(ret) \
    ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(-1), 0); \

#define RW_MATH_VPU_GetNegOne_R() \
    ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(-1), 0))

#define RW_MATH_VPU_GetTwo(ret) \
    ret = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(2), 0)); \

#define RW_MATH_VPU_GetTwo_R() \
    ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(2), 0))

#define RW_MATH_VPU_GetNegTwo_R() \
    ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(-2), 0))

#define RW_MATH_VPU_GetTrue(ret) \
    ret = RWMATH_VPU_DETAIL_VecSplatS32(-1)

#define RW_MATH_VPU_GetTrue_R() \
    RWMATH_VPU_DETAIL_VecSplatS32(-1)

#define RW_MATH_VPU_CompAnyTrueV3( ret, a ) \
    { \
        ::rw::math::vpu::VectorIntrinsic anytrueGatheredBytes = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm((a), (a), ::rw::math::vpu::detail::gGatherMostSignificantBytesV3); \
        ret = RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord(anytrueGatheredBytes, RW_MATH_VPU_GetZero_R() ); \
        RW_MATH_VPU_Not( ret, ret ); \
    }

#define RW_MATH_VPU_CompAllTrueV3( ret, a ) \
    { \
        ::rw::math::vpu::VectorIntrinsic anytrueGatheredBytes = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm((a), (a), ::rw::math::vpu::detail::gGatherMostSignificantBytesV3); \
        ret = RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord(anytrueGatheredBytes, RW_MATH_VPU_GetTrue_R() ); \
    }

#define RW_MATH_VPU_CompAnyTrueV4( ret, a ) \
    { \
        ::rw::math::vpu::VectorIntrinsic anytrueGatheredBytes = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm((a), (a), ::rw::math::vpu::detail::gGatherMostSignificantBytesV4); \
        ret = RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord(anytrueGatheredBytes, RW_MATH_VPU_GetZero_R() ); \
        RW_MATH_VPU_Not( ret, ret ); \
    }

#define RW_MATH_VPU_CompAllTrueV4( ret, a ) \
    { \
        ::rw::math::vpu::VectorIntrinsic anytrueGatheredBytes = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm((a), (a), ::rw::math::vpu::detail::gGatherMostSignificantBytesV4); \
        ret = RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord(anytrueGatheredBytes, RW_MATH_VPU_GetTrue_R() ); \
    }

//----------------------------------------------------------------------------
// RW_MATH_VPU_GetPI_R()
//----------------------------------------------------------------------------
// 3.1415927f --> 40490FDB
#define RW_MATH_VPU_GetPI(ret) \
    { \
        ret = ::rw::math::vpu::detail::gPI; \
    } \

#define RW_MATH_VPU_GetPI_R() \
    (::rw::math::vpu::detail::gPI)

#define RW_MATH_VPU_Get2PI(ret) \
    ret = ::rw::math::vpu::detail::g2PI;

#define RW_MATH_VPU_Get2PIReciprocal(ret) \
    ret = ::rw::math::vpu::detail::g2PIReciprocal;

#define RW_MATH_VPU_GetI(ret) \
    { \
        ret = ::rw::math::vpu::detail::gIVector; \
    } \

#define RW_MATH_VPU_GetI_R() \
    (::rw::math::vpu::detail::gIVector)

#define RW_MATH_VPU_GetJ(ret) \
    { \
        ret = ::rw::math::vpu::detail::gJVector; \
    } \

#define RW_MATH_VPU_GetJ_R() \
    (::rw::math::vpu::detail::gJVector)

#define RW_MATH_VPU_GetK(ret) \
    { \
        ret = ::rw::math::vpu::detail::gKVector; \
    } \

#define RW_MATH_VPU_GetK_R() \
    (::rw::math::vpu::detail::gKVector)

#define RW_MATH_VPU_GetL(ret) \
    { \
        ret = ::rw::math::vpu::detail::gLVector; \
    } \

#define RW_MATH_VPU_GetL_R() \
    (::rw::math::vpu::detail::gLVector)

#define RW_MATH_VPU_GetSignMaskXYZW(ret) \
    ret = RWMATH_VPU_DETAIL_VecSplatS32(static_cast<signed char>(-1)); \
    ret = RWMATH_VPU_DETAIL_VecSlw(ret, ret); \


#define RW_MATH_VPU_GetSignMaskXYZ(ret) \
    ret = RWMATH_VPU_DETAIL_VecSplatS32(static_cast<signed char>(-1)); \
    ret = RWMATH_VPU_DETAIL_VecSlw(ret, ret); \
    RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisW, RWMATH_VPU_DETAIL_VecSplatS32(0), VectorAxisX); \


#define RW_MATH_VPU_GetSignMaskXZ(ret) \
    { \
        ::rw::math::vpu::VectorIntrinsic _asmGetSignMaskXZ_zero = RWMATH_VPU_DETAIL_VecSplatS32(0); \
        ret = RWMATH_VPU_DETAIL_VecSplatS32(static_cast<signed char>(-1)); \
        ret = RWMATH_VPU_DETAIL_VecSlw(ret, ret); \
        RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisY, _asmGetSignMaskXZ_zero, VectorAxisY); \
        RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisW, _asmGetSignMaskXZ_zero, VectorAxisW); \
    } \


#define RW_MATH_VPU_GetSignMaskYW(ret) \
    { \
        ::rw::math::vpu::VectorIntrinsic _asmGetSignMaskYW_zero = RWMATH_VPU_DETAIL_VecSplatS32(0); \
        ret = RWMATH_VPU_DETAIL_VecSplatS32(static_cast<signed char>(-1)); \
        ret = RWMATH_VPU_DETAIL_VecSlw(ret, ret); \
        RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisX, _asmGetSignMaskYW_zero, VectorAxisX); \
        RW_MATH_VPU_SwizzleStore_I_I(ret, ret, VectorAxisZ, _asmGetSignMaskYW_zero, VectorAxisZ); \
    } \


// ret - float
// in_vec - VectorIntrinsic
// in_index_immediate - integer value (immediate value)
#define RW_MATH_VPU_GetFloatAxis_I(ret, in_vec, in_index_immediate) \
    ret = in_vec.v[in_index_immediate]

#define RW_MATH_VPU_Floor(ret, a1) \
    ret = __vrfim((a1))

#define RW_MATH_VPU_Ceil(ret, a1) \
    ret = __vrfip((a1))

// Rounds the value towards zero. i.e. truncates the value.
#define RW_MATH_VPU_RoundToZero(ret, a1) \
    ret = __vrfiz((a1))

#define RW_MATH_VPU_Mult(ret, in_vec0, in_vec1) \
    ret = RWMATH_VPU_DETAIL_VecMul( in_vec0, in_vec1 ); \


#define RW_MATH_VPU_MultFloat(ret, in_vec, in_float) \
    { \
        ::rw::math::vpu::VectorIntrinsic floatVec; \
        RW_MATH_VPU_CreateVectorV1( floatVec, in_float ); \
        RW_MATH_VPU_Mult( ret, in_vec, floatVec ); \
    } \


#define RW_MATH_VPU_Negate(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic signMask; \
        RW_MATH_VPU_GetSignMaskXYZW(signMask); \
        ret = RW_MATH_VPU_Xor_R(in_vec, signMask); \
    }


#define RW_MATH_VPU_NegateXZ(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic _asmNegateXZ_mask; \
        RW_MATH_VPU_GetSignMaskXZ(_asmNegateXZ_mask); \
        RW_MATH_VPU_Xor(ret, in_vec, _asmNegateXZ_mask); \
    } \


#define RW_MATH_VPU_NegateYW(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic _asmNegateYW_mask; \
        RW_MATH_VPU_GetSignMaskYW(_asmNegateYW_mask); \
        RW_MATH_VPU_Xor(ret, in_vec, _asmNegateYW_mask); \
    } \


#define RW_MATH_VPU_ProductDifference(ret, v0, v1, v2, v3) \
    ret = RWMATH_VPU_DETAIL_VecNmsub(v2, v3, RWMATH_VPU_DETAIL_VecMadd(v0, v1, RWMATH_VPU_DETAIL_VecSplatS32(0))); \


#define RW_MATH_VPU_ReciprocalEstimate(ret, in_vec) \
    ret = RWMATH_VPU_DETAIL_VecRe( in_vec );


/*
   Newton-Raphson method for finding the reciprocal of a number
   
   \description
   Suppose we want to find the reciprocal of a value (i.e. we wish to calculate
   1/v).  To use the Newton-Raphson method we need to find a function f(x) for
   which f(x)=0 when x = 1/v.  To start, let x=1/v since we want to find 1/v.
   Now, lets manipulate the equation to remove the 1/v (since we want to calculate
   1/v) and make the equation equal to 0.
   
    \code
   x = 1/v                  (multiply by v)
   x^(-1) = v
   0 = v - x^(-1)
   
   So, we can let f(x) = v - x^(-1)
    \endcode
   
   The Newton-Raphson method defines the following recurrence relation that lets
   us calculate progressively better estimates.
   
    \code
   x_(i+1) = x_i - f(x_i)/f'(x_i)
   
   For our defined function f(x), f'(x) = x^(-2)
   
   Now, by substitution:
   
   x_(i+1) = x_i - ( (v-x_i^(-1)) / x_i^(-2) ) 
           = x_i - ( (v-x_i^(-1)) * x_i^(2) ) 
           = x_i - (v * x_i^(2) - x_i) 
           = 2 * x_i - v * x_i^(2)
    \endcode
   
    \param v Input Vector
    \result The resulting Vector
 */
#define RW_MATH_VPU_ReciprocalFast(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic estimate, vOne; \
        RW_MATH_VPU_ReciprocalEstimate( estimate,  in_vec ); \
        RW_MATH_VPU_GetOne( vOne ) ; \
        ret = RWMATH_VPU_DETAIL_VecMadd(estimate, RWMATH_VPU_DETAIL_VecNmsub(estimate, in_vec, vOne), estimate); \
    } \


#define RW_MATH_VPU_Reciprocal(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic secondEstimate; \
        /* Do two iteration of Newton-Raphson estimation */ \
        RW_MATH_VPU_ReciprocalFast( secondEstimate,  in_vec ); \
        ::rw::math::vpu::VectorIntrinsic vOne; \
        RW_MATH_VPU_GetOne( vOne ) ; \
        ret = RWMATH_VPU_DETAIL_VecMadd(secondEstimate, RWMATH_VPU_DETAIL_VecNmsub(secondEstimate, in_vec, vOne), secondEstimate); \
    } \


#define RW_MATH_VPU_ReciprocalSquareRootEstimate(ret, in_vec) \
    ret = RWMATH_VPU_DETAIL_VecSqrtRe(in_vec);

#define RW_MATH_VPU_RotateLeft_V_I(ret, in_vec, rotateAmount) \
    ret = RWMATH_VPU_DETAIL_VecSld(in_vec, in_vec, sizeof(float)*rotateAmount);

#define RW_MATH_VPU_ReciprocalSquareRootFast(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic estimate; \
        /* Do one iteration of Newton-Raphson estimation */ \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( estimate, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic vOne, vHalf; \
        RW_MATH_VPU_GetOne( vOne ); \
        RW_MATH_VPU_GetHalf( vHalf ); \
        ret = RWMATH_VPU_DETAIL_VecMadd( \
                                          RWMATH_VPU_DETAIL_VecMul( estimate, vHalf ), \
                                          RWMATH_VPU_DETAIL_VecNmsub( in_vec, RWMATH_VPU_DETAIL_VecMul(estimate,estimate), vOne), \
                                           estimate \
                                          ); \
    } \


#define RW_MATH_VPU_ReciprocalSquareRoot(ret, in_vec) \
    { \
        /* Do two iterations of Newton-Raphson estimation */ \
        ::rw::math::vpu::VectorIntrinsic secondEstimate; \
        RW_MATH_VPU_ReciprocalSquareRootFast( secondEstimate, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic vOne, vHalf; \
        RW_MATH_VPU_GetOne( vOne ); \
        RW_MATH_VPU_GetHalf( vHalf ); \
        ret = RWMATH_VPU_DETAIL_VecMadd( \
                                          RWMATH_VPU_DETAIL_VecMul( secondEstimate, vHalf ), \
                                          RWMATH_VPU_DETAIL_VecNmsub( in_vec, RWMATH_VPU_DETAIL_VecMul(secondEstimate,secondEstimate), vOne), \
                                           secondEstimate \
                                          ); \
    } \


#define RW_MATH_VPU_Divide(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        RW_MATH_VPU_Reciprocal( reciprocal, in_vec1 ); \
        ret = RWMATH_VPU_DETAIL_VecMul( reciprocal, in_vec0 ); \
    } \


#define RW_MATH_VPU_DivideFloat(ret, in_vec, in_float) \
    { \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        float floatRecip = 1.0f/(in_float); \
        RW_MATH_VPU_CreateVectorV1( reciprocal, floatRecip ); \
        ret = RWMATH_VPU_DETAIL_VecMul( reciprocal, in_vec ); \
    } \


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
    ret = RWMATH_VPU_DETAIL_VecMadd( in_vec0, in_vec1, in_vec2 );


#define RW_MATH_VPU_Add(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Add_R( in_vec0, in_vec1 )

#define RW_MATH_VPU_Add_R(in_vec0, in_vec1) \
    __vaddfp( in_vec0, in_vec1 )

#define RW_MATH_VPU_Subtract(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Subtract_R( in_vec0, in_vec1 );

#define RW_MATH_VPU_Subtract_R(in_vec0, in_vec1) \
    __vsubfp( in_vec0, in_vec1 )

#define RW_MATH_VPU_MultSubNeg(ret, in_vec0, in_vec1, in_vec2) \
    ret = RWMATH_VPU_DETAIL_VecNmsub(in_vec0, in_vec1, in_vec2)

#define RW_MATH_VPU_And(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_And_R(in_vec0, in_vec1); 

#define RW_MATH_VPU_Or(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Or_R(in_vec0, in_vec1); 

#define RW_MATH_VPU_Not(ret, in_vec) \
    ret = RW_MATH_VPU_Not_R(in_vec, in_vec);

#define RW_MATH_VPU_Xor(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Xor_R(in_vec0, in_vec1)

//----------------------------------------------------------------------------
// _asmIsEqualV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsEqualV2(ret, in_vec0, in_vec1) \
    ::rw::math::vpu::VectorIntrinsic v0comp = RWMATH_VPU_DETAIL_DuplicateXYtoZW_R(in_vec0); \
    ::rw::math::vpu::VectorIntrinsic v1comp = RWMATH_VPU_DETAIL_DuplicateXYtoZW_R(in_vec1); \
    ret = RWMATH_VPU_DETAIL_VecAllEq( v0comp, v1comp );


#define RW_MATH_VPU_IsEqualV3(ret, in_vec0, in_vec1) \
    ::rw::math::vpu::VectorIntrinsic v0comp = RWMATH_VPU_DETAIL_DuplicateXtoW_R(in_vec0); \
    ::rw::math::vpu::VectorIntrinsic v1comp = RWMATH_VPU_DETAIL_DuplicateXtoW_R(in_vec1); \
    ret = RWMATH_VPU_DETAIL_VecAllEq( v0comp, v1comp );


#define RW_MATH_VPU_IsEqualV4(ret, in_vec0, in_vec1) \
    ret = RWMATH_VPU_DETAIL_VecAllEq( in_vec0, in_vec1 );


#define RW_MATH_VPU_ProductSum3(ret, v0, v1, v2, v3, v4, v5) \
    RW_MATH_VPU_Mult   (ret, v0, v1); \
    RW_MATH_VPU_MultAdd(ret, v2, v3, ret); \
    RW_MATH_VPU_MultAdd(ret, v4, v5, ret); \


#define RW_MATH_VPU_Abs(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic mask = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplatS32(static_cast<signed char>(-1)); \
        const ::rw::math::vpu::VectorIntrinsic signBitMask = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSlw( mask, mask ); \
        ret = (::rw::math::vpu::VectorIntrinsic)RW_MATH_VPU_And_Rc( in_vec, signBitMask ); \
    } \



//----------------------------------------------------------------------------
// _asmIsZeroV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsZeroA1V2(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue; \
        RW_MATH_VPU_Abs( comparisonValue, in_vec ); \
        comparisonValue = RWMATH_VPU_DETAIL_DuplicateXYtoZW_R(comparisonValue); \
        ret = ( !RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue, in_tolerance ) ); \
    } \


#define RW_MATH_VPU_IsZeroA1V3(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue; \
        RW_MATH_VPU_Abs( comparisonValue, in_vec ); \
        comparisonValue = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue); \
        ret = ( !RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue, in_tolerance ) ); \
    } \


#define RW_MATH_VPU_IsZeroA1V4(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue; \
        RW_MATH_VPU_Abs( comparisonValue, in_vec ); \
        ret = ( !RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue, in_tolerance ) ); \
    } \


#define RW_MATH_VPU_IsZeroA2V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue0, comparisonValue1; \
        RW_MATH_VPU_Abs( comparisonValue0, in_vec0 ); \
        RW_MATH_VPU_Abs( comparisonValue1, in_vec1 ); \
        int any_greater_than =  RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue0, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue1, in_tolerance ); \
        ret = (!any_greater_than); \
    } \


#define RW_MATH_VPU_IsZeroA3V3(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue0, comparisonValue1, comparisonValue2; \
        RW_MATH_VPU_Abs( comparisonValue0, in_vec0 ); \
        RW_MATH_VPU_Abs( comparisonValue1, in_vec1 ); \
        RW_MATH_VPU_Abs( comparisonValue2, in_vec2 ); \
        comparisonValue0 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue0); \
        comparisonValue1 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue1); \
        comparisonValue2 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue2); \
        int any_greater_than =  RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue0, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue1, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue2, in_tolerance ); \
        ret = (!any_greater_than); \
    } \


#define RW_MATH_VPU_IsZeroA3V4(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue0, comparisonValue1, comparisonValue2; \
        RW_MATH_VPU_Abs( comparisonValue0, in_vec0 ); \
        RW_MATH_VPU_Abs( comparisonValue1, in_vec1 ); \
        RW_MATH_VPU_Abs( comparisonValue2, in_vec2 ); \
        int any_greater_than =  RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue0, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue1, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue2, in_tolerance ); \
        ret = (!any_greater_than); \
    } \


#define RW_MATH_VPU_IsZeroA4V3(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue0, comparisonValue1, comparisonValue2, comparisonValue3; \
        RW_MATH_VPU_Abs( comparisonValue0, in_vec0 ); \
        RW_MATH_VPU_Abs( comparisonValue1, in_vec1 ); \
        RW_MATH_VPU_Abs( comparisonValue2, in_vec2 ); \
        RW_MATH_VPU_Abs( comparisonValue3, in_vec3 ); \
        comparisonValue0 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue0); \
        comparisonValue1 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue1); \
        comparisonValue2 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue2); \
        comparisonValue3 = RWMATH_VPU_DETAIL_DuplicateXtoW_R(comparisonValue3); \
        int any_greater_than =  RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue0, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue1, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue2, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue3, in_tolerance ); \
        ret = (!any_greater_than); \
    } \


#define RW_MATH_VPU_IsZeroA4V4(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic comparisonValue0, comparisonValue1, comparisonValue2, comparisonValue3; \
        RW_MATH_VPU_Abs( comparisonValue0, in_vec0 ); \
        RW_MATH_VPU_Abs( comparisonValue1, in_vec1 ); \
        RW_MATH_VPU_Abs( comparisonValue2, in_vec2 ); \
        RW_MATH_VPU_Abs( comparisonValue3, in_vec3 ); \
        int any_greater_than =  RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue0, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue1, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue2, in_tolerance ) | \
                                RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue3, in_tolerance ); \
        ret = (!any_greater_than); \
    } \


//----------------------------------------------------------------------------
// _asmIsSimilarV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsSimilarA1V2(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference = RW_MATH_VPU_Subtract_R( in_vec0, in_vec1 ); \
        RW_MATH_VPU_IsZeroA1V2( ret, difference, in_tolerance ); \
    } \


#define RW_MATH_VPU_IsSimilarA1V3(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference = RW_MATH_VPU_Subtract_R( in_vec0, in_vec1 ); \
        RW_MATH_VPU_IsZeroA1V3( ret, difference, in_tolerance ); \
    } \


#define RW_MATH_VPU_IsSimilarA1V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference = RW_MATH_VPU_Subtract_R( in_vec0, in_vec1 ); \
        RW_MATH_VPU_IsZeroA1V4( ret, difference, in_tolerance ); \
    } \


#define RW_MATH_VPU_IsSimilarA3V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference0 = RW_MATH_VPU_Subtract_R( in_vec0_x, in_vec1_x ); \
        ::rw::math::vpu::VectorIntrinsic difference1 = RW_MATH_VPU_Subtract_R( in_vec0_y, in_vec1_y ); \
        ::rw::math::vpu::VectorIntrinsic difference2 = RW_MATH_VPU_Subtract_R( in_vec0_z, in_vec1_z ); \
        RW_MATH_VPU_IsZeroA3V3( ret, difference0, difference1, difference2, in_tolerance ); \
    } \


#define RW_MATH_VPU_IsSimilarA4V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference0 = RW_MATH_VPU_Subtract_R( in_vec0_x, in_vec1_x ); \
        ::rw::math::vpu::VectorIntrinsic difference1 = RW_MATH_VPU_Subtract_R( in_vec0_y, in_vec1_y ); \
        ::rw::math::vpu::VectorIntrinsic difference2 = RW_MATH_VPU_Subtract_R( in_vec0_z, in_vec1_z ); \
        ::rw::math::vpu::VectorIntrinsic difference3 = RW_MATH_VPU_Subtract_R( in_vec0_w, in_vec1_w ); \
        RW_MATH_VPU_IsZeroA4V3( ret, difference0, difference1, difference2, difference3, in_tolerance ); \
    } \


#define RW_MATH_VPU_IsSimilarA3V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference0 = RW_MATH_VPU_Subtract_R( in_vec0_x, in_vec1_x ); \
        ::rw::math::vpu::VectorIntrinsic difference1 = RW_MATH_VPU_Subtract_R( in_vec0_y, in_vec1_y ); \
        ::rw::math::vpu::VectorIntrinsic difference2 = RW_MATH_VPU_Subtract_R( in_vec0_z, in_vec1_z ); \
        RW_MATH_VPU_IsZeroA4V3( ret, difference0, difference1, difference2, in_tolerance ); \
    } \


#define RW_MATH_VPU_IsSimilarA4V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic difference0 = RW_MATH_VPU_Subtract_R( in_vec0_x, in_vec1_x ); \
        ::rw::math::vpu::VectorIntrinsic difference1 = RW_MATH_VPU_Subtract_R( in_vec0_y, in_vec1_y ); \
        ::rw::math::vpu::VectorIntrinsic difference2 = RW_MATH_VPU_Subtract_R( in_vec0_z, in_vec1_z ); \
        ::rw::math::vpu::VectorIntrinsic difference3 = RW_MATH_VPU_Subtract_R( in_vec0_w, in_vec1_w ); \
        RW_MATH_VPU_IsZeroA4V4( ret, difference0, difference1, difference2, difference3, in_tolerance ); \
    } \


//----------------------------------------------------------------------------
// _asmCross()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_CrossV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic crossV2  = (::rw::math::vpu::VectorIntrinsic)__vpermwi(in_vec1, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(VectorAxisY, VectorAxisZ, VectorAxisX, VectorAxisW)); \
        ::rw::math::vpu::VectorIntrinsic crossV1  = (::rw::math::vpu::VectorIntrinsic)__vpermwi(in_vec0, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(VectorAxisY, VectorAxisZ, VectorAxisX, VectorAxisW)); \
        ::rw::math::vpu::VectorIntrinsic crossV3  = RWMATH_VPU_DETAIL_VecMul(in_vec0, crossV2); \
        crossV3 = RWMATH_VPU_DETAIL_VecNmsub(crossV1, in_vec1, crossV3); \
        ret = (::rw::math::vpu::VectorIntrinsic)__vpermwi(crossV3, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(VectorAxisY, VectorAxisZ, VectorAxisX, VectorAxisW)); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_DotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic product = RWMATH_VPU_DETAIL_VecMul(in_vec0, in_vec1); \
        ::rw::math::vpu::VectorIntrinsic x_product = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat(product, 0); \
        ::rw::math::vpu::VectorIntrinsic y_product = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat(product, 1); \
        ret = RW_MATH_VPU_Add_R( x_product, y_product ); \
    } \


#define RW_MATH_VPU_DotV3(ret, in_vec0, in_vec1) \
    { \
        ret = __vmsum3fp(in_vec0, in_vec1); \
    } \


#define RW_MATH_VPU_DotV4(ret, in_vec0, in_vec1) \
    { \
        ret = __vmsum4fp(in_vec0, in_vec1); \
    } \


// Note: It is difficult to directly compute the squareroot of a value using VMX, so this function
// computes x * x^(-1/2).  Of course this will give a mathematically undefined result for x = 0, so
// the code has a comparison that returns 0 if x = 0.
#define RW_MATH_VPU_SquareRoot(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic sq_zero = RWMATH_VPU_DETAIL_VecZero(); \
        const ::rw::math::vpu::VectorIntrinsic isZeroMask = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecCmpEq( sq_zero, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic sq_result = RWMATH_VPU_DETAIL_VecMul(in_vec, recipSqrt ); \
        ret = RW_MATH_VPU_Select_R( sq_result, sq_zero, isZeroMask ); \
    }

#define RW_MATH_VPU_SquareRootFast(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic sq_zero = RW_MATH_VPU_GetZero_R(); \
        ::rw::math::vpu::VectorIntrinsic isZeroMask; \
        RW_MATH_VPU_IsEqual( isZeroMask, sq_zero, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRootFast( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic sq_result; \
        RW_MATH_VPU_Mult( sq_result, in_vec, recipSqrt ); \
        ret = RW_MATH_VPU_Select_R( sq_result, sq_zero, isZeroMask ); \
    }

#define RW_MATH_VPU_SquareRootEstimate(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic sq_zero = RW_MATH_VPU_GetZero_R(); \
        ::rw::math::vpu::VectorIntrinsic isZeroMask; \
        RW_MATH_VPU_IsEqual( isZeroMask, sq_zero, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic sq_result; \
        RW_MATH_VPU_Mult( sq_result, in_vec, recipSqrt ); \
        ret = RW_MATH_VPU_Select_R( sq_result, sq_zero, isZeroMask ); \
    }


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
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV2( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRoot( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootDotFastV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootFast( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRootFast( ret, dotproduct ); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV3( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRoot( ret, dotproduct ); \
    } \


#define RW_MATH_VPU_ReciprocalSquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic dotproduct; \
        RW_MATH_VPU_DotV4( dotproduct, in_vec0, in_vec1 ); \
        RW_MATH_VPU_ReciprocalSquareRoot( ret, dotproduct ); \
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
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV2(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeEstimateV3(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeFastV3(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV3(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
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

#define RW_MATH_VPU_NormalizeV3(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV3(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \


#define RW_MATH_VPU_NormalizeV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_normalizev4_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV4(temp_normalizev4_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_normalizev4_one_over_rqrt_dot); \
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
        ::rw::math::vpu::VectorIntrinsic temp_dot; \
        RW_MATH_VPU_DotV2(temp_dot, in_vec, in_vec); \
        ::rw::math::vpu::VectorIntrinsic one_over_rqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot(one_over_rqrt, temp_dot); \
        ::rw::math::vpu::VectorIntrinsic result_mag; \
        RW_MATH_VPU_Mult(result_mag, temp_dot, one_over_rqrt); \
        RW_MATH_VPU_Mult(ret, in_vec, one_over_rqrt); \
        ::rw::math::vpu::VectorIntrinsic n_zero; \
        RW_MATH_VPU_GetZero(n_zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, n_zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, n_zero, input_is_zero); \
    } \


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
        ::rw::math::vpu::VectorIntrinsic n_zero; \
        RW_MATH_VPU_GetZero(n_zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, n_zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, n_zero, input_is_zero); \
    } \


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
        ::rw::math::vpu::VectorIntrinsic n_zero; \
        RW_MATH_VPU_GetZero(n_zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, n_zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, n_zero, input_is_zero); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Lerp(ret, in_vec0, in_vec1, in_t) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_lerp_v; \
        RW_MATH_VPU_Subtract(temp_lerp_v, in_vec1, in_vec0); \
        RW_MATH_VPU_MultAdd(ret, temp_lerp_v, in_t, in_vec0); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Splat_I(ret, in_vec0, in_index_immediate) \
    { \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( in_vec0, (int)(in_index_immediate) ); \
    } \


#define RW_MATH_VPU_Splat(ret, in_vec0, in_index) \
    { \
        ::rw::math::vpu::VectorIntrinsic permConstant = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( RWMATH_VPU_DETAIL_VecLvsl( 4*(int)(in_index), NULL ), 0 ); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(in_vec0, in_vec0, permConstant); \
    } \

#define RW_MATH_VPU_TransformVectorV3(ret, in_vec, in_x, in_y, in_z) \
    { \
        ::rw::math::vpu::VectorIntrinsic xSplat, ySplat, zSplat; \
        RW_MATH_VPU_Splat_I( xSplat, in_vec, 0 ); \
        RW_MATH_VPU_Splat_I( ySplat, in_vec, 1 ); \
        RW_MATH_VPU_Splat_I( zSplat, in_vec, 2 ); \
           RW_MATH_VPU_Mult( ret, in_x, xSplat ); \
        RW_MATH_VPU_MultAdd( ret, in_y, ySplat, ret ); \
        RW_MATH_VPU_MultAdd( ret, in_z, zSplat, ret ); \
    } \

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

//----------------------------------------------------------------------------
#define RW_MATH_VPU_TransposeA3V3(ret_x, ret_y, ret_z, in_x, in_y, in_z) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_y/*dummy arg*/); \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_y/*dummy arg*/); \
        ret_x = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp0, temp1); \
        ret_y = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp0, temp1); \
        ret_z = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp2, temp3); \
    } \


#define RW_MATH_VPU_TransposeA4V4(ret_x, ret_y, ret_z, ret_w, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp0 = RWMATH_VPU_DETAIL_VecMergeH(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp1 = RWMATH_VPU_DETAIL_VecMergeH(in_y, in_w); \
        ::rw::math::vpu::VectorIntrinsic temp2 = RWMATH_VPU_DETAIL_VecMergeL(in_x, in_z); \
        ::rw::math::vpu::VectorIntrinsic temp3 = RWMATH_VPU_DETAIL_VecMergeL(in_y, in_w); \
        ret_x = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp0, temp1); \
        ret_y = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp0, temp1); \
        ret_z = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(temp2, temp3); \
        ret_w = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(temp2, temp3); \
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

//----------------------------------------------------------------------------
#define RW_MATH_VPU_MatrixFromQuaternion(ret_x, ret_y, ret_z, in_q) \
    { \
        ::rw::math::vpu::VectorIntrinsic sqrt2s; \
        RW_MATH_VPU_GetSqrt2s(sqrt2s); \
        ::rw::math::vpu::VectorIntrinsic scaled_q = RWMATH_VPU_DETAIL_VecMul(sqrt2s, in_q ); \
        ::rw::math::vpu::VectorIntrinsic halves; \
        RW_MATH_VPU_GetHalves(halves); \
        ::rw::math::vpu::VectorIntrinsic neg_qq = RWMATH_VPU_DETAIL_VecNmsub( scaled_q, scaled_q, halves ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_yzx = __vpermwi(scaled_q, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(VectorAxisY, VectorAxisZ, VectorAxisX, VectorAxisX)); \
        ::rw::math::vpu::VectorIntrinsic xy_yz_zx = RWMATH_VPU_DETAIL_VecMul( scaled_q, scaled_q_yzx ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_zxy = __vpermwi(scaled_q, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(VectorAxisZ, VectorAxisX, VectorAxisY, VectorAxisX)); \
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
        ::rw::math::vpu::VectorIntrinsic neg_yy_zz_xx = __vpermwi(neg_qq, RWMATH_VPU_DETAIL_CreatePermuteImmediateControl(VectorAxisY, VectorAxisZ, VectorAxisX, VectorAxisX)); \
        ::rw::math::vpu::VectorIntrinsic diagonalValues = RW_MATH_VPU_Add_R(neg_qq, neg_yy_zz_xx); \
        RW_MATH_VPU_CreateVectorAxisA3_I( ret_x, /*(neg_yy+neg_zz)*/diagonalValues, VectorAxisY, /*xy+wz*/             addVec,         VectorAxisX, /*(zx-wy)*/         subVec,            VectorAxisZ ); \
        RW_MATH_VPU_CreateVectorAxisA3_I( ret_y, /*(xy-wz)*/        subVec,         VectorAxisX, /*(neg_zz+neg_xx)*/   diagonalValues, VectorAxisZ, /*(yz+wx)*/         addVec,            VectorAxisY ); \
        RW_MATH_VPU_CreateVectorAxisA3_I( ret_z, /*(zx+wy)*/        addVec,         VectorAxisZ, /*(yz-wx)*/           subVec,         VectorAxisY, /*(neg_xx+neg_yy)*/ diagonalValues,    VectorAxisX ); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Min(ret, in_vec0, in_vec1) \
    ret = RWMATH_VPU_DETAIL_VecMin( in_vec0, in_vec1 );


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Max(ret, in_vec0, in_vec1) \
    ret = RWMATH_VPU_DETAIL_VecMax( in_vec0, in_vec1 );

#define RW_MATH_VPU_ClampToNegPosRange(ret, in_vec0, range) \
    { \
        VectorIntrinsic cl_zero, signMask; \
        RW_MATH_VPU_GetSignMaskXYZW(signMask); \
        RW_MATH_VPU_GetZero(cl_zero); \
        VectorIntrinsic isEqualMask = RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord(RWMATH_VPU_DETAIL_VecCmpB(in_vec0, range), cl_zero); \
        ret = RW_MATH_VPU_Select_R(RW_MATH_VPU_Select_R(range, in_vec0, signMask), in_vec0, isEqualMask ); \
    }


// Trig functions
#define RW_MATH_VPU_Cos(ret, in_vec) \
    ret = RWMATH_Cos(in_vec);

#define RW_MATH_VPU_Sin(ret, in_vec) \
    ret = RWMATH_Sin(in_vec);

#define RW_MATH_VPU_SinCos(ret0, ret1, in_vec) \
    RWMATH_SinCos( &ret0, &ret1, in_vec );

#define RW_MATH_VPU_Tan(ret, in_vec) \
    ret = RWMATH_Tan(in_vec);

#define RW_MATH_VPU_ACos(ret, in_vec) \
    ret = RWMATH_ACos(in_vec);

#define RW_MATH_VPU_ASin(ret, in_vec) \
    ret = RWMATH_ASin(in_vec);

#define RW_MATH_VPU_ATan(ret, in_vec) \
    ret = RWMATH_ATan(in_vec);

#define RW_MATH_VPU_ATan2(ret, in_vec0, in_vec1) \
    { \
        VectorIntrinsic zeroVec = RWMATH_VPU_DETAIL_VecZero(); \
        VectorIntrinsic signBit; \
        RW_MATH_VPU_GetSignMaskXYZW(signBit); \
        VectorIntrinsic half_pi = RW_MATH_VPU_CreateVectorFromScalar_R(HALF_PI); \
        VectorIntrinsic pi = RW_MATH_VPU_CreateVectorFromScalar_R(PI); \
        \
        VectorIntrinsic a_signBit = RW_MATH_VPU_And_R(in_vec0, signBit); \
        \
        VectorIntrinsic bNeg = RWMATH_VPU_DETAIL_VecCmpGT(zeroVec, in_vec1); \
        VectorIntrinsic bZero = RWMATH_VPU_DETAIL_VecCmpEq(zeroVec, in_vec1); \
        \
        VectorIntrinsic res1 = RW_MATH_VPU_Or_R(half_pi, a_signBit); /* sign(a) * HALF_PI */ \
        \
        VectorIntrinsic bRecip; \
        RW_MATH_VPU_ReciprocalFast( bRecip, in_vec1 ); \
        VectorIntrinsic res2 = RWMATH_ATan( RWMATH_VPU_DETAIL_VecMadd( in_vec0, bRecip, zeroVec ) ); /* atan(a/b); */ \
        \
        VectorIntrinsic b_signBit = RW_MATH_VPU_And_R(in_vec0, signBit); \
        \
        VectorIntrinsic newRes = RW_MATH_VPU_Add_R(RW_MATH_VPU_Or_R(pi, b_signBit), res2); /* sign(a) * PI + res2 */ \
        res2 = RW_MATH_VPU_Select_R(res2, newRes, bNeg); \
        \
        ret = RW_MATH_VPU_Select_R(res2, res1, bZero); \
    }

#define RW_MATH_VPU_IsAnyEqual_Int(ret, in_vec0, in_vec1)  \
    { \
        unsigned int __rw_math_vpu_isanyequal_int_result; \
        __vcmpequwR(in_vec0, in_vec1, &__rw_math_vpu_isanyequal_int_result); \
        ret = RWMATH_COMPARE_ANY_TRUE(__rw_math_vpu_isanyequal_int_result); \
    }

#define RW_MATH_VPU_IsAllEqual_Int(ret, in_vec0, in_vec1) \
    { \
        unsigned int __rw_math_vpu_isanyequal_int_result; \
        __vcmpequwR(in_vec0, in_vec1, &__rw_math_vpu_isanyequal_int_result); \
        ret = RWMATH_COMPARE_ALL_TRUE(__rw_math_vpu_isanyequal_int_result); \
    }

#define RW_MATH_VPU_IsAnyTrue(ret, in_vec) \
    { \
        VectorIntrinsic trueVec; \
        RW_MATH_VPU_GetTrue(trueVec); \
        RW_MATH_VPU_IsAnyEqual_Int(ret, trueVec, in_vec); \
    }

#define RW_MATH_VPU_IsAllTrue(ret, in_vec) \
    { \
        VectorIntrinsic trueVec; \
        RW_MATH_VPU_GetTrue(trueVec); \
        RW_MATH_VPU_IsAllEqual_Int(ret, trueVec, in_vec); \
    }

#define RW_MATH_VPU_IsAnyFalse(ret, in_vec) \
    { \
        VectorIntrinsic falseVec; \
        RW_MATH_VPU_GetZero(falseVec); \
        RW_MATH_VPU_IsAnyEqual_Int(ret, falseVec, in_vec); \
    }

#define RW_MATH_VPU_IsAllFalse(ret, in_vec) \
    { \
        VectorIntrinsic falseVec; \
        RW_MATH_VPU_GetZero(falseVec); \
        RW_MATH_VPU_IsAllEqual_Int(ret, falseVec, in_vec); \
    }

namespace detail
{

RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic macro_impl_asmCreateVector( float f0 ) 
{
    ::rw::math::vpu::VectorIntrinsic result;
    RW_MATH_VPU_CreateVectorV1( result, f0 );
    return result;
}

RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic macro_impl_asmCreateVectorV2( float f0, float f1 ) 
{
    ::rw::math::vpu::VectorIntrinsic result = { f0, f1 };
    return result;
}

RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic macro_impl_asmCreateVectorV3( float f0, float f1, float f2 ) 
{
    ::rw::math::vpu::VectorIntrinsic result = { f0, f1, f2 };
    return result;
}

RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic macro_impl_asmCreateVectorV4( float f0, float f1, float f2, float f3 ) 
{
    ::rw::math::vpu::VectorIntrinsic result = { f0, f1, f2, f3 };
    return result;
}

} // namespace detail

} // namespace vpu
} // namespace math
} // namespace rw

#endif
