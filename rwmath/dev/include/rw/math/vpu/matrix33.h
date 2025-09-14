// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX33_H
#define RW_MATH_VPU_MATRIX33_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/fpu/matrix33.h"

namespace rw {
namespace math {
namespace vpu {

/*!
    \brief Represents a 3 by 3 matrix.

    The Matrix33 class is implemented using hardware efficient vector types.
    Therefore, the matrix should always be aligned on a 16byte boundary.
*/
class Matrix33
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfRows = 3 };
    enum { NumberOfCols = 3 };
    enum { NumberOfElement = 9 };

    #ifdef RW_MATH_MATRIX_PASS_BY_VALUE
        typedef const Matrix33  InParam;
    #else
        typedef const Matrix33& InParam;
    #endif
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix33& OutParam;
        typedef Matrix33& InOutParam;
    #endif

    template<class T>
    explicit RW_MATH_FORCE_INLINE Matrix33(const fpu::Matrix33Template<T> &m);

    RW_MATH_FORCE_INLINE Matrix33();
    RW_MATH_FORCE_INLINE Matrix33(const Matrix33& _m);
    RW_MATH_FORCE_INLINE Matrix33(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2);
    RW_MATH_FORCE_INLINE Matrix33(VectorIntrinsicInParam row0, VectorIntrinsicInParam row1, VectorIntrinsicInParam row2);
    RW_MATH_FORCE_INLINE Matrix33(const float& m0,  const float& m1,  const float& m2,
                                  const float& m3,  const float& m4,  const float& m5,
                                  const float& m6,  const float& m7,  const float& m8);
    RW_MATH_FORCE_INLINE Matrix33(VecFloat::InParam m0,  VecFloat::InParam m1,  VecFloat::InParam m2,
                                  VecFloat::InParam m3,  VecFloat::InParam m4,  VecFloat::InParam m5,
                                  VecFloat::InParam m6,  VecFloat::InParam m7,  VecFloat::InParam m8);
    //! Construct a Matrix33 from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Matrix33(const float m[9]);

    RW_MATH_FORCE_INLINE Matrix33& operator = (const Matrix33& m_);

    RW_MATH_FORCE_INLINE void SetZero();
    RW_MATH_FORCE_INLINE void SetIdentity();

    RW_MATH_FORCE_INLINE void Set(float m0,  float m1,  float m2,
                           float m3,  float m4,  float m5,
                           float m6,  float m7,  float m8);

    RW_MATH_FORCE_INLINE void Set(Vector3::InParam row0, Vector3::InParam row1, Vector3::InParam row2);

    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int i) const;
    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int row, int column) const;

    RW_MATH_FORCE_INLINE VecFloatRefIndex operator () (int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator () (int row, int column) const;

    RW_MATH_FORCE_INLINE Vector3&       GetRow(int i);
    RW_MATH_FORCE_INLINE const Vector3& GetRow(int i) const;
    
    RW_MATH_FORCE_INLINE Vector3&       GetX();    
    RW_MATH_FORCE_INLINE Vector3&       GetY();    
    RW_MATH_FORCE_INLINE Vector3&       GetZ();

    RW_MATH_FORCE_INLINE const Vector3& GetX() const;
    RW_MATH_FORCE_INLINE const Vector3& GetY() const;
    RW_MATH_FORCE_INLINE const Vector3& GetZ() const;

    RW_MATH_FORCE_INLINE Vector3&       XAxis();
    RW_MATH_FORCE_INLINE Vector3&       YAxis();
    RW_MATH_FORCE_INLINE Vector3&       ZAxis();

    RW_MATH_FORCE_INLINE const Vector3& XAxis() const;
    RW_MATH_FORCE_INLINE const Vector3& YAxis() const;
    RW_MATH_FORCE_INLINE const Vector3& ZAxis() const;

    RW_MATH_FORCE_INLINE Vector3&       Right();
    RW_MATH_FORCE_INLINE Vector3&       Up();
    RW_MATH_FORCE_INLINE Vector3&       At();

    RW_MATH_FORCE_INLINE const Vector3& Right() const;
    RW_MATH_FORCE_INLINE const Vector3& Up() const;
    RW_MATH_FORCE_INLINE const Vector3& At() const;

    RW_MATH_FORCE_INLINE Vector3 GetXColumn() const;
    RW_MATH_FORCE_INLINE Vector3 GetYColumn() const;
    RW_MATH_FORCE_INLINE Vector3 GetZColumn() const;

    RW_MATH_FORCE_INLINE void SetXColumn(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetYColumn(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetZColumn(Vector3::InParam v);

                               RW_MATH_FORCE_INLINE void SetElem(int i, float f);
                               RW_MATH_FORCE_INLINE void SetElem(int i, VecFloat::InParam f);
    template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRef<INDEX> f);
                               RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRefIndex::InParam f);
                               RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloat::InParam f);
                               RW_MATH_FORCE_INLINE void SetElem(int row, int col, float f);
    template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRef<INDEX> f);
                               RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRefIndex::InParam f);
    
    RW_MATH_FORCE_INLINE void SetRow(int i, Vector3::InParam v);
    
    RW_MATH_FORCE_INLINE void SetX(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetY(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetZ(Vector3::InParam v);

public:

    Vector3     xAxis;
    Vector3     yAxis;
    Vector3     zAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector3 Matrix33::* const sElements[];
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
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix33& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix33_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

// Note; defining this constructor here vs. fpu/matrix33.h avoids ps3 compiler error.
// This also allow us to remove the dependency of fpu header on vpu header
template<class T>
RW_MATH_FORCE_INLINE Matrix33Template<T>::Matrix33Template( const rw::math::vpu::Matrix33& m)
{
    xAxis = Vector3Template<T>(m.xAxis);
    yAxis = Vector3Template<T>(m.yAxis);
    zAxis = Vector3Template<T>(m.zAxis);
}

}}}

#endif
