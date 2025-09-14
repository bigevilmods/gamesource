// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_QUATERNION_H
#define RW_MATH_FPU_QUATERNION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
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
class QuaternionTemplate
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
    typedef const QuaternionTemplate& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef QuaternionTemplate& OutParam;
        typedef QuaternionTemplate& InOutParam;
    #endif

    RW_MATH_FORCE_INLINE QuaternionTemplate(const QuaternionTemplate<float> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.w);
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
        this->z = static_cast<T>(v.z);
        this->w = static_cast<T>(v.w);
    }

    RW_MATH_FORCE_INLINE QuaternionTemplate(const QuaternionTemplate<double> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.w);
#endif
        this->x = static_cast<T>(v.x);
        this->y = static_cast<T>(v.y);
        this->z = static_cast<T>(v.z);
        this->w = static_cast<T>(v.w);
    }

    explicit RW_MATH_INLINE QuaternionTemplate(const rw::math::vpu::Quaternion &v);

    RW_MATH_FORCE_INLINE QuaternionTemplate(T _x, T _y, T _z, T _w)
    {
        this->x = _x; 
        this->y = _y;
        this->z = _z;
        this->w = _w;
    }

    explicit RW_MATH_INLINE QuaternionTemplate(const T xyzw[4])
    {
        this->x = xyzw[0];
        this->y = xyzw[1];
        this->z = xyzw[2];
        this->w = xyzw[3];
    }

    RW_MATH_FORCE_INLINE QuaternionTemplate(const Vector3Template<T> &v3, T w )
    {
        this->x = v3.x;
        this->y = v3.y;
        this->z = v3.z;
        this->w = w;
    }

    RW_MATH_FORCE_INLINE QuaternionTemplate()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->x);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->y);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->z);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->w);
#endif
    }

    RW_MATH_FORCE_INLINE QuaternionTemplate& operator = (const QuaternionTemplate<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.x);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.y);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.z);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.w);
#endif
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = v.w;
        return *this;
    }

    RW_MATH_FORCE_INLINE void Set(T _x, T _y, T _z, T _w) 
    {
        this->x = _x; 
        this->y = _y; 
        this->z = _z; 
        this->w = _w;
    }

    RW_MATH_FORCE_INLINE void SetComponent(int i, const T& value)   { GET_COMPONENT(i) = value; }

    RW_MATH_FORCE_INLINE void SetX(T _x) { this->x = _x; }
    RW_MATH_FORCE_INLINE void SetY(T _y) { this->y = _y; }
    RW_MATH_FORCE_INLINE void SetZ(T _z) { this->z = _z; }
    RW_MATH_FORCE_INLINE void SetW(T _w) { this->w = _w; }

    RW_MATH_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_INLINE void SetIdentity() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));}

    RW_MATH_FORCE_INLINE T  GetX() const { return this->x; }
    RW_MATH_FORCE_INLINE T  GetY() const { return this->y; }
    RW_MATH_FORCE_INLINE T  GetZ() const { return this->z; }
    RW_MATH_FORCE_INLINE T  GetW() const { return this->w; }
    RW_MATH_FORCE_INLINE T& GetX()       { return this->x; }
    RW_MATH_FORCE_INLINE T& GetY()       { return this->y; }
    RW_MATH_FORCE_INLINE T& GetZ()       { return this->z; }
    RW_MATH_FORCE_INLINE T& GetW()       { return this->w; }

    RW_MATH_FORCE_INLINE const T& X() const { return this->x; }
    RW_MATH_FORCE_INLINE const T& Y() const { return this->y; }
    RW_MATH_FORCE_INLINE const T& Z() const { return this->z; }
    RW_MATH_FORCE_INLINE const T& W() const { return this->w; }
    RW_MATH_FORCE_INLINE T& X()       { return this->x; }
    RW_MATH_FORCE_INLINE T& Y()       { return this->y; }
    RW_MATH_FORCE_INLINE T& Z()       { return this->z; }
    RW_MATH_FORCE_INLINE T& W()       { return this->w; }

    RW_MATH_FORCE_INLINE T&       GetComponent(int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& GetComponent(int i) const { return GET_COMPONENT(i); }

    RW_MATH_FORCE_INLINE T&       operator [] (int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& operator [] (int i) const { return GET_COMPONENT(i); }

public:
    
    T x;
    T y;
    T z;
    T w;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static T QuaternionTemplate<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> float QuaternionTemplate<float>::*  const QuaternionTemplate<float>::sElements[];
    template<> double QuaternionTemplate<double>::*  const QuaternionTemplate<double>::sElements[];
#endif

typedef QuaternionTemplate<float>    Quaternion;
typedef QuaternionTemplate<float>    Quaternion_32;
typedef QuaternionTemplate<double>   Quaternion_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef QuaternionTemplate<float>   QuaternionU_32;
    //! \internal
    typedef QuaternionTemplate<double>  QuaternionU_64;
#endif

}}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::QuaternionTemplate<T>& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::fpu::QuaternionTemplate<T>), sizeof(T));
}

}

#undef GET_COMPONENT

#endif
