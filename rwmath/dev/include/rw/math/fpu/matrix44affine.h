// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_MATRIX44AFFINE_H
#define RW_MATH_FPU_MATRIX44AFFINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/types_fwd.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/quatpos.h"
#include "rw/math/fpu/vec_float.h"

namespace rw
{
namespace math
{
namespace fpu
{

template <class T>
class Matrix44AffineTemplate
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
    enum { NumberOfCols = 3 };
    enum { NumberOfElement = 12 };

    typedef const Matrix44AffineTemplate& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix44AffineTemplate& OutParam;
        typedef Matrix44AffineTemplate& InOutParam;
    #endif
    
    RW_MATH_INLINE void Set(T  m0,  T  m1,  T  m2, T  m3,  T  m4,  T  m5,
             T  m6,  T  m7,  T  m8, T  m9,  T m10,  T m11)
    {
        xAxis.Set( m0,  m1,  m2);
        yAxis.Set( m3,  m4,  m5);
        zAxis.Set( m6,  m7,  m8);
        wAxis.Set( m9, m10, m11);
    }

    
    RW_MATH_INLINE void Set(const Vector3Template<T>& row0, const Vector3Template<T>& row1, const Vector3Template<T>& row2, const Vector3Template<T>& row3)
    {
        xAxis = row0;
        yAxis = row1;
        zAxis = row2;
        wAxis = row3;
    }

    template <class INPUT_T>
    explicit RW_MATH_INLINE Matrix44AffineTemplate(const Matrix44Template<INPUT_T> &m);
    
    RW_MATH_INLINE Matrix44AffineTemplate(const Matrix44AffineTemplate<float> &m) 
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
        wAxis.x = static_cast<T>(m.wAxis.x);
        wAxis.y = static_cast<T>(m.wAxis.y);
        wAxis.z = static_cast<T>(m.wAxis.z);
    }

    
    RW_MATH_INLINE Matrix44AffineTemplate(const Matrix44AffineTemplate<double> &m) 
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
        wAxis.x = static_cast<T>(m.wAxis.x);
        wAxis.y = static_cast<T>(m.wAxis.y);
        wAxis.z = static_cast<T>(m.wAxis.z);
    }
    
    template <class INPUT_T>
    RW_MATH_INLINE Matrix44AffineTemplate(const Matrix44AffinePackedTemplate<INPUT_T> &m);
    
    explicit RW_MATH_INLINE Matrix44AffineTemplate( const rw::math::vpu::Matrix44Affine& m);

    RW_MATH_FORCE_INLINE Matrix44AffineTemplate() {}
    
    
    RW_MATH_INLINE Matrix44AffineTemplate(const Vector3Template<T>& row0, const Vector3Template<T>& row1, const Vector3Template<T>& row2, const Vector3Template<T>& row3)
    {
        xAxis = row0;
        yAxis = row1;
        zAxis = row2;
        wAxis = row3;
    }

    
    RW_MATH_FORCE_INLINE Matrix44AffineTemplate(T  m0,  T  m1,  T  m2, T  m3,  T  m4,  T  m5, 
                           T  m6,  T  m7,  T  m8, T  m9,  T m10,  T m11)
    {
        Set(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
    }

    
    RW_MATH_FORCE_INLINE explicit Matrix44AffineTemplate(const T m[12])
    {
        Set(m[0],  m[1],  m[2],  m[3],
            m[4],  m[5],  m[6],  m[7],
            m[8],  m[9], m[10], m[11]);
    }

    
    RW_MATH_INLINE Matrix44AffineTemplate(const QuaternionTemplate<T>& q, const Vector3Template<T>& pos)
    {
        T xx = q.x * q.x;
        T yy = q.y * q.y;
        T zz = q.z * q.z;
        T xy = q.x * q.y;
        T xz = q.x * q.z;
        T yz = q.y * q.z;
        T wx = q.w * q.x;
        T wy = q.w * q.y;
        T wz = q.w * q.z;

        const T one(1.0f);
        const T two(2.0f);
        Set(
            one-two*(yy+zz), two*(xy+wz),     two*(xz-wy),
            two*(xy-wz),     one-two*(xx+zz), two*(yz+wx),
            two*(xz+wy),     two*(yz-wx),     one-two*(xx+yy),
            pos.x,       pos.y,   pos.z
            );  
    }

    
    RW_MATH_INLINE void SetZero()
    {
        Set(0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f);  
    }

    
    RW_MATH_INLINE void SetIdentity()
    {
        Set(1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 0.0f);
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
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_FORCE_INLINE const T& GetElem(int row, int column) const
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_FORCE_INLINE T& operator () (int row, int column)
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_FORCE_INLINE const T& operator () (int row, int column) const
    {
        RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
        return GET_ROW_COL( row, column );
    }

    
    RW_MATH_FORCE_INLINE Vector3Template<T>&       GetRow(int i)
    {
        return GET_ROW(i);
    }

    
    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetRow(int i) const
    {
        return GET_ROW(i);
    }
    
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetX() { return xAxis; }    
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetY() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetZ() { return zAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& GetW() { return wAxis; }

    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetX() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetY() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetZ() const { return zAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& GetW() const { return wAxis; }

    RW_MATH_FORCE_INLINE Vector3Template<T>& XAxis() { return xAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& YAxis() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& ZAxis() { return zAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& WAxis() { return wAxis; }

    RW_MATH_FORCE_INLINE const Vector3Template<T>& XAxis() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& YAxis() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& ZAxis() const { return zAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& WAxis() const { return wAxis; }

    RW_MATH_FORCE_INLINE Vector3Template<T>& Right() { return xAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& Up() { return yAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& At() { return zAxis; }
    RW_MATH_FORCE_INLINE Vector3Template<T>& Pos() { return wAxis; }

    RW_MATH_FORCE_INLINE const Vector3Template<T>& Right() const { return xAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& Up() const { return yAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& At() const { return zAxis; }
    RW_MATH_FORCE_INLINE const Vector3Template<T>& Pos() const { return wAxis; }
    
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
    
    RW_MATH_INLINE void SetElem(int i, T f)
    {
        GET_ROW_COL(i/3, i%3) = f;
    }

    RW_MATH_INLINE void SetElem(int row, int column, T f)
    {
        GET_ROW_COL(row, column) = f;
    }
    
    RW_MATH_INLINE void  SetRow(int i, const Vector3Template<T>& v)
    {
        GET_ROW(i) = v;
    }

    
    RW_MATH_FORCE_INLINE void SetX(const Vector3Template<T>& v)
    {
        SetRow(0, v);
    }

    
    RW_MATH_FORCE_INLINE void    SetY(const Vector3Template<T>& v)
    {
        SetRow(1, v);
    }

    
    RW_MATH_FORCE_INLINE void    SetZ(const Vector3Template<T>& v)
    {
        SetRow(2, v);
    }

    
    RW_MATH_FORCE_INLINE void    SetW(const Vector3Template<T>& v)
    {
        SetRow(3, v);
    }

public:

    Vector3Template<T>   xAxis;
    Vector3Template<T>   yAxis;
    Vector3Template<T>   zAxis;
    Vector3Template<T>   wAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector3Template<T> Matrix44AffineTemplate<T>::* const sElements[];
#endif
};

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    template<> Vector3Template<float> Matrix44AffineTemplate<float>::* const Matrix44AffineTemplate<float>::sElements[];
    template<> Vector3Template<double> Matrix44AffineTemplate<double>::* const Matrix44AffineTemplate<double>::sElements[];
#endif

typedef Matrix44AffineTemplate<float>   Matrix44Affine;
typedef Matrix44AffineTemplate<float>   Matrix44Affine_32;
typedef Matrix44AffineTemplate<double>  Matrix44Affine_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef Matrix44AffineTemplate<float>   Matrix44AffineU_32;
    //! \internal
    typedef Matrix44AffineTemplate<double>  Matrix44AffineU_64;
#endif

} // namespace fpu
} // namespace math

    /*!
     *  Warning: This function \e conditionally endian swaps the class's data.
     *
     *  Normally this function is a no-op.  See \ref p_endian_swapping for details
     *  regarding when endian swapping will actually occur.
     */
    template <class T>
    inline void EndianSwap(rw::math::fpu::Matrix44AffineTemplate<T>& t)
    {
        EndianSwap(t.xAxis);
        EndianSwap(t.yAxis);
        EndianSwap(t.zAxis);
        EndianSwap(t.wAxis);
    }

} // namespace rw

#undef GET_ROW
#undef GET_ROW_COL

#include "rw/math/fpu/detail/matrix44affine_type_inline.h"

#endif
