// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_WII_VECTOR_INTRINSIC_MACRO_INLINE_H
#define RW_MATH_VPU_DETAIL_WII_VECTOR_INTRINSIC_MACRO_INLINE_H

namespace rw
{
namespace math
{

namespace fpu
{
    namespace detail
    {
        // use fpu's Sqrt implementation.
        float Sqrt(float a);
    }
}

namespace vpu
{

#define RW_MATH_VPU_V2_RotateLeft( ret, vec) \
{ \
    ret.i[0] = vec.i[1]; \
    ret.i[1] = vec.i[0]; \
} \

#define RW_MATH_VPU_V3_RotateLeft( ret, vec) \
{ \
    ret.i[0] = vec.i[1]; \
    ret.i[1] = vec.i[2]; \
    ret.i[2] = vec.i[0]; \
} \

#define RW_MATH_VPU_V3_RotateRight( ret, vec) \
{ \
    ret.i[0] = vec.i[2]; \
    ret.i[1] = vec.i[0]; \
    ret.i[2] = vec.i[1]; \
} \

#define RW_MATH_VPU_V4_RotateLeft( ret, vec) \
{ \
    ret.i[0] = vec.i[1]; \
    ret.i[1] = vec.i[2]; \
    ret.i[2] = vec.i[3]; \
    ret.i[3] = vec.i[0]; \
} \

#define RW_MATH_VPU_V4_RotateRight( ret, vec) \
{ \
    ret.i[0] = vec.i[3]; \
    ret.i[1] = vec.i[0]; \
    ret.i[2] = vec.i[1]; \
    ret.i[3] = vec.i[2]; \
} \

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
// RW_MATH_VPU_Select_R()
//----------------------------------------------------------------------------
// t0 = b & c;
// t1 = a & ^c;
// r = t0 | t1;
// use of VectorIntrinsicUnion is to get ride off aliasing issues
#define RW_MATH_VPU_Select(ret, a, b, c) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion asmSelect_ua, asmSelect_ub, asmSelect_uc, asmSelect_uret; \
        asmSelect_ua.vector = (a); \
        asmSelect_ub.vector = (b); \
        asmSelect_uc.vector = (c); \
        asmSelect_uret.scalarUInt[0] = (asmSelect_ub.scalarUInt[0] & asmSelect_uc.scalarUInt[0]) | (asmSelect_ua.scalarUInt[0] & ~asmSelect_uc.scalarUInt[0]); \
        asmSelect_uret.scalarUInt[1] = (asmSelect_ub.scalarUInt[1] & asmSelect_uc.scalarUInt[1]) | (asmSelect_ua.scalarUInt[1] & ~asmSelect_uc.scalarUInt[1]); \
        asmSelect_uret.scalarUInt[2] = (asmSelect_ub.scalarUInt[2] & asmSelect_uc.scalarUInt[2]) | (asmSelect_ua.scalarUInt[2] & ~asmSelect_uc.scalarUInt[2]); \
        asmSelect_uret.scalarUInt[3] = (asmSelect_ub.scalarUInt[3] & asmSelect_uc.scalarUInt[3]) | (asmSelect_ua.scalarUInt[3] & ~asmSelect_uc.scalarUInt[3]); \
        ret = asmSelect_uret.vector; \
    } \

RW_MATH_FORCE_INLINE VectorIntrinsic RW_MATH_VPU_LoadFromAlignedFloatPointer( const float *ptr )
{
    VectorIntrinsic result;
    result.f[0] = ptr[0];
    result.f[1] = ptr[1];
    result.f[2] = ptr[2];
    result.f[3] = ptr[3];
    return result;
}


#define RW_MATH_VPU_UnalignedLoad(ret, address) \
{ \
    ret.f[0] = address[0]; \
    ret.f[1] = address[1]; \
    ret.f[2] = address[2]; \
    ret.f[3] = address[3]; \
}

#define RW_MATH_VPU_UnalignedStore2(dest, vector) \
{ \
    destination.x = vector.f[0]; \
    destination.y = vector.f[1]; \
}

#define RW_MATH_VPU_UnalignedStore3(dest, vector) \
{ \
    destination.x = vector.f[0]; \
    destination.y = vector.f[1]; \
    destination.z = vector.f[2]; \
}

#define RW_MATH_VPU_UnalignedStore4(dest, vector) \
{ \
    destination.x = vector.f[0]; \
    destination.y = vector.f[1]; \
    destination.z = vector.f[2]; \
    destination.w = vector.f[3]; \
}

//----------------------------------------------------------------------------
// RW_MATH_VPU_IsGreater_R()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsGreater(ret, a, b) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion asmIsGreater_ua, asmIsGreater_ub, asmIsGreater_uret; \
        asmIsGreater_ua.vector = (a); \
        asmIsGreater_ub.vector = (b); \
        asmIsGreater_uret.scalarUInt[0] = asmIsGreater_ua.scalarFloat[0] > asmIsGreater_ub.scalarFloat[0] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[1] = asmIsGreater_ua.scalarFloat[1] > asmIsGreater_ub.scalarFloat[1] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[2] = asmIsGreater_ua.scalarFloat[2] > asmIsGreater_ub.scalarFloat[2] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[3] = asmIsGreater_ua.scalarFloat[3] > asmIsGreater_ub.scalarFloat[3] ? (0xffffffff) : (0x0); \
        ret = asmIsGreater_uret.vector; \
    } \

//----------------------------------------------------------------------------
// RW_MATH_VPU_IsGreaterEqual_R()
//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsGreaterEqual(ret, a, b) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion asmIsGreater_ua, asmIsGreater_ub, asmIsGreater_uret; \
        asmIsGreater_ua.vector = (a); \
        asmIsGreater_ub.vector = (b); \
        asmIsGreater_uret.scalarUInt[0] = asmIsGreater_ua.scalarFloat[0] >= asmIsGreater_ub.scalarFloat[0] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[1] = asmIsGreater_ua.scalarFloat[1] >= asmIsGreater_ub.scalarFloat[1] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[2] = asmIsGreater_ua.scalarFloat[2] >= asmIsGreater_ub.scalarFloat[2] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[3] = asmIsGreater_ua.scalarFloat[3] >= asmIsGreater_ub.scalarFloat[3] ? (0xffffffff) : (0x0); \
        ret = asmIsGreater_uret.vector; \
    } \

//----------------------------------------------------------------------------
#define RW_MATH_VPU_IsEqual(ret, a, b) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion asmIsGreater_ua, asmIsGreater_ub, asmIsGreater_uret; \
        asmIsGreater_ua.vector = (a); \
        asmIsGreater_ub.vector = (b); \
        asmIsGreater_uret.scalarUInt[0] = asmIsGreater_ua.scalarFloat[0] == asmIsGreater_ub.scalarFloat[0] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[1] = asmIsGreater_ua.scalarFloat[1] == asmIsGreater_ub.scalarFloat[1] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[2] = asmIsGreater_ua.scalarFloat[2] == asmIsGreater_ub.scalarFloat[2] ? (0xffffffff) : (0x0); \
        asmIsGreater_uret.scalarUInt[3] = asmIsGreater_ua.scalarFloat[3] == asmIsGreater_ub.scalarFloat[3] ? (0xffffffff) : (0x0); \
        ret = asmIsGreater_uret.vector; \
    } \

// The scalar comparison macros assume that all elements of the vector are
// identical.  The return result is either boolean or an integer that indicates
// true/false.
#define RW_MATH_VPU_Scalar_IsEqual(result, a, b) \
        result = (a.f[0] == b.f[0]);

#define RW_MATH_VPU_Scalar_IsGreater(result, a, b) \
        result = (a.f[0] > b.f[0]);

#define RW_MATH_VPU_Scalar_IsGreaterEqual(result, a, b) \
        result = (a.f[0] >= b.f[0]);

#define RW_MATH_VPU_Scalar_IsLess(result, a, b) \
        result = (a.f[0] < b.f[0]);

#define RW_MATH_VPU_Scalar_IsLessEqual(result, a, b) \
        result = (a.f[0] <= b.f[0]);

#define RW_MATH_VPU_CreateVectorMaskFromBool( ret, boolValue ) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vecUnion; \
        unsigned int value = boolValue ? 0xffffffff : 0; \
        vecUnion.scalarUInt[0] = value; \
        vecUnion.scalarUInt[1] = value; \
        vecUnion.scalarUInt[2] = value; \
        vecUnion.scalarUInt[3] = value; \
        ret = vecUnion.vector; \
    }


// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV4(ret, f0, f1, f2, f3) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarFloat[0] = f0; \
        vu_temp.scalarFloat[1] = f1; \
        vu_temp.scalarFloat[2] = f2; \
        vu_temp.scalarFloat[3] = f3; \
        ret = vu_temp.vector; \
    }


// create a vector intrinsic from a pointer to a float.
// The float data will be splatted across the entire vector.
#define RW_MATH_VPU_CreateVectorV1_FromMemory(ret, fPtr) \
    RW_MATH_VPU_CreateVectorV4( ret, *(fPtr), *(fPtr), *(fPtr), *(fPtr) );


// create a vector intrinsic data from 1 float
// ret - VectorIntrinsic
// f0 - float
#define RW_MATH_VPU_CreateVectorV1(ret, f0) \
    RW_MATH_VPU_CreateVectorV4( ret, f0, f0, f0, f0 );


// create a vector intrinsic data from two floats
#define RW_MATH_VPU_CreateVectorV2(ret, f0, f1) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarFloat[0] = f0; \
        vu_temp.scalarFloat[1] = f1; \
        vu_temp.scalarFloat[2] = f0; \
        vu_temp.scalarFloat[3] = f0; \
        ret = vu_temp.vector; \
    }


// create a vector intrinsic data from three floats
#define RW_MATH_VPU_CreateVectorV3(ret, f0, f1, f2) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarFloat[0] = f0; \
        vu_temp.scalarFloat[1] = f1; \
        vu_temp.scalarFloat[2] = f2; \
        vu_temp.scalarFloat[3] = f0; \
        ret = vu_temp.vector; \
    }


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
        ::rw::math::vpu::VectorIntrinsic temp_vec= vec_dest; \
        temp_vec.f[vec_dest_index] = vec_src.f[vec_src_index]; \
        ret = temp_vec; \
    } \


#define RW_MATH_VPU_SwizzleStore_I_I(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_vec= vec_dest; \
        temp_vec.f[vec_dest_index_immediate] = vec_src.f[vec_src_index_immediate]; \
        ret = temp_vec; \
    } \


#define RW_MATH_VPU_SwizzleStore_I_V(ret, vec_dest, vec_dest_index_immediate, vec_src, vec_src_index) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_vec= vec_dest; \
        temp_vec.f[vec_dest_index_immediate] = vec_src.f[vec_src_index]; \
        ret = temp_vec; \
    } \


#define RW_MATH_VPU_SwizzleStore_V_I(ret, vec_dest, vec_dest_index, vec_src, vec_src_index_immediate) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_vec= vec_dest; \
        temp_vec.f[vec_dest_index] = vec_src.f[vec_src_index_immediate]; \
        ret = temp_vec; \
    } \


#define RW_MATH_VPU_SwizzleYZWX(ret, vec_in) \
    ret.f[0] = vec_in.f[1]; \
    ret.f[1] = vec_in.f[2]; \
    ret.f[2] = vec_in.f[3]; \
    ret.f[3] = vec_in.f[0]; \


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
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 0.0f, 0.0f, 0.0f );


#define RW_MATH_VPU_GetHalf(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.5f, 0.5f, 0.5f, 0.5f );


#define RW_MATH_VPU_GetOne(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 1.0f, 1.0f, 1.0f, 1.0f );


#define RW_MATH_VPU_GetTwo(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 2.0f, 2.0f, 2.0f, 2.0f );


#define RW_MATH_VPU_GetNegOne(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, -1.0f, -1.0f, -1.0f, -1.0f );


#define RW_MATH_VPU_GetTrue(ret) \
    { \
        ret.i[0] = 0xffffffff; \
        ret.i[1] = 0xffffffff; \
        ret.i[2] = 0xffffffff; \
        ret.i[3] = 0xffffffff; \
    }

#define RW_MATH_VPU_CompAnyTrueV3( ret, a ) \
    { \
        uint32_t companytrue_result = a.i[0] | a.i[1] | a.i[2]; \
        ret.i[0] = companytrue_result; \
        ret.i[1] = companytrue_result; \
        ret.i[2] = companytrue_result; \
        ret.i[3] = companytrue_result; \
    }

#define RW_MATH_VPU_CompAllTrueV3( ret, a ) \
    { \
        uint32_t companytrue_result = a.i[0] & a.i[1] & a.i[2]; \
        ret.i[0] = companytrue_result; \
        ret.i[1] = companytrue_result; \
        ret.i[2] = companytrue_result; \
        ret.i[3] = companytrue_result; \
    }

#define RW_MATH_VPU_CompAnyTrueV4( ret, a ) \
    { \
        uint32_t companytrue_result = a.i[0] | a.i[1] | a.i[2] | a.i[3]; \
        ret.i[0] = companytrue_result; \
        ret.i[1] = companytrue_result; \
        ret.i[2] = companytrue_result; \
        ret.i[3] = companytrue_result; \
    }

#define RW_MATH_VPU_CompAllTrueV4( ret, a ) \
    { \
        uint32_t companytrue_result = a.i[0] & a.i[1] & a.i[2] & a.i[3]; \
        ret.i[0] = companytrue_result; \
        ret.i[1] = companytrue_result; \
        ret.i[2] = companytrue_result; \
        ret.i[3] = companytrue_result; \
    }

#define RW_MATH_VPU_GetPI(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 3.14159265358979323846f, 3.14159265358979323846f, 3.14159265358979323846f, 3.14159265358979323846f);

#define RW_MATH_VPU_Get2PI(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 6.28318530717958f, 6.28318530717958f, 6.28318530717958f, 6.28318530717958f );

#define RW_MATH_VPU_Get2PIReciprocal(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f );

#define RW_MATH_VPU_GetI(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 1.0f, 0.0f, 0.0f, 0.0f);


#define RW_MATH_VPU_GetJ(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 1.0f, 0.0f, 0.0f);


#define RW_MATH_VPU_GetK(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 0.0f, 1.0f, 0.0f);


#define RW_MATH_VPU_GetL(ret) \
    RW_MATH_VPU_CreateVectorV4(ret, 0.0f, 0.0f, 0.0f, 1.0f);


#define RW_MATH_VPU_GetSignMaskXYZW(ret) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarUInt[0] = 0x80000000; \
        vu_temp.scalarUInt[1] = 0x80000000; \
        vu_temp.scalarUInt[2] = 0x80000000; \
        vu_temp.scalarUInt[3] = 0x80000000; \
        ret = vu_temp.vector; \
    } \


#define RW_MATH_VPU_GetSignMaskXYZ(ret) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarUInt[0] = 0x80000000; \
        vu_temp.scalarUInt[1] = 0x80000000; \
        vu_temp.scalarUInt[2] = 0x80000000; \
        vu_temp.scalarUInt[3] = 0x00000000; \
        ret = vu_temp.vector; \
    } \


#define RW_MATH_VPU_GetSignMaskXZ(ret) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarUInt[0] = 0x80000000; \
        vu_temp.scalarUInt[1] = 0x00000000; \
        vu_temp.scalarUInt[2] = 0x80000000; \
        vu_temp.scalarUInt[3] = 0x00000000; \
        ret = vu_temp.vector; \
    } \


#define RW_MATH_VPU_GetSignMaskYW(ret) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion vu_temp; \
        vu_temp.scalarUInt[0] = 0x00000000; \
        vu_temp.scalarUInt[1] = 0x80000000; \
        vu_temp.scalarUInt[2] = 0x00000000; \
        vu_temp.scalarUInt[3] = 0x80000000; \
        ret = vu_temp.vector; \
    } \


// ret - float
// in_vec - VectorIntrinsic
// in_index_immediate - integer value (immediate value)
#define RW_MATH_VPU_GetFloatAxis_I(ret, in_vec, in_index_immediate) \
    { \
        ret = in_vec.f[in_index_immediate]; \
    } \

#define RW_MATH_VPU_Floor(ret, a1) \
    ret.f[0] = floorf(a1.f[0]); \
    ret.f[1] = floorf(a1.f[1]); \
    ret.f[2] = floorf(a1.f[2]); \
    ret.f[3] = floorf(a1.f[3]);

#define RW_MATH_VPU_Ceil(ret, a1) \
    ret.f[0] = ceilf(a1.f[0]); \
    ret.f[1] = ceilf(a1.f[1]); \
    ret.f[2] = ceilf(a1.f[2]); \
    ret.f[3] = ceilf(a1.f[3]);

#define RW_MATH_VPU_Mult(ret, in_vec0, in_vec1) \
    ret.f[0] = in_vec0.f[0]*in_vec1.f[0]; \
    ret.f[1] = in_vec0.f[1]*in_vec1.f[1]; \
    ret.f[2] = in_vec0.f[2]*in_vec1.f[2]; \
    ret.f[3] = in_vec0.f[3]*in_vec1.f[3];


#define RW_MATH_VPU_MultFloat(ret, in_vec, in_float) \
    ret.f[0] = in_vec.f[0]*in_float; \
    ret.f[1] = in_vec.f[1]*in_float; \
    ret.f[2] = in_vec.f[2]*in_float; \
    ret.f[3] = in_vec.f[3]*in_float;


#define RW_MATH_VPU_Negate(ret, in_vec) \
    ret.f[0] = -(in_vec.f[0]); \
    ret.f[1] = -(in_vec.f[1]); \
    ret.f[2] = -(in_vec.f[2]); \
    ret.f[3] = -(in_vec.f[3]);


#define RW_MATH_VPU_NegateXZ(ret, in_vec) \
    ret.f[0] = -(in_vec.f[0]); \
    ret.f[1] =  (in_vec.f[1]); \
    ret.f[2] = -(in_vec.f[2]); \
    ret.f[3] =  (in_vec.f[3]);


#define RW_MATH_VPU_NegateYW(ret, in_vec) \
    ret.f[0] =  (in_vec.f[0]); \
    ret.f[1] = -(in_vec.f[1]); \
    ret.f[2] =  (in_vec.f[2]); \
    ret.f[3] = -(in_vec.f[3]);


#define RW_MATH_VPU_ProductDifference(ret, v0, v1, v2, v3) \
    ret.f[0] = v0.f[0]*v1.f[0] - v2.f[0]*v3.f[0]; \
    ret.f[1] = v0.f[1]*v1.f[1] - v2.f[1]*v3.f[1]; \
    ret.f[2] = v0.f[2]*v1.f[2] - v2.f[2]*v3.f[2]; \
    ret.f[3] = v0.f[3]*v1.f[3] - v2.f[3]*v3.f[3];

namespace detail
{
	VectorIntrinsic SqrtEstimate(VectorIntrinsicInParam a);
    VectorIntrinsic SqrtFast(VectorIntrinsicInParam a);
	VectorIntrinsic SqrtFastV2(VectorIntrinsicInParam a);
    VectorIntrinsic Sqrt(VectorIntrinsicInParam a);
	VectorIntrinsic ReciprocalEstimate(VectorIntrinsicInParam a);
	VectorIntrinsic ReciprocalFast(VectorIntrinsicInParam a);
	VectorIntrinsic ReciprocalFastV2(VectorIntrinsicInParam a);
	VectorIntrinsic Reciprocal(VectorIntrinsicInParam a);
	VectorIntrinsic InvSqrt(VectorIntrinsicInParam a);
	VectorIntrinsic InvSqrtFast(VectorIntrinsicInParam a);
	VectorIntrinsic InvSqrtFastV2(VectorIntrinsicInParam a);
	VectorIntrinsic InvSqrtEstimate(VectorIntrinsicInParam a);
}

#define RW_MATH_VPU_ReciprocalEstimate(ret, in_vec) \
	ret = ::rw::math::vpu::detail::ReciprocalEstimate(in_vec); 

#define RW_MATH_VPU_ReciprocalFastV2(ret, in_vec) \
	ret = ::rw::math::vpu::detail::ReciprocalFastV2(in_vec);

#define RW_MATH_VPU_ReciprocalFast(ret, in_vec) \
	ret = ::rw::math::vpu::detail::ReciprocalFast(in_vec);

#define RW_MATH_VPU_Reciprocal(ret, in_vec) \
	ret = ::rw::math::vpu::detail::Reciprocal(in_vec);


#define RW_MATH_VPU_ReciprocalSquareRootEstimate(ret, in_vec) \
	ret = ::rw::math::vpu::detail::InvSqrtEstimate(in_vec);

#define RW_MATH_VPU_ReciprocalSquareRootFastV2(ret, in_vec) \
	ret = ::rw::math::vpu::detail::InvSqrtFastV2(in_vec);

#define RW_MATH_VPU_ReciprocalSquareRootFast(ret, in_vec) \
	ret = ::rw::math::vpu::detail::InvSqrtFast(in_vec);

#define RW_MATH_VPU_ReciprocalSquareRoot(ret, in_vec) \
	ret = ::rw::math::vpu::detail::InvSqrt(in_vec);


#define RW_MATH_VPU_SquareRoot(ret, in_vec) \
	ret = ::rw::math::vpu::detail::Sqrt(in_vec);

#define RW_MATH_VPU_SquareRootFastV2(ret, in_vec) \
	ret = ::rw::math::vpu::detail::SqrtFastV2(in_vec);

#define RW_MATH_VPU_SquareRootFast(ret, in_vec) \
	ret = ::rw::math::vpu::detail::SqrtFast(in_vec);

#define RW_MATH_VPU_SquareRootEstimate(ret, in_vec) \
	ret = ::rw::math::vpu::detail::SqrtEstimate(in_vec);


#define RW_MATH_VPU_Divide(ret, in_vec0, in_vec1) \
    ret.f[0] = in_vec0.f[0] / in_vec1.f[0]; \
    ret.f[1] = in_vec0.f[1] / in_vec1.f[1]; \
    ret.f[2] = in_vec0.f[2] / in_vec1.f[2]; \
    ret.f[3] = in_vec0.f[3] / in_vec1.f[3];


#define RW_MATH_VPU_DivideFloat(ret, in_vec, in_float) \
    ret.f[0] = in_vec.f[0] / in_float; \
    ret.f[1] = in_vec.f[1] / in_float; \
    ret.f[2] = in_vec.f[2] / in_float; \
    ret.f[3] = in_vec.f[3] / in_float;


// ret = in_vec0 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_DivideAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    ret.f[0] = in_vec0.f[0] / in_vec_scale.f[in_index_immediate]; \
    ret.f[1] = in_vec0.f[1] / in_vec_scale.f[in_index_immediate]; \
    ret.f[2] = in_vec0.f[2] / in_vec_scale.f[in_index_immediate]; \
    ret.f[3] = in_vec0.f[3] / in_vec_scale.f[in_index_immediate]; \


// ret0 = in_vec0 / in_vec_scale[in_index_immediate];
// ret1 = in_vec1 / in_vec_scale[in_index_immediate];
// ret2 = in_vec2 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_DivideAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
    ret0.f[0] = in_vec0.f[0] / in_vec_scale.f[in_index_immediate]; \
    ret0.f[1] = in_vec0.f[1] / in_vec_scale.f[in_index_immediate]; \
    ret0.f[2] = in_vec0.f[2] / in_vec_scale.f[in_index_immediate]; \
    ret0.f[3] = in_vec0.f[3] / in_vec_scale.f[in_index_immediate]; \
    ret1.f[0] = in_vec1.f[0] / in_vec_scale.f[in_index_immediate]; \
    ret1.f[1] = in_vec1.f[1] / in_vec_scale.f[in_index_immediate]; \
    ret1.f[2] = in_vec1.f[2] / in_vec_scale.f[in_index_immediate]; \
    ret1.f[3] = in_vec1.f[3] / in_vec_scale.f[in_index_immediate]; \
    ret2.f[0] = in_vec2.f[0] / in_vec_scale.f[in_index_immediate]; \
    ret2.f[1] = in_vec2.f[1] / in_vec_scale.f[in_index_immediate]; \
    ret2.f[2] = in_vec2.f[2] / in_vec_scale.f[in_index_immediate]; \
    ret2.f[3] = in_vec2.f[3] / in_vec_scale.f[in_index_immediate]; \


// ret = in_vec0 / in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA1_I(ret, in_vec0, in_vec_scale, in_index_immediate) \
    ret.f[0] = in_vec0.f[0] * in_vec_scale.f[in_index_immediate]; \
    ret.f[1] = in_vec0.f[1] * in_vec_scale.f[in_index_immediate]; \
    ret.f[2] = in_vec0.f[2] * in_vec_scale.f[in_index_immediate]; \
    ret.f[3] = in_vec0.f[3] * in_vec_scale.f[in_index_immediate]; \


// ret0 = in_vec0 * in_vec_scale[in_index_immediate];
// ret1 = in_vec1 * in_vec_scale[in_index_immediate];
// ret2 = in_vec2 * in_vec_scale[in_index_immediate];
#define RW_MATH_VPU_MultAxisA3_I(ret0, ret1, ret2, in_vec0, in_vec1, in_vec2, in_vec_scale, in_index_immediate) \
    ret0.f[0] = in_vec0.f[0] * in_vec_scale.f[in_index_immediate]; \
    ret0.f[1] = in_vec0.f[1] * in_vec_scale.f[in_index_immediate]; \
    ret0.f[2] = in_vec0.f[2] * in_vec_scale.f[in_index_immediate]; \
    ret0.f[3] = in_vec0.f[3] * in_vec_scale.f[in_index_immediate]; \
    ret1.f[0] = in_vec1.f[0] * in_vec_scale.f[in_index_immediate]; \
    ret1.f[1] = in_vec1.f[1] * in_vec_scale.f[in_index_immediate]; \
    ret1.f[2] = in_vec1.f[2] * in_vec_scale.f[in_index_immediate]; \
    ret1.f[3] = in_vec1.f[3] * in_vec_scale.f[in_index_immediate]; \
    ret2.f[0] = in_vec2.f[0] * in_vec_scale.f[in_index_immediate]; \
    ret2.f[1] = in_vec2.f[1] * in_vec_scale.f[in_index_immediate]; \
    ret2.f[2] = in_vec2.f[2] * in_vec_scale.f[in_index_immediate]; \
    ret2.f[3] = in_vec2.f[3] * in_vec_scale.f[in_index_immediate]; \


