// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECTOR2_OPERATION_INLINE_H
#define RW_MATH_FPU_VECTOR2_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/trig.h"
#include "rw/math/fpu/scalar.h"
#include "rw/math/fpu/vector2.h"
#include "rw/math/fpu/mask2_operation.h"
#include "rw/math/detail/static_asserts.h"

namespace rw
{
namespace math
{
namespace fpu
{

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>& operator +=(Vector2Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x+scalar, vec.y+scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>& operator -=(Vector2Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x-scalar, vec.y-scalar);
    return vec;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>& operator *=(Vector2Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x*scalar, vec.y*scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>& operator /=(Vector2Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x/scalar, vec.y/scalar);
    return vec;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>& operator +=(Vector2Template<T>& a, const Vector2Template<T>& b)
{
    a.Set(a.x+b.x, a.y+b.y);
    return a;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>& operator -=(Vector2Template<T>& a, const Vector2Template<T>& b)
{
    a.Set(a.x-b.x, a.y-b.y);
    return a;
}


template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>& operator *=(Vector2Template<T>& a, const Vector2Template<T>& b)
{
    a.Set(a.x*b.x, a.y*b.y);
    return a;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>& operator /=(Vector2Template<T>& a, const Vector2Template<T>& b)
{
    a.Set(a.x/b.x, a.y/b.y);
    return a;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator - (const Vector2Template<T>& vec)
{
    Vector2Template<T> r(-vec.x, -vec.y);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator * (const Vector2Template<T>& vec,   const T& scalar)
{
    Vector2Template<T> r(vec.x*scalar, vec.y*scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator * (const T& scalar, const Vector2Template<T>& vec)
{
    Vector2Template<T> r(vec.x*scalar, vec.y*scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator * (const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x*b.x, a.y*b.y);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator / (const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x/b.x, a.y/b.y);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator / (const Vector2Template<T>& vec, const T& scalar)
{
    Vector2Template<T> r(vec.x/scalar, vec.y/scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator + (const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x+b.x, a.y+b.y);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>  operator + (const Vector2Template<T>& a, const T& s)
{
    Vector2Template<T> r(a.x+s, a.y+s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>  operator + (const T& s, const Vector2Template<T>& a)
{
    Vector2Template<T> r(a.x+s, a.y+s);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T>  operator - (const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x-b.x, a.y-b.y);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>  operator - (const Vector2Template<T>& a, const T& s)
{
    Vector2Template<T> r(a.x-s, a.y-s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T>  operator - (const T& s, const Vector2Template<T>& a)
{
    Vector2Template<T> r(s-a.x, s-a.y);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE bool operator ==(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    return (a.x==b.x && a.y==b.y);
}

template<class T> 
RW_MATH_FORCE_INLINE bool operator !=(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    return (a.x!=b.x || a.y!=b.y);
}

template<class T> 
RW_MATH_FORCE_INLINE bool IsSimilar(const Vector2Template<T>& a, const Vector2Template<T>& b, T epsilon = SMALL_FLOAT)
{
    return (Abs(a.x-b.x) <= epsilon && Abs(a.y-b.y) <= epsilon);
}


template<class T> 
RW_MATH_FORCE_INLINE T Magnitude(const Vector2Template<T>& vec)
{
    return Sqrt(vec.x*vec.x+vec.y*vec.y);
}

template<class T> 
RW_MATH_FORCE_INLINE T MagnitudeFast(const Vector2Template<T>& vec)
{
    return SqrtFast(vec.x*vec.x+vec.y*vec.y);
}

template<class T> 
RW_MATH_FORCE_INLINE T MagnitudeEstimate(const Vector2Template<T>& vec)
{
    return SqrtEstimate(vec.x*vec.x+vec.y*vec.y);
}

template<class T> 
RW_MATH_FORCE_INLINE T MagnitudeSquared(const Vector2Template<T>& vec)
{
    return vec.x*vec.x+vec.y*vec.y;
}

template<class T> 
RW_MATH_FORCE_INLINE T MagnitudeRecip(const Vector2Template<T>& vec)
{
    return InvSqrt(vec.x*vec.x+vec.y*vec.y);
}

template<class T> 
RW_MATH_FORCE_INLINE T Dot(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
     return a.x*b.x + a.y*b.y;
}

template<class T> 
RW_MATH_FORCE_INLINE T Cross(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
     return a.x*b.y - a.y*b.x;
}

template<class T>
RW_MATH_INLINE T NormalizeReturnMagnitude(const Vector2Template<T>& vec, Vector2Template<T>& r)
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
RW_MATH_INLINE T CheckedNormalizeReturnMagnitude(const Vector2Template<T>& vec, Vector2Template<T>& r)
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
RW_MATH_FORCE_INLINE T CosAngleBetweenNormalizedVectors(const Vector2Template<T> &v0, const Vector2Template<T> &v1)
{
    T cosAngle;
    cosAngle = Dot(v0, v1);
    cosAngle = Clamp(cosAngle, static_cast<T>(-1.0f), static_cast<T>(1.0f));
    return cosAngle;
}

// functions return vector by value (default method of return vector)

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Negate(const Vector2Template<T>& vec)
{
    Vector2Template<T> r(-vec.x, -vec.y);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Mult(const Vector2Template<T>& vec, T scalar)
{
    Vector2Template<T> r(vec.x*scalar, vec.y*scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Divide(const Vector2Template<T>& vec, T scalar)
{
    Vector2Template<T> r(vec.x/scalar, vec.y/scalar);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Add(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x+b.x, a.y+b.y);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Subtract(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x-b.x, a.y-b.y);
    return r;
}

// Note: For now the fast version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
// fof fadf  helo 
template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> NormalizeFast(const Vector2Template<T>& vec)
{
    Vector2Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

// Note: For now the estimate version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> NormalizeEstimate(const Vector2Template<T>& vec)
{
    Vector2Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Normalize(const Vector2Template<T>& vec)
{
    Vector2Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> Normalize(const Vector2Template<T>& vec, T& magnitude_out)
{
    T l = MagnitudeRecip(vec);
    magnitude_out = Reciprocal(l);
    return Mult(vec, l);
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Mult(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x*b.x, a.y*b.y);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Divide(const Vector2Template<T>& a, const Vector2Template<T>& b)
{
    Vector2Template<T> r(a.x/b.x, a.y/b.y);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Reciprocal(const Vector2Template<T>& vec)
{
    T one = static_cast<T>(1.0f);
    Vector2Template<T> r(one/vec.x, one/vec.y);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> MultAdd(const Vector2Template<T>& a, const Vector2Template<T>& b, const Vector2Template<T>& c)
{
    return Add(Mult(a, b), c);
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Lerp(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t)
{
    Vector2Template<T> r(Subtract(v1, v0));
    r = Mult(r, t);
    return Add(v0, r);
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> NLerpEstimate(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t)
{
    return NormalizeEstimate(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> NLerpFast(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t)
{
    return NormalizeFast(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> NLerp(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t)
{
    return Normalize(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_INLINE Vector2Template<T> SLerp(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t)
{
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v0)-static_cast<T>(1.0f)) < static_cast<T>(0.01f), ("v0 must be of unit length") );
    RW_MATH_ASSERT_MSG(Abs(Magnitude(v1)-static_cast<T>(1.0f)) < static_cast<T>(0.01f), ("v1 must be of unit length") );
    RW_MATH_ASSERT_MSG(t<= static_cast<T>(1.0f) && t>= static_cast<T>(0.0f), ("t out of bounds; need 1>=t>=0") );

    T dp = Dot(v0, v1);

    if (dp > Cos(VECTOR_MIN_SLERP_ANGLE))  // switch to nlerp if angle < MIN_SLERP_ANGLE
    {
        return NLerp(v0, v1, t);
    }
    
    T theta = ACos(dp);
    Vector2Template<T> a, b;

    a = Mult(v0, Sin((static_cast<T>(1.0f)-t)*theta)/Sin(theta));
    b = Mult(v1, Sin(t*theta)/Sin(theta));
    return Add(a, b);
}

template<class T>
RW_MATH_FORCE_INLINE bool IsValid(const Vector2Template<T>& v)
{
    return (IsValid(v.x) && IsValid(v.y));
}


template<class T>
RW_MATH_FORCE_INLINE bool IsZero(const Vector2Template<T>& v, T tolerance = EPSILON)
{
    return (IsZero(v.x, tolerance) && IsZero(v.y, tolerance));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsAnyNaN(const Vector2Template<T>& v)
{
    return (bool)(IsNaN(v.x) | IsNaN(v.y));
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Min(const Vector2Template<T>& v0, const Vector2Template<T>& v1)
{
    Vector2Template<T> retVec(Min(v0.x, v1.x), Min(v0.y, v1.y));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Max(const Vector2Template<T>& v0, const Vector2Template<T>& v1)
{
    Vector2Template<T> retVec(Max(v0.x, v1.x), Max(v0.y, v1.y));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Clamp(const Vector2Template<T>& vecToClamp, const Vector2Template<T>& minVecValue, const Vector2Template<T>& maxVecValue )
{
    Vector2Template<T> retVec(Clamp(vecToClamp.x, minVecValue.x, maxVecValue.x), Clamp(vecToClamp.y, minVecValue.y, maxVecValue.y));
    return retVec;
}

/*!
   \brief Clamps each component of 'value' between the corresponding component of -range and range.

   Each component of \c value is clamped between the corresponding components of \c -range and \c range.
   This function is equivalent to \c Clamp( \c value, \c -range, \c range );

   \pre Each component of \c range must be a non-negative value or the result of this function is undefined.
*/
template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> ClampToNegPosRange(const Vector2Template<T>& vecToClamp, const Vector2Template<T>& range)
{
    Vector2Template<T> retVec(ClampToNegPosRange(vecToClamp.x, range.x), ClampToNegPosRange(vecToClamp.y, range.y));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Abs(const Vector2Template<T>& v)
{
    Vector2Template<T> retVec(Abs(v.x), Abs(v.y));
    return retVec;
}

template<class T>
RW_MATH_INLINE T AngleBetweenVectors(const Vector2Template<T>& v0, const Vector2Template<T>& v1)
{
    Vector2Template<T> nv0, nv1;
    nv0 = Normalize(v0);
    nv1 = Normalize(v1);
    T angle = CosAngleBetweenNormalizedVectors(nv0, nv1);
    return ACos(angle);
}

template<class T>
RW_MATH_INLINE T AngleBetweenVectorsFast(const Vector2Template<T>& v0, const Vector2Template<T>& v1)
{
    Vector2Template<T> nv0, nv1;
    nv0 = NormalizeFast(v0);
    nv1 = NormalizeFast(v1);
    T angle = CosAngleBetweenNormalizedVectors(nv0, nv1);
    return ACos(angle);
}

template <int PermConstant, class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Permute(const Vector2Template<T> &v0)
{
    (void)rw::math::detail::Assert_ValidVector2PermConstant<PermConstant>();
    return Vector2Template<T>(v0[(PermConstant&0x00f0)>>4],
                              v0[(PermConstant&0x000f)>>0]);
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompLessEqual(const Vector2Template<T> &a, const Vector2Template<T> &b)
{
    return Mask2Template<T>(
            b.x-a.x,
            b.y-a.y
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompGreaterEqual(const Vector2Template<T> &a, const Vector2Template<T> &b)
{
    return Mask2Template<T>(
            a.x-b.x,
            a.y-b.y
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompLessThan(const Vector2Template<T> &a, const Vector2Template<T> &b)
{
    return Not(CompGreaterEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompGreaterThan(const Vector2Template<T> &a, const Vector2Template<T> &b)
{
    return Not(CompLessEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompEqual(const Vector2Template<T> &a, const Vector2Template<T> &b)
{
    return And(CompLessEqual(a, b), CompGreaterEqual(a,b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompNotEqual(const Vector2Template<T> &a, const Vector2Template<T> &b)
{
    return Not(And(CompLessEqual(a, b), CompGreaterEqual(a,b)));
}

template <class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Select(const MaskScalarTemplate<T> &mask, const Vector2Template<T> &trueValue, const Vector2Template<T> &falseValue )
{
    return Vector2Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.y, falseValue.y )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Vector2Template<T> Select(const Mask2Template<T> &mask, const Vector2Template<T> &trueValue, const Vector2Template<T> &falseValue )
{
    return Vector2Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.y, falseValue.y )
        );
}

template<class T> 
RW_MATH_FORCE_INLINE T MaxComponent(const Vector2Template<T>& vec)
{
	return Max(vec.x, vec.y);
}

template<class T> 
RW_MATH_FORCE_INLINE T MinComponent(const Vector2Template<T>& vec)
{
    return Min(vec.x, vec.y);
}

template<class T> 
RW_MATH_FORCE_INLINE Vector2Template<T> SqrtFast(const Vector2Template<T>& vec)
{
	T vx = SqrtFast(vec.x);
	T vy = SqrtFast(vec.y);
    Vector2Template<T> r(vx, vy);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> InvSqrtFast(const Vector2Template<T>& vec)
{
	T vx = InvSqrtFast(vec.x);
	T vy = InvSqrtFast(vec.y);
    Vector2Template<T> r(vx, vy);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> ReciprocalFast(const Vector2Template<T>& vec)
{
	T vx = ReciprocalFast(vec.x);
	T vy = ReciprocalFast(vec.y);
    Vector2Template<T> r(vx, vy);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> RotateLeft(const Vector2Template<T>& vec)
{
	return Vector2Template<T>(vec.y, vec.x);
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> RotateRight(const Vector2Template<T>& vec)
{
	return Vector2Template<T>(vec.y, vec.x);
}

template<class T>
RW_MATH_FORCE_INLINE Vector2Template<T> LoadUnAlignedPacked(const Vector2Template<T>& value)
{
    return value;
}

template<class T>
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(Vector2 &destination, const Vector2Template<T>& value)
{
    destination = value;
}

// functions return vector by reference

#if RW_MATH_ENABLE_DEPRECATED_API


template<class T>
RW_MATH_INLINE void Negate(const Vector2Template<T>& vec, Vector2Template<T>& r) 
{
    r.Set(-vec.x, -vec.y);
}


template<class T>
RW_MATH_INLINE void Mult(const Vector2Template<T>& vec, T scalar, Vector2Template<T>& r) 
{
    r.Set(vec.x*scalar, vec.y*scalar);
}


template<class T> 
RW_MATH_INLINE void Divide(const Vector2Template<T>& vec, T scalar, Vector2Template<T>& r) 
{
    r.Set(vec.x/scalar, vec.y/scalar);   
}


template<class T> 
RW_MATH_INLINE void Add(const Vector2Template<T>& a, const Vector2Template<T>& b, Vector2Template<T>& r) 
{
    r.Set(a.x+b.x, a.y+b.y);
}


template<class T>
RW_MATH_INLINE void Subtract(const Vector2Template<T>& a, const Vector2Template<T>& b, Vector2Template<T>& r) 
{
    r.Set(a.x-b.x, a.y-b.y);
}



template<class T> 
RW_MATH_INLINE void Normalize(const Vector2Template<T>& vec, Vector2Template<T>& r) 
{
    Mult(vec, MagnitudeRecip(vec), r);
}


template<class T> 
RW_MATH_INLINE void Mult(const Vector2Template<T>& a, const Vector2Template<T>& b, Vector2Template<T>& r) 
{
    r.Set(a.x*b.x, a.y*b.y);
}


template<class T>
RW_MATH_INLINE void Lerp(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t, Vector2Template<T>& r) 
{
    Subtract(v1, v0, r);
    Mult(r, t, r);
    Add(v0, r, r);
}


template<class T> 
RW_MATH_INLINE void NLerp(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t, Vector2Template<T>& r) 
{
    Subtract(v1, v0, r);
    Mult(r, t, r);
    Add(v0, r, r);
    Normalize(r, r);
}


template<class T>
RW_MATH_INLINE void SLerp(const Vector2Template<T>& v0, const Vector2Template<T>& v1, T t, Vector2Template<T>& r) 
{
    T dp = Dot(v0, v1);

    if (dp > Cos(VECTOR_MIN_SLERP_ANGLE))  // switch to nlerp if angle < VECTOR_MIN_SLERP_ANGLE
    {
        NLerp(v0, v1, t, r);
    }
    else
    {
        T theta = ACos(dp);
        Vector2Template<T> a, b;

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
