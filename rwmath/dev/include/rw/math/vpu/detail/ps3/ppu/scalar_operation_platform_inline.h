// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_SCALAR_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_SCALAR_OPERATION_PLATFORM_INLINE_H

#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/fpu/detail/float_intrinsic_operation.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE float Min(const float a, const float b) 
{
    return (float)RW_MATH_FPU_Fsel((a)-(b), b, a);
}

RW_MATH_FORCE_INLINE float Max(const float a, const float b) 
{
    return (float)RW_MATH_FPU_Fsel((a)-(b), a, b);
}

RW_MATH_FORCE_INLINE double Min(const double a, const double b) 
{
    return RW_MATH_FPU_Fsel((a)-(b), b, a);
}

RW_MATH_FORCE_INLINE double Max(const double a, const double b) 
{
    return RW_MATH_FPU_Fsel((a)-(b), a, b);
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE float Abs(const float a) 
{
    return (float)RW_MATH_FPU_Fabs(a);
}

// this code is there for usability, so user can pass float and double into the vpu namespace and still get good performance
RW_MATH_FORCE_INLINE double Abs(const double a) 
{
    return RW_MATH_FPU_Fabs(a);
}


RW_MATH_INLINE bool IsNaN(VecFloat::InParam value)
{
    // Note:
    // Doing a float comparison here will not work.  The compiler may decide
    // that the a float value always equals itself even if it is a NaN.  Using
    // vector intrinsics to do the comparison solves the problem.  See the FPU
    // IsNan function for more details.
    return !_asmIsEqualV4( value.mV, value.mV );
}

RW_MATH_INLINE int32_t FastFloatToInt32(float a)
{
    return (int32_t)a;
}

RW_MATH_INLINE VecFloat Sgn(VecFloat::InParam a)
{
    // zero will be used to compare against to determine the sign.
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    VectorIntrinsic negOne = RW_MATH_VPU_Subtract_R(zero, one);

    // Determine if 'a' is greater than 0.
    VectorIntrinsic greaterThanZeroMask = RWMATH_VPU_DETAIL_VecCmpGT( a.mV, zero );
    // The next line will set result to 0 if 'a' is not greater than zero.  Otherwise it will set it to 1.
    VectorIntrinsic result0 = RW_MATH_VPU_Select_R( zero, one, greaterThanZeroMask );
    // Determine if 'a' is less than 0.
    VectorIntrinsic greaterEqualThanZeroMask = RW_MATH_VPU_IsGreaterEqual_R( a.mV, zero );
    // The next line will leave result unchanged if 'a' is not less than zero.  Otherwise it will be set to -1.
    return VecFloat(RW_MATH_VPU_Select_R( negOne, result0, greaterEqualThanZeroMask ));
}

RW_MATH_FORCE_INLINE bool IsZero(VecFloat::InParam v, VecFloat::InParam tolerance) 
{
    return (bool)RWMATH_VPU_DETAIL_VecAllGE( tolerance.mV, Abs(v).mV );
}

template<VectorAxis INDEX>
RW_MATH_INLINE bool IsZero(VecFloatRef<INDEX> value, VecFloat::InParam tolerance)
{
    return IsZero(VecFloat(value), tolerance);
}

RW_MATH_FORCE_INLINE bool IsSimilar(VecFloatRefIndex::InParam a, VecFloat::InParam b, VecFloat::InParam epsilon) 
{
    return IsZero(Subtract(a, b), epsilon);
}

RW_MATH_INLINE bool IsSimilar(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam epsilon)
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

}

}

}

#endif
