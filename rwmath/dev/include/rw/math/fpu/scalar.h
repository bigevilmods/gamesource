// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_SCALAR_H
#define RW_MATH_FPU_SCALAR_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/scalar_platform.h"
#include "rw/math/fpu/vec_float.h"
#include "rw/math/fpu/maskscalar_operation.h"
#include "rw/math/fpu/detail/float_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{
#ifdef EA_PLATFORM_REVOLUTION
    float Sqrt(float a);
	float SqrtFast(float a);
#endif
}

template <class T> RW_MATH_FORCE_INLINE int32_t FastFloatToInt32(T a)
{
    return (int32_t)a;
}

template <class T> RW_MATH_FORCE_INLINE T Floor(T a)
{
    return (T)Floor(static_cast<double>(a));
}

template <> RW_MATH_FORCE_INLINE float      Floor<float>(float a)
{
#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU) || defined(EA_PLATFORM_XENON) || defined(EA_PLATFORM_REVOLUTION)
    // Note: This implementation returns +0.0 when the input is -0.0.
    // The implementation works on the data as a double (for the most part).  This avoid unnecessary rounding back to single precision values.
    const double maxMantissa = 4503599627370496.0; // constant is pow(2, 52), 52 is the number of bits in the mantissa of a double precision floating point number.
    const double doubleInput = a;
    double c = RW_MATH_FPU_Fsel(doubleInput, -maxMantissa, maxMantissa);
    double result = (doubleInput-c) + c;
    result -= RW_MATH_FPU_Fsel( doubleInput-result, 0.0, 1.0);
    return static_cast<float>(result);
#else
    return floorf(a);
#endif
}

template <> RW_MATH_FORCE_INLINE double    Floor<double>(double a)
{
#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU) || defined(EA_PLATFORM_XENON) || defined(EA_PLATFORM_REVOLUTION)
    // Note: This implementation returns +0.0 when the input is -0.0.
    const double maxMantissa = 4503599627370496.0; // constant is pow(2, 52), 52 is the number of bits in the mantissa of a double precision floating point number.
    double c = RW_MATH_FPU_Fsel(a, -maxMantissa, maxMantissa);
    double result = (a-c) + c;
    result -= RW_MATH_FPU_Fsel( a-result, 0.0, 1.0);
    return result;
#else
    return floor(a);
#endif
}

template <class T> RW_MATH_FORCE_INLINE int32_t        IntFloor(T a)
{
    return (int) Floor(a);
}

RW_MATH_FORCE_INLINE float       Ceil(float a)          { return ceilf(a); }
RW_MATH_FORCE_INLINE double     Ceil(double a)          { return ceil(a); }

template <class T> RW_MATH_FORCE_INLINE int32_t         IntCeil(T a)
{
    return (int) Ceil(a);
}

template <class T> RW_MATH_FORCE_INLINE T         Round(T a)
{
    return Floor(a + 0.5f); 
}
template <class T> RW_MATH_FORCE_INLINE int32_t        IntRound(T a)
{
    return IntFloor(a + 0.5f);
}

template <class T> RW_MATH_FORCE_INLINE T        Modulo(T a, T modulus)
{
    RW_MATH_ASSERT_MSG( !IsZero(modulus), ("Modulus is zero.") );
    T modulo = a - ( (T)( (int)( ( a / modulus ) ) ) * modulus );
    return modulo;
}

template <class T> RW_MATH_FORCE_INLINE T           Abs(const T& a)       { return a >= static_cast<T>(0.0f) ? a : -a; }
RW_MATH_FORCE_INLINE float               Abs(const float a)       
{ 
    #if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
        return (a < 0.0f) ? -a : a;
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_PS3
        return (float)RW_MATH_FPU_Fabs(a);
    #else
        return (a < 0.0f) ? -a : a;
    #endif
}

RW_MATH_FORCE_INLINE double           Abs(const double a)       
{ 
    #if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_UNIX || defined EA_PROCESSOR_ARM
        return (a < 0.0) ? -a : a;
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_PS3
        return RW_MATH_FPU_Fabs(a);
    #elif defined EA_PLATFORM_PS3_SPU
        return (a < 0.0) ? -a : a; // we didn't use fabs, since on SPU fabs returns -nan for small numbers like -0.000000806
    #else
        return fabs(a);
    #endif
}

