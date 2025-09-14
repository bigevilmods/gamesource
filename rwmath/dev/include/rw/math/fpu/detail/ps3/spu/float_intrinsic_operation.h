// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H
#define RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

#include "rw/math/detail/base.h"
#include "rw/math/detail/ps3/spu/spu_macros.h"

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{

    RW_MATH_FORCE_INLINE float GenericFSel(float a, float b, float c )
    {
        vector float floatVec = __builtin_spu_promote_8( a, 0 );  
        vector unsigned int mask = __builtin_spu_cmpgt_6( (vector float)(0.0f), floatVec );
        vector float resultVec = __builtin_spu_sel_8 ( __builtin_spu_promote_8(b,0), __builtin_spu_promote_8(c,0), mask );
        float result = __builtin_spu_extract_8 ( resultVec, 0 );
        return result;
    }

    RW_MATH_FORCE_INLINE double GenericFSel(double a, double b, double c )
    {
        // Note: This could be optimized by examining the bit pattern of the
        // double and creating masks.  But the default compiler generated code
        // actually seems very fast.  Perhaps the compiler is doing something
        // intelligent with this code to remove branches??
        return ( a >= 0.0 ) ? b : c;
    }

#define RW_MATH_FPU_Fsel( a, b, c ) \
    ::rw::math::fpu::detail::GenericFSel((a), (b), (c))

    RW_MATH_FORCE_INLINE float FRecipEst(float a)
    {
        vector float floatVec = __builtin_spu_promote_8( a, 0 );  
        vector float resultVec = (vector float)__builtin_si_fi((vector signed char)(floatVec), __builtin_si_frest((vector signed char)(floatVec)));
        return __builtin_spu_extract_8 ( resultVec, 0 );
    }

	RW_MATH_FORCE_INLINE float FMuls(float a, float b)
    {
        vector float floatVecA = __builtin_spu_promote_8( a, 0 );  
        vector float floatVecB = __builtin_spu_promote_8( b, 0 );
        vector float resultVec = __builtin_spu_mul_0(floatVecA, floatVecB);
        return __builtin_spu_extract_8 ( resultVec, 0 );
    }

	RW_MATH_FORCE_INLINE float FMAdds(float a, float b, float c)
    {
        vector float floatVecA = __builtin_spu_promote_8( a, 0 );  
        vector float floatVecB = __builtin_spu_promote_8( b, 0 );
		vector float floatVecC = __builtin_spu_promote_8( c, 0 );
        vector float resultVec = __builtin_spu_madd_1(floatVecA, floatVecB, floatVecC);
        return __builtin_spu_extract_8 ( resultVec, 0 );
    }

	RW_MATH_FORCE_INLINE float FNMSubs(float a, float b, float c)
    {
        vector float floatVecA = __builtin_spu_promote_8( a, 0 );  
        vector float floatVecB = __builtin_spu_promote_8( b, 0 );
		vector float floatVecC = __builtin_spu_promote_8( c, 0 );
        vector float resultVec = __builtin_spu_nmsub_0(floatVecA, floatVecB, floatVecC);
        return __builtin_spu_extract_8 ( resultVec, 0 );
    }

#define RW_MATH_FPU_FRecipEst( a ) \
    ::rw::math::fpu::detail::FRecipEst(a)

#define RW_MATH_FPU_DRecipEst(a)\
    (1.0/(a))

#define RW_MATH_FPU_FRecipFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FRecipEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
    float nmsub = ::rw::math::fpu::detail::FNMSubs(a, estimate, 1.0f); \
	r = ::rw::math::fpu::detail::FMAdds(estimate, nmsub, estimate); \
} \

#define RW_MATH_FPU_DRecipFast( r, a ) \
{ \
	r = RW_MATH_FPU_DRecipEst( a ); \
} \

	RW_MATH_FORCE_INLINE float FInvSqrtEst(float a)
    {
        vector float floatVec = __builtin_spu_promote_8( a, 0 );  
        vector float resultVec = (vector float)__builtin_si_fi((vector signed char)(floatVec), __builtin_si_frsqest((vector signed char)(floatVec)));;
        return __builtin_spu_extract_8 ( resultVec, 0 );
    }

#define RW_MATH_FPU_FInvSqrtEst( a ) \
	::rw::math::fpu::detail::FInvSqrtEst(a)

#define RW_MATH_FPU_DInvSqrtEst( a ) \
	(1.0/(sqrt(a)))

#define RW_MATH_FPU_FInvSqrtFast( r, a ) \
{ \
	float estimate = RW_MATH_FPU_FInvSqrtEst( a ); \
	/* Do one iteration of Newton-Raphson estimation */ \
	float estimate_sqr = ::rw::math::fpu::detail::FMuls(estimate, estimate); \
	float nmsub = ::rw::math::fpu::detail::FNMSubs(a, estimate_sqr, 1.0f); \
	float estimate_half = ::rw::math::fpu::detail::FMuls(estimate, 0.5f); \
	r = ::rw::math::fpu::detail::FMAdds(estimate_half, nmsub, estimate); \
} \

#define RW_MATH_FPU_DInvSqrtFast( r, a ) \
{ \
	r = RW_MATH_FPU_DInvSqrtEst(a); \
} \

#define RW_MATH_FPU_FSqrtFast( r, a ) \
{ \
	RW_MATH_FPU_FInvSqrtFast( r, a ); \
	r = ::rw::math::fpu::detail::FMuls(r, a); \
	r = RW_MATH_FPU_Fsel(r, r, 0.0f); \
} \

#define RW_MATH_FPU_DSqrtFast( r, a ) \
{ \
	r = sqrt(a); \
} \

} // namespace detail
} // namespace fpu
} // namespace math
} // namespace rw

#endif // RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H


