// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_QUATPOS_OPERATION_H
#define RW_MATH_VPU_QUATPOS_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/quatpos.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

    
    RW_MATH_FORCE_INLINE bool operator ==(QuatPos::InParam a, QuatPos::InParam b);
    RW_MATH_FORCE_INLINE bool operator !=(QuatPos::InParam a, QuatPos::InParam b);

    RW_MATH_FORCE_INLINE bool IsSimilar(QuatPos::InParam a, QuatPos::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsValid(QuatPos::InParam q);
    
    RW_MATH_FORCE_INLINE QuatPos Mult(QuatPos::InParam a, QuatPos::InParam b);

    RW_MATH_FORCE_INLINE QuatPos QuatPosFromMatrix44(Matrix44::InParam);
    
    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromQuatPos(QuatPos::InParam);
    
    RW_MATH_FORCE_INLINE QuatPos QuatPosFromMatrix44Affine(Matrix44Affine::InParam);
    
    RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromQuatPos(QuatPos::InParam);
    
    RW_MATH_FORCE_INLINE Vector3 TransformPoint(Vector3::InParam point, QuatPos::InParam q);
   
    RW_MATH_FORCE_INLINE Vector3 QuatPosGetMatrix44AffineRowX(QuatPos::InParam quat);
    
    RW_MATH_FORCE_INLINE Vector3 QuatPosGetMatrix44AffineRowY(QuatPos::InParam quat);

    RW_MATH_FORCE_INLINE Vector3 QuatPosGetMatrix44AffineRowZ(QuatPos::InParam quat);

    RW_MATH_FORCE_INLINE QuatPos QuatPosFromAxisRotationAngle(Vector3::InParam axis, VecFloat::InParam lrAngleInRadians);

    RW_MATH_FORCE_INLINE QuatPos Select(MaskScalar::InParam mask, QuatPos::InParam trueValue, QuatPos::InParam falseValue );

    RW_MATH_FORCE_INLINE QuatPos Inverse(QuatPos::InParam in);

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/quatpos_operation_inline.h"
#endif

#endif
