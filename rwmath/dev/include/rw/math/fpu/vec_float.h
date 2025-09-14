// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECFLOAT_H
#define RW_MATH_FPU_VECFLOAT_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

namespace rw {
namespace math {
namespace fpu {

    typedef const float VecFloatInParam;
    typedef float& VecFloatOutParam;
    typedef float  VecFloat;
    typedef float& VecFloatRefX;
    typedef float& VecFloatRefY;
    typedef float& VecFloatRefZ;
    typedef float& VecFloatRefW;
    typedef float& VecFloatRefIndex;
    
    RW_MATH_FORCE_INLINE float GetFloat(float f);    
    RW_MATH_FORCE_INLINE float GetFloat(float f) { return f; }
    
}}}

#endif
