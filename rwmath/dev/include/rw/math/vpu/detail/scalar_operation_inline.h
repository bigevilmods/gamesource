// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SCALAR_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_SCALAR_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"
#include <math.h>
#include "rw/math/detail/base.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/detail/scalar_operation_platform_inline.h"
#include "rw/math/vpu/detail/maskscalar_type_inline.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE VecFloat Floor(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Floor(result, a.mV);
    return VecFloat(result);
}

#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined EA_PLATFORM_PS3 || defined EA_PLATFORM_XENON)
RW_MATH_FORCE_INLINE int32_t IntFloor(VecFloat::InParam a) 
{
    // Optimizations are currently disabled for this function due to a SNC compiler bug:
    //   https://ps3.scedev.net/support/issue/65974#n748628 
    // TODO enable optimizations after compiler is fixed.
#ifdef __SNC__
    _Pragma("control %push O=1")
#endif
    int result;
    VectorIntrinsic flooredVec;
    // Floor the vector
    RW_MATH_VPU_Floor(flooredVec, a.mV);
    // Convert to integer
    VectorIntrinsic intVector = RWMATH_VPU_DETAIL_VecCtsxs(flooredVec, 0);
    // Store single element to 'result'
    RWMATH_VPU_DETAIL_VecStvewx(intVector, 0, &result);
    return result;
#ifdef __SNC__
    _Pragma("control %pop O=1")
#endif
}
#else
RW_MATH_FORCE_INLINE int32_t IntFloor(VecFloat::InParam a) 
{
    float val = a.GetFloat();
    val = floorf(val);
    return int(val);
}
#endif

RW_MATH_FORCE_INLINE VecFloat Ceil(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Ceil(result, a.mV);
    return VecFloat(result);
}

#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined EA_PLATFORM_PS3 || defined EA_PLATFORM_XENON)
RW_MATH_FORCE_INLINE int32_t IntCeil(VecFloat::InParam a) 
{
    // Optimizations are currently disabled for this function due to a SNC compiler bug:
    //   https://ps3.scedev.net/support/issue/65974#n748628 
    // TODO enable optimizations after compiler is fixed.
#ifdef __SNC__
    _Pragma("control %push O=1")
#endif
    int result;
    VectorIntrinsic ceiledVec;
    // Ceil the vector
    RW_MATH_VPU_Ceil(ceiledVec, a.mV);
    // Convert to integer
    VectorIntrinsic intVector = RWMATH_VPU_DETAIL_VecCtsxs(ceiledVec, 0);
    // Store single element to 'result'
    RWMATH_VPU_DETAIL_VecStvewx(intVector, 0, &result);
    return result;
#ifdef __SNC__
    _Pragma("control %pop O=1")
#endif
}
#else
RW_MATH_FORCE_INLINE int32_t IntCeil(VecFloat::InParam a) 
{
    float val = a.GetFloat();
    val = ceilf(val);
    return int(val);
}
#endif

RW_MATH_FORCE_INLINE VecFloat Round(VecFloat::InParam a) 
{
    return Floor(Add(a, GetVecFloat_Half()));
}

RW_MATH_FORCE_INLINE int32_t IntRound(VecFloat::InParam a) 
{
    return IntFloor(Add(a, GetVecFloat_Half()));
}

// TODO : properly document the results returned by Modulo.  It looks like it behaves the same as fmod.
#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined(EA_PLATFORM_PS3) || defined( EA_PLATFORM_PS3_SPU) || defined(EA_PLATFORM_XENON))
RW_MATH_FORCE_INLINE VecFloat Modulo(VecFloat::InParam a, VecFloat::InParam modulus) 
{
    RW_MATH_ASSERT_MSG( !IsZero(modulus), ("Modulus is zero.") );
    VectorIntrinsic divResult, truncDivResult, result;
    RW_MATH_VPU_Divide( divResult, a.mV, modulus.mV );
    RW_MATH_VPU_RoundToZero( truncDivResult, divResult );
    RW_MATH_VPU_MultSubNeg( result, truncDivResult, modulus.mV, a.mV );
    return VecFloat(result);
}

#else
RW_MATH_FORCE_INLINE VecFloat Modulo(VecFloat::InParam a, VecFloat::InParam modulus) 
{
    RW_MATH_ASSERT_MSG( !IsZero(modulus), ("Modulus is zero.") );
    float value = a.GetFloat();
    float mod = modulus.GetFloat();
    float modulo = value - ( (float)( (int)( ( value / mod ) ) ) * mod );
    return VecFloat(modulo);
}
#endif

