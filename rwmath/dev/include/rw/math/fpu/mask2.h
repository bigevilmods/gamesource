// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASK2_H
#define RW_MATH_FPU_MASK2_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/fpu/maskscalar.h"

namespace rw
{
namespace math
{
namespace fpu
{


template <class T>
class Mask2Template
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
    typedef const Mask2Template<T>& InParam;
    typedef T FloatType;
    typedef T FastFloatType;

public:
    RW_MATH_FORCE_INLINE Mask2Template()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mX);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mY);
#endif
    }

    RW_MATH_FORCE_INLINE Mask2Template(const Mask2Template &v)
        :mX(v.mX)
        ,mY(v.mY)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mX);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mY);
#endif
    }

    RW_MATH_FORCE_INLINE Mask2Template<T>& operator =(const Mask2Template<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mX);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mY);
#endif
        this->mX = v.mX;
        this->mY = v.mY;
        return *this;
    }

    /*! 
     * Construct a mask vector from floating point numbers.
     *
     * A number less than zero represents false, and a number greater or equal to zero represents true.
     *
     * \param[in] x A floating point value that represents a boolean value. 
     * \param[in] y A floating point value that represents a boolean value. 
     */
    RW_MATH_FORCE_INLINE explicit Mask2Template(T x, T y)
        :mX(x)
        ,mY(y)
    {
    }

    RW_MATH_FORCE_INLINE explicit Mask2Template(const MaskScalarTemplate<T> &s)
        :mX(s.mS)
        ,mY(s.mS)
    {
    }

    RW_MATH_FORCE_INLINE explicit Mask2Template(const MaskScalarTemplate<T> &x, const MaskScalarTemplate<T> &y)
        :mX(x.mS)
        ,mY(y.mS)
    {
    }

    RW_MATH_FORCE_INLINE explicit Mask2Template(bool x, bool y)
        :mX(x ? T(1.0f) : T(-1.0f))
        ,mY(y ? T(1.0f) : T(-1.0f))
    {
    }

    RW_MATH_INLINE MaskScalarTemplate<T> GetX() const
    {
        return MaskScalarTemplate<T>(mX);
    }

    RW_MATH_INLINE MaskScalarTemplate<T> GetY() const
    {
        return MaskScalarTemplate<T>(mY);
    }

    RW_MATH_INLINE void SetX( typename MaskScalarTemplate<T>::InParam v )
    {
        mX = v.mS;
    }

    RW_MATH_INLINE void SetY( typename MaskScalarTemplate<T>::InParam v )
    {
        mY = v.mS;
    }

    RW_MATH_FORCE_INLINE void SetTrue()
    {
        mX = 1.0f;
        mY = 1.0f;
    }

    RW_MATH_FORCE_INLINE void SetFalse()
    {
        mX = -1.0f;
        mY = -1.0f;
    }

public:
    T mX;
    T mY;
};

typedef Mask2Template<float>  Mask2;
typedef Mask2Template<float>  Mask2_32;
typedef Mask2Template<double> Mask2_64;

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASK2_H
