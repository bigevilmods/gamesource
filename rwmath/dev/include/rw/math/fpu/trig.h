// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_TRIG_H
#define RW_MATH_FPU_TRIG_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/vec_float.h"

#if defined(EA_PLATFORM_PS3) && (CELL_SDK_VERSION >= 0x080000)
    #include "rw/math/fpu/detail/ps3/ppu/ppu_math_intrinsics.h"
#endif

namespace rw
{
namespace math
{
namespace fpu
{

template <class T> RW_MATH_FORCE_INLINE T      DegToRad(T angle_degrees)
{
    return angle_degrees * DEGREES_TO_RADIANS;
}

template <class T> RW_MATH_FORCE_INLINE T      RadToDeg(T angle_radians)
{
    return angle_radians * RADIANS_TO_DEGREES;
}

RW_MATH_FORCE_INLINE float          Sin(float a) { return sinf(a); }
RW_MATH_FORCE_INLINE double         Sin(double a) { return sin(a); }

RW_MATH_FORCE_INLINE float          Cos(float a) { return cosf(a); }
RW_MATH_FORCE_INLINE double         Cos(double a) { return cos(a); }

RW_MATH_FORCE_INLINE float          Tan(float a) { return tanf(a); }
RW_MATH_FORCE_INLINE double         Tan(double a) { return tan(a); }

RW_MATH_FORCE_INLINE float          ASin(float a) { return asinf(a); }
RW_MATH_FORCE_INLINE double         ASin(double a) { return asin(a); }

RW_MATH_FORCE_INLINE float          ACos(float a) { return acosf(a); }
RW_MATH_FORCE_INLINE double         ACos(double a) { return acos(a); }

RW_MATH_FORCE_INLINE float          ATan(float a) { return atanf(a); }
RW_MATH_FORCE_INLINE double         ATan(double a) { return atan(a); }

RW_MATH_FORCE_INLINE float          ATan2(float a, float b)
{
#ifdef EA_PLATFORM_PS3_SPU
    // This 'if' is to workaround a bug in Sony's implementation of atan2f.
    if( b == -0.0f )
    {
        // Note: Using 0 will cause GCC to optimize away this 'if' statement
        // and not modify the value at all in release builds.
        b = MIN_FLOAT;
    }
#endif
    return atan2f(a, b);
}

RW_MATH_FORCE_INLINE double         ATan2(double a, double b)
{
#ifdef EA_PLATFORM_PS3_SPU
    // This 'if' is to workaround a bug in Sony's implementation of atan2.
    if( b == -0.0f )
    {
        // Note: Using 0 will cause GCC to optimize away this 'if' statement
        // and not modify the value at all in release builds.
        b = MIN_FLOAT;
    }
#endif
    return atan2(a, b);
}

#if defined(EA_PLATFORM_PS3) && (CELL_SDK_VERSION >= 0x080000)
template <class T>
RW_MATH_FORCE_INLINE T NormalizeAngle(T a)
{
    return (T)NormalizeAngle((double)a);
}

template <>
RW_MATH_FORCE_INLINE double NormalizeAngle(double a)
{
    using namespace ::rw::math::fpu::detail;
    double scaledAngle = a * (1.0f/TWO_PI);
    double scaledAnglePlusHalf  = scaledAngle + 0.5f;
    double scaledAngleMinusHalf = scaledAngle - 0.5f;
    double scaledAngleMinusOne  = scaledAngle - 1.0f;
    double truncatedAngle = fcfid(fctiwz(scaledAngle));
    double fractionalAngle = scaledAngle - truncatedAngle;
    double wrapGreater = __fsel( scaledAngleMinusHalf - truncatedAngle, scaledAngleMinusOne - truncatedAngle, fractionalAngle );
    double addLess = __fsel( truncatedAngle-scaledAnglePlusHalf, TWO_PI, 0.0f );
    return FMADD( wrapGreater, TWO_PI, addLess );
}
#elif defined EA_PLATFORM_XENON
template <class T>
RW_MATH_FORCE_INLINE T NormalizeAngle(T a)
{
    return (T)NormalizeAngle((double)a);
}

template <>
RW_MATH_FORCE_INLINE double NormalizeAngle(double a)
{
    double scaledAngle = a * (1.0f/TWO_PI);
    double scaledAnglePlusHalf  = scaledAngle + 0.5f;
    double scaledAngleMinusHalf = scaledAngle - 0.5f;
    double scaledAngleMinusOne  = scaledAngle - 1.0f;
    double truncatedAngle = __fcfid(__fctiwz(scaledAngle));
    double fractionalAngle = scaledAngle - truncatedAngle;
    double wrapGreater = __fsel( scaledAngleMinusHalf - truncatedAngle, scaledAngleMinusOne - truncatedAngle, fractionalAngle );
    double addLess = __fsel( truncatedAngle-scaledAnglePlusHalf, TWO_PI, 0.0f );
    return wrapGreater * TWO_PI + addLess; // This expression compiles to an fmadd on VisualStudio if the correct compiler options are present.
}
#else
template <class T> RW_MATH_FORCE_INLINE T NormalizeAngle(T a)
{
    T scaledAngle = a * (T)(1.0f/TWO_PI);
    scaledAngle = scaledAngle - floorf(static_cast<float>(scaledAngle));
    if( scaledAngle > (T)0.5f )
    {
        scaledAngle -= (T)1.0f;
    }
    return scaledAngle * TWO_PI;
}
#endif

template <class T> RW_MATH_FORCE_INLINE void                           SinCos(T a, T& sin_out, T& cos_out)
{
    sin_out = Sin(a);
    cos_out = Cos(a);
}
template <class T> RW_MATH_FORCE_INLINE Vector4Template<T>     SinCos(T a)
{
    return Vector4Template<T>( Sin(a), Cos(a), 0.0f, 0.0f );
}
template <class T> RW_MATH_FORCE_INLINE Vector4Template<T> SinCosFast(T a)
{
    return SinCos<T>(a);
}
template <class T> RW_MATH_FORCE_INLINE Vector4Template<T> SinCosFast2(T a, T b)
{
    return Vector4Template<T>( Sin(a), Cos(a), Sin(b), Cos(b) );
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif
