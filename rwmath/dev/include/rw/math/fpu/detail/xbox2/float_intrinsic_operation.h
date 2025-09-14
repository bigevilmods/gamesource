// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H
#define RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{

#define RW_MATH_FPU_Fabs( a ) \
    __fabs((a))

#define RW_MATH_FPU_Fsel( a, b, c ) \
    __fsel((a), (b), (c))

#define RW_MATH_FPU_FRecipEst( a ) \
    __fres(a)

#define RW_MATH_FPU_DRecipEst(a)\
    RW_MATH_FPU_FRecipEst(a)

#define RW_MATH_FPU_DInvSqrtEst( a ) \
	__frsqrte(a)

#define RW_MATH_FPU_FInvSqrtEst( a ) \
	static_cast<float>(RW_MATH_FPU_DInvSqrtEst(a))

#define RW_MATH_FPU_FRecipFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
    float nmsub = 1.0f - estimate * a; \
	r = estimate * nmsub + estimate; \
} \

#define RW_MATH_FPU_DRecipFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
    double nmsub = 1.0 - estimate * a; \
	r = estimate * nmsub + estimate; \
} \

#define RW_MATH_FPU_FInvSqrtFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	float nmsub = 1.0f - estimate * estimate * a; \
	r = 0.5f * estimate * nmsub + estimate; \
} \

#define RW_MATH_FPU_DInvSqrtFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
    double nmsub = 1.0 - estimate * estimate * a; \
	r = 0.5 * estimate * nmsub + estimate; \
} \

#define RW_MATH_FPU_FSqrtFast( r, a ) \
{ \
	RW_MATH_FPU_FInvSqrtFast( r, a ); \
	r = r * a; \
	r = static_cast<float>(RW_MATH_FPU_Fsel(r, r, 0.0f)); \
} \

#define RW_MATH_FPU_DSqrtFast( r, a ) \
{ \
	RW_MATH_FPU_DInvSqrtFast( r, a ); \
	r = r * a; \
	r = RW_MATH_FPU_Fsel(r, r, 0.0); \
} \

} // namespace detail
} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

