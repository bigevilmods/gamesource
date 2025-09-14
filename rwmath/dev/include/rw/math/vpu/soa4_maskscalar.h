// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_MASKSCALAR_H
#define RW_MATH_SOA4_MASKSCALAR_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/soa4_soa4base.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask4.h"
#include "rw/math/vpu/vector_intrinsic.h"

namespace rw
{
namespace math
{
namespace vpu
{


class Soa4MaskScalar
{
public:
    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const Soa4MaskScalar InParam;
    #else
        typedef const Soa4MaskScalar& InParam;
    #endif

public:
    RW_MATH_FORCE_INLINE Soa4MaskScalar();
    RW_MATH_FORCE_INLINE Soa4MaskScalar(const Soa4MaskScalar&);
    RW_MATH_FORCE_INLINE Soa4MaskScalar& operator =(const Soa4MaskScalar&);
    RW_MATH_FORCE_INLINE explicit Soa4MaskScalar(VectorIntrinsicInParam);

public:
    VectorIntrinsic mV;
};

} // namespace vpu
} // namespace math
} // namespace rw

#include "rw/math/vpu/detail/soa4_maskscalar_type_inline.h"

#endif