#define RW_MATH_VPU_MultAdd(ret, in_vec0, in_vec1, in_vec2) \
    ret.f[0] = in_vec0.f[0] * in_vec1.f[0] + in_vec2.f[0]; \
    ret.f[1] = in_vec0.f[1] * in_vec1.f[1] + in_vec2.f[1]; \
    ret.f[2] = in_vec0.f[2] * in_vec1.f[2] + in_vec2.f[2]; \
    ret.f[3] = in_vec0.f[3] * in_vec1.f[3] + in_vec2.f[3]; \


#define RW_MATH_VPU_MultSubNeg(ret, in_vec0, in_vec1, in_vec2) \
    ret.f[0] = -(in_vec0.f[0] * in_vec1.f[0] - in_vec2.f[0]); \
    ret.f[1] = -(in_vec0.f[1] * in_vec1.f[1] - in_vec2.f[1]); \
    ret.f[2] = -(in_vec0.f[2] * in_vec1.f[2] - in_vec2.f[2]); \
    ret.f[3] = -(in_vec0.f[3] * in_vec1.f[3] - in_vec2.f[3]); \


#define RW_MATH_VPU_Add(ret, in_vec0, in_vec1) \
    ret.f[0] = in_vec0.f[0] + in_vec1.f[0]; \
    ret.f[1] = in_vec0.f[1] + in_vec1.f[1]; \
    ret.f[2] = in_vec0.f[2] + in_vec1.f[2]; \
    ret.f[3] = in_vec0.f[3] + in_vec1.f[3]; \


#define RW_MATH_VPU_Subtract(ret, in_vec0, in_vec1) \
    ret.f[0] = in_vec0.f[0] - in_vec1.f[0]; \
    ret.f[1] = in_vec0.f[1] - in_vec1.f[1]; \
    ret.f[2] = in_vec0.f[2] - in_vec1.f[2]; \
    ret.f[3] = in_vec0.f[3] - in_vec1.f[3]; \


#define RW_MATH_VPU_And(ret, in_vec0, in_vec1) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion detail_asmAnd_a, detail_asmAnd_b, detail_asmAnd_r; \
        detail_asmAnd_a.vector = in_vec0; \
        detail_asmAnd_b.vector = in_vec1; \
        detail_asmAnd_r.scalarUInt[0] = detail_asmAnd_a.scalarUInt[0] & detail_asmAnd_b.scalarUInt[0]; \
        detail_asmAnd_r.scalarUInt[1] = detail_asmAnd_a.scalarUInt[1] & detail_asmAnd_b.scalarUInt[1]; \
        detail_asmAnd_r.scalarUInt[2] = detail_asmAnd_a.scalarUInt[2] & detail_asmAnd_b.scalarUInt[2]; \
        detail_asmAnd_r.scalarUInt[3] = detail_asmAnd_a.scalarUInt[3] & detail_asmAnd_b.scalarUInt[3]; \
        ret = detail_asmAnd_r.vector; \
    } \


#define RW_MATH_VPU_Or(ret, in_vec0, in_vec1) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion detail_asmOr_a, detail_asmOr_b, detail_asmOr_r; \
        detail_asmOr_a.vector = in_vec0; \
        detail_asmOr_b.vector = in_vec1; \
        detail_asmOr_r.scalarUInt[0] = detail_asmOr_a.scalarUInt[0] | detail_asmOr_b.scalarUInt[0]; \
        detail_asmOr_r.scalarUInt[1] = detail_asmOr_a.scalarUInt[1] | detail_asmOr_b.scalarUInt[1]; \
        detail_asmOr_r.scalarUInt[2] = detail_asmOr_a.scalarUInt[2] | detail_asmOr_b.scalarUInt[2]; \
        detail_asmOr_r.scalarUInt[3] = detail_asmOr_a.scalarUInt[3] | detail_asmOr_b.scalarUInt[3]; \
        ret = detail_asmOr_r.vector; \
    } \


#define RW_MATH_VPU_Not(ret, in_vec) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion detail_asmNot_a, detail_asmNot_r; \
        detail_asmNot_a.vector = in_vec; \
        detail_asmNot_r.scalarUInt[0] = ~detail_asmNot_a.scalarUInt[0]; \
        detail_asmNot_r.scalarUInt[1] = ~detail_asmNot_a.scalarUInt[1]; \
        detail_asmNot_r.scalarUInt[2] = ~detail_asmNot_a.scalarUInt[2]; \
        detail_asmNot_r.scalarUInt[3] = ~detail_asmNot_a.scalarUInt[3]; \
        ret = detail_asmNot_r.vector; \
    } \


#define RW_MATH_VPU_Xor(ret, in_vec0, in_vec1) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion detail_asmXor_a, detail_asmXor_b, detail_asmXor_r; \
        detail_asmXor_a.vector = in_vec0; \
        detail_asmXor_b.vector = in_vec1; \
        detail_asmXor_r.scalarUInt[0] = detail_asmXor_a.scalarUInt[0] ^ detail_asmXor_b.scalarUInt[0]; \
        detail_asmXor_r.scalarUInt[1] = detail_asmXor_a.scalarUInt[1] ^ detail_asmXor_b.scalarUInt[1]; \
        detail_asmXor_r.scalarUInt[2] = detail_asmXor_a.scalarUInt[2] ^ detail_asmXor_b.scalarUInt[2]; \
        detail_asmXor_r.scalarUInt[3] = detail_asmXor_a.scalarUInt[3] ^ detail_asmXor_b.scalarUInt[3]; \
        ret = detail_asmXor_r.vector; \
    } \


#define RW_MATH_VPU_ProductSum3(ret, v0, v1, v2, v3, v4, v5) \
    RW_MATH_VPU_Mult   (ret, v0, v1); \
    RW_MATH_VPU_MultAdd(ret, v2, v3, ret); \
    RW_MATH_VPU_MultAdd(ret, v4, v5, ret); \


#define RW_MATH_VPU_IsEqualV2(ret, in_vec0, in_vec1) \
    if ((in_vec0.f[0] == in_vec1.f[0]) && \
        (in_vec0.f[1] == in_vec1.f[1])) \
    { \
        ret = true; \
    } \
    else \
    { \
        ret = false; \
    } \


#define RW_MATH_VPU_IsEqualV3(ret, in_vec0, in_vec1) \
    if ((in_vec0.f[0] == in_vec1.f[0]) && \
        (in_vec0.f[1] == in_vec1.f[1]) && \
        (in_vec0.f[2] == in_vec1.f[2])) \
    { \
        ret = true; \
    } \
    else \
    { \
        ret = false; \
    } \


#define RW_MATH_VPU_IsEqualV4(ret, in_vec0, in_vec1) \
    if ((in_vec0.f[0] == in_vec1.f[0]) && \
        (in_vec0.f[1] == in_vec1.f[1]) && \
        (in_vec0.f[2] == in_vec1.f[2]) && \
        (in_vec0.f[3] == in_vec1.f[3])) \
    { \
        ret = true; \
    } \
    else \
    { \
        ret = false; \
    } \


#define RW_MATH_VPU_Abs(ret, in_vec) \
    ret.f[0] = in_vec.f[0] >= 0 ? in_vec.f[0] : -in_vec.f[0]; \
    ret.f[1] = in_vec.f[1] >= 0 ? in_vec.f[1] : -in_vec.f[1]; \
    ret.f[2] = in_vec.f[2] >= 0 ? in_vec.f[2] : -in_vec.f[2]; \
    ret.f[3] = in_vec.f[3] >= 0 ? in_vec.f[3] : -in_vec.f[3]; \



