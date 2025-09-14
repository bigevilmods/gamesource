// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_TRIG_H
#define RW_MATH_VPU_TRIG_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vec_float.h"

namespace rw
{
namespace math
{
namespace vpu
{
    
    RW_MATH_FORCE_INLINE VecFloat DegToRad(VecFloat::InParam angle_degrees);
    RW_MATH_FORCE_INLINE VecFloat RadToDeg(VecFloat::InParam angle_radians);
    
    RW_MATH_FORCE_INLINE VecFloat Sin(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat Cos(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat Tan(VecFloat::InParam a);
    
    RW_MATH_FORCE_INLINE VecFloat ASin(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat ACos(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat ATan(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat ATan2(VecFloat::InParam a, VecFloat::InParam b);
    
    RW_MATH_FORCE_INLINE VecFloat NormalizeAngle(VecFloat::InParam a);
    
    RW_MATH_FORCE_INLINE void SinCos(VecFloat::InParam a, VecFloat& s_out, VecFloat& c_out);
    
    //! \return Resulting vector [sin(a), cos(a), 0, 0]
    RW_MATH_FORCE_INLINE vpu::Vector4 SinCos(VecFloat::InParam a);    

    //! \return Resulting vector [sin(a), cos(a), 0, 0]
    RW_MATH_FORCE_INLINE vpu::Vector4 SinCosFast(VecFloat::InParam a);    

    //! \return Resulting vector [sin(a), cos(a), sin(b), cos(b)]
    RW_MATH_FORCE_INLINE vpu::Vector4 SinCosFast2(VecFloat::InParam a, VecFloat::InParam b);  

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/trig_operation_inline.h"
#endif

#endif
