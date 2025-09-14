// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FLOAT_OPERATION_INLINE_H
#define RW_MATH_FLOAT_OPERATION_INLINE_H

#include "rw/math/fpu/detail/wii/float_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT fsel
    RW_MATH_FORCE_INLINE VPL_API float FloatSelect( float comparisonValue, float nonNegativeResult, float negativeResult )
    {
        register double a = comparisonValue;
        register double b = nonNegativeResult;
        register double c = negativeResult;
        register double result;
        asm { fsel result, a, b, c };
        return (float)result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT fsel
    RW_MATH_FORCE_INLINE VPL_API double DoubleSelect( double comparisonValue, double nonNegativeResult, double negativeResult )
    {
        register double a = comparisonValue;
        register double b = nonNegativeResult;
        register double c = negativeResult;
        register double result;
        asm { fsel result, a, b, c };
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT fres
    RW_MATH_FORCE_INLINE VPL_API float FloatReciprocalEstimate( float value )
    {
        return RW_MATH_FPU_FRecipEst(value);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT fres
    RW_MATH_FORCE_INLINE VPL_API double DoubleReciprocalEstimate( double value )
    {
        return RW_MATH_FPU_DRecipEst(value);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif
