// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_CONSTANTS_H
#define RW_MATH_FPU_CONSTANTS_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

namespace rw
{
namespace math
{
namespace fpu
{

#include "rw/math/detail/define_constants.h"

//! 32 bit number with the most significant bit set to one
const int32_t TOP_BIT32         = 0x80000000;

//! 32 bit number with every bit set except for the most significant bit
const int32_t NOT_TOP_BIT32     = ~TOP_BIT32;

//! 64 bit number with the most significant bit set to one
const int64_t TOP_BIT64 = (int64_t(1) << 63);

//! 64 bit number with every bit set except for the most significant bit
const int64_t NOT_TOP_BIT64     = ~TOP_BIT64;

//! Maximum positive 32 bit integer value
const int32_t MAX_INT32         = (1u << 31) - 1;

//! Most negative 32 bit integer value
const int32_t MIN_INT32         = -MAX_INT32 - 1;

//! Minimum normalized floating-point value.
const float  MIN_FLOAT          = 1.17549435e-38f;

//! Maximum normalized floating-point value.
const float  MAX_FLOAT          = RW_MATH_MAX_FLOAT;

/*! 
    \brief A small floating-point value.

    If two floats are within SMALL_FLOAT of each other, they are considered
    equal.  The reason for 1/pow(2) is that it leads to a float value where
    the mantissa is 0.  So, processors like the mips can create the value in
    one cycle doing a lui instruction.
*/
const float  SMALL_FLOAT        = (1.0f/65536.0f);

//!  The constant to use when converting from degree to radians
const float  DEGREES_TO_RADIANS   = RW_MATH_DEGREES_TO_RADIANS;

//!  The constant to use when converting from radians to degree
const float  RADIANS_TO_DEGREES  = 57.29577951f;

//! Constant for transcendental number e
const float  E                  = 2.7182818284590452354f;

//! Constant for transcendental number Pi
const float  PI                 = RW_MATH_PI;

//! Constant for 2*PI
const float  TWO_PI             = 2.0f * RW_MATH_PI;

//! Constant for PI/2
const float  HALF_PI            = 1.57079632679489661923f;

//! Constant for PI/4
const float  QUARTER_PI         = 0.78539816339744830962f;

//! Constant for sqrt(PI)
const float  SQRT_PI            = 1.77245385090551602792981f;

//! Constant for 1/PI
const float  INV_PI             = 0.31830988618379067154f;

//! Constant for sqrt(2)
const float  SQRT_2             = 1.41421356237309504880f;

//! Constant for sqrt(1/2)
const float  SQRT_HALF          = 0.70710678118654752440f;

//! Constant for sqrt(3)
const float  SQRT_3             = 1.73205080756887719000f;

//! A zero floating point value.
const float  ZERO_FLOAT         = 0.0f;

//! The representation of float(1.0f) in unsigned int32
const uint32_t FLOAT_ONE_AS_UINT32 = 0x3f800000;

/*
    \brief Minimum angle required to use SLerp function

    VECTOR_MIN_SLERP_ANGLE is the smallest angle which the SLerp function
    can use.  If the dot product of the two vectors is smaller than 
    VECTOR_MIN_SLERP_ANGLE (5 degrees), the NLerp function will be used.
*/
const float VECTOR_MIN_SLERP_ANGLE = ((5.0f * RW_MATH_PI) / 180.0f);

/*!
    \brief Minimum angle required to use SLerp function

    MATRIX_MIN_SLERP_ANGLE is the smallest angle which the SLerp function
    can use.  If the dot product of the two vectors is smaller than 
    MATRIX_MIN_SLERP_ANGLE (5 degrees), the NLerp function will be used.
*/
const float MATRIX_MIN_SLERP_ANGLE = 2.0f * RW_MATH_DEGREES_TO_RADIANS;

//! Tolerance beyond which a quaternion slerp does a linear interpolation
const float QUATERNION_SLERP_DOT_TOLERANCE = 0.999f;

/*!
    \brief Minimum reciprocal value
    
    Minimum reciprocal value, smaller than the value of MIN_FLOAT, but still a floating point value
    defined by IEEE floating point standards. It depends on the hardware to support such
    standards.
*/
const float MINIMUM_RECIPROCAL  = 1.0f / RW_MATH_MAX_FLOAT;

//! The default epsilon
const float EPSILON             = RW_MATH_EPSILON;

//! One minus the default epsilon
const float ONE_MINUS_EPSILON   = 1.0f - RW_MATH_EPSILON;

//! Tolerance for rounding off during calculation.
const float TOLERANCE           = 0.001f;

#include "rw/math/detail/undefine_constants.h"

} // namespace fpu
} // namespace math
} // namespace rw

#endif
