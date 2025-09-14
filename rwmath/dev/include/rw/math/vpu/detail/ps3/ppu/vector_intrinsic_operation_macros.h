// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_MACRO_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_MACRO_INLINE_H

namespace rw
{

namespace math
{

namespace vpu
{

typedef vector unsigned char VectorPermuteControl;

namespace detail
{

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

    template<VectorPermSource xsrc, VectorPermSource ysrc, VectorPermSource zsrc, VectorPermSource wsrc>
    struct VectorPermuteConstant
    {
        static const vector unsigned char value;
    };

    template<VectorPermSource xsrc, VectorPermSource ysrc, VectorPermSource zsrc, VectorPermSource wsrc>
    const vector unsigned char VectorPermuteConstant<xsrc, ysrc, zsrc, wsrc>::value = 
    {
        ((xsrc * 4) + 0), 
        ((xsrc * 4) + 1),
        ((xsrc * 4) + 2),
        ((xsrc * 4) + 3),
        ((ysrc * 4) + 0), 
        ((ysrc * 4) + 1),
        ((ysrc * 4) + 2),
        ((ysrc * 4) + 3),
        ((zsrc * 4) + 0), 
        ((zsrc * 4) + 1),
        ((zsrc * 4) + 2),
        ((zsrc * 4) + 3),
        ((wsrc * 4) + 0), 
        ((wsrc * 4) + 1),
        ((wsrc * 4) + 2),
        ((wsrc * 4) + 3)
    };

}

#define RW_MATH_VPU_V2_RotateLeft( ret, vec) \
{ \
    ret = RWMATH_VPU_DETAIL_VecPerm(vec, vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( ::rw::math::vpu::detail::VectorPermSource(1),::rw::math::vpu::detail::VectorPermSource(0),::rw::math::vpu::detail::VectorPermSource(2),::rw::math::vpu::detail::VectorPermSource(3) )); \
} \

#define RW_MATH_VPU_V3_RotateLeft( ret, vec) \
{ \
    ret = RWMATH_VPU_DETAIL_VecPerm(vec, vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( ::rw::math::vpu::detail::VectorPermSource(1),::rw::math::vpu::detail::VectorPermSource(2),::rw::math::vpu::detail::VectorPermSource(0),::rw::math::vpu::detail::VectorPermSource(3) )); \
} \

#define RW_MATH_VPU_V3_RotateRight( ret, vec) \
{ \
    ret = RWMATH_VPU_DETAIL_VecPerm(vec, vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( ::rw::math::vpu::detail::VectorPermSource(2),::rw::math::vpu::detail::VectorPermSource(0),::rw::math::vpu::detail::VectorPermSource(1),::rw::math::vpu::detail::VectorPermSource(3) )); \
} \

#define RW_MATH_VPU_V4_RotateLeft( ret, vec) \
{ \
    ret = RWMATH_VPU_DETAIL_VecPerm(vec, vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( ::rw::math::vpu::detail::VectorPermSource(1),::rw::math::vpu::detail::VectorPermSource(2),::rw::math::vpu::detail::VectorPermSource(3),::rw::math::vpu::detail::VectorPermSource(0) )); \
} \

#define RW_MATH_VPU_V4_RotateRight( ret, vec) \
{ \
    ret = RWMATH_VPU_DETAIL_VecPerm(vec, vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( ::rw::math::vpu::detail::VectorPermSource(3),::rw::math::vpu::detail::VectorPermSource(0),::rw::math::vpu::detail::VectorPermSource(1),::rw::math::vpu::detail::VectorPermSource(2) )); \
} \

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

#define RW_MATH_VPU_LoadFromAlignedFloatPointer( address ) \
    RWMATH_VPU_DETAIL_VecLd( 0, address )

#if CELL_SDK_VERSION >= 0x084000
// Unaligned loads are not atomic operations
#define RW_MATH_VPU_UnalignedLoad(ret, address) \
    ret = RW_MATH_VPU_Or_R( RWMATH_VPU_DETAIL_VecLvlx( 0, (address) ), RWMATH_VPU_DETAIL_VecLvrx( 16, (address) ) )

// Unaligned stores are not atomic operations
#define RW_MATH_VPU_UnalignedStore(vec, address) \
    RWMATH_VPU_DETAIL_VecStvlx( vec, 0, (address) ); RWMATH_VPU_DETAIL_VecStvrx( vec, 16, (address) )

#define RW_MATH_VPU_UnalignedStore2(dest, vec) \
    RWMATH_VPU_DETAIL_VecStvewx(RWMATH_VPU_DETAIL_VecSplat(vec, 0), 0, &dest.x); \
    RWMATH_VPU_DETAIL_VecStvewx(RWMATH_VPU_DETAIL_VecSplat(vec, 1), 4, &dest.x);

#define RW_MATH_VPU_UnalignedStore3(dest, vec) \
    RWMATH_VPU_DETAIL_VecStvewx(RWMATH_VPU_DETAIL_VecSplat(vec, 0), 0, &dest.x); \
    RWMATH_VPU_DETAIL_VecStvewx(RWMATH_VPU_DETAIL_VecSplat(vec, 1), 4, &dest.x); \
    RWMATH_VPU_DETAIL_VecStvewx(RWMATH_VPU_DETAIL_VecSplat(vec, 2), 8, &dest.x);

#define RW_MATH_VPU_UnalignedStore4(dest, vec) \
    RWMATH_VPU_DETAIL_VecStvlx( vec, 0, (&dest.x) ); RWMATH_VPU_DETAIL_VecStvrx( vec, 16, (&dest.x) )

#else

// Unaligned loads are not atomic operations
#define RW_MATH_VPU_UnalignedLoad(ret, address) \
    { \
        vector unsigned char unalignedLoad_MSQ, unalignedLoad_LSQ; \
        vector unsigned char unalignedLoad_mask; \
        unalignedLoad_MSQ = (vector unsigned char)RWMATH_VPU_DETAIL_VecLd(0, (address)); \
        unalignedLoad_LSQ = (vector unsigned char)RWMATH_VPU_DETAIL_VecLd(15, (address)); \
        unalignedLoad_mask = (vector unsigned char)RWMATH_VPU_DETAIL_VecLvsl(0, (address)); \
        ret = (vector float)RWMATH_VPU_DETAIL_VecPerm(unalignedLoad_MSQ, unalignedLoad_LSQ, unalignedLoad_mask); \
    }

// Unaligned stores are not atomic operations
#define RW_MATH_VPU_UnalignedStore(vec, address) \
    { \
        vector unsigned char unalignedStore_MSQ, unalignedStore_LSQ, unalignedStore_edges; \
        vector unsigned char unalignedStore_edgeAlign, unalignedStore_align; \
        unalignedStore_MSQ = (vector unsigned char)RWMATH_VPU_DETAIL_VecLd( 0, (address) ); \
        unalignedStore_LSQ = (vector unsigned char)RWMATH_VPU_DETAIL_VecLd( 15, (address) ); \
        unalignedStore_edgeAlign = (vector unsigned char)RWMATH_VPU_DETAIL_VecLvsr( 0, (address) ); \
        unalignedStore_edges = (vector unsigned char)RWMATH_VPU_DETAIL_VecPerm( unalignedStore_LSQ, unalignedStore_MSQ, unalignedStore_edgeAlign ); \
        unalignedStore_align = (vector unsigned char)RWMATH_VPU_DETAIL_VecLvsr( 0, (address) ); \
        unalignedStore_MSQ = (vector unsigned char)RWMATH_VPU_DETAIL_VecPerm( unalignedStore_edges, vec, unalignedStore_align ); \
        unalignedStore_LSQ = (vector unsigned char)RWMATH_VPU_DETAIL_VecPerm( vec, unalignedStore_edges, (address) ); \
        RWMATH_VPU_DETAIL_VecSt( unalignedStore_LSQ, 15, (address) ); \
        RWMATH_VPU_DETAIL_VecSt( unalignedStore_MSQ, 0, (address) ); \
    }

#endif

//----------------------------------------------------------------------------
// Helper macros
//----------------------------------------------------------------------------
#define RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( X_INDEX, Y_INDEX, Z_INDEX, W_INDEX ) \
        (::rw::math::vpu::detail::VectorPermuteConstant<X_INDEX, Y_INDEX, Z_INDEX, W_INDEX>::value)

#define RW_MATH_VPU_PermuteSingle_I(ret, vec, pattern) \
        ret = RWMATH_VPU_DETAIL_VecPerm(vec, vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( ::rw::math::vpu::detail::VectorPermSource(((pattern) & 0xf000) >> 12), ::rw::math::vpu::detail::VectorPermSource(((pattern) & 0x0f00) >>  8), ::rw::math::vpu::detail::VectorPermSource(((pattern) & 0x00f0) >>  4), ::rw::math::vpu::detail::VectorPermSource(((pattern) & 0x000f) >>  0) ))

#define RWMATH_VPU_DETAIL_GetSwizzleControlV( DEST, SRC ) \
        ::rw::math::vpu::detail::gSwizzleStoreConstants[(((DEST) * 4 /* 4 = Number of choices for SRC */) + (SRC))]

#define RWMATH_VPU_DETAIL_GetSwizzleControlI( DEST, SRC ) \
        (::rw::math::vpu::detail::VectorPermuteConstant< \
            (DEST==VectorAxisX) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::X0, \
            (DEST==VectorAxisY) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::Y0, \
            (DEST==VectorAxisZ) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::Z0, \
            (DEST==VectorAxisW) ? (::rw::math::vpu::detail::VectorPermSource)(((int)SRC)+4) : ::rw::math::vpu::detail::W0  \
         >::value)

#define RWMATH_VPU_DETAIL_DuplicateXtoW_R(in_vec) \
        ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(in_vec, in_vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::X0) ))

#define RWMATH_VPU_DETAIL_DuplicateXYtoZW_R(in_vec) \
        ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(in_vec, in_vec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Y0) ))

