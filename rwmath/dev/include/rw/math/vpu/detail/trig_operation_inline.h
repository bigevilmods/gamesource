// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_TRIG_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_TRIG_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/vector4.h"

#if defined EA_PLATFORM_PS3 || defined EA_PLATFORM_PS3_SPU || defined EA_PLATFORM_XENON || defined EA_PLATFORM_WINDOWS
    // VPU trig implementations
    namespace rw
    {
    namespace math
    {
    namespace vpu
    {

    RW_MATH_INLINE VecFloat Sin(VecFloat::InParam a)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Sin(result, a.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE VecFloat Cos(VecFloat::InParam a)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Cos(result, a.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE VecFloat Tan(VecFloat::InParam a)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_Tan(result, a.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE VecFloat ASin(VecFloat::InParam a)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_ASin(result, a.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE VecFloat ACos(VecFloat::InParam a)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_ACos(result, a.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE VecFloat ATan(VecFloat::InParam a)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_ATan(result, a.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE VecFloat ATan2(VecFloat::InParam a, VecFloat::InParam b)
    {
        VectorIntrinsic result;
        RW_MATH_VPU_ATan2(result, a.mV, b.mV);
        return VecFloat(result);
    }

    RW_MATH_INLINE void SinCos(VecFloat::InParam a, VecFloat& s_out, VecFloat& c_out)
    {
        rwMathAlignmentFunctionCheckAssert(a, sizeof(VectorIntrinsic));
        rwMathAlignmentFunctionCheckAssert(s_out, sizeof(VectorIntrinsic));
        rwMathAlignmentFunctionCheckAssert(c_out, sizeof(VectorIntrinsic));
        RW_MATH_VPU_SinCos( s_out.mV, c_out.mV, a.mV );
    }

    } // namespace vpu
    } // namespace math
    } // namespace rw
#else
    
    // fpu scalar implementations
    #include <math.h>

    namespace rw
    {
    namespace math
    {
    namespace vpu
    {

    RW_MATH_INLINE VecFloat Sin(VecFloat::InParam a)
    {
        float val = a.GetFloat();
        val = sinf(val);
        return VecFloat(val);
    }

    RW_MATH_INLINE VecFloat Cos(VecFloat::InParam a)
    {
        float val = a.GetFloat();
        val = cosf(val);
        return VecFloat(val);
    }

    RW_MATH_INLINE VecFloat Tan(VecFloat::InParam a)
    {
        float val = a.GetFloat();
        val = tanf(val);
        return VecFloat(val);
    }

    RW_MATH_INLINE VecFloat ASin(VecFloat::InParam a)
    {
        float val = a.GetFloat();
        val = asinf(val);
        return VecFloat(val);
    }

    RW_MATH_INLINE VecFloat ACos(VecFloat::InParam a)
    {
        float val = a.GetFloat();
        val = acosf(val);
        return VecFloat(val);
    }

    RW_MATH_INLINE VecFloat ATan(VecFloat::InParam a)
    {
        float val = a.GetFloat();
        val = atanf(val);
        return VecFloat(val);
    }

    RW_MATH_INLINE VecFloat ATan2(VecFloat::InParam a, VecFloat::InParam b)
    {
        return VecFloat( atan2f(a.GetFloat(), b.GetFloat()) );
    }

    RW_MATH_INLINE void SinCos(VecFloat::InParam a, VecFloat& s_out, VecFloat& c_out)
    {
        s_out = Sin(a);
        c_out = Cos(a);
    }

    } // namespace vpu
    } // namespace math
    } // namespace rw

#endif


namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE VecFloat DegToRad(VecFloat::InParam angle_degrees)
{
    return Mult(angle_degrees, DEGREES_TO_RADIANS );
}

RW_MATH_FORCE_INLINE VecFloat RadToDeg(VecFloat::InParam angle_radians)
{
    return Mult(angle_radians, RADIANS_TO_DEGREES );
}

RW_MATH_INLINE Vector4 SinCos(VecFloat::InParam a)
{
    return Vector4( Sin(a), Cos(a), GetVecFloat_Zero(), GetVecFloat_Zero() );
}

RW_MATH_INLINE Vector4 SinCosFast(VecFloat::InParam a)
{
    return SinCos(a);
}

#if defined EA_PLATFORM_PS3 || defined EA_PLATFORM_PS3_SPU || defined EA_PLATFORM_XENON
    RW_MATH_INLINE VecFloat NormalizeAngle(VecFloat::InParam a)
    {
        VectorIntrinsic floorScaledAngle, scaledAngle, angle, valueToSubtract, scaledAngleGreaterMask;
        RW_MATH_VPU_Mult(scaledAngle, a.mV, rw::math::vpu::detail::g2PIReciprocal );
        RW_MATH_VPU_Floor( floorScaledAngle, scaledAngle );
        scaledAngle = RW_MATH_VPU_Subtract_R( scaledAngle, floorScaledAngle);

        // Pseudo code for next block of code:
        //   if( scaledAngle > 0.5f )
        //   {
        //       scaledAngle = scaledAngle - 1.0f;
        //   }
        scaledAngleGreaterMask = RW_MATH_VPU_IsGreater_R( scaledAngle, RW_MATH_VPU_GetHalf_R() );
        valueToSubtract = RW_MATH_VPU_And_R( scaledAngleGreaterMask, RW_MATH_VPU_GetOne_R() );
        scaledAngle = RW_MATH_VPU_Subtract_R( scaledAngle, valueToSubtract );

        RW_MATH_VPU_Mult(angle, scaledAngle, rw::math::vpu::detail::g2PI)
        return VecFloat(angle);
    }
#else
    RW_MATH_FORCE_INLINE VecFloat NormalizeAngle(VecFloat::InParam a)
    {
        float angle = a.GetFloat();
        if( angle >= 0.0f )
        {
            float modAngle = fmodf(angle+PI, TWO_PI) - PI;
            return VecFloat(modAngle);
        }
        else
        {
            float modAngle = fmodf(angle-PI, TWO_PI) + PI;
            return VecFloat(modAngle);
        }
    }
#endif


// TODO this could be optimized into a single vector computation
RW_MATH_INLINE Vector4 SinCosFast2(VecFloat::InParam a, VecFloat::InParam b)
{
    // store sin(a) in x component, cos(a) in y component, sin(b) in z component, cos(b) in w component
    return Vector4( Sin(a), Cos(a), Sin(b), Cos(b) );
}


} // namespace vpu
} // namespace math
} // namespace rw

#endif
