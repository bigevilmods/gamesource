// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_CONSTANTS_OPERATION_H
#define RW_MATH_SOA4_CONSTANTS_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/soa4_soa4base.h"
#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/soa4_float.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4MaskScalar GetSoa4MaskScalar_True();
RW_MATH_FORCE_INLINE Soa4MaskScalar GetSoa4MaskScalar_False();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeTwo();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeOne();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeHalf();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_NegativeZero();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_Zero();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_Half();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_One();
RW_MATH_FORCE_INLINE Soa4Float GetSoa4Float_Two();

} // namespace vpu
} // namespace math
} // namespace rw

#include "rw/math/vpu/detail/soa4_constants_operation_inline.h"

#endif
