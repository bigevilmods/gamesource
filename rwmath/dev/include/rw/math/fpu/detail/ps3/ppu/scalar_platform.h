// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_PS3_PPU_SCALAR_PLATFORM_H
#define RW_MATH_FPU_PS3_PPU_SCALAR_PLATFORM_H

#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/vec_float.h"

namespace rw {
namespace math {
namespace fpu {

#ifdef __SNC__
RW_MATH_INLINE bool IsNaN(double value)
{ 
#pragma control %push fltedge
// This pragma is used to ensure SNC doesn't optimize away the float comparison
#pragma control fltedge=2
    return value != value;
#pragma control %pop fltedge
}
#else
RW_MATH_INLINE bool IsNaN(double value)
{ 
    // The IsNaN function needs to compute the boolean expression:
    //  (value != value)
    // Unfortunately, GCC's optimizer may decide that (value != value) is always false when
    // -ffast-math or -ffinite-math-only is specified.  To work around this problem, inline
    // assembly is used to explicitly do the comparison.

    bool result;
    asm volatile (
            "fcmpu  7, %1, %1\n"    // Do a floating point comparison
            "mfcr   %0\n"           // Move the condition register to the result.
            "rlwinm %0,%0,31,1\n"   // Select the bit of the condition register we are interested in.
            : "=r" (result)
            : "f" (value)
            : "cc"
            );
    return !result;
}
#endif

RW_MATH_INLINE bool              IsNaN(float value)   { return IsNaN((double)value); }

}}}

#endif
