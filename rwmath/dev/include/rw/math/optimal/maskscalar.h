// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_MASKSCALAR_H
#define RW_MATH_OPTIMAL_MASKSCALAR_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/maskscalar.h"
#else
    #include "rw/math/fpu/maskscalar.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif