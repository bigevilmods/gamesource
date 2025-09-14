// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_SCALAR_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_CROSS_SCALAR_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vec_float.h"

namespace rw
{

namespace math
{

namespace vpu
{

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE float Min(const float a, const float b) 
{
    return (a>b) ? b : a;
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE float Max(const float a, const float b) 
{
    return (a>b) ? a : b;
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE double Min(const double a, const double b) 
{
    return (a>b) ? b : a;
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE double Max(const double a, const double b) 
{
    return (a>b) ? a : b;
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE float Abs(const float a) 
{
    ::rw::math::detail::Flt32UInt32Union ua; // use union to force compile use memory aliasing analysis
    ua.f = a;
    ua.i = ua.i & 0x7FFFFFFF; // clear the sign bit
    return ua.f;
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE double Abs(const double a) 
{
    return fabs(a);
}

RW_MATH_FORCE_INLINE bool IsNaN(VecFloat::InParam value) 
{
    bool equal;
    RW_MATH_VPU_IsEqualV4(equal, value.mV, value.mV);
    return !equal;
}

RW_MATH_FORCE_INLINE int32_t FastFloatToInt32(float a) 
{
    return (int32_t)a;
}

RW_MATH_FORCE_INLINE VecFloat Sgn(VecFloat::InParam a) 
{   
    VecFloat zero(GetVecFloat_Zero());
    if( a == zero )
    {
        return zero;
    }
    else if( a >= zero )
    {
        return GetVecFloat_One();
    }
    else
    {
        return GetVecFloat_NegativeOne();
    }
}

RW_MATH_FORCE_INLINE bool IsZero(VecFloat::InParam value, VecFloat::InParam tolerance) 
{
    return (value <=  tolerance) && (value >= Negate(tolerance));
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsZero(const VecFloatRef<INDEX> value, VecFloat::InParam tolerance) 
{
    VecFloat value_vf(value);
    return (value_vf <=  tolerance) && (value_vf >= -tolerance);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloat::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, VecFloatRef<INDEX> b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloat::InParam a, const float& b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

// VecFloatRef comparison

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX> a, VecFloat::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRef<INDEX> a, const float& b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

// VecFloatRefIndex comparison

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, const float& b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

// float comparison
RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, VecFloat::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, VecFloatRef<INDEX> b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, VecFloatRefIndex::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_FORCE_INLINE bool IsSimilar(const float& a, const float& b, VecFloat::InParam epsilon) 
{
    return IsZero(VecFloat(a - b), epsilon);
}

}

}

}

#endif
