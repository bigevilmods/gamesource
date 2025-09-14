// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_CONSTANTS_OPERATION_H
#define RW_MATH_FPU_CONSTANTS_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/maskscalar.h"
#include "rw/math/fpu/mask2.h"
#include "rw/math/fpu/mask3.h"
#include "rw/math/fpu/mask4.h"
#include "rw/math/fpu/vector2.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/quatpos.h"
#include "rw/math/fpu/matrix33.h"
#include "rw/math/fpu/matrix44.h"
#include "rw/math/fpu/matrix44affine.h"

namespace rw
{
namespace math
{
namespace fpu
{

template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> GetMaskScalar_TrueTemplate()
{
    return MaskScalarTemplate<T>(static_cast<T>(1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> GetMask2_TrueTemplate()
{
    return Mask2Template<T>(static_cast<T>(1.0f), static_cast<T>(1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> GetMask3_TrueTemplate()
{
    return Mask3Template<T>(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> GetMask4_TrueTemplate()
{
    return Mask4Template<T>(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> GetMaskScalar_FalseTemplate()
{
    return MaskScalarTemplate<T>(static_cast<T>(-1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> GetMask2_FalseTemplate()
{
    return Mask2Template<T>(static_cast<T>(-1.0f), static_cast<T>(-1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> GetMask3_FalseTemplate()
{
    return Mask3Template<T>(static_cast<T>(-1.0f), static_cast<T>(-1.0f), static_cast<T>(-1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> GetMask4_FalseTemplate()
{
    return Mask4Template<T>(static_cast<T>(-1.0f), static_cast<T>(-1.0f), static_cast<T>(-1.0f), static_cast<T>(-1.0f));
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_NegativeTwoTemplate()
{
    return static_cast<T>(-2.0f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_NegativeOneTemplate()
{
    return static_cast<T>(-1.0f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_NegativeHalfTemplate()
{
    return static_cast<T>(-0.5f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_NegativeZeroTemplate()
{
    return static_cast<T>(-0.0f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_ZeroTemplate()
{
    return static_cast<T>(0.0f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_HalfTemplate()
{
    return static_cast<T>(0.5f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_OneTemplate()
{
    return static_cast<T>(1.0f);
}

template<class T>
RW_MATH_FORCE_INLINE T GetVecFloat_TwoTemplate()
{
    return static_cast<T>(2.0f);
}

/*
Get a Vector2 with all elements equal to zero

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> GetVector2_ZeroTemplate()
{
    return Vector2Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Vector2 with all elements equal to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> GetVector2_OneTemplate()
{
    return Vector2Template<T>(static_cast<T>(1.0f), static_cast<T>(1.0f));
}

/*
Get a Vector2 with all elements equal to zero but with X component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> GetVector2_XAxisTemplate()
{
    return Vector2Template<T>(static_cast<T>(1.0f), static_cast<T>(0.0f));
}

/*
Get a Vector2 with all elements equal to zero but with Y component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> GetVector2_YAxisTemplate()
{
    return Vector2Template<T>(static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get a Vector3 with all elements equal to zero

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> GetVector3_ZeroTemplate()
{
    return Vector3Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Vector3 with all elements equal to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> GetVector3_OneTemplate()
{
    return Vector3Template<T>(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f));
}

/*
Get a Vector3 with all elements equal to zero but with X component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> GetVector3_XAxisTemplate()
{
    return Vector3Template<T>(static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Vector3 with all elements equal to zero but with Y component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> GetVector3_YAxisTemplate()
{
    return Vector3Template<T>(static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f));
}

/*
Get a Vector3 with all elements equal to zero but with Z component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> GetVector3_ZAxisTemplate()
{
    return Vector3Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get a Vector4 with all elements equal to zero

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_ZeroTemplate()
{
    return Vector4Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Vector4 with all elements equal to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_OneTemplate()
{
    return Vector4Template<T>(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f));
}

/*
Get a Vector4 with all elements equal to zero but with W component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_OriginTemplate()
{
    return Vector4Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get a Vector4 with all elements equal to zero but with X component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_XAxisTemplate()
{
    return Vector4Template<T>(static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Vector4 with all elements equal to zero but with Y component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_YAxisTemplate()
{
    return Vector4Template<T>(static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Vector4 with all elements equal to zero but with Z component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_ZAxisTemplate()
{
    return Vector4Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f));
}

/*
Get a Vector4 with all elements equal to zero but with W component set to one

\return The resulting vector
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> GetVector4_WAxisTemplate()
{
    return Vector4Template<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get a 3x3 identity matrix

\return The resulting matrix
*/
template<class T>
Matrix33Template<T> GetMatrix33_IdentityTemplate()
{
    return Matrix33Template<T>(
        static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get a 3x3 matrix with all elements equal to zero

\return The resulting matrix
*/
template<class T>
Matrix33Template<T> GetMatrix33_ZeroTemplate()
{
    return Matrix33Template<T>(
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a 4x4 identity matrix

\return The resulting matrix
*/
template<class T>
Matrix44Template<T> GetMatrix44_IdentityTemplate()
{
    return Matrix44Template<T>(
        static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get a 4x4 matrix with all elements equal to zero

\return The resulting matrix
*/
template<class T>
Matrix44Template<T> GetMatrix44_ZeroTemplate()
{
    return Matrix44Template<T>(
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Matrix44Affine identity matrix

\return The resulting matrix
*/
template<class T>
Matrix44AffineTemplate<T> GetMatrix44Affine_IdentityTemplate()
{
    return Matrix44AffineTemplate<T>(
        static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a Matrix44Affine with all elements equal to zero

\return The resulting matrix
*/
template<class T>
Matrix44AffineTemplate<T> GetMatrix44Affine_ZeroTemplate()
{
    return Matrix44AffineTemplate<T>(
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

template<class T>
Matrix44AffinePackedTemplate<T> GetMatrix44AffinePacked_IdentityTemplate()
{
    return Matrix44AffinePackedTemplate<T>(
        static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

template<class T>
Matrix44AffinePackedTemplate<T> GetMatrix44AffinePacked_ZeroTemplate()
{
    return Matrix44AffinePackedTemplate<T>(
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
        static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get an identity quaternion

\return The resulting quaternion
*/
template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> GetQuaternion_IdentityTemplate()
{
    return QuaternionTemplate<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
}

/*
Get an quaternion with all elements equal to zero

\return The resulting quaternion
*/
template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> GetQuaternion_ZeroTemplate()
{
    return QuaternionTemplate<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get an quaternion with all elements equal to one

\return The resulting quaternion
*/
template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> GetQuaternion_OneTemplate()
{
    return QuaternionTemplate<T>(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f));
}

/*
Get a QuatPos with an identity quaternion and position = (0,0,0)

\return The resulting QuatPos
*/
template<class T>
RW_MATH_FORCE_INLINE QuatPosTemplate<T> GetQuatPos_IdentityTemplate()
{
    return QuatPosTemplate<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}

/*
Get a QuatPos with all the elements equal to zero

\return The resulting QuatPos
*/
template<class T>
RW_MATH_FORCE_INLINE QuatPosTemplate<T> GetQuatPos_ZeroTemplate()
{
    return QuatPosTemplate<T>(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
}


//----------------------------------------------------------------------------

RW_MATH_FORCE_INLINE MaskScalarTemplate<float> GetMaskScalar_True()
{
    return MaskScalarTemplate<float>(1.0f);
}

RW_MATH_FORCE_INLINE Mask2Template<float> GetMask2_True()
{
    return Mask2Template<float>(1.0f, 1.0f);
}

RW_MATH_FORCE_INLINE Mask3Template<float> GetMask3_True()
{
    return Mask3Template<float>(1.0f, 1.0f, 1.0f);
}

RW_MATH_FORCE_INLINE Mask4Template<float> GetMask4_True()
{
    return Mask4Template<float>(1.0f, 1.0f, 1.0f, 1.0f);
}

RW_MATH_FORCE_INLINE MaskScalarTemplate<float> GetMaskScalar_False()
{
    return MaskScalarTemplate<float>(-1.0f);
}

RW_MATH_FORCE_INLINE Mask2Template<float> GetMask2_False()
{
    return Mask2Template<float>(-1.0f, -1.0f);
}

RW_MATH_FORCE_INLINE Mask3Template<float> GetMask3_False()
{
    return Mask3Template<float>(-1.0f, -1.0f, -1.0f);
}

RW_MATH_FORCE_INLINE Mask4Template<float> GetMask4_False()
{
    return Mask4Template<float>(-1.0f, -1.0f, -1.0f, -1.0f);
}

RW_MATH_FORCE_INLINE float GetVecFloat_NegativeTwo()
{
    return -2.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_NegativeOne()
{
    return -1.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_NegativeHalf()
{
    return -0.5f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_NegativeZero()
{
    return -0.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_Zero()
{
    return 0.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_Half()
{
    return 0.5f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_One()
{
    return 1.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_Two()
{
    return 2.0f;
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_Zero()
{
    return GetVector2_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_One()
{
    return GetVector2_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_XAxis()
{
    return GetVector2_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_YAxis()
{
    return GetVector2_YAxisTemplate<float>();
}


RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_Zero()
{
    return GetVector3_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_One()
{
    return GetVector3_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_XAxis()
{
    return GetVector3_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_YAxis()
{
    return GetVector3_YAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_ZAxis()
{
    return GetVector3_ZAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_Zero()
{
    return GetVector4_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_One()
{
    return GetVector4_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_Origin()
{
    return GetVector4_OriginTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_XAxis()
{
    return GetVector4_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_YAxis()
{
    return GetVector4_YAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_ZAxis()
{
    return GetVector4_ZAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_WAxis()
{
    return GetVector4_WAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix33Template<float> GetMatrix33_Identity()
{
    return GetMatrix33_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix33Template<float> GetMatrix33_Zero()
{
    return GetMatrix33_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44Template<float> GetMatrix44_Identity()
{
    return GetMatrix44_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<float> GetMatrix44Affine_Identity()
{
    return GetMatrix44Affine_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate<float> GetMatrix44AffinePacked_Identity()
{
    return GetMatrix44AffinePacked_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44Template<float> GetMatrix44_Zero()
{
    return GetMatrix44_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<float> GetMatrix44Affine_Zero()
{
    return GetMatrix44Affine_ZeroTemplate<float>();
}


RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate<float> GetMatrix44AffinePacked_Zero()
{
    return GetMatrix44AffinePacked_ZeroTemplate<float>();
}


RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternion_Identity()
{
    return GetQuaternion_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternion_Zero()
{
    return GetQuaternion_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternion_One()
{
    return GetQuaternion_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<float> GetQuatPos_Identity()
{
    return GetQuatPos_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<float> GetQuatPos_Zero()
{
    return GetQuatPos_ZeroTemplate<float>();
}


//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE MaskScalarTemplate<float> GetMaskScalar_32_True()
{
    return MaskScalarTemplate<float>(1.0f);
}

RW_MATH_FORCE_INLINE Mask2Template<float> GetMask2_32_True()
{
    return Mask2Template<float>(1.0f, 1.0f);
}

RW_MATH_FORCE_INLINE Mask3Template<float> GetMask3_32_True()
{
    return Mask3Template<float>(1.0f, 1.0f, 1.0f);
}

RW_MATH_FORCE_INLINE Mask4Template<float> GetMask4_32_True()
{
    return Mask4Template<float>(1.0f, 1.0f, 1.0f, 1.0f);
}

RW_MATH_FORCE_INLINE MaskScalarTemplate<float> GetMaskScalar_32_False()
{
    return MaskScalarTemplate<float>(-1.0f);
}

RW_MATH_FORCE_INLINE Mask2Template<float> GetMask2_32_False()
{
    return Mask2Template<float>(-1.0f, -1.0f);
}

RW_MATH_FORCE_INLINE Mask3Template<float> GetMask3_32_False()
{
    return Mask3Template<float>(-1.0f, -1.0f, -1.0f);
}

RW_MATH_FORCE_INLINE Mask4Template<float> GetMask4_32_False()
{
    return Mask4Template<float>(-1.0f, -1.0f, -1.0f, -1.0f);
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_NegativeTwo()
{
    return -2.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_NegativeOne()
{
    return -1.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_NegativeHalf()
{
    return -0.5f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_NegativeZero()
{
    return -0.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_Zero()
{
    return 0.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_Half()
{
    return 0.5f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_One()
{
    return 1.0f;
}

RW_MATH_FORCE_INLINE float GetVecFloat_32_Two()
{
    return 2.0f;
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_32_Zero()
{
    return GetVector2_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_32_One()
{
    return GetVector2_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_32_XAxis()
{
    return GetVector2_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2_32_YAxis()
{
    return GetVector2_YAxisTemplate<float>();
}


RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_32_Zero()
{
    return GetVector3_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_32_One()
{
    return GetVector3_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_32_XAxis()
{
    return GetVector3_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_32_YAxis()
{
    return GetVector3_YAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3_32_ZAxis()
{
    return GetVector3_ZAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_Zero()
{
    return GetVector4_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_One()
{
    return GetVector4_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_Origin()
{
    return GetVector4_OriginTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_XAxis()
{
    return GetVector4_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_YAxis()
{
    return GetVector4_YAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_ZAxis()
{
    return GetVector4_ZAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4_32_WAxis()
{
    return GetVector4_WAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix33Template<float> GetMatrix33_32_Identity()
{
    return GetMatrix33_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix33Template<float> GetMatrix33_32_Zero()
{
    return GetMatrix33_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44Template<float> GetMatrix44_32_Identity()
{
    return GetMatrix44_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<float> GetMatrix44Affine_32_Identity()
{
    return GetMatrix44Affine_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44Template<float> GetMatrix44_32_Zero()
{
    return GetMatrix44_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<float> GetMatrix44Affine_32_Zero()
{
    return GetMatrix44Affine_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternion_32_Identity()
{
    return GetQuaternion_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternion_32_Zero()
{
    return GetQuaternion_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternion_32_One()
{
    return GetQuaternion_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<float> GetQuatPos_32_Identity()
{
    return GetQuatPos_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<float> GetQuatPos_32_Zero()
{
    return GetQuatPos_ZeroTemplate<float>();
}

//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE MaskScalarTemplate<double> GetMaskScalar_64_True()
{
    return MaskScalarTemplate<double>(1.0);
}

RW_MATH_FORCE_INLINE Mask2Template<double> GetMask2_64_True()
{
    return Mask2Template<double>(1.0, 1.0);
}

RW_MATH_FORCE_INLINE Mask3Template<double> GetMask3_64_True()
{
    return Mask3Template<double>(1.0, 1.0, 1.0);
}

RW_MATH_FORCE_INLINE Mask4Template<double> GetMask4_64_True()
{
    return Mask4Template<double>(1.0, 1.0, 1.0, 1.0);
}

RW_MATH_FORCE_INLINE MaskScalarTemplate<double> GetMaskScalar_64_False()
{
    return MaskScalarTemplate<double>(-1.0);
}

RW_MATH_FORCE_INLINE Mask2Template<double> GetMask2_64_False()
{
    return Mask2Template<double>(-1.0, -1.0);
}

RW_MATH_FORCE_INLINE Mask3Template<double> GetMask3_64_False()
{
    return Mask3Template<double>(-1.0, -1.0, -1.0);
}

RW_MATH_FORCE_INLINE Mask4Template<double> GetMask4_64_False()
{
    return Mask4Template<double>(-1.0, -1.0, -1.0, -1.0);
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_NegativeTwo()
{
    return -2.0;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_NegativeOne()
{
    return -1.0;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_NegativeHalf()
{
    return -0.5;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_NegativeZero()
{
    return -0.0;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_Zero()
{
    return 0.0;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_Half()
{
    return 0.5;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_One()
{
    return 1.0;
}

RW_MATH_FORCE_INLINE double GetVecFloat_64_Two()
{
    return 2.0f;
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2_64_Zero()
{
    return GetVector2_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2_64_One()
{
    return GetVector2_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2_64_XAxis()
{
    return GetVector2_XAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2_64_YAxis()
{
    return GetVector2_YAxisTemplate<double>();
}


RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3_64_Zero()
{
    return GetVector3_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3_64_One()
{
    return GetVector3_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3_64_XAxis()
{
    return GetVector3_XAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3_64_YAxis()
{
    return GetVector3_YAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3_64_ZAxis()
{
    return GetVector3_ZAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_Zero()
{
    return GetVector4_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_One()
{
    return GetVector4_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_Origin()
{
    return GetVector4_OriginTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_XAxis()
{
    return GetVector4_XAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_YAxis()
{
    return GetVector4_YAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_ZAxis()
{
    return GetVector4_ZAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4_64_WAxis()
{
    return GetVector4_WAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix33Template<double> GetMatrix33_64_Identity()
{
    return GetMatrix33_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix33Template<double> GetMatrix33_64_Zero()
{
    return GetMatrix33_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44Template<double> GetMatrix44_64_Identity()
{
    return GetMatrix44_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<double> GetMatrix44Affine_64_Identity()
{
    return GetMatrix44Affine_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44Template<double> GetMatrix44_64_Zero()
{
    return GetMatrix44_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<double> GetMatrix44Affine_64_Zero()
{
    return GetMatrix44Affine_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<double> GetQuaternion_64_Identity()
{
    return GetQuaternion_IdentityTemplate<double>();
}


RW_MATH_FORCE_INLINE QuaternionTemplate<double> GetQuaternion_64_Zero()
{
    return GetQuaternion_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<double> GetQuaternion_64_One()
{
    return GetQuaternion_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<double> GetQuatPos_64_Identity()
{
    return GetQuatPos_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<double> GetQuatPos_64_Zero()
{
    return GetQuatPos_ZeroTemplate<double>();
}

//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2U_32_Zero()
{
    return GetVector2_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2U_32_One()
{
    return GetVector2_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2U_32_XAxis()
{
    return GetVector2_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector2Template<float> GetVector2U_32_YAxis()
{
    return GetVector2_YAxisTemplate<float>();
}


RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3U_32_Zero()
{
    return GetVector3_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3U_32_One()
{
    return GetVector3_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3U_32_XAxis()
{
    return GetVector3_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3U_32_YAxis()
{
    return GetVector3_YAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector3Template<float> GetVector3U_32_ZAxis()
{
    return GetVector3_ZAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_Zero()
{
    return GetVector4_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_One()
{
    return GetVector4_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_Origin()
{
    return GetVector4_OriginTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_XAxis()
{
    return GetVector4_XAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_YAxis()
{
    return GetVector4_YAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_ZAxis()
{
    return GetVector4_ZAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Vector4Template<float> GetVector4U_32_WAxis()
{
    return GetVector4_WAxisTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix33Template<float> GetMatrix33U_32_Identity()
{
    return GetMatrix33_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix33Template<float> GetMatrix33U_32_Zero()
{
    return GetMatrix33_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44Template<float> GetMatrix44U_32_Identity()
{
    return GetMatrix44_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<float> GetMatrix44AffineU_32_Identity()
{
    return GetMatrix44Affine_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44Template<float> GetMatrix44U_32_Zero()
{
    return GetMatrix44_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<float> GetMatrix44AffineU_32_Zero()
{
    return GetMatrix44Affine_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternionU_32_Identity()
{
    return GetQuaternion_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternionU_32_Zero()
{
    return GetQuaternion_ZeroTemplate<float>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<float> GetQuaternionU_32_One()
{
    return GetQuaternion_OneTemplate<float>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<float> GetQuatPosU_32_Identity()
{
    return GetQuatPos_IdentityTemplate<float>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<float> GetQuatPosU_32_Zero()
{
    return GetQuatPos_ZeroTemplate<float>();
}

//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2U_64_Zero()
{
    return GetVector2_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2U_64_One()
{
    return GetVector2_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2U_64_XAxis()
{
    return GetVector2_XAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector2Template<double> GetVector2U_64_YAxis()
{
    return GetVector2_YAxisTemplate<double>();
}


RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3U_64_Zero()
{
    return GetVector3_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3U_64_One()
{
    return GetVector3_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3U_64_XAxis()
{
    return GetVector3_XAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3U_64_YAxis()
{
    return GetVector3_YAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector3Template<double> GetVector3U_64_ZAxis()
{
    return GetVector3_ZAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_Zero()
{
    return GetVector4_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_One()
{
    return GetVector4_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_Origin()
{
    return GetVector4_OriginTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_XAxis()
{
    return GetVector4_XAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_YAxis()
{
    return GetVector4_YAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_ZAxis()
{
    return GetVector4_ZAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Vector4Template<double> GetVector4U_64_WAxis()
{
    return GetVector4_WAxisTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix33Template<double> GetMatrix33U_64_Identity()
{
    return GetMatrix33_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix33Template<double> GetMatrix33U_64_Zero()
{
    return GetMatrix33_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44Template<double> GetMatrix44U_64_Identity()
{
    return GetMatrix44_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<double> GetMatrix44AffineU_64_Identity()
{
    return GetMatrix44Affine_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44Template<double> GetMatrix44U_64_Zero()
{
    return GetMatrix44_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE Matrix44AffineTemplate<double> GetMatrix44AffineU_64_Zero()
{
    return GetMatrix44Affine_ZeroTemplate<double>();
}


RW_MATH_FORCE_INLINE QuaternionTemplate<double> GetQuaternionU_64_Identity()
{
    return GetQuaternion_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<double> GetQuaternionU_64_Zero()
{
    return GetQuaternion_ZeroTemplate<double>();
}

RW_MATH_FORCE_INLINE QuaternionTemplate<double> GetQuaternionU_64_One()
{
    return GetQuaternion_OneTemplate<double>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<double> GetQuatPosU_64_Identity()
{
    return GetQuatPos_IdentityTemplate<double>();
}

RW_MATH_FORCE_INLINE QuatPosTemplate<double> GetQuatPosU_64_Zero()
{
    return GetQuatPos_ZeroTemplate<double>();
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif
