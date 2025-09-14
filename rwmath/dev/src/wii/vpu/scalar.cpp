// (c) Electronic Arts. All Rights Reserved.

// The Wii has some optimizer bugs that are causing invalid code to be generated.  Some functions
// are being compiled "non-inlined" in this file to avoid the bugs.
#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/detail/scalar_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{

    VecFloat SqrtEstimate(VecFloat::InParam a)
    {
        using namespace ::rw::math::ps::detail;
        VecFloat result;
        RW_MATH_PS_SquareRoot(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

    VecFloat SqrtFast(VecFloat::InParam a)
    {
        using namespace ::rw::math::ps::detail;
        VecFloat result;
        RW_MATH_PS_SquareRootFast(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

    VecFloat Sqrt(VecFloat::InParam a)
    {
        using namespace ::rw::math::ps::detail;
        VecFloat result;
        RW_MATH_PS_SquareRoot(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

	VecFloat ReciprocalEstimate(VecFloat::InParam a)
    {
        using namespace ::rw::math::ps::detail;
        VecFloat result;
        result.mV.ps0 = ps_res(a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

    VecFloat ReciprocalFast(VecFloat::InParam a)
    {
        VecFloat result;
        RW_MATH_PS_ReciprocalFast(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

	VecFloat Reciprocal(VecFloat::InParam a)
    {
        VecFloat result;
        RW_MATH_PS_Reciprocal(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

	VecFloat InvSqrtFast(VecFloat::InParam a)
    {
        using namespace ::rw::math::ps::detail;
        VecFloat result;
        RW_MATH_PS_ReciprocalSquareRootFast(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

    VecFloat InvSqrt(VecFloat::InParam a)
    {
        using namespace ::rw::math::ps::detail;
        VecFloat result;
        RW_MATH_PS_ReciprocalSquareRoot(result.mV.ps0, a.mV.ps0);
        result.mV.ps1 = result.mV.ps0;
        return result;
    }

	// VectorIntrinsic versions
	VectorIntrinsic Reciprocal(VectorIntrinsicInParam a)
	{
        VectorIntrinsic ret;
		RW_MATH_PS_Reciprocal(ret.ps0, a.ps0);
		RW_MATH_PS_Reciprocal(ret.ps1, a.ps1);
        return ret;
	}

	VectorIntrinsic ReciprocalFast(VectorIntrinsicInParam a)
    {
        VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalFast(ret.ps0, a.ps0);
		RW_MATH_PS_ReciprocalFast(ret.ps1, a.ps1);
        return ret;
    }

	VectorIntrinsic ReciprocalFastV2(VectorIntrinsicInParam a)
    {
        VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalFast(ret.ps0, a.ps0);
		ret.ps1 = a.ps1;
        return ret;
    }

	VectorIntrinsic ReciprocalEstimate(VectorIntrinsicInParam a)
	{
		VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalEstimate(ret.ps0, a.ps0);
		RW_MATH_PS_ReciprocalEstimate(ret.ps1, a.ps1);
        return ret;
	}

    VectorIntrinsic Sqrt(VectorIntrinsicInParam a)
	{
		VectorIntrinsic ret;
		RW_MATH_PS_SquareRoot(ret.ps0, a.ps0);
	    RW_MATH_PS_SquareRoot(ret.ps1, a.ps1);
		return ret;
	}

	VectorIntrinsic SqrtFast(VectorIntrinsicInParam a)
	{
	    VectorIntrinsic ret;
		RW_MATH_PS_SquareRootFast(ret.ps0, a.ps0);
	    RW_MATH_PS_SquareRootFast(ret.ps1, a.ps1);
		return ret;
	}

	VectorIntrinsic SqrtFastV2(VectorIntrinsicInParam a)
	{
	    VectorIntrinsic ret;
		RW_MATH_PS_SquareRootFast(ret.ps0, a.ps0);
	    ret.ps1 = a.ps1;
		return ret;
	}

	VectorIntrinsic SqrtEstimate(VectorIntrinsicInParam a)
	{
	    VectorIntrinsic ret;
		RW_MATH_PS_SquareRootEstimate(ret.ps0, a.ps0);
	    RW_MATH_PS_SquareRootEstimate(ret.ps1, a.ps1);
		return ret;
	}

	VectorIntrinsic InvSqrt(VectorIntrinsicInParam a)
	{
		VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalSquareRoot(ret.ps0, a.ps0);
		RW_MATH_PS_ReciprocalSquareRoot(ret.ps1, a.ps1);
		return ret;
	}

	VectorIntrinsic InvSqrtFast(VectorIntrinsicInParam a)
	{
	    VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalSquareRootFast(ret.ps0, a.ps0);
		RW_MATH_PS_ReciprocalSquareRootFast(ret.ps1, a.ps1);
		return ret;
	}

	VectorIntrinsic InvSqrtFastV2(VectorIntrinsicInParam a)
	{
	    VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalSquareRootFast(ret.ps0, a.ps0);
		ret.ps1 = a.ps1;
		return ret;
	}

    VectorIntrinsic InvSqrtEstimate(VectorIntrinsicInParam a)
	{
	    VectorIntrinsic ret;
		RW_MATH_PS_ReciprocalSquareRootEstimate(ret.ps0, a.ps0);
		RW_MATH_PS_ReciprocalSquareRootEstimate(ret.ps1, a.ps1);
		return ret;
	}

}
}
}
}