// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASKSCALAR_OPERATION_INLINE_H
#define RW_MATH_FPU_MASKSCALAR_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/maskscalar.h"
#include "rw/math/fpu/detail/float_intrinsic_operation.h"


namespace rw
{
namespace math
{
namespace fpu
{

//! Returns the boolean 'Not' of the mask.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> Not(const MaskScalarTemplate<T> &a)
{
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mS, T(-1.0f), T(1.0f) )));
}

//! Returns the boolean 'And' of the masks.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> And(const MaskScalarTemplate<T> &a, const MaskScalarTemplate<T> &b)
{
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mS, b.mS, T(-1.0f) )));
}

//! Returns the boolean 'Or' of the masks.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> Or(const MaskScalarTemplate<T> &a, const MaskScalarTemplate<T> &b)
{
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mS, a.mS, b.mS )));
}

//! Returns the boolean 'Xor' of the masks.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> Xor(const MaskScalarTemplate<T> &a, const MaskScalarTemplate<T> &b)
{
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mS, Not(b).mS, b.mS )));
}

//! Compare's two MaskScalar types and returns a mask indicating whether or not the masks are equal.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompEqual(const MaskScalarTemplate<T> &a, const MaskScalarTemplate<T> &b)
{
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mS, b.mS, Not(b).mS )));
}

//! Compare's two MaskScalar types and returns a mask indicating whether or not the masks are not equal.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompNotEqual(const MaskScalarTemplate<T> &a, const MaskScalarTemplate<T> &b)
{
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mS, Not(b).mS, b.mS )));
}

template <class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> Select(const MaskScalarTemplate<T> &mask, const MaskScalarTemplate<T> &trueValue, const MaskScalarTemplate<T> &falseValue )
{
    return MaskScalarTemplate<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mS, falseValue.mS )
        );
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASKSCALAR_OPERATION_INLINE_H
