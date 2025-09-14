// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_PLANE_H
#define RW_MATH_OPTIMAL_PLANE_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/plane.h"
#else
    #include "rw/math/fpu/plane.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif