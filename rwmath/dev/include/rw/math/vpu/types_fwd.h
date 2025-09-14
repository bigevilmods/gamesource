// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_TYPES_FWD_H
#define RW_MATH_VPU_TYPES_FWD_H

#if defined _MSC_VER
    #pragma once
#endif


#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_axis.h"
#include "rw/math/version.h"

namespace rw
{
namespace math
{

namespace vpu
{

    class VecFloat;
    class VecFloatRefIndex;
    template<VectorAxis INDEX> class VecFloatRef;

    class MaskScalar;
    class Mask2;
    class Mask3;
    class Mask4;
    class Vector2;
    class Vector3;
    class Vector4;
    class Vector3Plus;
    class Quaternion;
    class QuatPos;
    class Plane;
    class Sqt;

    class Matrix33;
    class Matrix44;
    class Matrix44Affine;
    class Matrix44AffinePacked;

    class Soa4Float;
    class Soa4Vector2;
    class Soa4Vector3;
    class Soa4Vector4;

    class Soa4MaskScalar;
    class Soa4Mask2;
    class Soa4Mask3;
    class Soa4Mask4;
} // namespace vpu

using namespace ::rw::math::vpu;

} // namespace math
} // namespace rw

#endif
