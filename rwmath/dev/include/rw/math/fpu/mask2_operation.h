// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASK2_OPERATION_INLINE_H
#define RW_MATH_FPU_MASK2_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/mask2.h"
#include "rw/math/fpu/detail/float_intrinsic_operation.h"


namespace rw
{
namespace math
{
namespace fpu
{

//! Returns the element-wise boolean 'Not' of the mask.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> Not(const Mask2Template<T> &a)
{
    return Mask2Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, T(-1.0f), T(1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mY, T(-1.0f), T(1.0f) ))
            );
}

//! Returns the element-wise boolean 'And' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> And(const Mask2Template<T> &a, const Mask2Template<T> &b)
{
    return Mask2Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, b.mX, T(-1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mY, b.mY, T(-1.0f) ))
            );
}

//! Returns the element-wise boolean 'Or' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> Or(const Mask2Template<T> &a, const Mask2Template<T> &b)
{
    return Mask2Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, a.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, a.mY, b.mY ))
            );
}

//! Returns the element-wise boolean 'Xor' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> Xor(const Mask2Template<T> &a, const Mask2Template<T> &b)
{
    Mask2Template<T> notB(Not(b));
    return Mask2Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, notB.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, notB.mY, b.mY ))
            );
}

//! Compare's two Mask2 types element-wise and returns a mask indicating whether or not the masks are equal.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompEqual(const Mask2Template<T> &a, const Mask2Template<T> &b)
{
    Mask2Template<T> notB(Not(b));
    return Mask2Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, b.mX, notB.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, b.mY, notB.mY ))
            );
}

//! Compare's two Mask2 types element-wise and returns a mask indicating whether or not the masks are not equal.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> CompNotEqual(const Mask2Template<T> &a, const Mask2Template<T> &b)
{
    Mask2Template<T> notB(Not(b));
    return Mask2Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, notB.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, notB.mY, b.mY ))
            );
}

// Returns a MaskScalar indicating if any of the elements of the mask are true.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompAnyTrue(const Mask2Template<T> &a)
{
    // These operations are structured to reduce instruction level data dependencies.
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mX, a.mX, a.mY )));
}

// Returns a MaskScalar indicating if all of the elements of the mask are true.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompAllTrue(const Mask2Template<T> &a)
{
    // These operations are structured to reduce instruction level data dependencies.
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( a.mX, a.mY, T(-1.0f) )));
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> Select(const MaskScalarTemplate<T> &mask, const Mask2Template<T> &trueValue, const Mask2Template<T> &falseValue )
{
    return Mask2Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mX, falseValue.mX ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mY, falseValue.mY )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Mask2Template<T> Select(const Mask2Template<T> &mask, const Mask2Template<T> &trueValue, const Mask2Template<T> &falseValue )
{
    return Mask2Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.mX, falseValue.mX ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.mY, falseValue.mY )
        );
}

//! Returns mask2 where the elements are rotated to the left by one.
template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> RotateLeft(const Mask2Template<T> &a)
{
	return Mask2Template<T>(a.mY, a.mX);
}

template<class T>
RW_MATH_FORCE_INLINE Mask2Template<T> RotateRight(const Mask2Template<T> &a)
{
	return Mask2Template<T>(a.mY, a.mX);
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASK2_OPERATION_INLINE_H
