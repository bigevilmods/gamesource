// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX44AFFINE_PACKED_H
#define RW_MATH_VPU_MATRIX44AFFINE_PACKED_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/quatpos.h"
#include "rw/math/fpu/matrix44affinepacked.h"

namespace rw {
namespace math {
namespace vpu {

class Matrix44AffinePacked
{
public:
    
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfRows = 3 };
    enum { NumberOfCols = 4 };
    enum { NumberOfElement = 12 };

    #ifdef RW_MATH_MATRIX_PASS_BY_VALUE
        typedef const Matrix44AffinePacked InParam;
    #else
        typedef const Matrix44AffinePacked& InParam;
    #endif 
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix44AffinePacked& OutParam;
        typedef Matrix44AffinePacked& InOutParam;
    #endif
    
    template<class T>
    explicit Matrix44AffinePacked(const fpu::Matrix44AffinePackedTemplate<T> &m);
    RW_MATH_FORCE_INLINE Matrix44AffinePacked();
    RW_MATH_FORCE_INLINE Matrix44AffinePacked(const Matrix44AffinePacked& m);
    RW_MATH_FORCE_INLINE Matrix44AffinePacked(const Matrix44Affine& m);
    explicit RW_MATH_FORCE_INLINE Matrix44AffinePacked(const Matrix44& m);
    RW_MATH_FORCE_INLINE Matrix44AffinePacked(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3);
    //! Construct a Matrix44Affine from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE Matrix44AffinePacked(const float m[12]);
    RW_MATH_FORCE_INLINE Matrix44AffinePacked(float  m0,  float  m1,  float  m2,
                                              float  m3,  float  m4,  float  m5,
                                              float  m6,  float  m7,  float  m8,
                                              float  m9,  float m10,  float  m11);
    RW_MATH_FORCE_INLINE Matrix44AffinePacked(VecFloat::InParam  m0,  VecFloat::InParam  m1,  VecFloat::InParam  m2,
                                              VecFloat::InParam  m3,  VecFloat::InParam  m4,  VecFloat::InParam  m5,
                                              VecFloat::InParam  m6,  VecFloat::InParam  m7,  VecFloat::InParam  m8,
                                              VecFloat::InParam  m9,  VecFloat::InParam m10,  VecFloat::InParam  m11);
    
    
    RW_MATH_FORCE_INLINE void SetZero();

    RW_MATH_FORCE_INLINE void Set(float  m0,  float  m1,  float  m2,  float  m3,
                                       float  m4,  float  m5,  float  m6,  float  m7,
                                       float  m8,  float  m9,  float m10,  float m11);

    RW_MATH_FORCE_INLINE void Set(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3);

    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int i) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int row, int column) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex operator () (int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator () (int row, int column) const;
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRef<INDEX> f);
    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRefIndex::InParam f);
    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloat::InParam f);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRef<INDEX> f);
    RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloat::InParam f);    
    RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRefIndex::InParam f);

public:

    Vector4 xAxis;
    Vector4 yAxis;
    Vector4 zAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector4 Matrix44AffinePacked::* const sElements[];
#endif
};

}

using namespace ::rw::math::vpu;

}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix44AffinePacked& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix44affinepacked_type_inline.h"
#endif


namespace rw {
namespace math {
namespace fpu {

template<class T>
RW_MATH_FORCE_INLINE Matrix44AffinePackedTemplate<T>::Matrix44AffinePackedTemplate(const rw::math::vpu::Matrix44AffinePacked& m)
{
    this->xAxis = m.xAxis;
    this->yAxis = m.yAxis;
    this->zAxis = m.zAxis;
}

}}}

#endif