#define RW_MATH_VPU_GetHalves(ret) \
    { \
        ret = RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(1), 1); \
    } \

#define RW_MATH_VPU_GetSqrt2s(ret) \
    { \
        ret = ::rw::math::vpu::detail::gSqrt2s; \
    } \

#define RW_MATH_VPU_Select(ret, a, b, c) \
    ret = RW_MATH_VPU_Select_R(a, b, c);

#define RW_MATH_VPU_IsGreater(ret, a, b) \
    ret = RWMATH_VPU_DETAIL_VecCmpGT(a, b);

#define RW_MATH_VPU_IsGreater_R(a, b) \
    RWMATH_VPU_DETAIL_VecCmpGT(a, b)

#define RW_MATH_VPU_IsGreaterEqual(ret, a, b) \
    ret = RW_MATH_VPU_IsGreaterEqual_R(a, b);

#define RW_MATH_VPU_IsEqual(ret, a, b) \
    ret = RWMATH_VPU_DETAIL_VecCmpEq(a, b);

#define RW_MATH_VPU_IsEqual_R(a, b) \
    RWMATH_VPU_DETAIL_VecCmpEq(a, b)

// The scalar comparison macros assume that all elements of the vector are identical.  The return result is either boolean or
// an integer that indicates true/false.
#define RW_MATH_VPU_Scalar_IsEqual(result, a, b) \
  result = RWMATH_VPU_DETAIL_VecAllEq(a, b)

