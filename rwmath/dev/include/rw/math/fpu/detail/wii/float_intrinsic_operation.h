// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H
#define RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{

    RW_MATH_FORCE_INLINE double GenericFabs(const register double a)
    {
        register double result;
        asm { fabs result, a };
        return result;
    }

#define RW_MATH_FPU_Fabs( a ) \
    ::rw::math::fpu::detail::GenericFabs((a))

    RW_MATH_FORCE_INLINE double GenericFSel(const register double a, const register double b, const register double c )
    {
        register double result;
        asm { fsel result, a, b, c };
        return result;
    }

#define RW_MATH_FPU_Fsel( a, b, c ) \
    ::rw::math::fpu::detail::GenericFSel((a), (b), (c))

    RW_MATH_FORCE_INLINE double GenericFRecipEst(const register double a)
    {
        register double result;
        asm { fres result, a };
        return result;
    }

	RW_MATH_FORCE_INLINE double GenericFMul(const register double a, const register double b)
    {
        register double result;
        asm { fmul result, a, b };
        return result;
    }

	RW_MATH_FORCE_INLINE double GenericFNmsub(const register double a, const register double b, const register double c)
    {
        register double result;
        asm { fnmsub result, a, b, c };
        return result;
    }

	RW_MATH_FORCE_INLINE double GenericFMadd(const register double a, const register double b, const register double c)
    {
        register double result;
        asm { fmadd result, a, b, c };
        return result;
    }

#define RW_MATH_FPU_FRecipEst( a ) \
    static_cast<float>(::rw::math::fpu::detail::GenericFRecipEst(a))

#define RW_MATH_FPU_DRecipEst(a)\
    ::rw::math::fpu::detail::GenericFRecipEst(a)

#define RW_MATH_FPU_DRecipFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
    double nmsub = ::rw::math::fpu::detail::GenericFNmsub(a, estimate, 1.0); \
	r = ::rw::math::fpu::detail::GenericFMadd(estimate, nmsub, estimate); \
} \

	RW_MATH_FORCE_INLINE double GenericFInvSqrtEst(const register double a)
    {
        register double result;
        asm { frsqrte result, a };
        return result;
    }

#define RW_MATH_FPU_DInvSqrtEst(a)\
    ::rw::math::fpu::detail::GenericFInvSqrtEst(a)

#define RW_MATH_FPU_DInvSqrtFast( r, a ) \
{ \
	double estimate = RW_MATH_FPU_DInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	double estimate_sqr = ::rw::math::fpu::detail::GenericFMul(estimate, estimate); \
	double nmsub = ::rw::math::fpu::detail::GenericFNmsub(a, estimate_sqr, 1.0); \
	double estimate_half = ::rw::math::fpu::detail::GenericFMul(estimate, 0.5); \
	r = ::rw::math::fpu::detail::GenericFMadd(estimate_half, nmsub, estimate); \
} \

#define RW_MATH_FPU_DSqrtFast( r, a ) \
{ \
	double invsqrt_estimate; \
	RW_MATH_FPU_DInvSqrtFast( invsqrt_estimate, a ); \
	r = ::rw::math::fpu::detail::GenericFMul(invsqrt_estimate, a); \
	r = RW_MATH_FPU_Fsel(r, r, 0.0); \
} \

} // namespace detail
} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

