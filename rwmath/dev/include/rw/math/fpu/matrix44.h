// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX44_H
#define RW_MATH_FPU_MATRIX44_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/types_fwd.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/vec_float.h"

namespace rw {
namespace math {
namespace fpu {


template <class T>
class Matrix44Template
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
    enum { NumberOfRows = 4 };
    enum { NumberOfCols = 4 };
    enum { NumberOfElement = 16 };
    
    typedef const Matrix44Template& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix44Template& OutParam;
        typedef Matrix44Template& InOutParam;
    #endif
    
    RW_MATH_INLINE void Set(T  m0,  T  m1,  T  m2, T  m3, T  m4,  T  m5,  T  m6, T  m7,
             T  m8,  T  m9,  T m10, T m11, T m12,  T m13,  T m14, T m15)
    {
        xAxis.Set( m0,  m1,  m2,  m3);
        yAxis.Set( m4,  m5,  m6,  m7);
        zAxis.Set( m8,  m9, m10, m11);
        wAxis.Set(m12, m13, m14, m15);
    }

    
    RW_MATH_INLINE void Set(const Vector4Template<T>& row0,
             const Vector4Template<T>& row1,
             const Vector4Template<T>& row2,
             const Vector4Template<T>& row3)
    {
        xAxis = row0;
        yAxis = row1;
        zAxis = row2;
        wAxis = row3;
    }

    
    RW_MATH_FORCE_INLINE Matrix44Template(const Matrix44Template<float> &m)
    {
        // Individual elements are assigned to improve compiler code generation on Wii.
        xAxis.x = static_cast<T>(m.xAxis.x);
        xAxis.y = static_cast<T>(m.xAxis.y);
        xAxis.z = static_cast<T>(m.xAxis.z);
        xAxis.w = static_cast<T>(m.xAxis.w);
        yAxis.x = static_cast<T>(m.yAxis.x);
        yAxis.y = static_cast<T>(m.yAxis.y);
        yAxis.z = static_cast<T>(m.yAxis.z);
        yAxis.w = static_cast<T>(m.yAxis.w);
        zAxis.x = static_cast<T>(m.zAxis.x);
        zAxis.y = static_cast<T>(m.zAxis.y);
        zAxis.z = static_cast<T>(m.zAxis.z);
        zAxis.w = static_cast<T>(m.zAxis.w);
        wAxis.x = static_cast<T>(m.wAxis.x);
        wAxis.y = static_cast<T>(m.wAxis.y);
        wAxis.z = static_cast<T>(m.wAxis.z);
        wAxis.w = static_cast<T>(m.wAxis.w);
    }

    
    RW_MATH_INLINE Matrix44Template(const Matrix44Template<double> &m) 
    {
        // Individual elements are assigned to improve compiler code generation on Wii.
        xAxis.x = static_cast<T>(m.xAxis.x);
        xAxis.y = static_cast<T>(m.xAxis.y);
        xAxis.z = static_cast<T>(m.xAxis.z);
        xAxis.w = static_cast<T>(m.xAxis.w);
        yAxis.x = static_cast<T>(m.yAxis.x);
        yAxis.y = static_cast<T>(m.yAxis.y);
        yAxis.z = static_cast<T>(m.yAxis.z);
        yAxis.w = static_cast<T>(m.yAxis.w);
        zAxis.x = static_cast<T>(m.zAxis.x);
        zAxis.y = static_cast<T>(m.zAxis.y);
        zAxis.z = static_cast<T>(m.zAxis.z);
        zAxis.w = static_cast<T>(m.zAxis.w);
        wAxis.x = static_cast<T>(m.wAxis.x);
        wAxis.y = static_cast<T>(m.wAxis.y);
        wAxis.z = static_cast<T>(m.wAxis.z);
        wAxis.w = static_cast<T>(m.wAxis.w);
    }

    template <class INPUT_T>
    explicit RW_MATH_INLINE Matrix44Template(const Matrix44AffineTemplate<INPUT_T> &_v);
    
