// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_VECTOR2_H
#define RW_MATH_OPTIMAL_VECTOR2_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/vector2.h"
#else
    #include "rw/math/fpu/vector2.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif