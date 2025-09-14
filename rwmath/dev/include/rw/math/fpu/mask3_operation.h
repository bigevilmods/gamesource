// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASK3_OPERATION_INLINE_H
#define RW_MATH_FPU_MASK3_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/mask3.h"
#include "rw/math/fpu/detail/float_intrinsic_operation.h"


namespace rw
{
namespace math
{
namespace fpu
{

//! Returns the element-wise boolean 'Not' of the mask.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> Not(const Mask3Template<T> &a)
{
    return Mask3Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, T(-1.0f), T(1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mY, T(-1.0f), T(1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mZ, T(-1.0f), T(1.0f) ))
            );
}

//! Returns the element-wise boolean 'And' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> And(const Mask3Template<T> &a, const Mask3Template<T> &b)
{
    return Mask3Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, b.mX, T(-1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mY, b.mY, T(-1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mZ, b.mZ, T(-1.0f) ))
            );
}

//! Returns the element-wise boolean 'Or' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> Or(const Mask3Template<T> &a, const Mask3Template<T> &b)
{
    return Mask3Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, a.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, a.mY, b.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, a.mZ, b.mZ ))
            );
}

//! Returns the element-wise boolean 'Xor' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> Xor(const Mask3Template<T> &a, const Mask3Template<T> &b)
{
    Mask3Template<T> notB(Not(b));
    return Mask3Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, notB.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, notB.mY, b.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, notB.mZ, b.mZ ))
            );
}

//! Compare's two Mask3 types element-wise and returns a mask indicating whether or not the masks are equal.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompEqual(const Mask3Template<T> &a, const Mask3Template<T> &b)
{
    Mask3Template<T> notB(Not(b));
    return Mask3Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, b.mX, notB.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, b.mY, notB.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, b.mZ, notB.mZ ))
            );
}

//! Compare's two Mask3 types element-wise and returns a mask indicating whether or not the masks are not equal.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> CompNotEqual(const Mask3Template<T> &a, const Mask3Template<T> &b)
{
    Mask3Template<T> notB(Not(b));
    return Mask3Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, notB.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, notB.mY, b.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, notB.mZ, b.mZ ))
            );
}

// Returns a MaskScalar indicating if any of the elements of the mask are true.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompAnyTrue(const Mask3Template<T> &a)
{
    // These operations are structured to reduce instruction level data dependencies.
    T tempResult1 = T(RW_MATH_FPU_Fsel( a.mX, a.mX, a.mY ));
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( tempResult1, tempResult1, a.mZ )));
}

// Returns a MaskScalar indicating if all of the elements of the mask are true.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompAllTrue(const Mask3Template<T> &a)
{
    // These operations are structured to reduce instruction level data dependencies.
    T tempResult1 = T(RW_MATH_FPU_Fsel( a.mX, a.mY, T(-1.0f) ));
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( tempResult1, a.mZ, T(-1.0f) )));
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> Select(const MaskScalarTemplate<T> &mask, const Mask3Template<T> &trueValue, const Mask3Template<T> &falseValue )
{
    return Mask3Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mX, falseValue.mX ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mY, falseValue.mY ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mZ, falseValue.mZ )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Mask3Template<T> Select(const Mask3Template<T> &mask, const Mask3Template<T> &trueValue, const Mask3Template<T> &falseValue )
{
    return Mask3Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.mX, falseValue.mX ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.mY, falseValue.mY ),
            (T)RW_MATH_FPU_Fsel( mask.mZ, trueValue.mZ, falseValue.mZ )
        );
}

//! Returns mask3 where the elements are rotated to the left by one.
template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> RotateLeft(const Mask3Template<T> &a)
{
	return Mask3Template<T>(a.mY, a.mZ, a.mX);
}

template<class T>
RW_MATH_FORCE_INLINE Mask3Template<T> RotateRight(const Mask3Template<T> &a)
{
    return Mask3Template<T>(a.mZ, a.mX, a.mY);
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASK3_OPERATION_INLINE_H
