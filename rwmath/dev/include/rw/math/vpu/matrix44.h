// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX44_H
#define RW_MATH_VPU_MATRIX44_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/fpu/matrix44.h"

namespace rw {
namespace math {
namespace vpu {

/*!
    \brief Represents a 4x4 matrix.

    The Matrix44 class is implemented using hardware efficient vector types.
    Therefore, the Matrix should always be aligned on a 16byte boundary.
*/
class Matrix44
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfRows = 4 };
    enum { NumberOfCols = 4 };
    enum { NumberOfElement = 16 };

    #ifdef RW_MATH_MATRIX_PASS_BY_VALUE
        typedef const Matrix44  InParam;
    #else
        typedef const Matrix44& InParam;
    #endif
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Matrix44& OutParam;
        typedef Matrix44& InOutParam;
    #endif
        
    template<class T>
    explicit RW_MATH_INLINE Matrix44(const fpu::Matrix44Template<T> &m);    
    RW_MATH_FORCE_INLINE Matrix44();    
    RW_MATH_FORCE_INLINE Matrix44(const Matrix44& m);    
    explicit RW_MATH_FORCE_INLINE Matrix44(const Matrix44Affine& m);    
    explicit RW_MATH_FORCE_INLINE Matrix44(const Matrix44AffinePacked& m);    
    RW_MATH_FORCE_INLINE Matrix44(Vector4::InParam row0, Vector4::InParam row1, Vector4::InParam row2, Vector4::InParam row3);
    RW_MATH_FORCE_INLINE Matrix44(VectorIntrinsicInParam row0, VectorIntrinsicInParam row1, VectorIntrinsicInParam row2, VectorIntrinsicInParam row3);
    RW_MATH_FORCE_INLINE Matrix44(float  m0,  float  m1,  float  m2, float  m3, float  m4,  float  m5,  float  m6, float  m7,
                                      float  m8,  float  m9,  float m10, float m11, float m12,  float m13,  float m14, float m15);
    RW_MATH_FORCE_INLINE Matrix44(VecFloat::InParam  m0,  VecFloat::InParam  m1,  VecFloat::InParam  m2, VecFloat::InParam  m3, VecFloat::InParam  m4,  VecFloat::InParam  m5,  VecFloat::InParam  m6, VecFloat::InParam  m7,
                                      VecFloat::InParam  m8,  VecFloat::InParam  m9,  VecFloat::InParam m10, VecFloat::InParam m11, VecFloat::InParam m12,  VecFloat::InParam m13,  VecFloat::InParam m14, VecFloat::InParam m15);

    //! Construct a Matrix44 from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Matrix44(const float m[16]);
    
    RW_MATH_FORCE_INLINE Matrix44& operator = (const Matrix44& v);
    
    RW_MATH_FORCE_INLINE void SetZero();
    RW_MATH_FORCE_INLINE void SetIdentity();
    RW_MATH_FORCE_INLINE void Set(float  m0,  float  m1,  float  m2, float  m3, float  m4,  float  m5,  float  m6, float  m7,
                                      float  m8,  float  m9,  float m10, float m11, float m12,  float m13,  float m14, float m15);
    
    RW_MATH_FORCE_INLINE void Set(Vector4::InParam row0, Vector4::InParam row1, Vector4::InParam row2, Vector4::InParam row3);
    
    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int i) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetElem(int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetElem(int row, int column) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex operator () (int row, int column);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator () (int row, int column) const;

    RW_MATH_FORCE_INLINE       Vector4& GetRow(int i);    
    RW_MATH_FORCE_INLINE const Vector4& GetRow(int i) const;

    
    RW_MATH_FORCE_INLINE Vector4& GetX();
    RW_MATH_FORCE_INLINE Vector4& GetY();
    RW_MATH_FORCE_INLINE Vector4& GetZ();
    RW_MATH_FORCE_INLINE Vector4& GetW();

    
    RW_MATH_FORCE_INLINE const Vector4& GetX() const;
    RW_MATH_FORCE_INLINE const Vector4& GetY() const;
    RW_MATH_FORCE_INLINE const Vector4& GetZ() const;
    RW_MATH_FORCE_INLINE const Vector4& GetW() const;

    RW_MATH_FORCE_INLINE Vector4& XAxis();
    RW_MATH_FORCE_INLINE Vector4& YAxis();
    RW_MATH_FORCE_INLINE Vector4& ZAxis();
    RW_MATH_FORCE_INLINE Vector4& WAxis();

    RW_MATH_FORCE_INLINE const Vector4& XAxis() const;
    RW_MATH_FORCE_INLINE const Vector4& YAxis() const;
    RW_MATH_FORCE_INLINE const Vector4& ZAxis() const;
    RW_MATH_FORCE_INLINE const Vector4& WAxis() const;
    
    RW_MATH_FORCE_INLINE Vector4&       Right();
    RW_MATH_FORCE_INLINE Vector4&       Up();
    RW_MATH_FORCE_INLINE Vector4&       At();
    RW_MATH_FORCE_INLINE Vector4&       Pos();
    RW_MATH_FORCE_INLINE const Vector4& Right() const;
    RW_MATH_FORCE_INLINE const Vector4& Up() const;
    RW_MATH_FORCE_INLINE const Vector4& At() const;
    RW_MATH_FORCE_INLINE const Vector4& Pos() const;

    RW_MATH_FORCE_INLINE Vector4 GetXColumn() const;
    RW_MATH_FORCE_INLINE Vector4 GetYColumn() const;
    RW_MATH_FORCE_INLINE Vector4 GetZColumn() const;
    RW_MATH_FORCE_INLINE Vector4 GetWColumn() const;

    RW_MATH_FORCE_INLINE void SetXColumn(Vector4::InParam v);
    RW_MATH_FORCE_INLINE void SetYColumn(Vector4::InParam v);
    RW_MATH_FORCE_INLINE void SetZColumn(Vector4::InParam v);
    RW_MATH_FORCE_INLINE void SetWColumn(Vector4::InParam v);

    RW_MATH_FORCE_INLINE void SetElem(int i, VecFloat::InParam f);

    template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRef<INDEX> f);
                               RW_MATH_FORCE_INLINE void SetElem(int i, VecFloatRefIndex::InParam f);
                               
    template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRef<INDEX> f);    
                               RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloat::InParam f);
                               RW_MATH_FORCE_INLINE void SetElem(int row, int col, VecFloatRefIndex::InParam f);

    
    RW_MATH_FORCE_INLINE void SetRow(int i, Vector4::InParam v);    
    
    RW_MATH_FORCE_INLINE void SetX(Vector4::InParam v);    
    RW_MATH_FORCE_INLINE void SetY(Vector4::InParam v);    
    RW_MATH_FORCE_INLINE void SetZ(Vector4::InParam v);    
    RW_MATH_FORCE_INLINE void SetW(Vector4::InParam v);

    Vector4     xAxis;
    Vector4     yAxis;
    Vector4     zAxis;
    Vector4     wAxis;

private:
#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    static Vector4 Matrix44::* const sElements[];
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
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Matrix44& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix44_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

    // NOTE defining this constructor here vs. fpu/matrix44.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template<class T>
    RW_MATH_FORCE_INLINE Matrix44Template<T>::Matrix44Template( const rw::math::vpu::Matrix44& m)
    {
        this->xAxis = Vector4Template<T>(m.xAxis);
        this->yAxis = Vector4Template<T>(m.yAxis);
        this->zAxis = Vector4Template<T>(m.zAxis);
        this->wAxis = Vector4Template<T>(m.wAxis);
    }

}}}

#endif
