// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_TYPE_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_TYPE_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{

namespace math
{

namespace vpu
{

//
//  Implementation for VecFloat
//

RW_MATH_FORCE_INLINE VecFloat::VecFloat(VectorIntrinsicInParam v) 
    :mV(v)
{
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& VecFloat::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    const_cast<VectorIntrinsic*>(&mV)->f[3] = const_cast<VectorIntrinsic*>(&mV)->f[2] = const_cast<VectorIntrinsic*>(&mV)->f[1] = mV.f[0];
    return mV;
}

}

}

}

#endif
