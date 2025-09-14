// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MATRIX44AFFINE_OPERATION_H
#define RW_MATH_VPU_MATRIX44AFFINE_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, float a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, Matrix44Affine::InParam b);

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Matrix44Affine& operator /=(Matrix44Affine& m, VecFloatRef<INDEX> a);        
RW_MATH_FORCE_INLINE Matrix44Affine& operator /=(Matrix44Affine& m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator /=(Matrix44Affine& m, float a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator /=(Matrix44Affine& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine& operator +=(Matrix44Affine& m, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine& operator -=(Matrix44Affine& m, Matrix44Affine::InParam b);
    
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam m, VecFloatRef<INDEX> a);
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Matrix44Affine operator * (VecFloatRef<INDEX> a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam m, float a);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (VecFloat::InParam a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (float a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (VecFloatRefIndex::InParam a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam a, Matrix44Affine::InParam b);    
RW_MATH_FORCE_INLINE Vector4        operator * (Vector4::InParam v, Matrix44Affine::InParam m);
    
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE Matrix44Affine operator / (Matrix44Affine::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix44Affine operator / (Matrix44Affine::InParam m, VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine operator / (Matrix44Affine::InParam m, float a);
RW_MATH_FORCE_INLINE Matrix44Affine operator / (Matrix44Affine::InParam m, VecFloatRefIndex::InParam a);

RW_MATH_FORCE_INLINE Matrix44Affine operator + (Matrix44Affine::InParam a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine operator - (Matrix44Affine::InParam m);    
RW_MATH_FORCE_INLINE Matrix44Affine operator - (Matrix44Affine::InParam a, Matrix44Affine::InParam b);

RW_MATH_FORCE_INLINE bool operator ==(Matrix44Affine::InParam a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE bool operator !=(Matrix44Affine::InParam a, Matrix44Affine::InParam b);

RW_MATH_FORCE_INLINE bool IsSimilar(Matrix44Affine::InParam a, Matrix44Affine::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
RW_MATH_FORCE_INLINE bool IsValid(Matrix44Affine::InParam a);
RW_MATH_FORCE_INLINE bool IsNormal(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsOrthogonal(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsRotation(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsRotation3x3(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsAnyNaN(Matrix44Affine::InParam m);


RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError(Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError(Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix44Affine::InParam matrix);

RW_MATH_FORCE_INLINE void SelfAdd(Matrix44Affine& a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE void SelfSubtract(Matrix44Affine& a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE void SelfMult(Matrix44Affine& m, VecFloat::InParam a);
    
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SelfMult(Matrix44Affine& m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE void SelfMult(Matrix44Affine& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE void SelfDivide(Matrix44Affine& m, VecFloat::InParam a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE void SelfDivide(Matrix44Affine& m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE void SelfDivide(Matrix44Affine& m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE VecFloat Determinant(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine Negate(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, VecFloat::InParam a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, VecFloatRefIndex::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine Divide(Matrix44Affine::InParam m, VecFloat::InParam a);

template<VectorAxis INDEX> RW_MATH_FORCE_INLINE Matrix44Affine Divide(Matrix44Affine::InParam m, VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE Matrix44Affine Divide(Matrix44Affine::InParam m, VecFloatRefIndex::InParam a);

RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, Matrix44Affine::InParam m);

RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine Add(Matrix44Affine::InParam a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine Subtract(Matrix44Affine::InParam a, Matrix44Affine::InParam b);
RW_MATH_FORCE_INLINE Matrix44Affine Transpose(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine OrthoNormalize(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine InverseOrthonormal(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine InverseOfMatrixWithOrthonormal3x3(Matrix44Affine::InParam in);
RW_MATH_INLINE Matrix44Affine Inverse(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine Inverse(Matrix44Affine::InParam m, VecFloat& determinant);
RW_MATH_FORCE_INLINE Vector3 TransformPoint(Vector3::InParam pt, Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE void TransformPoints(const Vector3* const pts, int numPoints, Matrix44Affine::InParam matrix, Vector3* transformedPts);
RW_MATH_FORCE_INLINE Vector3 TransformVector(Vector3::InParam vec, Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE void TransformVectors(const Vector3* const vecs, int numVectors, Matrix44Affine::InParam matrix, Vector3 * transformedVecs);
RW_MATH_FORCE_INLINE Vector3 Transform(Vector4::InParam vec, Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE void Transform(const Vector4* const vecs, int numVectors, Matrix44Affine::InParam matrix, Vector3 * transformedVecs);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromQuaternion(Quaternion::InParam q);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromEulerXYZ(Vector3::InParam euler);
RW_MATH_FORCE_INLINE Vector3 EulerXYZFromMatrix44Affine(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromAxisRotationAngle(Vector3::InParam inputAxis, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromXRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromYRotationAngle(VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromZRotationAngle(VecFloat::InParam angle);

RW_MATH_FORCE_INLINE Matrix44Affine RotateAroundXAxis(Matrix44Affine::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44Affine RotateAroundYAxis(Matrix44Affine::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44Affine RotateAroundZAxis(Matrix44Affine::InParam m, VecFloat::InParam angle);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineTransposeRotationalComponent(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineTranslate(Matrix44Affine::InParam m, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44Affine ScaleRow(Matrix44Affine::InParam m, VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2);
RW_MATH_FORCE_INLINE Matrix44Affine ScaleColumn(Matrix44Affine::InParam m, VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromDiagonal(VecFloat::InParam a);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScale(Vector3::InParam scale);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromTranslation(Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScaleQuaternion(Vector3::InParam scale, Quaternion::InParam q);        
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScaleTranslation(Vector3::InParam scale, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoLH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoRH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrtho(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoOffCenterLH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoOffCenterRH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromQuaternionTranslation(Quaternion::InParam q, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScaleQuaternionTranslation(Vector3::InParam scale, Quaternion::InParam q, Vector3::InParam trans);
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromLookAt(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition);
RW_MATH_FORCE_INLINE Vector3 QueryRotateDegenerateUnitAxis(Matrix44Affine::InParam matrix);
RW_MATH_INLINE void QueryRotate(Matrix44Affine::InParam matrix, Vector3& lpUnitAxis, VecFloat& lprAngle, Vector3& lpCenter);
RW_MATH_INLINE Matrix44Affine SLerp(Matrix44Affine::InParam lFrom, Matrix44Affine::InParam lTo, VecFloat::InParam lrParam, VecFloat& lprAngle);
RW_MATH_INLINE Matrix44Affine SLerpByAngle(Matrix44Affine::InParam lFrom, Matrix44Affine::InParam lTo, VecFloat::InParam lrMaxAngle, VecFloat& lprAngle) ;

RW_MATH_FORCE_INLINE bool IsValid3x3(Matrix44Affine::InParam a);
RW_MATH_FORCE_INLINE bool IsNormal3x3(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));    
RW_MATH_FORCE_INLINE bool IsOrthogonal3x3(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet3x3(Matrix44Affine::InParam a, VecFloat::InParam tolerance = VecFloat(EPSILON));
RW_MATH_FORCE_INLINE VecFloat Determinant3x3(Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Vector4 Mult3x3(Vector4::InParam v, Matrix44Affine::InParam m);
RW_MATH_FORCE_INLINE Vector3 Mult3x3(Vector3::InParam v, Matrix44Affine::InParam m);
RW_MATH_INLINE Matrix44Affine Inverse3x3(Matrix44Affine::InParam m, VecFloat& determinant);
RW_MATH_INLINE Matrix44Affine OrthoNormalize3x3(Matrix44Affine::InParam matrix);
RW_MATH_INLINE Matrix44Affine InverseGeneric3x3(Matrix44Affine::InParam in);
RW_MATH_FORCE_INLINE VecFloat GetNormalError3x3(Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError3x3(Matrix44Affine::InParam matrix);
RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError3x3(Matrix44Affine::InParam matrix);
RW_MATH_INLINE Matrix33 Get3x3(Matrix44Affine::InParam m);
RW_MATH_INLINE Matrix44Affine Set3x3(Matrix44Affine::InParam m, Matrix33::InParam m33);

RW_MATH_FORCE_INLINE Matrix44Affine Select(MaskScalar::InParam mask, Matrix44Affine::InParam trueValue, Matrix44Affine::InParam falseValue );

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/matrix44affine_operation_inline.h"
#endif

#endif
