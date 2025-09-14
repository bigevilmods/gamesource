// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECTOR3_OPERATION_INLINE_H
#define RW_MATH_FPU_VECTOR3_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/trig.h"
#include "rw/math/fpu/scalar.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/mask3_operation.h"
#include "rw/math/detail/static_asserts.h"

namespace rw
{
namespace math
{
namespace fpu
{

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator +=(Vector3Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x+scalar, vec.y+scalar, vec.z+scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator -=(Vector3Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x-scalar, vec.y-scalar, vec.z-scalar);
    return vec;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector3Template<T>& operator *=(Vector3Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x*scalar, vec.y*scalar, vec.z*scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator /=(Vector3Template<T>& vec, const T& scalar)
{
    T scalarInv= static_cast<T>(1.)/scalar;
    vec.Set(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator +=(Vector3Template<T>& a, const Vector3Template<T>& b)
{
    a.Set(a.x+b.x, a.y+b.y, a.z+b.z);
    return a;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator -=(Vector3Template<T>& a, const Vector3Template<T>& b)
{
    a.Set(a.x-b.x, a.y-b.y, a.z-b.z);
    return a;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator *=(Vector3Template<T>& a, const Vector3Template<T>& b)
{
    a.Set(a.x*b.x, a.y*b.y, a.z*b.z);
    return a;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>& operator /=(Vector3Template<T>& a, const Vector3Template<T>& b)
{
    a.Set(a.x/b.x, a.y/b.y, a.z/b.z);
    return a;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator - (const Vector3Template<T>& vec)
{
    Vector3Template<T> r(-vec.x, -vec.y, -vec.z);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector3Template<T>  operator * (const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x*b.x, a.y*b.y, a.z*b.z);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector3Template<T>  operator / (const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x/b.x, a.y/b.y, a.z/b.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator * (const Vector3Template<T>& vec, const T& scalar)
{
    Vector3Template<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator * (const T& scalar, const Vector3Template<T>& vec)
{
    Vector3Template<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator / (const Vector3Template<T>& vec, const T& scalar)
{
    T scalarInv= static_cast<T>(1.)/scalar;
    Vector3Template<T> r(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator + (const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    return Vector3Template<T>(a.x+b.x, a.y+b.y, a.z+b.z);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator + (const Vector3Template<T>& a, const T& s)
{
    Vector3Template<T> r(a.x+s, a.y+s, a.z+s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator + (const T& s, const Vector3Template<T>& a)
{
    Vector3Template<T> r(a.x+s, a.y+s, a.z+s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator - (const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x-b.x, a.y-b.y, a.z-b.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator - (const Vector3Template<T>& a, const T& s)
{
    Vector3Template<T> r(a.x-s, a.y-s, a.z-s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T>  operator - (const T& s, const Vector3Template<T>& a)
{
    Vector3Template<T> r(s-a.x, s-a.y, s-a.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE bool operator ==(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    return (a.x==b.x && a.y==b.y && a.z==b.z);
}

template<class T>
RW_MATH_FORCE_INLINE bool operator !=(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    return (a.x!=b.x || a.y!=b.y || a.z!=b.z);
}

template<class T>
RW_MATH_FORCE_INLINE bool IsSimilar(const Vector3Template<T>& a, const Vector3Template<T>& b, T epsilon = SMALL_FLOAT)
{
    return (Abs(a.x-b.x) <= epsilon && 
            Abs(a.y-b.y) <= epsilon &&
            Abs(a.z-b.z) <= epsilon);
}

template<class T>
RW_MATH_FORCE_INLINE T Magnitude(const Vector3Template<T>& vec)
{
    return Sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeFast(const Vector3Template<T>& vec)
{
    return SqrtFast(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeEstimate(const Vector3Template<T>& vec)
{
    return SqrtEstimate(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeSquared(const Vector3Template<T>& vec)
{
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeRecip(const Vector3Template<T>& vec)
{
    return InvSqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE T Dot(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

// Should we be deprecating this function?  Did we intend to deprecate the other Cross function?

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Cross(const Vector3Template<T>& v0, const Vector3Template<T>& v1) 
{
    Vector3Template<T> r;
    r.x =v0.y*v1.z - v0.z*v1.y;    
    r.y =v0.z*v1.x - v0.x*v1.z;
    r.z =v0.x*v1.y - v0.y*v1.x;
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE T NormalizeReturnMagnitude(const Vector3Template<T>& vec, Vector3Template<T>& r)
{
    T l = MagnitudeSquared(vec);

    if (l > static_cast<T>(0.0f))
    {
        T invl = InvSqrt(l);
        r = Mult(vec, invl);
        return l*invl;
    }

    return static_cast<T>(0.0f);
}

/*!
 This function normalizes vectors that have non-zero magnitude, and gracefully handles vectors with magnitudes that are zero.

 \param[in] vec The input vector to normalize.
 \param[out] r The result vector.  The vector will be the result of normalizing 'vec', or it will be a zero vector if 'vec' has a magnitude of zero (or nearly zero).

 \return Returns the magnitude of the vector
 */
template<class T>
RW_MATH_FORCE_INLINE T CheckedNormalizeReturnMagnitude(const Vector3Template<T>& vec, Vector3Template<T>& r)
{
    T l = MagnitudeSquared(vec);

    if (l > static_cast<T>(0.0f))
    {
        T invl = InvSqrt(l);
        r = Mult(vec, invl);
        return l*invl;
    }
    else
    {
        r.SetZero();        
        return static_cast<T>(0.0f);
    }
}

template<class T> 
RW_MATH_FORCE_INLINE T CosAngleBetweenNormalizedVectors(const Vector3Template<T> &v0, const Vector3Template<T> &v1)
{
    T cosAngle;
    cosAngle = Dot(v0, v1);
    cosAngle = Clamp(cosAngle, static_cast<T>(-1.0f), static_cast<T>(1.0f));
    return cosAngle;
}


// functions return vector by value (default method of return vector)

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Negate(const Vector3Template<T>& vec)
{
    Vector3Template<T> r(-vec.x, -vec.y, -vec.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Mult(const Vector3Template<T>& vec, T scalar)
{
    Vector3Template<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Divide(const Vector3Template<T>& vec, T scalar)
{
    T scalarInv= static_cast<T>(1.)/scalar;
    Vector3Template<T> r(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Add(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x+b.x, a.y+b.y, a.z+b.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Subtract(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x-b.x, a.y-b.y, a.z-b.z);
    return r;
}

// Note: For now the fast version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> NormalizeFast(const Vector3Template<T>& vec)
{
    Vector3Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

// Note: For now the estimate version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> NormalizeEstimate(const Vector3Template<T>& vec)
{
    Vector3Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Normalize(const Vector3Template<T>& vec)
{
    Vector3Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Normalize(const Vector3Template<T>& vec, T &result)
{
    result = Magnitude(vec);
    Vector3Template<T> r(Mult(vec, static_cast<T>(1.0f)/result));
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Mult(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x*b.x, a.y*b.y, a.z*b.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Divide(const Vector3Template<T>& a, const Vector3Template<T>& b)
{
    Vector3Template<T> r(a.x/b.x, a.y/b.y, a.z/b.z);
    return r;
}

template<class T>
RW_MATH_INLINE Vector3Template<T> Reciprocal(const Vector3Template<T>& vec)
{
    T one = static_cast<T>(1.0f);
    Vector3Template<T> r(one/vec.x, one/vec.y, one/vec.z);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> MultAdd(const Vector3Template<T>& a, const Vector3Template<T>& b, const Vector3Template<T>& c)
{
    return Add(Mult(a, b), c);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Lerp(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t)
{
    Vector3Template<T> r(Subtract(v1, v0));
    r = Mult(r, t);
    return Add(v0, r);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> NLerpEstimate(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t)
{
    return NormalizeEstimate(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> NLerpFast(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t)
{
    return NormalizeFast(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> NLerp(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t)
{
    return Normalize(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_INLINE Vector3Template<T> SLerp(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t)
{
    T dp = Dot(v0, v1);

    if (dp > Cos(VECTOR_MIN_SLERP_ANGLE))  // switch to nlerp if angle < MIN_SLERP_ANGLE
    {
        return NLerp(v0, v1, t);
    }

    T theta = ACos(dp);
    Vector3Template<T> a, b;

    a = Mult(v0, Sin((static_cast<T>(1.0f)-t)*theta)/Sin(theta));
    b = Mult(v1, Sin(t*theta)/Sin(theta));
    return Add(a, b);
}

// this function casts from Vector3 to Vector2, use with caution
template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> GetXY(const Vector3Template<T>& v)
{
    return Vector2Template<T>(v.x, v.y);
}

template<class T>
RW_MATH_FORCE_INLINE void Cross(const Vector3Template<T>& v0, const Vector3Template<T>& v1, Vector3Template<T>& r)
{
    r = Vector3Template<T>(
            v0.y*v1.z - v1.y*v0.z,
            v0.z*v1.x - v0.x*v1.z,
            v0.x*v1.y - v0.y*v1.x
        );
}

template<class T>
RW_MATH_FORCE_INLINE bool IsValid(const Vector3Template<T>& v)
{
    return (IsValid(v.x) && IsValid(v.y) && IsValid(v.z));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsZero(const Vector3Template<T>& v, T tolerance = EPSILON)
{
    return (IsZero(v.x, tolerance) && IsZero(v.y, tolerance) && IsZero(v.z, tolerance));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsAnyNaN(const Vector3Template<T>& v)
{
    return (bool)(IsNaN(v.x) | IsNaN(v.y) | IsNaN(v.z));
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Min(const Vector3Template<T>& v0, const Vector3Template<T>& v1)
{
    Vector3Template<T> retVec(Min(v0.x, v1.x), Min(v0.y, v1.y), Min(v0.z, v1.z));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Max(const Vector3Template<T>& v0, const Vector3Template<T>& v1)
{
    Vector3Template<T> retVec(Max(v0.x, v1.x), Max(v0.y, v1.y), Max(v0.z, v1.z));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Clamp(const Vector3Template<T>& vecToClamp, const Vector3Template<T>& minVecValue, const Vector3Template<T>& maxVecValue )
{
    Vector3Template<T> retVec(Clamp(vecToClamp.x, minVecValue.x, maxVecValue.x), Clamp(vecToClamp.y, minVecValue.y, maxVecValue.y), Clamp(vecToClamp.z, minVecValue.z, maxVecValue.z));
    return retVec;
}

/*!
   \brief Clamps each component of 'value' between the corresponding component of -range and range.

   Each component of \c value is clamped between the corresponding components of \c -range and \c range.
   This function is equivalent to \c Clamp( \c value, \c -range, \c range );

   \pre Each component of \c range must be a non-negative value or the result of this function is undefined.
*/
template<class T>
RW_MATH_INLINE Vector3Template<T> ClampToNegPosRange(const Vector3Template<T>& vecToClamp, const Vector3Template<T>& range)
{
    Vector3Template<T> retVec(ClampToNegPosRange(vecToClamp.x, range.x), ClampToNegPosRange(vecToClamp.y, range.y), ClampToNegPosRange(vecToClamp.z, range.z));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Abs(const Vector3Template<T>& v)
{
    Vector3Template<T> retVec(Abs(v.x), Abs(v.y), Abs(v.z));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE T AngleBetweenVectors(const Vector3Template<T>& v0, const Vector3Template<T>& v1)
{
    Vector3Template<T> nv0, nv1;
    nv0 = NormalizeFast(v0);
    nv1 = NormalizeFast(v1);
    T angle = CosAngleBetweenNormalizedVectors(nv0, nv1);
    return ACos(angle);
}

template<class T>
RW_MATH_FORCE_INLINE T AngleBetweenVectors(const Vector3Template<T>& v0, const Vector3Template<T>& v1, const Vector3Template<T> &windAxis)
{
    Vector3Template<T> nv0 = NormalizeFast(v0);
    Vector3Template<T> nv1 = NormalizeFast(v1);
    T angle;
    Vector3Template<T> norm;
    angle = ACos(CosAngleBetweenNormalizedVectors(nv0, nv1));
    norm = Cross(nv0, nv1);
    if(Dot(norm, windAxis) < static_cast<T>(0.0f))
    {
        return(static_cast<T>(TWO_PI) - angle);
    }
    return(angle);
}

template <int PermConstant, class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Permute(const Vector3Template<T> &v0)
{
    (void)rw::math::detail::Assert_ValidVector3PermConstant<PermConstant>();
    return Vector3Template<T>(v0[(PermConstant&0x0f00)>>8],
                              v0[(PermConstant&0x00f0)>>4],
                              v0[(PermConstant&0x000f)>>0]);
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompLessEqual(const Vector3Template<T> &a, const Vector3Template<T> &b)
{
    return Mask3Template<T>(
            b.x-a.x,
            b.y-a.y,
            b.z-a.z
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompGreaterEqual(const Vector3Template<T> &a, const Vector3Template<T> &b)
{
    return Mask3Template<T>(
            a.x-b.x,
            a.y-b.y,
            a.z-b.z
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompLessThan(const Vector3Template<T> &a, const Vector3Template<T> &b)
{
    return Not(CompGreaterEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompGreaterThan(const Vector3Template<T> &a, const Vector3Template<T> &b)
{
    return Not(CompLessEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompEqual(const Vector3Template<T> &a, const Vector3Template<T> &b)
{
    return And(CompLessEqual(a, b), CompGreaterEqual(a,b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompNotEqual(const Vector3Template<T> &a, const Vector3Template<T> &b)
{
    return Not(And(CompLessEqual(a, b), CompGreaterEqual(a,b)));
}

template <class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Select(const MaskScalarTemplate<T> &mask, const Vector3Template<T> &trueValue, const Vector3Template<T> &falseValue )
{
    return Vector3Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.y, falseValue.y ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.z, falseValue.z )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Vector3Template<T> Select(const Mask3Template<T> &mask, const Vector3Template<T> &trueValue, const Vector3Template<T> &falseValue )
{
    return Vector3Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.y, falseValue.y ),
            (T)RW_MATH_FPU_Fsel( mask.mZ, trueValue.z, falseValue.z )
        );
}

template<class T>
RW_MATH_FORCE_INLINE T MaxComponent(const Vector3Template<T>& vec)
{
    return Max(vec.x, vec.y, vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE T MinComponent(const Vector3Template<T>& vec)
{
    return Min(vec.x, vec.y, vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> SqrtFast(const Vector3Template<T>& vec)
{
	T vx = SqrtFast(vec.x);
	T vy = SqrtFast(vec.y);
	T vz = SqrtFast(vec.z);
    Vector3Template<T> r(vx, vy, vz);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> InvSqrtFast(const Vector3Template<T>& vec)
{
	T vx = InvSqrtFast(vec.x);
	T vy = InvSqrtFast(vec.y);
	T vz = InvSqrtFast(vec.z);
    Vector3Template<T> r(vx, vy, vz);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> ReciprocalFast(const Vector3Template<T>& vec)
{
	T vx = ReciprocalFast(vec.x);
	T vy = ReciprocalFast(vec.y);
	T vz = ReciprocalFast(vec.z);
    Vector3Template<T> r(vx, vy, vz);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> RotateLeft(const Vector3Template<T>& vec)
{
	return Vector3Template<T>(vec.y, vec.z, vec.x);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> RotateRight(const Vector3Template<T>& vec)
{
	return Vector3Template<T>(vec.z, vec.x, vec.y);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Template<T> LoadUnAlignedPacked(const Vector3Template<T>& value)
{
    return value;
}

template<class T>
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(Vector3 &destination, const Vector3Template<T>& value)
{
    destination = value;
}

// functions return vector by reference

#if RW_MATH_ENABLE_DEPRECATED_API


template<class T>
RW_MATH_FORCE_INLINE void Negate(const Vector3Template<T>& vec, Vector3Template<T> &r) 
{
    r.Set(-vec.x, -vec.y, -vec.z);
}


template<class T>
RW_MATH_FORCE_INLINE void Mult(const Vector3Template<T>& vec, T scalar, Vector3Template<T>& r) 
{
    r.Set(vec.x*scalar, vec.y*scalar, vec.z*scalar);
}


template<class T>
RW_MATH_FORCE_INLINE void Divide(const Vector3Template<T>& vec, T scalar, Vector3Template<T>& r) 
{
    r.Set(vec.x/scalar, vec.y/scalar, vec.z/scalar);
}


template<class T>
RW_MATH_FORCE_INLINE void Add(const Vector3Template<T>& a, const Vector3Template<T>& b, Vector3Template<T>& r) 
{
    r.Set(a.x+b.x, a.y+b.y, a.z+b.z);
}


template<class T>
RW_MATH_FORCE_INLINE void Subtract(const Vector3Template<T>& a, const Vector3Template<T>& b, Vector3Template<T>& r) 
{
    r.Set(a.x-b.x, a.y-b.y, a.z-b.z);
}


template<class T>
RW_MATH_FORCE_INLINE void Normalize(const Vector3Template<T>& vec, Vector3Template<T>& r) 
{
    Mult(vec, MagnitudeRecip(vec), r);
}


template<class T>
RW_MATH_FORCE_INLINE void Mult(const Vector3Template<T>& a, const Vector3Template<T>& b, Vector3Template<T>& r) 
{
    r.Set(a.x*b.x, a.y*b.y, a.z*b.z);
}


template<class T>
RW_MATH_FORCE_INLINE void Lerp(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t, Vector3Template<T>& r) 
{
    Subtract(v1, v0, r);
    Mult(r, t,r );
    Add(v0, r,r );
}


template<class T>
RW_MATH_FORCE_INLINE void NLerp(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t, Vector3Template<T>& r) 
{
    Subtract(v1, v0, r);
    Mult(r, t, r);
    Add(v0, r, r);
    Normalize(r, r);
}


template<class T>
RW_MATH_INLINE void SLerp(const Vector3Template<T>& v0, const Vector3Template<T>& v1, T t, Vector3Template<T>& r) 
{
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v0)-static_cast<T>(1.0f)) < static_cast<T>(0.01f), ("v0 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v1)-static_cast<T>(1.0f)) < static_cast<T>(0.01f), ("v1 must be of unit length") );
    RW_MATH_ASSERT_MSG(t<= static_cast<T>(1.0f) && t>= static_cast<T>(0.0f), ("t out of bounds; need 1>=t>=0") );

    T dp = Dot(v0, v1);

    if (dp > Cos(VECTOR_MIN_SLERP_ANGLE))  // switch to nlerp if angle < MIN_SLERP_ANGLE
    {
        NLerp(v0, v1, t, r);
    }
    else
    {
        T theta = ACos(dp);
        Vector3Template<T> a, b;

        Mult(v0, Sin((static_cast<T>(1.0f)-t)*theta)/Sin(theta), a);
        Mult(v1, Sin(t*theta)/Sin(theta), b);
        Add(a, b, r);
    }
}

#endif

} // namespace fpu
} // namespace math
} // namespace rw

#endif
