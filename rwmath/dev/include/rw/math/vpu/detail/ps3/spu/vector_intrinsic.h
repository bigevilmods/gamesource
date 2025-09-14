// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_PS3_SPU_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_PS3_SPU_VECTOR_INTRINSIC_H

namespace rw
{
namespace math
{
namespace vpu
{

typedef vector float VectorIntrinsic;
typedef VectorIntrinsic VectorIntrinsicInParam;

namespace detail
{
    extern const VectorIntrinsic gQuatFromMat_xSigns;
    extern const VectorIntrinsic gQuatFromMat_ySigns;
    extern const VectorIntrinsic gQuatFromMat_zSigns;
    extern const VectorIntrinsic gGatherMostSignificantBytesV3;
    extern const VectorIntrinsic gGatherMostSignificantBytesV4;
    extern const VectorIntrinsic gSqrt2s;
    extern const VectorIntrinsic gIVector;
    extern const VectorIntrinsic gJVector;
    extern const VectorIntrinsic gKVector;
    extern const VectorIntrinsic gLVector;
    extern const VectorIntrinsic gSignMaskXYZ;
    extern const VectorIntrinsic gSignMaskXZ;
    extern const VectorIntrinsic gSignMaskYW;
    extern const VectorIntrinsic gExponentAndMantissa;
    extern const VectorIntrinsic gCrossProductPermuteConstant;
    extern const VectorIntrinsic g2PI;
    extern const VectorIntrinsic g2PIReciprocal;
    extern const VectorIntrinsic gMergeHighPermuteConstant;
    extern const VectorIntrinsic gMergeLowPermuteConstant;

    // Table of values for swizzle store macro
    const size_t NUM_SWIZZLE_STORE_CONSTANTS = 16;  // (4 choices for Dest) * (4 choices for Src)
    extern const VectorIntrinsic gSwizzleStoreConstants[NUM_SWIZZLE_STORE_CONSTANTS];

    // Table with two boolean vectors (false and true)
    extern const VectorIntrinsic gBooleanConstants[2];

    // Table of values for the shift left double macro
    const size_t NUM_SHIFT_LEFT_DOUBLE_CONSTANTS = 17;  // 17 possible shifts
    extern const VectorIntrinsic gShiftLeftDoubleConstants[NUM_SHIFT_LEFT_DOUBLE_CONSTANTS];

    // Table of values for splatting a vector element (32bits) to all elements
    // of the result.
    const size_t NUM_SPLAT_ELEMENT_CONSTANTS = 4;   // 4 choices for element index to splat
    extern const VectorIntrinsic gSplatElementPermuteConstants[NUM_SPLAT_ELEMENT_CONSTANTS];

    // The following constants are used only by the VPL.
    extern const VectorIntrinsic gMergeHighBytePermuteConstant;
    extern const VectorIntrinsic gMergeLowBytePermuteConstant;
    extern const VectorIntrinsic gMergeHighHalfPermuteConstant;
    extern const VectorIntrinsic gMergeLowHalfPermuteConstant;

    // Please don't call this method unless you have a valid reason to do so.
    void ForceVectorsToMemory(VectorIntrinsic *);

    extern const VectorIntrinsic gCosMinSlerpAngle;

    extern vector unsigned char gStore3Table0[4];
    extern vector unsigned char gStore3Table1[4];
    extern vector unsigned char gStore2Table0[4];
    extern vector unsigned char gStore2Table1[4];

} // namespace detail

} // namespace vpu
} // namespace math
} // namespace rw

#endif