template <class T> RW_MATH_FORCE_INLINE T    Min(const T& a, const T& b)  
{
    return (a < b? a : b);
}

RW_MATH_FORCE_INLINE float        Min(const float a, const float b)  
{ 
    #if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
        return (a>b) ? b : a;
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_PS3
        return (float)RW_MATH_FPU_Fsel((a)-(b), b, a);
    #else
        return (a>b) ? b : a;
    #endif
}

RW_MATH_FORCE_INLINE double        Min(const double a, const double b)  
{ 
    #if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
        return (a>b) ? b : a;
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_PS3
        return RW_MATH_FPU_Fsel((a)-(b), b, a);
    #else
        return (a>b) ? b : a;
    #endif
}

template <class T> RW_MATH_FORCE_INLINE T    Min(const T& a, const T& b, const T& c)
{
    return Min(Min(a, b), c);
}

template <class T> RW_MATH_FORCE_INLINE T    Max(const T& a, const T& b)
{
    return (a > b? a : b);
}

RW_MATH_FORCE_INLINE float        Max(const float a, const float b)  
{
    #if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
        return (a>b) ? a : b;
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_PS3
        return (float)RW_MATH_FPU_Fsel((a)-(b), a, b);
    #else
        return (a>b) ? a : b;
    #endif
}

RW_MATH_FORCE_INLINE double        Max(const double a, const double b)  
{
    #if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
        return (a>b) ? a : b;
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_REVOLUTION || defined EA_PLATFORM_PS3
        return RW_MATH_FPU_Fsel((a)-(b), a, b);
    #else
        return (a>b) ? a : b;
    #endif
}

template <class T> RW_MATH_FORCE_INLINE T    Max(const T& a, const T& b, const T& c)  { return Max(Max(a, b), c); }

template <class T> RW_MATH_FORCE_INLINE T          Lerp(T a, T b, T t)
{
    RW_MATH_ASSERT_MSG(0.0f <= t && t <= 1.0f, ("t is out of bounds") );
    return a + (b-a) * t;
}

template <class T> RW_MATH_FORCE_INLINE T    Reciprocal(T a)
{
    return 1.0f / a;
}

template <class T> RW_MATH_FORCE_INLINE T ReciprocalFast(T a)
{
    return Reciprocal(a);
}

template <> RW_MATH_FORCE_INLINE float ReciprocalFast<float>(float a)
{
#ifdef EA_PLATFORM_REVOLUTION
	return Reciprocal(a); // ps instructions for ReciprocalFast is actaully a bit slower than the division on Wii.
#else
	float ret;
	RW_MATH_FPU_FRecipFast(ret, a);
	return ret;
#endif
}

template <> RW_MATH_FORCE_INLINE double ReciprocalFast<double>(double a)
{
#ifdef EA_PLATFORM_REVOLUTION
	return Reciprocal(a);
#else
    double ret;
	RW_MATH_FPU_DRecipFast(ret, a);
	return ret;
#endif
}

RW_MATH_FORCE_INLINE float ReciprocalEstimate(float a)
{
    return RW_MATH_FPU_FRecipEst(a);
}

RW_MATH_FORCE_INLINE double ReciprocalEstimate(double a)
{
    return RW_MATH_FPU_DRecipEst(a);
}

template <class T> RW_MATH_FORCE_INLINE T          Sqrt(T a)
{
    return Sqrt((float)a);
}

template <class T> RW_MATH_FORCE_INLINE T      SqrtEstimate(T a)
{
    return static_cast<T>(Sqrt(static_cast<float>(a)));
}

template <class T> RW_MATH_FORCE_INLINE T      SqrtFast(T a)
{
    return static_cast<T>(Sqrt(static_cast<float>(a)));
}

