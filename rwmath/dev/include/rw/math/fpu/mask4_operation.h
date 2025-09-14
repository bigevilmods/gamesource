// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MASK4_OPERATION_INLINE_H
#define RW_MATH_FPU_MASK4_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/mask4.h"
#include "rw/math/fpu/detail/float_intrinsic_operation.h"


namespace rw
{
namespace math
{
namespace fpu
{

//! Returns the element-wise boolean 'Not' of the mask.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> Not(const Mask4Template<T> &a)
{
    return Mask4Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, T(-1.0f), T(1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mY, T(-1.0f), T(1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mZ, T(-1.0f), T(1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mW, T(-1.0f), T(1.0f) ))
            );
}

//! Returns the element-wise boolean 'And' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> And(const Mask4Template<T> &a, const Mask4Template<T> &b)
{
    return Mask4Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, b.mX, T(-1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mY, b.mY, T(-1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mZ, b.mZ, T(-1.0f) )),
            T(RW_MATH_FPU_Fsel( a.mW, b.mW, T(-1.0f) ))
            );
}

//! Returns the element-wise boolean 'Or' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> Or(const Mask4Template<T> &a, const Mask4Template<T> &b)
{
    return Mask4Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, a.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, a.mY, b.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, a.mZ, b.mZ )),
            T(RW_MATH_FPU_Fsel( a.mW, a.mW, b.mW ))
            );
}

//! Returns the element-wise boolean 'Xor' of the masks.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> Xor(const Mask4Template<T> &a, const Mask4Template<T> &b)
{
    Mask4Template<T> notB(Not(b));
    return Mask4Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, notB.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, notB.mY, b.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, notB.mZ, b.mZ )),
            T(RW_MATH_FPU_Fsel( a.mW, notB.mW, b.mW ))
            );
}

//! Compare's two Mask4 types element-wise and returns a mask indicating whether or not the masks are equal.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompEqual(const Mask4Template<T> &a, const Mask4Template<T> &b)
{
    Mask4Template<T> notB(Not(b));
    return Mask4Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, b.mX, notB.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, b.mY, notB.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, b.mZ, notB.mZ )),
            T(RW_MATH_FPU_Fsel( a.mW, b.mW, notB.mW ))
            );
}

//! Compare's two Mask4 types element-wise and returns a mask indicating whether or not the masks are not equal.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> CompNotEqual(const Mask4Template<T> &a, const Mask4Template<T> &b)
{
    Mask4Template<T> notB(Not(b));
    return Mask4Template<T>(
            T(RW_MATH_FPU_Fsel( a.mX, notB.mX, b.mX )),
            T(RW_MATH_FPU_Fsel( a.mY, notB.mY, b.mY )),
            T(RW_MATH_FPU_Fsel( a.mZ, notB.mZ, b.mZ )),
            T(RW_MATH_FPU_Fsel( a.mW, notB.mW, b.mW ))
            );
}

// Returns a MaskScalar indicating if any of the elements of the mask are true.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompAnyTrue(const Mask4Template<T> &a)
{
    // These operations are structured to reduce instruction level data dependencies.
    T tempResult1 = T(RW_MATH_FPU_Fsel( a.mX, a.mX, a.mY ));
    T tempResult2 = T(RW_MATH_FPU_Fsel( a.mZ, a.mZ, a.mW ));
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( tempResult1, tempResult1, tempResult2 )));
}

// Returns a MaskScalar indicating if all of the elements of the mask are true.
template<class T>
RW_MATH_FORCE_INLINE MaskScalarTemplate<T> CompAllTrue(const Mask4Template<T> &a)
{
    // These operations are structured to reduce instruction level data dependencies.
    T tempResult1 = T(RW_MATH_FPU_Fsel( a.mX, a.mY, T(-1.0f) ));
    T tempResult2 = T(RW_MATH_FPU_Fsel( a.mZ, a.mW, T(-1.0f) ));
    return MaskScalarTemplate<T>(T(RW_MATH_FPU_Fsel( tempResult1, tempResult2, T(-1.0f) )));
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> Select(const MaskScalarTemplate<T> &mask, const Mask4Template<T> &trueValue, const Mask4Template<T> &falseValue )
{
    return Mask4Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mX, falseValue.mX ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mY, falseValue.mY ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mZ, falseValue.mZ ),
            (T)RW_MATH_FPU_Fsel( mask.mS, trueValue.mW, falseValue.mW )
        );
}

template <class T>
RW_MATH_FORCE_INLINE Mask4Template<T> Select(const Mask4Template<T> &mask, const Mask4Template<T> &trueValue, const Mask4Template<T> &falseValue )
{
    return Mask4Template<T>(
            (T)RW_MATH_FPU_Fsel( mask.mX, trueValue.mX, falseValue.mX ),
            (T)RW_MATH_FPU_Fsel( mask.mY, trueValue.mY, falseValue.mY ),
            (T)RW_MATH_FPU_Fsel( mask.mZ, trueValue.mZ, falseValue.mZ ),
            (T)RW_MATH_FPU_Fsel( mask.mW, trueValue.mW, falseValue.mW )
        );
}

//! Returns mask4 where the elements are rotated to the left by one.
template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> RotateLeft(const Mask4Template<T> &a)
{
	return Mask4Template<T>(a.mY, a.mZ, a.mW, a.mX);
}

template<class T>
RW_MATH_FORCE_INLINE Mask4Template<T> RotateRight(const Mask4Template<T> &a)
{
    return Mask4Template<T>(a.mW, a.mX, a.mY, a.mZ);
}

} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_MASK4_OPERATION_INLINE_H
