// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX44AFFINE_H
#define RW_MATH_VPU_MATRIX44AFFINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/quatpos.h"
#include "rw/math/fpu/matrix44affine.h"

namespace rw {
namespace math {
namespace vpu {

/*!
    \brief Represents a 4x4 matrix.

    The Matrix44Affine class is implemented using hardware efficient vector types.
    Therefore, the Matrix should always be aligned on a 16byte boundary.

    The Matrix44Affine class is \i conceptually very similar to the Matrix44 class.
    Both classes represent a 4x4 matrix.  Also, both classes have functions that
    operate on the class as if it were a 4x4 matrix.  The main difference between
    the classes is that the Matrix44Affine class assumes the right most column is
    set to (0, 0, 0, 1).  To maintain this assumption and maintain high
    performance, users are not able to get, set, or index this portion of the
    matrix.  Also, some functions/methods may be slightly different that expected
    because the last column of the matrix can't be modified.  For example, the
    SetZero method only sets the 4x3 sub-matrix to zero, since the last column is
    always assumed to be (0 ,0 ,0 ,1).
*/
class Matrix44Affine
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfRows = 4 };
    enum { NumberOfCols = 3 };
    enum { NumberOfElement = 12 };
    
    #ifdef RW_MATH_MATRIX_PASS_BY_VALUE
        typedef const Matrix44Affine  InParam;
    #else
        typedef const Matrix44Affine& InParam;
    #endif
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix44Affine& OutParam;
        typedef Matrix44Affine& InOutParam;
    #endif
    
    template<class T>
    explicit Matrix44Affine(const fpu::Matrix44AffineTemplate<T> &m);    
    RW_MATH_FORCE_INLINE Matrix44Affine();    
    explicit RW_MATH_FORCE_INLINE Matrix44Affine(const Matrix44& m);
    RW_MATH_FORCE_INLINE Matrix44Affine(const Matrix44Affine& m);
    RW_MATH_FORCE_INLINE Matrix44Affine(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3);
    RW_MATH_FORCE_INLINE Matrix44Affine(VectorIntrinsicInParam row0, VectorIntrinsicInParam row1, VectorIntrinsicInParam row2, VectorIntrinsicInParam row3);    
    RW_MATH_FORCE_INLINE Matrix44Affine(float  m0,  float  m1,  float  m2, float  m3,  float  m4,  float  m5,
                                        float  m6,  float  m7,  float  m8, float  m9,  float m10,  float m11);
    RW_MATH_FORCE_INLINE Matrix44Affine(VecFloat::InParam  m0,  VecFloat::InParam  m1,  VecFloat::InParam  m2, VecFloat::InParam  m3,  VecFloat::InParam  m4,  VecFloat::InParam  m5,
                                        VecFloat::InParam  m6,  VecFloat::InParam  m7,  VecFloat::InParam  m8, VecFloat::InParam  m9,  VecFloat::InParam m10,  VecFloat::InParam m11);
    //! Construct a Matrix44Affine from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Matrix44Affine(const float m[12]);
    RW_MATH_FORCE_INLINE Matrix44Affine(Quaternion::InParam q, Vector3::InParam pos);
    RW_MATH_FORCE_INLINE Matrix44Affine(const Matrix44AffinePacked& m);
    RW_MATH_FORCE_INLINE Matrix44Affine& operator = (const Matrix44Affine& v);
        
    RW_MATH_FORCE_INLINE void SetZero();
    RW_MATH_FORCE_INLINE void SetIdentity();

    RW_MATH_FORCE_INLINE void Set(float  m0,  float  m1,  float  m2, float  m3,  float  m4,  float  m5,
                                      float  m6,  float  m7,  float  m8, float  m9,  float m10,  float m11);

    RW_MATH_FORCE_INLINE void Set(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2, Vector3::InParam row3);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRef<INDEX> f);
    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRefIndex::InParam f);
    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloat::InParam f);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRef<INDEX> f);
    RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloat::InParam f);
    RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRefIndex::InParam f);

    RW_MATH_FORCE_INLINE void SetRow(int i, Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetX(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetY(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetZ(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetW(Vector3::InParam v);

    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int i) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int row, int column) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex operator () (int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator () (int row, int column) const;

    RW_MATH_FORCE_INLINE       Vector3& GetRow(int i);
    RW_MATH_FORCE_INLINE const Vector3& GetRow(int i) const;

    RW_MATH_FORCE_INLINE Vector3& GetX();
    RW_MATH_FORCE_INLINE Vector3& GetY();
    RW_MATH_FORCE_INLINE Vector3& GetZ();
    RW_MATH_FORCE_INLINE Vector3& GetW();
    
    RW_MATH_FORCE_INLINE const Vector3& GetX() const;
    RW_MATH_FORCE_INLINE const Vector3& GetY() const;
    RW_MATH_FORCE_INLINE const Vector3& GetZ() const;
    RW_MATH_FORCE_INLINE const Vector3& GetW() const;

    RW_MATH_FORCE_INLINE Vector3& XAxis();
    RW_MATH_FORCE_INLINE Vector3& YAxis();
    RW_MATH_FORCE_INLINE Vector3& ZAxis();
    RW_MATH_FORCE_INLINE Vector3& WAxis();

    RW_MATH_FORCE_INLINE const Vector3& XAxis() const;
    RW_MATH_FORCE_INLINE const Vector3& YAxis() const;
    RW_MATH_FORCE_INLINE const Vector3& ZAxis() const;
    RW_MATH_FORCE_INLINE const Vector3& WAxis() const;

    RW_MATH_FORCE_INLINE Vector3& Right();
    RW_MATH_FORCE_INLINE Vector3& Up();
    RW_MATH_FORCE_INLINE Vector3& At();
    RW_MATH_FORCE_INLINE Vector3& Pos();

    RW_MATH_FORCE_INLINE const Vector3& Right() const;
    RW_MATH_FORCE_INLINE const Vector3& Up() const;
    RW_MATH_FORCE_INLINE const Vector3& At() const;
    RW_MATH_FORCE_INLINE const Vector3& Pos() const;

    RW_MATH_FORCE_INLINE Vector4 GetXColumn() const;
    RW_MATH_FORCE_INLINE Vector4 GetYColumn() const;
    RW_MATH_FORCE_INLINE Vector4 GetZColumn() const;

    RW_MATH_FORCE_INLINE void SetXColumn(Vector4::InParam v);
    RW_MATH_FORCE_INLINE void SetYColumn(Vector4::InParam v);
    RW_MATH_FORCE_INLINE void SetZColumn(Vector4::InParam v);

public:

    Vector3 xAxis;
    Vector3 yAxis;
    Vector3 zAxis;
    Vector3 wAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector3 Matrix44Affine::* const sElements[];
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
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix44Affine& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix44affine_type_inline.h"
#endif


namespace rw {
namespace math {
namespace fpu {

    // NOTE defining this constructor here vs. fpu/matrix44affine.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template<class T>
    RW_MATH_FORCE_INLINE Matrix44AffineTemplate<T>::Matrix44AffineTemplate( const rw::math::vpu::Matrix44Affine& m)
    {
        this->xAxis = Vector3Template<T>(m.xAxis);
        this->yAxis = Vector3Template<T>(m.yAxis);
        this->zAxis = Vector3Template<T>(m.zAxis);
        this->wAxis = Vector3Template<T>(m.wAxis);
    }

}}}

#endif
