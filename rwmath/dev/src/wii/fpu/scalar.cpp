// (c) Electronic Arts. All Rights Reserved.
#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/fpu/scalar.h"

namespace rw
{
namespace math
{
namespace fpu
{

namespace detail
{

float Sqrt(float a)
{
    using namespace ::rw::math::ps::detail;
    mw_ps ps_result;
    RW_MATH_PS_SquareRoot(ps_result, a);
    return ps_extract0( ps_result );
}

float SqrtFast(float a)
{
	using namespace ::rw::math::ps::detail;
    mw_ps ps_result;
    RW_MATH_PS_SquareRootFast(ps_result, a);
    return ps_extract0( ps_result );
}

} // namespace detail
} // namespace fpu
} // namespace math
} // namespace rw