#define RW_MATH_VPU_IsZeroA1V2(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Abs(temp_v, in_vec); \
        if ((temp_v.f[0] <= in_tolerance.f[0]) && \
            (temp_v.f[1] <= in_tolerance.f[1])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA1V3(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Abs(temp_v, in_vec); \
        if ((temp_v.f[0] <= in_tolerance.f[0]) && \
            (temp_v.f[1] <= in_tolerance.f[1]) && \
            (temp_v.f[2] <= in_tolerance.f[2])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA1V4(ret, in_vec, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Abs(temp_v, in_vec); \
        if ((temp_v.f[0] <= in_tolerance.f[0]) && \
            (temp_v.f[1] <= in_tolerance.f[1]) && \
            (temp_v.f[2] <= in_tolerance.f[2]) && \
            (temp_v.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA2V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v0; \
        ::rw::math::vpu::VectorIntrinsic temp_v1; \
        RW_MATH_VPU_Abs(temp_v0, in_vec0); \
        RW_MATH_VPU_Abs(temp_v1, in_vec1); \
        if ((temp_v0.f[0] <= in_tolerance.f[0]) && \
            (temp_v0.f[1] <= in_tolerance.f[1]) && \
            (temp_v0.f[2] <= in_tolerance.f[2]) && \
            (temp_v0.f[3] <= in_tolerance.f[3]) && \
            (temp_v1.f[0] <= in_tolerance.f[0]) && \
            (temp_v1.f[1] <= in_tolerance.f[1]) && \
            (temp_v1.f[2] <= in_tolerance.f[2]) && \
            (temp_v1.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA3V3(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v0; \
        ::rw::math::vpu::VectorIntrinsic temp_v1; \
        ::rw::math::vpu::VectorIntrinsic temp_v2; \
        RW_MATH_VPU_Abs(temp_v0, in_vec0); \
        RW_MATH_VPU_Abs(temp_v1, in_vec1); \
        RW_MATH_VPU_Abs(temp_v2, in_vec2); \
        if ((temp_v0.f[0] <= in_tolerance.f[0]) && \
            (temp_v0.f[1] <= in_tolerance.f[1]) && \
            (temp_v0.f[2] <= in_tolerance.f[2]) && \
            (temp_v1.f[0] <= in_tolerance.f[0]) && \
            (temp_v1.f[1] <= in_tolerance.f[1]) && \
            (temp_v1.f[2] <= in_tolerance.f[2]) && \
            (temp_v2.f[0] <= in_tolerance.f[0]) && \
            (temp_v2.f[1] <= in_tolerance.f[1]) && \
            (temp_v2.f[2] <= in_tolerance.f[2])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA3V4(ret, in_vec0, in_vec1, in_vec2, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v0; \
        ::rw::math::vpu::VectorIntrinsic temp_v1; \
        ::rw::math::vpu::VectorIntrinsic temp_v2; \
        RW_MATH_VPU_Abs(temp_v0, in_vec0); \
        RW_MATH_VPU_Abs(temp_v1, in_vec1); \
        RW_MATH_VPU_Abs(temp_v2, in_vec2); \
        if ((temp_v0.f[0] <= in_tolerance.f[0]) && \
            (temp_v0.f[1] <= in_tolerance.f[1]) && \
            (temp_v0.f[2] <= in_tolerance.f[2]) && \
            (temp_v0.f[3] <= in_tolerance.f[3]) && \
            (temp_v1.f[0] <= in_tolerance.f[0]) && \
            (temp_v1.f[1] <= in_tolerance.f[1]) && \
            (temp_v1.f[2] <= in_tolerance.f[2]) && \
            (temp_v1.f[3] <= in_tolerance.f[3]) && \
            (temp_v2.f[0] <= in_tolerance.f[0]) && \
            (temp_v2.f[1] <= in_tolerance.f[1]) && \
            (temp_v2.f[2] <= in_tolerance.f[2]) && \
            (temp_v2.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA4V3(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v0; \
        ::rw::math::vpu::VectorIntrinsic temp_v1; \
        ::rw::math::vpu::VectorIntrinsic temp_v2; \
        ::rw::math::vpu::VectorIntrinsic temp_v3; \
        RW_MATH_VPU_Abs(temp_v0, in_vec0); \
        RW_MATH_VPU_Abs(temp_v1, in_vec1); \
        RW_MATH_VPU_Abs(temp_v2, in_vec2); \
        RW_MATH_VPU_Abs(temp_v3, in_vec3); \
        if ((temp_v0.f[0] <= in_tolerance.f[0]) && \
            (temp_v0.f[1] <= in_tolerance.f[1]) && \
            (temp_v0.f[2] <= in_tolerance.f[2]) && \
            (temp_v1.f[0] <= in_tolerance.f[0]) && \
            (temp_v1.f[1] <= in_tolerance.f[1]) && \
            (temp_v1.f[2] <= in_tolerance.f[2]) && \
            (temp_v2.f[0] <= in_tolerance.f[0]) && \
            (temp_v2.f[1] <= in_tolerance.f[1]) && \
            (temp_v2.f[2] <= in_tolerance.f[2]) && \
            (temp_v3.f[0] <= in_tolerance.f[0]) && \
            (temp_v3.f[1] <= in_tolerance.f[1]) && \
            (temp_v3.f[2] <= in_tolerance.f[2])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsZeroA4V4(ret, in_vec0, in_vec1, in_vec2, in_vec3, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v0; \
        ::rw::math::vpu::VectorIntrinsic temp_v1; \
        ::rw::math::vpu::VectorIntrinsic temp_v2; \
        ::rw::math::vpu::VectorIntrinsic temp_v3; \
        RW_MATH_VPU_Abs(temp_v0, in_vec0); \
        RW_MATH_VPU_Abs(temp_v1, in_vec1); \
        RW_MATH_VPU_Abs(temp_v2, in_vec2); \
        RW_MATH_VPU_Abs(temp_v3, in_vec3); \
        if ((temp_v0.f[0] <= in_tolerance.f[0]) && \
            (temp_v0.f[1] <= in_tolerance.f[1]) && \
            (temp_v0.f[2] <= in_tolerance.f[2]) && \
            (temp_v0.f[3] <= in_tolerance.f[3]) && \
            (temp_v1.f[0] <= in_tolerance.f[0]) && \
            (temp_v1.f[1] <= in_tolerance.f[1]) && \
            (temp_v1.f[2] <= in_tolerance.f[2]) && \
            (temp_v1.f[3] <= in_tolerance.f[3]) && \
            (temp_v2.f[0] <= in_tolerance.f[0]) && \
            (temp_v2.f[1] <= in_tolerance.f[1]) && \
            (temp_v2.f[2] <= in_tolerance.f[2]) && \
            (temp_v2.f[3] <= in_tolerance.f[3]) && \
            (temp_v3.f[0] <= in_tolerance.f[0]) && \
            (temp_v3.f[1] <= in_tolerance.f[1]) && \
            (temp_v3.f[2] <= in_tolerance.f[2]) && \
            (temp_v3.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA1V2(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Subtract(temp_v, in_vec0, in_vec1); \
        RW_MATH_VPU_Abs(temp_v, temp_v); \
        if ((temp_v.f[0] <= in_tolerance.f[0]) && \
            (temp_v.f[1] <= in_tolerance.f[1])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA1V3(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Subtract(temp_v, in_vec0, in_vec1); \
        RW_MATH_VPU_Abs(temp_v, temp_v); \
        if ((temp_v.f[0] <= in_tolerance.f[0]) && \
            (temp_v.f[1] <= in_tolerance.f[1]) && \
            (temp_v.f[2] <= in_tolerance.f[2])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA1V4(ret, in_vec0, in_vec1, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Subtract(temp_v, in_vec0, in_vec1); \
        RW_MATH_VPU_Abs(temp_v, temp_v); \
        if ((temp_v.f[0] <= in_tolerance.f[0]) && \
            (temp_v.f[1] <= in_tolerance.f[1]) && \
            (temp_v.f[2] <= in_tolerance.f[2]) && \
            (temp_v.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA3V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x; \
        ::rw::math::vpu::VectorIntrinsic temp_y; \
        ::rw::math::vpu::VectorIntrinsic temp_z; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Abs(temp_x, temp_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Abs(temp_y, temp_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Abs(temp_z, temp_z); \
        if ((temp_x.f[0] <= in_tolerance.f[0]) && \
            (temp_x.f[1] <= in_tolerance.f[1]) && \
            (temp_x.f[2] <= in_tolerance.f[2]) && \
            (temp_y.f[0] <= in_tolerance.f[0]) && \
            (temp_y.f[1] <= in_tolerance.f[1]) && \
            (temp_y.f[2] <= in_tolerance.f[2]) && \
            (temp_z.f[0] <= in_tolerance.f[0]) && \
            (temp_z.f[1] <= in_tolerance.f[1]) && \
            (temp_z.f[2] <= in_tolerance.f[2])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA4V3(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x; \
        ::rw::math::vpu::VectorIntrinsic temp_y; \
        ::rw::math::vpu::VectorIntrinsic temp_z; \
        ::rw::math::vpu::VectorIntrinsic temp_w; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Abs(temp_x, temp_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Abs(temp_y, temp_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Abs(temp_z, temp_z); \
        RW_MATH_VPU_Subtract(temp_w, in_vec0_w, in_vec1_w); \
        RW_MATH_VPU_Abs(temp_w, temp_w); \
        if ((temp_x.f[0] <= in_tolerance.f[0]) && \
            (temp_x.f[1] <= in_tolerance.f[1]) && \
            (temp_x.f[2] <= in_tolerance.f[2]) && \
            (temp_y.f[0] <= in_tolerance.f[0]) && \
            (temp_y.f[1] <= in_tolerance.f[1]) && \
            (temp_y.f[2] <= in_tolerance.f[2]) && \
            (temp_z.f[0] <= in_tolerance.f[0]) && \
            (temp_z.f[1] <= in_tolerance.f[1]) && \
            (temp_z.f[2] <= in_tolerance.f[2]) && \
            (temp_w.f[0] <= in_tolerance.f[0]) && \
            (temp_w.f[1] <= in_tolerance.f[1]) && \
            (temp_w.f[2] <= in_tolerance.f[2])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA3V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec1_x, in_vec1_y, in_vec1_z, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x; \
        ::rw::math::vpu::VectorIntrinsic temp_y; \
        ::rw::math::vpu::VectorIntrinsic temp_z; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Abs(temp_x, temp_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Abs(temp_y, temp_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Abs(temp_z, temp_z); \
        if ((temp_x.f[0] <= in_tolerance.f[0]) && \
            (temp_x.f[1] <= in_tolerance.f[1]) && \
            (temp_x.f[2] <= in_tolerance.f[2]) && \
            (temp_x.f[3] <= in_tolerance.f[3]) && \
            (temp_y.f[0] <= in_tolerance.f[0]) && \
            (temp_y.f[1] <= in_tolerance.f[1]) && \
            (temp_y.f[2] <= in_tolerance.f[2]) && \
            (temp_y.f[3] <= in_tolerance.f[3]) && \
            (temp_z.f[0] <= in_tolerance.f[0]) && \
            (temp_z.f[1] <= in_tolerance.f[1]) && \
            (temp_z.f[2] <= in_tolerance.f[2]) && \
            (temp_z.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_IsSimilarA4V4(ret, in_vec0_x, in_vec0_y, in_vec0_z, in_vec0_w, in_vec1_x, in_vec1_y, in_vec1_z, in_vec1_w, in_tolerance) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_x; \
        ::rw::math::vpu::VectorIntrinsic temp_y; \
        ::rw::math::vpu::VectorIntrinsic temp_z; \
        ::rw::math::vpu::VectorIntrinsic temp_w; \
        RW_MATH_VPU_Subtract(temp_x, in_vec0_x, in_vec1_x); \
        RW_MATH_VPU_Abs(temp_x, temp_x); \
        RW_MATH_VPU_Subtract(temp_y, in_vec0_y, in_vec1_y); \
        RW_MATH_VPU_Abs(temp_y, temp_y); \
        RW_MATH_VPU_Subtract(temp_z, in_vec0_z, in_vec1_z); \
        RW_MATH_VPU_Abs(temp_z, temp_z); \
        RW_MATH_VPU_Subtract(temp_w, in_vec0_w, in_vec1_w); \
        RW_MATH_VPU_Abs(temp_w, temp_w); \
        if ((temp_x.f[0] <= in_tolerance.f[0]) && \
            (temp_x.f[1] <= in_tolerance.f[1]) && \
            (temp_x.f[2] <= in_tolerance.f[2]) && \
            (temp_x.f[3] <= in_tolerance.f[3]) && \
            (temp_y.f[0] <= in_tolerance.f[0]) && \
            (temp_y.f[1] <= in_tolerance.f[1]) && \
            (temp_y.f[2] <= in_tolerance.f[2]) && \
            (temp_y.f[3] <= in_tolerance.f[3]) && \
            (temp_z.f[0] <= in_tolerance.f[0]) && \
            (temp_z.f[1] <= in_tolerance.f[1]) && \
            (temp_z.f[2] <= in_tolerance.f[2]) && \
            (temp_z.f[3] <= in_tolerance.f[3]) && \
            (temp_w.f[0] <= in_tolerance.f[0]) && \
            (temp_w.f[1] <= in_tolerance.f[1]) && \
            (temp_w.f[2] <= in_tolerance.f[2]) && \
            (temp_w.f[3] <= in_tolerance.f[3])) \
        { \
            ret = true; \
        } \
        else \
        { \
            ret = false; \
        } \
    } \


#define RW_MATH_VPU_CrossV3(ret, in_vec0, in_vec1) \
        RW_MATH_VPU_CreateVectorV4(ret, \
                        in_vec0.f[1] * in_vec1.f[2] - in_vec0.f[2] * in_vec1.f[1], \
                        in_vec0.f[2] * in_vec1.f[0] - in_vec0.f[0] * in_vec1.f[2], \
                        in_vec0.f[0] * in_vec1.f[1] - in_vec0.f[1] * in_vec1.f[0], \
                        0.0f );


#define RW_MATH_VPU_DotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1]; \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \


#define RW_MATH_VPU_DotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2]; \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \


#define RW_MATH_VPU_DotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2] + temp_v.f[3]; \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \


#define RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1]; \
        sum = rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    }

#define RW_MATH_VPU_SquareRootFastDotV2(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootEstimateDotV2(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV2(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2]; \
        sum = rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    }

#define RW_MATH_VPU_SquareRootFastDotV3(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV3(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2] + temp_v.f[3]; \
        sum = rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    }

#define RW_MATH_VPU_SquareRootFastDotV4(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_SquareRootEstimateDotV4(ret, in_vec0, in_vec1) \
    RW_MATH_VPU_SquareRootDotV4(ret, in_vec0, in_vec1)

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootDotV2(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootDotV3(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \


#define RW_MATH_VPU_ReciprocalSquareRootDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2] + temp_v.f[3]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootEstimateDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2] + temp_v.f[3]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
    } \

#define RW_MATH_VPU_ReciprocalSquareRootFastDotV4(ret, in_vec0, in_vec1) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_v; \
        RW_MATH_VPU_Mult(temp_v, in_vec0, in_vec1); \
        float sum = temp_v.f[0] + temp_v.f[1] + temp_v.f[2] + temp_v.f[3]; \
        sum = 1.0f / rw::math::fpu::detail::Sqrt(sum); \
        RW_MATH_VPU_CreateVectorV4(ret, sum, sum, sum, sum); \
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

#define RW_MATH_VPU_NormalizeV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeFastV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootFastDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
        RW_MATH_VPU_Mult(ret, in_vec, temp_one_over_rqrt_dot); \
    } \

#define RW_MATH_VPU_NormalizeEstimateV4(ret, in_vec) \
    { \
        ::rw::math::vpu::VectorIntrinsic temp_one_over_rqrt_dot; \
        RW_MATH_VPU_ReciprocalSquareRootEstimateDotV4(temp_one_over_rqrt_dot, in_vec, in_vec); \
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
        ::rw::math::vpu::VectorIntrinsic zero; \
        RW_MATH_VPU_GetZero(zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero, input_is_zero); \
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
        ::rw::math::vpu::VectorIntrinsic zero; \
        RW_MATH_VPU_GetZero(zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero, input_is_zero); \
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
        ::rw::math::vpu::VectorIntrinsic zero; \
        RW_MATH_VPU_GetZero(zero); \
        ::rw::math::vpu::VectorIntrinsic input_is_zero; \
        RW_MATH_VPU_IsEqual(input_is_zero, temp_dot, zero); \
        RW_MATH_VPU_Select(ret_mag, result_mag, zero, input_is_zero); \
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
        RW_MATH_VPU_CreateVectorV4(ret, in_vec0.f[in_index_immediate], in_vec0.f[in_index_immediate], in_vec0.f[in_index_immediate], in_vec0.f[in_index_immediate]); \
    } \


#define RW_MATH_VPU_Splat(ret, in_vec0, in_index) \
    { \
        RW_MATH_VPU_CreateVectorV4(ret, in_vec0.f[in_index], in_vec0.f[in_index], in_vec0.f[in_index], in_vec0.f[in_index]); \
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
    }

#define RW_MATH_VPU_TransformPointV3(ret, in_vec, in_x, in_y, in_z, in_w) \
    { \
        ::rw::math::vpu::VectorIntrinsic xSplat, ySplat, zSplat; \
        RW_MATH_VPU_Splat_I( xSplat, in_vec, 0 ); \
        RW_MATH_VPU_Splat_I( ySplat, in_vec, 1 ); \
        RW_MATH_VPU_Splat_I( zSplat, in_vec, 2 ); \
        RW_MATH_VPU_MultAdd( ret, in_x, xSplat, in_w ); \
        RW_MATH_VPU_MultAdd( ret, in_y, ySplat, ret ); \
        RW_MATH_VPU_MultAdd( ret, in_z, zSplat, ret ); \
    }

#define RW_MATH_VPU_ExtractXElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    RW_MATH_VPU_CreateVectorV3(ret, in_x.f[0], in_y.f[0], in_z.f[0]);

#define RW_MATH_VPU_ExtractYElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    RW_MATH_VPU_CreateVectorV3(ret, in_x.f[1], in_y.f[1], in_z.f[1]);

#define RW_MATH_VPU_ExtractZElementsAsVectorA3V3(ret, in_x, in_y, in_z) \
    RW_MATH_VPU_CreateVectorV3(ret, in_x.f[2], in_y.f[2], in_z.f[2]);

#define RW_MATH_VPU_ExtractXElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    RW_MATH_VPU_CreateVectorV4(ret, in_x.f[0], in_y.f[0], in_z.f[0], in_w.f[0]);

#define RW_MATH_VPU_ExtractYElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    RW_MATH_VPU_CreateVectorV4(ret, in_x.f[1], in_y.f[1], in_z.f[1], in_w.f[1]);

#define RW_MATH_VPU_ExtractZElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    RW_MATH_VPU_CreateVectorV4(ret, in_x.f[2], in_y.f[2], in_z.f[2], in_w.f[2]);

#define RW_MATH_VPU_ExtractWElementsAsVectorA4V4(ret, in_x, in_y, in_z, in_w) \
    RW_MATH_VPU_CreateVectorV4(ret, in_x.f[3], in_y.f[3], in_z.f[3], in_w.f[3]);

#define RW_MATH_VPU_TransposeA3V3(ret_x, ret_y, ret_z, in_x, in_y, in_z) \
    RW_MATH_VPU_CreateVectorV3(ret_x, in_x.f[0], in_y.f[0], in_z.f[0]); \
    RW_MATH_VPU_CreateVectorV3(ret_y, in_x.f[1], in_y.f[1], in_z.f[1]); \
    RW_MATH_VPU_CreateVectorV3(ret_z, in_x.f[2], in_y.f[2], in_z.f[2]); \


#define RW_MATH_VPU_TransposeA4V4(ret_x, ret_y, ret_z, ret_w, in_x, in_y, in_z, in_w) \
    RW_MATH_VPU_CreateVectorV4(ret_x, in_x.f[0], in_y.f[0], in_z.f[0], in_w.f[0]); \
    RW_MATH_VPU_CreateVectorV4(ret_y, in_x.f[1], in_y.f[1], in_z.f[1], in_w.f[1]); \
    RW_MATH_VPU_CreateVectorV4(ret_z, in_x.f[2], in_y.f[2], in_z.f[2], in_w.f[2]); \
    RW_MATH_VPU_CreateVectorV4(ret_w, in_x.f[3], in_y.f[3], in_z.f[3], in_w.f[3]); \

#define RW_MATH_VPU_TransposeA4V3(ret_x, ret_y, ret_z, in_x, in_y, in_z, in_w) \
    RW_MATH_VPU_CreateVectorV4(ret_x, in_x.f[0], in_y.f[0], in_z.f[0], in_w.f[0]); \
    RW_MATH_VPU_CreateVectorV4(ret_y, in_x.f[1], in_y.f[1], in_z.f[1], in_w.f[1]); \
    RW_MATH_VPU_CreateVectorV4(ret_z, in_x.f[2], in_y.f[2], in_z.f[2], in_w.f[2]);


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
        using namespace ::rw::math::ps::detail; \
        mw_ps in_x_ps0 = in_x.ps0; \
        mw_ps in_x_ps1 = in_x.ps1; \
        mw_ps in_y_ps0 = in_y.ps0; \
        mw_ps in_y_ps1 = in_y.ps1; \
        mw_ps in_z_ps0 = in_z.ps0; \
        mw_ps in_z_ps1 = in_z.ps1; \
        mw_ps in_w_ps0 = in_w.ps0; \
        mw_ps in_w_ps1 = in_w.ps1; \
        mw_ps res_x_ps0, res_x_ps1, res_y_ps0, res_y_ps1, res_z_ps0, res_z_ps1, res_w_ps0, res_w_ps1, det_ps; \
        RW_MATH_PS_InverseA4V3(ret_d, res_x_ps0, res_x_ps1, res_y_ps0, res_y_ps1, res_z_ps0, res_z_ps1, res_w_ps0, res_w_ps1, in_x_ps0, in_x_ps1, in_y_ps0, in_y_ps1, in_z_ps0, in_z_ps1, in_w_ps0, in_w_ps1 ) \
        ret_x.ps0 = res_x_ps0; \
        ret_x.ps1 = res_x_ps1; \
        ret_y.ps0 = res_y_ps0; \
        ret_y.ps1 = res_y_ps1; \
        ret_z.ps0 = res_z_ps0; \
        ret_z.ps1 = res_z_ps1; \
        ret_w.ps0 = res_w_ps0; \
        ret_w.ps1 = res_w_ps1; \
        ret_d.ps0 = det_ps; \
        ret_d.ps1 = det_ps; \
    }


//----------------------------------------------------------------------------
#define RW_MATH_VPU_MatrixFromQuaternion(ret_x, ret_y, ret_z, in_q) \
    { \
        float temp_mfq_xx = in_q.f[0] * in_q.f[0]; \
        float temp_mfq_yy = in_q.f[1] * in_q.f[1]; \
        float temp_mfq_zz = in_q.f[2] * in_q.f[2]; \
        float temp_mfq_xy = in_q.f[0] * in_q.f[1]; \
        float temp_mfq_xz = in_q.f[0] * in_q.f[2]; \
        float temp_mfq_yz = in_q.f[1] * in_q.f[2]; \
        float temp_mfq_wx = in_q.f[3] * in_q.f[0]; \
        float temp_mfq_wy = in_q.f[3] * in_q.f[1]; \
        float temp_mfq_wz = in_q.f[3] * in_q.f[2]; \
        const float temp_mfq_one = 1.0f; \
        const float temp_mfq_two = 2.0f; \
        RW_MATH_VPU_CreateVectorV3( ret_x, temp_mfq_one - temp_mfq_two*(temp_mfq_yy + temp_mfq_zz),                temp_mfq_two * (temp_mfq_xy + temp_mfq_wz),              temp_mfq_two * (temp_mfq_xz - temp_mfq_wy) ); \
        RW_MATH_VPU_CreateVectorV3( ret_y,                temp_mfq_two*(temp_mfq_xy - temp_mfq_wz), temp_mfq_one - temp_mfq_two * (temp_mfq_xx + temp_mfq_zz),              temp_mfq_two * (temp_mfq_yz + temp_mfq_wx) ); \
        RW_MATH_VPU_CreateVectorV3( ret_z,                temp_mfq_two*(temp_mfq_xz + temp_mfq_wy),                temp_mfq_two * (temp_mfq_yz - temp_mfq_wx), temp_mfq_one-temp_mfq_two * (temp_mfq_xx + temp_mfq_yy) ); \
    } \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Min(ret, in_vec0, in_vec1) \
    ret.f[0] = (in_vec0.f[0] < in_vec1.f[0]) ? in_vec0.f[0] : in_vec1.f[0]; \
    ret.f[1] = (in_vec0.f[1] < in_vec1.f[1]) ? in_vec0.f[1] : in_vec1.f[1]; \
    ret.f[2] = (in_vec0.f[2] < in_vec1.f[2]) ? in_vec0.f[2] : in_vec1.f[2]; \
    ret.f[3] = (in_vec0.f[3] < in_vec1.f[3]) ? in_vec0.f[3] : in_vec1.f[3]; \


//----------------------------------------------------------------------------
#define RW_MATH_VPU_Max(ret, in_vec0, in_vec1) \
    ret.f[0] = (in_vec0.f[0] > in_vec1.f[0]) ? in_vec0.f[0] : in_vec1.f[0]; \
    ret.f[1] = (in_vec0.f[1] > in_vec1.f[1]) ? in_vec0.f[1] : in_vec1.f[1]; \
    ret.f[2] = (in_vec0.f[2] > in_vec1.f[2]) ? in_vec0.f[2] : in_vec1.f[2]; \
    ret.f[3] = (in_vec0.f[3] > in_vec1.f[3]) ? in_vec0.f[3] : in_vec1.f[3]; \

#define RW_MATH_VPU_ClampToNegPosRange(ret, in_vec0, in_vec1) \
    ret.f[0] = (in_vec0.f[0] > in_vec1.f[0]) ? in_vec1.f[0] : ( (in_vec0.f[0] < -in_vec1.f[0]) ? -in_vec1.f[0] : in_vec0.f[0]); \
    ret.f[1] = (in_vec0.f[1] > in_vec1.f[1]) ? in_vec1.f[1] : ( (in_vec0.f[1] < -in_vec1.f[1]) ? -in_vec1.f[1] : in_vec0.f[1]); \
    ret.f[2] = (in_vec0.f[2] > in_vec1.f[2]) ? in_vec1.f[2] : ( (in_vec0.f[2] < -in_vec1.f[2]) ? -in_vec1.f[2] : in_vec0.f[2]); \
    ret.f[3] = (in_vec0.f[3] > in_vec1.f[3]) ? in_vec1.f[3] : ( (in_vec0.f[3] < -in_vec1.f[3]) ? -in_vec1.f[3] : in_vec0.f[3]); \

inline ::rw::math::vpu::VectorIntrinsic RWMATH_VPU_DETAIL_VecMergeH( VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = a.f[0];
    r.f[1] = b.f[0];
    r.f[2] = a.f[1];
    r.f[3] = b.f[1];
    return r;
}

inline ::rw::math::vpu::VectorIntrinsic RWMATH_VPU_DETAIL_VecMergeL( VectorIntrinsicInParam a, VectorIntrinsicInParam b);
inline ::rw::math::vpu::VectorIntrinsic RWMATH_VPU_DETAIL_VecMergeL( VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = a.f[2];
    r.f[1] = b.f[2];
    r.f[2] = a.f[3];
    r.f[3] = b.f[3];
    return r;
}


//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_CreateVectorFromScalar_R(float f0)
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = f0;
    r.f[1] = f0;
    r.f[2] = f0;
    r.f[3] = f0;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetZero_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = 0.f;
    r.f[1] = 0.f;
    r.f[2] = 0.f;
    r.f[3] = 0.f;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetOne_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = 1.f;
    r.f[1] = 1.f;
    r.f[2] = 1.f;
    r.f[3] = 1.f;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetHalf_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = 0.5f;
    r.f[1] = 0.5f;
    r.f[2] = 0.5f;
    r.f[3] = 0.5f;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetNegHalf_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = -0.5f;
    r.f[1] = -0.5f;
    r.f[2] = -0.5f;
    r.f[3] = -0.5f;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetTwo_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = 2.f;
    r.f[1] = 2.f;
    r.f[2] = 2.f;
    r.f[3] = 2.f;
    return r;
}

inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetNegTwo_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = -2.f;
    r.f[1] = -2.f;
    r.f[2] = -2.f;
    r.f[3] = -2.f;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetNegOne_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = -1.f;
    r.f[1] = -1.f;
    r.f[2] = -1.f;
    r.f[3] = -1.f;
    return r;
}

inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetTrue_R()
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.i[0] = 0xffffffff;
    r.i[1] = 0xffffffff;
    r.i[2] = 0xffffffff;
    r.i[3] = 0xffffffff;
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetI_R() { ::rw::math::vpu::VectorIntrinsic r; r.f[0] = 1.f; r.f[1] = 0.f; r.f[2] = 0.f; r.f[3] = 0.f; return r; }
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetJ_R() { ::rw::math::vpu::VectorIntrinsic r; r.f[0] = 0.f; r.f[1] = 1.f; r.f[2] = 0.f; r.f[3] = 0.f; return r; }
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetK_R() { ::rw::math::vpu::VectorIntrinsic r; r.f[0] = 0.f; r.f[1] = 0.f; r.f[2] = 1.f; r.f[3] = 0.f; return r; }
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_GetL_R() { ::rw::math::vpu::VectorIntrinsic r; r.f[0] = 0.f; r.f[1] = 0.f; r.f[2] = 0.f; r.f[3] = 1.f; return r; }

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_Add_R(VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = a.f[0] + b.f[0];
    r.f[1] = a.f[1] + b.f[1];
    r.f[2] = a.f[2] + b.f[2];
    r.f[3] = a.f[3] + b.f[3];
    return r;
}

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_Subtract_R(VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    ::rw::math::vpu::VectorIntrinsic r;
    r.f[0] = a.f[0] - b.f[0];
    r.f[1] = a.f[1] - b.f[1];
    r.f[2] = a.f[2] - b.f[2];
    r.f[3] = a.f[3] - b.f[3];
    return r;
}

//----------------------------------------------------------------------------
inline void RW_MATH_VPU_SetXToZero(::rw::math::vpu::VectorIntrinsic& ret, VectorIntrinsicInParam v) { ret = v; ret.f[0] = 0.f; }
inline void RW_MATH_VPU_SetYToZero(::rw::math::vpu::VectorIntrinsic& ret, VectorIntrinsicInParam v) { ret = v; ret.f[1] = 0.f; }
inline void RW_MATH_VPU_SetZToZero(::rw::math::vpu::VectorIntrinsic& ret, VectorIntrinsicInParam v) { ret = v; ret.f[2] = 0.f; }
inline void RW_MATH_VPU_SetWToZero(::rw::math::vpu::VectorIntrinsic& ret, VectorIntrinsicInParam v) { ret = v; ret.f[3] = 0.f; }

//----------------------------------------------------------------------------
inline void RW_MATH_VPU_NormalizeFastV3(::rw::math::vpu::VectorIntrinsic& ret, VectorIntrinsicInParam v) { RW_MATH_VPU_NormalizeV3(ret, v) }

//----------------------------------------------------------------------------
inline ::rw::math::vpu::VectorIntrinsic RW_MATH_VPU_Select_R(VectorIntrinsicInParam a, VectorIntrinsicInParam b, VectorIntrinsicInParam mask)
{
    VectorIntrinsic r;
    r.i[0] = (a.i[0] & ~mask.i[0]) | (b.i[0] & mask.i[0]);
    r.i[1] = (a.i[1] & ~mask.i[1]) | (b.i[1] & mask.i[1]);
    r.i[2] = (a.i[2] & ~mask.i[2]) | (b.i[2] & mask.i[2]);
    r.i[3] = (a.i[3] & ~mask.i[3]) | (b.i[3] & mask.i[3]);
    return r;
}

//----------------------------------------------------------------------------
#define RW_MATH_VPU_Compare_R_Impl(_name_, _op_) \
    inline ::rw::math::vpu::VectorIntrinsic _name_(VectorIntrinsicInParam a, VectorIntrinsicInParam b) \
    { \
        VectorIntrinsic r; \
        r.i[0] = a.f[0] _op_ b.f[0] ? 0xffffffffU : 0; \
        r.i[1] = a.f[1] _op_ b.f[1] ? 0xffffffffU : 0; \
        r.i[2] = a.f[2] _op_ b.f[2] ? 0xffffffffU : 0; \
        r.i[3] = a.f[3] _op_ b.f[3] ? 0xffffffffU : 0; \
        return r; \
    }
    
RW_MATH_VPU_Compare_R_Impl(RW_MATH_VPU_IsEqual_R, ==)
RW_MATH_VPU_Compare_R_Impl(RW_MATH_VPU_IsGreater_R, >)
RW_MATH_VPU_Compare_R_Impl(RW_MATH_VPU_IsGreaterEqual_R, >=)

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
        rw::math::vpu::detail::VectorIntrinsicUnion in_vecU; \
        in_vecU.vector = (in_vec); \
        ret = in_vecU.scalarUInt[0] || in_vecU.scalarUInt[1] || in_vecU.scalarUInt[2] || in_vecU.scalarUInt[3]; \
    }

#define RW_MATH_VPU_IsAllTrue(ret, in_vec) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion in_vecU; \
        in_vecU.vector = (in_vec); \
        ret = in_vecU.scalarUInt[0] && in_vecU.scalarUInt[1] && in_vecU.scalarUInt[2] && in_vecU.scalarUInt[3]; \
    }

#define RW_MATH_VPU_IsAnyFalse(ret, in_vec) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion in_vecU; \
        in_vecU.vector = (in_vec); \
        ret = !(in_vecU.scalarUInt[0] && in_vecU.scalarUInt[1] && in_vecU.scalarUInt[2] && in_vecU.scalarUInt[3]); \
    }

#define RW_MATH_VPU_IsAllFalse(ret, in_vec) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion in_vecU; \
        in_vecU.vector = (in_vec); \
        ret = !(in_vecU.scalarUInt[0] || in_vecU.scalarUInt[1] || in_vecU.scalarUInt[2] || in_vecU.scalarUInt[3]); \
    }

#define RW_MATH_VPU_PermuteSingle_I(ret, in_vec, pattern) \
    { \
        rw::math::vpu::detail::VectorIntrinsicUnion in_vecU, retU; \
        in_vecU.vector = (in_vec); \
        retU.scalarFloat[0] = in_vecU.scalarFloat[(pattern&0xf000)>>12]; \
        retU.scalarFloat[1] = in_vecU.scalarFloat[(pattern&0x0f00)>> 8]; \
        retU.scalarFloat[2] = in_vecU.scalarFloat[(pattern&0x00f0)>> 4]; \
        retU.scalarFloat[3] = in_vecU.scalarFloat[(pattern&0x000f)>> 0]; \
        ret = retU.vector; \
    }

} // namespace vpu
} // namespace math
} // namespace rw

#endif
