// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VECUTIL_OPERATION_H
#define RW_MATH_VECUTIL_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpl/detail/vplbase.h"

namespace rw
{
namespace math
{
namespace vpl
{

    /*! \brief Creates a vector from 4 \c float values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic(float v0, float v1, float v2, float v3);

    /*! \brief Creates a vector from 4 unsigned integer values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_UInt(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);

    /*! \brief Creates a vector from 4 signed integer values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_Int(int32_t v0, int32_t v1, int32_t v2, int32_t v3);

    /*! \brief Creates a vector from 8 unsigned short values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_UShort(uint16_t v0, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4, uint16_t v5, uint16_t v6, uint16_t v7);

    /*! \brief Creates a vector from 8 signed short values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_Short(int16_t v0, int16_t v1, int16_t v2, int16_t v3, int16_t v4, int16_t v5, int16_t v6, int16_t v7);

    /*! \brief Creates a vector from 16 unsigned char values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_UChar(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5, uint8_t v6, uint8_t v7, uint8_t v8, uint8_t v9, uint8_t v10, uint8_t v11, uint8_t v12, uint8_t v13, uint8_t v14, uint8_t v15);

    /*! \brief Creates a vector from 16 signed char values
     *
     *  See \ref s_static_initialization_order_init_via_func for a discussion of limitations when using this function to initialize global constants.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreateIntrinsic_Char(int8_t v0, int8_t v1, int8_t v2, int8_t v3, int8_t v4, int8_t v5, int8_t v6, int8_t v7, int8_t v8, int8_t v9, int8_t v10, int8_t v11, int8_t v12, int8_t v13, int8_t v14, int8_t v15);

    /*! \brief Creates a vector from a 4bit unsigned integer value.
     *
     *  This function initializes each 32bit integer element of the vector with the \c value specified.
     *  \c value must be in the range [0,15] for correct operation.
     *
     *  Note: \c value's range was chosen so this operation could be efficiently implemented using VMX
     *  without triggering a LHS.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UInt(uint32_t value);

    /*! \brief Creates a vector from a 4bit unsigned integer value.
     *
     *  This function initializes each 16bit integer element of the vector with the \c value specified.
     *  \c value must be in the range [0,15] for correct operation.
     *
     *  Note: \c value's range was chosen so this operation could be efficiently implemented using VMX
     *  without triggering a LHS.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UShort(uint32_t value);

    /*! \brief Creates a vector from a 4bit unsigned integer value.
     *
     *  This function initializes each 8bit integer element of the vector with the \c value specified.
     *  \c value must be in the range [0,15] for correct operation.
     *
     *  Note: \c value's range was chosen so this operation could be efficiently implemented using VMX
     *  without triggering a LHS.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UChar(uint32_t value);

    /*! \brief Convert a vector of floats to a vector of half floats.
     *
     *  This function converts a vector of floats \c vec to a vector of the corresponding half floats
	 *  All float values in \c vec must be in the valid range that can be represented by a half float value.
     *  Additionally, special values, +/-INFINITY, NaN, and the denormalized, are not supported. The result
     *  is undefined for these special and out-of-range float values. Positive and negative zeros are supported. 
     *
     *  Note: define RW_MATH_ENABLE_ASSERTS to enable assertion for invalid float and unsupported special values.
     *
     *  VPL_AUTODOC
     */
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilConvertFloatToHalf( VectorIntrinsicInParam vec );

	 /*! \brief Convert a vector of half floats to a vector of floats.
     *
     *  This function converts a vector of half floats \c vec to a vector of the corresponding floats
     *  special half float values, +/-INFINITY, NaN, and the denormalized, are not supported. The result
     *  is undefined for these special half floats. Positive and negative zeros are supported. 
	 *
     *  Note: define RW_MATH_ENABLE_ASSERTS to enable assertion for unsupported special half float values.
     *
     *  VPL_AUTODOC
     */
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilConvertHalfToFloat( VectorIntrinsicInParam vec );

} // namespace vpl
} // namespace math
} // namespace rw

#include "rw/math/vpl/detail/vecutil_operation_inline.h"

#endif // RW_MATH_VECUTIL_OPERATION_H

