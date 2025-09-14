// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_MASK3_H
#define RW_MATH_VPU_SOA4_MASK3_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/soa4_maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

class Soa4Mask3
{
public:
    typedef const Soa4Mask3& InParam;

public:
    RW_MATH_FORCE_INLINE Soa4Mask3();
    RW_MATH_FORCE_INLINE Soa4Mask3(const Soa4Mask3&);
    RW_MATH_FORCE_INLINE Soa4Mask3& operator =(const Soa4Mask3&);

    RW_MATH_FORCE_INLINE explicit Soa4Mask3(Soa4MaskScalar::InParam s);
    RW_MATH_FORCE_INLINE Soa4Mask3(Soa4MaskScalar::InParam, Soa4MaskScalar::InParam, Soa4MaskScalar::InParam);
    RW_MATH_FORCE_INLINE Soa4Mask3(VectorIntrinsicInParam, VectorIntrinsicInParam, VectorIntrinsicInParam);

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& X() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& X();

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& Y() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& Y();

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& Z() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& Z();

public:
    Soa4MaskScalar x;
    Soa4MaskScalar y;
    Soa4MaskScalar z;
};

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_mask3_type_inline.h"
#endif

#endif // RW_MATH_VPU_MASK2_H