#define RW_MATH_VPU_Scalar_IsGreater(result, a, b) \
  result = RWMATH_VPU_DETAIL_VecAllGT(a, b)

#define RW_MATH_VPU_Scalar_IsGreaterEqual(result, a, b) \
  result = RWMATH_VPU_DETAIL_VecAllGE(a, b)

#define RW_MATH_VPU_Scalar_IsLess(result, a, b) \
  result = RWMATH_VPU_DETAIL_VecAllGT(b, a)

#define RW_MATH_VPU_Scalar_IsLessEqual(result, a, b) \
  result = RWMATH_VPU_DETAIL_VecAllGE(b, a)

#if defined(__SNC__) && __SN_VER__ < 35001
// Older versions of SNC produced incorrect code for the standard implementation of this macro.
// This less efficient version is used as a workaround.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    RW_MATH_VPU_CreateVectorV1(ret, *(fPtr))

#elif CELL_SDK_VERSION >= 0x084000

// create a vector intrinsic from a pointer to a float.
// The float data will be splatted across the entire vector.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    { \
        ret = RWMATH_VPU_DETAIL_VecSplat(RWMATH_VPU_DETAIL_VecLvlx(0, (fPtr)), 0); \
    }

#else

// create a vector intrinsic from a pointer to a float.
// The float data will be splatted across the entire vector.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    { \
        VectorIntrinsic alignmentPermution = RWMATH_VPU_DETAIL_VecSplat(RWMATH_VPU_DETAIL_VecLvsl(0, (fPtr)), 0); \
        VectorIntrinsic unalignedData = RWMATH_VPU_DETAIL_VecLd(0, (fPtr)); \
        ret = RWMATH_VPU_DETAIL_VecPerm(unalignedData, unalignedData, alignmentPermution); \
    }

#endif

#define RW_MATH_VPU_CreateVectorMaskFromBool( ret, boolValue ) \
    { \
        ret = ::rw::math::vpu::detail::gBooleanConstants[ boolValue ? 1 : 0 ]; \
    }

// create a vector intrinsic data from 1 float
// ret - VectorIntrinsic
// f0 - float
#define RW_MATH_VPU_CreateVectorV1(ret, f0) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion u; \
        u.scalarFloat[0] = f0; \
        u.scalarFloat[1] = f0; \
        u.scalarFloat[2] = f0; \
        u.scalarFloat[3] = f0; \
        ret = u.vector; \
    }
 
//! \todo macroize
RW_MATH_FORCE_INLINE ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_CreateVectorFromScalar_R(float f0) 
{
    ::rw::math::vpu::VectorIntrinsic ret;
    RW_MATH_VPU_CreateVectorV1(ret, f0);
    return ret;
}

