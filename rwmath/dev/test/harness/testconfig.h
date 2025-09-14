// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_CONFIG_H
#define RW_MATH_UNITTEST_CONFIG_H

#if defined(_MSC_VER)
    #pragma once
#endif

namespace testing
{
    static const int RW_MATH_UNITTEST_TEST_VECTOR_ARRAY_SIZE = 64;
    static const int RW_MATH_UNITTEST_TEST_MATRIX_ARRAY_SIZE = 64;
    static const int RW_MATH_UNITTEST_TEST_LOOP_COUNT_LONG = 1000;
}

#if defined _MSC_VER
    #pragma warning(disable:4723) // potential divide by 0
#endif

#endif
