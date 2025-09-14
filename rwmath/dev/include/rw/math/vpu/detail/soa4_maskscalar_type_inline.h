// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_DETAIL_MASKSCALAR_TYPE_INLINE_H
#define RW_MATH_SOA4_DETAIL_MASKSCALAR_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4MaskScalar::Soa4MaskScalar()
{
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE Soa4MaskScalar::Soa4MaskScalar(const Soa4MaskScalar &v)
    :mV(v.mV)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
}

RW_MATH_FORCE_INLINE Soa4MaskScalar& Soa4MaskScalar::operator =(const Soa4MaskScalar &v)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}

RW_MATH_FORCE_INLINE Soa4MaskScalar::Soa4MaskScalar(VectorIntrinsicInParam v)
    :mV(v)
{
}


} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_SOA4_DETAIL_MASKSCALAR_TYPE_INLINE_H