#define RW_MATH_VPU_CreateVectorV2(ret, f0, f1) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion u; \
        u.scalarFloat[0] = f0; \
        u.scalarFloat[1] = f1; \
        u.scalarFloat[2] = f0; \
        u.scalarFloat[3] = f0; \
        ret = u.vector; \
    }

#define RW_MATH_VPU_CreateVectorV3(ret, f0, f1, f2) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion u; \
        u.scalarFloat[0] = f0; \
        u.scalarFloat[1] = f1; \
        u.scalarFloat[2] = f2; \
        u.scalarFloat[3] = f0; \
        ret = u.vector; \
    }

#define RW_MATH_VPU_CreateVectorV4(ret, f0, f1, f2, f3) \
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion u; \
        u.scalarFloat[0] = f0; \
        u.scalarFloat[1] = f1; \
        u.scalarFloat[2] = f2; \
        u.scalarFloat[3] = f3; \
        ret = u.vector; \
    }

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
    { \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(vec_dest, vec_src, RWMATH_VPU_DETAIL_GetSwizzleControlV(vec_dest_index, vec_src_index)); \
    } \


#define RW_MATH_VPU_SwizzleStore_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic permuteControl = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_GetSwizzleControlI(  (vec_dest_index_immediate),  (vec_src_index_immediate) ); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(vec_dest, vec_src, permuteControl); \
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
        ::rw::math::vpu::VectorIntrinsic result = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec0, in_vec1, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI((::rw::math::vpu::detail::VectorPermSource)in_index0_immediate, (::rw::math::vpu::detail::VectorPermSource)((int)in_index1_immediate+4), ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
        /* Then store the final result in the z position */ \
        RW_MATH_VPU_SwizzleStore_I_I( ret, result, VectorAxisZ, in_vec2, in_index2_immediate ); \
    } \


#define RW_MATH_VPU_CreateVectorAxisA4_I(ret, in_vec0, in_index0_immediate, in_vec1, in_index1_immediate, in_vec2, in_index2_immediate, in_vec3, in_index3_immediate) \
    { \
        /* Permute the first two values (i.e. x,y) into the ZW position of a temporary vector. */ \
        ::rw::math::vpu::VectorIntrinsic temp0 = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec0, in_vec1, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0, (::rw::math::vpu::detail::VectorPermSource)in_index0_immediate, (::rw::math::vpu::detail::VectorPermSource)(in_index1_immediate+4)) ); \
        /* Permute the last two values (i.e. z,w) into the XY position of a temporary vector. */ \
        ::rw::math::vpu::VectorIntrinsic temp1 = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm( in_vec2, in_vec3, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI((::rw::math::vpu::detail::VectorPermSource)in_index2_immediate, (::rw::math::vpu::detail::VectorPermSource)(in_index3_immediate+4), ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
        /* Now combine the two temporary values into a single vector using RWMATH_VPU_DETAIL_VecSld (better than a permute). */ \
        ret = RWMATH_VPU_DETAIL_VecSld( temp0, temp1, sizeof(float)*2 ); \
    } \

#define RW_MATH_VPU_GetZero_R() \
    RWMATH_VPU_DETAIL_VecZero()

#define RW_MATH_VPU_GetZero(ret) \
    ret = RW_MATH_VPU_GetZero_R()

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

//----------------------------------------------------------------------------
// RW_MATH_VPU_GetTwo_R()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetTwo(ret) \
    ret = RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(2), 0); \

#define RW_MATH_VPU_GetTwo_R() \
    RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(2), 0)

#define RW_MATH_VPU_GetNegTwo_R() \
    RWMATH_VPU_DETAIL_VecVcfsx(RWMATH_VPU_DETAIL_VecSplatS32(-2), 0)

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
#define RW_MATH_VPU_GetPI(ret) \
    ret = ::rw::math::vpu::detail::gPI;

#define RW_MATH_VPU_GetPI_R() \
    (::rw::math::vpu::detail::gPI)

#define RW_MATH_VPU_Get2PI(ret) \
    ret = ::rw::math::vpu::detail::g2PI;

#define RW_MATH_VPU_Get2PIReciprocal(ret) \
    ret = ::rw::math::vpu::detail::g2PIReciprocal;

//----------------------------------------------------------------------------
// RW_MATH_VPU_GetI_R()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetI(ret) \
    { \
        ret = ::rw::math::vpu::detail::gIVector; \
    } \

#define RW_MATH_VPU_GetI_R() \
    (::rw::math::vpu::detail::gIVector)

//----------------------------------------------------------------------------
// RW_MATH_VPU_GetJ_R()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetJ(ret) \
    { \
        ret = ::rw::math::vpu::detail::gJVector; \
    } \

#define RW_MATH_VPU_GetJ_R() \
    (::rw::math::vpu::detail::gJVector)

//----------------------------------------------------------------------------
// RW_MATH_VPU_GetK_R()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_GetK(ret) \
    { \
        ret = ::rw::math::vpu::detail::gKVector; \
    } \

#define RW_MATH_VPU_GetK_R() \
    (::rw::math::vpu::detail::gKVector)

//----------------------------------------------------------------------------
// RW_MATH_VPU_GetL_R()
//----------------------------------------------------------------------------
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
    { \
        ret = RWMATH_VPU_DETAIL_VecSplatS32(static_cast<signed char>(-1)); \
        ret = RWMATH_VPU_DETAIL_VecSlw(ret, ret); \
        ret = RWMATH_VPU_DETAIL_VecSld( ret, RWMATH_VPU_DETAIL_VecZero(), sizeof(float)*1 ); \
    }


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
    { \
        ::rw::math::vpu::detail::VectorIntrinsicUnion vecUnion; \
        vecUnion.vector = in_vec; \
        ret = vecUnion.scalarFloat[(int)(in_index_immediate)]; \
    } \

#define RW_MATH_VPU_Floor(ret, a1) \
    ret = RWMATH_VPU_DETAIL_VecRfim((a1))

#define RW_MATH_VPU_Ceil(ret, a1) \
    ret = RWMATH_VPU_DETAIL_VecRfip((a1))

// Rounds the value towards zero. i.e. truncates the value.
#define RW_MATH_VPU_RoundToZero(ret, a1) \
    ret = RWMATH_VPU_DETAIL_VecRfiz((a1))


//----------------------------------------------------------------------------
// _asmMult()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_Mult(ret, in_vec0, in_vec1) \
    ret = RWMATH_VPU_DETAIL_VecMadd( in_vec0, in_vec1, RWMATH_VPU_DETAIL_VecZero() ); \


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
    ret = RWMATH_VPU_DETAIL_VecSqrtRe(in_vec); \

#define RW_MATH_VPU_RotateLeft_V_I(ret, in_vec, rotateAmount) \
    ret = RWMATH_VPU_DETAIL_VecSld(in_vec, in_vec, sizeof(float)*rotateAmount);

#define RW_MATH_VPU_ReciprocalSquareRootFast(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic estimate; \
        /* Do one iteration of Newton-Raphson estimation */ \
        RW_MATH_VPU_ReciprocalSquareRootEstimate( estimate, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ::rw::math::vpu::VectorIntrinsic vOne, vHalf; \
        RW_MATH_VPU_GetOne( vOne ); \
        RW_MATH_VPU_GetHalf( vHalf ); \
        ret = RWMATH_VPU_DETAIL_VecMadd( \
                                          RWMATH_VPU_DETAIL_VecMadd( estimate, vHalf, vZero ), \
                                          RWMATH_VPU_DETAIL_VecNmsub( in_vec, RWMATH_VPU_DETAIL_VecMadd(estimate,estimate,vZero), vOne), \
                                           estimate \
                                          ); \
    } \


#define RW_MATH_VPU_ReciprocalSquareRoot(ret, in_vec) \
    { \
        /* Do two iterations of Newton-Raphson estimation */ \
        ::rw::math::vpu::VectorIntrinsic secondEstimate; \
        RW_MATH_VPU_ReciprocalSquareRootFast( secondEstimate, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ::rw::math::vpu::VectorIntrinsic vOne, vHalf; \
        RW_MATH_VPU_GetOne( vOne ); \
        RW_MATH_VPU_GetHalf( vHalf ); \
        ret = RWMATH_VPU_DETAIL_VecMadd( \
                                          RWMATH_VPU_DETAIL_VecMadd( secondEstimate, vHalf, vZero ), \
                                          RWMATH_VPU_DETAIL_VecNmsub( in_vec, RWMATH_VPU_DETAIL_VecMadd(secondEstimate,secondEstimate, vZero), vOne), \
                                           secondEstimate \
                                          ); \
    } \


#define RW_MATH_VPU_Divide(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        RW_MATH_VPU_Reciprocal( reciprocal, in_vec1 ); \
        ret = RWMATH_VPU_DETAIL_VecMadd( reciprocal, in_vec0, RWMATH_VPU_DETAIL_VecZero() ); \
    } \


#define RW_MATH_VPU_DivideFloat(ret, in_vec, in_float) \
    { \
        ::rw::math::vpu::VectorIntrinsic reciprocal; \
        float floatRecip = 1.0f/(in_float); \
        RW_MATH_VPU_CreateVectorV1( reciprocal, floatRecip ); \
        ret = RWMATH_VPU_DETAIL_VecMadd( reciprocal, in_vec, RWMATH_VPU_DETAIL_VecZero() ); \
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
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ret0 = RWMATH_VPU_DETAIL_VecMadd( in_vec0, reciprocal, vZero ); \
        ret1 = RWMATH_VPU_DETAIL_VecMadd( in_vec1, reciprocal, vZero ); \
        ret2 = RWMATH_VPU_DETAIL_VecMadd( in_vec2, reciprocal, vZero ); \
    } \


