// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_XBOX2_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_XBOX2_VECTOR_INTRINSIC_H

#if defined _MSC_VER
    #pragma once
#endif

#include <vectorintrinsics.h>    // required for __vector4
#include "rw/math/detail/base.h"


namespace rw {
namespace math {
namespace vpu {

typedef __vector4 VectorIntrinsic;

// Passing by reference seems like the fastest way to pass vectors for Xenon.
// Passing by value leads to unnecessary vector stores to memory.  These
// additional stores have a significant performance impact!
typedef const VectorIntrinsic& VectorIntrinsicInParam;

namespace detail
{
    extern const VectorIntrinsic gQuatFromMat_xSigns;
    extern const VectorIntrinsic gQuatFromMat_ySigns;
    extern const VectorIntrinsic gQuatFromMat_zSigns;
    extern const VectorIntrinsic gXYZMask;
    extern const VectorIntrinsic gGatherMostSignificantBytesV3;
    extern const VectorIntrinsic gGatherMostSignificantBytesV4;
    extern const VectorIntrinsic gPI;
    extern const VectorIntrinsic gSqrt2s;
    extern const VectorIntrinsic gIVector;
    extern const VectorIntrinsic gJVector;
    extern const VectorIntrinsic gKVector;
    extern const VectorIntrinsic gLVector;
    extern const VectorIntrinsic gCrossProductPermuteConstant;
    extern const VectorIntrinsic g2PI;
    extern const VectorIntrinsic g2PIReciprocal;
    extern const VectorIntrinsic gLn2;
    extern const VectorIntrinsic gLn2Reciprocal;

    // Table of values for swizzle store macro
    const size_t NUM_SWIZZLE_STORE_CONSTANTS = 16;  // (4 choices for Dest) * (4 choices for Src)
    extern const VectorIntrinsic gSwizzleStoreConstants[NUM_SWIZZLE_STORE_CONSTANTS];
    extern const VectorIntrinsic gBooleanConstants[2];
    
    extern const VectorIntrinsic gCosMinSlerpAngle;
}

}}}

#endif
