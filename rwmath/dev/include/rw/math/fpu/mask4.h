// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASK4_H
#define RW_MATH_FPU_MASK4_H

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
class Mask4Template
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
    typedef const Mask4Template<T>& InParam;
    typedef T FloatType;
    typedef T FastFloatType;

public:
    RW_MATH_FORCE_INLINE Mask4Template()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mX);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mY);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mZ);
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mW);
#endif
    }

    RW_MATH_FORCE_INLINE Mask4Template(const Mask4Template &v)
        :mX(v.mX)
        ,mY(v.mY)
        ,mZ(v.mZ)
        ,mW(v.mW)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mX);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mY);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mZ);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mW);
#endif
    }

    RW_MATH_FORCE_INLINE Mask4Template<T>& operator =(const Mask4Template<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mX);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mY);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mZ);
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mW);
#endif
        this->mX = v.mX;
        this->mY = v.mY;
        this->mZ = v.mZ;
        this->mW = v.mW;
        return *this;
    }

    /*! 
     * Construct a mask vector from floating point numbers.
     *
     * A number less than zero represents false, and a number greater or equal to zero represents true.
     *
     * \param[in] x A floating point value that represents a boolean value. 
     * \param[in] y A floating point value that represents a boolean value. 
     * \param[in] z A floating point value that represents a boolean value. 
     * \param[in] w A floating point value that represents a boolean value. 
     */
    RW_MATH_FORCE_INLINE explicit Mask4Template(T x, T y, T z, T w)
        :mX(x)
        ,mY(y)
        ,mZ(z)
        ,mW(w)
    {
    }

    RW_MATH_FORCE_INLINE explicit Mask4Template(const MaskScalarTemplate<T> &s)
        :mX(s.mS)
        ,mY(s.mS)
        ,mZ(s.mS)
        ,mW(s.mS)
    {
    }

    RW_MATH_FORCE_INLINE explicit Mask4Template(const MaskScalarTemplate<T> &x, const MaskScalarTemplate<T> &y, const MaskScalarTemplate<T> &z, const MaskScalarTemplate<T> &w)
        :mX(x.mS)
        ,mY(y.mS)
        ,mZ(z.mS)
        ,mW(w.mS)
    {
    }

    RW_MATH_FORCE_INLINE explicit Mask4Template(bool x, bool y, bool z, bool w)
        :mX(x ? T(1.0f) : T(-1.0f))
        ,mY(y ? T(1.0f) : T(-1.0f))
        ,mZ(z ? T(1.0f) : T(-1.0f))
        ,mW(w ? T(1.0f) : T(-1.0f))
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

    RW_MATH_INLINE MaskScalarTemplate<T> GetZ() const
    {
        return MaskScalarTemplate<T>(mZ);
    }

    RW_MATH_INLINE MaskScalarTemplate<T> GetW() const
    {
        return MaskScalarTemplate<T>(mW);
    }

    RW_MATH_INLINE void SetX( typename MaskScalarTemplate<T>::InParam v )
    {
        mX = v.mS;
    }

    RW_MATH_INLINE void SetY( typename MaskScalarTemplate<T>::InParam v )
    {
        mY = v.mS;
    }

    RW_MATH_INLINE void SetZ( typename MaskScalarTemplate<T>::InParam v )
    {
        mZ = v.mS;
    }

    RW_MATH_INLINE void SetW( typename MaskScalarTemplate<T>::InParam v )
    {
        mW = v.mS;
    }

    RW_MATH_FORCE_INLINE void SetTrue()
    {
        mX = 1.0f;
        mY = 1.0f;
        mZ = 1.0f;
        mW = 1.0f;
    }

    RW_MATH_FORCE_INLINE void SetFalse()
    {
        mX = -1.0f;
        mY = -1.0f;
        mZ = -1.0f;
        mW = -1.0f;
    }

public:
    T mX;
    T mY;
    T mZ;
    T mW;
};

typedef Mask4Template<float>  Mask4;
typedef Mask4Template<float>  Mask4_32;
typedef Mask4Template<double> Mask4_64;

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASK4_H