// ret = in_vec0 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic splattedAxis = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( in_vec_scale, in_index_immediate ); \
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ret = RWMATH_VPU_DETAIL_VecMadd( in_vec0, splattedAxis, vZero ); \
    } \


// ret0 = in_vec0 * in_vec_scale[in_index_immediate];
// ret1 = in_vec1 * in_vec_scale[in_index_immediate];
// ret2 = in_vec2 * in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic splattedAxis = RWMATH_VPU_DETAIL_VecSplat( in_vec_scale, in_index_immediate ); \
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ret0 = RWMATH_VPU_DETAIL_VecMadd( in_vec0, splattedAxis, vZero ); \
        ret1 = RWMATH_VPU_DETAIL_VecMadd( in_vec1, splattedAxis, vZero ); \
        ret2 = RWMATH_VPU_DETAIL_VecMadd( in_vec2, splattedAxis, vZero ); \
    } \


#define RW_MATH_VPU_MultAdd(ret, in_vec0, in_vec1, in_vec2) \
    ret = RWMATH_VPU_DETAIL_VecMadd( in_vec0, in_vec1, in_vec2 );


#define RW_MATH_VPU_MultSubNeg(ret, in_vec0, in_vec1, in_vec2) \
    ret = RWMATH_VPU_DETAIL_VecNmsub( in_vec0, in_vec1, in_vec2 );

#define RW_MATH_VPU_Add(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Add_R( in_vec0, in_vec1 );

#define RW_MATH_VPU_Add_R(in_vec0, in_vec1) \
    RWMATH_VPU_DETAIL_VecAdd(in_vec0, in_vec1)

#define RW_MATH_VPU_Subtract(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Subtract_R( in_vec0, in_vec1 )

#define RW_MATH_VPU_Subtract_R(in_vec0, in_vec1) \
    RWMATH_VPU_DETAIL_VecSub( in_vec0, in_vec1 )

#define RW_MATH_VPU_And(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_And_R(in_vec0, in_vec1); 


#define RW_MATH_VPU_Or(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Or_R(in_vec0, in_vec1); 


#define RW_MATH_VPU_Not_R(a1, a2) \
    RWMATH_VPU_DETAIL_VecNor((a1), (a2))

#define RW_MATH_VPU_Not(ret, in_vec) \
    ret = RW_MATH_VPU_Not_R(in_vec, in_vec);


#define RW_MATH_VPU_Xor(ret, in_vec0, in_vec1) \
    ret = RW_MATH_VPU_Xor_R(in_vec0, in_vec1)


#define RW_MATH_VPU_ProductSum3(ret, v0, v1, v2, v3, v4, v5) \
    RW_MATH_VPU_Mult   (ret, v0, v1); \
    RW_MATH_VPU_MultAdd(ret, v2, v3, ret); \
    RW_MATH_VPU_MultAdd(ret, v4, v5, ret); \



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
        ret = ( !(bool)RWMATH_VPU_DETAIL_VecAnyGT( comparisonValue, in_tolerance ) ); \
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
        ::rw::math::vpu::VectorIntrinsic crossV2  = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(in_vec1, in_vec1, ::rw::math::vpu::detail::gCrossProductPermuteConstant); \
        ::rw::math::vpu::VectorIntrinsic crossV1  = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(in_vec0, in_vec0, ::rw::math::vpu::detail::gCrossProductPermuteConstant); \
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ::rw::math::vpu::VectorIntrinsic crossV3  = RWMATH_VPU_DETAIL_VecMadd(in_vec0, crossV2, vZero); \
        crossV3 = RWMATH_VPU_DETAIL_VecNmsub(crossV1, in_vec1, crossV3); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(crossV3, crossV3, ::rw::math::vpu::detail::gCrossProductPermuteConstant); \
    } \


//----------------------------------------------------------------------------
// _asmDotV4()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_DotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic product = RWMATH_VPU_DETAIL_VecMadd(in_vec0, in_vec1, RWMATH_VPU_DETAIL_VecZero()); \
        ::rw::math::vpu::VectorIntrinsic x_product = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat(product, 0); \
        ::rw::math::vpu::VectorIntrinsic y_product = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat(product, 1); \
        ret = RW_MATH_VPU_Add_R( x_product, y_product ); \
    } \


