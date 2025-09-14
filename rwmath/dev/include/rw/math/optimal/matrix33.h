// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_MATRIX33_H
#define RW_MATH_OPTIMAL_MATRIX33_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/matrix33.h"
#else
    #include "rw/math/fpu/matrix33.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif