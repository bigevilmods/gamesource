// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_PS3_PPU_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_PS3_PPU_VECTOR_INTRINSIC_H

#include "rw/math/detail/base.h"


namespace rw {
namespace math {
namespace vpu {

typedef vector float VectorIntrinsic;
typedef VectorIntrinsic VectorIntrinsicInParam;

namespace detail
{
    extern const VectorIntrinsic gQuatFromMat_xSigns;
    extern const VectorIntrinsic gQuatFromMat_ySigns;
    extern const VectorIntrinsic gQuatFromMat_zSigns;
    extern const VectorIntrinsic gXYZMask;
    extern const VectorIntrinsic gGatherMostSignificantBytesV3;
    extern const VectorIntrinsic gGatherMostSignificantBytesV4;
    extern const VectorIntrinsic gSqrt2s;
    extern const VectorIntrinsic gIVector;
    extern const VectorIntrinsic gJVector;
    extern const VectorIntrinsic gKVector;
    extern const VectorIntrinsic gLVector;
    extern const VectorIntrinsic gCrossProductPermuteConstant;
    extern const VectorIntrinsic gHalfPI;
    extern const VectorIntrinsic gPI;
    extern const VectorIntrinsic g2PI;
    extern const VectorIntrinsic g2PIReciprocal;

    // Table of values for swizzle store macro
    const size_t NUM_SWIZZLE_STORE_CONSTANTS = 16;  // (4 choices for Dest) * (4 choices for Src)
    extern const VectorIntrinsic gSwizzleStoreConstants[NUM_SWIZZLE_STORE_CONSTANTS];
    extern const VectorIntrinsic gBooleanConstants[2];

    extern const VectorIntrinsic gCosMinSlerpAngle;
}

}}}

#endif
