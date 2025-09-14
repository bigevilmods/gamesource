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

#define SSE_SHUFFLE( x_index, y_index, z_index, w_index ) \
    ( ((w_index)<<6) | ((z_index)<<4) | ((y_index)<<2) | (x_index) )
#define SSE_COMP_MASK( x_flag, y_flag, z_flag, w_flag ) \
    ( ((w_flag)<<3) | ((z_flag)<<2) | ((y_flag)<<1) | (x_flag) )

RW_MATH_FORCE_INLINE float Min(const float a, const float b) 
{
    return (a>b) ? b : a;
}

RW_MATH_FORCE_INLINE float Max(const float a, const float b) 
{
    return (a>b) ? a : b;
}

RW_MATH_FORCE_INLINE double Min(const double a, const double b) 
{
    return (a>b) ? b : a;
}

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
    return _mm_cvtss_si32( _mm_load_ss( &a ) );
}

RW_MATH_FORCE_INLINE VecFloat Sgn(VecFloat::InParam a) 
{
    // zero will be used to compare against to determine the sign.
    VectorIntrinsic zero = _mm_setzero_ps();
    VectorIntrinsic one = RW_MATH_VPU_CreateVectorFromScalar_R(1.0f);
    VectorIntrinsic negOne = RW_MATH_VPU_CreateVectorFromScalar_R(-1.0f);

    // Determine if 'a' is greater than 0.
    VectorIntrinsic greaterThanZeroMask = _mm_cmpgt_ps( a.mV, zero );
    // The next line will set result to 0 if 'a' is not greater than zero.  Otherwise it will set it to 1.
    VectorIntrinsic result = _mm_and_ps( greaterThanZeroMask, one );
    // Determine if 'a' is less than 0.
    VectorIntrinsic lessThanZeroMask = _mm_cmplt_ps( a.mV, zero );
    // The next line will leave result unchanged if 'a' is not less than zero.  Otherwise it will be set to -1.
    result = _mm_or_ps( result, _mm_and_ps( lessThanZeroMask, negOne ) );

    return VecFloat(result);
}

RW_MATH_FORCE_INLINE bool IsZero(VecFloat::InParam v, VecFloat::InParam tolerance) 
{
    VecFloat absValue = Abs(v);
    VectorIntrinsic comparisonMask = _mm_cmple_ps( absValue.mV, tolerance.mV );
    int msbMask = _mm_movemask_ps( comparisonMask );
    bool result = (msbMask == SSE_COMP_MASK(1,1,1,1));
    return result;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE bool IsZero(VecFloatRef<INDEX> value, VecFloat::InParam tolerance) 
{
    VectorIntrinsic absValue;
    RW_MATH_VPU_Abs( absValue, value.mV );
    
    VectorIntrinsic comparisonMask = _mm_cmple_ps( absValue, tolerance.mV );
    int msbMask = _mm_movemask_ps( comparisonMask );
    bool result = ((msbMask & (1<<INDEX)) != 0);
    return result;
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
    return IsZero(Subtract(a, b), epsilon);
}
#undef SSE_SHUFFLE
#undef SSE_COMP_MASK

}

}

}

#endif
