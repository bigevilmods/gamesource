// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_MATRIX44_H
#define RW_MATH_OPTIMAL_MATRIX44_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/matrix44.h"
#else
    #include "rw/math/fpu/matrix44.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif