// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECTOR4_OPERATION_INLINE_H
#define RW_MATH_FPU_VECTOR4_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/trig.h"
#include "rw/math/fpu/scalar.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/mask4_operation.h"
#include "rw/math/detail/static_asserts.h"

namespace rw
{
namespace math
{
namespace fpu
{

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>& operator +=(Vector4Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x+scalar, vec.y+scalar, vec.z+scalar, vec.w+scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>& operator -=(Vector4Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x-scalar, vec.y-scalar, vec.z-scalar, vec.w-scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>& operator *=(Vector4Template<T>& vec, const T& scalar)
{
    vec.Set(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>& operator /=(Vector4Template<T>& vec, const T& scalar)
{
    T scalarInv= static_cast<T>(1.)/scalar;
    vec.Set(vec.x *scalarInv, vec.y *scalarInv, vec.z *scalarInv, vec.w *scalarInv);
    return vec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>& operator +=(Vector4Template<T>& a, const Vector4Template<T>& b)
{
    a.Set(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
    return a;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector4Template<T>& operator -=(Vector4Template<T>& a, const Vector4Template<T>& b)
{
    a.Set(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
    return a;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector4Template<T>& operator *=(Vector4Template<T>& a, const Vector4Template<T>& b)
{
    a.Set(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
    return a;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector4Template<T>& operator /=(Vector4Template<T>& a, const Vector4Template<T>& b)
{
    a.Set(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
    return a;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator - (const Vector4Template<T>& vec)
{
    Vector4Template<T> r(-vec.x, -vec.y, -vec.z, -vec.w);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector4Template<T>  operator * (const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
    return r;
}

template<class T> 
RW_MATH_FORCE_INLINE Vector4Template<T>  operator / (const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator * (const Vector4Template<T>& vec, const T& scalar)
{
    Vector4Template<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator * (const T& scalar, const Vector4Template<T>& vec)
{
    Vector4Template<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator / (const Vector4Template<T>& vec, const T& scalar)
{
    T scalarInv= static_cast<T>(1.)/scalar;
    Vector4Template<T> r(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator + (const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator + (const Vector4Template<T>& a, const T& s)
{
    Vector4Template<T> r(a.x+s, a.y+s, a.z+s, a.w+s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator + (const T& s, const Vector4Template<T>& a)
{
    Vector4Template<T> r(a.x+s, a.y+s, a.z+s, a.w+s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator - (const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator - (const Vector4Template<T>& a, const T& s)
{
    Vector4Template<T> r(a.x-s, a.y-s, a.z-s, a.w-s);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T>  operator - (const T& s, const Vector4Template<T>& a)
{
    Vector4Template<T> r(s-a.x, s-a.y, s-a.z, s-a.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE bool operator ==(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}

template<class T>
RW_MATH_FORCE_INLINE bool operator !=(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    return (a.x!=b.x || a.y!=b.y || a.z!=b.z || a.w!=b.w);
}

template<class T>
RW_MATH_FORCE_INLINE bool IsSimilar(const Vector4Template<T>& a, const Vector4Template<T>& b, T epsilon = SMALL_FLOAT)
{
    return (Abs(a.x-b.x) <= epsilon && 
            Abs(a.y-b.y) <= epsilon &&
            Abs(a.z-b.z) <= epsilon &&
            Abs(a.w-b.w) <= epsilon);
}

template<class T>
RW_MATH_FORCE_INLINE T Magnitude(const Vector4Template<T>& vec)
{
    return Sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeFast(const Vector4Template<T>& vec)
{
    return SqrtFast(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeEstimate(const Vector4Template<T>& vec)
{
    return SqrtEstimate(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeSquared(const Vector4Template<T>& vec)
{
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w;
}

template<class T>
RW_MATH_FORCE_INLINE T MagnitudeRecip(const Vector4Template<T>& vec)
{
    return InvSqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
}

template<class T>
RW_MATH_FORCE_INLINE T Dot(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

template<class T>
RW_MATH_INLINE T NormalizeReturnMagnitude(const Vector4Template<T>& vec, Vector4Template<T>& r)
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
RW_MATH_INLINE T CheckedNormalizeReturnMagnitude(const Vector4Template<T>& vec, Vector4Template<T>& r)
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

// functions return vector by value (default method of return vector)

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Negate(const Vector4Template<T>& vec)
{
    Vector4Template<T> r(-vec.x, -vec.y, -vec.z, -vec.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Mult(const Vector4Template<T>& vec, T scalar)
{
    Vector4Template<T> r(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Divide(const Vector4Template<T>& vec, T scalar)
{
    T scalarInv= static_cast<T>(1.)/scalar;
    Vector4Template<T> r(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Add(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Subtract(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
    return r;
}

// Note: For now the fast version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> NormalizeFast(const Vector4Template<T>& vec)
{
    Vector4Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

// Note: For now the estimate version of Normalize is the same as Normalize.  But, it could
// be optimized in the future for some platforms.
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> NormalizeEstimate(const Vector4Template<T>& vec)
{
    Vector4Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Normalize(const Vector4Template<T>& vec)
{
    Vector4Template<T> r(Mult(vec, MagnitudeRecip(vec)));
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Mult(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Divide(const Vector4Template<T>& a, const Vector4Template<T>& b)
{
    Vector4Template<T> r(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Reciprocal(const Vector4Template<T>& vec)
{
    T one = static_cast<T>(1.0f);
    Vector4Template<T> r(one/vec.x, one/vec.y, one/vec.z, one/vec.w);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> MultAdd(const Vector4Template<T>& a, const Vector4Template<T>& b, const Vector4Template<T>& c)
{
    return Add(Mult(a, b), c);
}

//! This function returns a Vector4 formed by dividing each component of the input vector by the vector's W component.
//! 
//! \pre The w component should be non-zero.  Passing a value that is zero (or extremely close to zero) will give an undefined result that is implementation dependent.
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> DivideByW(const Vector4Template<T>& v)
{
    T factor= static_cast<T>(1.)/v.w;
    Vector4Template<T> ret;
    ret.Set(v.x*factor, v.y*factor, v.z*factor, v.w*factor);
    return ret;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Lerp(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t)
{
    Vector4Template<T> r(Subtract(v1, v0));
    r = Mult(r, t);
    return Add(v0, r);
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> NLerpEstimate(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t)
{
    return NormalizeEstimate(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> NLerpFast(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t)
{
    return NormalizeFast(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> NLerp(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t)
{
    return Normalize(Lerp(v0, v1, t));
}

template<class T>
RW_MATH_INLINE Vector4Template<T> SLerp(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t)
{
    T dp = Dot(v0, v1);

    if (dp > Cos(VECTOR_MIN_SLERP_ANGLE))  // switch to nlerp if angle < MIN_SLERP_ANGLE
    {
        return NLerp(v0, v1, t);
    }
    
    T theta = ACos(dp);
    Vector4Template<T> a, b;

    a = Mult(v0, Sin((static_cast<T>(1.0f)-t)*theta)/Sin(theta));
    b = Mult(v1, Sin(t*theta)/Sin(theta));
    return Add(a, b);
}

template<class T>
RW_MATH_FORCE_INLINE void SetXYZ(Vector4Template<T>& v, T x, T y, T z)
{
    v.x = x;
    v.y = y;
    v.z = z;
}

// this function casts from Vector4 to Vector3, use with caution
template<class T>
RW_MATH_INLINE Vector3Template<T> GetXYZ(const Vector4Template<T>& v)
{
    return Vector3Template<T>(v.x, v.y, v.z);
}

// this function casts from Vector4 to Vector2, use with caution
template<class T>
RW_MATH_INLINE Vector2Template<T> GetXY(const Vector4Template<T>& v)
{
    return Vector2Template<T>(v.x, v.y);
}


template<class T>
RW_MATH_FORCE_INLINE T MagnitudeXYZ(const Vector4Template<T>& v) 
{
    return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}


template<class T>
RW_MATH_FORCE_INLINE T MagnitudeSquaredXYZ(const Vector4Template<T>& v) 
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}


template<class T>
RW_MATH_FORCE_INLINE T MagnitudeRecipXYZ(const Vector4Template<T>& v) 
{
    return static_cast<T>(1.0f)/Sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}


template<class T>
RW_MATH_FORCE_INLINE T DotXYZ(const Vector4Template<T>& v0, const Vector4Template<T>& v1) 
{
    return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;
}

// Should we be deprecating this function?  Did we intend to deprecate the other Cross function?

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> CrossXYZ(const Vector4Template<T>& v0, const Vector4Template<T>& v1) 
{
    Vector4Template<T> r;
    r.x =v0.y*v1.z - v1.y*v0.z;    
    r.y =v0.z*v1.x - v0.x*v1.z;
    r.z =v0.x*v1.y - v0.y*v1.x;
    r.w =static_cast<T>(0.0f);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE bool IsValid(const Vector4Template<T>& v)
{
    return (IsValid(v.x) && IsValid(v.y) && IsValid(v.z) && IsValid(v.w));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsZero(const Vector4Template<T>& v, T tolerance = EPSILON)
{
    return (IsZero(v.x, tolerance) && IsZero(v.y, tolerance) && IsZero(v.z, tolerance) && IsZero(v.w, tolerance));
}

template<class T>
RW_MATH_FORCE_INLINE bool IsAnyNaN(const Vector4Template<T>& v)
{
    return (bool)(IsNaN(v.x) | IsNaN(v.y) | IsNaN(v.z) | IsNaN(v.w));
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Min(const Vector4Template<T>& v0, const Vector4Template<T>& v1)
{
    return Vector4Template<T>(Min(v0.x, v1.x), Min(v0.y, v1.y), Min(v0.z, v1.z), Min(v0.w, v1.w));
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Max(const Vector4Template<T>& v0, const Vector4Template<T>& v1)
{
    return Vector4Template<T>(Max(v0.x, v1.x), Max(v0.y, v1.y), Max(v0.z, v1.z), Max(v0.w, v1.w));
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Clamp(const Vector4Template<T>& vecToClamp, const Vector4Template<T>& minVecValue, const Vector4Template<T>& maxVecValue )
{
    Vector4Template<T> retVec(Clamp(vecToClamp.x, minVecValue.x, maxVecValue.x), Clamp(vecToClamp.y, minVecValue.y, maxVecValue.y), Clamp(vecToClamp.z, minVecValue.z, maxVecValue.z), Clamp(vecToClamp.w, minVecValue.w, maxVecValue.w));
    return retVec;
}

/*!
   \brief Clamps each component of 'value' between the corresponding component of -range and range.

   Each component of \c value is clamped between the corresponding components of \c -range and \c range.
   This function is equivalent to \c Clamp( \c value, \c -range, \c range );

   \pre Each component of \c range must be a non-negative value or the result of this function is undefined.
*/
template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> ClampToNegPosRange(const Vector4Template<T>& vecToClamp, const Vector4Template<T>& range)
{
    Vector4Template<T> retVec(ClampToNegPosRange(vecToClamp.x, range.x), ClampToNegPosRange(vecToClamp.y, range.y), ClampToNegPosRange(vecToClamp.z, range.z), ClampToNegPosRange(vecToClamp.w, range.w));
    return retVec;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Abs(const Vector4Template<T>& v)
{
    return Vector4Template<T>(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));
}

template <int PermConstant, class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Permute(const Vector4Template<T> &v0)
{
    (void)rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
    return Vector4Template<T>(v0[(PermConstant&0xf000)>>12],
                              v0[(PermConstant&0x0f00)>> 8],
                              v0[(PermConstant&0x00f0)>> 4],
                              v0[(PermConstant&0x000f)>> 0]);
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompLessEqual(const Vector4Template<T> &a, const Vector4Template<T> &b)
{
    return Mask4Template<T>(
            b.x-a.x,
            b.y-a.y,
            b.z-a.z,
            b.w-a.w
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompGreaterEqual(const Vector4Template<T> &a, const Vector4Template<T> &b)
{
    return Mask4Template<T>(
            a.x-b.x,
            a.y-b.y,
            a.z-b.z,
            a.w-b.w
            );
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompLessThan(const Vector4Template<T> &a, const Vector4Template<T> &b)
{
    return Not(CompGreaterEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompGreaterThan(const Vector4Template<T> &a, const Vector4Template<T> &b)
{
    return Not(CompLessEqual(a, b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompEqual(const Vector4Template<T> &a, const Vector4Template<T> &b)
{
    return And(CompLessEqual(a, b), CompGreaterEqual(a,b));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompNotEqual(const Vector4Template<T> &a, const Vector4Template<T> &b)
{
    return Not(And(CompLessEqual(a, b), CompGreaterEqual(a,b)));
}

template <class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Select(const MaskScalarTemplate<T> &mask, const Vector4Template<T> &trueValue, const Vector4Template<T> &falseValue )
{
    return Vector4Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.y, falseValue.y ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.z, falseValue.z ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.w, falseValue.w )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Vector4Template<T> Select(const Mask4Template<T> &mask, const Vector4Template<T> &trueValue, const Vector4Template<T> &falseValue )
{
    return Vector4Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.x, falseValue.x ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.y, falseValue.y ),
            (T)RW_MATH_FPU_Fsel( mask.mZ, trueValue.z, falseValue.z ),
            (T)RW_MATH_FPU_Fsel( mask.mW, trueValue.w, falseValue.w )
        );
}

template<class T>
RW_MATH_FORCE_INLINE T MaxComponent(const Vector4Template<T>& vec)
{
    return Max(Max(vec.x, vec.y, vec.z), vec.w);
}

template<class T>
RW_MATH_FORCE_INLINE T MinComponent(const Vector4Template<T>& vec)
{
    return Min(Min(vec.x, vec.y, vec.z), vec.w);
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> SqrtFast(const Vector4Template<T>& vec)
{
	T vx = SqrtFast(vec.x);
	T vy = SqrtFast(vec.y);
	T vz = SqrtFast(vec.z);
	T vw = SqrtFast(vec.w);
    Vector4Template<T> r(vx, vy, vz, vw);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> InvSqrtFast(const Vector4Template<T>& vec)
{
	T vx = InvSqrtFast(vec.x);
	T vy = InvSqrtFast(vec.y);
	T vz = InvSqrtFast(vec.z);
	T vw = InvSqrtFast(vec.w);
    Vector4Template<T> r(vx, vy, vz, vw);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> ReciprocalFast(const Vector4Template<T>& vec)
{
	T vx = ReciprocalFast(vec.x);
	T vy = ReciprocalFast(vec.y);
	T vz = ReciprocalFast(vec.z);
	T vw = ReciprocalFast(vec.w);
    Vector4Template<T> r(vx, vy, vz, vw);
    return r;
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> RotateLeft(const Vector4Template<T>& vec)
{
	return Vector4Template<T>(vec.y, vec.z, vec.w, vec.x);
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> RotateRight(const Vector4Template<T>& vec)
{
	return Vector4Template<T>(vec.w, vec.x, vec.y, vec.z);
}

template<class T>
RW_MATH_FORCE_INLINE Vector4Template<T> LoadUnAlignedPacked(const Vector4Template<T>& value)
{
    return value;
}

template<class T>
RW_MATH_FORCE_INLINE void StoreUnAlignedPacked(Vector4 &destination, const Vector4Template<T>& value)
{
    destination = value;
}

// functions return vector by reference

#if RW_MATH_ENABLE_DEPRECATED_API


template<class T>
RW_MATH_INLINE void Negate(const Vector4Template<T>& vec, Vector4Template<T>& r) 
{
    r.Set(-vec.x, -vec.y, -vec.z, -vec.w);
}


template<class T>
RW_MATH_INLINE void Mult(const Vector4Template<T>& vec, T scalar, Vector4Template<T>& r) 
{
    r.Set(vec.x*scalar, vec.y*scalar, vec.z*scalar, vec.w*scalar);
}


template<class T>
RW_MATH_INLINE void Divide(const Vector4Template<T>& vec, T scalar, Vector4Template<T>& r) 
{
    T scalarInv= static_cast<T>(1.)/scalar;
    r.Set(vec.x*scalarInv, vec.y*scalarInv, vec.z*scalarInv, vec.w*scalarInv);
}


template<class T>
RW_MATH_INLINE void Add(const Vector4Template<T>& a, const Vector4Template<T>& b, Vector4Template<T>& r) 
{
    r.Set(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}


template<class T>
RW_MATH_INLINE void Subtract(const Vector4Template<T>& a, const Vector4Template<T>& b, Vector4Template<T>& r) 
{
    r.Set(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}


template<class T>
RW_MATH_INLINE void Normalize(const Vector4Template<T>& vec, Vector4Template<T>& r) 
{
    Mult(vec, MagnitudeRecip(vec), r);
}


template<class T>
RW_MATH_INLINE void Mult(const Vector4Template<T>& a, const Vector4Template<T>& b, Vector4Template<T>& r) 
{
    r.Set(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}


template<class T>
RW_MATH_INLINE void Lerp(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t, Vector4Template<T>& r) 
{
    Subtract(v1, v0, r);
    Mult(r, t,r );
    Add(v0, r,r );
}


template<class T>
RW_MATH_INLINE void NLerp(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t, Vector4Template<T>& r) 
{
    Subtract(v1, v0, r);
    Mult(r, t, r);
    Add(v0, r, r);
    Normalize(r, r);
}


template<class T>
RW_MATH_INLINE void SLerp(const Vector4Template<T>& v0, const Vector4Template<T>& v1, T t, Vector4Template<T>& r) 
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
        Vector4Template<T> a, b;

        Mult(v0, Sin((static_cast<T>(1.0f)-t)*theta)/Sin(theta), a);
        Mult(v1, Sin(t*theta)/Sin(theta), b);
        Add(a, b, r);
    }
}


template<class T>
RW_MATH_INLINE void CrossXYZ(const Vector4Template<T>& v0, const Vector4Template<T>& v1, Vector4Template<T>& r) 
{
    r = Vector4Template<T>(
            v0.y*v1.z - v1.y*v0.z, 
            v0.z*v1.x - v0.x*v1.z,
            v0.x*v1.y - v0.y*v1.x,
            static_cast<T>(0.0f)
        );
}


#endif

} // namespace fpu
} // namespace math
} // namespace rw

#endif
