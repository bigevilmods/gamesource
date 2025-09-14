// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_OPTIMAL_SQT_OPERATION_H
#define RW_MATH_OPTIMAL_SQT_OPERATION_H

#include "rw/math/detail/base.h"

#if defined(RWMATH_VPU_IS_OPTIMAL)
    #include "rw/math/vpu/sqt_operation.h"
#else
    #include "rw/math/fpu/sqt_operation.h"
#endif

RWMATH_DETAIL_INCLUDE_OPTIMAL

#endif