#define RW_MATH_VPU_DotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic xx_yy_zz = RWMATH_VPU_DETAIL_VecMadd( in_vec0, in_vec1, RWMATH_VPU_DETAIL_VecZero() ); \
        ::rw::math::vpu::VectorIntrinsic yy = RWMATH_VPU_DETAIL_VecSld( xx_yy_zz, xx_yy_zz, sizeof(float)*1 ); \
        ::rw::math::vpu::VectorIntrinsic zz = RWMATH_VPU_DETAIL_VecSld( xx_yy_zz, xx_yy_zz, sizeof(float)*2 ); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( RW_MATH_VPU_Add_R(RW_MATH_VPU_Add_R( xx_yy_zz, yy ), zz), 0 ); \
    } \


#define RW_MATH_VPU_DotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic xx_yy_zz_ww = RWMATH_VPU_DETAIL_VecMadd( in_vec0, in_vec1, RWMATH_VPU_DETAIL_VecZero() ); \
        ::rw::math::vpu::VectorIntrinsic zz_ww = RWMATH_VPU_DETAIL_VecSld( xx_yy_zz_ww, xx_yy_zz_ww, sizeof(float)*2 ); \
        ::rw::math::vpu::VectorIntrinsic xxpluszz_yyplusww = RW_MATH_VPU_Add_R(xx_yy_zz_ww, zz_ww); \
        ::rw::math::vpu::VectorIntrinsic yyplusww = RWMATH_VPU_DETAIL_VecSld( xxpluszz_yyplusww, xxpluszz_yyplusww, sizeof(float)*1 ); \
        ret = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecSplat( RW_MATH_VPU_Add_R( xxpluszz_yyplusww, yyplusww ), 0 ); \
    } \