template <>
RW_MATH_FORCE_INLINE VecFloat Abs<VecFloat>(const VecFloat& a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Abs(ret, a.mV);
    return VecFloat(ret);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Abs(const VecFloatRef<INDEX>& a) 
{
    VectorIntrinsic ret;
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I(aVI, a.mV, INDEX);
    RW_MATH_VPU_Abs(ret, aVI);
    return VecFloat(ret);
}

template <>
RW_MATH_FORCE_INLINE VecFloat Min<VecFloat>(const VecFloat& a, const VecFloat& b) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Min(result, a.mV, b.mV);
    return VecFloat(result);
}

template <>
RW_MATH_FORCE_INLINE VecFloat Min<VecFloat>(const VecFloat& a, const VecFloat& b, const VecFloat& c) 
{
    VectorIntrinsic r0, r1;
    RW_MATH_VPU_Min(r0, a.mV, b.mV);
    RW_MATH_VPU_Min(r1, r0, c.mV);
    return VecFloat(r1);
}

template <>
RW_MATH_FORCE_INLINE VecFloat Max<VecFloat>(const VecFloat& a, const VecFloat& b) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Max(result, a.mV, b.mV);
    return VecFloat(result);
}

template <>
RW_MATH_FORCE_INLINE VecFloat Max<VecFloat>(const VecFloat& a, const VecFloat& b, const VecFloat& c) 
{
    VectorIntrinsic r0, r1;
    RW_MATH_VPU_Max(r0, a.mV, b.mV);
    RW_MATH_VPU_Max(r1, r0, c.mV);
    return VecFloat(r1);
}

RW_MATH_FORCE_INLINE VecFloat Lerp(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam t) 
{
    RW_MATH_ASSERT_MSG(GetVecFloat_Zero() <= t && t <= GetVecFloat_One(), ("t is out of bounds") );
    VectorIntrinsic result, delta;
    RW_MATH_VPU_Subtract(delta, b.mV, a.mV);
    RW_MATH_VPU_MultAdd(result, delta, t.mV, a.mV);
    return VecFloat(result);
}

#if defined(EA_PLATFORM_REVOLUTION)
namespace detail
{
    VecFloat SqrtEstimate(VecFloat::InParam a);
    VecFloat SqrtFast(VecFloat::InParam a);
    VecFloat Sqrt(VecFloat::InParam a);
	VecFloat ReciprocalEstimate(VecFloat::InParam a);
	VecFloat ReciprocalFast(VecFloat::InParam a);
	VecFloat Reciprocal(VecFloat::InParam a);
	VecFloat InvSqrt(VecFloat::InParam a);
	VecFloat InvSqrtFast(VecFloat::InParam a);
}
RW_MATH_FORCE_INLINE VecFloat Reciprocal(VecFloat::InParam a) 
{
	return detail::Reciprocal(a);
}
RW_MATH_FORCE_INLINE VecFloat ReciprocalFast(VecFloat::InParam a) 
{
    return detail::ReciprocalFast(a);
}
RW_MATH_FORCE_INLINE VecFloat ReciprocalEstimate(VecFloat::InParam a) 
{
    return detail::ReciprocalEstimate(a);
}
RW_MATH_FORCE_INLINE VecFloat Sqrt(VecFloat::InParam a) 
{
    return detail::Sqrt(a);
}
RW_MATH_FORCE_INLINE VecFloat SqrtFast(VecFloat::InParam a) 
{
    return detail::SqrtFast(a);
}
RW_MATH_FORCE_INLINE VecFloat SqrtEstimate(VecFloat::InParam a) 
{
    return detail::SqrtEstimate(a);
}

RW_MATH_FORCE_INLINE VecFloat InvSqrt(VecFloat::InParam a) 
{
	return detail::InvSqrt(a);
}

RW_MATH_FORCE_INLINE VecFloat InvSqrtFast(VecFloat::InParam a) 
{
	return detail::InvSqrtFast(a);
}
#else
RW_MATH_FORCE_INLINE VecFloat Reciprocal(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Reciprocal( result, a.mV );
    return VecFloat(result);
}

RW_MATH_FORCE_INLINE VecFloat ReciprocalFast(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalFast( result, a.mV );
    return VecFloat(result);
}

RW_MATH_FORCE_INLINE VecFloat ReciprocalEstimate(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalEstimate( result, a.mV );
    return VecFloat(result);
}

