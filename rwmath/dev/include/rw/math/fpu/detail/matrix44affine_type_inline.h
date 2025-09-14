// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX44AFFINE_TYPE_INLINE_H
#define RW_MATH_FPU_MATRIX44AFFINE_TYPE_INLINE_H

// Usually the implementation for fpu class members is contained inside the class
// declaration.  However, this can sometimes lead to cyclic dependencies, so sometimes
// separate implementation files are required.

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/matrix44affine.h"
#include "rw/math/fpu/matrix44affinepacked.h"
#include "rw/math/fpu/matrix44.h"

namespace rw
{
namespace math
{
namespace fpu
{

    template <class T>
    template <class INPUT_T>
    RW_MATH_INLINE Matrix44AffineTemplate<T>::Matrix44AffineTemplate(const Matrix44AffinePackedTemplate<INPUT_T> &m) 
        :xAxis(Vector3Template<T>(static_cast<T>(m.xAxis.x), static_cast<T>(m.yAxis.x), static_cast<T>(m.zAxis.x)))
        ,yAxis(Vector3Template<T>(static_cast<T>(m.xAxis.y), static_cast<T>(m.yAxis.y), static_cast<T>(m.zAxis.y)))
        ,zAxis(Vector3Template<T>(static_cast<T>(m.xAxis.z), static_cast<T>(m.yAxis.z), static_cast<T>(m.zAxis.z)))
        ,wAxis(Vector3Template<T>(static_cast<T>(m.xAxis.w), static_cast<T>(m.yAxis.w), static_cast<T>(m.zAxis.w)))
    {
    }

    template <class T>
    template <class INPUT_T>
    RW_MATH_INLINE Matrix44AffineTemplate<T>::Matrix44AffineTemplate(const Matrix44Template<INPUT_T> &m) 
        :xAxis(Vector3Template<T>(static_cast<T>(m.xAxis.x), static_cast<T>(m.xAxis.y), static_cast<T>(m.xAxis.z)))
        ,yAxis(Vector3Template<T>(static_cast<T>(m.yAxis.x), static_cast<T>(m.yAxis.y), static_cast<T>(m.yAxis.z)))
        ,zAxis(Vector3Template<T>(static_cast<T>(m.zAxis.x), static_cast<T>(m.zAxis.y), static_cast<T>(m.zAxis.z)))
        ,wAxis(Vector3Template<T>(static_cast<T>(m.wAxis.x), static_cast<T>(m.wAxis.y), static_cast<T>(m.wAxis.z)))
    {
        RW_MATH_ASSERT_MSG(
                IsSimilar(m.xAxis.w, static_cast<T>(0.0f)) &&
                IsSimilar(m.yAxis.w, static_cast<T>(0.0f)) &&
                IsSimilar(m.zAxis.w, static_cast<T>(0.0f)) &&
                IsSimilar(m.wAxis.w, static_cast<T>(1.0f)),
                ("The input matrix is not an affine transformation.") );
    }


} // namespace fpu
} // namespace math
} // namespace rw

#include "rw/math/fpu/detail/matrix44affine_type_inline.h"

#endif
