// (c) Electronic Arts. All Rights Reserved.
#ifndef RW_MATH_SOA4_DETAIL_TRIG_OPERATION_INLINE_H
#define RW_MATH_SOA4_DETAIL_TRIG_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/constants.h"
#include "rw/math/vpu/soa4_float_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_INLINE Soa4Float DegToRad(Soa4Float::InParam angle_degrees)
{
    return Mult(angle_degrees, Soa4FloatFromFloat(::rw::math::fpu::DEGREES_TO_RADIANS));
}

RW_MATH_INLINE Soa4Float RadToDeg(Soa4Float::InParam angle_radians)
{
    return Mult(angle_radians, Soa4FloatFromFloat(::rw::math::fpu::RADIANS_TO_DEGREES));
}

RW_MATH_INLINE Soa4Float Sin(Soa4Float::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Sin(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float Cos(Soa4Float::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Cos(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float Tan(Soa4Float::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Tan(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float ASin(Soa4Float::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ASin(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float ACos(Soa4Float::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ACos(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float ATan(Soa4Float::InParam a)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ATan(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float ATan2(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ATan2(result, a.mV, b.mV);
    return Soa4Float(result);
}

RW_MATH_INLINE Soa4Float NormalizeAngle(Soa4Float::InParam a)
{
    VectorIntrinsic floorScaledAngle, scaledAngle, angle, valueToSubtract, scaledAngleGreaterMask;
    VectorIntrinsic twoPI, twoPIRecip;
    RW_MATH_VPU_Get2PI(twoPI);
    RW_MATH_VPU_Get2PIReciprocal(twoPIRecip);
    RW_MATH_VPU_Mult(scaledAngle, a.mV, twoPIRecip );
    RW_MATH_VPU_Floor( floorScaledAngle, scaledAngle );
    scaledAngle = RW_MATH_VPU_Subtract_R( scaledAngle, floorScaledAngle);

    // Pseudo code for next block of code:
    //   if( scaledAngle > 0.5f )
    //   {
    //       scaledAngle = scaledAngle - 1.0f;
    //   }
    scaledAngleGreaterMask = RW_MATH_VPU_IsGreater_R( scaledAngle, RW_MATH_VPU_GetHalf_R() );
    RW_MATH_VPU_And( valueToSubtract, scaledAngleGreaterMask, RW_MATH_VPU_GetOne_R() );
    scaledAngle = RW_MATH_VPU_Subtract_R( scaledAngle, valueToSubtract );

    RW_MATH_VPU_Mult(angle, scaledAngle, twoPI)
    return Soa4Float(angle);
}

RW_MATH_INLINE void SinCos(Soa4Float::InParam a, Soa4Float& s_out, Soa4Float& c_out)
{
    rwMathAlignmentFunctionCheckAssert(a, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(s_out, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(c_out, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SinCos( s_out.mV, c_out.mV, a.mV );
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
