// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR3_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR3_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_INLINE Vector3 SLerp(Vector3::InParam v0, Vector3::InParam v1, VecFloat::InParam t)
{
    RW_MATH_ASSERT_MSG(Abs(MagnitudeSquared(v0)-GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f)), ("v0 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(MagnitudeSquared(v1)-GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f)), ("v1 must be of unit length") );
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds") );

    VecFloat dp = Dot(v0,v1);
    
    if(dp > Cos(VecFloat(VECTOR_MIN_SLERP_ANGLE)))  // switch to nlerp if angle < VECTOR_MIN_SLERP_ANGLE
    {
        return NLerp(v0,v1,t);
    }

    VecFloat theta = ACos(dp);

    Vector3 a,b;
    a = Mult(v0, Sin((GetVecFloat_One()-t)*theta)/Sin(theta));
    b = Mult(v1, Sin(t*theta)/Sin(theta));
    return Add(a, b);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
