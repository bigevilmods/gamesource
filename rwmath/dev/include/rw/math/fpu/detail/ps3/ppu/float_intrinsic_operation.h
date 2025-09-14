// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H
#define RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

#include "rw/math/detail/base.h"

#if (CELL_SDK_VERSION < 0x080000)
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#else

#include "rw/math/fpu/detail/ps3/ppu/ppu_math_intrinsics.h"


#endif

#endif // RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

