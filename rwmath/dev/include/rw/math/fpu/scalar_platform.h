// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_SCALAR_PLATFORM_H
#define RW_MATH_FPU_SCALAR_PLATFORM_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if defined EA_PLATFORM_PS3
    #include "rw/math/fpu/detail/ps3/ppu/scalar_platform.h"
#elif defined EA_PLATFORM_PSP2
    #include "rw/math/fpu/detail/psp2/scalar_platform.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/fpu/detail/ps3/spu/scalar_platform.h"
#else

#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/vec_float.h"

namespace rw {
namespace math {
namespace fpu {

RW_MATH_INLINE bool IsNaN(float value)   { return value != value; }
RW_MATH_INLINE bool IsNaN(double value)   { return value != value; }

}}}

#endif
#endif
