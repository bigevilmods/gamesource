// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FLOAT_OPERATION_H
#define RW_MATH_FLOAT_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpl/detail/vplbase.h"

namespace rw
{
namespace math
{
namespace vpl
{

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU | VPL_BROADWAY )
    /*! \brief Performs a floating point select operation.
     *  
     *  This function returns one of two values (either nonNegativeResult or
     *  negativeResult) depending on the value of comparisonValue.
     *
     *  It is roughly equivalent to:
     *  \code
     *      (comparisonValue >= 0.0f) ? nonNegativeResult : negativeResult;
     *  \endcode
     *
     *  \e Note: The VPL does not define whether a value of -0 for \c
     *  comparisonValue will return \c nonNegativeResult or \c negativeResult.
     *  This may vary between platforms.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API float FloatSelect( float comparisonValue, float nonNegativeResult, float negativeResult );

    /*! \brief Performs a floating point reciprocal estimate
     *  
     *  This function calculates a reciprocal estimate of 'value'
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API float FloatReciprocalEstimate( float value );

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_BROADWAY )
    /*! \brief Performs a floating point select operation.
     *  
     *  This function returns one of two values (either nonNegativeResult or
     *  negativeResult) depending on the value of comparisonValue.
     *
     *  It is roughly equivalent to:
     *  \code
     *      (comparisonValue >= 0.0) ? nonNegativeResult : negativeResult;
     *  \endcode
     *
     *  \e Note: The VPL does not define whether a value of -0 for \c
     *  comparisonValue will return \c nonNegativeResult or \c negativeResult.
     *  This may vary between platforms.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API double DoubleSelect( double comparisonValue, double nonNegativeResult, double negativeResult );

    /*! \brief Performs a floating point reciprocal estimate
     *  
     *  This function calculates a reciprocal estimate of 'value'
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API double DoubleReciprocalEstimate( double value );
#endif

} // namespace vpl
} // namespace math
} // namespace rw

#include "rw/math/vpl/detail/float_operation_inline.h"

#endif // RW_MATH_FLOAT_OPERATION_H

