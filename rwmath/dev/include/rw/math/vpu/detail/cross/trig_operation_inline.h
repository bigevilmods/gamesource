// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_TRIG_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_CROSS_TRIG_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#include <math.h>

#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/vector4.h"

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

RW_MATH_FORCE_INLINE VecFloat NormalizeAngle(VecFloat::InParam a)
{
    // TODO optimize using intrinsics and branchless code.
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

RW_MATH_INLINE void SinCos(VecFloat::InParam a, VecFloat& s_out, VecFloat& c_out)
{
    s_out = Sin(a);
    c_out = Cos(a);
}

RW_MATH_INLINE Vector4 SinCos(VecFloat::InParam a)
{
    return Vector4( Sin(a), Cos(a), GetVecFloat_Zero(), GetVecFloat_Zero() );
}

RW_MATH_INLINE Vector4 SinCosFast(VecFloat::InParam a)
{
    return SinCos(a);
}

// store sin(a) in x component, cos(a) in y component, sin(b) in z component, cos(b) in w component
RW_MATH_INLINE Vector4 SinCosFast2(VecFloat::InParam a, VecFloat::InParam b)
{
    return Vector4( Sin(a), Cos(a), Sin(b), Cos(b) );
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif // RW_MATH_VPU_DETAIL_CROSS_TRIG_OPERATION_INLINE_H
