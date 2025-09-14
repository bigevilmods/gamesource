// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX44_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX44_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"

namespace rw
{

namespace math
{

namespace vpu
{

#if defined(EA_PLATFORM_REVOLUTION)
namespace detail
{
    void Matrix44Copy(Matrix44& a, const Matrix44& b);
}
RW_MATH_FORCE_INLINE Matrix44& Matrix44::operator = (const Matrix44& m) 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(m, sizeof(VectorIntrinsic));
    detail::Matrix44Copy(*this, m);
    return *this;
}
RW_MATH_FORCE_INLINE Matrix44::Matrix44(const Matrix44& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    detail::Matrix44Copy(*this, m);
}
#else
RW_MATH_FORCE_INLINE Matrix44& Matrix44::operator = (const Matrix44& m) 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(m, sizeof(VectorIntrinsic));
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
    wAxis.mV = m.wAxis.mV;
    return *this;
}
RW_MATH_FORCE_INLINE Matrix44::Matrix44(const Matrix44& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
    wAxis.mV = m.wAxis.mV;
}
#endif

RW_MATH_FORCE_INLINE Vector4& Matrix44::GetRow(int i) 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return this->*sElements[i];
#else
    return (&xAxis)[i];
#endif
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::GetRow(int i) const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return this->*sElements[i];
#else
    return (&xAxis)[i];
#endif
}

RW_MATH_FORCE_INLINE void Matrix44::Set(
                                 float  m0,  float  m1,  float  m2, float  m3,    //  x  row 0
                                 float  m4,  float  m5,  float  m6, float  m7,    //  y  row 1
                                 float  m8,  float  m9,  float m10, float m11,    //  z  row 2
                                 float m12,  float m13,  float m14, float m15     //  w  row 3
                                 ) 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    xAxis.Set( m0,  m1,  m2,  m3);
    yAxis.Set( m4,  m5,  m6,  m7);
    zAxis.Set( m8,  m9, m10, m11);
    wAxis.Set(m12, m13, m14, m15);
}

RW_MATH_FORCE_INLINE void Matrix44::Set(Vector4::InParam row0, Vector4::InParam row1, Vector4::InParam row2, Vector4::InParam row3) 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    xAxis = row0;
    yAxis = row1;
    zAxis = row2;
    wAxis = row3;
}

RW_MATH_FORCE_INLINE void Matrix44::SetZero() 
{
    VectorIntrinsic zero;
    RW_MATH_VPU_GetZero(zero);
    xAxis.mV = zero;
    yAxis.mV = zero;
    zAxis.mV = zero;
    wAxis.mV = zero;
}

RW_MATH_FORCE_INLINE void Matrix44::SetIdentity() 
{
    RW_MATH_VPU_GetI(xAxis.mV);
    RW_MATH_VPU_GetJ(yAxis.mV);
    RW_MATH_VPU_GetK(zAxis.mV);
    RW_MATH_VPU_GetL(wAxis.mV);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44::GetElem(int row, int column) 
{
    return GetRow(row).GetComponent(column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44::GetElem(int row, int column) const 
{
    return GetRow(row).GetComponent(column);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44::GetElem(int i) 
{
    return GetElem( i / 4, i % 4 );
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44::GetElem(int i) const 
{
    return GetElem( i / 4, i % 4 );
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44::operator () (int row, int column) 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44::operator () (int row, int column) const 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::GetX() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::GetY() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::GetZ() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::GetW() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::GetX() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::GetY() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::GetZ() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::GetW() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::XAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::YAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::ZAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::WAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::XAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::YAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::ZAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::WAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::Right() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::Up() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::At() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector4& Matrix44::Pos() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::Right() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::Up() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::At() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector4& Matrix44::Pos() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE Vector4 Matrix44::GetXColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Matrix44::GetYColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractYElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Matrix44::GetZColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractZElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Matrix44::GetWColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractWElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}


RW_MATH_FORCE_INLINE void Matrix44::SetXColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisX, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisX, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisX, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisX, v.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Matrix44::SetYColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisY, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisY, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisY, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisY, v.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Matrix44::SetZColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisZ, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisZ, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisZ, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisZ, v.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Matrix44::SetWColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisW, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisW, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisW, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisW, v.mV, VectorAxisW );
}


RW_MATH_FORCE_INLINE void Matrix44::SetElem(int row, int col, VecFloat::InParam f) 
{
    GetRow(row).SetComponent(col, f);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE void Matrix44::SetElem(int row, int col, VecFloatRef<INDEX> f)
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix44::SetElem(int row, int col, VecFloatRefIndex::InParam f) 
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix44::SetElem(int i, VecFloat::InParam f) 
{
    SetElem( i / 4, i % 4, f );
}

template<VectorAxis INDEX> void Matrix44::SetElem(int i, VecFloatRef<INDEX> f)
{
    SetElem( i, VecFloat(f) );
}

RW_MATH_FORCE_INLINE void Matrix44::SetElem(int i, VecFloatRefIndex::InParam f) 
{
    SetElem( i, VecFloat(f) );
}

RW_MATH_FORCE_INLINE void Matrix44::SetRow(int i, Vector4::InParam v) 
{
    GetRow(i) = v;
}

RW_MATH_FORCE_INLINE void Matrix44::SetX(Vector4::InParam v) 
{
    SetRow(0, v);
}

RW_MATH_FORCE_INLINE void Matrix44::SetY(Vector4::InParam v) 
{
    SetRow(1, v);
}

RW_MATH_FORCE_INLINE void Matrix44::SetZ(Vector4::InParam v) 
{
    SetRow(2, v);
}

RW_MATH_FORCE_INLINE void Matrix44::SetW(Vector4::InParam v) 
{
    SetRow(3, v);
}

template<class T>
RW_MATH_FORCE_INLINE Matrix44::Matrix44(const fpu::Matrix44Template<T> &_m) 
:xAxis(Vector4(_m.XAxis()))
,yAxis(Vector4(_m.YAxis()))
,zAxis(Vector4(_m.ZAxis()))
,wAxis(Vector4(_m.WAxis()))
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44() 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44(const Matrix44Affine& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, m.xAxis.mV, VectorAxisW, zero, VectorAxisW );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, m.yAxis.mV, VectorAxisW, zero, VectorAxisW );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, m.zAxis.mV, VectorAxisW, zero, VectorAxisW );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, m.wAxis.mV, VectorAxisW,  one, VectorAxisW );
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44(const Matrix44AffinePacked& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    VectorIntrinsic lVector = RW_MATH_VPU_GetL_R(); // (0, 0, 0, 1)
    RW_MATH_VPU_TransposeA4V4(xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, lVector);
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44(Vector4::InParam row0, Vector4::InParam row1, Vector4::InParam row2, Vector4::InParam row3) 
:xAxis(row0)
,yAxis(row1)
,zAxis(row2)
,wAxis(row3)
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44(VectorIntrinsicInParam row0, VectorIntrinsicInParam row1, VectorIntrinsicInParam row2, VectorIntrinsicInParam row3) 
:xAxis(row0)
,yAxis(row1)
,zAxis(row2)
,wAxis(row3)
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44(     float  m0,  float  m1,  float  m2, float  m3,    //  x  row 0
                                             float  m4,  float  m5,  float  m6, float  m7,    //  y  row 1
                                             float  m8,  float  m9,  float m10, float m11,    //  z  row 2
                                             float m12,  float m13,  float m14, float m15     //  w  row 3
                                             )
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    Set(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15);
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44(     VecFloat::InParam  m0,  VecFloat::InParam  m1,  VecFloat::InParam  m2, VecFloat::InParam  m3,    //  x  row 0
                                             VecFloat::InParam  m4,  VecFloat::InParam  m5,  VecFloat::InParam  m6, VecFloat::InParam  m7,    //  y  row 1
                                             VecFloat::InParam  m8,  VecFloat::InParam  m9,  VecFloat::InParam m10, VecFloat::InParam m11,    //  z  row 2
                                             VecFloat::InParam m12,  VecFloat::InParam m13,  VecFloat::InParam m14, VecFloat::InParam m15     //  w  row 3
                                             )
    :xAxis(Vector4( m0,  m1,  m2,  m3))
    ,yAxis(Vector4( m4,  m5,  m6,  m7))
    ,zAxis(Vector4( m8,  m9, m10, m11))
    ,wAxis(Vector4(m12, m13, m14, m15))
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44::Matrix44( const float m[16] ) 
    :xAxis( &m[ 0] )
    ,yAxis( &m[ 4] )
    ,zAxis( &m[ 8] )
    ,wAxis( &m[12] )
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*m, sizeof(VectorIntrinsic));
}

}

}

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix44& t)
{
    EndianSwap(t.xAxis);
    EndianSwap(t.yAxis);
    EndianSwap(t.zAxis);
    EndianSwap(t.wAxis);
}

}

#endif
