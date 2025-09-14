// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_HALF_OPERATION_H
#define RW_MATH_HALF_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/half/half_types.h"
#include "rw/math/fpu/vector2.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/mini.h"

namespace rw
{
namespace math
{
namespace half
{

    /*!
        \brief Convert a float to a half float.

        This function converts a float to its corresponding half float. The float value must be in the valid range that can be represented by a half float value. 
        Additionally, special values like +/-INFINITY, +/-NaN, and the denormalized, are not supported. The result is undefined for these special and out-of-range float values.
        +/-Zeros are supported.

        Note: define RW_MATH_ENABLE_ASSERTS to enable assertion for invalid floats and unsupported special float values.
    */
    RW_MATH_INLINE half_t HalfFromFloat(float fVal)
    {
        return mini::FP16::MiniFromFloat(fVal);
    }

    /*!
        \brief Convert a half float to a float.

        This function converts a half float to its corresponding float. Special values like +/-INFINITY, +/-NaN, and the denormalized, are not supported. 
        The result is undefined for these special half floats. +/-Zeros are supported.

        Note: define RW_MATH_ENABLE_ASSERTS to enable assertion for unsupported special half float values.
    */
    RW_MATH_INLINE float  FloatFromHalf(half_t hVal)
    {
        return mini::FP16::FloatFromMini(hVal);
    }

    /*!
        \brief Checks if a half float is +/-NaN.
    */
    RW_MATH_INLINE bool IsHalfNaN(half_t value)
    {
        return mini::FP16::IsNaN(value);
    }

    /*!
        \brief Checks if a half float is +/-INFINITY.
    */
    RW_MATH_INLINE bool IsHalfInfinite(half_t value)
    {
        return mini::FP16::IsInfinite(value);
    }

    RW_MATH_FORCE_INLINE fpu::Vector2 Vector2FromHalfVector2(HalfVector2::InParam hv2)
    {
        return fpu::Vector2(FloatFromHalf(hv2.x), FloatFromHalf(hv2.y));
    }

    RW_MATH_FORCE_INLINE fpu::Vector3 Vector3FromHalfVector3(HalfVector3::InParam hv3)
    {
        return fpu::Vector3(FloatFromHalf(hv3.x), FloatFromHalf(hv3.y), FloatFromHalf(hv3.z));
    }

    RW_MATH_FORCE_INLINE fpu::Vector4 Vector4FromHalfVector4(HalfVector4::InParam hv4)
    {
        return fpu::Vector4(FloatFromHalf(hv4.x), FloatFromHalf(hv4.y), FloatFromHalf(hv4.z), FloatFromHalf(hv4.w));
    }

    RW_MATH_FORCE_INLINE HalfVector2 HalfVector2FromVector2(fpu::Vector2::InParam v2)
    {
        return HalfVector2(HalfFromFloat(v2.x), HalfFromFloat(v2.y));
    }

    RW_MATH_FORCE_INLINE HalfVector3 HalfVector3FromVector3(fpu::Vector3::InParam v3)
    {
        return HalfVector3(HalfFromFloat(v3.x), HalfFromFloat(v3.y), HalfFromFloat(v3.z));
    }

    RW_MATH_FORCE_INLINE HalfVector4 HalfVector4FromVector4(fpu::Vector4::InParam v4)
    {
        return HalfVector4(HalfFromFloat(v4.x), HalfFromFloat(v4.y), HalfFromFloat(v4.z), HalfFromFloat(v4.w));
    }

} // namespace half
} // namespace math
} // namespace rw

#endif
