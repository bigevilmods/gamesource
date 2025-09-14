// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SCALAR_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_SCALAR_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE float Min(const float a, const float b) 
{
    return (float)__fsel((a)-(b), b, a);
}

RW_MATH_FORCE_INLINE float Max(const float a, const float b) 
{
    return (float)__fsel((a)-(b), a, b);
}

RW_MATH_FORCE_INLINE double Min(const double a, const double b) 
{
    return __fsel((a)-(b), b, a);
}

RW_MATH_FORCE_INLINE double Max(const double a, const double b) 
{
    return __fsel((a)-(b), a, b);
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE float Abs(const float a) 
{
    return __fabs(a);
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE double Abs(const double a) 
{
    return __fabs(a);
}

RW_MATH_INLINE bool IsNaN(VecFloat::InParam value)
{
    bool equal;
    RW_MATH_VPU_IsEqualV4(equal, value.mV, value.mV);
    return !equal;
}

RW_MATH_INLINE int32_t FastFloatToInt32(float a)
{
    return (int32_t)a;
}

RW_MATH_INLINE VecFloat Sgn(VecFloat::InParam a)
{
    // zero will be used to compare against to determine the sign.
    VectorIntrinsic zero = __vspltisw(0);
    VectorIntrinsic one = RW_MATH_VPU_CreateVectorFromScalar_R(1.0f);
    VectorIntrinsic negOne = RW_MATH_VPU_CreateVectorFromScalar_R(-1.0f);

    // Determine if 'a' is greater than 0.
    VectorIntrinsic greaterThanZeroMask = __vcmpgtfp( a.mV, zero );
    // The next line will set result to 0 if 'a' is not greater than zero.  Otherwise it will set it to 1.
    VectorIntrinsic result0 = __vsel( zero, one, greaterThanZeroMask );
    // Determine if 'a' is less than 0.
    VectorIntrinsic greaterEqualThanZeroMask = __vcmpgefp( a.mV, zero );
    // The next line will leave result unchanged if 'a' is not less than zero.  Otherwise it will be set to -1.
    return VecFloat(__vsel( negOne, result0, greaterEqualThanZeroMask ));
}

RW_MATH_FORCE_INLINE bool IsZero(VecFloat::InParam v, VecFloat::InParam tolerance) 
{
    VectorIntrinsic result = __vcmpgtfp(Abs(v).mV, tolerance.mV);
    VectorIntrinsic ret = __vperm(result, result, detail::gGatherMostSignificantBytesV4);
    return ((int*)(&ret))[0] == 0;
}

// following code cause MSVC to produce some weird compiler error. As far as we know, GCC is okay.
//#ifdef __GNUC__

template<VectorAxis INDEX>
RW_MATH_INLINE bool IsZero(VecFloatRef<INDEX> value, VecFloat::InParam tolerance)
{
    VecFloat v(value);
    VectorIntrinsic result = __vcmpgtfp(Abs(v).mV, tolerance.mV);
    VectorIntrinsic ret = __vperm(result, result, detail::gGatherMostSignificantBytesV4);
    return ((int*)(&ret))[0] == 0;
}

//#endif 

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
    return IsZero(VecFloat(a-b), epsilon);
}

}

}

}

#endif
