// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_VEC_FLOAT_H
#define RW_MATH_SOA4_VEC_FLOAT_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/soa4_soa4base.h"

#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/vector4.h"

namespace rw
{
namespace math
{
namespace vpu
{

/*!
    \brief The Soa4Float class represents four scalar values.

    The Soa4Float class is implemented using hardware efficient vector types.
    Therefore, the Soa4Float should always be aligned on a 16 byte boundary.
*/
class Soa4Float
{
public:

    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const Soa4Float InParam;
    #else
        typedef const Soa4Float& InParam;
    #endif
    

    RW_MATH_FORCE_INLINE Soa4Float();
    RW_MATH_FORCE_INLINE Soa4Float(const Soa4Float& v);
    RW_MATH_FORCE_INLINE explicit Soa4Float(VectorIntrinsicInParam v);

    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;

    RW_MATH_FORCE_INLINE Soa4Float& operator =(const Soa4Float& v);

public:

    VectorIntrinsic  mV;
};

} // namespace vpu
} // namespace math
} // namespace rw


#include "rw/math/vpu/detail/soa4_float_type_inline.h"

#endif
