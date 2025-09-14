// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX44_OPERATION_H
#define RW_MATH_VPU_MATRIX44_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE Matrix44&   operator *=(Matrix44& m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44&   operator /=(Matrix44& m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44    operator * (Matrix44::InParam m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44    operator * (VecFloat::InParam a, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44    operator / (Matrix44::InParam m, VecFloat::InParam a);

RW_MATH_FORCE_INLINE Matrix44&   operator *=(Matrix44& m,          float a);
RW_MATH_FORCE_INLINE Matrix44&   operator /=(Matrix44& m,          float a);
RW_MATH_FORCE_INLINE Matrix44    operator * (Matrix44::InParam m,             float a);
RW_MATH_FORCE_INLINE Matrix44    operator * (float a, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44    operator / (Matrix44::InParam m,             float a);

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44&   operator *=(Matrix44& m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44&   operator /=(Matrix44& m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44    operator * (Matrix44::InParam m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44    operator * (VecFloatRef<INDEX> a, Matrix44::InParam m);
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44    operator / (Matrix44::InParam m, VecFloatRef<INDEX> a);

RW_MATH_FORCE_INLINE Matrix44&   operator *=(Matrix44& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44&   operator /=(Matrix44& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44    operator * (Matrix44::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44    operator * (VecFloatRefIndex::InParam a, Matrix44::InParam m);
RW_MATH_FORCE_INLINE Matrix44    operator / (Matrix44::InParam m, VecFloatRefIndex::InParam a);

RW_MATH_FORCE_INLINE Vector4     operator * (Vector4::InParam v, Matrix44::InParam m);
RW_MATH_FORCE_INLINE Matrix44&   operator *=(Matrix44& m, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44&   operator +=(Matrix44& m, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44&   operator -=(Matrix44& m, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44    operator - (Matrix44::InParam m);
RW_MATH_FORCE_INLINE Matrix44    operator + (Matrix44::InParam m, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44    operator - (Matrix44::InParam m, Matrix44::InParam b);

RW_MATH_FORCE_INLINE Matrix44    operator * (Matrix44::InParam m, Matrix44::InParam b);

RW_MATH_FORCE_INLINE bool        operator ==(Matrix44::InParam a, Matrix44::InParam b);

RW_MATH_FORCE_INLINE bool        operator !=(Matrix44::InParam a, Matrix44::InParam b);

RW_MATH_FORCE_INLINE bool          IsSimilar(Matrix44::InParam a, Matrix44::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool           IsAnyNaN(Matrix44::InParam m);


RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError3x3(Matrix44::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError3x3(Matrix44::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix44::InParam matrix);

RW_MATH_FORCE_INLINE bool            IsValid(Matrix44::InParam a);
RW_MATH_FORCE_INLINE bool           IsNormal(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool       IsOrthogonal(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsRotation(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsRotation3x3(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError(Matrix44::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError(Matrix44::InParam matrix);
RW_MATH_FORCE_INLINE void SelfAdd(Matrix44& a, Matrix44::InParam b);
RW_MATH_FORCE_INLINE void SelfSubtract(Matrix44& a, Matrix44::InParam b);
RW_MATH_FORCE_INLINE void SelfMult(Matrix44& m, VecFloat::InParam a);

/*
Multiply matrix a by matrix b

\param a The matrix to multiply.  It will store the result of the operation
\param b The scalar to multiply the matrix with.

*/
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void SelfMult(Matrix44& m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE void SelfMult(Matrix44& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE void SelfDivide(Matrix44& m, VecFloat::InParam a);

/*
Divide matrix a by matrix b

\param a The matrix to divide.  It will store the result of the operation
\param b The scalar to divide the matrix with.

*/
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void SelfDivide(Matrix44& m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE void SelfDivide(Matrix44& m, VecFloatRefIndex::InParam a);

RW_MATH_FORCE_INLINE VecFloat Determinant(Matrix44::InParam m);

RW_MATH_FORCE_INLINE Matrix44 Negate(Matrix44::InParam m);

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44::InParam m, VecFloat::InParam a);

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44 Divide(Matrix44::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix44 Divide(Matrix44::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44 Divide(Matrix44::InParam m, VecFloat::InParam a);

RW_MATH_FORCE_INLINE Vector4  Mult(Vector4::InParam v, Matrix44::InParam m);

RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44Affine::InParam m, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44 Add(Matrix44::InParam a, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44 Subtract(Matrix44::InParam a, Matrix44::InParam b);
RW_MATH_FORCE_INLINE Matrix44 Transpose(Matrix44::InParam m);
RW_MATH_INLINE             Matrix44 OrthoNormalize(Matrix44::InParam m);
RW_MATH_FORCE_INLINE Matrix44 InverseOrthonormal(Matrix44::InParam m);
RW_MATH_FORCE_INLINE Matrix44 Inverse(Matrix44::InParam m);
RW_MATH_INLINE             Matrix44 Inverse(Matrix44::InParam m, VecFloat& determinant);

RW_MATH_FORCE_INLINE Vector3 TransformPoint(Vector3::InParam pt, Matrix44::InParam matrix);
RW_MATH_FORCE_INLINE Vector3 TransformVector(Vector3::InParam vec, Matrix44::InParam matrix);
RW_MATH_FORCE_INLINE Vector3 Transform(Vector4::InParam vec, Matrix44::InParam matrix);

RW_MATH_FORCE_INLINE void TransformPoints(const Vector3* const pts, int numPoints, Matrix44::InParam matrix, Vector3* transformedPts);
RW_MATH_FORCE_INLINE void TransformVectors(const Vector3* const vecs, int numVectors, Matrix44::InParam matrix, Vector3 * transformedVecs);
RW_MATH_FORCE_INLINE void Transform(const Vector4* const vecs, int numVectors, Matrix44::InParam matrix, Vector3 * transformedVecs);

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromAxisRotationAngle(Vector3::InParam inputAxis, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromXRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromYRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromZRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromQuaternion(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromEulerXYZ(Vector3::InParam euler);
RW_MATH_FORCE_INLINE Vector3 EulerXYZFromMatrix44(Matrix44::InParam m);

RW_MATH_FORCE_INLINE Matrix44 Matrix44InverseRotationalComponent(Matrix44::InParam src);
RW_MATH_FORCE_INLINE Matrix44 RotateAroundXAxis(Matrix44::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 RotateAroundYAxis(Matrix44::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 RotateAroundZAxis(Matrix44::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44 Matrix44TransposeRotationalComponent(Matrix44::InParam m);
RW_MATH_FORCE_INLINE Matrix44 Matrix44Translate(Matrix44::InParam m, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44 ScaleRow(Matrix44::InParam m, VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2);
RW_MATH_FORCE_INLINE Matrix44 ScaleColumn(Matrix44::InParam m, VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromDiagonal(VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScale(Vector3::InParam scale);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromTranslation(Vector3::InParam trans);

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScaleQuaternion(Vector3::InParam scale, Quaternion::InParam q);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScaleTranslation(Vector3::InParam scale, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoLH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoRH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrtho(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoOffCenterLH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoOffCenterRH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromQuaternionTranslation(Quaternion::InParam q, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScaleQuaternionTranslation(Vector3::InParam scale, Quaternion::InParam q, Vector3::InParam trans);

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveLH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveRH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspective(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveFovLH(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveFovRH(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveFov(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveOffCenterLH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveOffCenterRH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromLookAtLH(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromLookAtRH(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition);
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromLookAt(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition);

RW_MATH_FORCE_INLINE Vector3 QueryRotateDegenerateUnitAxis(Matrix44::InParam matrix);
RW_MATH_INLINE void QueryRotate(Matrix44::InParam matrix, Vector3& lpUnitAxis, VecFloat& lprAngle, Vector3& lpCenter);

RW_MATH_INLINE Matrix44 SLerp(Matrix44::InParam lFrom, Matrix44::InParam lTo, VecFloat::InParam lrParam, VecFloat& lprAngle);
RW_MATH_INLINE Matrix44 SLerpByAngle(Matrix44::InParam lFrom, Matrix44::InParam lTo, VecFloat::InParam lrMaxAngle, VecFloat& lprAngle);

RW_MATH_FORCE_INLINE bool IsValid3x3(Matrix44::InParam a);
RW_MATH_FORCE_INLINE bool IsNormal3x3(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));

RW_MATH_FORCE_INLINE bool IsOrthogonal3x3(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet3x3(Matrix44::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));

RW_MATH_FORCE_INLINE VecFloat Determinant3x3(Matrix44::InParam m);

RW_MATH_FORCE_INLINE Vector4 Mult3x3(Vector4::InParam v, Matrix44::InParam m);

RW_MATH_FORCE_INLINE Vector3 Mult3x3(Vector3::InParam v, Matrix44::InParam m);

RW_MATH_INLINE Matrix44 Inverse3x3(Matrix44::InParam m);

RW_MATH_FORCE_INLINE Matrix44 Inverse3x3(Matrix44::InParam m, VecFloat& determinant);

RW_MATH_INLINE Matrix44 OrthoNormalize3x3(Matrix44::InParam matrix);

RW_MATH_FORCE_INLINE VecFloat GetNormalError3x3(Matrix44::InParam matrix);

RW_MATH_INLINE Matrix33 Get3x3(Matrix44::InParam m);
RW_MATH_INLINE Matrix44 Set3x3(Matrix44::InParam m, Matrix33::InParam m33);

RW_MATH_FORCE_INLINE Matrix44 Select(MaskScalar::InParam mask, Matrix44::InParam trueValue, Matrix44::InParam falseValue );

#if RW_MATH_ENABLE_DEPRECATED_API

    //! \internal
    //! \todo this is used by a deprecated function, remove this after deprecated function is removed
    enum MatrixType
    {
        MATRIXTYPE_NA                     = 0x00,
        MATRIXTYPE_NORMAL                 = 0x01,
        MATRIXTYPE_ORTHOGONAL             = 0x02,
        MATRIXTYPE_ORTHONORMAL            = 0x03,
        MATRIXTYPE_IDENTITY               = 0x04
    };

    RW_MATH_FORCE_INLINE
    Matrix44 Matrix44Invert3x3(Matrix44::InParam in, MatrixType type);    
    RW_MATH_FORCE_INLINE
    void Negate(Matrix44::InParam m, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Mult(Matrix44::InParam m, VecFloat::InParam a, Matrix44& result);
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE
    void Mult(Matrix44::InParam m, VecFloatRef<INDEX> a, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Mult(Matrix44::InParam m, VecFloatRefIndex::InParam a, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Divide(Matrix44::InParam m, VecFloat::InParam a, Matrix44& result);
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE
    void Divide(Matrix44::InParam m, VecFloatRef<INDEX> a, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Divide(Matrix44::InParam m, VecFloatRefIndex::InParam a, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Mult(Vector4::InParam v, Matrix44::InParam m, Vector4& result);
    RW_MATH_FORCE_INLINE 
    void Mult(Matrix44::InParam m, Matrix44::InParam b, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Add(Matrix44::InParam a, Matrix44::InParam b, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Subtract(Matrix44::InParam a, Matrix44::InParam b, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Transpose(Matrix44::InParam m, Matrix44& result);
    RW_MATH_FORCE_INLINE
    VecFloat        Inverse(Matrix44::InParam m, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromQuaternion(Quaternion::InParam q, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromAxisAngle(Vector3::InParam axis, VecFloat::InParam angle, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromEulerXYZ(Vector3::InParam euler, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void EulerXYZFromMatrix44(Matrix44::InParam m, Vector3& result);
    RW_MATH_FORCE_INLINE
    void Mult3x3(Vector4::InParam v, Matrix44::InParam m, Vector4& result);
    RW_MATH_FORCE_INLINE
    void Mult3x3(Vector3::InParam v, Matrix44::InParam m, Vector3& result);
    RW_MATH_FORCE_INLINE
    VecFloat Matrix44Inverse3x3(Matrix44::InParam src, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44InverseRotTrans(Matrix44::InParam src, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromScale(Vector3::InParam scale, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromTranslation(Vector3::InParam trans, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromScaleQuaternion(Vector3::InParam scale, Quaternion::InParam q, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromScaleTranslation(Vector3::InParam scale, Vector3::InParam trans, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44Diagonal(VecFloat::InParam a, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44RotationX(VecFloat::InParam angle, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44RotationY(VecFloat::InParam angle, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44RotationZ(VecFloat::InParam angle, Matrix44& result);
    RW_MATH_FORCE_INLINE void Matrix44RotateX(VecFloat::InParam angle, Matrix44& m);
    RW_MATH_FORCE_INLINE void Matrix44RotateY(VecFloat::InParam angle, Matrix44& m);
    RW_MATH_FORCE_INLINE void Matrix44RotateZ(VecFloat::InParam angle, Matrix44& m);
    RW_MATH_FORCE_INLINE void Matrix44TransposeRotation(Matrix44& m);
    RW_MATH_FORCE_INLINE void Matrix44Translate(Vector3::InParam trans, Matrix44& m);
    RW_MATH_FORCE_INLINE void Matrix44ScaleRow(VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2, Matrix44& m);
    RW_MATH_FORCE_INLINE void Matrix44ScaleColumn(VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2, Matrix44& m);
    RW_MATH_FORCE_INLINE
    void Matrix44Perspective(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result); RW_MATH_FORCE_INLINE
    void Matrix44Ortho(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44OrthoOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromQuaternionTranslation(Quaternion::InParam q, Vector3::InParam trans, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44FromScaleQuaternionTranslation(Vector3::InParam scale, Quaternion::InParam q, Vector3::InParam trans, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44PerspectiveFov(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44LookAt(Vector3::InParam eye, Vector3::InParam focal, Vector3::InParam up, Matrix44& result);
    RW_MATH_FORCE_INLINE
    void Matrix44PerspectiveOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result);

#endif

}}}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix44_operation_inline.h"
#endif

#endif
