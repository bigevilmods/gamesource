// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_QUATERNION_H
#define RW_MATH_OPTIMAL_QUATERNION_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/quaternion.h"
#else
    #include "rw/math/fpu/quaternion.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif