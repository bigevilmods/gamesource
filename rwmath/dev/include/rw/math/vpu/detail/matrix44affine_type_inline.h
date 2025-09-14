// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_Matrix44Affine_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_Matrix44Affine_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/matrix44affine.h"
#include "rw/math/vpu/matrix44affinepacked.h"
#include "rw/math/vpu/constants_operation.h"

#ifdef RW_MATH_ENABLE_ASSERTS
// Currently scalar.h is only needed for some assertion code.
#include "rw/math/vpu/scalar.h"
#endif

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Matrix44Affine& Matrix44Affine::operator = (const Matrix44Affine& m) 
{
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
    wAxis.mV = m.wAxis.mV;
    return *this;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::GetRow(int i) 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return this->*sElements[i];
#else
    return (&xAxis)[i];
#endif
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::GetRow(int i) const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return this->*sElements[i];
#else
    return (&xAxis)[i];
#endif
}

RW_MATH_FORCE_INLINE void Matrix44Affine::Set(float  m0,  float  m1,  float  m2, float  m3,  float  m4,  float  m5,
                                                   float  m6,  float  m7,  float  m8, float  m9,  float m10,  float  m11) 
{
    xAxis.Set( m0,  m1,  m2);
    yAxis.Set( m3,  m4,  m5);
    zAxis.Set( m6,  m7,  m8);
    wAxis.Set( m9, m10, m11);
}

RW_MATH_FORCE_INLINE void Matrix44Affine::Set(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3) 
{
    xAxis = row0;
    yAxis = row1;
    zAxis = row2;
    wAxis = row3;
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetZero() 
{
    xAxis.SetZero();
    yAxis.SetZero();
    zAxis.SetZero();
    wAxis.SetZero();
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetIdentity() 
{
    RW_MATH_VPU_GetI(xAxis.mV);
    RW_MATH_VPU_GetJ(yAxis.mV);
    RW_MATH_VPU_GetK(zAxis.mV);
    RW_MATH_VPU_GetZero(wAxis.mV);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44Affine::GetElem(int row, int column) 
{
    RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
    return GetRow(row).GetComponent(column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44Affine::GetElem(int row, int column) const 
{
    RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );
    return GetRow(row).GetComponent(column);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44Affine::GetElem(int i) 
{
    const int row = i / 3;
    const int column = i % 3;
    return GetElem( row, column );
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44Affine::GetElem(int i) const 
{
    return const_cast<Matrix44Affine*>(this)->GetElem(i);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44Affine::operator () (int row, int column) 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44Affine::operator () (int row, int column) const 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::GetX() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::GetY() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::GetZ() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::GetW() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::GetX() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::GetY() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::GetZ() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::GetW() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::XAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::YAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::ZAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::WAxis() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::XAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::YAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::ZAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::WAxis() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::Right() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::Up() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::At() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE Vector3& Matrix44Affine::Pos() 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::Right() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return xAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::Up() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return yAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::At() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return zAxis;
}

RW_MATH_FORCE_INLINE const Vector3& Matrix44Affine::Pos() const 
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    return wAxis;
}

RW_MATH_FORCE_INLINE Vector4 Matrix44Affine::GetXColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Matrix44Affine::GetYColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractYElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE Vector4 Matrix44Affine::GetZColumn() const
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    VectorIntrinsic result;
    RW_MATH_VPU_ExtractZElementsAsVectorA4V4( result, xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV );
    return Vector4(result);
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetXColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisX, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisX, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisX, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisX, v.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetYColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisY, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisY, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisY, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisY, v.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetZColumn(Vector4::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_I_I( xAxis.mV, xAxis.mV, VectorAxisZ, v.mV, VectorAxisX );
    RW_MATH_VPU_SwizzleStore_I_I( yAxis.mV, yAxis.mV, VectorAxisZ, v.mV, VectorAxisY );
    RW_MATH_VPU_SwizzleStore_I_I( zAxis.mV, zAxis.mV, VectorAxisZ, v.mV, VectorAxisZ );
    RW_MATH_VPU_SwizzleStore_I_I( wAxis.mV, wAxis.mV, VectorAxisZ, v.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetElem(int row, int col, VecFloat::InParam f) 
{
    GetRow(row).SetComponent(col, f);
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetElem(int i, VecFloat::InParam f) 
{
    const int row = i / 3;
    const int column = i % 3;
    SetElem( row, column, f );
}

template<VectorAxis INDEX> void Matrix44Affine::SetElem(int i, VecFloatRef<INDEX> f)
{
    SetElem( i, VecFloat(f) );
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetElem(int i, VecFloatRefIndex::InParam f) 
{
    SetElem( i, VecFloat(f) );
}

template<VectorAxis INDEX> void Matrix44Affine::SetElem(int row, int col, VecFloatRef<INDEX> f)
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetElem(int row, int col, VecFloatRefIndex::InParam f) 
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetRow(int i, Vector3::InParam v) 
{
    GetRow(i) = v;
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetX(Vector3::InParam v) 
{
    SetRow(0, v);
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetY(Vector3::InParam v) 
{
    SetRow(1, v);
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetZ(Vector3::InParam v) 
{
    SetRow(2, v);
}

RW_MATH_FORCE_INLINE void Matrix44Affine::SetW(Vector3::InParam v) 
{
    SetRow(3, v);
}

template<class T>
RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(const fpu::Matrix44AffineTemplate<T> &m) 
:xAxis( Vector3(m.XAxis()) )
,yAxis( Vector3(m.YAxis()) )
,zAxis( Vector3(m.ZAxis()) )
,wAxis( Vector3(m.WAxis()) )
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}


RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine() 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(const Matrix44Affine& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
    wAxis.mV = m.wAxis.mV;
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(const Matrix44& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    RW_MATH_ASSERT_MSG(
            IsSimilar(m.xAxis.W(), GetVecFloat_Zero()) &&
            IsSimilar(m.yAxis.W(), GetVecFloat_Zero()) &&
            IsSimilar(m.zAxis.W(), GetVecFloat_Zero()) &&
            IsSimilar(m.wAxis.W(), GetVecFloat_One()),
            ("The input matrix is not an affine transformation.") );
    xAxis.mV = m.xAxis.mV;
    yAxis.mV = m.yAxis.mV;
    zAxis.mV = m.zAxis.mV;
    wAxis.mV = m.wAxis.mV;
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    xAxis = row0;
    yAxis = row1;
    zAxis = row2;
    wAxis = row3;
}


RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(VectorIntrinsicInParam row0, VectorIntrinsicInParam row1, VectorIntrinsicInParam row2, VectorIntrinsicInParam row3) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    xAxis.mV = row0;
    yAxis.mV = row1;
    zAxis.mV = row2;
    wAxis.mV = row3;
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(
    float  m0,  float  m1,  float  m2,     //  x  row 0
    float  m3,  float  m4,  float  m5,     //  y  row 1
    float  m6,  float  m7,  float  m8,     //  z  row 2
    float  m9,  float m10,  float m11      //  w  row 3
    ) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    Set(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(
    VecFloat::InParam  m0,  VecFloat::InParam  m1,  VecFloat::InParam  m2,     //  x  row 0
    VecFloat::InParam  m3,  VecFloat::InParam  m4,  VecFloat::InParam  m5,     //  y  row 1
    VecFloat::InParam  m6,  VecFloat::InParam  m7,  VecFloat::InParam  m8,     //  z  row 2
    VecFloat::InParam  m9,  VecFloat::InParam m10,  VecFloat::InParam m11      //  w  row 3
    ) 
    :xAxis(Vector3( m0,  m1,  m2))
    ,yAxis(Vector3( m3,  m4,  m5))
    ,zAxis(Vector3( m6,  m7,  m8))
    ,wAxis(Vector3( m9, m10, m11))
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine( const float m[12] ) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*m, sizeof(VectorIntrinsic));
    xAxis = Vector3(m[0], m[1], m[2]);
    yAxis = Vector3(m[3], m[4], m[5]);
    zAxis = Vector3(m[6], m[7], m[8]);
    wAxis = Vector3(m[9], m[10], m[11]);
}

RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(Quaternion::InParam q, Vector3::InParam pos) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    RW_MATH_VPU_MatrixFromQuaternion(xAxis.mV, yAxis.mV, zAxis.mV, q.mV);
    wAxis.mV = pos.mV;
}


RW_MATH_FORCE_INLINE Matrix44Affine::Matrix44Affine(const Matrix44AffinePacked& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    VectorIntrinsic dummy; RW_MATH_VPU_GetZero(dummy);
    RW_MATH_VPU_TransposeA4V4(xAxis.mV, yAxis.mV, zAxis.mV, wAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, dummy);
}

} // namespace vpu

} // namespace math

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix44Affine& t)
{
    EndianSwap(t.xAxis);
    EndianSwap(t.yAxis);
    EndianSwap(t.zAxis);
    EndianSwap(t.wAxis);
}

} // namespace rw

#endif
