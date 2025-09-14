// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_Matrix44AffinePacked_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_Matrix44AffinePacked_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/matrix44affine.h"
#include "rw/math/vpu/matrix44affinepacked.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"

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

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::Set(
    float  m0,  float  m1,  float  m2,
    float  m3,  float  m4,  float  m5,
    float  m6,  float  m7,  float  m8,
    float  m9,  float m10,  float  m11
    ) 
{
    xAxis.Set(m0, m3, m6, m9);
    yAxis.Set(m1, m4, m7, m10);
    zAxis.Set(m2, m5, m8, m11);
}

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::Set(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3) 
{
    Matrix44 temp(row0, row1, row2, row3);
    xAxis = temp.GetXColumn();
    yAxis = temp.GetYColumn();
    zAxis = temp.GetZColumn();
}

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::SetZero() 
{
    xAxis.SetZero();
    yAxis.SetZero();
    zAxis.SetZero();
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44AffinePacked::GetElem(int row, int column) 
{
    RW_MATH_ASSERT_MSG( 0 <= row && row <= 3 && 0 <= column && column <= 2, ("Illegal row/column index (%d,%d)", row, column) );

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return (this->*sElements[column]).GetComponent(row); // not row, column, it's transposed!!!
#else
    return (&xAxis)[column].GetComponent(row); // not row, column, it's transposed!!!
#endif
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44AffinePacked::GetElem(int row, int column) const 
{
    return const_cast<Matrix44AffinePacked*>(this)->GetElem(row, column);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44AffinePacked::GetElem(int i) 
{
    const int row = i / 3;
    const int column = i % 3;
    return GetElem( row, column );
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44AffinePacked::GetElem(int i) const 
{
    return const_cast<Matrix44AffinePacked*>(this)->GetElem(i);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Matrix44AffinePacked::operator () (int row, int column) 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Matrix44AffinePacked::operator () (int row, int column) const 
{
    return GetElem(row, column);
}

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::SetElem(int row, int col, VecFloat::InParam f) 
{
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    return (this->*sElements[col]).SetComponent(row, f);
#else
    (&xAxis)[col].SetComponent(row, f);
#endif
}

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::SetElem(int i, VecFloat::InParam f) 
{
    const int row = i / 3;
    const int column = i % 3;
    SetElem( row, column, f );
}

template<VectorAxis INDEX> void Matrix44AffinePacked::SetElem(int i, VecFloatRef<INDEX> f)
{
    SetElem( i, VecFloat(f) );
}

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::SetElem(int i, VecFloatRefIndex::InParam f) 
{
    SetElem( i, VecFloat(f) );
}

template<VectorAxis INDEX> void Matrix44AffinePacked::SetElem(int row, int col, VecFloatRef<INDEX> f)
{
    SetElem(row, col, VecFloat(f));
}

RW_MATH_FORCE_INLINE void Matrix44AffinePacked::SetElem(int row, int col, VecFloatRefIndex::InParam f) 
{
    SetElem(row, col, VecFloat(f));
}

template<class T>
RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(const fpu::Matrix44AffinePackedTemplate<T> &m) 
:xAxis( Vector4(m.xAxis) )
,yAxis( Vector4(m.yAxis) )
,zAxis( Vector4(m.zAxis) )
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}


RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked() 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(const Matrix44AffinePacked& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    (*this) = m;
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    Set(row0, row1, row2, row3);
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(
    float  m0,  float  m1,  float  m2,  float  m3,
    float  m4,  float  m5,  float  m6,  float  m7,
    float  m8,  float  m9,  float m10,  float m11
    ) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    Set(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(
    VecFloat::InParam m0,  VecFloat::InParam  m1,  VecFloat::InParam  m2,
    VecFloat::InParam m3,  VecFloat::InParam  m4,  VecFloat::InParam  m5,
    VecFloat::InParam m6,  VecFloat::InParam  m7,  VecFloat::InParam  m8,
    VecFloat::InParam m9,  VecFloat::InParam m10,  VecFloat::InParam m11
    ) 
    :xAxis(m0, m3, m6,  m9)
    ,yAxis(m1, m4, m7, m10)
    ,zAxis(m2, m5, m8, m11)
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked( const float m[12] ) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*m, sizeof(VectorIntrinsic));
    Set(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11]);
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(const Matrix44Affine& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    RW_MATH_VPU_TransposeA4V3(xAxis.mV, yAxis.mV, zAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV);
}

RW_MATH_FORCE_INLINE Matrix44AffinePacked::Matrix44AffinePacked(const Matrix44& m) 
{
    rwMathAlignmentConstructorCheckAssert(*this, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(m, sizeof(VectorIntrinsic));
    RW_MATH_ASSERT_MSG(
            IsSimilar(m.xAxis.W(), GetVecFloat_Zero()) &&
            IsSimilar(m.yAxis.W(), GetVecFloat_Zero()) &&
            IsSimilar(m.zAxis.W(), GetVecFloat_Zero()) &&
            IsSimilar(m.wAxis.W(), GetVecFloat_One()),
            ("The input matrix is not an affine transformation.") );
    RW_MATH_VPU_TransposeA4V3(xAxis.mV, yAxis.mV, zAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV);
}


} // namespace vpu

} // namespace math

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix44AffinePacked& t)
{
    EndianSwap(reinterpret_cast<math::vpu::Vector4*>(&t)[0]);
    EndianSwap(reinterpret_cast<math::vpu::Vector4*>(&t)[0]);
    EndianSwap(reinterpret_cast<math::vpu::Vector4*>(&t)[0]);
}

} // namespace rw

#endif
