// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX33_OPERATION_H
#define RW_MATH_VPU_MATRIX33_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Matrix33&   operator *=(Matrix33& m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix33&   operator /=(Matrix33& m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix33    operator * (Matrix33::InParam m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix33    operator * (VecFloat::InParam a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33    operator / (Matrix33::InParam m, VecFloat::InParam a);

RW_MATH_FORCE_INLINE Matrix33&   operator *=(Matrix33& m, float a);
RW_MATH_FORCE_INLINE Matrix33&   operator /=(Matrix33& m, float a);
RW_MATH_FORCE_INLINE Matrix33    operator * (float a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33    operator * (Matrix33::InParam m, float a);
RW_MATH_FORCE_INLINE Matrix33    operator / (Matrix33::InParam m, float a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33&  operator *=(Matrix33& m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33&  operator /=(Matrix33& m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33   operator * (Matrix33::InParam m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33   operator * (VecFloatRef<INDEX> a, Matrix33::InParam b);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33   operator / (Matrix33::InParam m, VecFloatRef<INDEX> a);

RW_MATH_FORCE_INLINE Matrix33&   operator *=(Matrix33& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix33&   operator /=(Matrix33& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix33    operator * (Matrix33::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix33    operator * (VecFloatRefIndex::InParam a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33    operator / (Matrix33::InParam m, VecFloatRefIndex::InParam a);

RW_MATH_FORCE_INLINE Vector3     operator * (Vector3::InParam v, Matrix33::InParam m);

RW_MATH_FORCE_INLINE Matrix33&   operator *=(Matrix33& m, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33&   operator +=(Matrix33& m, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33&   operator -=(Matrix33& m, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33    operator - (Matrix33::InParam m);
RW_MATH_FORCE_INLINE Matrix33    operator + (Matrix33::InParam a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33    operator - (Matrix33::InParam a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE Matrix33    operator * (Matrix33::InParam a, Matrix33::InParam b);

RW_MATH_FORCE_INLINE bool operator ==(Matrix33::InParam a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE bool operator !=(Matrix33::InParam a, Matrix33::InParam b);

RW_MATH_FORCE_INLINE bool IsSimilar(Matrix33::InParam a, Matrix33::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool IsAnyNaN(Matrix33::InParam a);

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError(Matrix33::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError(Matrix33::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix33::InParam matrix);

RW_MATH_FORCE_INLINE bool IsValid(Matrix33::InParam a);

RW_MATH_FORCE_INLINE bool IsNormal(Matrix33::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));

RW_MATH_FORCE_INLINE bool IsOrthogonal(Matrix33::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet(Matrix33::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsRotation(Matrix33::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));

RW_MATH_FORCE_INLINE void SelfAdd(Matrix33& a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE void SelfSubtract(Matrix33& a, Matrix33::InParam b);
RW_MATH_FORCE_INLINE void SelfMult(Matrix33& m, VecFloat::InParam a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SelfMult(Matrix33& m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE void SelfMult(Matrix33& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE void SelfDivide(Matrix33& m, VecFloat::InParam a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SelfDivide(Matrix33& m, VecFloatRef<INDEX> a);

RW_MATH_FORCE_INLINE void SelfDivide(Matrix33& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE VecFloat Determinant(Matrix33::InParam m);
RW_MATH_FORCE_INLINE Matrix33 Negate(Matrix33::InParam m);
RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, VecFloat::InParam a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix33 Divide(Matrix33::InParam m, VecFloat::InParam a);
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix33 Divide(Matrix33::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix33 Divide(Matrix33::InParam m, VecFloatRefIndex::InParam a);

RW_MATH_FORCE_INLINE Vector3  Mult(Vector3::InParam v, Matrix33::InParam m);
RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, Matrix33::InParam b);

RW_MATH_FORCE_INLINE Matrix33 Add(Matrix33::InParam a, Matrix33::InParam b);

RW_MATH_FORCE_INLINE Matrix33 Subtract(Matrix33::InParam a, Matrix33::InParam b);

RW_MATH_FORCE_INLINE Matrix33 Transpose(Matrix33::InParam m);

/*!
    \brief Computes an orthonormal matrix given an input matrix.

    The function accepts a matrix as input and returns an orthonormalized
    matrix that is "similar".  How similar the resulting matrix is to the input
    matrix depends heavily on how close the input matrix is to being
    orthonormal.
    
    \pre The input matrix must consist of four row vectors that are linearly
    independent.  Put another way, the input matrix must have a rank of four.

    See http://mathworld.wolfram.com/LinearlyIndependent.html
*/
RW_MATH_INLINE Matrix33 OrthoNormalize(Matrix33::InParam m);

/*!
    \brief Computes a inverse given an input matrix which should be orthonormalized.
    \pre The input matrix must be orthonormalized.
    */
RW_MATH_FORCE_INLINE Matrix33 InverseOrthonormal(Matrix33::InParam m);

RW_MATH_FORCE_INLINE Matrix33 Inverse(Matrix33::InParam m);
RW_MATH_INLINE Matrix33 Inverse(Matrix33::InParam m, VecFloat& determinant) ;

RW_MATH_FORCE_INLINE Vector3 Transform(Vector3::InParam vec, Matrix33::InParam matrix);

RW_MATH_FORCE_INLINE void Transform(const Vector3* const vecs, int numVectors, Matrix33::InParam matrix, Vector3* transformedVecs);
RW_MATH_FORCE_INLINE Matrix33 Matrix33FromQuaternion(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Matrix33 Matrix33FromEulerXYZ(Vector3::InParam euler);

/*! 
    \brief Constructs a Euler angles vector based on the input matrix

    The XYZ convention is also referred as pitch-roll-yaw
    See http://mathworld.wolfram.com/EulerAngles.html
*/
RW_MATH_INLINE Vector3 EulerXYZFromMatrix33(Matrix33::InParam m);

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromAxisRotationAngle(Vector3::InParam inputAxis, VecFloat::InParam angle);

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromXRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix33 Matrix33FromYRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix33 Matrix33FromZRotationAngle(VecFloat::InParam angle);

RW_MATH_FORCE_INLINE Matrix33 RotateAroundXAxis(Matrix33::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix33 RotateAroundYAxis(Matrix33::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix33 RotateAroundZAxis(Matrix33::InParam m, VecFloat::InParam angle);

RW_MATH_FORCE_INLINE Matrix33 ScaleRow(Matrix33::InParam m, VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2);
RW_MATH_FORCE_INLINE Matrix33 ScaleColumn(Matrix33::InParam m, VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2);

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromDiagonal(VecFloat::InParam a);

RW_MATH_INLINE Vector3 QueryRotateDegenerateUnitAxis(Matrix33::InParam lMatrix);
RW_MATH_INLINE void QueryRotate(Matrix33::InParam matrix, Vector3& unitAxis, VecFloat& angle);
RW_MATH_INLINE Matrix33 SLerp(Matrix33::InParam matrixFrom, Matrix33::InParam matrixTo, VecFloat::InParam param, VecFloat& angle);
RW_MATH_INLINE Matrix33 SLerpByAngle(Matrix33::InParam matrixFrom, Matrix33::InParam matrixTo, VecFloat::InParam maxAngle, VecFloat& angle);

RW_MATH_FORCE_INLINE Matrix33 Select(MaskScalar::InParam mask, Matrix33::InParam trueValue, Matrix33::InParam falseValue );

// ---------------------------------------------------------
#if RW_MATH_ENABLE_DEPRECATED_API

     RW_MATH_FORCE_INLINE
    void Negate(Matrix33::InParam m, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Mult(Matrix33::InParam m, VecFloat::InParam a, Matrix33& result);

     template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE
    void Mult(Matrix33::InParam m, VecFloatRef<INDEX> a, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Mult(Matrix33::InParam m, VecFloatRefIndex::InParam a, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Divide(Matrix33::InParam m, VecFloat::InParam a, Matrix33& result);

     template<VectorAxis INDEX> 
    RW_MATH_FORCE_INLINE
    void Divide(Matrix33::InParam m, VecFloatRef<INDEX> a, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Divide(Matrix33::InParam m, VecFloatRefIndex::InParam a, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Mult(Vector3::InParam v, Matrix33::InParam m, Vector3& result);

     RW_MATH_FORCE_INLINE
    void Mult(Matrix33::InParam m, Matrix33::InParam b, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Add(Matrix33::InParam a, Matrix33::InParam b, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Subtract(Matrix33::InParam a, Matrix33::InParam b, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Transpose(Matrix33::InParam m, Matrix33& result);

     RW_MATH_FORCE_INLINE
    VecFloat Inverse(Matrix33::InParam m, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Matrix33FromQuaternion(Quaternion::InParam q, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Matrix33FromAxisAngle(Vector3::InParam axis, VecFloat::InParam angle, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void Matrix33FromEulerXYZ(Vector3::InParam euler, Matrix33& result);

     RW_MATH_FORCE_INLINE
    void EulerXYZFromMatrix33(Matrix33::InParam m, Vector3& result);

#endif

}}}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix33_operation_inline.h"
#endif

#endif
