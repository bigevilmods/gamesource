// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_WINDOWS_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_WINDOWS_VECTOR_INTRINSIC_H

#if defined _MSC_VER
    #pragma once
#endif

#include <xmmintrin.h>
#include "rw/math/detail/base.h"


namespace rw {
namespace math {
namespace vpu {

typedef __m128 VectorIntrinsic;
typedef const VectorIntrinsic & VectorIntrinsicInParam;

namespace detail
{
    extern const VectorIntrinsic gXYZMask;
    extern const VectorIntrinsic gXYZWMask;
    extern const VectorIntrinsic gNegOneOverTwoPi;
    extern const VectorIntrinsic gRemoveFraction;
    extern const VectorIntrinsic gOneHalf;
    extern const VectorIntrinsic gOneQuarter;
    extern const VectorIntrinsic gOneHalfPi;

    extern const VectorIntrinsic gTaylorCosXCoefficient;
    extern const VectorIntrinsic gTaylorCosX3Coefficient;
    extern const VectorIntrinsic gTaylorCosX5Coefficient;
    extern const VectorIntrinsic gTaylorCosX7Coefficient;
    extern const VectorIntrinsic gTaylorCosX9Coefficient;
    extern const VectorIntrinsic gTaylorCosX11Coefficient;
    extern const VectorIntrinsic gTaylorCosX13Coefficient;
    extern const VectorIntrinsic gTaylorCosX15Coefficient;
    extern const VectorIntrinsic gTaylorCosX17Coefficient;
    extern const VectorIntrinsic gTaylorCosX19Coefficient;

    extern const VectorIntrinsic gCosMinSlerpAngle;
} // namespace detail
} // namespace vpu

namespace vpl
{
namespace detail
{
    using ::rw::math::vpu::VectorIntrinsic;

    extern const VectorIntrinsic vecTwo31Float;
    extern const VectorIntrinsic vecNegTwo31Float;
    extern const VectorIntrinsic vecTwo32Float;
    extern const VectorIntrinsic vecZero;
    extern const VectorIntrinsic vecOne;
    extern const VectorIntrinsic maxHalfFloatFromUnsignedInteger;
    extern const VectorIntrinsic vecNegNaN;
    extern const VectorIntrinsic vecPosNaN;
    extern const FourUnsignedInt sixtyFourUInt;
    extern const VectorIntrinsic ScaleExponentArray[32];
    extern const FourUnsignedInt InsertionMaskArray[16];
}
}

} // namespace math
} // namespace rw

#endif