// Note: It is difficult to directly compute the squareroot of a value using VMX, so this function
// computes x * x^(-1/2).  Of course this will give a mathematically undefined result for x = 0, so
// the code has a comparison that returns 0 if x = 0.
#define RW_MATH_VPU_SquareRoot(ret, in_vec) \
    { \
        const ::rw::math::vpu::VectorIntrinsic zero_vi = RWMATH_VPU_DETAIL_VecZero(); \
        ::rw::math::vpu::VectorIntrinsic isZeroMask = RWMATH_VPU_DETAIL_VecCmpEq( zero_vi, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic recipSqrt; \
        RW_MATH_VPU_ReciprocalSquareRoot( recipSqrt, in_vec ); \
        ::rw::math::vpu::VectorIntrinsic result_madd = RWMATH_VPU_DETAIL_VecMadd(in_vec, recipSqrt, zero_vi ); \
        ret = RW_MATH_VPU_Select_R( result_madd, zero_vi, isZeroMask ); \
    } \

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


#define RW_MATH_VPU_NormalizeV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
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
        ::rw::math::vpu::VectorIntrinsic zero_vi; \
        RW_MATH_VPU_GetZero(zero_vi); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero_vi); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero_vi, input_is_zero); \
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
        ::rw::math::vpu::VectorIntrinsic zero_vi; \
        RW_MATH_VPU_GetZero(zero_vi); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero_vi); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero_vi, input_is_zero); \
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
        ::rw::math::vpu::VectorIntrinsic zero_vi; \
        RW_MATH_VPU_GetZero(zero_vi); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero_vi); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero_vi, input_is_zero); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Lerp(ret, in_vec0, in_vec1, in_t) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_lerp_v; \
        RW_MATH_VPU_Subtract(temp_lerp_v, in_vec1, in_vec0); \
        RW_MATH_VPU_MultAdd(ret, temp_lerp_v, in_t, in_vec0); \
    } \


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
        ::rw::math::vpu::VectorIntrinsic vZero = RWMATH_VPU_DETAIL_VecZero(); \
        ::rw::math::vpu::VectorIntrinsic sqrt2s; \
        RW_MATH_VPU_GetSqrt2s(sqrt2s); \
        ::rw::math::vpu::VectorIntrinsic scaled_q = RWMATH_VPU_DETAIL_VecMadd(sqrt2s, in_q, vZero); \
        ::rw::math::vpu::VectorIntrinsic halves; \
        RW_MATH_VPU_GetHalves(halves); \
        ::rw::math::vpu::VectorIntrinsic neg_qq = RWMATH_VPU_DETAIL_VecNmsub( scaled_q, scaled_q, halves ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_yzx = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(scaled_q, scaled_q, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
        ::rw::math::vpu::VectorIntrinsic xy_yz_zx = RWMATH_VPU_DETAIL_VecMadd( scaled_q, scaled_q_yzx, vZero ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_zxy = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(scaled_q, scaled_q, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::X0) ); \
        ::rw::math::vpu::VectorIntrinsic scaled_q_w; \
        RW_MATH_VPU_Splat_I(scaled_q_w, scaled_q, VectorAxisW); \
        ::rw::math::vpu::VectorIntrinsic wz_wx_wy = RWMATH_VPU_DETAIL_VecMadd(scaled_q_w, scaled_q_zxy, vZero); \
        /*                                                      \
           Calculate:                                           \
             addVec = (xy + wz, yz + wx, zx + wy) and           \
             subVec = (xy - wz, yz - wx, zx - wy)               \
         */ \
        ::rw::math::vpu::VectorIntrinsic addVec = RW_MATH_VPU_Add_R( xy_yz_zx, wz_wx_wy ); \
        ::rw::math::vpu::VectorIntrinsic subVec = RW_MATH_VPU_Subtract_R( xy_yz_zx, wz_wx_wy ); \
        /* Calculate */                                                         \
        /* diagonalValues = ( neg_xx+neg_yy, neg_yy+neg_zz, neg_zz+neg_xx ) */  \
        ::rw::math::vpu::VectorIntrinsic neg_yy_zz_xx = (::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(neg_qq, neg_qq, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0) ); \
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

// Trig functions
#define RW_MATH_VPU_Cos(ret, in_vec) \
    ret = cosf4(in_vec);

#define RW_MATH_VPU_Sin(ret, in_vec) \
    ret = sinf4(in_vec);

#define RW_MATH_VPU_SinCos(ret0, ret1, in_vec) \
    ret0 = sinf4(in_vec); \
    ret1 = cosf4(in_vec);

#define RW_MATH_VPU_Tan(ret, in_vec) \
    ret = tanf4(in_vec);

#define RW_MATH_VPU_ACos(ret, in_vec) \
    ret = acosf4(in_vec);

#define RW_MATH_VPU_ASin(ret, in_vec) \
    ret = asinf4(in_vec);

#define RW_MATH_VPU_ATan(ret, in_vec) \
    ret = atanf4(in_vec);

// The macro below implements this simplified C code in a branch free fashion.
//   if ( b == 0.0f )
//   {
//       res1 = sign(a) * HALF_PI;
//       return res1;
//   }
//   else
//   {
//       res2 = atan(a/b);
//       // Use the arguments to determine the quadrant of the result:
//       if ( b < 0.0f )
//       {
//           res2 = sign(a) * PI + res2;
//       }
//       else
//       {
//           res2;
//       }
//       return res2;
//   }
#define RW_MATH_VPU_ATan2( ret, in_y, in_x ) \
{ \
    VectorIntrinsic zeroVec = RWMATH_VPU_DETAIL_VecZero(); /* create zero vector, so it can be used in a lot places */ \
    VectorIntrinsic signBit; \
    RW_MATH_VPU_GetSignMaskXYZW(signBit); \
 \
    VectorIntrinsic a_signBit = RW_MATH_VPU_And_R((vec_int4)in_y, signBit); \
 \
    VectorIntrinsic bNeg = RWMATH_VPU_DETAIL_VecCmpGT(zeroVec, in_x); \
    VectorIntrinsic bZero = RWMATH_VPU_DETAIL_VecCmpEq(zeroVec, in_x); \
 \
    VectorIntrinsic res1 = RW_MATH_VPU_Or_R((vec_int4)::rw::math::vpu::detail::gHalfPI, a_signBit); /* sign(in_y) HALF_PI */ \
 \
    VectorIntrinsic bRecip; \
    RW_MATH_VPU_ReciprocalFast( bRecip, in_x ); \
    VectorIntrinsic res2 = atanf4( RWMATH_VPU_DETAIL_VecMadd( in_y, bRecip, zeroVec ) ); /* atan(in_y/in_x); */ \
 \
    VectorIntrinsic b_signBit = RW_MATH_VPU_And_R((vec_int4)in_y, signBit); \
 \
    VectorIntrinsic newRes = RW_MATH_VPU_Add_R(RW_MATH_VPU_Or_R((vec_int4)::rw::math::vpu::detail::gPI, b_signBit), res2); /* sign(in_y) * PI + res2 */ \
    res2 = RW_MATH_VPU_Select_R(res2, newRes, bNeg); \
 \
    ret = RW_MATH_VPU_Select_R(res2, res1, bZero); \
}

#define RW_MATH_VPU_ClampToNegPosRange(ret, in_vec0, range) \
    { \
        VectorIntrinsic zero, signMask; \
        RW_MATH_VPU_GetSignMaskXYZW(signMask); \
        RW_MATH_VPU_GetZero(zero); \
        VectorIntrinsic isEqualMask = RWMATH_VPU_DETAIL_VecCmpEqUnsignedWord(RWMATH_VPU_DETAIL_VecCmpB(in_vec0, range), zero); \
        ret = RW_MATH_VPU_Select_R(RW_MATH_VPU_Select_R(range, in_vec0, signMask), in_vec0, isEqualMask ); \
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

} // namespace vpu

} // namespace math

} // namespace rw

#endif
