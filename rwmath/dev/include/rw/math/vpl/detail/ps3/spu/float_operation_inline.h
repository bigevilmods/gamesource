// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FLOAT_OPERATION_INLINE_H
#define RW_MATH_FLOAT_OPERATION_INLINE_H

#include "rw/math/fpu/detail/float_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API float FloatSelect( float comparisonValue, float nonNegativeResult, float negativeResult )
    {
        vector float floatVec = __builtin_spu_promote_8( comparisonValue, 0 );  
        vector unsigned int mask = __builtin_spu_cmpgt_6( (vector float)(0.0f), floatVec );
        vector float resultVec = __builtin_spu_sel_8 ( __builtin_spu_promote_8(nonNegativeResult,0), __builtin_spu_promote_8(negativeResult,0), mask );
        float result = __builtin_spu_extract_8 ( resultVec, 0 );
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT __fres / fres
    RW_MATH_FORCE_INLINE VPL_API float FloatReciprocalEstimate( float value )
    {
        return RW_MATH_FPU_FRecipEst(value);
    }
} // namespace vpl
} // namespace math
} // namespace rw

#ifdef RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#include "rw/math/vpl/detail/simulated/float_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{
    using rw::math::vpl::detail::simulated_floatops::DoubleSelect;
    using rw::math::vpl::detail::simulated_floatops::DoubleReciprocalEstimate;
} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#endif // RW_MATH_FLOAT_OPERATION_INLINE_H
