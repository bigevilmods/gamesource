// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_MINI_H
#define RW_MATH_MINI_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"


namespace rw
{
namespace math
{
namespace mini
{


namespace detail
{

template <unsigned int size> struct uintN_t { };

template <> struct uintN_t<1>   { typedef uint8_t T; };
template <> struct uintN_t<2>   { typedef uint16_t T; };
template <> struct uintN_t<3>   { typedef uint32_t T; };
template <> struct uintN_t<4>   { typedef uint32_t T; };

}


//template <unsigned int _mantissa_bits, unsigned int _exponent_bits, unsigned int _exponent_bias = (1 << (_exponent_bits-1)) - 1> // freebox compiler for some reason
//thinks there is a operation order ambiguity with this line ( with the first "-" ) and will give a warning suggesting additional paranthesis, using equivalent ( for
// realistic number of exponent bits ) line instead which it is happy with
template <unsigned int _mantissa_bits, unsigned int _exponent_bits, unsigned int _exponent_bias = ( (1 << _exponent_bits) >> 1 ) - 1> 
class MiniFloat
{
public:

    enum { bits = 1 + _exponent_bits + _mantissa_bits };


    typedef typename detail::uintN_t<(bits + 7) / 8>::T mini_t;


    // \brief Convert a float to a mini float
    //
    // This function converts a float to its corresponding mini float. The float
    // value must be in the valid range that can be represented by a mini float.
    // Additionally, special values like +/-INFINITY, +/-NaN and the denormalized
    // are not supported.  The result is undefined for these special and out-of-range
    // float values.  +/-Zeros are supported.
    //
    // Note: define RW_MATH_ENABLE_ASSERTS to enable assertion for invalid floats and
    // unsupported special float values.
    //
    static mini_t MiniFromFloat(float fVal);


    // \brief Convert a mini float to a float
    //
    // This function converts a mini float to its corresponding float. Special
    // values like +/-INFINITY, +/-NaN and the denormalized are not supported.  The
    // result is undefined for these special mini floats.  +/-Zeros are supported.
    //
    // Note: define RW_MATH_ENABLE_ASSERTS to enable assertion for unsupported
    // special mini float values.
    //
    static float FloatFromMini(mini_t val);


    // \brief Checks if a mini float is +/-NaN
    //
    static bool IsNaN(mini_t value);


    // \brief Checks if a mini float is +/-INFINITY
    //
    static bool IsInfinite(mini_t value);


    // \brief Checks if a mini float is +/-ZERO
    //
    static bool IsZero(mini_t value);

private:
    enum { sign_bits = 1 };
    enum { exponent_bits = _exponent_bits };
    enum { mantissa_bits = _mantissa_bits };

    enum { sign_shift = exponent_bits + mantissa_bits };
    enum { exponent_shift = mantissa_bits };
    enum { mantissa_shift = 0 };

    enum { sign_mask = ((1 << sign_bits) - 1) << sign_shift };
    enum { exponent_mask = ((1 << exponent_bits) - 1) << exponent_shift };
    enum { mantissa_mask = ((1 << mantissa_bits) - 1) << mantissa_shift };

