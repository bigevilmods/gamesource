// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_QUATERNION_OPERATION_INLINE_H
#define RW_MATH_FPU_QUATERNION_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/constants.h"
#include "rw/math/fpu/trig.h"
#include "rw/math/fpu/scalar.h"

#include "rw/math/fpu/constants_operation.h"

#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/quaternion.h"

#include "rw/math/fpu/matrix44.h"
#include "rw/math/fpu/matrix44affine.h"
#include "rw/math/fpu/matrix33.h"
#include "rw/math/fpu/matrix33_operation.h"

#include "rw/math/fpu/mask4_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>& operator +=(QuaternionTemplate<T>& vec, const T& scalar)
{
    vec.Set(vec.x+scalar, vec.y+scalar, vec.z+scalar, vec.w+scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>& operator -=(QuaternionTemplate<T>& vec, const T& scalar)
{
    vec.Set(vec.x-scalar, vec.y-scalar, vec.z-scalar, vec.w-scalar);
    return vec;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>& operator *=(QuaternionTemplate<T>& vec, const T& scalar)
{
    vec.Set(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return vec;
}
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>& operator /=(QuaternionTemplate<T>& vec, const T& scalar)
{
    T scalarInv= static_cast<T>(1.0f)/scalar;
    vec.Set(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
    return vec;
}
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>& operator +=(QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    a.Set(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
    return a;
}
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>& operator -=(QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    a.Set(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
    return a;
}

/*! 
    Negate this quaternion
*/
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator - (const QuaternionTemplate<T>& vec)
{
    QuaternionTemplate<T> r(-vec.x, -vec.y, -vec.z, -vec.w);
    return r;
}

/*!
 Multiplies two quaternions together.

 Note: This routine does \e not perform a component wise multiplication of quaternions.  Instead,
 it performs quaternion multiplication.

*/
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator * (const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    return Mult(a, b);
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator * (const QuaternionTemplate<T>& vec, const T& scalar)
{
    QuaternionTemplate<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator * (const T& scalar, const QuaternionTemplate<T>& vec)
{
    QuaternionTemplate<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator / (const QuaternionTemplate<T>& vec, const T& scalar)
{
    T scalarInv= static_cast<T>(1.0f)/scalar;
    QuaternionTemplate<T> r(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator + (const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    QuaternionTemplate<T> r(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator + (const QuaternionTemplate<T>& a, const T& s)
{
    QuaternionTemplate<T> r(a.x+s, a.y+s, a.z+s, a.w+s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator + (const T& s, const QuaternionTemplate<T>& a)
{
    QuaternionTemplate<T> r(a.x+s, a.y+s, a.z+s, a.w+s);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator - (const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    QuaternionTemplate<T> r(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator - (const QuaternionTemplate<T>& a, const T& s)
{
    QuaternionTemplate<T> r(a.x-s, a.y-s, a.z-s, a.w-s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>  operator - (const T& s, const QuaternionTemplate<T>& a)
{
    QuaternionTemplate<T> r(s-a.x, s-a.y, s-a.z, s-a.w);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE T MagnitudeSquared(const QuaternionTemplate<T>& vec)
{
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w;
}

template<class T>
RW_MATH_FORCE_INLINE bool IsAnyNaN(const QuaternionTemplate<T>& q)
{
    return (bool)(IsNaN(q.x) | IsNaN(q.y) | IsNaN(q.z) | IsNaN(q.w));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsValid(const QuaternionTemplate<T>&q)
{
    return (IsValid(q.x) && IsValid(q.y) && IsValid(q.z) && IsValid(q.w));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsIdentity(const QuaternionTemplate<T>&q, T tolerance = EPSILON)
{
    return IsSimilar( q, GetQuaternion_IdentityTemplate<T>(), tolerance );
}

template<class T>
RW_MATH_FORCE_INLINE bool IsUnit(const QuaternionTemplate<T>&q)
{
    return IsSimilar( MagnitudeSquared<T>(q), static_cast<T>(1.0f), static_cast<T>(SMALL_FLOAT) );
}

template<class T>
RW_MATH_FORCE_INLINE bool IsPure(const QuaternionTemplate<T>&q)
{
    bool result = IsZero(q.w);
    return result;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Min(const QuaternionTemplate<T>& v0, const QuaternionTemplate<T>& v1)
{
    return QuaternionTemplate<T>(Min(v0.x, v1.x), Min(v0.y, v1.y), Min(v0.z, v1.z), Min(v0.w, v1.w));
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Max(const QuaternionTemplate<T>& v0, const QuaternionTemplate<T>& v1)
{
    return QuaternionTemplate<T>(Max(v0.x, v1.x), Max(v0.y, v1.y), Max(v0.z, v1.z), Max(v0.w, v1.w));
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Clamp(const QuaternionTemplate<T>& quatToClamp, const QuaternionTemplate<T>& minQuatValue, const QuaternionTemplate<T>& maxQuatValue )
{
    QuaternionTemplate<T> retVec(Clamp(quatToClamp.x, minQuatValue.x, maxQuatValue.x), Clamp(quatToClamp.y, minQuatValue.y, maxQuatValue.y), Clamp(quatToClamp.z, minQuatValue.z, maxQuatValue.z), Clamp(quatToClamp.w, minQuatValue.w, maxQuatValue.w));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Abs(const QuaternionTemplate<T>& v)
{
    return QuaternionTemplate<T>(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));
}

template<class T> 
RW_MATH_FORCE_INLINE bool operator ==(const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}

template<class T> 
RW_MATH_FORCE_INLINE bool operator !=(const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    return (a.x!=b.x || a.y!=b.y || a.z!=b.z || a.w!=b.w);
}

/*!
 * This function may be used to determine if two quaternions are element wise similar.
 *
 * Note: This function compares pair of elements to see if there difference is
 * less that \c epsilon.  If you would like to determine if two quaternions represent
 * the same rotation, you may want to consider using \c IsSimilarRotation instead.
 *
 * \param epsilon A tolerance used during comparison.
 *
 * \return Returns true iff the two quaternions have similar values (element wise).
 */
template<class T> 
RW_MATH_INLINE bool IsSimilar(const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b, T epsilon = (T)(SMALL_FLOAT))
{
    return (Abs(a.x-b.x) <= epsilon && 
            Abs(a.y-b.y) <= epsilon &&
            Abs(a.z-b.z) <= epsilon &&
            Abs(a.w-b.w) <= epsilon);
}

/*!
 * This function may be used to determine if two quaternions represent the same rotation.
 *
 * Note: The result of this function is only defined for quaternions that are
 * normalized.
 *
 * \param epsilon A tolerance used during comparison.
 *
 * \return Returns true iff the two quaternions represent the same rotation.
 */
template<class T> 
RW_MATH_INLINE bool IsSimilarRotation(const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b, T epsilon = (T)(SMALL_FLOAT))
{
    QuaternionTemplate<T> A(a);
    if (Dot(a, b) < static_cast<T>(0.0f))
    {
        // quaternions are in opposite hemispheres, so negate one
        A = Negate(A);
    }
    T magSquared = MagnitudeSquared(A - b);
    T epsilonSquared = (epsilon * epsilon);
    return magSquared < epsilonSquared;
}

template<class T> 
RW_MATH_FORCE_INLINE T Magnitude(const QuaternionTemplate<T>& vec)
{
    return Sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
}

template<class T> 
RW_MATH_FORCE_INLINE T MagnitudeRecip(const QuaternionTemplate<T>& vec)
{
    return static_cast<T>(1.0f) / Sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
}

template<class T> 
RW_MATH_FORCE_INLINE T Dot(const QuaternionTemplate<T>& a, const QuaternionTemplate<T>& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

// functions return vector by value (default method of return vector)

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T>       Pow(const QuaternionTemplate<T>& vec, T exponent)
{
    QuaternionTemplate<T> r;
    // Check for the case of an identity quaternion.
    // This will protect against divide by zero
    if (fabs(vec.w) < .9999f) 
    {
        // Extract the half angle alpha (alpha = theta/2)
        T alpha = static_cast<T>(acos(vec.w));
        // Compute new alpha value
        T newAlpha = alpha * exponent;
        // Compute new w value
        r.w = static_cast<T>(cos(newAlpha));
        // Compute new xyz values
        T mult = static_cast<T>(sin(newAlpha) / sin(alpha));
        r.x = vec.x * mult;
        r.y = vec.y * mult;
        r.z = vec.z * mult;
    }

    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>       Negate(const QuaternionTemplate<T>& vec)
{
    QuaternionTemplate<T> r(-vec.x, -vec.y, -vec.z, -vec.w);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>         Mult(const QuaternionTemplate<T>& vec,     T scalar)
{
    QuaternionTemplate<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>       Divide(const QuaternionTemplate<T>& vec,     T scalar)
{
    T scalarInv= static_cast<T>(1.0f)/scalar;
    QuaternionTemplate<T> r(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>          Add(const QuaternionTemplate<T>& a,       const QuaternionTemplate<T>& b)
{
    QuaternionTemplate<T> r(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>     Subtract(const QuaternionTemplate<T>& a,       const QuaternionTemplate<T>& b)
{
    QuaternionTemplate<T> r(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
    return r;
}

// Note: For now the fast version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>    NormalizeFast(const QuaternionTemplate<T>& vec)
{
    QuaternionTemplate<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

// Note: For now the estimate version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>    NormalizeEstimate(const QuaternionTemplate<T>& vec)
{
    QuaternionTemplate<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>    Normalize(const QuaternionTemplate<T>& vec)
{
    QuaternionTemplate<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>    Normalize(const QuaternionTemplate<T>& q, T &magnitude)
{
    magnitude = Magnitude(q);
    return QuaternionTemplate<T> (Mult(q, static_cast<T>(1.0f)/magnitude));
}

template<class T>
RW_MATH_INLINE T NormalizeReturnMagnitude(const QuaternionTemplate<T>& q, QuaternionTemplate<T>& r)
{
    T l = MagnitudeSquared(q);

    if (l > static_cast<T>(0.0f))
    {
        T invl = InvSqrt(l);
        r = Mult(q, invl);
        return l*invl;
    }

    return static_cast<T>(0.0f);
}

/*!
 This function normalizes quaternions that have non-zero magnitude, and gracefully handles quaternions with magnitudes that are zero.

 \param[in] q The input quaternion to normalize.
 \param[out] r The result quaternion.  The quaternion will be the result of normalizing 'q', or it will be a zero quaternion if 'q' has a magnitude of zero (or nearly zero).

 \return Returns the magnitude of the quaternion
 */
template<class T>
RW_MATH_INLINE T CheckedNormalizeReturnMagnitude(const QuaternionTemplate<T>& q, QuaternionTemplate<T>& r)
{
    T l = MagnitudeSquared(q);

    if (l > static_cast<T>(0.0f))
    {
        T invl = InvSqrt(l);
        r = Mult(q, invl);
        return l*invl;
    }
    else
    {
        r.SetZero();        
        return static_cast<T>(0.0f);
    }
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>         Mult(const QuaternionTemplate<T>& a,       const QuaternionTemplate<T>& b)
{
    Vector3Template<T> v1(a.x, a.y, a.z);
    Vector3Template<T> v2(b.x, b.y, b.z);
    Vector3Template<T> lImaginary(v2*a.w + v1*b.w + Cross(v1, v2));
    T lrReal = a.w*b.w - Dot(v1, v2);
    return QuaternionTemplate<T>(lImaginary.x, lImaginary.y, lImaginary.z, lrReal);

}

/*! 
    Linear interpolation between two quaternions.

    Lerp calculates a linear interpolation between two quaternions.
    It is faster than the SLerp function.

*/
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Lerp(const QuaternionTemplate<T>& q0, const QuaternionTemplate<T>& q1,     T t)
{
    // bipolar
    if (Dot(q0,q1) > static_cast<T>(0.0f))
    {
        return Add(Mult(Subtract(q1,q0),t),q0);
    }
    else
    {
        return Add(Mult(Add(q1,q0),-t), q0);
    }
    
}
template<class T> 
RW_MATH_INLINE QuaternionTemplate<T>        SLerp(const QuaternionTemplate<T>& v0,      const QuaternionTemplate<T>& v1,     T t)
{
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v0)-static_cast<T>(1.0f)) < static_cast<T>(0.01f), ("v0 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v1)-static_cast<T>(1.0f)) < static_cast<T>(0.01f), ("v1 must be of unit length") );
    RW_MATH_ASSERT_MSG(t<= static_cast<T>(1.0f) && t>= static_cast<T>(0.0f), ("t out of bounds; need 1>=t>=0") );

    // Jan: since we may have to negate v0, and we don't want to change input data without
    // user knowing about it, we will copy it to an internal variable which we will use
    // instead.

    QuaternionTemplate<T> v0temp(v0);

    T dp = Dot(v0temp,v1);

    if(dp < static_cast<T>(0.0f)) 
    {
        //take shorter 4d arc

        v0temp = Negate(v0temp);
        dp = -dp;
    }

    if(dp > Cos(static_cast<T>(VECTOR_MIN_SLERP_ANGLE)))  // switch to nlerp if angle < VECTOR_MIN_SLERP_ANGLE
    {
        return NLerp(v0temp,v1,t);
    }

    T theta = ACos(dp);

    v0temp = Mult(v0temp, Sin((static_cast<T>(1.0f)-t)*theta)/Sin(theta));
    return Add(v0temp, 
            Mult(v1, Sin(t*theta)/Sin(theta))
            );
}
// **********************************************************************************************************
//
//    SLerpNoCheck
//
//    Description:
//        Interpolates (SLERPs) between two quaternions, does not flip if they are in different hemispheres
//
// **********************************************************************************************************
template<class T> 
RW_MATH_INLINE QuaternionTemplate<T> SLerpNoCheck(const QuaternionTemplate<T>& quat1,      const QuaternionTemplate<T>& quat2,     T ratio)
{
    QuaternionTemplate<T> lQuatResult;
    T  alpha;
    T  beta;
    T  cosTheta;

    RW_MATH_ASSERT_MSG(Abs(Magnitude(quat1)-static_cast<T>(1.0f)) < static_cast<T>(0.01), ("quat1 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(Magnitude(quat2)-static_cast<T>(1.0f)) < static_cast<T>(0.01), ("quat2 must be of unit length") );
    RW_MATH_ASSERT_MSG(ratio<= static_cast<T>(1.0f) && ratio>= static_cast<T>(0.0f), ("ratio out of bounds; need 1>=ratio>=0") );

    alpha = ratio;
    beta = static_cast<T>(1.0f) - alpha;

    // Compute dot product (the cosine of the angle between quaternions)
    cosTheta = Dot( quat1, quat2 );

    // If the quaternions are close together, do linear interpolation (for speed)
    // but if the quaternions aren't close, do the spherical interpolation
    if( cosTheta < QUATERNION_SLERP_DOT_TOLERANCE )
    {
        T        lrTheta;
        T        lrCosecTheta;

        lrTheta = ACos(cosTheta);
        lrCosecTheta = Sin(lrTheta);
        lrCosecTheta = static_cast<T>(1.0f) / lrCosecTheta;

        beta *=  lrTheta;
        beta = Sin(beta);
        beta *=  lrCosecTheta;

        alpha *=  lrTheta;
        alpha = Sin(alpha);
        alpha *=  lrCosecTheta;
    }

    // Do the interpolation for the rotation
    lQuatResult.x = beta * quat1.x + alpha * quat2.x ;
    lQuatResult.y = beta * quat1.y + alpha * quat2.y ;
    lQuatResult.z = beta * quat1.z + alpha * quat2.z ;
    lQuatResult.w = beta * quat1.w + alpha * quat2.w ;

    return lQuatResult;
}


template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>        NLerpEstimate(const QuaternionTemplate<T>& q0,      const QuaternionTemplate<T>& q1,     T t)
{
    return NormalizeEstimate( Lerp(q0,q1,t) );
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>        NLerpFast(const QuaternionTemplate<T>& q0,      const QuaternionTemplate<T>& q1,     T t)
{
    return NormalizeFast( Lerp(q0,q1,t) );
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>        NLerp(const QuaternionTemplate<T>& q0,      const QuaternionTemplate<T>& q1,     T t)
{
    return Normalize( Lerp(q0,q1,t) );
}

// additional for Quaternion

/********************************************************************************************

    Concat

    Description:
        Concatenates two quats (quaternion multiply). Result is scaled to length one (unit
        quaternion). Returns a new quaterion as output.

*********************************************************************************************/
template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Concat(const QuaternionTemplate<T>& q0,      const QuaternionTemplate<T>& q1)
{
    // Need to make this resultant quat a unit quat.
    QuaternionTemplate<T> result = Normalize(q0*q1);

    return result;
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
template<class T> 
RW_MATH_INLINE uint32_t CompressRotation(const QuaternionTemplate<T>& quat)
{
    // Does not work on non unit Quats
    RW_MATH_ASSERT_MSG(IsUnit(quat), ("quat must be of unit length"));

    uint32_t    coord;
    uint32_t    row;
    uint32_t    col;
    uint32_t    colCount;
    uint32_t    result;
    T    rowAngle;
    T    colAngle;
    Vector4Template<T>  absQuat(Abs(quat.x), Abs(quat.y), Abs(quat.z), Abs(quat.w));
    Vector3Template<T>  axis;
    T    length;
    const uint32_t wBits = 10;
    const uint32_t maxW  = (1 << wBits) - 1;

    
    // Store W
    result = (uint32_t) (absQuat.w * static_cast<T>(1023.0f));
    if(result > maxW)
    {
        result = maxW;
    }

    //store the signs of each value
    if( quat.x < static_cast<T>(0.0f) ) result |= (1 << 10);
    if( quat.y < static_cast<T>(0.0f) ) result |= (1 << 11);
    if( quat.z < static_cast<T>(0.0f) ) result |= (1 << 12);
    if( quat.w < static_cast<T>(0.0f) ) result |= (1 << 13);
    
    // Calculate axis (X,Y,Z), within the positive eighth of the sphere
    axis.Set(absQuat.x, absQuat.y, absQuat.z);
    length = Magnitude(axis);
    if(length < static_cast<T>(0.001))
    {
        // Quaternion is very close to (0,0,0,1)

        // Set row and column angles
        rowAngle = HALF_PI;
        colAngle = ZERO_FLOAT;
    }
    else
    {
        // Normalize axis
        axis *= static_cast<T>(1.0f) / length;

        // really make sure we're in the right range for sin/cos calls
        // rounding error may have pushed us just over or under
        axis.x = Clamp( axis.x, static_cast<T>(-1.0f), static_cast<T>(1.0f) ) ;
        axis.y = Clamp( axis.y, static_cast<T>(-1.0f), static_cast<T>(1.0f) ) ;
        axis.z = Clamp( axis.z, static_cast<T>(-1.0f), static_cast<T>(1.0f) ) ;
    
        // Calculate row and column angles
        rowAngle = ACos( axis.y );
        colAngle = ASin( axis.x );
    }

    // Calc row index (by the way, 511.0f == sqrt( 2^18bits ) - 1)
    row = (uint32_t)(((rowAngle) / HALF_PI) * static_cast<T>(511.0f));

    RW_MATH_ASSERT_MSG(row < 512, ("row index out of bounds"));

    //calc column count (= number of positions in this row - 1)
    colCount = row * 2;

    //calc column
    col = (uint32_t)((colAngle / HALF_PI) * colCount);

    //calc E
    coord = (row * row) + col;
    //const uint32_t eBits = 18;
    //const uint32_t maxE  = (1 << eBits) - 1;
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
template<class T> 
RW_MATH_INLINE QuaternionTemplate<T>      UnCompressRotation(uint32_t rotation)
{
    int32_t  e;
    int32_t  w;
    int32_t  row;
    int32_t  col;
    int32_t  colCount;
    T   rowAngle;
    T   colAngle;
    T   squaredZ;
    T   axisScale;
    QuaternionTemplate<T>  result;

    // Extract E
    e = static_cast<int32_t>(rotation >> 14);

    // Calculate row and column
    row = (int32_t) Sqrt( static_cast<T>( e) );
    col = e - (row * row);

    // Calculate column count = total - 1
    colCount = 2 * row;

    // Calculate row and column angles
    rowAngle = ( static_cast<T>(row) / static_cast<T>(511.0f) ) * HALF_PI;
    if(colCount > 0)
    {
        colAngle = ( static_cast<T>(col) / static_cast<T>(colCount) ) * HALF_PI;
    }
    else
    {
        colAngle = static_cast<T>(0.0f);
    }

    // Get X and Y
    result.y =Cos( rowAngle );
    result.x =Sin( colAngle );

    //extract W
    w = static_cast<int32_t>(rotation & 1023 );
    result.w =static_cast<T>(w) / static_cast<T>(1023.0f);

    // Sin(Theta) = sqrtf(1-cosf(Theta)^2)
    axisScale = Sqrt(static_cast<T>(1.0f) - (result.w * result.w));
    if(axisScale > static_cast<T>(0.001))
    {
        result.x =result.x * axisScale;
        result.y =result.y * axisScale;
    }
    else
    {
        // Quaternion is close to (0,0,0,1)
        result.x =static_cast<T>(0.0f);
        result.y =static_cast<T>(0.0f);
        result.z =static_cast<T>(0.0f);
        return result;
    }

    // calculate Z
    // Rounding/accuracy errors mean squaredZ may end up being slightly negative. 
    // So we must be careful not to take the sqrt of a negative number
    squaredZ = static_cast<T>(1.0f) - result.x*result.x  -  
                result.y*result.y - result.w*result.w;
    if(squaredZ > static_cast<T>(0.0f))
    {
        result.z =Sqrt( squaredZ );
    }
    else
    {
        result.z =static_cast<T>(0.0f);
    }

    //extract signs
    if( rotation & (1 << 10) ) result.x =-result.x;
    if( rotation & (1 << 11) ) result.y =-result.y;
    if( rotation & (1 << 12) ) result.z =-result.z;
    if( rotation & (1 << 13) ) result.w =-result.w;

    return result;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>      Inverse(const QuaternionTemplate<T>& q)
{
    T scalarInv= static_cast<T>(1.0f)/Dot(q, q);
    QuaternionTemplate<T> r(-q.x*scalarInv, -q.y*scalarInv, -q.z*scalarInv, q.w*scalarInv);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>         Conj(const QuaternionTemplate<T>& q)
{
    QuaternionTemplate<T> r(-q.x, -q.y, -q.z, q.w);
    return r;
}

template<class T> 
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromAxisRotationAngle(const Vector3Template<T>& v, T angle_radians)
{
    RW_MATH_ASSERT_MSG(Abs(MagnitudeSquared(v) - static_cast<T>(1.0f)) < static_cast<T>(0.0001), ("axis must be of unit length") );

    T s, c;

    SinCos(angle_radians/static_cast<T>(2.0f), s, c);

    QuaternionTemplate<T> r(v.x*s, v.y*s, v.z*s, c);
    return r;
}

template<class T> 
RW_MATH_INLINE void AxisAngleFromQuaternion(const QuaternionTemplate<T>& q,       Vector3Template<T>& axis_out, T& angle_out)
{
    angle_out = ACos(q.w) * static_cast<T>(2.0f);  

    T sinSquared = 1.0f - q.w * q.w;
    if( sinSquared < SMALL_FLOAT )
    {
        // Return an arbitrary axis of unit length
        axis_out.Set(T(1.0f), T(0.0f), T(0.0f));
    }
    else
    {
        // Previously s was calculated using:
        //   s = 1.0f / sin(angle_out/2);
        // Now, the value is computed more directly, and with lower latency.
        // 
        // The following is a trig identity:
        //   1 = cos(angle)^2 + sin(angle)^2
        // We also know that:
        //   angle_out = acos(q.w) * 2
        //   q.w = cos(angle_out/2)
        // So by substitution:
        //   1 = cos(angle_out/2)^2 + sin(angle_out/2)^2
        //   1 = q.w^2 + sin(angle_out/2)^2
        //   1 - q.w^2 = sin(angle_out/2)^2
        //   sin(angle_out/2) = sqrt(1-q.w^2)
        //   s = invsqrt( 1-q.w^2 )
        T s = InvSqrt( sinSquared );
        axis_out.Set(q.x*s, q.y*s, q.z*s );
    }

}

/*!
   This function converts a quaternion to a set of euler angles that represent the same rotation.

   It is important to note that multiple euler angles represent the same rotation (and therefore the
   same quaternion) so, in general:

   \code
        EulerXYZFromQuaternion(QuaternionFromEulerXYZ(eulerAngles)) != eulerAngles
   \endcode
 */
template<class T> 
RW_MATH_INLINE Vector3Template<T> EulerXYZFromQuaternion(const QuaternionTemplate<T>& q)
{
    return EulerXYZFromMatrix33(Matrix33FromQuaternion(q));
}

template<class T> 
RW_MATH_INLINE Vector3Template<T> TransformVector(const Vector3Template<T>& v, const QuaternionTemplate<T>& q)
{
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T yz = q.y * q.z;
    T wx = q.w * q.x;
    T wy = q.w * q.y;
    T wz = q.w * q.z;
    
    const T one(1.0f);
    const T two(2.0f);
    return Vector3Template<T>(
            v.x *(one-two*(yy+zz)) + v.y * two*(xy-wz)     + v.z * two*(xz+wy),
            v.x * two*(xy+wz)    + v.y * (one-two*(xx+zz)) + v.z * two*(yz-wx),
            v.x * two*(xz-wy)    + v.y * two*(yz+wx)     + v.z * (one-two*(xx+yy))
        );

}

template<class T> 
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromMatrix44(const Matrix44Template<T>& m, T epsilon = (T)(0.0f))
{
    Matrix33Template<T> mat33(GetXYZ(m.XAxis()), GetXYZ(m.YAxis()), GetXYZ(m.ZAxis()));

    return QuaternionFromMatrix33( mat33, epsilon );
}

template<class T> 
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromMatrix44Affine(const Matrix44AffineTemplate<T>& m, T epsilon = (T)(0.0f))
{
    Matrix33Template<T> mat33(m.XAxis(), m.YAxis(), m.ZAxis());

    return QuaternionFromMatrix33( mat33, epsilon );
}

template<class T> 
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromMatrix33(const Matrix33Template<T>& m, T epsilon = (T)(0.0f))
{
    RW_MATH_ASSERT_MSG(IsRotation(m, T(0.01f)), ("The matrix provided must be a rotation matrix in order to convert it to a quaternion.") );
    T tr = m.xAxis.x + m.yAxis.y + m.zAxis.z;

    if (tr > epsilon)
    {
        T s = Sqrt(tr + static_cast<T>(1.0f));
        T s2 = static_cast<T>(0.5f) / s;
        return QuaternionTemplate<T>(
                (m.yAxis.z - m.zAxis.y) * s2,
                (m.zAxis.x - m.xAxis.z) * s2,
                (m.xAxis.y - m.yAxis.x) * s2,
                static_cast<T>(0.5f) * s
            );
    }
    else
    {
        if ((m.xAxis.x > m.yAxis.y) && (m.xAxis.x > m.zAxis.z)) 
        {   // m[0][0] is greatest 
            T s = Sqrt((m.xAxis.x - (m.yAxis.y + m.zAxis.z)) + static_cast<T>(1.0f));
            T s2 = static_cast<T>(0.5f) / s;
            return QuaternionTemplate<T>(
                    static_cast<T>(0.5f) * s,
                    (m.yAxis.x + m.xAxis.y) * s2,
                    (m.xAxis.z + m.zAxis.x) * s2,
                    (m.yAxis.z - m.zAxis.y) * s2
                );
        } 
        else if (m.yAxis.y > m.zAxis.z) 
        {   // m[1][1] is greatest 
            T s = Sqrt((m.yAxis.y - (m.zAxis.z + m.xAxis.x)) + static_cast<T>(1.0f));
            T s2 = static_cast<T>(0.5f) / s;
            return QuaternionTemplate<T>(
                    (m.yAxis.x + m.xAxis.y) * s2,
                    static_cast<T>(0.5f) * s,
                    (m.zAxis.y + m.yAxis.z) * s2,
                    (m.zAxis.x - m.xAxis.z) * s2
                );
        } 
        else 
        {   // m[2][2] is greatest 
            T s = Sqrt((m.zAxis.z - (m.xAxis.x + m.yAxis.y)) + static_cast<T>(1.0f));
            T s2 = static_cast<T>(0.5f) / s;
            return QuaternionTemplate<T>(
                    (m.xAxis.z + m.zAxis.x) * s2,
                    (m.zAxis.y + m.yAxis.z) * s2,
                    static_cast<T>(0.5f) * s,
                    (m.xAxis.y - m.yAxis.x) * s2
                );
        } 
    }
}

template<class T> 
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromEulerXYZ(const Vector3Template<T>& euler)
{
    T ci, cj, ch, si, sj, sh;

    T ti = euler.x * static_cast<T>(0.5f);
    T tj = euler.y * static_cast<T>(0.5f);
    T th = euler.z * static_cast<T>(0.5f);

    SinCos(ti, si, ci);
    SinCos(tj, sj, cj);
    SinCos(th, sh, ch);

    T cc = ci * ch;
    T cs = ci * sh;
    T sc = si * ch;
    T ss = si * sh;

    return QuaternionTemplate<T>(
        cj * sc - sj * cs,
        cj * ss + sj * cc,
        cj * cs - sj * sc,
        cj * cc + sj * ss
    );
}

template<class T>
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromLookDirection(const Vector3Template<T>& lookDirection, const Vector3Template<T>& up)
{
    // Note: lookDirection is actually the same as a "look at" position since we use the origin as the eye position.
    Matrix44Template<T> matrix = Matrix44FromLookAt( lookDirection, up, GetVector3_ZeroTemplate<T>() );
    return QuaternionFromMatrix44(matrix);
}

template<class T>
RW_MATH_INLINE QuaternionTemplate<T> QuaternionFromLookAt(const Vector3Template<T>& lookAt, const Vector3Template<T>& up, const Vector3Template<T>& eyePosition)
{
    Matrix44Template<T> matrix = Matrix44FromLookAt( lookAt, up, eyePosition );
    return QuaternionFromMatrix44(matrix);
}

template<class T>
RW_MATH_INLINE Vector3Template<T> QuaternionGetMatrix44AffineRowX(const QuaternionTemplate<T>& quat)
{
    QuaternionTemplate<T> quatTemp = quat * T(SQRT_2);
    T twoXY = quatTemp.x * quatTemp.y;
    T twoXZ = quatTemp.x * quatTemp.z;
    T twoYY = quatTemp.y * quatTemp.y;
    T twoZZ = quatTemp.z * quatTemp.z;
    T twoWY = quatTemp.w * quatTemp.y;
    T twoWZ = quatTemp.w * quatTemp.z;
    return Vector3Template<T>(
       T(1.0f) - twoYY - twoZZ,
                         twoXY + twoWZ,
                         twoXZ - twoWY
           );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> QuaternionGetMatrix44AffineRowY(const QuaternionTemplate<T>& quat)
{
    QuaternionTemplate<T> quatTemp = quat * T(SQRT_2);
    T twoXX = quatTemp.x * quatTemp.x;
    T twoXY = quatTemp.x * quatTemp.y;
    T twoYZ = quatTemp.y * quatTemp.z;
    T twoZZ = quatTemp.z * quatTemp.z;
    T twoWX = quatTemp.w * quatTemp.x;
    T twoWZ = quatTemp.w * quatTemp.z;
    return Vector3Template<T>(
                              twoXY - twoWZ,
            T(1.0f) - twoXX - twoZZ,
                              twoYZ + twoWX
            );
}

template<class T>
RW_MATH_INLINE Vector3Template<T> QuaternionGetMatrix44AffineRowZ(const QuaternionTemplate<T>& quat)
{
    QuaternionTemplate<T> quatTemp = quat * T(SQRT_2);
    T twoXX = quatTemp.x * quatTemp.x;
    T twoXZ = quatTemp.x * quatTemp.z;
    T twoYY = quatTemp.y * quatTemp.y;
    T twoYZ = quatTemp.y * quatTemp.z;
    T twoWX = quatTemp.w * quatTemp.x;
    T twoWY = quatTemp.w * quatTemp.y;
    return Vector3Template<T>(
                                 twoXZ + twoWY,
                                 twoYZ - twoWX,
               T(1.0f) - twoXX - twoYY
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompLessEqual(const QuaternionTemplate<T> &a, const QuaternionTemplate<T> &b)
{
    return Mask4Template<T>(
            b.x-a.x,
            b.y-a.y,
            b.z-a.z,
            b.w-a.w
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompGreaterEqual(const QuaternionTemplate<T> &a, const QuaternionTemplate<T> &b)
{
    return Mask4Template<T>(
            a.x-b.x,
            a.y-b.y,
            a.z-b.z,
            a.w-b.w
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompLessThan(const QuaternionTemplate<T> &a, const QuaternionTemplate<T> &b)
{
    return Not(CompGreaterEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompGreaterThan(const QuaternionTemplate<T> &a, const QuaternionTemplate<T> &b)
{
    return Not(CompLessEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompEqual(const QuaternionTemplate<T> &a, const QuaternionTemplate<T> &b)
{
    return And(CompLessEqual(a, b), CompGreaterEqual(a,b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompNotEqual(const QuaternionTemplate<T> &a, const QuaternionTemplate<T> &b)
{
    return Not(And(CompLessEqual(a, b), CompGreaterEqual(a,b)));
}

template <class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Select(const MaskScalarTemplate<T> &mask, const QuaternionTemplate<T> &trueValue, const QuaternionTemplate<T> &falseValue )
{
    return QuaternionTemplate<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.y, falseValue.y ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.z, falseValue.z ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.w, falseValue.w )
        );
}

template <class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> Select(const Mask4Template<T> &mask, const QuaternionTemplate<T> &trueValue, const QuaternionTemplate<T> &falseValue )
{
    return QuaternionTemplate<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.y, falseValue.y ),
            (T)RW_MATH_FPU_Fsel( mask.mZ, trueValue.z, falseValue.z ),
            (T)RW_MATH_FPU_Fsel( mask.mW, trueValue.w, falseValue.w )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Vector3Template<T> GetXYZ(const QuaternionTemplate<T>& q)
{
    return Vector3Template<T>(q.x, q.y, q.z);
}

template<class T>
RW_MATH_FORCE_INLINE QuaternionTemplate<T> LoadUnAlignedPacked(const QuaternionTemplate<T>& value)
{
    return value;
}

template<class T>
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(Quaternion &destination, const QuaternionTemplate<T>& value)
{
    destination = value;
}

// functions return vector by reference

#if RW_MATH_ENABLE_DEPRECATED_API

template<class T> 
RW_MATH_INLINE void Negate(const QuaternionTemplate<T>& vec,     QuaternionTemplate<T> &r) 
{
    r.Set(-vec.x, -vec.y, -vec.z, -vec.w);
    
}

template<class T> 
RW_MATH_INLINE void Mult(const QuaternionTemplate<T>& vec,     T scalar, QuaternionTemplate<T>& r) 
{
    r.Set(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    
}

template<class T> 
RW_MATH_FORCE_INLINE QuaternionTemplate<T>         MultByVector(const QuaternionTemplate<T>& q,       const Vector4Template<T>& v)
{
    return QuaternionTemplate<T>(q.x*v.x, q.y*v.y, q.z*v.z, q.w*v.w);
}

template<class T> 
RW_MATH_INLINE void Divide(const QuaternionTemplate<T>& vec,     T scalar, QuaternionTemplate<T>& r) 
{
    T scalarInv= static_cast<T>(1.0f)/scalar;
    r.Set(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
    
}

template<class T> 
RW_MATH_INLINE void Add(const QuaternionTemplate<T>& a,       const QuaternionTemplate<T>& b,     QuaternionTemplate<T>& r) 
{
    r.Set(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

template<class T> 
RW_MATH_INLINE void Subtract(const QuaternionTemplate<T>& a,       const QuaternionTemplate<T>& b,     QuaternionTemplate<T>& r) 
{
    r.Set(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}

template<class T> 
RW_MATH_INLINE void Normalize(const QuaternionTemplate<T>& vec,      QuaternionTemplate<T>& r) 
{
    r= Mult(vec, MagnitudeRecip(vec));
}

template<class T> 
RW_MATH_INLINE void Mult(const QuaternionTemplate<T>& a,        const QuaternionTemplate<T>& b,      QuaternionTemplate<T>& r) 
{
    r.Set(a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x, 
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w, 
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z 
        );

}

template<class T> 
RW_MATH_INLINE void Lerp(const QuaternionTemplate<T>& v0,       const QuaternionTemplate<T>& v1,     T t,                            QuaternionTemplate<T>& r) 
{
    r= Lerp(v0, v1, t);
}

template<class T> 
RW_MATH_INLINE void SLerp(const QuaternionTemplate<T>& v0,       const QuaternionTemplate<T>& v1,     T t,                            QuaternionTemplate<T>& r) 
{
    r= SLerp(v0, v1, t);
}

template<class T> 
RW_MATH_INLINE void NLerp(const QuaternionTemplate<T>& v0,       const QuaternionTemplate<T>& v1,     T t,                            QuaternionTemplate<T>& r) 
{
    r= NLerp(v0, v1, t);
}

// additional for Quaternion
template<class T> 
RW_MATH_INLINE void Inverse(const QuaternionTemplate<T>& q, QuaternionTemplate<T>& r) 
{
    r = Inverse(q);
}

template<class T> 
RW_MATH_INLINE void Conj(const QuaternionTemplate<T>& q, QuaternionTemplate<T>& r) 
{
    r = Conj(q);
}

template<class T> 
RW_MATH_INLINE void    QuaternionFromAxisAngle(const Vector3Template<T>& v,    T angle_radians,                     QuaternionTemplate<T>& r) 
{
    r = QuaternionFromAxisRotationAngle( v, angle_radians );
}

template<class T> 
RW_MATH_INLINE void     EulerXYZFromQuaternion(const QuaternionTemplate<T>& q, Vector3Template<T> &r) 
{
    r = EulerXYZFromQuaternion(q);
}

template<class T> 
RW_MATH_INLINE void Vector3Transform(const QuaternionTemplate<T>& q, const Vector3Template<T>& v,  Vector3Template<T>& r) 
{
    r = TransformVector(v, q);
}

template<class T> 
RW_MATH_INLINE void     QuaternionFromMatrix44(const Matrix44Template<T>& m, QuaternionTemplate<T>& q, T epsilon = (T)(0.0f)) 
{
    q = QuaternionFromMatrix44(m, epsilon);
}

template<class T> 
RW_MATH_INLINE void     QuaternionFromMatrix33(const Matrix33Template<T>& m, QuaternionTemplate<T>& q, T epsilon = (T)(0.0f)) 
{
    q = QuaternionFromMatrix33(m, epsilon);
}

template<class T> 
RW_MATH_INLINE void     QuaternionFromEulerXYZ(const Vector3Template<T>& euler, QuaternionTemplate<T>& r) 
{
    r = QuaternionFromEulerXYZ(euler);
}

#endif

#ifdef EA_PLATFORM_REVOLUTION
// These functions provide template specializations that are optimized for
// single precision types on the Wii.
template<> 
QuaternionTemplate<float> Lerp(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t);

template<> 
QuaternionTemplate<float> NormalizeFast(const QuaternionTemplate<float>& vec);

template<> 
QuaternionTemplate<float> NormalizeEstimate(const QuaternionTemplate<float>& vec);

template<> 
QuaternionTemplate<float> Normalize(const QuaternionTemplate<float>& vec);

template<> 
QuaternionTemplate<float> NLerpEstimate(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t);

template<> 
QuaternionTemplate<float> NLerpFast(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t);

template<> 
QuaternionTemplate<float> NLerp(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t);
#endif

} // namespace fpu
} // namespace math
} // namespace rw


#endif
