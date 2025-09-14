// (c) 2003 Electronic Arts Inc.

// This file implements many of the linear algebra functions declared in
// XbLinear.h as classes inherited from EA3Expr::Query.  This provides
// a full suite of linear algebra routines for use in expressions.

// The following 3D vector operators and functions are already declared
// in EA3Expr.h:
//
//   vector + vector
//   vector - vector
//   vector * matrix
//   matrix * vector
//   matrix * matrix
//   vector == vector
//   matrix == matrix

#ifndef _EA3_EXPR_LINEAR_H_
#define _EA3_EXPR_LINEAR_H_

#include "EA3Expr.h"
#include "XbMathUtil.h"

namespace EA3Expr  {

	// 3D vector functions:

#ifdef DOXYGEN
	/*! 
	Create a new 3 dimensional vector
	*/
	Result Vec3(Expression &x, Expression &y, Expression &z);
#else
	EXPR_THREE_EXPR_CLASS(Vec3,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (
			(r1.type == Result::RS_FLOAT || r1.type == Result::RS_INT || r1.type == Result::RS_UINT) &&
			(r2.type == Result::RS_FLOAT || r2.type == Result::RS_INT || r2.type == Result::RS_UINT) &&
			(r3.type == Result::RS_FLOAT || r3.type == Result::RS_INT || r3.type == Result::RS_UINT))
			return (XbVec3f(r1.toFloat(), r2.toFloat(), r3.toFloat()));
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the element of a vector (the evaluated expression vec) 
	corresponding to the value of the index (the evaluated expression idx) or
	return 0 
	if vec does not evaluate to a vector or idx does not evaluate to an integer in [0,2].
	*/
	Result GetVecElement(Expression &vec, Expression &idx);
#else
	EXPR_TWO_EXPR_CLASS(GetVecElement,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_VEC3 &&
			(r2.type == Result::RS_INT || r2.type == Result::RS_UINT)) {
				intptr_t index = r2.toInt();
				if (index>=0 && index<3)
					return r1.data.v3()[(int)index];
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the vector (the evaluated expression vec) 
	with the element corresponding to the value of the index (the evaluated expression idx)
	replaced by a specified value (the evaluated expression elt) or
	return 0 
	if vec does not evaluate to a vector or idx does not evaluate to an integer in [0,2]
	or elt does not evaluate to a number.
	*/
	Result SetVecElement(Expression &vec, Expression &idx, Expression &elt);
#else
	EXPR_THREE_EXPR_CLASS(SetVecElement,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_VEC3 &&
			(r2.type == Result::RS_UINT || r2.type == Result::RS_INT) &&
			(r3.type == Result::RS_UINT || r3.type == Result::RS_INT || r3.type == Result::RS_FLOAT)) 
		{
				int index = (int)r2.toInt();
				if (index>=0 && index<3) {
					r1.data.v3()[index] = r3.toFloat();
					return r1.data.v3();
				}
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the normalization of the evaluated expression vec or 
	return 0 if vec does evaluate to a vector.
	*/
	Result Normalize(Expression &vec);
#else
	EXPR_ONE_EXPR_CLASS(Normalize,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
			r.data.v3().normalize();
			return r.data.v3();
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the vector length of the evaluated expression vec or 
	return 0 if vec does evaluate to a vector.
	*/
	Result Length(Expression &vec);
#else
	EXPR_ONE_EXPR_CLASS(Length,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
			return r.data.v3().length();
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the squared length of the evaluated expression vec or 
	return 0 if expression does not evaluate to a vector.
	*/
	Result SqrLength(Expression &vec);
#else
	EXPR_ONE_EXPR_CLASS(SqrLength,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
			return r.data.v3().sqrlength();
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the vector (the evaluated expression vec) with each element negated 
	or 0 if vec does not evaluate to a vector.
	*/
	Result NegateVec(Expression &vec);
#else
	EXPR_ONE_EXPR_CLASS(NegateVec,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
			return r.data.v3().negate();
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the dot product of two vectors (the evaluated expressions vec1, vec2) 
	or 0 if both expressions do not evaluate to vectors.
	*/
	Result Dot(Expression &vec1, Expression &vec2);
#else
	EXPR_TWO_EXPR_CLASS(Dot,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_VEC3 && r2.type == Result::RS_VEC3) {
			return r1.data.v3().dot(r2.data.v3());
		}
		return 0;
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the cross product of two vectors (the evaluated expressions vec1, vec2) 
	or 0 if both expressions do not evaluate to vectors.
	*/
	Result Cross(Expression &vec1, Expression &vec2);
#else
	EXPR_TWO_EXPR_CLASS(Cross,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_VEC3 && r2.type == Result::RS_VEC3) {
			return r1.data.v3().cross(r2.data.v3());
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the angle between two vectors (the evaluated expressions vec1, vec2) 
	or 0 if both expressions do not evaluate to vectors.
	*/
	Result AngleBetweenVecs(Expression &vec1, Expression &vec2);
#else
	EXPR_TWO_EXPR_CLASS(AngleBetweenVecs,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_VEC3 && r2.type == Result::RS_VEC3) {
			return r1.data.v3().angleTo(r2.data.v3());
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return a vector perpendicular to the vector (the evaluated expression vec) 
	or 0 if vec does not evaluate to a vector.
	*/
	Result PerpendicularVec(Expression &vec);
#else
	EXPR_ONE_EXPR_CLASS(PerpendicularVec,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
			return r.data.v3().perpendicular();
		}
		return Result();
	});
#endif

	// 4X4 matrix functions:

#ifdef DOXYGEN
	/*! 
	Return a 4X4 matrix representing 
	a translation expressed as a vector (the evaluated expression trn) 
	a rotation expressed as a vector of euler angles (the evaluated expression rot)
	and a scaling factor expressed as a vector (non-uniform scaling)
	or a number (the evaluated expression scl) 
	or return 0 
	if trn does not evaluate to a vector or rot does not evaluate to an vector
	or scl does not evaluate to a vector or a number.
	*/
	Result Matrix(Expression &trn, Expression &rot, Expression &scl);
#else
	EXPR_THREE_EXPR_CLASS(Matrix,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_VEC3  && r2.type == Result::RS_VEC3) {
			XbVec3f scale;
			if (r3.type == Result::RS_VEC3)
				scale = r3.data.v3();				
			else if (r3.type == Result::RS_FLOAT || r3.type == Result::RS_INT || r3.type == Result::RS_UINT)
				scale = XbVec3f(r3.toFloat(), r3.toFloat(), r3.toFloat());
			else
				return Result();
			XbRotation rot(XbEuler(r2.data.v3(), XbEuler::XYZ, XbEuler::STATIC));
			XbMatrix mat;
			mat.setTransform(r1.data.v3(), rot, scale);
			return mat;
		}
		return Result();
	});
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif // unreferenced formal parameter

#ifdef DOXYGEN
	/*! 
	Return a 4X4 identity matrix
	*/
	Result IdentityMat();
#else
	EXPR_NO_ARGS_CLASS(IdentityMat,{
		XbMatrix mat;
		mat.makeIdentity();
		return mat;
	});
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif // unreferenced formal parameter

#ifdef DOXYGEN
	/*! 
	Return the first three elements of the row of the matrix (the evaluated expression mat) 
	with the row corresponding to the value of the index (the evaluated expression idx)
	or return 0 
	if mat does not evaluate to a matrix or idx does not evaluate to an integer in [0,3].
	NOTE: the result will contain a vector of length 3.
	*/
	Result GetMatRow(Expression &mat, Expression &idx);
#else
	EXPR_TWO_EXPR_CLASS(GetMatRow,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_MATRIX &&
			(r2.type == Result::RS_UINT || r2.type == Result::RS_INT)) {
				intptr_t index = r2.toInt();
				if (index>=0 && index<4) // column major form
					return XbVec3f(r1.data.m44()[0][index], r1.data.m44()[1][index], r1.data.m44()[2][index]);
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the modified matrix (the evaluated expression mat) 
	with the first 3 elements of the row at the index (the evaluated expression idx)
	replaced by a 3 element vector (the evaluated expression vec)
	or return 0 
	if mat does not evaluate to a matrix or idx does not evaluate to an integer in [0,3]
	or vec does not evaluate to a vector.
	NOTE: the 4th element of the row will remain unchanged.
	*/
	Result SetMatRow(Expression &mat, Expression &idx, Expression &vec);
#else
	EXPR_THREE_EXPR_CLASS(SetMatRow,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_MATRIX &&
			(r2.type == Result::RS_UINT || r2.type == Result::RS_INT)) 
		{
				intptr_t index = r2.toInt();
				if (index>=0 && index<4 && r3.type == Result::RS_VEC3) {
					r1.data.m44()[0][index] = r3.data.v3()[0];
					r1.data.m44()[1][index] = r3.data.v3()[1];
					r1.data.m44()[2][index] = r3.data.v3()[2];
					return r1.data.m44();
				}
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the first three elements of the column of the matrix (the evaluated expression mat) 
	with the column corresponding to the value of the index (the evaluated expression idx)
	or return 0 
	if mat does not evaluate to a matrix or idx does not evaluate to an integer in [0,3].
	NOTE: the result will contain a vector of length 3.
	*/
	Result GetMatCol(Expression &mat, Expression &idx);
#else
	EXPR_TWO_EXPR_CLASS(GetMatCol,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_MATRIX &&
			(r2.type == Result::RS_UINT || r2.type == Result::RS_INT)) {
				int index = (int)r2.toInt();
				if (index>=0 && index<4) // column major form
					return XbVec3f(r1.data.m44()[index][0], r1.data.m44()[index][1], r1.data.m44()[index][2]);
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the modified matrix (the evaluated expression mat) 
	with the first 3 elements of the column at the index (the evaluated expression idx)
	replaced by a 3 element vector (the evaluated expression vec)
	or return 0 
	if mat does not evaluate to a matrix or idx does not evaluate to an integer in [0,3]
	or vec does not evaluate to a vector.
	NOTE: the 4th element of the column will remain unchanged.
	*/
	Result SetMatCol(Expression &mat, Expression &idx, Expression &vec);
#else
	EXPR_THREE_EXPR_CLASS(SetMatCol,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_MATRIX &&
			(r2.type == Result::RS_UINT || r2.type == Result::RS_INT)) 
		{
			int index = (int)r2.toInt();
			if (index>=0 && index<4 && r3.type == Result::RS_VEC3) {
				r1.data.m44()[index][0] = r3.data.v3()[0];
				r1.data.m44()[index][1] = r3.data.v3()[1];
				r1.data.m44()[index][2] = r3.data.v3()[2];
				return r1.data.m44();
			}
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the element of the matrix (the evaluated expression mat) 
	corresponding to the value of the column index (the evaluated expression cidx)
	and the value of the row index (the evaluated expression ridx) 
	or return 0 
	if mat does not evaluate to a vector or cidx does not evaluate to an integer in [0,3]
	or ridx does not evaluate to an integer in [0,3].
	*/
	Result GetMatElement(Expression &mat, Expression &cidx, Expression &ridx);
#else
	EXPR_THREE_EXPR_CLASS(GetMatElement,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_MATRIX &&
			(r2.type == Result::RS_INT || r2.type == Result::RS_UINT) &&
			(r3.type == Result::RS_INT || r3.type == Result::RS_UINT)) {
				int col = (int)r2.toInt();
				if (col>=0 && col<4) {
					int row = (int)r3.toInt();
					if (row>=0 && row<4)
						return r1.data.m44()[col][row];
				}
			}
			return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the modified matrix (the evaluated expression mat) 
	with the element at the column index (the evaluated expression cidx)
	and the row index (the evaluated expression ridx)
	replaced by a specified number (the evaluated expression val)
	or return 0 
	if mat does not evaluate to a matrix or cidx does not evaluate to an integer in [0,3]
	or ridx does not evaluate to an integer in [0,3] or val does not evaluate to a number.
	*/
	Result SetMatElement(Expression &mat, Expression &cidx, Expression &ridx, Expression &val);
#else
	EXPR_FOUR_EXPR_CLASS(SetMatElement,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		Result r4 = expr4.eval(b);
		if (r1.type == Result::RS_MATRIX &&
			(r2.type == Result::RS_INT || r2.type == Result::RS_UINT) &&
			(r3.type == Result::RS_INT || r3.type == Result::RS_UINT) &&
			(r4.type == Result::RS_FLOAT || r4.type == Result::RS_INT || r4.type == Result::RS_UINT))
		{
			int col = (int)r2.toInt();
			if (col>=0 && col<4) {
				int row = (int)r3.toInt();
				if (row>=0 && row<4) {
					r1.data.m44()[col][row] = r4.toFloat();
					return r1.data.m44();
				}
			}
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix corresponding to pointing the camera
	from a specified eye position (the evaluated expression eye)
	to a specified point in the scene (the evaluated expression at)
	with a specified up direction (the evaluated expression up) 
	or return 0
	if eye does not evaluate to a vector or at does not evaluate to a vector
	or up does not evaluate to a vector
	or the up direction is parallel to the line of sight.
	*/
	Result LookAtMat(Expression &eye, Expression &at, Expression &up);
#else
	EXPR_THREE_EXPR_CLASS_EXPORT(EA3_EXPORT,LookAtMat);
#endif

#ifdef DOXYGEN
	/*! 
	Return the inverse of a matrix (evaluated expression mat)
	or return 0
	if mat does not evaluate to a matrix.
	*/
	Result InvertMat(Expression &mat);
#else
	EXPR_ONE_EXPR_CLASS(InvertMat,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_MATRIX)
			return XbMatrix(r.data.m44().inverse());
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the translation vector contained in a matrix (evaluated expression mat)
	or return 0
	if mat does not evaluate to a matrix.
	*/
	Result Mat2TranslationVec(Expression &mat);
#else
	EXPR_ONE_EXPR_CLASS(Mat2TranslationVec,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_MATRIX) {
			const XbVec3f &tx = r.data.m44().getTranslation();
			return XbVec3f(tx[0], tx[1], tx[2]);
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the transpose of a matrix (evaluated expression mat)
	or return 0
	if mat does not evaluate to a matrix.
	*/
	Result TransposeMat(Expression &mat);
#else
	EXPR_ONE_EXPR_CLASS(TransposeMat,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_MATRIX) 
			return r.data.m44().transpose();
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return a matrix (evaluated expression mat)
	with only the 3X3 rotation part transposed
	or return 0
	if mat does not evaluate to a matrix.
	*/
	Result Transpose3Mat(Expression &mat);
#else
	EXPR_ONE_EXPR_CLASS(Transpose3Mat,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_MATRIX) {
			const XbMatrix &m = r.data.m44();
			return XbMatrix(
				m[0][0], m[1][0], m[2][0], m[0][3],
				m[0][1], m[1][1], m[2][1], m[1][3],
				m[0][2], m[1][2], m[2][2], m[2][3],
				m[3][0], m[3][1], m[3][2], m[3][3] );
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix corresponding to a rotation (all angles in radians)
	represented by a vector of euler angles (the evaluated expression angles)
	or return 0
	if angles does not evaluate to a vector.
	NOTE: order of evaluation is XYZ, rotation is STATIC
	*/
	Result Euler2Mat(Expression &angles);
#else
	EXPR_ONE_EXPR_CLASS(Euler2Mat,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3)
				return XbMatrix(XbEuler(r.data.v3(), XbEuler::XYZ, XbEuler::STATIC));
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix corresponding to a rotation expressed as quaternion coefficients
	first imaginary coefficient (the evaluated expression qx)
	second imaginary coefficient (the evaluated expression qy)
	third imaginary coefficient (the evaluated expression qz)
	scalar coefficient (the evaluated expression qw)
	or return 0
	if qx does not evaluate to a number or qy does not evaluate to a number
	or qz does not evaluate to a number or qw does not evaluate to a number.
	*/
	Result Quat2Mat(Expression &qx, Expression &qy, Expression &qz, Expression &qw);
#else
	EXPR_FOUR_EXPR_CLASS(Quat2Mat,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		Result r4 = expr4.eval(b);
		if (
			(r1.type == Result::RS_FLOAT || r1.type == Result::RS_INT || r1.type == Result::RS_UINT) &&
			(r2.type == Result::RS_FLOAT || r2.type == Result::RS_INT || r2.type == Result::RS_UINT) &&
			(r3.type == Result::RS_FLOAT || r3.type == Result::RS_INT || r3.type == Result::RS_UINT) &&
			(r4.type == Result::RS_FLOAT || r4.type == Result::RS_INT || r4.type == Result::RS_UINT)) 
		{
			XbMatrix mat;
			mat.setRotate(XbRotation(r1.toFloat(), r2.toFloat(), r3.toFloat(), r4.toFloat()));
			return mat;
		}
		return Result();
	});

#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix which produces a rotation about the axis vector (the evaluated expression axis) 
	by an angle, in radians (the evaluated expression angle)
	or return 0 
	if axis does not evaluate to a vector or angle does not evaluate to a number.
	*/
	Result AxisRot2Mat(Expression &axis, Expression &angle);
#else
	EXPR_TWO_EXPR_CLASS(AxisRot2Mat,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_VEC3 &&
			(r2.type == Result::RS_FLOAT || r2.type == Result::RS_UINT || r2.type == Result::RS_INT)) 
		{
			XbMatrix mat;
			mat.setRotate(XbRotation(r1.data.v3(), r2.toFloat()));
			return mat;
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix which produces a rotation from one vector (the evaluated expression vec1) 
	to a second vector (the evaluated expression vec2)
	or return 0 
	if vec1 does not evaluate to a vector or vec2 does not evaluate to a vector.
	*/
	Result FromTo2Mat(Expression &vec1, Expression &vec2);
#else
	EXPR_TWO_EXPR_CLASS(FromTo2Mat,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		if (r1.type == Result::RS_VEC3 && r2.type == Result::RS_VEC3) {
			XbMatrix mat;
			mat.setRotate(XbRotation(r1.data.v3(), r2.data.v3()));
			return mat;
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix which will produce a (potentially) non-uniform scaling
	represented by a vector (the evaluated expression scl)
	or return 0
	if scl does not evaluate to a vector.
	*/
	Result Scale2Mat(Expression &scl);
#else
	EXPR_ONE_EXPR_CLASS(Scale2Mat,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
				XbMatrix mat;
				mat.setScale(r.data.v3());
				return mat;
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix which will produce a translation represented by a vector (the evaluated expression trn)
	or return 0
	if trn does not evaluate to a vector.
	*/
	Result Translation2Mat(Expression &trn);
#else
	EXPR_ONE_EXPR_CLASS(Translation2Mat,{
		Result r = expr.eval(b);
		if (r.type == Result::RS_VEC3) {
			XbMatrix mat;
			mat.setTranslate(r.data.v3());
			return mat;
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
	Return the matrix which will produce a rotation which blends
	one rotation matrix (the evaluated expression mat1)
	with a second rotation matrix (the evaluated expression mat2)
	using a blending factor between 0 and 1 (the evaluated expression p) 
	or return 0
	if mat1 does not evaluate to a matrix or mat2 does not evaluate to a matrix
	or p does not evaluate to a number in [0,1].
	NOTE: both matrices must implement rotations, and there can be no shearing 
	or scaling in either matrix
	*/
	Result BlendMats(Expression &mat1, Expression &mat2, Expression &p);
#else
	EXPR_THREE_EXPR_CLASS(BlendMats,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_MATRIX && r2.type == Result::RS_MATRIX &&
			(r3.type == Result::RS_FLOAT || r3.type == Result::RS_INT || r3.type == Result::RS_UINT))
		{
			float t = r3.toFloat();
			if (t>=0 && t<=1) {
				XbMatrix mat;
				interpolateMatrices(r1.data.m44(), r2.data.m44(), t, mat);
				return mat;
			}
		}
		return Result();
	});
#endif

#ifdef DOXYGEN
	/*! 
  Return the matrix which will produce a rotation which blends
  one rotation matrix (the evaluated expression mat1)
  with a second rotation matrix (the evaluated expression mat2)
  using a blending factor between 0 and 1 (the evaluated expression p) 
  or return 0
  if mat1 does not evaluate to a matrix or mat2 does not evaluate to a matrix
  or p does not evaluate to a number in [0,1].
	NOTE: both matrices must implement rotations, there can be no shearing
	in either matrix, and any scaling must be uniform 
	*/
	Result BlendScaledMats(Expression &mat1, Expression &mat2, Expression &p);
#else
	EXPR_THREE_EXPR_CLASS(BlendScaledMats,{
		Result r1 = expr1.eval(b);
		Result r2 = expr2.eval(b);
		Result r3 = expr3.eval(b);
		if (r1.type == Result::RS_MATRIX && r2.type == Result::RS_MATRIX &&
			(r3.type == Result::RS_FLOAT || r3.type == Result::RS_INT || r3.type == Result::RS_UINT))
		{
			float t = r3.toFloat();
			if (t>=0 && t<=1) {
				XbMatrix mat;
				interpolateMatricesWithScale(r1.data.m44(), r2.data.m44(), t, mat);
				return mat;
			}
		}
		return Result();
	});
#endif

} // namespace EA3Expr
#endif // _EA3_EXPR_LINEAR_H_