// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_CONSTANTS_H
#define RW_MATH_VPU_CONSTANTS_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/fpu/constants.h"

namespace rw {
namespace math {
namespace vpu {

using ::rw::math::fpu::TOP_BIT32;
using ::rw::math::fpu::NOT_TOP_BIT32;
using ::rw::math::fpu::TOP_BIT64;
using ::rw::math::fpu::NOT_TOP_BIT64;
using ::rw::math::fpu::MAX_INT32;
using ::rw::math::fpu::MIN_INT32;
using ::rw::math::fpu::MIN_FLOAT;
using ::rw::math::fpu::MAX_FLOAT;
using ::rw::math::fpu::SMALL_FLOAT;
using ::rw::math::fpu::DEGREES_TO_RADIANS;
using ::rw::math::fpu::RADIANS_TO_DEGREES;
using ::rw::math::fpu::E;
using ::rw::math::fpu::PI;                
using ::rw::math::fpu::TWO_PI;
using ::rw::math::fpu::HALF_PI;
using ::rw::math::fpu::QUARTER_PI;
using ::rw::math::fpu::SQRT_PI;
using ::rw::math::fpu::INV_PI;
using ::rw::math::fpu::SQRT_2;
using ::rw::math::fpu::SQRT_HALF;
using ::rw::math::fpu::SQRT_3;
using ::rw::math::fpu::ZERO_FLOAT;
using ::rw::math::fpu::FLOAT_ONE_AS_UINT32;
using ::rw::math::fpu::VECTOR_MIN_SLERP_ANGLE;
using ::rw::math::fpu::MATRIX_MIN_SLERP_ANGLE;
using ::rw::math::fpu::QUATERNION_SLERP_DOT_TOLERANCE;
using ::rw::math::fpu::MINIMUM_RECIPROCAL;
using ::rw::math::fpu::EPSILON;
using ::rw::math::fpu::ONE_MINUS_EPSILON;
using ::rw::math::fpu::TOLERANCE;

}}}

#endif
