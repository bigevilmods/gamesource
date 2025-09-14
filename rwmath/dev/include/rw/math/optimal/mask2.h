// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_MASK2_H
#define RW_MATH_OPTIMAL_MASK2_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/mask2.h"
#else
    #include "rw/math/fpu/mask2.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif