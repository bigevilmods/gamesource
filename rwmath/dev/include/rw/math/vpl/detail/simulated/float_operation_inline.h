// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SIMULATED_FLOAT_OPERATION_INLINE_H
#define RW_MATH_SIMULATED_FLOAT_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{
namespace detail
{
namespace simulated_floatops
{
    
    RW_MATH_FORCE_INLINE VPL_API float FloatSelect( float comparisonValue, float nonNegativeResult, float negativeResult )
    {
        return ( comparisonValue >= 0.0f ) ? nonNegativeResult : negativeResult;
    }

    RW_MATH_FORCE_INLINE VPL_API double DoubleSelect( double comparisonValue, double nonNegativeResult, double negativeResult )
    {
        return ( comparisonValue >= 0.0 ) ? nonNegativeResult : negativeResult;
    }

    RW_MATH_FORCE_INLINE VPL_API float FloatReciprocalEstimate( float value )
    {
        return 1.0f/value;
    }

    RW_MATH_FORCE_INLINE VPL_API double DoubleReciprocalEstimate( double value )
    {
        return 1.0/value;
    }

} // namespace simulated_floatops
} // namespace detail
} // namespace vpl
} // namespace math
} // namespace rw

#endif
