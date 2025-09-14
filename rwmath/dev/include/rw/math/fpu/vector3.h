// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECTOR3_H
#define RW_MATH_FPU_VECTOR3_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector2.h"
#include "rw/math/fpu/vec_float.h"
#include "rw/math/types_fwd.h"

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    #define GET_COMPONENT(i) \
        (this->*sElements[i])
#else
    #define GET_COMPONENT(i) \
        (&this->x)[i]
#endif

namespace rw {
namespace math {
namespace fpu {

template <class T>
class Vector3Template
{
public:
#ifdef RWCROSS
    // To avoid any potential impact on Renderware ACP code, the FPU types
    // claim to be unaligned.  On standard builds, the FPU types are aligned
    // according to the required alignement specified by the compiler.
    enum { IsAligned = 0 };
    enum { Alignment = 1 };
#else
    enum { IsAligned = 1 };
    enum { Alignment = EA_ALIGN_OF(T) };
#endif
    enum { NumberOfElement = 3 };
    typedef const Vector3Template& InParam;
    typedef T FloatType;
    typedef T FastFloatType;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Vector3Template& OutParam;
        typedef Vector3Template& InOutParam;
    #endif

    RW_MATH_FORCE_INLINE Vector3Template(const Vector3Template<float> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
        this->z = static_cast<T>(v.z);
    }

    RW_MATH_FORCE_INLINE Vector3Template(const Vector3Template<double> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
        this->z = static_cast<T>(v.z);
    }

    explicit RW_MATH_INLINE Vector3Template(const rw::math::vpu::Vector3 &v);

    RW_MATH_FORCE_INLINE Vector3Template(const Vector2Template<T>& v2, const T _z)
    {
        this->x = v2.x;
        this->y = v2.y;
        this->z = _z;
    }

    //! This constructor creates a vector with all its elements equal to \c scalar.
    RW_MATH_FORCE_INLINE explicit Vector3Template(T scalar)
    {
        this->x = scalar;
        this->y = scalar;
        this->z = scalar;
    }

    RW_MATH_FORCE_INLINE Vector3Template(T _x, T _y, T _z) 
    {
        this->x = _x;
        this->y = _y;
        this->z = _z;
    }

    RW_MATH_FORCE_INLINE Vector3Template& operator = (const Vector3Template<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
#endif
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        return *this;
    }

    explicit RW_MATH_INLINE Vector3Template(const T xyz[3])
    {
        this->x = xyz[0];
        this->y = xyz[1];
        this->z = xyz[2];
    }
    
    RW_MATH_FORCE_INLINE Vector3Template() 
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->x);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->y);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->z);
#endif
    }

    RW_MATH_FORCE_INLINE void Set(T _x, T _y, T _z) { this->x = _x; this->y = _y; this->z = _z; }

    RW_MATH_FORCE_INLINE void SetComponent(int i, T value)       { GET_COMPONENT(i) = value; }

    RW_MATH_FORCE_INLINE void SetX(T _x) { this->x = _x; }
    RW_MATH_FORCE_INLINE void SetY(T _y) { this->y = _y; }
    RW_MATH_FORCE_INLINE void SetZ(T _z) { this->z = _z; }

    RW_MATH_FORCE_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_FORCE_INLINE const T&  GetX() const { return this->x; }
    RW_MATH_FORCE_INLINE const T&  GetY() const { return this->y; }
    RW_MATH_FORCE_INLINE const T&  GetZ() const { return this->z; }
    RW_MATH_FORCE_INLINE T& GetX()       { return this->x; }
    RW_MATH_FORCE_INLINE T& GetY()       { return this->y; }
    RW_MATH_FORCE_INLINE T& GetZ()       { return this->z; }

    RW_MATH_FORCE_INLINE const T&  X() const { return this->x; }
    RW_MATH_FORCE_INLINE const T&  Y() const { return this->y; }
    RW_MATH_FORCE_INLINE const T&  Z() const { return this->z; }
    RW_MATH_FORCE_INLINE T& X()       { return this->x; }
    RW_MATH_FORCE_INLINE T& Y()       { return this->y; }
    RW_MATH_FORCE_INLINE T& Z()       { return this->z; }

    RW_MATH_FORCE_INLINE T&       GetComponent(int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& GetComponent(int i) const { return GET_COMPONENT(i); }

    RW_MATH_FORCE_INLINE T&       operator [] (int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& operator [] (int i) const { return GET_COMPONENT(i); }

public:
    
    T x;
    T y;   
    T z;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static T Vector3Template<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> float Vector3Template<float>::*  const Vector3Template<float>::sElements[];
    template<> double Vector3Template<double>::*  const Vector3Template<double>::sElements[];
#endif

typedef Vector3Template<float>   Vector3;
typedef Vector3Template<float>   Vector3_32;
typedef Vector3Template<double>  Vector3_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef Vector3Template<float>   Vector3U_32;
    //! \internal
    typedef Vector3Template<double>  Vector3U_64;
#endif

}}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::Vector3Template<T>& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::fpu::Vector3Template<T>), sizeof(T));
}

}

#undef GET_COMPONENT

#endif
