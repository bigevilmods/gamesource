// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECTOR2_H
#define RW_MATH_FPU_VECTOR2_H

#if defined _MSC_VER
    #pragma once
#endif


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
class Vector2Template
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
    enum { NumberOfElement = 2 };
    typedef const Vector2Template& InParam;
    typedef T FloatType;
    typedef T FastFloatType;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Vector2Template& OutParam;
        typedef Vector2Template& InOutParam;
    #endif

    RW_MATH_FORCE_INLINE Vector2Template(const Vector2Template<float> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
    }

    RW_MATH_FORCE_INLINE Vector2Template(const Vector2Template<double> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
    }

    explicit RW_MATH_INLINE Vector2Template(const rw::math::vpu::Vector2 &v);

    //! This constructor creates a vector with all its elements equal to \c scalar.
    RW_MATH_FORCE_INLINE explicit Vector2Template(T scalar)
    {
        this->x = scalar;
        this->y = scalar;
    }

    RW_MATH_FORCE_INLINE Vector2Template(T _x, T _y)
    {
        this->x = _x;
        this->y = _y;
    }

    RW_MATH_FORCE_INLINE explicit Vector2Template(const T xy[2])
    {
        this->x = xy[0];
        this->y = xy[1];
    }

    RW_MATH_FORCE_INLINE Vector2Template()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->x);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->y);
#endif
    }

    RW_MATH_FORCE_INLINE Vector2Template& operator = (const Vector2Template<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
#endif
        this->x = v.x;
        this->y = v.y;
        return *this;
    }

    RW_MATH_FORCE_INLINE void Set(T _x, T _y)
    {
        this->x = _x;
        this->y = _y;
    }

    RW_MATH_FORCE_INLINE void SetComponent(int i, T value)       { GET_COMPONENT(i) = value; }

    RW_MATH_FORCE_INLINE void SetX(T _x)
    {
        this->x = _x;
    }
    RW_MATH_FORCE_INLINE void SetY(T _y)
    {
        this->y = _y;
    }

    RW_MATH_FORCE_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_FORCE_INLINE const T&  GetX() const { return this->x; }
    RW_MATH_FORCE_INLINE const T&  GetY() const { return this->y; }
    RW_MATH_FORCE_INLINE T& GetX()       { return this->x; }
    RW_MATH_FORCE_INLINE T& GetY()       { return this->y; }

    RW_MATH_FORCE_INLINE const T&  X() const { return this->x; }
    RW_MATH_FORCE_INLINE const T&  Y() const { return this->y; }
    RW_MATH_FORCE_INLINE T& X()       { return this->x; }
    RW_MATH_FORCE_INLINE T& Y()       { return this->y; }

    RW_MATH_FORCE_INLINE T&       GetComponent(int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& GetComponent(int i) const { return GET_COMPONENT(i); }

    RW_MATH_FORCE_INLINE T&       operator [] (int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& operator [] (int i) const { return GET_COMPONENT(i); }

public:
    
    T x;
    T y;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static T Vector2Template<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> float Vector2Template<float>::*  const Vector2Template<float>::sElements[];
    template<> double Vector2Template<double>::*  const Vector2Template<double>::sElements[];
#endif

typedef Vector2Template<float>  Vector2;
typedef Vector2Template<float>  Vector2_32;
typedef Vector2Template<double> Vector2_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef Vector2Template<float>  Vector2U_32;
    //! \internal
    typedef Vector2Template<double> Vector2U_64;
#endif

}}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::Vector2Template<T>& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::fpu::Vector2Template<T>), sizeof(T));
}

}

#undef GET_COMPONENT

#endif
