// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_QUATERNION_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_QUATERNION_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpu/detail/wii/quaternion_operation_platform_inline.h"
#else
#include "rw/math/fpu/scalar.h"
#include "rw/math/fpu/trig.h"
#include "rw/math/vpu/matrix33_operation.h"
#include "rw/math/vpu/matrix44_operation.h"
#include "rw/math/vpu/detail/soa4_float_operation_inline.h"
#include "rw/math/vpu/soa4_trig.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE  Quaternion Mult(Quaternion::InParam a, Quaternion::InParam b) 
{
    //Vector3 v1(a.mV);
    //Vector3 v2(b.mV);
    //Vector3 lImaginary(v2*a.W() + v1*b.W() + Cross(v1, v2));
    //VecFloat lrReal = a.W()*b.W() - Dot(v1, v2);
    //return Quaternion(lImaginary.X(), lImaginary.Y(), lImaginary.Z(), lrReal);

    VectorIntrinsic aW, bW, imaginary, real, dot_a_b, aW_mult_bW;
    RW_MATH_VPU_Splat_I(aW, a.mV, VectorAxisW);
    RW_MATH_VPU_Splat_I(bW, b.mV, VectorAxisW);
    RW_MATH_VPU_CrossV3(imaginary, a.mV, b.mV);
    RW_MATH_VPU_MultAdd(imaginary, b.mV, aW, imaginary);
    RW_MATH_VPU_MultAdd(imaginary, a.mV, bW, imaginary);

    RW_MATH_VPU_DotV3(dot_a_b, a.mV, b.mV);
    RW_MATH_VPU_Mult(aW_mult_bW, aW, bW);
    RW_MATH_VPU_Subtract(real, aW_mult_bW, dot_a_b);

    RW_MATH_VPU_SwizzleStore_I_I(imaginary, imaginary, VectorAxisW, real, VectorAxisX);
    return Quaternion(imaginary);
}

RW_MATH_FORCE_INLINE  Quaternion Lerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
{
    //if (Dot(q0,q1)>GetVecFloat_Zero())
    //{
    //    return Add(Mult(Subtract(q1,q0),t),q0);
    //}
    //else
    //{
    //    return Add(Mult(Add(q1,q0),-t), q0);
    //}

    VectorIntrinsic dot_q0_q1, zero_vector, is_dot_q0_q1_gte_zero;
    RW_MATH_VPU_GetZero(zero_vector);
    RW_MATH_VPU_DotV4(dot_q0_q1, q0.mV, q1.mV);
    RW_MATH_VPU_IsGreater(is_dot_q0_q1_gte_zero, dot_q0_q1, zero_vector);

    VectorIntrinsic result_gt_zero, result_lt_zero, result;
    RW_MATH_VPU_Subtract(result_gt_zero, q1.mV, q0.mV);
    RW_MATH_VPU_MultAdd(result_gt_zero, result_gt_zero, t.mV, q0.mV);

    RW_MATH_VPU_Add(result_lt_zero, q1.mV, q0.mV);
    RW_MATH_VPU_MultSubNeg(result_lt_zero, result_lt_zero, t.mV, q0.mV);

    RW_MATH_VPU_Select(result, result_lt_zero, result_gt_zero, is_dot_q0_q1_gte_zero);
    return Quaternion(result);
}

RW_MATH_INLINE Quaternion SLerp(Quaternion::InParam v0, Quaternion::InParam v1, VecFloat::InParam t)
{
    using namespace rw::math::vpu::detail;
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v0)-GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f)), ("v0 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v1)-GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f)), ("v1 must be of unit length") );

    // Jan: since we may have to negate v0, and we don't want to change input data without
    // user knowing about it, we will copy it to an internal variable which we will use
    // instead.

    Quaternion v0temp = v0;
    Quaternion v0tempNeg = Negate(v0temp);
    VecFloat dp = Dot(v0temp,v1);
    VecFloat dpNeg = -dp;

    MaskScalar selMask = CompLessThan(dp, VecFloat(RW_MATH_VPU_GetZero_R()));
    v0temp = Select(selMask, v0tempNeg, v0temp);
    dp = Select(selMask, dpNeg, dp);
    
    if(dp > VecFloat(gCosMinSlerpAngle))  // switch to nlerp if angle < VECTOR_MIN_SLERP_ANGLE
    {
        return NLerp(v0temp,v1,t);
    }

    VecFloat theta = ACos(dp);

    Soa4Float soa4Angles = Soa4FloatFromVecFloats((GetVecFloat_One()-t)*theta, t*theta, theta, theta);
    Soa4Float sinResult = Sin(soa4Angles);
    VecFloat sr0, sr1, sr2, sr2Reciprocal;
    Soa4FloatToVecFloats(sinResult, sr0, sr1, sr2, sr2);
    sr2Reciprocal = Reciprocal(sr2);

    v0temp = Mult(v0temp, sr0 * sr2Reciprocal);
    Quaternion result;
    sr1 = sr1 * sr2Reciprocal;
    RW_MATH_VPU_MultAdd(result.mV, v1.mV, sr1.mV, v0temp.mV);
    return result;
}

RW_MATH_INLINE Quaternion SLerpNoCheck(Quaternion::InParam quat1, Quaternion::InParam quat2, VecFloat::InParam t)
{
    RW_MATH_ASSERT_MSG(Abs(Magnitude(quat1)-GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f)), ("quat1 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(Magnitude(quat2)-GetVecFloat_One()) < VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f)), ("quat2 must be of unit length") );
    RW_MATH_ASSERT_MSG(t <= GetVecFloat_One() && t >= GetVecFloat_Zero(), ("t out of bounds; need 0<=t<=1") );

    VecFloat alpha = t;
    VecFloat beta = VecFloat(RW_MATH_VPU_GetOne_R()) - alpha;

    // Compute dot product (the cosine of the angle between quaternions)
    VecFloat cosTheta = Dot( quat1, quat2 );

    // If the quaternions are close together, do linear interpolation (for speed)
    // but if the quaternions aren't close, do the spherical interpolation
    if( cosTheta < VecFloat(QUATERNION_SLERP_DOT_TOLERANCE) )
    {
        VecFloat lrTheta = ACos(cosTheta);
        VecFloat lrCosecTheta = Sin(lrTheta);
        lrCosecTheta = VecFloat(RW_MATH_VPU_GetOne_R()) / lrCosecTheta;

        beta *=  lrTheta;
        beta = Sin(beta);
        beta *=  lrCosecTheta;

        alpha *=  lrTheta;
        alpha = Sin(alpha);
        alpha *=  lrCosecTheta;
    }

    // Do the interpolation for the rotation
    return Quaternion(beta * quat1 + alpha * quat2);
}

// additional for Quaternion
RW_MATH_FORCE_INLINE  Quaternion Inverse(Quaternion::InParam q) 
{
    // s = 1.0f / Dot(q, q);
    VectorIntrinsic s;
    RW_MATH_VPU_DotV4(s, q.mV, q.mV);
    RW_MATH_VPU_Reciprocal(s, s);

    // conj(q) * s;
    VectorIntrinsic v, mask;
    RW_MATH_VPU_GetSignMaskXYZ(mask);
    RW_MATH_VPU_Xor(v, q.mV, mask);

    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, v, s);

    return Quaternion(ret);
}

RW_MATH_FORCE_INLINE  Quaternion Conj(Quaternion::InParam q) 
{
    VectorIntrinsic v, mask;
    RW_MATH_VPU_GetSignMaskXYZ(mask);
    RW_MATH_VPU_Xor(v, q.mV, mask);
    return Quaternion(v);
}

