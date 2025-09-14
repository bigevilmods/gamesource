// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX33_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX33_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/matrix33.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE Matrix33& Matrix33::operator = (const Matrix33& m) 
{
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
    return *this;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::GetRow(int i) 
{
    RW_MATH_ASSERT_MSG(i >= 0 && i <= 2, ("Invalid Matrix33 row(%d)", i));
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return this->*sElements[i];
#else
    return (&xAxis)[i];
#endif
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::GetRow(int i) const 
{
    RW_MATH_ASSERT_MSG(i >= 0 && i <= 2, ("Invalid Matrix33 row(%d)", i));
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return this->*sElements[i];
#else
    return (&xAxis)[i];
#endif
}

RW_MATH_FORCE_INLINE void Matrix33::Set(
                    float m0,  float m1,  float m2,     //  x  row 0
                    float m3,  float m4,  float m5,     //  y  row 1
                    float m6,  float m7,  float m8      //  z  row 2
                  ) 
{
    xAxis.Set(m0, m1, m2);
    yAxis.Set(m3, m4, m5);
    zAxis.Set(m6, m7, m8);
}

RW_MATH_FORCE_INLINE void Matrix33::Set(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2) 
{
    xAxis = row0;
    yAxis = row1;
    zAxis = row2;
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33(VectorIntrinsicInParam row0, VectorIntrinsicInParam row1, VectorIntrinsicInParam row2)
{
    xAxis.mV = row0;
    yAxis.mV = row1;
    zAxis.mV = row2;
}

RW_MATH_FORCE_INLINE void Matrix33::SetZero() 
{
    RW_MATH_VPU_GetZero(xAxis.mV);
    yAxis.mV = xAxis.mV;
    zAxis.mV = xAxis.mV;
}

RW_MATH_FORCE_INLINE void Matrix33::SetIdentity() 
{
    RW_MATH_VPU_GetI(xAxis.mV);
    RW_MATH_VPU_GetJ(yAxis.mV);
    RW_MATH_VPU_GetK(zAxis.mV);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix33::GetElem(int row, int column) 
{
    return GetRow(row).GetComponent(column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix33::GetElem(int row, int column)  const
{
    return GetRow(row).GetComponent(column);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix33::GetElem(int i) 
{
    return GetElem( i / 3, i % 3 );
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix33::GetElem(int i)  const
{
    return GetElem( i / 3, i % 3 );
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix33::operator () (int row, int column) 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix33::operator () (int row, int column)  const
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::GetX() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::GetY() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::GetZ() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::GetX() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::GetY() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::GetZ() const 
{
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::XAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::YAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::ZAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::XAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::YAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::ZAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::Right() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::Up() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix33::At() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::Right() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::Up() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix33::At() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector3 Matrix33::GetXColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractXElementsAsVectorA3V3( result, xAxis.mV, yAxis.mV, zAxis.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Matrix33::GetYColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractYElementsAsVectorA3V3( result, xAxis.mV, yAxis.mV, zAxis.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE Vector3 Matrix33::GetZColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractZElementsAsVectorA3V3( result, xAxis.mV, yAxis.mV, zAxis.mV );
    return Vector3(result);
}

RW_MATH_FORCE_INLINE void Matrix33::SetXColumn(Vector3::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisX, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisX, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisX, v.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Matrix33::SetYColumn(Vector3::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisY, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisY, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisY, v.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Matrix33::SetZColumn(Vector3::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisZ, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisZ, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisZ, v.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Matrix33::SetElem(int row, int col, VecFloat::InParam f) 
{
    GetRow(row).SetComponent(col, f);
}

RW_MATH_FORCE_INLINE void Matrix33::SetElem(int i, VecFloat::InParam f) 
{
    SetElem( i / 3, i % 3, f );
}

RW_MATH_FORCE_INLINE void Matrix33::SetElem(int row, int col, float f) 
{
    GetRow(row).SetComponent(col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix33::SetElem(int i, float f) 
{
    SetElem(i / 3, i % 3, VecFloat(f));
}

template<VectorAxis INDEX> void Matrix33::SetElem(int i, VecFloatRef<INDEX> f)
{
    SetElem( i, VecFloat(f) );
}

RW_MATH_FORCE_INLINE void Matrix33::SetElem(int i, VecFloatRefIndex::InParam f) 
{
    SetElem( i, VecFloat(f) );
}

template<VectorAxis INDEX> void Matrix33::SetElem(int row, int col, VecFloatRef<INDEX> f)
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix33::SetElem(int row, int col, VecFloatRefIndex::InParam f) 
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix33::SetRow(int i, Vector3::InParam v) 
{
    GetRow(i) = v;
}

RW_MATH_FORCE_INLINE void Matrix33::SetX(Vector3::InParam v) 
{
    SetRow(0, v);
}

RW_MATH_FORCE_INLINE void Matrix33::SetY(Vector3::InParam v) 
{
    SetRow(1, v);
}

RW_MATH_FORCE_INLINE void Matrix33::SetZ(Vector3::InParam v) 
{
    SetRow(2, v);
}

template<class T>
RW_MATH_FORCE_INLINE Matrix33::Matrix33(const fpu::Matrix33Template<T> &_m) 
:xAxis(Vector3(_m.XAxis()))
,yAxis(Vector3(_m.YAxis()))
,zAxis(Vector3(_m.ZAxis()))
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33() 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33(const Matrix33& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2) 
:xAxis(row0)
,yAxis(row1)
,zAxis(row2)
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33(
    const float& m0,  const float& m1,  const float& m2,     //  x  row 0
    const float& m3,  const float& m4,  const float& m5,     //  y  row 1
    const float& m6,  const float& m7,  const float& m8      //  z  row 2
    ) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    Set(m0, m1, m2, m3, m4, m5, m6, m7, m8);
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33(
    VecFloat::InParam m0,  VecFloat::InParam m1,  VecFloat::InParam m2,     //  x  row 0
    VecFloat::InParam m3,  VecFloat::InParam m4,  VecFloat::InParam m5,     //  y  row 1
    VecFloat::InParam m6,  VecFloat::InParam m7,  VecFloat::InParam m8      //  z  row 2
    ) 
    :xAxis(m0, m1, m2)
    ,yAxis(m3, m4, m5)
    ,zAxis(m6, m7, m8)
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix33::Matrix33(const float m[9]) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    Set(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
}

}

}

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix33& t)
{
    EndianSwap(t.xAxis);
    EndianSwap(t.yAxis);
    EndianSwap(t.zAxis);
}

}

#endif