template <> RW_MATH_FORCE_INLINE float      SqrtFast<float>(float a)
{
#ifdef EA_PLATFORM_REVOLUTION
	return ::rw::math::fpu::detail::SqrtFast(a);
#elif defined EA_PLATFORM_PS3_SPU
	return sqrtf(a);
#else
    float ret;
	RW_MATH_FPU_FSqrtFast(ret, a);
	return ret;
#endif
}

template <> RW_MATH_FORCE_INLINE double      SqrtFast<double>(double a)
{
	double ret;
	RW_MATH_FPU_DSqrtFast(ret, a);
	return ret;
}

template <> RW_MATH_FORCE_INLINE float       Sqrt<float>(float a)
{
#if defined(EA_PLATFORM_PS3) && (CELL_SDK_VERSION >= 0x080000)
    return FSQRTS(a);
#elif defined EA_PLATFORM_REVOLUTION
    return detail::Sqrt(a);
#else
    return sqrtf(a);
#endif
}

template <> RW_MATH_FORCE_INLINE double     Sqrt<double>(double a)
{
#if defined(EA_PLATFORM_PS3) && (CELL_SDK_VERSION >= 0x080000)
    return FSQRT(a);
#else
    return sqrt(a);
#endif
}

RW_MATH_FORCE_INLINE float    InvSqrt(float a)
{
#ifdef EA_PLATFORM_REVOLUTION
    using namespace ::rw::math::ps::detail;
    mw_ps ps_result;
    RW_MATH_PS_ReciprocalSquareRoot(ps_result, a);
    return ps_extract0( ps_result );
#else
    return 1.0f/Sqrt(a);
#endif
}

RW_MATH_FORCE_INLINE double  InvSqrt(double a)
{
    return 1.0/Sqrt(a);
}

RW_MATH_FORCE_INLINE float    InvSqrtFast(float a)
{
#ifdef EA_PLATFORM_REVOLUTION
    using namespace ::rw::math::ps::detail;
    mw_ps ps_result;
    RW_MATH_PS_ReciprocalSquareRootFast(ps_result, a);
    return ps_extract0( ps_result );
#else
    float ret;
	RW_MATH_FPU_FInvSqrtFast(ret, a);
	return ret;
#endif
}

RW_MATH_FORCE_INLINE double  InvSqrtFast(double a)
{
    double ret;
	RW_MATH_FPU_DInvSqrtFast(ret, a);
	return ret;
}

template <class T> RW_MATH_FORCE_INLINE T       DivSqrt(T a, T dividend)
{
    RW_MATH_ASSERT_MSG( a > EPSILON, ("a must be greater than epsilon") );
    return dividend / Sqrt( a );
}

template <class T> RW_MATH_FORCE_INLINE T   DivSqrtFast(T a, T dividend)
{
    return DivSqrt(a, dividend);
}

RW_MATH_INLINE float   Pow(float base, float exponent)
{
#ifdef EA_PLATFORM_PS3
    // Sony's PPU implementation of pow and powf are buggy.  However the VMX
    // version seems to be implemented correctly, so the FPU code will actually
    // rely on the VMX function to calculate the correct value.  This change
    // should be reverted once Sony fixes their functions.
    union VectorFloatUnion
    {
        vector float vecValue;
        float fValue;
    };
    VectorFloatUnion baseU, exponentU, resultU;
    baseU.fValue = base;
    exponentU.fValue = exponent;
    resultU.vecValue = powf4(baseU.vecValue, exponentU.vecValue);
    return resultU.fValue;
#else
    return powf(base, exponent);
#endif
}

RW_MATH_INLINE double  Pow(double base, double exponent)
{
#ifdef EA_PLATFORM_PS3
    // Sony's PPU implementation of pow and powf are buggy.  However the VMX
    // version seems to be implemented correctly, so the FPU code will actually
    // rely on the VMX function to calculate the correct value.  This change
    // should be reverted once Sony fixes their functions.
    // (The double function simply calls the float version of Pow to avoid code duplication.)
    return Pow((float)base, (float)exponent);
#elif defined(EA_PLATFORM_PS3_SPU)
    // The SPU's double precision pow routine produces in correct results for
    // some inputs (e.g. pow(0.001f, 126.0f) ).  So, the floating point version
    // is used instead.  This change should be reverted once Sony fixes the
    // pow function on the SPU.
    return powf(base, exponent);
#else
    return pow(base, exponent);
#endif
}

