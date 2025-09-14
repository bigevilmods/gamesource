// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SOA4_VECTOR4_H
#define RW_MATH_VPU_SOA4_VECTOR4_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/soa4_float.h"
#include "rw/math/vpu/vector_intrinsic.h"

namespace rw {
namespace math {
namespace vpu {

//! Vector with four components
class Soa4Vector4
{
public:
    typedef const Soa4Vector4& InParam;

public:
    RW_MATH_FORCE_INLINE Soa4Vector4();
    RW_MATH_FORCE_INLINE Soa4Vector4(const Soa4Vector4& rhs);
    //! This constructor creates a vector with all its elements equal to \c scalar.
    RW_MATH_FORCE_INLINE explicit Soa4Vector4(Soa4Float::InParam scalar);

    RW_MATH_FORCE_INLINE Soa4Vector4(VectorIntrinsicInParam x,
                                     VectorIntrinsicInParam y,
                                     VectorIntrinsicInParam z,
                                     VectorIntrinsicInParam w);
    RW_MATH_FORCE_INLINE Soa4Vector4(Soa4Float::InParam x,
                                     Soa4Float::InParam y,
                                     Soa4Float::InParam z,
                                     Soa4Float::InParam w);

    RW_MATH_FORCE_INLINE Soa4Vector4& operator = (Soa4Vector4::InParam v);

    RW_MATH_FORCE_INLINE const Soa4Float& X() const;
    RW_MATH_FORCE_INLINE       Soa4Float& X();

    RW_MATH_FORCE_INLINE const Soa4Float& Y() const;
    RW_MATH_FORCE_INLINE       Soa4Float& Y();

    RW_MATH_FORCE_INLINE const Soa4Float& Z() const;
    RW_MATH_FORCE_INLINE       Soa4Float& Z();

    RW_MATH_FORCE_INLINE const Soa4Float& W() const;
    RW_MATH_FORCE_INLINE       Soa4Float& W();

public:
    Soa4Float x;
    Soa4Float y;
    Soa4Float z;
    Soa4Float w;
};

}

using ::rw::math::vpu::Soa4Vector4;

}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Soa4Vector4& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Soa4Vector4), sizeof(float));
}

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/soa4_vector4_type_inline.h"
#endif


#endif