    enum { exponent_bias = _exponent_bias };
};


template <unsigned int _m, unsigned int _e, unsigned int _b>
RW_MATH_INLINE typename MiniFloat<_m,_e,_b>::mini_t MiniFloat<_m,_e,_b>::MiniFromFloat(float fVal)
{
    rw::math::detail::Flt32UInt32Union fu32Union;
    fu32Union.f = fVal;

    uint32_t sign = (fu32Union.i & 0x80000000) >> (31 - sign_shift);
    uint32_t mantissa = (fu32Union.i & 0x7FFFFF) >> (23 - mantissa_bits);
    int32_t exponent = (int32_t)((fu32Union.i & 0x7F800000) >> 23);

#ifdef RW_MATH_ENABLE_ASSERTS
    RW_MATH_ASSERT_MSG( exponent != 255, "The input float is either +/-Infinity or NaN!" );
    RW_MATH_ASSERT_MSG( exponent != 0 || (fu32Union.i & 0x7fffffff) == 0,  "The input float is denormalized!" );
    RW_MATH_ASSERT_MSG( (fu32Union.i & 0x7fffffff) == 0 || (exponent >= 128 - exponent_bias && exponent <= 125 + (1 << exponent_bits) - exponent_bias), "The input float is out of range for a mini float!" );
#endif

    // Rebias the exponent and handle unrepresentable values
    exponent = exponent - 127 + exponent_bias;
    if (exponent <= 0)
        return static_cast<mini_t>(sign);                   // +/- ZERO
    if (exponent >= (1 << exponent_bits)-1)
        return static_cast<mini_t>(sign | exponent_mask);   // +/- INF
    exponent <<= exponent_shift;

    uint32_t result = sign | exponent | mantissa;
    return static_cast<mini_t>(result);
}


template <unsigned int _m, unsigned int _e, unsigned int _b>
RW_MATH_INLINE float MiniFloat<_m,_e,_b>::FloatFromMini(mini_t val)
{
    uint32_t pVal = static_cast<uint32_t>(val);
    uint32_t sign = (pVal & sign_mask) << (31 - sign_shift);
    uint32_t mantissa = (pVal & mantissa_mask) << (23 - mantissa_bits);
    uint32_t exponent = (pVal & exponent_mask) >> exponent_shift;

#ifdef RW_MATH_ENABLE_ASSERTS
    RW_MATH_ASSERT_MSG( exponent != exponent_mask, "The input mini float is either +/-Infinity or NaN!" );
    RW_MATH_ASSERT_MSG( exponent != 0 || (pVal & mantissa_mask) == 0,  "The input mini float is denormalized!" );
#endif

    // Handle abnormal values
    if (exponent == 0)
    {
        rw::math::detail::Flt32UInt32Union fu32Union;
        fu32Union.i = sign;         // +/- ZERO
        return fu32Union.f;
    }
    if (exponent == exponent_mask)
    {
        rw::math::detail::Flt32UInt32Union fu32Union;
        fu32Union.i = ((pVal & mantissa_mask) == 0)
            ? (0x7F800000 | sign)   // +/- INF
            :  0x7FFFFFFF;          // NAN
        return fu32Union.f;
    }
    exponent = (exponent + 127 - exponent_bias) << 23;

    rw::math::detail::Flt32UInt32Union fu32Union;
    fu32Union.i = sign | exponent | mantissa;
    return fu32Union.f;
}


template <unsigned int _m, unsigned int _e, unsigned int _b>
RW_MATH_INLINE bool MiniFloat<_m,_e,_b>::IsNaN(mini_t value)
{
    // NAN if all exponent bits are set and mantissa is non-zero
    return ((value & exponent_mask) == exponent_mask) &&
           ((value & mantissa_mask) != 0);
}


template <unsigned int _m, unsigned int _e, unsigned int _b>
RW_MATH_INLINE bool MiniFloat<_m,_e,_b>::IsInfinite(mini_t value)
{
    // INF if all exponent bits are set and mantissa is zero
    return ((value & exponent_mask) == exponent_mask) &&
           ((value & mantissa_mask) == 0);
}


template <unsigned int _m, unsigned int _e, unsigned int _b>
RW_MATH_INLINE bool MiniFloat<_m,_e,_b>::IsZero(mini_t value)
{
    // ZERO if all exponent bits are zero and mantissa is zero
    return (value & (exponent_mask | mantissa_mask)) == 0;
}


typedef MiniFloat<10, 5>    FP16;   // s10e5 (IEEE 754-2008 binary16 AKA "half")
typedef MiniFloat<16, 7>    FP24;   // s16e7 (ATI 24-bit float)


} // namespace mini
} // namespace math
} // namespace rw


#endif