    template <class INPUT_T>
    explicit RW_MATH_INLINE Matrix44Template(const Matrix44AffinePackedTemplate<INPUT_T> &m);
    
    explicit RW_MATH_INLINE Matrix44Template( const rw::math::vpu::Matrix44& m);

    RW_MATH_FORCE_INLINE Matrix44Template() {}

    
    RW_MATH_FORCE_INLINE Matrix44Template(const Vector4Template<T>& row0,
                                          const Vector4Template<T>& row1,
                                          const Vector4Template<T>& row2,
                                          const Vector4Template<T>& row3)

    {
        this->xAxis = row0;
        this->yAxis = row1;
        this->zAxis = row2;
        this->wAxis = row3;
    }

    
    RW_MATH_FORCE_INLINE Matrix44Template(T  m0,  T  m1,  T  m2, T  m3, T  m4,  T  m5,  T  m6, T  m7,
                     T  m8,  T  m9,  T m10, T m11, T m12,  T m13,  T m14, T m15)
    {
        Set(m0,  m1,  m2,  m3, m4,  m5,  m6,  m7,
            m8,  m9, m10, m11, m12, m13, m14, m15);
    }

    
    explicit RW_MATH_INLINE Matrix44Template(const T m[16])
    {
        Set(m[0],  m[1],  m[2],  m[3], m[4],  m[5],  m[6],  m[7],
            m[8],  m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
    }

    RW_MATH_FORCE_INLINE Matrix44Template& operator = (const Matrix44Template<T> &m)
    {
        this->xAxis = m.xAxis;
        this->yAxis = m.yAxis;
        this->zAxis = m.zAxis;
        this->wAxis = m.wAxis;
        return *this;
    }
    
    RW_MATH_INLINE void SetZero()
    {
        Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
    }

    
    RW_MATH_INLINE void SetIdentity()
    {
        Set(static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
    }

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    #define GET_ROW(row) \
        (this->*sElements[row])

    #define GET_ROW_COL(row, col) \
        (this->*sElements[row]).GetComponent(col)
#else
    #define GET_ROW(row) \
        (&xAxis)[row]

    #define GET_ROW_COL(row, col) \
        (&(GET_ROW(row)).x)[col]
#endif
    
    RW_MATH_INLINE T& GetElem(int i)
    {
        return GET_ROW_COL( (i)/4, (i)%4 );
    }

    
    RW_MATH_INLINE const T& GetElem(int i) const
    {
        return GET_ROW_COL( (i)/4, (i)%4 );
    }

    
    RW_MATH_INLINE T& GetElem(int row, int column)
    {
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_INLINE const T& GetElem(int row, int column) const
    {
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_INLINE T& operator () (int row, int column)
    {
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_INLINE const T& operator () (int row, int column) const
    {
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_INLINE Vector4Template<T>& GetRow(int i)
    {
        return GET_ROW(i);
    }

    
    RW_MATH_INLINE const Vector4Template<T>& GetRow(int i) const
    {
        return GET_ROW(i);
    }

    
    RW_MATH_FORCE_INLINE Vector4Template<T>& GetX() { return xAxis; }    
    RW_MATH_FORCE_INLINE Vector4Template<T>& GetY() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& GetZ() { return zAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& GetW() { return wAxis; }
    
    RW_MATH_FORCE_INLINE const Vector4Template<T>& GetX() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& GetY() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& GetZ() const { return zAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& GetW() const { return wAxis; }
    
    RW_MATH_FORCE_INLINE Vector4Template<T>& XAxis() { return xAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& YAxis() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& ZAxis() { return zAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& WAxis() { return wAxis; }

    RW_MATH_FORCE_INLINE const Vector4Template<T>& XAxis() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& YAxis() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& ZAxis() const { return zAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& WAxis() const { return wAxis; }

    RW_MATH_FORCE_INLINE Vector4Template<T>& Right() { return xAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& Up() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& At() { return zAxis; }
    RW_MATH_FORCE_INLINE Vector4Template<T>& Pos() { return wAxis; }

    RW_MATH_FORCE_INLINE const Vector4Template<T>& Right() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& Up() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& At() const { return zAxis; }
    RW_MATH_FORCE_INLINE const Vector4Template<T>& Pos() const { return wAxis; }

    RW_MATH_INLINE Vector4Template<T> GetXColumn() const
    {
        return Vector4Template<T>( xAxis.x, yAxis.x, zAxis.x, wAxis.x );
    }
    
    RW_MATH_INLINE Vector4Template<T> GetYColumn() const
    {
        return Vector4Template<T>( xAxis.y, yAxis.y, zAxis.y, wAxis.y );
    }
    
    RW_MATH_INLINE Vector4Template<T> GetZColumn() const
    {
        return Vector4Template<T>( xAxis.z, yAxis.z, zAxis.z, wAxis.z );
    }
    
    RW_MATH_INLINE Vector4Template<T> GetWColumn() const
    {
        return Vector4Template<T>( xAxis.w, yAxis.w, zAxis.w, wAxis.w );
    }

    RW_MATH_INLINE void SetXColumn(const Vector4Template<T> &col)
    {
        xAxis.x = col.x;
        yAxis.x = col.y;
        zAxis.x = col.z;
        wAxis.x = col.w;
    }
    
    RW_MATH_INLINE void SetYColumn(const Vector4Template<T> &col)
    {
        xAxis.y = col.x;
        yAxis.y = col.y;
        zAxis.y = col.z;
        wAxis.y = col.w;
    }
    
    RW_MATH_INLINE void SetZColumn(const Vector4Template<T> &col)
    {
        xAxis.z = col.x;
        yAxis.z = col.y;
        zAxis.z = col.z;
        wAxis.z = col.w;
    }
    
    RW_MATH_INLINE void SetWColumn(const Vector4Template<T> &col)
    {
        xAxis.w = col.x;
        yAxis.w = col.y;
        zAxis.w = col.z;
        wAxis.w = col.w;
    }
    
    RW_MATH_INLINE void SetElem(int i, T f)
    {
        GET_ROW_COL(i/4, i%4) = f;
    }

    
    RW_MATH_INLINE void SetElem(int row, int column, T f)
    {
        GET_ROW_COL(row, column) = f;
    }

    
    RW_MATH_INLINE void SetRow(int i, const Vector4Template<T>& v)
    {
        GET_ROW(i) = v;
    }

    
    RW_MATH_INLINE void SetX(const Vector4Template<T>& v) { xAxis = v; }
    RW_MATH_INLINE void SetY(const Vector4Template<T>& v) { yAxis = v; }
    RW_MATH_INLINE void SetZ(const Vector4Template<T>& v) { zAxis = v; }
    RW_MATH_INLINE void SetW(const Vector4Template<T>& v) { wAxis = v; }

public:

    Vector4Template<T> xAxis;
    Vector4Template<T> yAxis;
    Vector4Template<T> zAxis;
    Vector4Template<T> wAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector4Template<T> Matrix44Template<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> Vector4Template<float> Matrix44Template<float>::* const Matrix44Template<float>::sElements[];
    template<> Vector4Template<double> Matrix44Template<double>::* const Matrix44Template<double>::sElements[];
#endif

typedef Matrix44Template<float>   Matrix44;
typedef Matrix44Template<float>   Matrix44_32;
typedef Matrix44Template<double>  Matrix44_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef Matrix44Template<float>   Matrix44U_32;
    //! \internal
    typedef Matrix44Template<double>  Matrix44U_64;
#endif

}}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::Matrix44Template<T>& t)
{
    EndianSwap(t.xAxis);
    EndianSwap(t.yAxis);
    EndianSwap(t.zAxis);
    EndianSwap(t.wAxis);
}

}

#undef GET_ROW
#undef GET_ROW_COL

#include "rw/math/fpu/detail/matrix44_type_inline.h"

#endif
