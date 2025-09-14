// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SOA4_DETAIL_SCALAR_OPERATION_INLINE_H
#define RW_MATH_SOA4_DETAIL_SCALAR_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include <math.h>
#include "rw/math/vpu/detail/soa4_soa4base.h"
#include "rw/math/vpu/soa4_constants_operation.h"
#include "rw/math/vpu/soa4_maskscalar_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

    
RW_MATH_FORCE_INLINE Soa4Float Floor(Soa4Float::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Floor(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Ceil(Soa4Float::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Ceil(result, a.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Round(Soa4Float::InParam a) 
{
    VectorIntrinsic result, half;
    RW_MATH_VPU_GetHalf(half);
    RW_MATH_VPU_Add(result, half, a.mV);
    RW_MATH_VPU_Floor(result, result);
    return Soa4Float(result);
}

// TODO : properly document the results returned by Modulo.  It looks like it behaves the same as fmod.
#if defined(EA_PLATFORM_PS3) || defined( EA_PLATFORM_PS3_SPU) || defined(EA_PLATFORM_XENON) || defined(EA_PLATFORM_WINDOWS)
RW_MATH_FORCE_INLINE Soa4Float Modulo(Soa4Float::InParam a, Soa4Float::InParam modulus) 
{
    RW_MATH_ASSERT_MSG( IsAllFalse(CompEqual(modulus, GetSoa4Float_Zero())), ("Modulus is zero.") );
    VectorIntrinsic divResult, truncDivResult, result;
    RW_MATH_VPU_Divide( divResult, a.mV, modulus.mV );
    RW_MATH_VPU_RoundToZero( truncDivResult, divResult );
    RW_MATH_VPU_MultSubNeg( result, truncDivResult, modulus.mV, a.mV );
    return Soa4Float(result);
}

#else
namespace detail
{
    RW_MATH_FORCE_INLINE float Modulo(float a, float modulus)
    {
        return a - ( (float)( (int)( ( a / modulus ) ) ) * modulus );
    }
}
RW_MATH_FORCE_INLINE Soa4Float Modulo(Soa4Float::InParam a, Soa4Float::InParam modulus) 
{
    RW_MATH_ASSERT_MSG( IsAllFalse(CompEqual(modulus, GetSoa4Float_Zero())), ("Modulus is zero.") );
    Soa4Float result;
    RW_MATH_DETAIL_VECFLOAT_APPLY_BINARY(result.mV, a.mV, modulus.mV, detail::Modulo);
    return Soa4Float(result);
}
#endif

RW_MATH_FORCE_INLINE Soa4Float Abs(Soa4Float::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Abs(ret, a.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float Min(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Min(result, a.mV, b.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Min(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam c)
{
    VectorIntrinsic r0, r1;
    RW_MATH_VPU_Min(r0, a.mV, b.mV);
    RW_MATH_VPU_Min(r1, r0, c.mV);
    return Soa4Float(r1);
}

RW_MATH_FORCE_INLINE Soa4Float Max(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_Max(result, a.mV, b.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Max(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam c)
{
    VectorIntrinsic r0, r1;
    RW_MATH_VPU_Max(r0, a.mV, b.mV);
    RW_MATH_VPU_Max(r1, r0, c.mV);
    return Soa4Float(r1);
}

RW_MATH_FORCE_INLINE Soa4Float Lerp(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam t) 
{
    RW_MATH_ASSERT_MSG(IsAllTrue( And(CompLessEqual(GetSoa4Float_Zero(), t), CompLessEqual(t, GetSoa4Float_One()))), ("t is out of bounds") );
    VectorIntrinsic result, delta;
    RW_MATH_VPU_Subtract(delta, b.mV, a.mV);
    RW_MATH_VPU_MultAdd(result, delta, t.mV, a.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Reciprocal(Soa4Float::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_Reciprocal( result, a.mV );
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float ReciprocalFast(Soa4Float::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalFast( result, a.mV );
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Sqrt(Soa4Float::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRoot(ret, a.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float SqrtFast(Soa4Float::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootFast(ret, a.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float SqrtEstimate(Soa4Float::InParam a) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootEstimate(ret, a.mV);
    return Soa4Float(ret);
}

RW_MATH_FORCE_INLINE Soa4Float InvSqrt(Soa4Float::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalSquareRoot( result, a.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float InvSqrtFast(Soa4Float::InParam a) 
{
    VectorIntrinsic result;
    RW_MATH_VPU_ReciprocalSquareRootFast( result, a.mV);
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4Float Pow(Soa4Float::InParam base, Soa4Float::InParam exponent) 
{
#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU)
    return Soa4Float( powf4(base.mV, exponent.mV) );
#elif defined(EA_PLATFORM_XENON)
    return Soa4Float( RWMATH_Pow(base.mV, exponent.mV) );
#else
    Soa4Float result;
    RW_MATH_DETAIL_VECFLOAT_APPLY_BINARY(result.mV, base.mV, exponent.mV, powf);
    return Soa4Float(result);
#endif
}


RW_MATH_FORCE_INLINE Soa4Float Clamp(Soa4Float::InParam value, Soa4Float::InParam min, Soa4Float::InParam max) 
{
    return Min(max, Max(min, value));
}

RW_MATH_FORCE_INLINE Soa4Float Log(Soa4Float::InParam a) 
{
#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU)
    return Soa4Float( logf4(a.mV) );
#elif defined(EA_PLATFORM_XENON)
    // Call Xenon's Log function which calculates a result with a base of 2.
    // Then multiply by ln(2) to convert to the result to base e.
    return Soa4Float( RWMATH_VPU_DETAIL_VecMul(RWMATH_Log(a.mV), ::rw::math::vpu::detail::gLn2) );
#else
    Soa4Float result;
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY(result.mV, a.mV, logf);
    return Soa4Float(result);
#endif
}

RW_MATH_FORCE_INLINE Soa4Float Exp(Soa4Float::InParam a) 
{
#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU)
    return Soa4Float( expf4(a.mV) );
#elif defined(EA_PLATFORM_XENON)
    return Soa4Float( RWMATH_Exp(RWMATH_VPU_DETAIL_VecMul(a.mV, ::rw::math::vpu::detail::gLn2Reciprocal)) );
#else
    Soa4Float result;
    RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY(result.mV, a.mV, expf);
    return Soa4Float(result);
#endif
}

RW_MATH_INLINE Soa4Float Sgn(Soa4Float::InParam a)
{
    // zero will be used to compare against to determine the sign.
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    VectorIntrinsic negOne = RW_MATH_VPU_Subtract_R(zero, one);

    // Determine if 'a' is greater than 0.
    VectorIntrinsic greaterThanZeroMask;
    RW_MATH_VPU_IsGreater( greaterThanZeroMask, a.mV, zero );
    // The next line will set result to 0 if 'a' is not greater than zero.  Otherwise it will set it to 1.
    VectorIntrinsic result0 = RW_MATH_VPU_Select_R( zero, one, greaterThanZeroMask );
    // Determine if 'a' is less than 0.
    VectorIntrinsic greaterEqualThanZeroMask = RW_MATH_VPU_IsGreaterEqual_R( a.mV, zero );
    // The next line will leave result unchanged if 'a' is not less than zero.  Otherwise it will be set to -1.
    return Soa4Float(RW_MATH_VPU_Select_R( negOne, result0, greaterEqualThanZeroMask ));
}

RW_MATH_FORCE_INLINE Soa4Float SgnNonZero(Soa4Float::InParam a) 
{
    VectorIntrinsic greaterEqualZeroMask, result;
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    VectorIntrinsic negOne = RW_MATH_VPU_GetNegOne_R();
    RW_MATH_VPU_IsGreaterEqual( greaterEqualZeroMask, a.mV, RW_MATH_VPU_GetZero_R());
    RW_MATH_VPU_Select( result, negOne, one, greaterEqualZeroMask );
    return Soa4Float(result); 
}

RW_MATH_FORCE_INLINE Soa4Float MultAdd(Soa4Float::InParam a, Soa4Float::InParam b, Soa4Float::InParam c)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAdd( result, a.mV, b.mV, c.mV );
    return Soa4Float(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar CompLessThan(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not(result, result);
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar CompGreaterThan(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar CompLessEqual(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    RW_MATH_VPU_Not(result, result);
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar CompGreaterEqual(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar CompEqual(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4MaskScalar CompNotEqual(Soa4Float::InParam a, Soa4Float::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Soa4MaskScalar(result);
}

RW_MATH_FORCE_INLINE Soa4Float   Select(Soa4MaskScalar::InParam mask, Soa4Float::InParam trueValue, Soa4Float::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Soa4Float(result);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
