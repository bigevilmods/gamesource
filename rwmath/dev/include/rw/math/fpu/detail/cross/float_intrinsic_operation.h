// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H
#define RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{
    RW_MATH_FORCE_INLINE double GenericFSel(double a, double b, double c )
    {
        return ( a >= 0.0 ) ? b : c;
    }

    RW_MATH_FORCE_INLINE float GenericFSel(float a, float b, float c )
    {
        return ( a >= 0.0f ) ? b : c;
    }

#define RW_MATH_FPU_Fsel( a, b, c ) \
    ::rw::math::fpu::detail::GenericFSel((a), (b), (c))

#define RW_MATH_FPU_FRecipEst( a ) \
    1.0f/(a)

#define RW_MATH_FPU_DRecipEst(a)\
    1.0/(a)

#define RW_MATH_FPU_FRecipFast( r, a ) \
{ \
	r = RW_MATH_FPU_FRecipEst( a ); \
} \

#define RW_MATH_FPU_DRecipFast( r, a ) \
{ \
	r = RW_MATH_FPU_DRecipEst( a ); \
} \

#define RW_MATH_FPU_FInvSqrtEst( a ) \
    1.0f/(sqrtf(a))

#define RW_MATH_FPU_DInvSqrtEst(a)\
    1.0/(sqrt(a))

#define RW_MATH_FPU_FInvSqrtFast( r, a ) \
{ \
	r = RW_MATH_FPU_FInvSqrtEst( a ); \
} \

#define RW_MATH_FPU_DInvSqrtFast( r, a ) \
{ \
	r = RW_MATH_FPU_DInvSqrtEst( a ); \
} \

#define RW_MATH_FPU_FSqrtFast( r, a ) \
{ \
	r = sqrtf(a); \
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

