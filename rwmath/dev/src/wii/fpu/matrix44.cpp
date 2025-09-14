// (c) Electronic Arts. All Rights Reserved.

// This function fails to execute properly when optimizations are disabled.
// I suspect this is due to a hardware bug when paired registers are not
// saved and restored properly.

#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/fpu/matrix44affine_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{

	template<>
	Vector4Template<float> Mult(const Vector4Template<float> &v, const Matrix44Template<float> &m)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps result_ps0, result_ps1;
		mw_ps v4_ps0 = ps_load_0(&v.x);
		mw_ps v4_ps1 = ps_load_0(&v.z);
		mw_ps xAxis_ps0 = ps_load_0(&m.xAxis.x);
		mw_ps xAxis_ps1 = ps_load_0(&m.xAxis.z);
		mw_ps yAxis_ps0 = ps_load_0(&m.yAxis.x);
		mw_ps yAxis_ps1 = ps_load_0(&m.yAxis.z);
		mw_ps zAxis_ps0 = ps_load_0(&m.zAxis.x);
		mw_ps zAxis_ps1 = ps_load_0(&m.zAxis.z);
		mw_ps wAxis_ps0 = ps_load_0(&m.wAxis.x);
		mw_ps wAxis_ps1 = ps_load_0(&m.wAxis.z);
		
		RW_MATH_PS_MultV4M44(result_ps0, result_ps1, v4_ps0, v4_ps1, xAxis_ps0, xAxis_ps1, yAxis_ps0, yAxis_ps1, zAxis_ps0, zAxis_ps1, wAxis_ps0, wAxis_ps1);

		Vector4Template<float> result;
		ps_store_0( result_ps0, &result.x );
		ps_store_0( result_ps1, &result.z );
		return result;
	}

	template<>
	Matrix44Template<float> Mult(const Matrix44Template<float> &m, const Matrix44Template<float> &b)
	{
		using namespace ::rw::math::ps::detail;
		mw_ps r_xAxis_ps0, r_xAxis_ps1, r_yAxis_ps0, r_yAxis_ps1, r_zAxis_ps0, r_zAxis_ps1, r_wAxis_ps0, r_wAxis_ps1;
		mw_ps m_xAxis_ps0 = ps_load_0(&m.xAxis.x);
		mw_ps m_xAxis_ps1 = ps_load_0(&m.xAxis.z);
		mw_ps m_yAxis_ps0 = ps_load_0(&m.yAxis.x);
		mw_ps m_yAxis_ps1 = ps_load_0(&m.yAxis.z);
		mw_ps m_zAxis_ps0 = ps_load_0(&m.zAxis.x);
		mw_ps m_zAxis_ps1 = ps_load_0(&m.zAxis.z);
		mw_ps m_wAxis_ps0 = ps_load_0(&m.wAxis.x);
		mw_ps m_wAxis_ps1 = ps_load_0(&m.wAxis.z);
		mw_ps b_xAxis_ps0 = ps_load_0(&b.xAxis.x);
		mw_ps b_xAxis_ps1 = ps_load_0(&b.xAxis.z);
		mw_ps b_yAxis_ps0 = ps_load_0(&b.yAxis.x);
		mw_ps b_yAxis_ps1 = ps_load_0(&b.yAxis.z);
		mw_ps b_zAxis_ps0 = ps_load_0(&b.zAxis.x);
		mw_ps b_zAxis_ps1 = ps_load_0(&b.zAxis.z);
		mw_ps b_wAxis_ps0 = ps_load_0(&b.wAxis.x);
		mw_ps b_wAxis_ps1 = ps_load_0(&b.wAxis.z);

		RW_MATH_PS_MultM44(r_xAxis_ps0, r_xAxis_ps1, r_yAxis_ps0, r_yAxis_ps1, r_zAxis_ps0, r_zAxis_ps1, r_wAxis_ps0, r_wAxis_ps1, m_xAxis_ps0, m_xAxis_ps1, m_yAxis_ps0, m_yAxis_ps1, m_zAxis_ps0, m_zAxis_ps1, m_wAxis_ps0, m_wAxis_ps1, b_xAxis_ps0, b_xAxis_ps1, b_yAxis_ps0, b_yAxis_ps1, b_zAxis_ps0, b_zAxis_ps1, b_wAxis_ps0, b_wAxis_ps1);
	
		Matrix44Template<float> result;
		ps_store_0(r_xAxis_ps0, &result.xAxis.x);
		ps_store_0(r_xAxis_ps1, &result.xAxis.z);
		ps_store_0(r_yAxis_ps0, &result.yAxis.x);
		ps_store_0(r_yAxis_ps1, &result.yAxis.z);
		ps_store_0(r_zAxis_ps0, &result.zAxis.x);
		ps_store_0(r_zAxis_ps1, &result.zAxis.z);
		ps_store_0(r_wAxis_ps0, &result.wAxis.x);
		ps_store_0(r_wAxis_ps1, &result.wAxis.z);
        return result;
	}


} // namespace fpu
} // namespace math
} // namespace rw