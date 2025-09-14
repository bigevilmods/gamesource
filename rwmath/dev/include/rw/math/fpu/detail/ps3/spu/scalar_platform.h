// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_PS3_SPU_SCALAR_PLATFORM_H
#define RW_MATH_FPU_PS3_SPU_SCALAR_PLATFORM_H

#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/vec_float.h"

namespace rw {
namespace math {
namespace fpu {

// There's no NaN value in SPU (both for float and double).  
// All NaN operations evaluates to 0    
RW_MATH_INLINE bool IsNaN(double) { return false; }
RW_MATH_INLINE bool IsNaN(float)  { return false; }

}}}

#endif
