// (c) Electronic Arts. All Rights Reserved.

// Q: 
// Why does this file exist?
//
// A:
// The SPU compiler (as of the 082_SDK) has some issues with constants.  If a constant
// is constructed using other constants the compiler will not generate the dependent
// constant at compile time.  For example:
//     const float one = 0.5f + 0.5f;
//     const float two = one + one;
// In this example 'one' will be calculated at compile time because it is composed of literals.
// However, 'two' will not be calculated at compile time because it is based on another
// constant.
//
// This file defines constants so they can be used in expressions to create other constants. 
// That way, the compiler can properly construct constants at compile time.

// Important:
//
// This file should only be used in the math library internally.  i.e. users should not include this file.
//
// Any math library files that include this header should also include 'undefine_constants.h' to undefine
// the constants to avoid polluting the global defines.
//
// All of the defines in this file should be undefined in 'undefine_constants.h'.


#define RW_MATH_PI                          3.1415928f
#define RW_MATH_EPSILON                     1.192092896e-07f
#define RW_MATH_DEGREES_TO_RADIANS          0.01745329252f

#ifdef __SPU__
    // The SPU uses a slightly different constant to ensure the compiler generates the same value as the PPU
    #define RW_MATH_MAX_FLOAT                   3.402823467e+38f
#else
    #define RW_MATH_MAX_FLOAT                   3.402823466e+38f
#endif