/*********************************************************************************************

    CompressRotation

    Description:
    Compresses the quaternion's rotation into 32 bits

    Returns:
    Compressed rotation

    Bits 00-09 inclusive contain W (10 bits)
    Bits 10-13 inclusive contain sign bits (10 = sX, 11 = sY, 12 = sZ, 13 = sW) (4 bits)
    Bits 14-31 inclusive contain E (18 bits)

    (from Games Programming Gems 3 section 2.4f)

*********************************************************************************************/
RW_MATH_INLINE uint32_t CompressRotation(Quaternion::InParam quat)
{
    // Does not work on non unit Quats
    RW_MATH_ASSERT_MSG(IsUnit(quat), ("quat must be of unit length"));

    uint32_t    coord;
    uint32_t    row;
    uint32_t    col;
    uint32_t    colCount;
    uint32_t    result;
    VecFloat    rowAngle;
    VecFloat    colAngle;
    Vector4     absQuat(Abs(quat.X()), Abs(quat.Y()), Abs(quat.Z()), Abs(quat.W()));
    Vector3     axis;
    VecFloat    length;
    const uint32_t wBits = 10;
    const uint32_t maxW  = (1 << wBits) - 1;
    const VecFloat zero = GetVecFloat_Zero();
    const VecFloat one = GetVecFloat_One();

    // no longer as const, used in place, so we don't get warning on release build
    // const uint32_t eBits = 18;
    // const uint32_t maxE  = (1 << eBits) - 1;
    
    // Store W
    result = (uint32_t)(absQuat.W().GetFloat() * 1023.0f);
    if(result > maxW)
    {
        result = maxW;
    }

    //store the signs of each value
    if (quat.X() < zero) result |= (1 << 10);
    if (quat.Y() < zero) result |= (1 << 11);
    if (quat.Z() < zero) result |= (1 << 12);
    if (quat.W() < zero) result |= (1 << 13);
    
    // Calculate axis (X,Y,Z), within the positive eighth of the sphere
    axis.Set(absQuat.X(), absQuat.Y(), absQuat.Z());
    length = Magnitude(axis);
    if (length < VecFloat(0.001f))
    {
        // Quaternion is very close to (0,0,0,1)

        // Set row and column angles
        rowAngle = VecFloat(HALF_PI);
        colAngle = VecFloat(ZERO_FLOAT);
    }
    else
    {
        // Normalize axis
        axis *= Reciprocal(length);

        // really make sure we're in the right range for sin/cos calls
        // rounding error may have pushed us just over or under
        axis.X() = ClampToNegPosRange(axis.X(), one);
        axis.Y() = ClampToNegPosRange(axis.Y(), one);
        axis.Z() = ClampToNegPosRange(axis.Z(), one);
    
        // Calculate row and column angles
        rowAngle = ACos(axis.Y());
        colAngle = ASin(axis.X());
    }

    // Calc row index (by the way, 511.0f == sqrt( 2^18bits ) - 1)
    row = (uint32_t)((rowAngle.GetFloat() / HALF_PI) * 511.0f);

    RW_MATH_ASSERT_MSG(row < 512, ("row index out of bounds"));

    //calc column count (= number of positions in this row - 1)
    colCount = row * 2;

    //calc column
    col = (uint32_t)((colAngle.GetFloat() / HALF_PI) * (float)colCount);

    //calc E
    coord = (row * row) + col;

    // maxE is replaced with (1 << eBits) - 1, so we don't get warning in release build.
    //RW_MATH_ASSERT_MSG( coord <= maxE, ("coord > maxE") );
    RW_MATH_ASSERT_MSG( coord <= ((1 << 18) - 1), ("coord > maxE") );

    
    //store E
    result |= (coord << 14);
    
    //return compressed rotation
    return result;
}

/*********************************************************************************************

    UncompressRotation

    Description:
     Uncompresses the compressed data into our rotation data.

*********************************************************************************************/
RW_MATH_INLINE Quaternion UnCompressRotation(uint32_t rotation)
{
    int32_t  e;
    int32_t  w;
    int32_t  row;
    int32_t  col;
    int32_t  colCount;
    VecFloat rowAngle;
    VecFloat colAngle;
    VecFloat squaredZ;
    VecFloat axisScale;
    Quaternion  result;
    const VecFloat zero = GetVecFloat_Zero();
    const VecFloat one = GetVecFloat_One();

    // Extract E
    e = static_cast<int32_t>(rotation >> 14);

    // Calculate row and column
    row = (int32_t) rw::math::fpu::Sqrt( static_cast<float>(e) );
    col = e - (row * row);

    // Calculate column count = total - 1
    colCount = 2 * row;

    // Calculate row and column angles
    rowAngle = ( (float)row / 511.0f ) * HALF_PI;
    if(colCount > 0)
    {
        colAngle = ( (float)col / (float)colCount ) * HALF_PI;
    }
    else
    {
        colAngle = zero;
    }

    // Set X and Y
    result.Set(Sin(colAngle), Cos(rowAngle), zero, zero);

    // extract W
    w = static_cast<int32_t>(rotation & 1023 );
    result.W() =(float)w / 1023.0f;

    // Sin(Theta) = sqrtf(1-cosf(Theta)^2)
    axisScale = Sqrt(one - result.W() * result.W());
    if (axisScale > VecFloat(0.001f))
    {
        result.X() *= axisScale;
        result.Y() *= axisScale;
    }
    else
    {
        // Quaternion is close to (0,0,0,1)
        result.X() = zero;
        result.Y() = zero;
        result.Z() = zero;
        return result;
    }

    // calculate Z
    // Rounding/accuracy errors mean squaredZ may end up being slightly negative. 
    // So we must be careful not to take the sqrt of a negative number
    squaredZ = one - (result.X() * result.X())  -  
        (result.Y() * result.Y()) - (result.W() * result.W());
    // "Clamp" squaredZ above 0.
    squaredZ = Max(squaredZ, zero);
    result.Z() = Sqrt(squaredZ);

    //extract signs
    if( rotation & (1 << 10) ) result.X() =-result.X();
    if( rotation & (1 << 11) ) result.Y() =-result.Y();
    if( rotation & (1 << 12) ) result.Z() =-result.Z();
    if( rotation & (1 << 13) ) result.W() =-result.W();

    return result;
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
#endif
