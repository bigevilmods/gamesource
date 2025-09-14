// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FLOAT_OPERATION_INLINE_H
#define RW_MATH_FLOAT_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/detail/float_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __fsel / fsel
    RW_MATH_FORCE_INLINE VPL_API float FloatSelect( float comparisonValue, float nonNegativeResult, float negativeResult )
    {
        return (float)__fsel( comparisonValue, nonNegativeResult, negativeResult );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __fsel / fsel
    RW_MATH_FORCE_INLINE VPL_API double DoubleSelect( double comparisonValue, double nonNegativeResult, double negativeResult )
    {
        return __fsel( comparisonValue, nonNegativeResult, negativeResult );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __fres / fres
    RW_MATH_FORCE_INLINE VPL_API float FloatReciprocalEstimate( float value )
    {
        return RW_MATH_FPU_FRecipEst(value);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __fres / fres
    RW_MATH_FORCE_INLINE VPL_API double DoubleReciprocalEstimate( double value )
    {
        return RW_MATH_FPU_DRecipEst(value);
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif
