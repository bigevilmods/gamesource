// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_DETAIL_CONSTANTS_OPERATION_INLINE_H
#define RW_MATH_SOA4_DETAIL_CONSTANTS_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/soa4_float.h"

namespace rw
{
namespace math
{
namespace vpu
{

//
// Mask Constants
//
RW_MATH_FORCE_INLINE Soa4MaskScalar GetSoa4MaskScalar_True()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetTrue(result);
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar GetSoa4MaskScalar_False()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetZero(result);
    return Soa4MaskScalar(result);
}

//
// Soa4Float Constants
//
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeTwo()
{
    return Soa4Float(RW_MATH_VPU_GetNegTwo_R());
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeOne()
{
    return Soa4Float(RW_MATH_VPU_GetNegOne_R());
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeHalf()
{
    return Soa4Float(RW_MATH_VPU_GetNegHalf_R());
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeZero()
{
    VectorIntrinsic result;
    RW_MATH_VPU_GetSignMaskXYZW(result);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_Zero()
{
    return Soa4Float(RW_MATH_VPU_GetZero_R());
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_Half()
{
    return Soa4Float(RW_MATH_VPU_GetHalf_R());
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_One()
{
    return Soa4Float(RW_MATH_VPU_GetOne_R());
}

RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_Two()
{
    return Soa4Float(RW_MATH_VPU_GetTwo_R());
}


} // namespace vpu
} // namespace math
} // namespace rw

#endif
