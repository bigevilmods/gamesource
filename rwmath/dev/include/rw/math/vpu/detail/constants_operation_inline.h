// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CONSTANTS_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_CONSTANTS_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask2.h"
#include "rw/math/vpu/mask3.h"
#include "rw/math/vpu/mask4.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/vector2.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/quatpos.h"
#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/matrix44affinepacked.h"

namespace rw
{
namespace math
{
namespace vpu
{

//
// Mask Constants
//
RW_MATH_FORCE_INLINE MaskScalar GetMaskScalar_True()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetTrue(result);
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE Mask2 GetMask2_True()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetTrue(result);
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask3 GetMask3_True()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetTrue(result);
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask4 GetMask4_True()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetTrue(result);
    return Mask4(result);
}

RW_MATH_FORCE_INLINE MaskScalar GetMaskScalar_False()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetZero(result);
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE Mask2 GetMask2_False()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetZero(result);
    return Mask2(result);
}

RW_MATH_FORCE_INLINE Mask3 GetMask3_False()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetZero(result);
    return Mask3(result);
}

RW_MATH_FORCE_INLINE Mask4 GetMask4_False()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetZero(result);
    return Mask4(result);
}


//
// VecFloat Constants
//
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeTwo()
{
    return VecFloat(RW_MATH_VPU_GetNegTwo_R());
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeOne()
{
    return VecFloat(RW_MATH_VPU_GetNegOne_R());
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeHalf()
{
    return VecFloat(RW_MATH_VPU_GetNegHalf_R());
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeZero()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetSignMaskXYZW(result);
    return VecFloat(result);
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_Zero()
{
    return VecFloat(RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_Half()
{
    return VecFloat(RW_MATH_VPU_GetHalf_R());
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_One()
{
    return VecFloat(RW_MATH_VPU_GetOne_R());
}

RW_MATH_FORCE_INLINE VecFloat GetVecFloat_Two()
{
    return VecFloat(RW_MATH_VPU_GetTwo_R());
}

//
// Vector2 Constants
//
RW_MATH_FORCE_INLINE Vector2 GetVector2_Zero()
{
    return Vector2(RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Vector2 GetVector2_One()
{
    return Vector2(RW_MATH_VPU_GetOne_R());
}

RW_MATH_FORCE_INLINE Vector2 GetVector2_XAxis()
{
    return Vector2(RW_MATH_VPU_GetI_R());
}

RW_MATH_FORCE_INLINE Vector2 GetVector2_YAxis()
{
    return Vector2(RW_MATH_VPU_GetJ_R());
}

//
// Vector3 Constants
//
RW_MATH_FORCE_INLINE Vector3 GetVector3_Zero()
{
    return Vector3(RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Vector3 GetVector3_One()
{
    return Vector3(RW_MATH_VPU_GetOne_R());
}

RW_MATH_FORCE_INLINE Vector3 GetVector3_XAxis()
{
    return Vector3(RW_MATH_VPU_GetI_R());
}

RW_MATH_FORCE_INLINE Vector3 GetVector3_YAxis()
{
    return Vector3(RW_MATH_VPU_GetJ_R());
}

RW_MATH_FORCE_INLINE Vector3 GetVector3_ZAxis()
{
    return Vector3(RW_MATH_VPU_GetK_R());
}

//
// Vector4 Constants
//
RW_MATH_FORCE_INLINE Vector4 GetVector4_Zero()
{
    return Vector4(RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Vector4 GetVector4_One()
{
    return Vector4(RW_MATH_VPU_GetOne_R());
}

RW_MATH_FORCE_INLINE Vector4 GetVector4_Origin()
{
    return Vector4(RW_MATH_VPU_GetL_R());
}

RW_MATH_FORCE_INLINE Vector4 GetVector4_XAxis()
{
    return Vector4(RW_MATH_VPU_GetI_R());
}

RW_MATH_FORCE_INLINE Vector4 GetVector4_YAxis()
{
    return Vector4(RW_MATH_VPU_GetJ_R());
}

RW_MATH_FORCE_INLINE Vector4 GetVector4_ZAxis()
{
    return Vector4(RW_MATH_VPU_GetK_R());
}

RW_MATH_FORCE_INLINE Vector4 GetVector4_WAxis()
{
    return Vector4(RW_MATH_VPU_GetL_R());
}

//
// Matrix33 Constants
//
RW_MATH_FORCE_INLINE Matrix33 GetMatrix33_Identity()
{
    return Matrix33(RW_MATH_VPU_GetI_R(), RW_MATH_VPU_GetJ_R(), RW_MATH_VPU_GetK_R());
}

RW_MATH_FORCE_INLINE Matrix33 GetMatrix33_Zero()
{
    return Matrix33(RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R());
}

//
// Matrix44 Constants
//
RW_MATH_FORCE_INLINE Matrix44 GetMatrix44_Identity()
{
    return Matrix44(RW_MATH_VPU_GetI_R(), RW_MATH_VPU_GetJ_R(), RW_MATH_VPU_GetK_R(), RW_MATH_VPU_GetL_R());
}

RW_MATH_FORCE_INLINE Matrix44 GetMatrix44_Zero()
{
    return Matrix44(RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R());
}

//
// Matrix44Affine Constants
//
RW_MATH_FORCE_INLINE Matrix44Affine GetMatrix44Affine_Identity()
{
    return Matrix44Affine(RW_MATH_VPU_GetI_R(), RW_MATH_VPU_GetJ_R(), RW_MATH_VPU_GetK_R(), RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Matrix44Affine GetMatrix44Affine_Zero()
{
    return Matrix44Affine(RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R());
}

//
// Matrix44AffinePacked Constants
//
RW_MATH_FORCE_INLINE Matrix44AffinePacked GetMatrix44AffinePacked_Identity() 
{
    return Matrix44AffinePacked(RW_MATH_VPU_GetI_R(), RW_MATH_VPU_GetJ_R(), RW_MATH_VPU_GetK_R(), RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked GetMatrix44AffinePacked_Zero() 
{
    return Matrix44AffinePacked( RW_MATH_VPU_GetZero_R(),
                                 RW_MATH_VPU_GetZero_R(),
                                 RW_MATH_VPU_GetZero_R(),
                                 RW_MATH_VPU_GetZero_R() );
}

//
// Quaternion Constants
//
RW_MATH_FORCE_INLINE Quaternion GetQuaternion_Identity()
{
    return Quaternion(RW_MATH_VPU_GetL_R());
}

RW_MATH_FORCE_INLINE Quaternion GetQuaternion_Zero()
{
    return Quaternion(RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Quaternion GetQuaternion_One()
{
    return Quaternion(RW_MATH_VPU_GetOne_R());
}

RW_MATH_FORCE_INLINE QuatPos GetQuatPos_Identity()
{
    return QuatPos(RW_MATH_VPU_GetL_R(), RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE QuatPos GetQuatPos_Zero()
{
    return QuatPos(RW_MATH_VPU_GetZero_R(), RW_MATH_VPU_GetZero_R());
}

} //namespace vpu
} //namespace math
} //namespace rw

#endif