RW_MATH_FORCE_INLINE VecFloat Sqrt(VecFloat::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRoot(ret, a.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat SqrtFast(VecFloat::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFast(ret, a.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat SqrtEstimate(VecFloat::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootEstimate(ret, a.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat InvSqrt(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalSquareRoot( result, a.mV);
    return VecFloat(result);
}

RW_MATH_FORCE_INLINE VecFloat InvSqrtFast(VecFloat::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalSquareRootFast( result, a.mV);
    return VecFloat(result);
}
#endif

RW_MATH_FORCE_INLINE VecFloat DivSqrt(VecFloat::InParam a, VecFloat::InParam dividend) 
{
    RW_MATH_ASSERT_MSG( a > VecFloat(EPSILON), ("a must be greater than epsilon") );
    VecFloat divSqrt = Divide(dividend, Sqrt( a ));
    return divSqrt;
}

RW_MATH_FORCE_INLINE VecFloat DivSqrtFast(VecFloat::InParam a, VecFloat::InParam dividend) 
{
    return DivSqrt(a, dividend);
}

RW_MATH_FORCE_INLINE VecFloat Pow(VecFloat::InParam base, VecFloat::InParam exponent) 
{
#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU))
    return VecFloat( powf4(base.mV, exponent.mV) );
#elif defined(EA_PLATFORM_XENON)
    return VecFloat( RWMATH_Pow(base.mV, exponent.mV) );
#else
    return VecFloat( powf(base.GetFloat(), exponent.GetFloat()) );
#endif
}


RW_MATH_FORCE_INLINE VecFloat Clamp(VecFloat::InParam value, VecFloat::InParam min, VecFloat::InParam max) 
{
    return Min(max, Max(min, value));
}

RW_MATH_FORCE_INLINE VecFloat ClampToNegPosRange(VecFloat::InParam value, VecFloat::InParam range)
{
    VectorIntrinsic result;
    RW_MATH_VPU_ClampToNegPosRange( result, value.mV, range.mV );
    return VecFloat(result);
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE float Clamp(const float value, const float min, const float max) 
{
    return Min(max, Max(min, value));
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE double Clamp(const double value, const double min, const double max) 
{
    return Min(max, Max(min, value));
}

RW_MATH_FORCE_INLINE VecFloat Log(VecFloat::InParam a) 
{
#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU))
    return VecFloat( logf4(a.mV) );
#elif defined(EA_PLATFORM_XENON)
    // Call Xenon's Log function which calculates a result with a base of 2.
    // Then multiply by ln(2) to convert to the result to base e.
    return VecFloat( RWMATH_VPU_DETAIL_VecMul(RWMATH_Log(a.mV), detail::gLn2) );
#else
    float val = a.GetFloat();
    val = logf(val);
    return VecFloat(val);
#endif
}

RW_MATH_FORCE_INLINE VecFloat Exp(VecFloat::InParam a) 
{
#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU))
    return VecFloat( expf4(a.mV) );
#elif defined(EA_PLATFORM_XENON)
    return VecFloat( RWMATH_Exp(RWMATH_VPU_DETAIL_VecMul(a.mV, detail::gLn2Reciprocal)) );
#else
    float val = a.GetFloat();
    val = expf(val);
    return VecFloat(val);
#endif
}

RW_MATH_FORCE_INLINE VecFloat SgnNonZero(VecFloat::InParam a) 
{
    VectorIntrinsic greaterEqualZeroMask, result;
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    VectorIntrinsic negOne = RW_MATH_VPU_GetNegOne_R();
    RW_MATH_VPU_IsGreaterEqual( greaterEqualZeroMask, a.mV, RW_MATH_VPU_GetZero_R());
    RW_MATH_VPU_Select( result, negOne, one, greaterEqualZeroMask );
    return VecFloat(result); 
}

RW_MATH_FORCE_INLINE bool IsValid(VecFloat::InParam value) 
{
    return !IsNaN(value);
}

RW_MATH_FORCE_INLINE VecFloat MultAdd(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam c)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAdd( result, a.mV, b.mV, c.mV );
    return VecFloat(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompLessThan(VecFloat::InParam a, VecFloat::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, b.mV, a.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompGreaterThan(VecFloat::InParam a, VecFloat::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompLessEqual(VecFloat::InParam a, VecFloat::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, b.mV, a.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompGreaterEqual(VecFloat::InParam a, VecFloat::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompEqual(VecFloat::InParam a, VecFloat::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE MaskScalar CompNotEqual(VecFloat::InParam a, VecFloat::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return MaskScalar(result);
}

RW_MATH_FORCE_INLINE VecFloat   Select(MaskScalar::InParam mask, VecFloat::InParam trueValue, VecFloat::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return VecFloat(result);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
