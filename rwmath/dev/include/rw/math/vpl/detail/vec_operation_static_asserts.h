// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_STATIC_ASSERTS_H
#define RW_MATH_VEC_OPERATION_STATIC_ASSERTS_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/static_asserts.h"

namespace rw
{
namespace math
{
namespace vpl
{
namespace detail
{

//////////////////////////////////////////////////////////////////////////////
// This file contains the logic for various assertions.  The intent is to
// keep the implementation code clean of complex assertions, and to allow
// sharing of assertions between platform specific code.
//////////////////////////////////////////////////////////////////////////////

// Ensure Value is in the range [-16, 15]
template <int Value>
class Assert_5BitSignedValue
    : public ::rw::math::detail::RWMathStaticAssert<
        Value < 16 &&
        Value >= -16
    >
{
    RW_MATH_STATIC_ASSERT_CLASS_BODY(Assert_5BitSignedValue)
};

template <int Value, int Min, int Max>
class Assert_ValueInRange
    : public ::rw::math::detail::RWMathStaticAssert<
        Value <= Max &&
        Value >= Min
    >
{
    RW_MATH_STATIC_ASSERT_CLASS_BODY(Assert_ValueInRange)
};

} // namespace detail
} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_VEC_OPERATION_STATIC_ASSERTS_H
