// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_QUATERNION_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_QUATERNION_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/matrix33_operation.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/detail/quaternion_operation_platform_inline.h"

namespace rw
{

namespace math
{

namespace vpu
{

// basic functions

// functions return vector by value (default method of return vector)
RW_MATH_FORCE_INLINE Quaternion Pow(Quaternion::InParam v, VecFloat::InParam e)
{
    Quaternion r;

    VectorIntrinsic w, w_abs, identity_quat_guard, alpha, new_alpha;
    VectorIntrinsic new_w, scale, selection_mask;

    RW_MATH_VPU_Splat_I(w, v.mV, VectorAxisW);
    RW_MATH_VPU_CreateVectorV1(identity_quat_guard, 0.9999f);
    RW_MATH_VPU_Abs(w_abs, w);
    RW_MATH_VPU_IsGreater(selection_mask, identity_quat_guard, w_abs);

    // Extract the half angle alpha (alpha = theta/2)
    RW_MATH_VPU_ACos(alpha, w);
    // Compute new alpha value
    RW_MATH_VPU_Mult(new_alpha, alpha, e.mV);
    // Compute new w value
    RW_MATH_VPU_Cos(new_w, new_alpha);
    RW_MATH_VPU_Select(new_w, w, new_w, selection_mask);
    // Compute new xyz values
    RW_MATH_VPU_Sin(new_alpha, new_alpha);
    RW_MATH_VPU_Sin(alpha, alpha);
    //RW_MATH_VPU_Select(alpha, one, alpha, selection_mask);
    RW_MATH_VPU_Divide(scale, new_alpha, alpha);
    RW_MATH_VPU_Mult(r.mV, v.mV, scale);
    RW_MATH_VPU_Select(r.mV, v.mV, r.mV, selection_mask);

    RW_MATH_VPU_SwizzleStore_I_I( r.mV, r.mV, VectorAxisW, new_w, VectorAxisW );

    return r;
}

RW_MATH_FORCE_INLINE  Quaternion Negate(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Negate(ret, q.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Mult(Quaternion::InParam q, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, q.mV, s.mV);
    return Quaternion(ret);
}

template<VectorAxis INDEX> Quaternion Mult(Quaternion::InParam q, VecFloatRef<INDEX> s)
{
    VectorIntrinsic ret, scale;
    RW_MATH_VPU_Splat_I(scale, s.mV, INDEX);
    RW_MATH_VPU_Mult(ret, q.mV, scale);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Mult(Quaternion::InParam q, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic ret, scale;
    RW_MATH_VPU_Splat(scale, s.mV, s.mIndex);
    RW_MATH_VPU_Mult(ret, q.mV, scale);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Mult(Quaternion::InParam q, float s) 
{
    VectorIntrinsic ret, scale;
    RW_MATH_VPU_CreateVectorV1(scale, s);
    RW_MATH_VPU_Mult(ret, q.mV, scale);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE Quaternion MultByVector(Quaternion::InParam q, Vector4::InParam v)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, q.mV, v.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Divide(Quaternion::InParam q, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide(ret, q.mV, s.mV);
    return Quaternion(ret);
}

template<VectorAxis INDEX> Quaternion Divide(Quaternion::InParam q, VecFloatRef<INDEX> s)
{
    VectorIntrinsic ret, scale;
    RW_MATH_VPU_Splat_I(scale, s.mV, INDEX);
    RW_MATH_VPU_Divide(ret, q.mV, scale);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Divide(Quaternion::InParam q, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic ret, scale;
    RW_MATH_VPU_Splat(scale, s.mV, s.mIndex);
    RW_MATH_VPU_Divide(ret, q.mV, scale);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Divide(Quaternion::InParam q, float s) 
{
    VectorIntrinsic ret, scale;
    RW_MATH_VPU_CreateVectorV1(scale, s);
    RW_MATH_VPU_Divide(ret, q.mV, scale);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Add(Quaternion::InParam a, Quaternion::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Add(ret, a.mV, b.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Subtract(Quaternion::InParam a, Quaternion::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a.mV, b.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  VecFloat Dot(Quaternion::InParam a, Quaternion::InParam b) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV4(ret, a.mV, b.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE  VecFloat Magnitude(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_SquareRootDotV4(ret, q.mV, q.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE  VecFloat MagnitudeSquared(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV4(ret, q.mV, q.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE  VecFloat MagnitudeRecip(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalSquareRootDotV4(ret, q.mV, q.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Normalize(Quaternion::InParam q, VecFloat& magnitude) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeWithMagnitudeV4(ret, magnitude.mV, q.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  VecFloat NormalizeReturnMagnitude(Quaternion::InParam q, Quaternion& result) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeWithMagnitudeV4(result.mV, ret, q.mV);
    return VecFloat(ret);
}

RW_MATH_FORCE_INLINE VecFloat CheckedNormalizeReturnMagnitude(Quaternion::InParam v, Quaternion& result)
{
    VectorIntrinsic normalized;
    VectorIntrinsic magnitude;
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic minMagnitude = zero;
    RW_MATH_VPU_NormalizeWithMagnitudeV4(normalized, magnitude, v.mV);
    VectorIntrinsic magnitudeSufficientMask;
    RW_MATH_VPU_IsGreater(magnitudeSufficientMask, magnitude, minMagnitude);
    result.mV = RW_MATH_VPU_Select_R( zero, normalized, magnitudeSufficientMask );
    VectorIntrinsic magResult;
    RW_MATH_VPU_And( magResult, magnitude, magnitudeSufficientMask );
    return VecFloat(magResult);
}

#if defined(EA_PLATFORM_REVOLUTION)
namespace detail
{
    Quaternion NormalizeEstimate(Quaternion::InParam q);
    Quaternion NormalizeFast(Quaternion::InParam q);
    Quaternion Normalize(Quaternion::InParam q);
    Quaternion NLerpEstimate(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t);
    Quaternion NLerpFast(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t);
    Quaternion NLerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t);
}

RW_MATH_FORCE_INLINE Quaternion NLerpEstimate(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    return detail::NLerpEstimate(q0, q1, t);
}
RW_MATH_FORCE_INLINE Quaternion NLerpFast(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    return detail::NLerpFast(q0, q1, t);
}
RW_MATH_FORCE_INLINE Quaternion NLerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    return detail::NLerp(q0, q1, t);
}
RW_MATH_FORCE_INLINE  Quaternion NormalizeEstimate(Quaternion::InParam q) 
{
    return detail::NormalizeEstimate(q);
}
RW_MATH_FORCE_INLINE  Quaternion NormalizeFast(Quaternion::InParam q) 
{
    return detail::NormalizeFast(q);
}

RW_MATH_FORCE_INLINE  Quaternion Normalize(Quaternion::InParam q) 
{
    return detail::Normalize(q);
}
#else
RW_MATH_FORCE_INLINE  Quaternion NormalizeEstimate(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV4(ret, q.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion NormalizeFast(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV4(ret, q.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Normalize(Quaternion::InParam q) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV4(ret, q.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion NLerpEstimate(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    Quaternion l = Lerp(q0, q1, t);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeEstimateV4(ret, l.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion NLerpFast(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    Quaternion l = Lerp(q0, q1, t);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeFastV4(ret, l.mV);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion NLerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    Quaternion l = Lerp(q0, q1, t);
    VectorIntrinsic ret;
    RW_MATH_VPU_NormalizeV4(ret, l.mV);
    return Quaternion(ret);
}

#endif


RW_MATH_FORCE_INLINE  Quaternion Concat(Quaternion::InParam a, Quaternion::InParam b) 
{
    Quaternion q = Mult(a, b);
    RW_MATH_VPU_NormalizeV4(q.mV, q.mV);
    return q;
}

RW_MATH_FORCE_INLINE Quaternion QuaternionFromAxisRotationAngle(Vector3::InParam axis, VecFloat::InParam angle_radians)
{

    RW_MATH_ASSERT_MSG(Abs(MagnitudeSquared(axis) - GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.0001f)), ("axis must be of unit length") );

    VecFloat s, c;

    SinCos(angle_radians * VecFloat(RW_MATH_VPU_GetHalf_R()), s, c);

    Vector4 result(axis * s, c);
    return Quaternion( result.mV );
}

RW_MATH_FORCE_INLINE  void AxisAngleFromQuaternion(Quaternion::InParam q, Vector3& axis_out, VecFloat& angle_out) 
{
    VecFloat smallFloat(SMALL_FLOAT);  // TODO construct smallFloat more efficiently
    angle_out = ACos(q.W()) * VecFloat(RW_MATH_VPU_GetTwo_R());  

    VectorIntrinsic qW;
    VectorIntrinsic qWSquared;
    VectorIntrinsic sRecipSqrt;
    VectorIntrinsic tempResult;
    VectorIntrinsic iAxis;
    RW_MATH_VPU_GetI( iAxis );
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    RW_MATH_VPU_Splat_I( qW, q.mV, VectorAxisW );
    RW_MATH_VPU_Mult( qWSquared, qW, qW );
    VectorIntrinsic sinSquared = RW_MATH_VPU_Subtract_R(one, qWSquared );
    VectorIntrinsic isNotSmall = RW_MATH_VPU_IsGreaterEqual_R( sinSquared, smallFloat.mV );
    RW_MATH_VPU_ReciprocalSquareRoot( sRecipSqrt, sinSquared );
    RW_MATH_VPU_Mult( tempResult, q.mV, sRecipSqrt );
    RW_MATH_VPU_Select( axis_out.mV, iAxis, tempResult, isNotSmall );
}

RW_MATH_FORCE_INLINE  Vector3 EulerXYZFromQuaternion(Quaternion::InParam q) 
{
    Matrix33 m = Matrix33FromQuaternion(q);
    return EulerXYZFromMatrix33(m);
}

RW_MATH_FORCE_INLINE  Vector3 TransformVector(Vector3::InParam v, Quaternion::InParam q) 
{
    // Here is an alternate implementation for the function that should give the same result. 
    //
    // VecFloat xx = q2.X() * q2.X();
    // VecFloat yy = q2.Y() * q2.Y();
    // VecFloat zz = q2.Z() * q2.Z();
    // VecFloat xy = q2.X() * q2.Y();
    // VecFloat xz = q2.X() * q2.Z();
    // VecFloat yz = q2.Y() * q2.Z();
    // VecFloat wx = q2.W() * q2.X();
    // VecFloat wy = q2.W() * q2.Y();
    // VecFloat wz = q2.W() * q2.Z();
    // 
    // const VecFloat one(GetVecFloat_One());
    // const VecFloat two(GetVecFloat_Two());
    // Vector3 result3(
    //      v.X() *(one-two*(yy+zz)) + v.Y() * two*(xy-wz)     + v.Z() * two*(xz+wy),
    //      v.X() * two*(xy+wz)    + v.Y() * (one-two*(xx+zz)) + v.Z() * two*(yz-wx),
    //      v.X() * two*(xz-wy)    + v.Y() * two*(yz+wx)     + v.Z() * (one-two*(xx+yy))
    //  );

    // this is one gets used
    // v + 2.0f * CrossV3(q, CrossV3(q, v) + q.W() * v);
    VectorIntrinsic two, q_cross_v, ret, qW;
    RW_MATH_VPU_GetTwo(two);
    RW_MATH_VPU_Splat_I(qW, q.mV, VectorAxisW);
    RW_MATH_VPU_CrossV3(q_cross_v, q.mV, v.mV);
    RW_MATH_VPU_MultAdd(ret, qW, v.mV, q_cross_v);
    RW_MATH_VPU_CrossV3(ret, q.mV, ret);
    RW_MATH_VPU_MultAdd(ret, two, ret, v.mV);
    return Vector3(ret);
}

#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && (defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS3_SPU) || defined(EA_PLATFORM_XENON))
RW_MATH_FORCE_INLINE Quaternion QuaternionFromMatrix33(Matrix33::InParam m, VecFloat::InParam epsilon)
{
    using namespace rw::math::vpu::detail;
    RW_MATH_ASSERT_MSG(IsRotation(m, VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f))), ("The matrix provided must be a rotation matrix in order to convert it to a quaternion.") );
    VectorIntrinsic half = RW_MATH_VPU_GetHalf_R();
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic recipSqrt;

    VectorIntrinsic splatX, splatY, splatZ;
    RW_MATH_VPU_Splat_I( splatX, m.xAxis.mV, 0 );
    RW_MATH_VPU_Splat_I( splatY, m.yAxis.mV, 1 );
    RW_MATH_VPU_Splat_I( splatZ, m.zAxis.mV, 2 );

    VectorIntrinsic signedX = RW_MATH_VPU_Xor_R( splatX, gQuatFromMat_xSigns );
    VectorIntrinsic signedY = RW_MATH_VPU_Xor_R( splatY, gQuatFromMat_ySigns );
    VectorIntrinsic signedZ = RW_MATH_VPU_Xor_R( splatZ, gQuatFromMat_zSigns );
    VectorIntrinsic sumXY = RW_MATH_VPU_Add_R( signedX, signedY );
    VectorIntrinsic sumZOne = RW_MATH_VPU_Add_R( signedZ, one );
    VectorIntrinsic valuesToSqrt =  RW_MATH_VPU_Add_R( sumXY, sumZOne );

    VectorIntrinsic trace = RW_MATH_VPU_Add_R( sumXY, splatZ );
    RW_MATH_VPU_Splat_I( trace, trace, 0 );

    // RW_MATH_VPU_ReciprocalSquareRootEstimate could be used instead to increase performance.
    RW_MATH_VPU_ReciprocalSquareRoot( recipSqrt, valuesToSqrt );
    VectorIntrinsic s, s2;
    RW_MATH_VPU_Mult( s, valuesToSqrt, recipSqrt );
    RW_MATH_VPU_Mult( s2, half, recipSqrt );

    VectorIntrinsic quat_from_mat_temp1, quat_from_mat_temp2;
    RW_MATH_VPU_CreateVectorAxisA4_I( quat_from_mat_temp1, m.yAxis.mV, 2, m.zAxis.mV, 0, m.xAxis.mV, 1, half, 0 );
    RW_MATH_VPU_CreateVectorAxisA4_I( quat_from_mat_temp2, m.zAxis.mV, 1, m.xAxis.mV, 2, m.yAxis.mV, 0, zero, 0 );
    VectorIntrinsic N = RW_MATH_VPU_Subtract_R( quat_from_mat_temp1, quat_from_mat_temp2 );
    VectorIntrinsic S = RW_MATH_VPU_Add_R( quat_from_mat_temp1, quat_from_mat_temp2 );

    // trace is greater than epsilon
    VectorIntrinsic permuteScales0 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( X1, X1, X1, X0 );
    VectorIntrinsic scale0 = RWMATH_VPU_DETAIL_VecPerm( s, s2, permuteScales0 );
    VectorIntrinsic unscaledQ0 = N;
    VectorIntrinsic scaledQ0;
    RW_MATH_VPU_Mult( scaledQ0, unscaledQ0, scale0 );

    // m[0][0] is greatest 
    VectorIntrinsic permuteScales1 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( Y0, Y1, Y1, Y1 );
    VectorIntrinsic scale1 = RWMATH_VPU_DETAIL_VecPerm( s, s2, permuteScales1 );
    VectorIntrinsic permute1 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( W0, Z1, Y1, X0 );
    VectorIntrinsic unscaledQ1 = RWMATH_VPU_DETAIL_VecPerm(N, S, permute1);
    VectorIntrinsic scaledQ1;
    RW_MATH_VPU_Mult( scaledQ1, unscaledQ1, scale1 );

    // m[1][1] is greatest 
    VectorIntrinsic permuteScales2 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( Z1, Z0, Z1, Z1 );
    VectorIntrinsic scale2 = RWMATH_VPU_DETAIL_VecPerm( s, s2, permuteScales2 );
    VectorIntrinsic permute2 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( Z1, W0, X1, Y0 );
    VectorIntrinsic unscaledQ2 = RWMATH_VPU_DETAIL_VecPerm(N, S, permute2);
    VectorIntrinsic scaledQ2;
    RW_MATH_VPU_Mult( scaledQ2, unscaledQ2, scale2 );

    // m[2][2] is greatest 
    VectorIntrinsic permuteScales3 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( W1, W1, W0, W1 );
    VectorIntrinsic scale3 = RWMATH_VPU_DETAIL_VecPerm( s, s2, permuteScales3 );
    VectorIntrinsic permute3 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI( Y1, X1, W0, Z0 );
    VectorIntrinsic unscaledQ3 = RWMATH_VPU_DETAIL_VecPerm(N, S, permute3);
    VectorIntrinsic scaledQ3;
    RW_MATH_VPU_Mult( scaledQ3, unscaledQ3, scale3 );

    //  This C code represents the behaviour of the selection code below:
    //
    //  if (trace > epsilon) // Condition3
    //  {
    //      return Quaternion(scaledQ0);
    //  }
    //  else
    //  {
    //      if ((m.xAxis.X() > m.yAxis.Y()) && (m.xAxis.X() > m.zAxis.Z()))  // Condition2
    //      {    
    //          return Quaternion(scaledQ1);
    //      } 
    //      else if (m.yAxis.Y() > m.zAxis.Z())  // Condition1
    //      {   
    //          return Quaternion(scaledQ2);
    //      } 
    //      else 
    //      {   
    //          return Quaternion(scaledQ3);
    //      } 
    //  }

    // Use selects to implement conditions.
    VectorIntrinsic condition1Mask = RWMATH_VPU_DETAIL_VecCmpGT( splatY, splatZ );
    VectorIntrinsic condition2Mask = RW_MATH_VPU_And_R(
                                        RWMATH_VPU_DETAIL_VecCmpGT( splatX, splatY ),
                                        RWMATH_VPU_DETAIL_VecCmpGT( splatX, splatZ )
                                        );
    VectorIntrinsic condition3Mask = RWMATH_VPU_DETAIL_VecCmpGT( trace, epsilon.mV );
    VectorIntrinsic result = RW_MATH_VPU_Select_R( scaledQ3, scaledQ2, condition1Mask );
    result = RW_MATH_VPU_Select_R( result, scaledQ1, condition2Mask );
    result = RW_MATH_VPU_Select_R( result, scaledQ0, condition3Mask );
    return Quaternion(result);

}
#elif defined(EA_PLATFORM_REVOLUTION)
namespace detail
{
    Quaternion QuaternionFromMatrix33(Matrix33::InParam m, VecFloat::InParam epsilon);
}
RW_MATH_FORCE_INLINE Quaternion QuaternionFromMatrix33(Matrix33::InParam m, VecFloat::InParam epsilon)
{
    return detail::QuaternionFromMatrix33(m, epsilon);
}
#else
RW_MATH_FORCE_INLINE Quaternion QuaternionFromMatrix33(Matrix33::InParam m, VecFloat::InParam epsilon)
{
    RW_MATH_ASSERT_MSG(IsRotation(m, VecFloat(0.01f)), ("The matrix provided must be a rotation matrix in order to convert it to a quaternion.") );
    VecFloat tr = m.xAxis.X() + m.yAxis.Y() + m.zAxis.Z();
    VecFloat half(GetVecFloat_Half());

    if (tr > epsilon)
    {
        VecFloat s = Sqrt(tr + GetVecFloat_One());
        VecFloat s2 = half / s;
        return Quaternion(
            (m.yAxis.Z() - m.zAxis.Y()) * s2,
            (m.zAxis.X() - m.xAxis.Z()) * s2,
            (m.xAxis.Y() - m.yAxis.X()) * s2,
            half * s
            );
    }
    else
    {
        if ((m.xAxis.X() > m.yAxis.Y()) && (m.xAxis.X() > m.zAxis.Z())) 
        {   // m[0][0] is greatest 
            VecFloat s = Sqrt((m.xAxis.X() - (m.yAxis.Y() + m.zAxis.Z())) + GetVecFloat_One());
            VecFloat s2 = half / s;
            return Quaternion(
                half * s,
                (m.yAxis.X() + m.xAxis.Y()) * s2,
                (m.xAxis.Z() + m.zAxis.X()) * s2,
                (m.yAxis.Z() - m.zAxis.Y()) * s2
                );
        } 
        else if (m.yAxis.Y() > m.zAxis.Z()) 
        {   // m[1][1] is greatest 
            VecFloat s = Sqrt((m.yAxis.Y() - (m.zAxis.Z() + m.xAxis.X())) + GetVecFloat_One());
            VecFloat s2 = half / s;
            return Quaternion(
                (m.yAxis.X() + m.xAxis.Y()) * s2,
                half * s,
                (m.zAxis.Y() + m.yAxis.Z()) * s2,
                (m.zAxis.X() - m.xAxis.Z()) * s2
                );
        } 
        else 
        {   // m[2][2] is greatest 
            VecFloat s = Sqrt((m.zAxis.Z() - (m.xAxis.X() + m.yAxis.Y())) + GetVecFloat_One());
            VecFloat s2 = half / s;
            return Quaternion(
                (m.xAxis.Z() + m.zAxis.X()) * s2,
                (m.zAxis.Y() + m.yAxis.Z()) * s2,
                half * s,
                (m.xAxis.Y() - m.yAxis.X()) * s2
                );
        } 
    }
}
#endif

RW_MATH_FORCE_INLINE  Quaternion QuaternionFromMatrix44(Matrix44::InParam m, VecFloat::InParam epsilon) 
{
    return QuaternionFromMatrix33( Matrix33(m.xAxis.mV, m.yAxis.mV, m.zAxis.mV), epsilon );
}

RW_MATH_FORCE_INLINE  Quaternion QuaternionFromMatrix44Affine(Matrix44Affine::InParam m, VecFloat::InParam epsilon) 
{
    return QuaternionFromMatrix33( Matrix33(m.xAxis, m.yAxis, m.zAxis), epsilon );
}

RW_MATH_FORCE_INLINE Quaternion QuaternionFromEulerXYZ(Vector3::InParam euler)
{
    VectorIntrinsic ti_tj_th;
    VectorIntrinsic sin_values, cos_values;

    const VecFloat half(RW_MATH_VPU_GetHalf_R());
    RW_MATH_VPU_Mult( ti_tj_th, euler.mV, half.mV );
    RW_MATH_VPU_SinCos( sin_values, cos_values, ti_tj_th );

    const Vector3 sinVec(sin_values);
    const Vector3 cosVec(cos_values);

    const VecFloat cc = cosVec.X() * cosVec.Z();
    const VecFloat cs = cosVec.X() * sinVec.Z();
    const VecFloat sc = sinVec.X() * cosVec.Z();
    const VecFloat ss = sinVec.X() * sinVec.Z();

    const VecFloat sj = sinVec.Y();
    const VecFloat cj = cosVec.Y();

    return Quaternion(
        (cj * sc) - (sj * cs),
        (cj * ss) + (sj * cc),
        (cj * cs) - (sj * sc),
        (cj * cc) + (sj * ss)
        );
}

RW_MATH_FORCE_INLINE  Quaternion QuaternionFromLookDirection(Vector3::InParam lookDirection, Vector3::InParam up) 
{
    // Note: lookDirection is actually the same as a "look at" position since we use the origin as the eye position.
    Matrix44 matrix = Matrix44FromLookAt( lookDirection, up, GetVector3_Zero() );
    return QuaternionFromMatrix44(matrix);
}

RW_MATH_FORCE_INLINE  Quaternion QuaternionFromLookAt(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition) 
{
    return QuaternionFromMatrix44( Matrix44FromLookAt( lookAt, up, eyePosition ) );
}

RW_MATH_FORCE_INLINE Vector3 QuaternionGetMatrix44AffineRowX(Quaternion::InParam quat)
{
    Quaternion quatTemp = quat * VecFloat(SQRT_2);
    VecFloat twoXY = quatTemp.X() * quatTemp.Y();
    VecFloat twoXZ = quatTemp.X() * quatTemp.Z();
    VecFloat twoYY = quatTemp.Y() * quatTemp.Y();
    VecFloat twoZZ = quatTemp.Z() * quatTemp.Z();
    VecFloat twoWY = quatTemp.W() * quatTemp.Y();
    VecFloat twoWZ = quatTemp.W() * quatTemp.Z();
    return Vector3(
        GetVecFloat_One() - twoYY - twoZZ,
        twoXY + twoWZ,
        twoXZ - twoWY
        );
}

RW_MATH_FORCE_INLINE Vector3 QuaternionGetMatrix44AffineRowY(Quaternion::InParam quat)
{
    Quaternion quatTemp = quat * VecFloat(SQRT_2);
    VecFloat twoXX = quatTemp.X() * quatTemp.X();
    VecFloat twoXY = quatTemp.X() * quatTemp.Y();
    VecFloat twoYZ = quatTemp.Y() * quatTemp.Z();
    VecFloat twoZZ = quatTemp.Z() * quatTemp.Z();
    VecFloat twoWX = quatTemp.W() * quatTemp.X();
    VecFloat twoWZ = quatTemp.W() * quatTemp.Z();
    return Vector3(
        twoXY - twoWZ,
        GetVecFloat_One() - twoXX - twoZZ,
        twoYZ + twoWX
        );
}

RW_MATH_FORCE_INLINE Vector3 QuaternionGetMatrix44AffineRowZ(Quaternion::InParam quat)
{
    Quaternion quatTemp = quat * VecFloat(SQRT_2);
    VecFloat twoXX = quatTemp.X() * quatTemp.X();
    VecFloat twoXZ = quatTemp.X() * quatTemp.Z();
    VecFloat twoYY = quatTemp.Y() * quatTemp.Y();
    VecFloat twoYZ = quatTemp.Y() * quatTemp.Z();
    VecFloat twoWX = quatTemp.W() * quatTemp.X();
    VecFloat twoWY = quatTemp.W() * quatTemp.Y();
    return Vector3(
        twoXZ + twoWY,
        twoYZ - twoWX,
        VecFloat(RW_MATH_VPU_GetOne_R()) - twoXX - twoYY
        );
}

RW_MATH_FORCE_INLINE Mask4   CompLessThan(Quaternion::InParam a, Quaternion::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, b.mV, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompGreaterThan(Quaternion::InParam a, Quaternion::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompLessEqual(Quaternion::InParam a, Quaternion::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, b.mV, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompGreaterEqual(Quaternion::InParam a, Quaternion::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompEqual(Quaternion::InParam a, Quaternion::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompNotEqual(Quaternion::InParam a, Quaternion::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Quaternion Select(MaskScalar::InParam mask, Quaternion::InParam trueValue, Quaternion::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Quaternion(result);
}

RW_MATH_FORCE_INLINE Quaternion Select(Mask4::InParam mask, Quaternion::InParam trueValue, Quaternion::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Quaternion(result);
}

RW_MATH_FORCE_INLINE Vector3 GetXYZ(Quaternion::InParam q)
{
    return Vector3(q.mV);
}

RW_MATH_FORCE_INLINE Quaternion LoadUnAlignedPacked(const rw::math::fpu::Quaternion &value)
{
    VectorIntrinsic result;
    RW_MATH_VPU_UnalignedLoad(result, reinterpret_cast<const float*>((&value)));
    return Quaternion(result);
}

RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(rw::math::fpu::Quaternion &destination, Quaternion::InParam value)
{
    RW_MATH_VPU_UnalignedStore4(destination, value.mV);
}


// functions return vector by reference

#if RW_MATH_ENABLE_DEPRECATED_API

RW_MATH_FORCE_INLINE  void Negate(Quaternion::InParam quat, Quaternion& r) 
{
    r = Negate(quat);
}

RW_MATH_FORCE_INLINE  void Mult(Quaternion::InParam quat, VecFloat::InParam scalar, Quaternion& r) 
{
    r = Mult(quat, scalar);
}

template<VectorAxis INDEX> void Mult(Quaternion::InParam quat, VecFloatRef<INDEX> s, Quaternion& r)
{
    r = Mult(quat, s);
}

RW_MATH_FORCE_INLINE  void Mult(Quaternion::InParam quat, VecFloatRefIndex::InParam s, Quaternion& r) 
{
    r = Mult(quat, s);
}

RW_MATH_FORCE_INLINE  void Divide(Quaternion::InParam quat, VecFloat::InParam scalar, Quaternion& r) 
{
    r = Divide(quat, scalar);
}

template<VectorAxis INDEX> void Divide(Quaternion::InParam quat, VecFloatRef<INDEX> s, Quaternion& r)
{
    r = Divide(quat, s);
}

RW_MATH_FORCE_INLINE  void Divide(Quaternion::InParam quat, VecFloatRefIndex::InParam s, Quaternion& r) 
{
    r = Divide(quat, s);
}

RW_MATH_FORCE_INLINE  void Add(Quaternion::InParam a, Quaternion::InParam b, Quaternion& r) 
{
    r = Add(a,b);
}

RW_MATH_FORCE_INLINE  void Subtract(Quaternion::InParam a, Quaternion::InParam b, Quaternion& r) 
{
    r = Subtract(a,b);
}

RW_MATH_FORCE_INLINE  void Dot(Quaternion::InParam a, Quaternion::InParam b, VecFloat& r) 
{
    r = Dot(a,b);
}

RW_MATH_FORCE_INLINE  void Mult(Quaternion::InParam a, Quaternion::InParam b, Quaternion& r) 
{
    r = Mult(a, b);
}

RW_MATH_FORCE_INLINE  void Normalize(Quaternion::InParam quat, Quaternion& r) 
{
    r = Normalize(quat);
}

RW_MATH_FORCE_INLINE  void Lerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t, Quaternion& r) 
{
    r = Lerp( q0, q1, t );
}

template<VectorAxis INDEX> void Lerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloatRef<INDEX> t, Quaternion& r)
{
    r = Lerp( q0, q1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE  void Lerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloatRefIndex::InParam t, Quaternion& r) 
{
    r = Lerp( q0, q1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE  void SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t, Quaternion& r) 
{
    r = SLerp(v0, v1, t);
}

template<VectorAxis INDEX> void SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRef<INDEX> t, Quaternion& r)
{
    r = SLerp(v0, v1, VecFloat(t));
}

RW_MATH_FORCE_INLINE  void SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRefIndex::InParam t, Quaternion& r) 
{
    r = SLerp(v0, v1, VecFloat(t));
}


RW_MATH_FORCE_INLINE  void NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t, Quaternion& r) 
{
    r = NLerp( v0, v1, t );
}

template<VectorAxis INDEX> void NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRef<INDEX> t, Quaternion& r)
{
    r = NLerp( v0, v1, VecFloat(t) );
}

RW_MATH_FORCE_INLINE  void NLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloatRefIndex::InParam t, Quaternion& r) 
{
    r = NLerp( v0, v1, VecFloat(t) );
}


// additional for Quaternion
RW_MATH_FORCE_INLINE  void Inverse(Quaternion::InParam q, Quaternion& r) 
{
    r = Inverse(q);
}

RW_MATH_FORCE_INLINE  void Conj(Quaternion::InParam q, Quaternion& r) 
{
    r = Conj(q);
}

RW_MATH_FORCE_INLINE  void QuaternionFromAxisAngle(Vector3::InParam q, VecFloat::InParam angle_radians, Quaternion& r) 
{
    r = QuaternionFromAxisRotationAngle( q, angle_radians );
}

template<VectorAxis INDEX> void QuaternionFromAxisAngle(Vector3::InParam q, VecFloatRef<INDEX> angle_radians, Quaternion& r)
{
    r = QuaternionFromAxisRotationAngle( q, VecFloat(angle_radians) );
}

RW_MATH_FORCE_INLINE  void QuaternionFromAxisAngle(Vector3::InParam q, VecFloatRefIndex::InParam angle_radians, Quaternion& r) 
{
    r = QuaternionFromAxisRotationAngle( q, VecFloat(angle_radians) );
}

RW_MATH_FORCE_INLINE  void EulerXYZFromQuaternion(Quaternion::InParam q, Vector3& r) 
{
    r = EulerXYZFromQuaternion(q);
}

RW_MATH_FORCE_INLINE  void Vector3Transform(Quaternion::InParam q, Vector3::InParam v, Vector3& r) 
{
    r = TransformVector(v, q);
}

RW_MATH_FORCE_INLINE  void QuaternionFromMatrix44(Matrix44::InParam m, Quaternion& q, VecFloat::InParam epsilon) 
{
    q = QuaternionFromMatrix44(m, epsilon);
}

RW_MATH_FORCE_INLINE  void QuaternionFromMatrix33(Matrix33::InParam m, Quaternion& q, VecFloat::InParam epsilon) 
{
    q = QuaternionFromMatrix33(m, epsilon);
}

RW_MATH_FORCE_INLINE  void QuaternionFromEulerXYZ(Vector3::InParam euler, Quaternion& r) 
{
    r = QuaternionFromEulerXYZ(euler);
}

#endif


}

}

}

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE  bool operator ==(Quaternion::InParam a, Quaternion::InParam b) 
{
    return _asmIsEqualV4( a.mV, b.mV );
}

RW_MATH_FORCE_INLINE  bool operator !=(Quaternion::InParam a, Quaternion::InParam b) 
{
    return  !_asmIsEqualV4(a.mV, b.mV);
}

RW_MATH_FORCE_INLINE  bool IsSimilar(Quaternion::InParam a, Quaternion::InParam b, VecFloat::InParam epsilon) 
{
    return _asmIsSimilarV4(a.mV, b.mV, epsilon.mV);
}

RW_MATH_FORCE_INLINE  bool IsSimilarRotation(Quaternion::InParam a, Quaternion::InParam b, VecFloat::InParam epsilon) 
{
    VectorIntrinsic epsilonSquared;
    VectorIntrinsic signs;
    VectorIntrinsic dotA_B;
    VectorIntrinsic signMask;
    VectorIntrinsic aTemp;
    VectorIntrinsic difference;
    VectorIntrinsic magnitudeSquared;
    RW_MATH_VPU_GetSignMaskXYZW(signMask);
    RW_MATH_VPU_Mult( epsilonSquared, epsilon.mV, epsilon.mV );
    RW_MATH_VPU_DotV4( dotA_B, a.mV, b.mV );
    RW_MATH_VPU_And(signs, dotA_B, signMask); // Get the signs of Dot(a, b)
    RW_MATH_VPU_Xor( aTemp, a.mV, signs );  // This will flip the sign of 'a' if the result of Dot(a, b) is less than 0.
    RW_MATH_VPU_Subtract( difference, aTemp, b.mV );
    RW_MATH_VPU_DotV4( magnitudeSquared, difference, difference );

    bool result;
    RW_MATH_VPU_IsZeroA1V4(result, magnitudeSquared, epsilonSquared);
    return result;
}

RW_MATH_FORCE_INLINE  bool IsValid(Quaternion::InParam q) 
{
    return (IsValid(q.X()) && IsValid(q.Y()) && IsValid(q.Z()) && IsValid(q.W()));
}

RW_MATH_FORCE_INLINE  bool IsIdentity(Quaternion::InParam q, VecFloat::InParam tolerance) 
{
    return IsSimilar( q, GetQuaternion_Identity(), tolerance );
}

RW_MATH_FORCE_INLINE  bool IsUnit(Quaternion::InParam q) 
{
    return IsSimilar( MagnitudeSquared(q), GetVecFloat_One(), VecFloat(SMALL_FLOAT) );
}

RW_MATH_FORCE_INLINE  bool IsPure(Quaternion::InParam q) 
{
    bool result = IsZero(q.W());
    return result;
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Quaternion::InParam q) 
{
    bool equal;
    RW_MATH_VPU_IsEqualV4(equal, q.mV, q.mV);
    return !equal;
}

RW_MATH_FORCE_INLINE  Quaternion& operator *= (Quaternion& v, VecFloat::InParam a) 
{
    RW_MATH_VPU_Mult( v.mV, v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator /= (Quaternion& v, VecFloat::InParam a) 
{
    RW_MATH_VPU_Divide( v.mV, v.mV, a.mV );
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator += (Quaternion& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Add_R(v.mV,a.mV);
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator -= (Quaternion& v, VecFloat::InParam a) 
{
    v.mV = RW_MATH_VPU_Subtract_R(v.mV,a.mV);
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion operator * (Quaternion::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, s.mV );
    return Quaternion( ret );
}

RW_MATH_FORCE_INLINE  Quaternion operator * (VecFloat::InParam s, Quaternion::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, s.mV, v.mV );
    return Quaternion( ret );
}

RW_MATH_FORCE_INLINE  Quaternion operator / (Quaternion::InParam v, VecFloat::InParam s) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Divide( ret, v.mV, s.mV );
    return Quaternion( ret );
}

RW_MATH_FORCE_INLINE  Quaternion operator + (Quaternion::InParam v, VecFloat::InParam s) 
{
    return Quaternion(RW_MATH_VPU_Add_R(v.mV,s.mV));
}

RW_MATH_FORCE_INLINE  Quaternion operator - (Quaternion::InParam v, VecFloat::InParam s) 
{
    return Quaternion(RW_MATH_VPU_Subtract_R(v.mV,s.mV));
}

RW_MATH_FORCE_INLINE  Quaternion operator + (VecFloat::InParam s, Quaternion::InParam v) 
{
    return Quaternion(RW_MATH_VPU_Add_R(v.mV,s.mV));
}

RW_MATH_FORCE_INLINE  Quaternion operator - (VecFloat::InParam s, Quaternion::InParam v) 
{
    return Quaternion(RW_MATH_VPU_Subtract_R(s.mV, v.mV));
}


// float
Quaternion& operator *= (Quaternion& v, float s)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    v.mV = ret;
    return v;
}

Quaternion& operator /= (Quaternion& v, float s)
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    divisor = RW_MATH_VPU_CreateVectorFromScalar_R(s);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    v.mV = result;
    return v;
}

Quaternion& operator += (Quaternion& v, float s)
{
    v.mV = RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return v;
}

Quaternion& operator -= (Quaternion& v, float s)
{
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return v;
}

Quaternion operator * (Quaternion::InParam v, float s)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return Quaternion(ret);
}

Quaternion operator * (float s, Quaternion::InParam v)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult( ret, v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s));
    return Quaternion(ret);
}

Quaternion operator / (Quaternion::InParam v, float s)
{
    VectorIntrinsic result;
    VectorIntrinsic divisor;
    divisor = RW_MATH_VPU_CreateVectorFromScalar_R(s);
    RW_MATH_VPU_Divide( result, v.mV, divisor );
    return Quaternion(result);
}

Quaternion operator + (Quaternion::InParam v, float s)
{
    return Quaternion(RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

Quaternion operator - (Quaternion::InParam v, float s)
{
    return Quaternion(RW_MATH_VPU_Subtract_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

Quaternion operator + (float s, Quaternion::InParam v)
{
    return Quaternion(RW_MATH_VPU_Add_R(v.mV, RW_MATH_VPU_CreateVectorFromScalar_R(s)));
}

Quaternion operator - (float s, Quaternion::InParam v)
{
    return Quaternion(RW_MATH_VPU_Subtract_R(RW_MATH_VPU_CreateVectorFromScalar_R(s), v.mV));
}


// VecFloatRef
template<VectorAxis INDEX> Quaternion& operator *= (Quaternion& v, VecFloatRef<INDEX> a)
{
    RW_MATH_VPU_MultAxisA1_I( v.mV, v.mV, a.mV, INDEX );
    return v;
}

template<VectorAxis INDEX> Quaternion& operator /= (Quaternion& v, VecFloatRef<INDEX> a)
{
    RW_MATH_VPU_DivideAxisA1_I( v.mV, v.mV, a.mV, INDEX );
    return v;
}

template<VectorAxis INDEX> Quaternion& operator += (Quaternion& v, VecFloatRef<INDEX> a)
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I( aVI, a.mV, INDEX );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX> Quaternion& operator -= (Quaternion& v, VecFloatRef<INDEX> a)
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat_I( aVI, a.mV, INDEX );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

template<VectorAxis INDEX> Quaternion operator * (Quaternion::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Quaternion(result);
}

template<VectorAxis INDEX> Quaternion operator * (VecFloatRef<INDEX> s, Quaternion::InParam v)
{
    VectorIntrinsic result;
    RW_MATH_VPU_MultAxisA1_I( result, v.mV, s.mV, INDEX );
    return Quaternion(result);
}

template<VectorAxis INDEX> Quaternion operator / (Quaternion::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic result;
    RW_MATH_VPU_DivideAxisA1_I( result, v.mV, s.mV, INDEX );
    return Quaternion(result);
}

template<VectorAxis INDEX> Quaternion operator + (Quaternion::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Quaternion(ret);
}

template<VectorAxis INDEX> Quaternion operator - (Quaternion::InParam v, VecFloatRef<INDEX> s)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Quaternion(ret);
}

template<VectorAxis INDEX> Quaternion operator + (VecFloatRef<INDEX> s, Quaternion::InParam v)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Quaternion(ret);
}

template<VectorAxis INDEX> Quaternion operator - (VecFloatRef<INDEX> s, Quaternion::InParam v)
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat_I( sVI, s.mV, INDEX );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Quaternion(ret);
}


// VecFloatRefIndex
RW_MATH_FORCE_INLINE  Quaternion& operator *= (Quaternion& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Mult( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator /= (Quaternion& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    RW_MATH_VPU_Divide( v.mV, v.mV, aVI );
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator += (Quaternion& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Add_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator -= (Quaternion& v, VecFloatRefIndex::InParam a) 
{
    VectorIntrinsic aVI;
    RW_MATH_VPU_Splat( aVI, a.mV, a.mIndex );
    v.mV = RW_MATH_VPU_Subtract_R(v.mV, aVI);
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion operator * (Quaternion::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, v.mV, sVI );
    return Quaternion(result);
}

RW_MATH_FORCE_INLINE  Quaternion operator * (VecFloatRefIndex::InParam s, Quaternion::InParam v) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Mult( result, sVI, v.mV );
    return Quaternion(result);
}

RW_MATH_FORCE_INLINE  Quaternion operator / (Quaternion::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic result;
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    RW_MATH_VPU_Divide( result, v.mV, sVI );
    return Quaternion(result);
}

RW_MATH_FORCE_INLINE  Quaternion operator + (Quaternion::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion operator - (Quaternion::InParam v, VecFloatRefIndex::InParam s) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(v.mV, sVI);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion operator + (VecFloatRefIndex::InParam s, Quaternion::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Add_R(v.mV, sVI);
    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion operator - (VecFloatRefIndex::InParam s, Quaternion::InParam v) 
{
    VectorIntrinsic sVI;
    RW_MATH_VPU_Splat( sVI, s.mV, s.mIndex );
    VectorIntrinsic ret = RW_MATH_VPU_Subtract_R(sVI, v.mV);
    return Quaternion(ret);
}


//
// Quaternion operations
//

RW_MATH_FORCE_INLINE  Quaternion operator - (Quaternion::InParam v) 
{
    return Negate(v);
}

RW_MATH_FORCE_INLINE  Quaternion& operator += (Quaternion& v, Quaternion::InParam b) 
{
    v = Add(v,b);
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion& operator -= (Quaternion& v, Quaternion::InParam b) 
{
    v = Subtract(v,b);
    return v;
}

RW_MATH_FORCE_INLINE  Quaternion operator - (Quaternion::InParam a, Quaternion::InParam b) 
{
    return Subtract(a,b);
}

RW_MATH_FORCE_INLINE  Quaternion operator + (Quaternion::InParam a, Quaternion::InParam b) 
{
    return Add(a, b);
}

RW_MATH_FORCE_INLINE  Quaternion operator * (Quaternion::InParam a, Quaternion::InParam b) 
{
    return Mult(a, b);
}

RW_MATH_FORCE_INLINE Quaternion Min(Quaternion::InParam v0, Quaternion::InParam v1) 
{
    VectorIntrinsic r;
    RW_MATH_VPU_Min(r, v0.mV, v1.mV);
    return Quaternion(r);
}

RW_MATH_FORCE_INLINE Quaternion Max(Quaternion::InParam v0, Quaternion::InParam v1) 
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    return Quaternion(r);
}

RW_MATH_FORCE_INLINE Quaternion Clamp(Quaternion::InParam v0, Quaternion::InParam v1, Quaternion::InParam v2)
{
    VectorIntrinsic r;
    RW_MATH_VPU_Max(r, v0.mV, v1.mV);
    RW_MATH_VPU_Min(r, r, v2.mV);
    return Quaternion(r);
}

RW_MATH_FORCE_INLINE Quaternion Abs(Quaternion::InParam v) 
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Abs(ret, v.mV);
    return Quaternion(ret);
}

} // namespace vpu

} // namespace math

} // namespace rw


#endif
