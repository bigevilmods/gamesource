// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_VECTOR2_OPERATION_H
#define RW_MATH_OPTIMAL_VECTOR2_OPERATION_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/vector2_operation.h"
#else
    #include "rw/math/fpu/vector2_operation.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif