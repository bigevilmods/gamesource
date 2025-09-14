#include <eabase/eabase.h>
#include "EA3ExprLinear.h"
using namespace EA3Expr;

Result LookAtMat::eval(EA3Base *b) const {
	Result r1 = expr1.eval(b);
	Result r2 = expr2.eval(b);
	Result r3 = expr3.eval(b);
	if (r1.type == Result::RS_VEC3 && r2.type == Result::RS_VEC3 && r3.type == Result::RS_VEC3)
	{
		const XbVec3f &from = r1.data.v3();
		const XbVec3f &to   = r2.data.v3();
		const XbVec3f &up   = r3.data.v3();

		XbVec3f z_primed = from - to;
		z_primed.normalize();
		XbVec3f x_primed = up.cross(z_primed);
		if (x_primed.sqrlength() < 0.0001f)
			return Result(); // line of sight ~parallel to up vector :(
		x_primed.normalize();
		XbVec3f y_primed = z_primed.cross(x_primed);
		y_primed.normalize();

		XbVec3f trans(-from.dot(x_primed), -from.dot(y_primed), -from.dot(z_primed));

		XbMatrix mat(
			x_primed[0], x_primed[1], x_primed[2], trans[0],
			y_primed[0], y_primed[1], y_primed[2], trans[1],
			z_primed[0], z_primed[1], z_primed[2], trans[2],
			0          , 0          , 0          , 1 );
		return mat;
	}
	return Result();
}

//Result Quat2Mat::eval(EA3Base *b) const {
//	Result r1 = expr1.eval(b);
//	Result r2 = expr2.eval(b);
//	Result r3 = expr3.eval(b);
//	Result r4 = expr4.eval(b);
//	if (
//		(r1.type == Result::RS_FLOAT || r1.type == Result::RS_INT || r1.type == Result::RS_UINT) &&
//		(r2.type == Result::RS_FLOAT || r2.type == Result::RS_INT || r2.type == Result::RS_UINT) &&
//		(r3.type == Result::RS_FLOAT || r3.type == Result::RS_INT || r3.type == Result::RS_UINT) &&
//		(r4.type == Result::RS_FLOAT || r4.type == Result::RS_INT || r4.type == Result::RS_UINT)) 
//	{
//		XbMatrix mat;
//		mat.setRotate(XbRotation(r1.toFloat(), r2.toFloat(), r3.toFloat(), r4.toFloat()));
//		return mat;
//	}
//	return Result();
//}
