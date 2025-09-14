// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_TRIG_H
#define RW_MATH_SOA4_TRIG_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/soa4_soa4base.h"
#include "rw/math/vpu/soa4_float.h"

namespace rw
{
namespace math
{
namespace vpu
{
    
    RW_MATH_FORCE_INLINE Soa4Float DegToRad(Soa4Float::InParam angle_degrees);
    RW_MATH_FORCE_INLINE Soa4Float RadToDeg(Soa4Float::InParam angle_radians);
    
    RW_MATH_FORCE_INLINE Soa4Float Sin(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Cos(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float Tan(Soa4Float::InParam a);
    
    RW_MATH_FORCE_INLINE Soa4Float ASin(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float ACos(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float ATan(Soa4Float::InParam a);
    RW_MATH_FORCE_INLINE Soa4Float ATan2(Soa4Float::InParam a, Soa4Float::InParam b);
    
    RW_MATH_FORCE_INLINE Soa4Float NormalizeAngle(Soa4Float::InParam a);
    
    RW_MATH_FORCE_INLINE void SinCos(Soa4Float::InParam a, Soa4Float& s_out, Soa4Float& c_out);
    
} // namespace vpu
} // namespace math
} // namespace rw

#include "rw/math/vpu/detail/soa4_trig_operation_inline.h"

#endif
