// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_VECTOR3PLUS_H
#define RW_MATH_FPU_VECTOR3PLUS_H

#if defined _MSC_VER
    #pragma once
#endif


#include "rw/math/fpu/vector3.h"
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
class Vector3PlusTemplate
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
    enum { NumberOfElement = 4 };
    typedef const Vector3PlusTemplate& InParam;
    typedef T FloatType;
    typedef T FastFloatType;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Vector3PlusTemplate& OutParam;
        typedef Vector3PlusTemplate& InOutParam;
    #endif

    RW_MATH_INLINE Vector3PlusTemplate(const Vector3PlusTemplate<float> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.UserData());
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
        this->z = static_cast<T>(v.z);
        this->userData = static_cast<T>(v.UserData());
    }

    RW_MATH_INLINE Vector3PlusTemplate(const Vector3PlusTemplate<double> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.UserData());
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
        this->z = static_cast<T>(v.z);
        this->userData = static_cast<T>(v.UserData());
    }

    RW_MATH_INLINE Vector3PlusTemplate(const Vector3Template<T> &v, T userData)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->userData = userData;
    }

    explicit RW_MATH_INLINE Vector3PlusTemplate(const rw::math::vpu::Vector3Plus &v);

    RW_MATH_INLINE Vector3PlusTemplate()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->x);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->y);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->z);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->userData);
#endif
    }

	RW_MATH_FORCE_INLINE Vector3PlusTemplate& operator = (const Vector3PlusTemplate<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.UserData());
#endif
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->userData = v.UserData();
        return *this;
    }

    RW_MATH_INLINE Vector3PlusTemplate(T _xyz, T plus)
    {
        Set(_xyz, _xyz, _xyz, plus);
    }

    RW_MATH_INLINE Vector3PlusTemplate( T _x, T _y, T _z, T plus )
    {
        Set(_x, _y, _z, plus);
    }
    
    explicit RW_MATH_INLINE Vector3PlusTemplate( T xyzplus[4] )
    {
        Set(xyzplus[0],xyzplus[1],xyzplus[2],xyzplus[3]);
    }

    RW_MATH_INLINE void Set(T _x, T _y, T _z, T plus)
    {
        this->x = _x;
        this->y = _y;
        this->z = _z;
        this->userData = plus;
    }

    RW_MATH_INLINE void Set(const Vector3Template<T> &v, const rw::math::fpu::VecFloat &userData)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->userData = static_cast<T> (userData);
    }
    
    RW_MATH_INLINE void SetComponent(int i, T value)       { GET_COMPONENT(i) = value; }

    RW_MATH_INLINE void SetX(T _x)
    {
        this->x = _x;
    }
    
    RW_MATH_INLINE void SetY(T _y)
    {
        this->y = _y;
    }
    
    RW_MATH_INLINE void SetZ(T _z)
    {
        this->z = _z;
    }
    
    RW_MATH_INLINE void SetPlus(T plus)
    {
        this->userData = plus;
    }

    RW_MATH_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_INLINE const T& GetX() const { return this->x; }
    RW_MATH_INLINE T&       GetX() { return this->x; }

    RW_MATH_INLINE const T& GetY() const { return this->y; }
    RW_MATH_INLINE T&       GetY() { return this->y; }

    RW_MATH_INLINE const T& GetZ() const { return this->z; }
    RW_MATH_INLINE T&       GetZ() { return this->z; }

    RW_MATH_INLINE const T& GetPlus() const { return this->userData; }
    RW_MATH_INLINE T&       GetPlus() { return this->userData; }

    RW_MATH_INLINE const T& X() const { return this->x; }
    RW_MATH_INLINE T&       X() { return this->x; }

    RW_MATH_INLINE const T& Y() const { return this->y; }
    RW_MATH_INLINE T&       Y() { return this->y; }

    RW_MATH_INLINE const T& Z() const { return this->z; }
    RW_MATH_INLINE T&       Z() { return this->z; }

    RW_MATH_INLINE const T& UserData() const { return this->userData; }
    RW_MATH_INLINE T&       UserData() { return this->userData; }

    RW_MATH_INLINE T&       GetComponent(int i) { return GET_COMPONENT(i); }
    RW_MATH_INLINE const T& GetComponent(int i) const { return GET_COMPONENT(i); }

    RW_MATH_INLINE T&       operator [] (int i) { return GET_COMPONENT(i); }
    RW_MATH_INLINE const T& operator [] (int i) const { return GET_COMPONENT(i); }

    // get and set as Vector3 object
    RW_MATH_INLINE void           SetVector3(const Vector3Template<T>& v3)
    {
        Set(v3.x, v3.y, v3.z, UserData());
    }

    RW_MATH_INLINE Vector3Template<T> GetVector3() const
    {
        return Vector3Template<T>(this->x, this->y, this->z);
    }

public:
    
    T x;
    T y;
    T z;
    T userData;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static T Vector3PlusTemplate<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> float Vector3PlusTemplate<float>::*  const Vector3PlusTemplate<float>::sElements[];
    template<> double Vector3PlusTemplate<double>::*  const Vector3PlusTemplate<double>::sElements[];
#endif

typedef Vector3PlusTemplate<float>   Vector3Plus;
typedef Vector3PlusTemplate<float>   Vector3Plus_32;
typedef Vector3PlusTemplate<double>  Vector3Plus_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef Vector3PlusTemplate<float>   Vector3PlusU_32;
    //! \internal
    typedef Vector3PlusTemplate<double>  Vector3PlusU_64;
#endif

}}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::Vector3PlusTemplate<T>& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::fpu::Vector3PlusTemplate<T>), sizeof(T));
}

}

#undef GET_COMPONENT

#endif
