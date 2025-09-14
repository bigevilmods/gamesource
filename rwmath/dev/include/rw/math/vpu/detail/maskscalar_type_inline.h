// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MASKSCALAR_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_MASKSCALAR_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE MaskScalar::MaskScalar()
{
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE MaskScalar::MaskScalar(const MaskScalar &v)
    :mV(v.mV)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
}

RW_MATH_FORCE_INLINE MaskScalar& MaskScalar::operator =(const MaskScalar &v)
{
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}
#endif

RW_MATH_FORCE_INLINE MaskScalar::MaskScalar(VectorIntrinsicInParam v)
    :mV(v)
{
}

RW_MATH_FORCE_INLINE MaskScalar::MaskScalar(bool v)
{
    RW_MATH_VPU_CreateVectorMaskFromBool(mV, v);
}

RW_MATH_FORCE_INLINE MaskScalar::MaskScalar(uint32_t v)
{
    detail::VectorIntrinsicUnion value;
    value.scalarUInt[0] = v;
    value.scalarUInt[1] = v;
    value.scalarUInt[2] = v;
    value.scalarUInt[3] = v;
    mV = value.vector;
}

RW_MATH_FORCE_INLINE bool MaskScalar::GetBool() const
{
#ifdef EA_PLATFORM_PS3_SPU
    bool result;
    RWMATH_VPU_DETAIL_MaskVectorXValueToBool(result, mV);
    return result;
#else
        bool equalToFalse;
        RW_MATH_VPU_IsEqualV4(equalToFalse, mV, RW_MATH_VPU_GetZero_R());
    #ifdef RW_MATH_ENABLE_ASSERTS
        bool equalToTrue;
        // An XOR with true is done to generate a value to compare against zero.  A
        // floating point compare can't be performed against "true" because it is NaN.
        VectorIntrinsic xorWithTrue;
        RW_MATH_VPU_Xor(xorWithTrue, mV, RW_MATH_VPU_GetTrue_R());
        RW_MATH_VPU_IsEqualV4(equalToTrue, xorWithTrue, RW_MATH_VPU_GetZero_R());
        RW_MATH_ASSERT_MSG( equalToFalse != equalToTrue, ("MaskScalar is not a boolean value.") );
    #endif
        return !equalToFalse;
#endif
}


RW_MATH_FORCE_INLINE uint32_t MaskScalar::ToUInt32() const
{
    detail::VectorIntrinsicUnion value;
    value.vector = mV;
    return value.scalarUInt[0];
}

RW_MATH_FORCE_INLINE void MaskScalar::SetTrue()
{
    RW_MATH_VPU_GetTrue(mV);
}

RW_MATH_FORCE_INLINE void MaskScalar::SetFalse()
{
    RW_MATH_VPU_GetZero(mV);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_DETAIL_MASKSCALAR_TYPE_INLINE_H
