// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_MASK2_H
#define RW_MATH_VPU_SOA4_MASK2_H

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

class Soa4Mask2
{
public:
    typedef const Soa4Mask2& InParam;

public:
    RW_MATH_FORCE_INLINE Soa4Mask2();
    RW_MATH_FORCE_INLINE Soa4Mask2(const Soa4Mask2&);
    RW_MATH_FORCE_INLINE Soa4Mask2& operator =(const Soa4Mask2&);

    RW_MATH_FORCE_INLINE explicit Soa4Mask2(Soa4MaskScalar::InParam s);
    RW_MATH_FORCE_INLINE Soa4Mask2(Soa4MaskScalar::InParam, Soa4MaskScalar::InParam);
    RW_MATH_FORCE_INLINE Soa4Mask2(VectorIntrinsicInParam, VectorIntrinsicInParam);

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& X() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& X();

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& Y() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& Y();

public:
    Soa4MaskScalar x;
    Soa4MaskScalar y;
};

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_mask2_type_inline.h"
#endif

#endif // RW_MATH_VPU_MASK2_H
