// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_PS3_PSP2_SCALAR_PLATFORM_H
#define RW_MATH_FPU_PS3_PSP2_SCALAR_PLATFORM_H

#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/vec_float.h"

#include <stdio.h>

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{
bool CompareEqual(double a, double b);
}

RW_MATH_INLINE bool IsNaN(double value)
{ 
    return !detail::CompareEqual(value, value);
}

RW_MATH_INLINE bool              IsNaN(float value)   { return IsNaN((double)value); }

}
}
}

#endif
