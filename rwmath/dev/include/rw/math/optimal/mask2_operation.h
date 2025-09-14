// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_MASK2_OPERATION_H
#define RW_MATH_OPTIMAL_MASK2_OPERATION_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/mask2_operation.h"
#else
    #include "rw/math/fpu/mask2_operation.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif