// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX44AFFINE_PACKED_H
#define RW_MATH_FPU_MATRIX44AFFINE_PACKED_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/vec_float.h"
#include "rw/math/types_fwd.h"

#ifdef RW_MATH_ENABLE_ASSERTS
// Currently scalar.h is only needed for some assertion code.
#include "rw/math/fpu/scalar.h"
#endif

namespace rw {
namespace math {
namespace fpu {

template <class T>
class Matrix44AffinePackedTemplate
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
    enum { NumberOfRows = 3 };
    enum { NumberOfCols = 4 };
    enum { NumberOfElement = 12 };
    
    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(){}
    RW_MATH_FORCE_INLINE explicit Matrix44AffinePackedTemplate( const rw::math::vpu::Matrix44AffinePacked& m);

    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(const Matrix44AffinePackedTemplate<float>& m)
    {
        this->xAxis = m.xAxis;
        this->yAxis = m.yAxis;
        this->zAxis = m.zAxis;
    }

    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(const Matrix44AffinePackedTemplate<double>& m)
    {
        this->xAxis = m.xAxis;
        this->yAxis = m.yAxis;
        this->zAxis = m.zAxis;
    }

    
    template <class INPUT_T>
    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(const Matrix44AffineTemplate<INPUT_T>& m)
        :xAxis(Vector4Template<T>(static_cast<T>(m.xAxis.x), static_cast<T>(m.yAxis.x), static_cast<T>(m.zAxis.x), static_cast<T>(m.wAxis.x)))
        ,yAxis(Vector4Template<T>(static_cast<T>(m.xAxis.y), static_cast<T>(m.yAxis.y), static_cast<T>(m.zAxis.y), static_cast<T>(m.wAxis.y)))
        ,zAxis(Vector4Template<T>(static_cast<T>(m.xAxis.z), static_cast<T>(m.yAxis.z), static_cast<T>(m.zAxis.z), static_cast<T>(m.wAxis.z)))
    {
    }

    
    template <class INPUT_T>
    explicit RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(const Matrix44Template<INPUT_T>& m)
        :xAxis(Vector4Template<T>(static_cast<T>(m.xAxis.x), static_cast<T>(m.yAxis.x), static_cast<T>(m.zAxis.x), static_cast<T>(m.wAxis.x)))
        ,yAxis(Vector4Template<T>(static_cast<T>(m.xAxis.y), static_cast<T>(m.yAxis.y), static_cast<T>(m.zAxis.y), static_cast<T>(m.wAxis.y)))
        ,zAxis(Vector4Template<T>(static_cast<T>(m.xAxis.z), static_cast<T>(m.yAxis.z), static_cast<T>(m.zAxis.z), static_cast<T>(m.wAxis.z)))
    {
        RW_MATH_ASSERT_MSG(
                IsSimilar(m.xAxis.w, static_cast<T>(0.0f)) &&
                IsSimilar(m.yAxis.w, static_cast<T>(0.0f)) &&
                IsSimilar(m.zAxis.w, static_cast<T>(0.0f)) &&
                IsSimilar(m.wAxis.w, static_cast<T>(1.0f)),
                ("The input matrix is not an affine transformation.") );
    }

    
    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(const Vector3Template<T>& row0, const Vector3Template<T>& row1, const Vector3Template<T>& row2, const Vector3Template<T>& row3)
    {
        this->xAxis.Set(row0.x, row1.x, row2.x, row3.x);
        this->yAxis.Set(row0.y, row1.y, row2.y, row3.y);
        this->zAxis.Set(row0.z, row1.z, row2.z, row3.z);
    }

    
    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(T  m0,  T  m1,  T  m2, T  m3,  T  m4,  T  m5,
                                                           T  m6,  T  m7,  T  m8, T  m9,  T m10,  T  m11)
    {
        this->xAxis.Set(m0, m3, m6,  m9);
        this->yAxis.Set(m1, m4, m7, m10);
        this->zAxis.Set(m2, m5, m8, m11);
    }

    
    RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate(const T m[12])
    {
        this->xAxis.Set(m[0], m[3], m[6],  m[9]);
        this->yAxis.Set(m[1], m[4], m[7], m[10]);
        this->zAxis.Set(m[2], m[5], m[8], m[11]);
    }

    
    RW_MATH_FORCE_INLINE void SetZero()
    {
        this->xAxis.SetZero();
        this->yAxis.SetZero();
        this->zAxis.SetZero();
    }

    
    RW_MATH_FORCE_INLINE void Set(T  m0,  T  m1,  T  m2, T  m3,  T  m4,  T  m5,
                                       T  m6,  T  m7,  T  m8, T  m9,  T m10,  T  m11)
    {
        this->xAxis.Set(m0, m3, m6,  m9);
        this->yAxis.Set(m1, m4, m7, m10);
        this->zAxis.Set(m2, m5, m8, m11);
    }

    
    RW_MATH_FORCE_INLINE void Set(const Vector3Template<T>& row0, const Vector3Template<T>& row1, const Vector3Template<T>& row2, const Vector3Template<T>& row3)
    {
        this->xAxis.Set(static_cast<T>(row0.x), static_cast<T>(row1.x), static_cast<T>(row2.x), static_cast<T>(row3.x));
        this->yAxis.Set(static_cast<T>(row0.y), static_cast<T>(row1.y), static_cast<T>(row2.y), static_cast<T>(row3.y));
        this->zAxis.Set(static_cast<T>(row0.z), static_cast<T>(row1.z), static_cast<T>(row2.z), static_cast<T>(row3.z));
    }

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    #define GET_ROW_COL(row, col) \
        (this->*sElements[col]).GetComponent(row)       /* Note: col and row are swapped (compared to standard matrices) since the packed matrix is transposed. */
#else
    #define GET_ROW_COL(row, col) \
        (&((&xAxis)[col]).x)[row]                       /* Note: col and row are swapped (compared to standard matrices) since the packed matrix is transposed. */
#endif
    
    
    RW_MATH_FORCE_INLINE T& GetElem(int row, int column)
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL(row, column);
    }

    
    RW_MATH_FORCE_INLINE const T& GetElem(int row, int column) const
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL(row, column);
    }

    
    RW_MATH_FORCE_INLINE T& GetElem(int i)
    {
        const int row = i / 3;
        const int column = i % 3;
        return GetElem( row, column );
    }
    
    RW_MATH_FORCE_INLINE const T& GetElem(int i) const
    {
        const int row = i / 3;
        const int column = i % 3;
        return GetElem( row, column );
    }

    
    RW_MATH_FORCE_INLINE T& operator () (int row, int column)
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL(row, column);
    }

    
    RW_MATH_FORCE_INLINE const T& operator () (int row, int column) const
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL(row, column);
    }

    
    RW_MATH_FORCE_INLINE void SetElem(int row, int column, const T& f)
    {
        GET_ROW_COL(row, column) = f;
    }

    
    RW_MATH_FORCE_INLINE void SetElem(int i, const T& f)
    {
        const int row = i / 3;
        const int column = i % 3;
        GET_ROW_COL(row, column) = f;
    }

    Vector4Template<T>     xAxis;
    Vector4Template<T>     yAxis;
    Vector4Template<T>     zAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector4Template<T> Matrix44AffinePackedTemplate<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> Vector4Template<float> Matrix44AffinePackedTemplate<float>::* const Matrix44AffinePackedTemplate<float>::sElements[];
    template<> Vector4Template<double> Matrix44AffinePackedTemplate<double>::* const Matrix44AffinePackedTemplate<double>::sElements[];
#endif

typedef Matrix44AffinePackedTemplate<float>   Matrix44AffinePacked;
typedef Matrix44AffinePackedTemplate<float>   Matrix44AffinePacked_32;
typedef Matrix44AffinePackedTemplate<double>  Matrix44AffinePacked_64;

//! \internal
//! \todo deprecate
typedef Matrix44AffinePackedTemplate<float>   Matrix44AffinePackedU_32;

//! \internal
//! \todo deprecate
typedef Matrix44AffinePackedTemplate<double>  Matrix44AffinePackedU_64;

}}

    /*!
     *  Warning: This function \e conditionally endian swaps the class's data.
     *
     *  Normally this function is a no-op.  See \ref p_endian_swapping for details
     *  regarding when endian swapping will actually occur.
     */
    template <class T>
    inline void EndianSwap(rw::math::fpu::Matrix44AffinePackedTemplate<T>& t)
    {
        EndianSwap(t.xAxis);
        EndianSwap(t.yAxis);
        EndianSwap(t.zAxis);
    }

}

#undef GET_ROW_COL

#endif