template <class T> RW_MATH_FORCE_INLINE T         Clamp(T value, T min, T max)
{
    return Min(max, Max(min, value));
}

/*!
   \brief Clamps 'value' between -range and range.

   This function clamps the input \c value to the range [\c -range, \c range].

   \pre \c range must be a non-negative value or the result of this function is undefined.
*/
template <class T> RW_MATH_FORCE_INLINE T         ClampToNegPosRange(T value, T range)
{
    return Min(range, Max(-range, value));
}

template <class T> RW_MATH_FORCE_INLINE T           Log(T a)
{
    // cast param to float in case a is an int
    return static_cast<T>(Log(static_cast<float>(a)));
}
template <> RW_MATH_FORCE_INLINE float   Log<float>(float a)    { return logf(a); }
template <> RW_MATH_FORCE_INLINE double  Log<double>(double a)  { return log(a); }


template <class T> RW_MATH_FORCE_INLINE T Exp(T a)
{
    //cast param to float in case a is an int
    return static_cast<T>(Exp(static_cast<float>(a)));
}
template <> RW_MATH_FORCE_INLINE float   Exp<float>(float a)    { return expf(a); }
template <> RW_MATH_FORCE_INLINE double  Exp<double>(double a)  { return exp(a); }


template <class T> RW_MATH_INLINE T           Sgn(T a)
{
    if( a == 0.0f )
    {
        return 0.0f;
    }
    else if( a >= 0.0f )
    {
        return 1.0f;
    }
    else
    {
        return -1.0f;
    }
}

template <class T> RW_MATH_INLINE T           SgnNonZero(T a)
{
    if( a >= 0.0f )
    {
        return 1.0f;
    }
    else
    {
        return -1.0f;
    }
}

RW_MATH_INLINE bool            IsValid(float value)   { return !IsNaN(value); }
RW_MATH_INLINE bool            IsValid(double value)   { return !IsNaN(value); }

RW_MATH_INLINE bool             IsZero(float value, float tolerance = EPSILON) { return (value <=  tolerance) && (value >= -tolerance); }
RW_MATH_INLINE bool             IsZero(double value, double tolerance = EPSILON) { return (value <=  tolerance) && (value >= -tolerance); }

RW_MATH_INLINE bool          IsSimilar(float a, float b, float epsilon = SMALL_FLOAT)
{
    return Abs(a-b) < epsilon;
}

RW_MATH_INLINE bool          IsSimilar(double a, double b, double epsilon = SMALL_FLOAT)
{
    return Abs(a-b) < epsilon;
}

RW_MATH_FORCE_INLINE float MultAdd(float a, float b, float c)
{
    // In release builds the compiler should be able to convert this expression to a fused multiply add on
    // hardware that supports the instruction.
    return a * b + c;
}

RW_MATH_FORCE_INLINE double MultAdd(double a, double b, double c)
{
    // In release builds the compiler should be able to convert this expression to a fused multiply add on
    // hardware that supports the instruction.
    return a * b + c;
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompLessEqual(T a, T b)
{
    return MaskScalarTemplate<T>(b-a);
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompGreaterEqual(T a, T b)
{
    return MaskScalarTemplate<T>(a-b);
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompLessThan(T a, T b)
{
    return Not(CompGreaterEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompGreaterThan(T a, T b)
{
    return Not(CompLessEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompEqual(T a, T b)
{
    return And(CompLessEqual(a, b), CompGreaterEqual(a,b));
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompNotEqual(T a, T b)
{
    return Not(And(CompLessEqual(a, b), CompGreaterEqual(a,b)));
}

template <class T>
RW_MATH_FORCE_INLINE T Select(typename MaskScalarTemplate<T>::InParam mask, T trueValue, T falseValue )
{
    return (T)RW_MATH_FPU_Fsel( mask.mS, trueValue, falseValue );
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif
