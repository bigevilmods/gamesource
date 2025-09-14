// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_DETAIL_VEC_FLOAT_TYPE_INLINE_H
#define RW_MATH_SOA4_DETAIL_VEC_FLOAT_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Soa4Float::Soa4Float() 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE Soa4Float::Soa4Float(const Soa4Float& v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
}

RW_MATH_FORCE_INLINE Soa4Float::Soa4Float(VectorIntrinsicInParam v) 
{
    // for alignment check, we must check before we do a assignment
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    mV = v;
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& Soa4Float::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE Soa4Float &Soa4Float::operator =(const Soa4Float &v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
