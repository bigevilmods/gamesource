// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_MASKSCALAR_OPERATION_H
#define RW_MATH_SOA4_MASKSCALAR_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/mask4.h"
#include "rw/math/vpu/soa4_maskscalar.h"
#include "rw/math/vpu/vec_float.h"

namespace rw
{
namespace math
{
namespace vpu
{

//
// Conversions to SoA4
//
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromBool(bool v);
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromBools(bool v0, bool v1, bool v2, bool v3);
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromUInt(uint32_t v);
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromUInts(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromMaskScalar(MaskScalar::InParam v);
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromMaskScalars(MaskScalar::InParam v0, MaskScalar::InParam v1, MaskScalar::InParam v2, MaskScalar::InParam v3 );
RW_MATH_FORCE_INLINE Soa4MaskScalar  Soa4MaskScalarFromMask4(Mask4::InParam v);

//
// Conversions back to AoS
//
RW_MATH_FORCE_INLINE void Soa4MaskScalarToBools(Soa4MaskScalar::InParam v, bool& r0, bool& r1, bool& r2, bool& r3 );
RW_MATH_FORCE_INLINE void Soa4MaskScalarToMaskScalars(Soa4MaskScalar::InParam v, MaskScalar& r0, MaskScalar& r1, MaskScalar& r2, MaskScalar& r3);
RW_MATH_FORCE_INLINE Mask4 Soa4MaskScalarToMask4(Soa4MaskScalar::InParam v);

RW_MATH_FORCE_INLINE bool IsAnyTrue(Soa4MaskScalar::InParam a);
RW_MATH_FORCE_INLINE bool IsAllTrue(Soa4MaskScalar::InParam a);
RW_MATH_FORCE_INLINE bool IsAnyFalse(Soa4MaskScalar::InParam a);
RW_MATH_FORCE_INLINE bool IsAllFalse(Soa4MaskScalar::InParam a);

RW_MATH_FORCE_INLINE Soa4MaskScalar Not(Soa4MaskScalar::InParam a);
RW_MATH_FORCE_INLINE Soa4MaskScalar And(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar Or(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar Xor(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompEqual(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b);
RW_MATH_FORCE_INLINE Soa4MaskScalar CompNotEqual(Soa4MaskScalar::InParam a, Soa4MaskScalar::InParam b);

} // namespace vpu
} // namespace math
} // namespace rw

#include "rw/math/vpu/detail/soa4_maskscalar_operation_inline.h"

#endif // RW_MATH_SOA4_MASKSCALAR_OPERATION_H
