// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_PLANE_H
#define RW_MATH_FPU_PLANE_H

#if defined _MSC_VER
    #pragma once
#endif


#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/vec_float.h"
#include "rw/math/types_fwd.h"

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    #define GET_COMPONENT(i) \
        (this->*sElements[i])
#else
    #define GET_COMPONENT(i) \
        (&this->a)[i]
#endif

namespace rw {
namespace math {
namespace fpu {

enum PlaneIntersection
{
    PI_ZERO,
    PI_ONE,
    PI_INFINITY
};

template <class T>
class PlaneTemplate
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
    typedef const PlaneTemplate& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef PlaneTemplate& OutParam;
        typedef PlaneTemplate& InOutParam;
    #endif

    RW_MATH_FORCE_INLINE PlaneTemplate(const PlaneTemplate<float> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.a);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.b);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.c);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.d);
#endif
        this->a = static_cast<T>(v.a);
        this->b = static_cast<T>(v.b);
        this->c = static_cast<T>(v.c);
        this->d = static_cast<T>(v.d);
    }

    RW_MATH_FORCE_INLINE PlaneTemplate(const PlaneTemplate<double> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.a);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.b);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.c);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.d);
#endif
        this->a = static_cast<T>(v.a);
        this->b = static_cast<T>(v.b);
        this->c = static_cast<T>(v.c);
        this->d = static_cast<T>(v.d);
    }

    explicit RW_MATH_INLINE PlaneTemplate(const rw::math::vpu::Plane &v);

    RW_MATH_FORCE_INLINE PlaneTemplate()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->a);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->b);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->c);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->d);
#endif
    }

    RW_MATH_FORCE_INLINE PlaneTemplate& operator = (const PlaneTemplate<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.a);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.b);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.c);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.d);
#endif
        this->a = v.a;
        this->b = v.b;
        this->c = v.c;
        this->d = v.d;
        return *this;
    }

    RW_MATH_FORCE_INLINE PlaneTemplate(const Vector3Template<T> &normal, T _d)
    {
        Set(normal, _d);
    }

    RW_MATH_FORCE_INLINE explicit PlaneTemplate(const T abcd[4])
    {
        Set(abcd[0], abcd[1], abcd[2], abcd[3]);
    }

    RW_MATH_FORCE_INLINE PlaneTemplate(T _a, T _b, T _c, T _d)
    {
        Set(_a, _b, _c, _d);
    }

    RW_MATH_FORCE_INLINE void Set(const Vector3Template<T> &normal, T _d)
    {
        Set(normal.x, normal.y, normal.z, _d);
    }

    RW_MATH_FORCE_INLINE void Set(T _a, T _b, T _c, T _d)
    {
        this->a = _a;
        this->b = _b;
        this->c = _c;
        this->d = _d;
    }

    RW_MATH_FORCE_INLINE void SetComponent(int i, T value)       { GET_COMPONENT(i) = value; }

    RW_MATH_FORCE_INLINE void SetA(T _a) { this->a = _a; }
    RW_MATH_FORCE_INLINE void SetB(T _b) { this->b = _b; }
    RW_MATH_FORCE_INLINE void SetC(T _c) { this->c = _c; }
    RW_MATH_FORCE_INLINE void SetD(T _d) { this->d = _d; }

    RW_MATH_FORCE_INLINE void A(T _a) { this->a = _a; }
    RW_MATH_FORCE_INLINE void B(T _b) { this->b = _b; }
    RW_MATH_FORCE_INLINE void C(T _c) { this->c = _c; }
    RW_MATH_FORCE_INLINE void D(T _d) { this->d = _d; }

    RW_MATH_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f)); }

    RW_MATH_FORCE_INLINE const T&  GetA() const { return this->a; }
    RW_MATH_FORCE_INLINE const T&  GetB() const { return this->b; }
    RW_MATH_FORCE_INLINE const T&  GetC() const { return this->c; }
    RW_MATH_FORCE_INLINE const T&  GetD() const { return this->d; }
    RW_MATH_FORCE_INLINE T& GetA()              { return this->a; }
    RW_MATH_FORCE_INLINE T& GetB()              { return this->b; }
    RW_MATH_FORCE_INLINE T& GetC()              { return this->c; }
    RW_MATH_FORCE_INLINE T& GetD()              { return this->d; }

    RW_MATH_FORCE_INLINE const T&  A() const { return this->a; }
    RW_MATH_FORCE_INLINE const T&  B() const { return this->b; }
    RW_MATH_FORCE_INLINE const T&  C() const { return this->c; }
    RW_MATH_FORCE_INLINE const T&  D() const { return this->d; }
    RW_MATH_FORCE_INLINE T& A()       { return this->a; }
    RW_MATH_FORCE_INLINE T& B()       { return this->b; }
    RW_MATH_FORCE_INLINE T& C()       { return this->c; }
    RW_MATH_FORCE_INLINE T& D()       { return this->d; }

    RW_MATH_FORCE_INLINE T&       GetComponent(int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& GetComponent(int i) const { return GET_COMPONENT(i); }

    RW_MATH_FORCE_INLINE T&       operator [] (int i)       { return GET_COMPONENT(i); }
    RW_MATH_FORCE_INLINE const T& operator [] (int i) const { return GET_COMPONENT(i); }

    RW_MATH_FORCE_INLINE void SetNormal(const Vector3Template<T>& v)
    {
        Set(v, this->d);
    }

    RW_MATH_FORCE_INLINE Vector3Template<T> GetNormal() const
    {
        return Vector3Template<T>(this->a, this->b, this->c);
    }

    RW_MATH_FORCE_INLINE       T& SignedDistance()       { return d; }
    RW_MATH_FORCE_INLINE const T& SignedDistance() const { return d; }

    RW_MATH_FORCE_INLINE T Distance() const { return Abs(d); }

    RW_MATH_FORCE_INLINE Vector4Template<T> GetVector() const
    { 
        return Vector4Template<T>(this->a, this->b, this->c, this->d);
    }

public:
    
    T a;
    T b;
    T c;
    T d;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static T PlaneTemplate<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> float PlaneTemplate<float>::*  const PlaneTemplate<float>::sElements[];
    template<> double PlaneTemplate<double>::*  const PlaneTemplate<double>::sElements[];
#endif

typedef PlaneTemplate<float>   Plane;
typedef PlaneTemplate<float>   Plane_32;
typedef PlaneTemplate<double>  Plane_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef PlaneTemplate<float>   PlaneU_32;
    //! \internal
    typedef PlaneTemplate<double>  PlaneU_64;
#endif

}

}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::PlaneTemplate<T>& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::fpu::PlaneTemplate<T>), sizeof(T));
}

}

#undef GET_COMPONENT

#endif
