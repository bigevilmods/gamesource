// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_STATIC_ASSERTS_H
#define RW_MATH_STATIC_ASSERTS_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

namespace rw
{
namespace math
{
namespace detail
{

//////////////////////////////////////////////////////////////////////////////
// This file contains the logic for various assertions.  The intent is to
// keep the implementation code clean of complex assertions, and to allow
// sharing of assertions between platform specific code.
//////////////////////////////////////////////////////////////////////////////

// This class is used to statically assert at compile time.
//
// Instantiating the class with a 'false' template parameter will cause a
// template instantiation error.  If you are trying to determine the cause of a
// compilation failure, look at the condition that was used to instantiate the
// RWMathStaticAssert template.
template <bool> class RWMathStaticAssert;
template <> class RWMathStaticAssert<true>{};

#ifdef __SNC__
    // SNC likes to generate a call to memset to initialize an empty class with 0.
    // This adds a completely unnecessary function call for a static assert.
    // This "class body" is used to avoid this code generation in release builds.
    #define RW_MATH_STATIC_ASSERT_CLASS_BODY(class_name) \
        public: \
        class_name() \
            :dummy(0) {} \
        int dummy; 
#else
    #define RW_MATH_STATIC_ASSERT_CLASS_BODY(class_name)
#endif

// Ensure that the permution constant only indexes elements 0, 1, 2, or 3 since
// a single vector is being indexed into.
template <int PermConstant>
class Assert_ValidSinglePermConstant
    : public ::rw::math::detail::RWMathStaticAssert<
        (PermConstant & 0x3333) == PermConstant
    >
{
    RW_MATH_STATIC_ASSERT_CLASS_BODY(Assert_ValidSinglePermConstant)
};

// Ensure that the permution constant only indexes elements 0, 1, 2 since
// a single vector is being indexed into.
template <int PermConstant>
class Assert_ValidVector3PermConstant
    : public ::rw::math::detail::RWMathStaticAssert<
        (PermConstant & 0x333) == PermConstant &&
        ((PermConstant & 0x300)>>8) <= 0x2 &&
        ((PermConstant & 0x030)>>4) <= 0x2 &&
        ((PermConstant & 0x003)>>0) <= 0x2
    >
{
    RW_MATH_STATIC_ASSERT_CLASS_BODY(Assert_ValidVector3PermConstant)
};

// Ensure that the permution constant only indexes elements 0, 1, 2 since
// a single vector is being indexed into.
template <int PermConstant>
class Assert_ValidVector2PermConstant
    : public ::rw::math::detail::RWMathStaticAssert<
        (PermConstant & 0x33) == PermConstant &&
        ((PermConstant & 0x030)>>4) <= 0x1 &&
        ((PermConstant & 0x003)>>0) <= 0x1
    >
{
    RW_MATH_STATIC_ASSERT_CLASS_BODY(Assert_ValidVector2PermConstant)
};

} // namespace detail
} // namespace math
} // namespace rw

#endif // RW_MATH_STATIC_ASSERTS_H
