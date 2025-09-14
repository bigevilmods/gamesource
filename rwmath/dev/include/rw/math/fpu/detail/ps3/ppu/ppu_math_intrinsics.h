// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_PPU_MATH_INTRINSICS_H
#define RW_MATH_FPU_PPU_MATH_INTRINSICS_H

#include <ppu_intrinsics.h>

namespace rw
{

namespace math
{

namespace fpu
{

namespace detail
{

#ifdef __SNC__

#define FMADD(a, b, c) \
    __builtin_fmadd(a,b,c)

#define FMADDS(a, b, c) \
    __builtin_fmadds(a,b,c)

#define FSQRTS(a) \
    __fsqrts(a)

#define FSQRT(a) \
    __builtin_fsqrt(a)

#define FNMSUB(a, b, c) \
    __builtin_fnmsub(a,b,c)

#define FNMSUBS(a, b, c) \
    __builtin_fnmsubs(a,b,c)

RW_MATH_INLINE double fcfid(double x) {
  return __builtin_fcfid(x);
}

RW_MATH_INLINE double fctiwz(double x) {
  return __builtin_fctiwz(x);
}

#define RW_MATH_FPU_Fabs( a ) \
    __builtin_fabs((a))

#define RW_MATH_FPU_Fsel( a, b, c ) \
    __builtin_fsel((a), (b), (c))

#define RW_MATH_FPU_Fsels( a, b, c ) \
    __builtin_fsels((a), (b), (c))

#define RW_MATH_FPU_FRecipEst( a ) \
    __builtin_fre(a)

#define RW_MATH_FPU_DRecipEst(a)\
    RW_MATH_FPU_FRecipEst(a)

#define RW_MATH_FPU_FRecipFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	float nmsub = FNMSUBS(a, estimate, 1.0f); \
	r = FMADDS(estimate, nmsub, estimate); \
} \

#define RW_MATH_FPU_DRecipFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
    double nmsub = FNMSUB(a, estimate, 1.0); \
	r = FMADD(estimate, nmsub, estimate); \
} \

#define RW_MATH_FPU_DInvSqrtEst( a ) \
	__builtin_frsqrte(a)

#define RW_MATH_FPU_FInvSqrtEst( a ) \
	static_cast<float>(RW_MATH_FPU_DInvSqrtEst(a))

#define RW_MATH_FPU_FInvSqrtFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	float estimate_sqr = estimate * estimate; \
	float nmsub = FNMSUBS(a, estimate_sqr, 1.0f); \
	float estimate_half = 0.5f * estimate; \
	r = FMADDS(estimate_half, nmsub, estimate); \
} \

#define RW_MATH_FPU_DInvSqrtFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	double estimate_sqr = estimate * estimate; \
	double nmsub = FNMSUB(a, estimate_sqr, 1.0); \
	double estimate_half = 0.5 * estimate; \
	r = FMADD(estimate_half, nmsub, estimate); \
} \

#define RW_MATH_FPU_FSqrtFast( r, a ) \
{ \
	RW_MATH_FPU_FInvSqrtFast( r, a ); \
	r = r * a; \
	r = RW_MATH_FPU_Fsels(r, r, 0.0f); \
} \

#define RW_MATH_FPU_DSqrtFast( r, a ) \
{ \
	RW_MATH_FPU_DInvSqrtFast( r, a ); \
	r = r * a; \
	r = RW_MATH_FPU_Fsel(r, r, 0.0); \
} \

#else

#define FMADD(a, b, c) \
    __fmadd(a,b,c)

#define FMADDS(a, b, c) \
    __fmadds(a,b,c)

#define FSQRTS(a) \
    __fsqrts(a)

#define FSQRT(a) \
    __fsqrt(a)

#define FNMSUB(a, b, c) \
    __fnmsub(a,b,c)

#define FNMSUBS(a, b, c) \
    __fnmsubs(a,b,c)

#define FMUL(a, b) \
    __fmul(a,b)

#define FMULS(a, b) \
    __fmuls(a,b)

RW_MATH_INLINE double fcfid(double x) {
  float r;
  asm("fcfid %0,%1" : "=f"(r) : "f"(x));
  return r;
}

RW_MATH_INLINE double fctiwz(double x) {
  double r;
  asm("fctiwz %0,%1" : "=f"(r) : "f"(x));
  return r;
}

#define RW_MATH_FPU_Fabs( a ) \
    __fabs((a))

#define RW_MATH_FPU_Fsels( a, b, c ) \
    __fsels((a), (b), (c))

#define RW_MATH_FPU_Fsel( a, b, c ) \
    __fsel((a), (b), (c))

#define RW_MATH_FPU_FRecipEst( a ) \
    __fres(a)

#define RW_MATH_FPU_DRecipEst(a)\
    RW_MATH_FPU_FRecipEst(a)

#define RW_MATH_FPU_FRecipFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	float nmsub = FNMSUBS(a, estimate, 1.0f); \
	r = FMADDS(estimate, nmsub, estimate); \
} \

#define RW_MATH_FPU_DRecipFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	double nmsub = FNMSUB(a, estimate, 1.0); \
	r = FMADD(estimate, nmsub, estimate); \
} \

#define RW_MATH_FPU_DInvSqrtEst( a ) \
	__frsqrte(a)

#define RW_MATH_FPU_FInvSqrtEst( a ) \
	static_cast<float>(RW_MATH_FPU_DInvSqrtEst(a))

#define RW_MATH_FPU_FInvSqrtFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	float estimate_sqr = FMULS(estimate, estimate); \
	float nmsub = FNMSUBS(a, estimate_sqr, 1.0f); \
	float estimate_half = FMULS(0.5f, estimate); \
	r = FMADDS(estimate_half, nmsub, estimate); \
} \

#define RW_MATH_FPU_DInvSqrtFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	double estimate_sqr = FMUL(estimate, estimate); \
	double nmsub = FNMSUB(a, estimate_sqr, 1.0); \
	double estimate_half = FMUL(0.5, estimate); \
	r = FMADD(estimate_half, nmsub, estimate); \
} \

#define RW_MATH_FPU_FSqrtFast( r, a ) \
{ \
	float invsqrt_estimate; \
	RW_MATH_FPU_FInvSqrtFast( invsqrt_estimate, a ); \
	r = FMULS(invsqrt_estimate, a); \
	r = RW_MATH_FPU_Fsels(r, r, 0.0f); \
} \

#define RW_MATH_FPU_DSqrtFast( r, a ) \
{ \
	double invsqrt_estimate; \
	RW_MATH_FPU_DInvSqrtFast( invsqrt_estimate, a ); \
	r = FMUL(invsqrt_estimate, a); \
	r = RW_MATH_FPU_Fsel(r, r, 0.0); \
} \

#endif

} // namespace detail

} // namespace fpu

} // namespace math

} // namespace rw

#endif // RW_MATH_FPU_PPU_MATH_INTRINSICS_H

