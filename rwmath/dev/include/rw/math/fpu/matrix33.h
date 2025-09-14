// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX33_H
#define RW_MATH_FPU_MATRIX33_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vec_float.h"
#include "rw/math/types_fwd.h"

namespace rw {
namespace math {
namespace fpu {

template <class T>
class Matrix33Template
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
    enum { NumberOfCols = 3 };
    enum { NumberOfElement = 9 };
    
    typedef const Matrix33Template& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix33Template& OutParam;
        typedef Matrix33Template& InOutParam;
    #endif
    
    RW_MATH_INLINE void Set(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8)
    {
        xAxis.Set( m0,  m1,  m2);
        yAxis.Set( m3,  m4,  m5);
        zAxis.Set( m6,  m7,  m8);
    }

    
    RW_MATH_INLINE void Set(const Vector3Template<T>& row0, const Vector3Template<T>& row1, const Vector3Template<T>& row2)
    {
        xAxis = row0;
        yAxis = row1;
        zAxis = row2;
    }

    
    RW_MATH_FORCE_INLINE Matrix33Template(const Matrix33Template<float> &m) 
    {
        // Individual elements are assigned to improve compiler code generation on Wii.
        xAxis.x = static_cast<T>(m.xAxis.x);
        xAxis.y = static_cast<T>(m.xAxis.y);
        xAxis.z = static_cast<T>(m.xAxis.z);
        yAxis.x = static_cast<T>(m.yAxis.x);
        yAxis.y = static_cast<T>(m.yAxis.y);
        yAxis.z = static_cast<T>(m.yAxis.z);
        zAxis.x = static_cast<T>(m.zAxis.x);
        zAxis.y = static_cast<T>(m.zAxis.y);
        zAxis.z = static_cast<T>(m.zAxis.z);
    }

    
    RW_MATH_FORCE_INLINE Matrix33Template(const Matrix33Template<double> &m) 
    {
        // Individual elements are assigned to improve compiler code generation on Wii.
        xAxis.x = static_cast<T>(m.xAxis.x);
        xAxis.y = static_cast<T>(m.xAxis.y);
        xAxis.z = static_cast<T>(m.xAxis.z);
        yAxis.x = static_cast<T>(m.yAxis.x);
        yAxis.y = static_cast<T>(m.yAxis.y);
        yAxis.z = static_cast<T>(m.yAxis.z);
        zAxis.x = static_cast<T>(m.zAxis.x);
        zAxis.y = static_cast<T>(m.zAxis.y);
        zAxis.z = static_cast<T>(m.zAxis.z);
    }

    
    RW_MATH_INLINE explicit Matrix33Template( const rw::math::vpu::Matrix33& m);

    
    RW_MATH_FORCE_INLINE Matrix33Template()
    {
    }

    
    RW_MATH_FORCE_INLINE Matrix33Template(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8)
    {
        Set(m0,  m1,  m2, m3,  m4,  m5, m6,  m7,  m8);
    }

    
    RW_MATH_INLINE explicit Matrix33Template(const T m[9])
    {
        Set(
            m[0], m[1], m[2],
            m[3], m[4], m[5],
            m[6], m[7], m[8]
            );
    }

    
    RW_MATH_INLINE Matrix33Template(const Vector3Template<T>& row0, const Vector3Template<T>& row1, const Vector3Template<T>& row2)
    {
        xAxis = row0;
        yAxis = row1;
        zAxis = row2;
    }

    
    RW_MATH_INLINE void SetZero()
    {
        Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
    }

    
    RW_MATH_INLINE void SetIdentity()
    {
        Set(static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f),
            static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
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
        return GET_ROW_COL( (i)/3, (i)%3 );
    }
        
    
    RW_MATH_INLINE const T& GetElem(int i) const
    {
        return GET_ROW_COL( (i)/3, (i)%3 );
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

    
    RW_MATH_INLINE Vector3Template<T>& GetRow(int i)
    {
        return GET_ROW(i);
    }

    
    RW_MATH_INLINE const Vector3Template<T>& GetRow(int i) const
    {
        return GET_ROW(i);
    }

    
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetX() { return xAxis; }    
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetY() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetZ() { return zAxis; }

    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetX() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetY() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetZ() const { return zAxis; }

    RW_MATH_FORCE_INLINE Vector3Template<T>& XAxis() { return xAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& YAxis() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& ZAxis() { return zAxis; }
    
    RW_MATH_FORCE_INLINE const Vector3Template<T>& XAxis() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& YAxis() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& ZAxis() const { return zAxis; }
    
    RW_MATH_FORCE_INLINE Vector3Template<T>& Right() { return xAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& Up() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& At() { return zAxis; }

    RW_MATH_FORCE_INLINE const Vector3Template<T>& Right() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& Up() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& At() const { return zAxis; }
    
    RW_MATH_INLINE Vector3Template<T> GetXColumn() const
    {
        return Vector3Template<T>( xAxis.x, yAxis.x, zAxis.x );
    }
    
    RW_MATH_INLINE Vector3Template<T> GetYColumn() const
    {
        return Vector3Template<T>( xAxis.y, yAxis.y, zAxis.y );
    }
    
    RW_MATH_INLINE Vector3Template<T> GetZColumn() const
    {
        return Vector3Template<T>( xAxis.z, yAxis.z, zAxis.z );
    }
    
    RW_MATH_INLINE void SetXColumn(const Vector3Template<T> &col)
    {
        xAxis.x = col.x;
        yAxis.x = col.y;
        zAxis.x = col.z;
    }
    
    RW_MATH_INLINE void SetYColumn(const Vector3Template<T> &col)
    {
        xAxis.y = col.x;
        yAxis.y = col.y;
        zAxis.y = col.z;
    }
    
    RW_MATH_INLINE void SetZColumn(const Vector3Template<T> &col)
    {
        xAxis.z = col.x;
        yAxis.z = col.y;
        zAxis.z = col.z;
    }
    
    RW_MATH_INLINE void  SetElem(int i, T f)
    {
        GET_ROW_COL(i/3, i%3) = f;
    }

    
    RW_MATH_INLINE void  SetElem(int row, int column, T f)
    {
        GET_ROW_COL(row, column) = f;
    }

    
    RW_MATH_INLINE void  SetRow(int i, const Vector3Template<T>& v)
    {
        GET_ROW(i) = v;
    }

    
    RW_MATH_INLINE void SetX(const Vector3Template<T>& v) { xAxis = v; }    
    RW_MATH_INLINE void SetY(const Vector3Template<T>& v) { yAxis = v; }
    RW_MATH_INLINE void SetZ(const Vector3Template<T>& v) { zAxis = v; }


public:

    Vector3Template<T>   xAxis;
    Vector3Template<T>   yAxis;
    Vector3Template<T>   zAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector3Template<T> Matrix33Template<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> Vector3Template<float> Matrix33Template<float>::* const Matrix33Template<float>::sElements[];
    template<> Vector3Template<double> Matrix33Template<double>::* const Matrix33Template<double>::sElements[];
#endif

typedef Matrix33Template<float>   Matrix33;
typedef Matrix33Template<float>   Matrix33_32;
typedef Matrix33Template<double>  Matrix33_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef Matrix33Template<float>   Matrix33U_32;
    //! \internal
    typedef Matrix33Template<double>  Matrix33U_64;
#endif

}}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::Matrix33Template<T>& t)
{
    EndianSwap(t.xAxis);
    EndianSwap(t.yAxis);
    EndianSwap(t.zAxis);
}                 

}

#undef GET_ROW
#undef GET_ROW_COL

#endif
