// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_MASK4_H
#define RW_MATH_VPU_SOA4_MASK4_H

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

class Soa4Mask4
{
public:
    typedef const Soa4Mask4& InParam;

public:
    RW_MATH_FORCE_INLINE Soa4Mask4();
    RW_MATH_FORCE_INLINE Soa4Mask4(const Soa4Mask4&);
    RW_MATH_FORCE_INLINE Soa4Mask4& operator =(const Soa4Mask4&);

    RW_MATH_FORCE_INLINE explicit Soa4Mask4(Soa4MaskScalar::InParam s);
    RW_MATH_FORCE_INLINE Soa4Mask4(Soa4MaskScalar::InParam, Soa4MaskScalar::InParam, Soa4MaskScalar::InParam, Soa4MaskScalar::InParam);
    RW_MATH_FORCE_INLINE Soa4Mask4(VectorIntrinsicInParam, VectorIntrinsicInParam, VectorIntrinsicInParam, VectorIntrinsicInParam);

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& X() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& X();

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& Y() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& Y();

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& Z() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& Z();

    RW_MATH_FORCE_INLINE const Soa4MaskScalar& W() const;
    RW_MATH_FORCE_INLINE       Soa4MaskScalar& W();

public:
    Soa4MaskScalar x;
    Soa4MaskScalar y;
    Soa4MaskScalar z;
    Soa4MaskScalar w;
};

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_mask4_type_inline.h"
#endif

#endif // RW_MATH_VPU_MASK2_H


