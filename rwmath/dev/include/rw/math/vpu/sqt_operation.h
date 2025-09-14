// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SQT_OPERATION_H
#define RW_MATH_VPU_SQT_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/sqt.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

    
    RW_MATH_FORCE_INLINE bool operator ==(Sqt::InParam a, Sqt::InParam b);
    RW_MATH_FORCE_INLINE bool operator !=(Sqt::InParam a, Sqt::InParam b);
    RW_MATH_FORCE_INLINE Sqt& operator *=(Sqt& s, Sqt::InParam b);
    RW_MATH_FORCE_INLINE Sqt& operator *=(Sqt& s, QuatPos::InParam qp);
    RW_MATH_FORCE_INLINE Sqt operator * (Sqt::InParam a, Sqt::InParam b);
    RW_MATH_FORCE_INLINE Sqt operator * (Sqt::InParam s, QuatPos::InParam qp);

    RW_MATH_FORCE_INLINE bool IsSimilar(Sqt::InParam a, Sqt::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsValid(Sqt::InParam s);

    RW_MATH_FORCE_INLINE Sqt SqtFromMatrix44(Matrix44::InParam m);
    RW_MATH_FORCE_INLINE Sqt SqtFromOrthonormalMatrix44(Matrix44::InParam m);
    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromSqt(Sqt::InParam s);
    RW_MATH_FORCE_INLINE Sqt SqtFromMatrix44Affine(Matrix44Affine::InParam m);
    RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromSqt(Sqt::InParam s);

    RW_MATH_FORCE_INLINE Sqt Mult(Sqt::InParam s0, Sqt::InParam s1);
    RW_MATH_FORCE_INLINE Sqt Mult(Sqt::InParam s, QuatPos::InParam qp);

    RW_MATH_FORCE_INLINE Sqt SLerp(Sqt::InParam s0, Sqt::InParam s1, VecFloat::InParam t);
    RW_MATH_FORCE_INLINE Sqt Lerp(Sqt::InParam s0, Sqt::InParam s1, VecFloat::InParam t);

    RW_MATH_FORCE_INLINE Sqt Select(MaskScalar::InParam mask, Sqt::InParam trueValue, Sqt::InParam falseValue );

    RW_MATH_FORCE_INLINE Sqt Inverse(Sqt::InParam in);

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/sqt_operation_inline.h"
#endif

#endif
