// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FLOAT_OPERATION_INLINE_H
#define RW_MATH_FLOAT_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#ifdef RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#include "rw/math/vpl/detail/simulated/float_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{
    // This using statement imports all simulated float operations into the vpl namespace.
    using namespace rw::math::vpl::detail::simulated_floatops;
} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#endif // RW_MATH_FLOAT_OPERATION_INLINE_H
