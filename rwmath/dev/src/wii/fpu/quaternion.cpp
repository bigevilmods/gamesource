// (c) Electronic Arts. All Rights Reserved.

// The Wii has some optimizer bugs that are causing invalid code to be generated.  Some functions
// are being compiled "non-inlined" in this file to avoid the bugs.
#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/fpu/quaternion_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{
	template<> 
	QuaternionTemplate<float> Lerp(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q0_ps0 = ps_load_0(&q0.x);
		mw_ps q0_ps1 = ps_load_0(&q0.z);
		mw_ps q1_ps0 = ps_load_0(&q1.x);
		mw_ps q1_ps1 = ps_load_0(&q1.z);
		mw_ps t_ps = ps_set( t, t );

		RW_MATH_PS_Lerp(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, t_ps, t_ps);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
	template<> 
	QuaternionTemplate<float> NormalizeFast(const QuaternionTemplate<float>& vec)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q_ps0 = ps_load_0(&vec.x);
		mw_ps q_ps1 = ps_load_0(&vec.z);

		RW_MATH_PS_NormalizeFast(ret_ps0, ret_ps1, q_ps0, q_ps1);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
	template<> 
	QuaternionTemplate<float> NormalizeEstimate(const QuaternionTemplate<float>& vec)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q_ps0 = ps_load_0(&vec.x);
		mw_ps q_ps1 = ps_load_0(&vec.z);

		RW_MATH_PS_NormalizeEstimate(ret_ps0, ret_ps1, q_ps0, q_ps1);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
	template<> 
	QuaternionTemplate<float> Normalize(const QuaternionTemplate<float>& vec)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q_ps0 = ps_load_0(&vec.x);
		mw_ps q_ps1 = ps_load_0(&vec.z);

		RW_MATH_PS_Normalize(ret_ps0, ret_ps1, q_ps0, q_ps1);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
	template<> 
	QuaternionTemplate<float> NLerpEstimate(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q0_ps0 = ps_load_0(&q0.x);
		mw_ps q0_ps1 = ps_load_0(&q0.z);
		mw_ps q1_ps0 = ps_load_0(&q1.x);
		mw_ps q1_ps1 = ps_load_0(&q1.z);
		mw_ps t_ps = ps_set( t, t );

		RW_MATH_PS_NLerpEstimate(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, t_ps, t_ps);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
	template<> 
	QuaternionTemplate<float> NLerpFast(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q0_ps0 = ps_load_0(&q0.x);
		mw_ps q0_ps1 = ps_load_0(&q0.z);
		mw_ps q1_ps0 = ps_load_0(&q1.x);
		mw_ps q1_ps1 = ps_load_0(&q1.z);
		mw_ps t_ps = ps_set( t, t );

		RW_MATH_PS_NLerpFast(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, t_ps, t_ps);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
	template<> 
	QuaternionTemplate<float> NLerp(const QuaternionTemplate<float>& q0, const QuaternionTemplate<float>& q1, float t)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps ret_ps0, ret_ps1;
		mw_ps q0_ps0 = ps_load_0(&q0.x);
		mw_ps q0_ps1 = ps_load_0(&q0.z);
		mw_ps q1_ps0 = ps_load_0(&q1.x);
		mw_ps q1_ps1 = ps_load_0(&q1.z);
		mw_ps t_ps = ps_set( t, t );

		RW_MATH_PS_NLerp(ret_ps0, ret_ps1, q0_ps0, q0_ps1, q1_ps0, q1_ps1, t_ps, t_ps);

		QuaternionTemplate<float> result;
		ps_store_0( ret_ps0, &result.x );
		ps_store_0( ret_ps1, &result.z );
		return result;
	}
}
}
}