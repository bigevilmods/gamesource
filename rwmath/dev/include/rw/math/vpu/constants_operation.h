// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_CONSTANTS_OPERATION_H
#define RW_MATH_VPU_CONSTANTS_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"

namespace rw
{
namespace math
{
namespace vpu
{

    class Vector2;
    class Vector3;
    class Vector4;
    class Quaternion;
    class QuatPos;
    class Matrix33;
    class Matrix44;
    class Matrix44Affine;
    class Matrix44AffinePacked;

RW_MATH_FORCE_INLINE MaskScalar GetMaskScalar_True();
RW_MATH_FORCE_INLINE Mask2 GetMask2_True();
RW_MATH_FORCE_INLINE Mask3 GetMask3_True();
RW_MATH_FORCE_INLINE Mask4 GetMask4_True();
RW_MATH_FORCE_INLINE MaskScalar GetMaskScalar_False();
RW_MATH_FORCE_INLINE Mask2 GetMask2_False();
RW_MATH_FORCE_INLINE Mask3 GetMask3_False();
RW_MATH_FORCE_INLINE Mask4 GetMask4_False();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeTwo();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeOne();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeHalf();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_NegativeZero();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_Zero();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_Half();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_One();
RW_MATH_FORCE_INLINE VecFloat GetVecFloat_Two();

RW_MATH_FORCE_INLINE Vector2 GetVector2_Zero();
RW_MATH_FORCE_INLINE Vector2 GetVector2_One();
RW_MATH_FORCE_INLINE Vector2 GetVector2_XAxis();
RW_MATH_FORCE_INLINE Vector2 GetVector2_YAxis();

RW_MATH_FORCE_INLINE Vector3 GetVector3_Zero();
RW_MATH_FORCE_INLINE Vector3 GetVector3_One();
RW_MATH_FORCE_INLINE Vector3 GetVector3_XAxis();
RW_MATH_FORCE_INLINE Vector3 GetVector3_YAxis();
RW_MATH_FORCE_INLINE Vector3 GetVector3_ZAxis();

RW_MATH_FORCE_INLINE Vector4 GetVector4_Zero();
RW_MATH_FORCE_INLINE Vector4 GetVector4_One();
RW_MATH_FORCE_INLINE Vector4 GetVector4_Origin();
RW_MATH_FORCE_INLINE Vector4 GetVector4_XAxis();
RW_MATH_FORCE_INLINE Vector4 GetVector4_YAxis();
RW_MATH_FORCE_INLINE Vector4 GetVector4_ZAxis();
RW_MATH_FORCE_INLINE Vector4 GetVector4_WAxis();

RW_MATH_FORCE_INLINE Matrix33 GetMatrix33_Identity();
RW_MATH_FORCE_INLINE Matrix33 GetMatrix33_Zero();

RW_MATH_FORCE_INLINE Matrix44 GetMatrix44_Identity();
RW_MATH_FORCE_INLINE Matrix44 GetMatrix44_Zero();

RW_MATH_FORCE_INLINE Matrix44Affine GetMatrix44Affine_Identity();
RW_MATH_FORCE_INLINE Matrix44Affine GetMatrix44Affine_Zero();

RW_MATH_FORCE_INLINE Matrix44AffinePacked GetMatrix44AffinePacked_Identity();
RW_MATH_FORCE_INLINE Matrix44AffinePacked GetMatrix44AffinePacked_Zero();

RW_MATH_FORCE_INLINE Quaternion GetQuaternion_Identity();
RW_MATH_FORCE_INLINE Quaternion GetQuaternion_Zero();
RW_MATH_FORCE_INLINE Quaternion GetQuaternion_One();

RW_MATH_FORCE_INLINE QuatPos GetQuatPos_Identity();
RW_MATH_FORCE_INLINE QuatPos GetQuatPos_Zero();

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/constants_operation_inline.h"
#endif

#endif
