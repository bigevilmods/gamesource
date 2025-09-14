// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASKSCALAR_H
#define RW_MATH_FPU_MASKSCALAR_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/fpu/vec_float.h"

namespace rw
{
namespace math
{
namespace fpu
{


template <class T>
class MaskScalarTemplate
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
    enum { NumberOfElement = 1 };
    typedef const MaskScalarTemplate<T>& InParam;
    typedef T FloatType;
    typedef T FastFloatType;

public:
    RW_MATH_FORCE_INLINE MaskScalarTemplate()
    {
#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
		RW_MATH_FPU_MEMBER_INIT_FUNCTION(this->mS);
#endif
    }

    RW_MATH_FORCE_INLINE MaskScalarTemplate(const MaskScalarTemplate<T> &v)
        :mS(v.mS)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mS);
#endif
    }

    RW_MATH_FORCE_INLINE MaskScalarTemplate<T>& operator =(const MaskScalarTemplate<T> &v)
    {
#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
		RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(v.mS);
#endif
        this->mS = v.mS;
        return *this;
    }

    /*! 
     * Construct a mask scalar from a floating point value.
     *
     * \param[in] v A floating point value that represents a boolean value.  A number less than zero represents false, and a number greater or equal to zero represents true.
     */
    RW_MATH_FORCE_INLINE explicit MaskScalarTemplate(T v)
        :mS(v)
    {
    }

    RW_MATH_FORCE_INLINE explicit MaskScalarTemplate(bool v)
        :mS( v ? T(1.0f) : T(-1.0f) )
    {
    }

    RW_MATH_FORCE_INLINE bool GetBool() const
    {
        return ( mS >= T(0.0f) );
    }

    RW_MATH_FORCE_INLINE void SetTrue()
    {
        mS = 1.0f;
    }

    RW_MATH_FORCE_INLINE void SetFalse()
    {
        mS = -1.0f;
    }

public:
    T mS;
};

typedef MaskScalarTemplate<float>  MaskScalar;
typedef MaskScalarTemplate<float>  MaskScalar_32;
typedef MaskScalarTemplate<double> MaskScalar_64;

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASKSCALAR_H
