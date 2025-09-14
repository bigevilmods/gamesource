// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SCALAR_H
#define RW_MATH_VPU_SCALAR_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/constants.h"

namespace rw
{
namespace math
{
namespace vpu
{

    RW_MATH_FORCE_INLINE int32_t FastFloatToInt32(float a);

    RW_MATH_FORCE_INLINE int32_t IntFloor(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE int32_t IntCeil(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE int32_t IntRound(VecFloat::InParam a);
    
    RW_MATH_FORCE_INLINE VecFloat Floor(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat Ceil(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat Round(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat Modulo(VecFloat::InParam a, VecFloat::InParam modulus);

    template <class T> RW_MATH_FORCE_INLINE T Abs(const T& a);
    template <class T> RW_MATH_FORCE_INLINE T Min(const T& a, const T& b);
    template <class T> RW_MATH_FORCE_INLINE T Min(const T& a, const T& b, const T& c);
    template <class T> RW_MATH_FORCE_INLINE T Max(const T& a, const T& b);
    template <class T> RW_MATH_FORCE_INLINE T Max(const T& a, const T& b, const T& c);
    
    template <class T> RW_MATH_FORCE_INLINE T Abs(const T& a) { return a >= static_cast<T>(0.0f) ? a : -a; }
    template <class T> RW_MATH_FORCE_INLINE T Min(const T& a, const T& b) { return (a < b? a : b); }
    template <class T> RW_MATH_FORCE_INLINE T Min(const T& a, const T& b, const T& c) { return Min(Min(a, b), c); }
    template <class T> RW_MATH_FORCE_INLINE T Max(const T& a, const T& b) { return (a > b? a : b); }
    template <class T> RW_MATH_FORCE_INLINE T Max(const T& a, const T& b, const T& c) { return Max(Max(a, b), c); }

    RW_MATH_FORCE_INLINE float Abs(const float a);
    RW_MATH_FORCE_INLINE double Abs(const double a);

    RW_MATH_FORCE_INLINE float Min(const float a, const float b);
    RW_MATH_FORCE_INLINE double Min(const double a, const double b);

    RW_MATH_FORCE_INLINE float Max(const float a, const float b);
    RW_MATH_FORCE_INLINE double Max(const double a, const double b);

    template <>
    RW_MATH_FORCE_INLINE VecFloat Abs<VecFloat>(const VecFloat& a);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE VecFloat Abs(const VecFloatRef<INDEX>& a);

    template <>
    RW_MATH_FORCE_INLINE VecFloat Min<VecFloat>(const VecFloat& a, const VecFloat& b);

    template <>
    RW_MATH_FORCE_INLINE VecFloat Min<VecFloat>(const VecFloat& a, const VecFloat& b, const VecFloat& c);

    template <>
    RW_MATH_FORCE_INLINE VecFloat Max<VecFloat>(const VecFloat& a, const VecFloat& b);

    template <>
    RW_MATH_FORCE_INLINE VecFloat Max<VecFloat>(const VecFloat& a, const VecFloat& b, const VecFloat& c);

    RW_MATH_FORCE_INLINE VecFloat Lerp(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam t);

    RW_MATH_FORCE_INLINE VecFloat Reciprocal(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat ReciprocalFast(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat ReciprocalEstimate(VecFloat::InParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic Reciprocal(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic ReciprocalFast(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic ReciprocalFastV2(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic ReciprocalEstimate(VectorIntrinsicInParam a);
    
    RW_MATH_FORCE_INLINE VecFloat Sqrt(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat SqrtFast(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE VecFloat SqrtEstimate(VecFloat::InParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic Sqrt(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic SqrtFast(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic SqrtFastV2(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic SqrtEstimate(VectorIntrinsicInParam a);

    RW_MATH_FORCE_INLINE VecFloat InvSqrt(VecFloat::InParam a);    
    RW_MATH_FORCE_INLINE VecFloat InvSqrtFast(VecFloat::InParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic InvSqrt(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic InvSqrtFast(VectorIntrinsicInParam a);
	RW_MATH_FORCE_INLINE VectorIntrinsic InvSqrtFastV2(VectorIntrinsicInParam a);
    RW_MATH_FORCE_INLINE VectorIntrinsic InvSqrtEstimate(VectorIntrinsicInParam a);

    RW_MATH_FORCE_INLINE VecFloat DivSqrt(VecFloat::InParam a, VecFloat::InParam dividend);
    RW_MATH_FORCE_INLINE VecFloat DivSqrtFast(VecFloat::InParam a, VecFloat::InParam dividend);
    
    RW_MATH_FORCE_INLINE VecFloat Pow(VecFloat::InParam base, VecFloat::InParam exponent);    
    RW_MATH_FORCE_INLINE VecFloat Clamp(VecFloat::InParam value, VecFloat::InParam min, VecFloat::InParam max);
    RW_MATH_FORCE_INLINE VecFloat ClampToNegPosRange(VecFloat::InParam value, VecFloat::InParam range);

    //! This routine was placed in the vpu namespace because users were
    //! accidentally calling rw::math::vpu::Clamp for float values.  However,
    //! users should instead be calling rw::math::fpu::Clamp directly.
    RW_MATH_FORCE_INLINE float Clamp(const float value, const float min, const float max);

    //! This routine was placed in the vpu namespace because users were
    //! accidentally calling rw::math::vpu::Clamp for double values.  However,
    //! users should instead be calling rw::math::fpu::Clamp directly.
    RW_MATH_FORCE_INLINE double Clamp(const double value, const double min, const double max);
    
    RW_MATH_FORCE_INLINE VecFloat Log(VecFloat::InParam a);

    
    RW_MATH_FORCE_INLINE VecFloat Exp(VecFloat::InParam a);
    
    RW_MATH_FORCE_INLINE VecFloat Sgn(VecFloat::InParam a);
    
    RW_MATH_FORCE_INLINE VecFloat SgnNonZero(VecFloat::InParam a);

    RW_MATH_FORCE_INLINE bool IsValid(VecFloat::InParam value);
    RW_MATH_FORCE_INLINE bool IsNaN(VecFloat::InParam value);    
    RW_MATH_FORCE_INLINE bool IsZero(VecFloat::InParam value, VecFloat::InParam tolerance = VecFloat(EPSILON));

    #if !defined _MSC_VER //! \todo following code cause MSVC to produce some weird compiler error. As far as we know, GCC is okay.
        template<VectorAxis INDEX>
        RW_MATH_FORCE_INLINE bool IsZero(const VecFloatRef<INDEX> value, VecFloat::InParam tolerance = VecFloat(EPSILON));
    #endif
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, VecFloatRef<INDEX> b,        VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, const float& b,              VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, VecFloat::InParam b,         VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
   
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX> a,  VecFloat::InParam b,         VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));    
    template<VectorAxis INDEX1, VectorAxis INDEX2>
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b,       VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX> a,  VecFloatRefIndex::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX> a,  const float& b,              VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
   
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b,        VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));    
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));    
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, const float& b,              VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloat::InParam b,         VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
   
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, VecFloatRef<INDEX> b,        VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, VecFloat::InParam b,         VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, const float& b,              VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));

    RW_MATH_FORCE_INLINE VecFloat MultAdd(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam c);

    RW_MATH_FORCE_INLINE MaskScalar CompLessThan(VecFloat::InParam a, VecFloat::InParam b);
    RW_MATH_FORCE_INLINE MaskScalar CompGreaterThan(VecFloat::InParam a, VecFloat::InParam b);
    RW_MATH_FORCE_INLINE MaskScalar CompLessEqual(VecFloat::InParam a, VecFloat::InParam b);
    RW_MATH_FORCE_INLINE MaskScalar CompGreaterEqual(VecFloat::InParam a, VecFloat::InParam b);
    RW_MATH_FORCE_INLINE MaskScalar CompEqual(VecFloat::InParam a, VecFloat::InParam b);
    RW_MATH_FORCE_INLINE MaskScalar CompNotEqual(VecFloat::InParam a, VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloat   Select(MaskScalar::InParam mask, VecFloat::InParam trueValue, VecFloat::InParam falseValue );
} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/scalar_operation_inline.h"
#endif

#endif
